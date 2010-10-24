/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id: AliAnalysisTaskCaloFilter.cxx $ */

//////////////////////////////////////////////////////////
// Filter the ESDCaloClusters and ESDCaloCells of EMCAL,
// PHOS or both, creating the corresponing AODCaloClusters
// and AODCaloCells.
// Keep also the AODHeader information and the vertex.
// Needed for calorimeter calibration.
// Copy of AliAnalysisTaskESDfilter.
// Author: Gustavo Conesa Balbastre (INFN - Frascati)
//////////////////////////////////////////////////////////

#include "AliAnalysisTaskCaloFilter.h"
#include "AliESDEvent.h"
#include "AliAODEvent.h"
#include "AliLog.h"
#include "AliVCluster.h"
#include "AliVCaloCells.h"
#include "AliEMCALRecoUtils.h"
#include "AliEMCALGeometry.h"
#include "AliVEventHandler.h"
#include "AliAnalysisManager.h"
#include "AliInputEventHandler.h"

ClassImp(AliAnalysisTaskCaloFilter)
  
////////////////////////////////////////////////////////////////////////

AliAnalysisTaskCaloFilter::AliAnalysisTaskCaloFilter():
  AliAnalysisTaskSE(), //fCuts(0x0),
  fCaloFilter(0), fCorrect(kFALSE), 
  fEMCALGeo(0x0),fEMCALGeoName("EMCAL_FIRSTYEAR"), 
  fEMCALRecoUtils(new AliEMCALRecoUtils)
{
  // Default constructor
}

//__________________________________________________
AliAnalysisTaskCaloFilter::AliAnalysisTaskCaloFilter(const char* name):
  AliAnalysisTaskSE(name), //fCuts(0x0),
  fCaloFilter(0), fCorrect(kFALSE),
  fEMCALGeo(0x0),fEMCALGeoName("EMCAL_FIRSTYEAR"), 
  fEMCALRecoUtils(new AliEMCALRecoUtils)
{
  // Constructor
}

//__________________________________________________
AliAnalysisTaskCaloFilter::~AliAnalysisTaskCaloFilter()
{
  //Destructor.
	
  if(fEMCALGeo)       delete fEMCALGeo;	
  if(fEMCALRecoUtils) delete fEMCALRecoUtils ;
  
}

//__________________________________________________
void AliAnalysisTaskCaloFilter::UserCreateOutputObjects()
{
  // Init geometry 
	
  fEMCALGeo =  AliEMCALGeometry::GetInstance(fEMCALGeoName) ;	
  
}  

//__________________________________________________
void AliAnalysisTaskCaloFilter::UserExec(Option_t */*option*/)
{
  // Execute analysis for current event
  //
  
  //Magic line to write events to file
  AliAnalysisManager::GetAnalysisManager()->GetOutputEventHandler()->SetFillAOD(kTRUE);
  
  if (fDebug > 0)  
    printf("CaloFilter: Analysing event # %d\n", (Int_t)Entry());
  
  // Copy input ESD or AOD header, vertex, CaloClusters and CaloCells to output AOD
  
  AliVEvent* event = InputEvent();
  Bool_t bAOD = kFALSE;
  if(!strcmp(event->GetName(),"AliAODEvent")) bAOD=kTRUE;
  Bool_t bESD = kFALSE;
  if(!strcmp(event->GetName(),"AliESDEvent")) bESD=kTRUE;
  
  if(!event) {
    printf("AliAnalysisTaskCaloFilter::CreateAODFromESD() - This event does not contain Input?");
    return;
  }
  
  // set arrays and pointers
  Float_t posF[3];
  Double_t pos[3];
  
  Double_t covVtx[6];
  
  for (Int_t i = 0; i < 6; i++)  covVtx[i] = 0.;
      
  AliAODHeader* header = AODEvent()->GetHeader();
  
  header->SetRunNumber(event->GetRunNumber());
  header->SetOfflineTrigger(fInputHandler->IsEventSelected()); // propagate the decision of the physics selection
  if(bESD)
    header->SetFiredTriggerClasses(((AliESDEvent*)event)->GetFiredTriggerClasses());
  header->SetTriggerMask(event->GetTriggerMask()); 
  header->SetTriggerCluster(event->GetTriggerCluster());
  
  header->SetBunchCrossNumber(event->GetBunchCrossNumber());
  header->SetOrbitNumber(event->GetOrbitNumber());
  header->SetPeriodNumber(event->GetPeriodNumber());
  header->SetEventType(event->GetEventType());
  header->SetMuonMagFieldScale(-999.); // FIXME
  header->SetCentrality(-999.);        // FIXME
  
  
  header->SetTriggerMask(event->GetTriggerMask()); 
  header->SetTriggerCluster(event->GetTriggerCluster());
  header->SetMagneticField(event->GetMagneticField());
  header->SetZDCN1Energy(event->GetZDCN1Energy());
  header->SetZDCP1Energy(event->GetZDCP1Energy());
  header->SetZDCN2Energy(event->GetZDCN2Energy());
  header->SetZDCP2Energy(event->GetZDCP2Energy());
  header->SetZDCEMEnergy(event->GetZDCEMEnergy(0),event->GetZDCEMEnergy(1));
  Float_t diamxy[2]={event->GetDiamondX(),event->GetDiamondY()};
  Float_t diamcov[3]; event->GetDiamondCovXY(diamcov);
  header->SetDiamond(diamxy,diamcov);
  if(bESD){
    header->SetDiamondZ(((AliESDEvent*)event)->GetDiamondZ(),((AliESDEvent*)event)->GetSigma2DiamondZ());
  }
  //
  //
  Int_t nVertices = 1 ;/* = prim. vtx*/;
  Int_t nCaloClus = event->GetNumberOfCaloClusters();
  
  AODEvent()->ResetStd(0, nVertices, 0, 0, 0, nCaloClus, 0, 0);
  
  // Access to the AOD container of vertices
  TClonesArray &vertices = *(AODEvent()->GetVertices());
  Int_t jVertices=0;
  
  // Add primary vertex. The primary tracks will be defined
  // after the loops on the composite objects (V0, cascades, kinks)
  event->GetPrimaryVertex()->GetXYZ(pos);
  Float_t chi = 0;
  if      (bESD){
    ((AliESDEvent*)event)->GetPrimaryVertex()->GetCovMatrix(covVtx);
    chi = ((AliESDEvent*)event)->GetPrimaryVertex()->GetChi2toNDF();
  }
  else if (bAOD){
    ((AliAODEvent*)event)->GetPrimaryVertex()->GetCovMatrix(covVtx);
    chi = ((AliAODEvent*)event)->GetPrimaryVertex()->GetChi2perNDF();//Different from ESD?
  }
  
  AliAODVertex * primary = new(vertices[jVertices++])
    AliAODVertex(pos, covVtx, chi, NULL, -1, AliAODVertex::kPrimary);
  primary->SetName(event->GetPrimaryVertex()->GetName());
  primary->SetTitle(event->GetPrimaryVertex()->GetTitle());
  
  // Access to the AOD container of clusters
  TClonesArray &caloClusters = *(AODEvent()->GetCaloClusters());
  Int_t jClusters=0;
  
  for (Int_t iClust=0; iClust<nCaloClus; ++iClust) {
    
    AliVCluster * cluster = event->GetCaloCluster(iClust);
    
    //Check which calorimeter information we want to keep.
    
    if(fCaloFilter!=kBoth){
      if     (fCaloFilter==kPHOS  && cluster->IsEMCAL()) continue ;
      else if(fCaloFilter==kEMCAL && cluster->IsPHOS())  continue ;
    }  
    
    //--------------------------------------------------------------
    //If EMCAL, and requested, correct energy, position ...
    //printf("Hello IsEMCAL? %d, Correct? %d\n",cluster->IsEMCAL(), fCorrect);
    if(cluster->IsEMCAL() && fCorrect){
      Float_t position[]={0,0,0};
      if(DebugLevel() > 2)
        printf("Check cluster %d for bad channels and close to border\n",cluster->GetID());
      if(fEMCALRecoUtils->ClusterContainsBadChannel(fEMCALGeo,cluster->GetCellsAbsId(), cluster->GetNCells())) continue;	
      if(fEMCALRecoUtils->CheckCellFiducialRegion(fEMCALGeo, cluster, event->GetEMCALCells())) continue;
      if(DebugLevel() > 2)
      { 
        printf("Filter, before  : i %d, E %f, dispersion %f, m02 %f, m20 %f\n",iClust,cluster->E(),
               cluster->GetDispersion(),cluster->GetM02(),cluster->GetM20());
        cluster->GetPosition(position);
        printf("Filter, before  : i %d, x %f, y %f, z %f\n",cluster->GetID(), position[0], position[1], position[2]);
      }
            
      if(fEMCALRecoUtils->IsRecalibrationOn())	fEMCALRecoUtils->RecalibrateClusterEnergy(fEMCALGeo, cluster, event->GetEMCALCells());
      cluster->SetE(fEMCALRecoUtils->CorrectClusterEnergyLinearity(cluster));
      fEMCALRecoUtils->RecalculateClusterPosition(fEMCALGeo, event->GetEMCALCells(),cluster);
      
      if(DebugLevel() > 2)
      { 
        printf("Filter, after   : i %d, E %f, dispersion %f, m02 %f, m20 %f\n",cluster->GetID(),cluster->E(),
               cluster->GetDispersion(),cluster->GetM02(),cluster->GetM20());
        cluster->GetPosition(position);
        printf("Filter, after   : i %d, x %f, y %f, z %f\n",cluster->GetID(), position[0], position[1], position[2]);
      }    
      
    }
    //--------------------------------------------------------------

    //Now fill AODs
    Int_t id       = cluster->GetID();
    Float_t energy = cluster->E();
    cluster->GetPosition(posF);
    
    AliAODCaloCluster *caloCluster = new(caloClusters[jClusters++]) 
      AliAODCaloCluster(id,
			0,
			0x0,
			energy,
			posF,
			NULL,
			cluster->GetType());
    
    caloCluster->SetCaloCluster(cluster->GetDistanceToBadChannel(),
				cluster->GetDispersion(),
				cluster->GetM20(), cluster->GetM02(),
				cluster->GetEmcCpvDistance(),  
				cluster->GetNExMax(),cluster->GetTOF()) ;
    
    caloCluster->SetPIDFromESD(cluster->GetPID());
    caloCluster->SetNCells(cluster->GetNCells());
    caloCluster->SetCellsAbsId(cluster->GetCellsAbsId());
    caloCluster->SetCellsAmplitudeFraction(cluster->GetCellsAmplitudeFraction());
    
    if(DebugLevel() > 2)
    { 
      printf("Filter, aod       : i %d, E %f, dispersion %f, m02 %f, m20 %f\n",caloCluster->GetID(),caloCluster->E(),
             caloCluster->GetDispersion(),caloCluster->GetM02(),caloCluster->GetM20());
      caloCluster->GetPosition(posF);
      printf("Filter, aod       : i %d, x %f, y %f, z %f\n",caloCluster->GetID(), posF[0], posF[1], posF[2]);
    }    
    
  } 
  caloClusters.Expand(jClusters); // resize TObjArray to 'remove' slots for pseudo clusters	 
  // end of loop on calo clusters
  
  // fill EMCAL cell info
  if ((fCaloFilter==kBoth ||  fCaloFilter==kEMCAL) && event->GetEMCALCells()) { // protection against missing ESD information
    AliVCaloCells &eventEMcells = *(event->GetEMCALCells());
    Int_t nEMcell = eventEMcells.GetNumberOfCells() ;
    
    AliAODCaloCells &aodEMcells = *(AODEvent()->GetEMCALCells());
    aodEMcells.CreateContainer(nEMcell);
    aodEMcells.SetType(AliVCaloCells::kEMCALCell);
    Double_t calibFactor = 1.;   
    for (Int_t iCell = 0; iCell < nEMcell; iCell++) { 
      if(fCorrect && fEMCALRecoUtils->IsRecalibrationOn()){ 
        Int_t imod = -1, iphi =-1, ieta=-1,iTower = -1, iIphi = -1, iIeta = -1; 
        fEMCALGeo->GetCellIndex(eventEMcells.GetCellNumber(iCell),imod,iTower,iIphi,iIeta); 
        fEMCALGeo->GetCellPhiEtaIndexInSModule(imod,iTower,iIphi, iIeta,iphi,ieta);	
        calibFactor = fEMCALRecoUtils->GetEMCALChannelRecalibrationFactor(imod,ieta,iphi);
      }
      aodEMcells.SetCell(iCell,eventEMcells.GetCellNumber(iCell),eventEMcells.GetAmplitude(iCell)*calibFactor);
    }
    aodEMcells.Sort();
  }
  
  // fill PHOS cell info
  if ((fCaloFilter==kBoth ||  fCaloFilter==kPHOS) && event->GetPHOSCells()) { // protection against missing ESD information
    AliVCaloCells &eventPHcells = *(event->GetPHOSCells());
    Int_t nPHcell = eventPHcells.GetNumberOfCells() ;
    
    AliAODCaloCells &aodPHcells = *(AODEvent()->GetPHOSCells());
    aodPHcells.CreateContainer(nPHcell);
    aodPHcells.SetType(AliVCaloCells::kPHOSCell);
    for (Int_t iCell = 0; iCell < nPHcell; iCell++) {      
      aodPHcells.SetCell(iCell,eventPHcells.GetCellNumber(iCell),eventPHcells.GetAmplitude(iCell));
    }
    aodPHcells.Sort();
  }
  
  
  return;
}

//_____________________________________________________
//void AliAnalysisTaskCaloFilter::LocalInit()
//{
//	// Local Initialization
//	
//	// Create cuts/param objects and publish to slot
//	const Int_t buffersize = 255;
//	char onePar[buffersize] ;
//	fCuts = new TList();
//  
//	snprintf(onePar,buffersize, "Calorimeter Filtering Option %d", fCaloFilter) ;
//	fCuts->Add(new TObjString(onePar));
//	snprintf(onePar,buffersize, "Not only filter but correct? %d cells;", fCorrect) ;
//	fCuts->Add(new TObjString(onePar));
//  
//	// Post Data
//	PostData(2, fCuts);
//	
//}


//__________________________________________________
void AliAnalysisTaskCaloFilter::Terminate(Option_t */*option*/)
{
  // Terminate analysis
  //
    if (fDebug > 1) printf("AnalysisCaloFilter: Terminate() \n");
}

