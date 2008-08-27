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

/* $Id$ */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// class for ITS reconstruction                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "Riostream.h"
#include "AliITSReconstructor.h"
#include "AliRun.h"
#include "AliRawReader.h"
#include "AliITSDetTypeRec.h"
#include "AliITSgeom.h"
#include "AliITSLoader.h"
#include "AliITStrackerMI.h"
#include "AliITStrackerV2.h"
#include "AliITStrackerSA.h"
#include "AliITSVertexerIons.h"
#include "AliITSVertexerFast.h"
#include "AliITSVertexerFixed.h"
#include "AliITSVertexer3D.h"
#include "AliITSVertexerZ.h"
#include "AliITSVertexerCosmics.h"
#include "AliESDEvent.h"
#include "AliITSpidESD.h"
#include "AliITSpidESD1.h"
#include "AliITSpidESD2.h"
#include "AliITSInitGeometry.h"


ClassImp(AliITSReconstructor)

//___________________________________________________________________________
AliITSReconstructor::AliITSReconstructor() : AliReconstructor(),
fItsPID(0),
fDetTypeRec(0)
{
  // Default constructor
}
 //___________________________________________________________________________
AliITSReconstructor::~AliITSReconstructor(){
// destructor
  delete fItsPID;
  if(fDetTypeRec) delete fDetTypeRec;
} 
//______________________________________________________________________
AliITSReconstructor::AliITSReconstructor(const AliITSReconstructor &ob) :AliReconstructor(ob),
fItsPID(ob.fItsPID),
fDetTypeRec(ob.fDetTypeRec)

{
  // Copy constructor
}

//______________________________________________________________________
AliITSReconstructor& AliITSReconstructor::operator=(const AliITSReconstructor&  ob ){
  // Assignment operator
  this->~AliITSReconstructor();
  new(this) AliITSReconstructor(ob);
  return *this;
}

//______________________________________________________________________
void AliITSReconstructor::Init() {
    // Initalize this constructor bet getting/creating the objects
    // nesseary for a proper ITS reconstruction.
    // Inputs:
    //   none.
    // Output:
    //   none.
    // Return:
    //   none.

    AliITSInitGeometry initgeom;
    AliITSgeom *geom = initgeom.CreateAliITSgeom();
    AliInfo(Form("Geometry name: %s",(initgeom.GetGeometryName()).Data()));

    fDetTypeRec = new AliITSDetTypeRec();
    fDetTypeRec->SetITSgeom(geom);
    fDetTypeRec->SetDefaults();

    return;
}

//_____________________________________________________________________________
void AliITSReconstructor::Reconstruct(TTree *digitsTree, TTree *clustersTree) const
{
// reconstruct clusters

  Int_t cluFindOpt = GetRecoParam()->GetClusterFinder();
  Bool_t useV2=kTRUE;   // Default: V2 cluster finder
  if(cluFindOpt==1) useV2=kFALSE;

  fDetTypeRec->SetTreeAddressD(digitsTree);
  fDetTypeRec->MakeBranch(clustersTree,"R");
  fDetTypeRec->SetTreeAddressR(clustersTree);
  fDetTypeRec->DigitsToRecPoints(digitsTree,clustersTree,0,"All",useV2);    
}

//_________________________________________________________________
void AliITSReconstructor::Reconstruct(AliRawReader* rawReader, TTree *clustersTree) const
{
  // reconstruct clusters from raw data
 
  fDetTypeRec->SetDefaultClusterFindersV2(kTRUE);
  fDetTypeRec->DigitsToRecPoints(rawReader,clustersTree);
}

//_____________________________________________________________________________
AliTracker* AliITSReconstructor::CreateTracker() const
{
// create a ITS tracker

  Int_t trackerOpt = GetRecoParam()->GetTracker();
  AliTracker* tracker;    
  if (trackerOpt==1) {
    tracker = new AliITStrackerMI(0);
    AliITStrackerMI *mit=(AliITStrackerMI*)tracker;
    mit->SetDetTypeRec(fDetTypeRec);
  }  
  else if (trackerOpt==2) {
    tracker = new AliITStrackerV2(0);
  }
  else {
    tracker =  new AliITStrackerSA(0);  // inherits from AliITStrackerMI
    AliITStrackerSA *sat=(AliITStrackerSA*)tracker;
    sat->SetDetTypeRec(fDetTypeRec);
    if(GetRecoParam()->GetTrackerSAOnly()) sat->SetSAFlag(kTRUE);
    if(sat->GetSAFlag())AliDebug(1,"Tracking Performed in ITS only\n");
    sat->SetOuterStartLayer(GetRecoParam()->GetOuterStartLayerSA());
  }

  Int_t pidOpt = GetRecoParam()->GetPID();

  AliITSReconstructor* nc = const_cast<AliITSReconstructor*>(this);
  if(pidOpt==1){
    Info("FillESD","ITS LandauFitPID option has been selected\n");
    nc->fItsPID = new AliITSpidESD2((AliITStrackerMI*)tracker);
  }
  else{
    Info("FillESD","ITS default PID\n");
    Double_t parITS[] = {79.,0.13, 5.}; //IB: this is  "pp tuning"
    nc->fItsPID = new AliITSpidESD1(parITS);
  }
 
  return tracker;
  
}

//_____________________________________________________________________________
AliVertexer* AliITSReconstructor::CreateVertexer() const
{
// create a ITS vertexer

  Int_t vtxOpt = GetRecoParam()->GetVertexer();
  if(vtxOpt==3){
    Info("CreateVertexer","a AliITSVertexerIons object has been selected\n");
    return new AliITSVertexerIons();
  }
  if(vtxOpt==4){
    Double_t smear[3]={0.005,0.005,0.01};
    Info("CreateVertexer","a AliITSVertexerFast object has been selected\n"); 
    return new AliITSVertexerFast(smear);
  }
  if(vtxOpt==1){
    Info("CreateVertexer","a AliITSVertexerZ object has been selected\n");
    return new AliITSVertexerZ();
  }
  if(vtxOpt==2){
    Info("CreateVertexer","a AliITSVertexerCosmics object has been selected\n");
    return new AliITSVertexerCosmics();
  }
  if(vtxOpt==5){ 
    Info("CreateVertexer","vertex is fixed in the position of the TDI\n");
    return new AliITSVertexerFixed("TDI");
  }
  if(vtxOpt==6){ 
    Info("CreateVertexer","vertex is fixed in the position of the TED\n");
    return new AliITSVertexerFixed("TED");
  }
  // by default an AliITSVertexer3D object is instatiated
  Info("CreateVertexer","a AliITSVertexer3D object has been selected\n");
  AliITSVertexer3D*  vtxr = new AliITSVertexer3D();
  Float_t dzw=GetRecoParam()->GetVertexer3DWideFiducialRegionZ();
  Float_t drw=GetRecoParam()->GetVertexer3DWideFiducialRegionR();
  vtxr->SetWideFiducialRegion(dzw,drw);
  Float_t dzn=GetRecoParam()->GetVertexer3DNarrowFiducialRegionZ();
  Float_t drn=GetRecoParam()->GetVertexer3DNarrowFiducialRegionR();
  vtxr->SetNarrowFiducialRegion(dzn,drn);
  Float_t dphil=GetRecoParam()->GetVertexer3DLooseDeltaPhiCut();
  Float_t dphit=GetRecoParam()->GetVertexer3DTightDeltaPhiCut();
  vtxr->SetDeltaPhiCuts(dphil,dphit);
  Float_t dcacut=GetRecoParam()->GetVertexer3DDCACut();
  vtxr->SetDCACut(dcacut);
  return vtxr;
}

//_____________________________________________________________________________
void AliITSReconstructor::FillESD(TTree * /*digitsTree*/, TTree *clustersTree, 
				  AliESDEvent* esd) const
{
// make PID, find V0s and cascade
  if(fItsPID!=0) {
    Int_t pidOpt = GetRecoParam()->GetPID();
    if(pidOpt==1){
      fItsPID->MakePID(clustersTree,esd);
    }else{
      fItsPID->MakePID(esd);
    }
  }
  else {
    Error("FillESD","!! cannot do the PID !!\n");
  }
}
