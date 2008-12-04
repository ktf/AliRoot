/**************************************************************************
 * Copyright(c) 2007-2009, ALICE Experiment at CERN, All rights reserved. *
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

//-------------------------------------------------------------------------
//               Implementation of the ITS tracker class
//    It reads AliITSRecPoint clusters and creates AliITStrackMI tracks
//                   and fills with them the ESD
//          Origin: Marian Ivanov, CERN, Marian.Ivanov@cern.ch 
//          Current support and development: 
//                     Andrea Dainese, andrea.dainese@lnl.infn.it
//     dE/dx analysis by: Boris Batyunya, JINR, Boris.Batiounia@cern.ch
//     Params moved to AliITSRecoParam by: Andrea Dainese, INFN
//     Material budget from TGeo by: Ludovic Gaudichet & Andrea Dainese, INFN
//-------------------------------------------------------------------------

#include <TMatrixD.h>
#include <TTree.h>
#include <TTreeStream.h>
#include <TDatabasePDG.h>
#include <TString.h>
#include <TRandom.h>


#include "AliLog.h"
#include "AliESDEvent.h"
#include "AliESDtrack.h"
#include "AliESDVertex.h"
#include "AliV0.h"
#include "AliHelix.h"
#include "AliITSRecPoint.h"
#include "AliITSgeomTGeo.h"
#include "AliITSReconstructor.h"
#include "AliTrackPointArray.h"
#include "AliAlignObj.h"
#include "AliITSClusterParam.h"
#include "AliCDBManager.h"
#include "AliCDBEntry.h"
#include "AliITSsegmentation.h"
#include "AliITSCalibration.h"
#include "AliITSCalibrationSPD.h"
#include "AliITSCalibrationSDD.h"
#include "AliITSCalibrationSSD.h"
#include "AliITSPlaneEff.h"
#include "AliITSPlaneEffSPD.h"
#include "AliITSPlaneEffSDD.h"
#include "AliITSPlaneEffSSD.h"
#include "AliITStrackerMI.h"

ClassImp(AliITStrackerMI)

AliITStrackerMI::AliITSlayer AliITStrackerMI::fgLayers[AliITSgeomTGeo::kNLayers]; // ITS layers

AliITStrackerMI::AliITStrackerMI():AliTracker(),
fI(0),
fBestTrack(),
fTrackToFollow(),
fTrackHypothesys(),
fBestHypothesys(),
fOriginal(),
fCurrentEsdTrack(),
fPass(0),
fAfterV0(kFALSE),
fLastLayerToTrackTo(0),
fCoefficients(0),
fEsd(0),
fTrackingPhase("Default"),
fUseTGeo(3),
fNtracks(0),
fxOverX0Pipe(-1.),
fxTimesRhoPipe(-1.),
fxOverX0PipeTrks(0),
fxTimesRhoPipeTrks(0),
fxOverX0ShieldTrks(0),
fxTimesRhoShieldTrks(0),
fxOverX0LayerTrks(0),
fxTimesRhoLayerTrks(0),
fDebugStreamer(0),
fITSChannelStatus(0),
fDetTypeRec(0),
fPlaneEff(0) {
  //Default constructor
  Int_t i;
  for(i=0;i<4;i++) fSPDdetzcentre[i]=0.;
  for(i=0;i<2;i++) {fxOverX0Shield[i]=-1.;fxTimesRhoShield[i]=-1.;}
  for(i=0;i<6;i++) {fxOverX0Layer[i]=-1.;fxTimesRhoLayer[i]=-1.;}
}
//------------------------------------------------------------------------
AliITStrackerMI::AliITStrackerMI(const Char_t *geom) : AliTracker(),
fI(AliITSgeomTGeo::GetNLayers()),
fBestTrack(),
fTrackToFollow(),
fTrackHypothesys(),
fBestHypothesys(),
fOriginal(),
fCurrentEsdTrack(),
fPass(0),
fAfterV0(kFALSE),
fLastLayerToTrackTo(AliITSRecoParam::GetLastLayerToTrackTo()),
fCoefficients(0),
fEsd(0),
fTrackingPhase("Default"),
fUseTGeo(3),
fNtracks(0),
fxOverX0Pipe(-1.),
fxTimesRhoPipe(-1.),
fxOverX0PipeTrks(0),
fxTimesRhoPipeTrks(0),
fxOverX0ShieldTrks(0),
fxTimesRhoShieldTrks(0),
fxOverX0LayerTrks(0),
fxTimesRhoLayerTrks(0),
fDebugStreamer(0),
fITSChannelStatus(0),
fDetTypeRec(0),
fPlaneEff(0) {
  //--------------------------------------------------------------------
  //This is the AliITStrackerMI constructor
  //--------------------------------------------------------------------
  if (geom) {
    AliWarning("\"geom\" is actually a dummy argument !");
  }

  fCoefficients = 0;
  fAfterV0     = kFALSE;

  for (Int_t i=1; i<AliITSgeomTGeo::GetNLayers()+1; i++) {
    Int_t nlad=AliITSgeomTGeo::GetNLadders(i);
    Int_t ndet=AliITSgeomTGeo::GetNDetectors(i);

    Double_t xyz[3], &x=xyz[0], &y=xyz[1], &z=xyz[2];
    AliITSgeomTGeo::GetOrigTranslation(i,1,1,xyz); 
    Double_t poff=TMath::ATan2(y,x);
    Double_t zoff=z;
    Double_t r=TMath::Sqrt(x*x + y*y);

    AliITSgeomTGeo::GetOrigTranslation(i,1,2,xyz);
    r += TMath::Sqrt(x*x + y*y);
    AliITSgeomTGeo::GetOrigTranslation(i,2,1,xyz);
    r += TMath::Sqrt(x*x + y*y);
    AliITSgeomTGeo::GetOrigTranslation(i,2,2,xyz);
    r += TMath::Sqrt(x*x + y*y);
    r*=0.25;

    new (fgLayers+i-1) AliITSlayer(r,poff,zoff,nlad,ndet);

    for (Int_t j=1; j<nlad+1; j++) {
      for (Int_t k=1; k<ndet+1; k++) { //Fill this layer with detectors
        TGeoHMatrix m; AliITSgeomTGeo::GetOrigMatrix(i,j,k,m);
        const TGeoHMatrix *tm=AliITSgeomTGeo::GetTracking2LocalMatrix(i,j,k);
        m.Multiply(tm);
        Double_t txyz[3]={0.};
	xyz[0]=0.;xyz[1]=0.;xyz[2]=0.;
        m.LocalToMaster(txyz,xyz);
        r=TMath::Sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1]);
        Double_t phi=TMath::ATan2(xyz[1],xyz[0]);

        if (phi<0) phi+=TMath::TwoPi();
        else if (phi>=TMath::TwoPi()) phi-=TMath::TwoPi();

        AliITSdetector &det=fgLayers[i-1].GetDetector((j-1)*ndet + k-1); 
        new(&det) AliITSdetector(r,phi); 
	// compute the real radius (with misalignment)
        TGeoHMatrix mmisal(*(AliITSgeomTGeo::GetMatrix(i,j,k)));
        mmisal.Multiply(tm);
	xyz[0]=0.;xyz[1]=0.;xyz[2]=0.;
        mmisal.LocalToMaster(txyz,xyz);
        Double_t rmisal=TMath::Sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1]);
	det.SetRmisal(rmisal);
	
      } // end loop on detectors
    } // end loop on ladders
  } // end loop on layers


  fI=AliITSgeomTGeo::GetNLayers();

  fPass=0;
  fConstraint[0]=1; fConstraint[1]=0;

  Double_t xyzVtx[]={AliITSReconstructor::GetRecoParam()->GetXVdef(),
		     AliITSReconstructor::GetRecoParam()->GetYVdef(),
		     AliITSReconstructor::GetRecoParam()->GetZVdef()}; 
  Double_t ersVtx[]={AliITSReconstructor::GetRecoParam()->GetSigmaXVdef(),
		     AliITSReconstructor::GetRecoParam()->GetSigmaYVdef(),
		     AliITSReconstructor::GetRecoParam()->GetSigmaZVdef()}; 
  SetVertex(xyzVtx,ersVtx);

  for (Int_t i=0; i<AliITSgeomTGeo::GetNLayers(); i++) fLayersNotToSkip[i]=AliITSRecoParam::GetLayersNotToSkip(i);
  fLastLayerToTrackTo=AliITSRecoParam::GetLastLayerToTrackTo();
  for (Int_t i=0;i<100000;i++){
    fBestTrackIndex[i]=0;
  }

  // store positions of centre of SPD modules (in z)
  Double_t tr[3];
  AliITSgeomTGeo::GetTranslation(1,1,1,tr);
  fSPDdetzcentre[0] = tr[2];
  AliITSgeomTGeo::GetTranslation(1,1,2,tr);
  fSPDdetzcentre[1] = tr[2];
  AliITSgeomTGeo::GetTranslation(1,1,3,tr);
  fSPDdetzcentre[2] = tr[2];
  AliITSgeomTGeo::GetTranslation(1,1,4,tr);
  fSPDdetzcentre[3] = tr[2];

  fUseTGeo = AliITSReconstructor::GetRecoParam()->GetUseTGeoInTracker();
  if(AliITSReconstructor::GetRecoParam()->GetExtendedEtaAcceptance() && fUseTGeo!=1 && fUseTGeo!=3) {
    AliWarning("fUseTGeo changed to 3 because fExtendedEtaAcceptance is kTRUE");
    fUseTGeo = 3;
  }

  for(Int_t i=0;i<2;i++) {fxOverX0Shield[i]=-1.;fxTimesRhoShield[i]=-1.;}
  for(Int_t i=0;i<6;i++) {fxOverX0Layer[i]=-1.;fxTimesRhoLayer[i]=-1.;}
  
  fDebugStreamer = new TTreeSRedirector("ITSdebug.root");

  // only for plane efficiency evaluation
  if (AliITSReconstructor::GetRecoParam()->GetComputePlaneEff()) {
    Int_t iplane=AliITSReconstructor::GetRecoParam()->GetIPlanePlaneEff();
    if(AliITSReconstructor::GetRecoParam()->GetLayersToSkip(iplane))
      AliWarning(Form("Evaluation of Plane Eff for layer %d will be attempted without removing it from tracker",iplane));
    if (iplane<2) fPlaneEff = new AliITSPlaneEffSPD();
    else if (iplane<4) fPlaneEff = new AliITSPlaneEffSDD();
    else fPlaneEff = new AliITSPlaneEffSSD();
    if(AliITSReconstructor::GetRecoParam()->GetReadPlaneEffFromOCDB())
       if(!fPlaneEff->ReadFromCDB()) {AliWarning("AliITStrackerMI reading of AliITSPlaneEff from OCDB failed") ;}
    if(AliITSReconstructor::GetRecoParam()->GetHistoPlaneEff()) fPlaneEff->SetCreateHistos(kTRUE);
  }
}
//------------------------------------------------------------------------
AliITStrackerMI::AliITStrackerMI(const AliITStrackerMI &tracker):AliTracker(tracker),
fI(tracker.fI),
fBestTrack(tracker.fBestTrack),
fTrackToFollow(tracker.fTrackToFollow),
fTrackHypothesys(tracker.fTrackHypothesys),
fBestHypothesys(tracker.fBestHypothesys),
fOriginal(tracker.fOriginal),
fCurrentEsdTrack(tracker.fCurrentEsdTrack),
fPass(tracker.fPass),
fAfterV0(tracker.fAfterV0),
fLastLayerToTrackTo(tracker.fLastLayerToTrackTo),
fCoefficients(tracker.fCoefficients),
fEsd(tracker.fEsd),
fTrackingPhase(tracker.fTrackingPhase),
fUseTGeo(tracker.fUseTGeo),
fNtracks(tracker.fNtracks),
fxOverX0Pipe(tracker.fxOverX0Pipe),
fxTimesRhoPipe(tracker.fxTimesRhoPipe),
fxOverX0PipeTrks(0),
fxTimesRhoPipeTrks(0),
fxOverX0ShieldTrks(0),
fxTimesRhoShieldTrks(0),
fxOverX0LayerTrks(0),
fxTimesRhoLayerTrks(0),
fDebugStreamer(tracker.fDebugStreamer),
fITSChannelStatus(tracker.fITSChannelStatus),
fDetTypeRec(tracker.fDetTypeRec),
fPlaneEff(tracker.fPlaneEff) {
  //Copy constructor
  Int_t i;
  for(i=0;i<4;i++) {
    fSPDdetzcentre[i]=tracker.fSPDdetzcentre[i];
  }
  for(i=0;i<6;i++) {
    fxOverX0Layer[i]=tracker.fxOverX0Layer[i];
    fxTimesRhoLayer[i]=tracker.fxTimesRhoLayer[i];
  }
  for(i=0;i<2;i++) {
    fxOverX0Shield[i]=tracker.fxOverX0Shield[i];
    fxTimesRhoShield[i]=tracker.fxTimesRhoShield[i];
  }
}
//------------------------------------------------------------------------
AliITStrackerMI & AliITStrackerMI::operator=(const AliITStrackerMI &tracker){
  //Assignment operator
  this->~AliITStrackerMI();
  new(this) AliITStrackerMI(tracker);
  return *this;
}
//------------------------------------------------------------------------
AliITStrackerMI::~AliITStrackerMI()
{
  //
  //destructor
  //
  if (fCoefficients) delete [] fCoefficients;
  DeleteTrksMaterialLUT();
  if (fDebugStreamer) {
    //fDebugStreamer->Close();
    delete fDebugStreamer;
  }
  if(fITSChannelStatus) delete fITSChannelStatus;
  if(fPlaneEff) delete fPlaneEff;
}
//------------------------------------------------------------------------
void AliITStrackerMI::SetLayersNotToSkip(Int_t *l) {
  //--------------------------------------------------------------------
  //This function set masks of the layers which must be not skipped
  //--------------------------------------------------------------------
  for (Int_t i=0; i<AliITSgeomTGeo::GetNLayers(); i++) fLayersNotToSkip[i]=l[i];
}
//------------------------------------------------------------------------
void AliITStrackerMI::ReadBadFromDetTypeRec() {
  //--------------------------------------------------------------------
  //This function read ITS bad detectors, chips, channels from AliITSDetTypeRec
  //i.e. from OCDB
  //--------------------------------------------------------------------

  if(!AliITSReconstructor::GetRecoParam()->GetUseBadZonesFromOCDB()) return;

  Info("ReadBadFromDetTypeRec","Reading info about bad ITS detectors and channels");

  if(!fDetTypeRec) Error("ReadBadFromDetTypeRec","AliITSDetTypeRec nof found!\n");

  // ITS channels map
  if(fITSChannelStatus) delete fITSChannelStatus;
  fITSChannelStatus = new AliITSChannelStatus(fDetTypeRec);

  // ITS detectors and chips
  Int_t i=0,j=0,k=0,ndet=0;
  for (i=1; i<AliITSgeomTGeo::GetNLayers()+1; i++) {
    Int_t nBadDetsPerLayer=0;
    ndet=AliITSgeomTGeo::GetNDetectors(i);    
    for (j=1; j<AliITSgeomTGeo::GetNLadders(i)+1; j++) {
      for (k=1; k<ndet+1; k++) {
        AliITSdetector &det=fgLayers[i-1].GetDetector((j-1)*ndet + k-1);  
	det.ReadBadDetectorAndChips(i-1,(j-1)*ndet + k-1,fDetTypeRec);
	if(det.IsBad()) {nBadDetsPerLayer++;}
      } // end loop on detectors
    } // end loop on ladders
    Info("ReadBadFromDetTypeRec",Form("Layer %d: %d bad out of %d",i-1,nBadDetsPerLayer,ndet*AliITSgeomTGeo::GetNLadders(i)));
  } // end loop on layers
  
  return;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::LoadClusters(TTree *cTree) {
  //--------------------------------------------------------------------
  //This function loads ITS clusters
  //--------------------------------------------------------------------
  TBranch *branch=cTree->GetBranch("ITSRecPoints");
  if (!branch) { 
    Error("LoadClusters"," can't get the branch !\n");
    return 1;
  }

  static TClonesArray dummy("AliITSRecPoint",10000), *clusters=&dummy;
  branch->SetAddress(&clusters);

  Int_t i=0,j=0,ndet=0;
  Int_t detector=0;
  for (i=0; i<AliITSgeomTGeo::GetNLayers(); i++) {
    ndet=fgLayers[i].GetNdetectors();
    Int_t jmax = j + fgLayers[i].GetNladders()*ndet;
    for (; j<jmax; j++) {           
      if (!cTree->GetEvent(j)) continue;
      Int_t ncl=clusters->GetEntriesFast();
      SignDeltas(clusters,GetZ());
 
      while (ncl--) {
        AliITSRecPoint *c=(AliITSRecPoint*)clusters->UncheckedAt(ncl);
        detector=c->GetDetectorIndex();

	if (!c->Misalign()) AliWarning("Can't misalign this cluster !");

        fgLayers[i].InsertCluster(new AliITSRecPoint(*c));
      }
      clusters->Delete();
      // add dead zone "virtual" cluster in SPD, if there is a cluster within 
      // zwindow cm from the dead zone      
      if (i<2 && AliITSReconstructor::GetRecoParam()->GetAddVirtualClustersInDeadZone()) {
	for (Float_t xdead = 0; xdead < AliITSRecoParam::GetSPDdetxlength(); xdead += (i+1.)*AliITSReconstructor::GetRecoParam()->GetXPassDeadZoneHits()) {
	  Int_t lab[4]   = {0,0,0,detector};
	  Int_t info[3]  = {0,0,i};
	  Float_t q      = 0.; // this identifies virtual clusters
	  Float_t hit[5] = {xdead,
			    0.,
			    AliITSReconstructor::GetRecoParam()->GetSigmaXDeadZoneHit2(),
			    AliITSReconstructor::GetRecoParam()->GetSigmaZDeadZoneHit2(),
			    q};
	  Bool_t local   = kTRUE;
	  Double_t zwindow = AliITSReconstructor::GetRecoParam()->GetZWindowDeadZone();
	  hit[1] = fSPDdetzcentre[0]+0.5*AliITSRecoParam::GetSPDdetzlength();
	  if (TMath::Abs(fgLayers[i].GetDetector(detector).GetZmax()-hit[1])<zwindow) 
	    fgLayers[i].InsertCluster(new AliITSRecPoint(lab,hit,info,local));
	  hit[1] = fSPDdetzcentre[1]-0.5*AliITSRecoParam::GetSPDdetzlength();
	  if (TMath::Abs(fgLayers[i].GetDetector(detector).GetZmax()-hit[1])<zwindow) 
	    fgLayers[i].InsertCluster(new AliITSRecPoint(lab,hit,info,local));
	  hit[1] = fSPDdetzcentre[1]+0.5*AliITSRecoParam::GetSPDdetzlength();
	  if (TMath::Abs(fgLayers[i].GetDetector(detector).GetZmax()-hit[1])<zwindow) 
	    fgLayers[i].InsertCluster(new AliITSRecPoint(lab,hit,info,local));
	  hit[1] = fSPDdetzcentre[2]-0.5*AliITSRecoParam::GetSPDdetzlength();
	  if (TMath::Abs(fgLayers[i].GetDetector(detector).GetZmax()-hit[1])<zwindow) 
	    fgLayers[i].InsertCluster(new AliITSRecPoint(lab,hit,info,local));
	  hit[1] = fSPDdetzcentre[2]+0.5*AliITSRecoParam::GetSPDdetzlength();
	  if (TMath::Abs(fgLayers[i].GetDetector(detector).GetZmax()-hit[1])<zwindow) 
	    fgLayers[i].InsertCluster(new AliITSRecPoint(lab,hit,info,local));
	  hit[1] = fSPDdetzcentre[3]-0.5*AliITSRecoParam::GetSPDdetzlength();
	  if (TMath::Abs(fgLayers[i].GetDetector(detector).GetZmax()-hit[1])<zwindow) 
	    fgLayers[i].InsertCluster(new AliITSRecPoint(lab,hit,info,local));
	}
      } // "virtual" clusters in SPD
      
    }
    //
    fgLayers[i].ResetRoad(); //road defined by the cluster density
    fgLayers[i].SortClusters();
  }

  dummy.Clear();

  return 0;
}
//------------------------------------------------------------------------
void AliITStrackerMI::UnloadClusters() {
  //--------------------------------------------------------------------
  //This function unloads ITS clusters
  //--------------------------------------------------------------------
  for (Int_t i=0; i<AliITSgeomTGeo::GetNLayers(); i++) fgLayers[i].ResetClusters();
}
//------------------------------------------------------------------------
void AliITStrackerMI::FillClusterArray(TObjArray* array) const {
  //--------------------------------------------------------------------
  // Publishes all pointers to clusters known to the tracker into the
  // passed object array.
  // The ownership is not transfered - the caller is not expected to delete
  // the clusters.
  //--------------------------------------------------------------------

  for(Int_t i=0; i<AliITSgeomTGeo::GetNLayers(); i++) {
    for(Int_t icl=0; icl<fgLayers[i].GetNumberOfClusters(); icl++) {
      AliCluster *cl = (AliCluster*)fgLayers[i].GetCluster(icl);
      array->AddLast(cl);
    }
  }

  return;
}
//------------------------------------------------------------------------
static Int_t CorrectForTPCtoITSDeadZoneMaterial(AliITStrackMI *t) {
  //--------------------------------------------------------------------
  // Correction for the material between the TPC and the ITS
  //--------------------------------------------------------------------
  if (t->GetX() > AliITSRecoParam::Getriw()) {   // inward direction 
      if (!t->PropagateToTGeo(AliITSRecoParam::Getriw(),1)) return 0;// TPC inner wall
      if (!t->PropagateToTGeo(AliITSRecoParam::Getrcd(),1)) return 0;// TPC central drum
      if (!t->PropagateToTGeo(AliITSRecoParam::Getrs(),1))  return 0;// ITS screen
  } else if (t->GetX() < AliITSRecoParam::Getrs()) {  // outward direction
      if (!t->PropagateToTGeo(AliITSRecoParam::Getrs(),1))        return 0;// ITS screen
      if (!t->PropagateToTGeo(AliITSRecoParam::Getrcd(),1))       return 0;// TPC central drum
      if (!t->PropagateToTGeo(AliITSRecoParam::Getriw()+0.001,1)) return 0;// TPC inner wall
  } else {
    Error("CorrectForTPCtoITSDeadZoneMaterial","Track is already in the dead zone !");
    return 0;
  }
  
  return 1;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::Clusters2Tracks(AliESDEvent *event) {
  //--------------------------------------------------------------------
  // This functions reconstructs ITS tracks
  // The clusters must be already loaded !
  //--------------------------------------------------------------------


  fTrackingPhase="Clusters2Tracks";

  TObjArray itsTracks(15000);
  fOriginal.Clear();
  fEsd = event;         // store pointer to the esd 

  // temporary (for cosmics)
  if(event->GetVertex()) {
    TString title = event->GetVertex()->GetTitle();
    if(title.Contains("cosmics")) {
      Double_t xyz[3]={GetX(),GetY(),GetZ()};
      Double_t exyz[3]={0.1,0.1,0.1};
      SetVertex(xyz,exyz);
    }
  }
  // temporary

  {/* Read ESD tracks */
    Double_t pimass = TDatabasePDG::Instance()->GetParticle(211)->Mass();
    Int_t nentr=event->GetNumberOfTracks();
    Info("Clusters2Tracks", "Number of ESD tracks: %d\n", nentr);
    while (nentr--) {
      AliESDtrack *esd=event->GetTrack(nentr);
      //  ---- for debugging:
      //if(TMath::Abs(esd->GetX()-83.65)<0.1) { FILE *f=fopen("tpc.dat","a"); fprintf(f,"%f %f %f %f %f %f\n",(Float_t)event->GetEventNumberInFile(),(Float_t)TMath::Abs(esd->GetLabel()),(Float_t)esd->GetX(),(Float_t)esd->GetY(),(Float_t)esd->GetZ(),(Float_t)esd->Pt()); fclose(f); }

      if ((esd->GetStatus()&AliESDtrack::kTPCin)==0) continue;
      if (esd->GetStatus()&AliESDtrack::kTPCout) continue;
      if (esd->GetStatus()&AliESDtrack::kITSin) continue;
      if (esd->GetKinkIndex(0)>0) continue;   //kink daughter
      AliITStrackMI *t=0;
      try {
        t=new AliITStrackMI(*esd);
      } catch (const Char_t *msg) {
        //Warning("Clusters2Tracks",msg);
        delete t;
        continue;
      }
      t->GetDZ(GetX(),GetY(),GetZ(),t->GetDP());              //I.B.
      Double_t vdist = TMath::Sqrt(t->GetD(0)*t->GetD(0)+t->GetD(1)*t->GetD(1));


      // look at the ESD mass hypothesys !
      if (t->GetMass()<0.9*pimass) t->SetMass(pimass); 
      // write expected q
      t->SetExpQ(TMath::Max(0.8*t->GetESDtrack()->GetTPCsignal(),30.));

      if (esd->GetV0Index(0)>0 && t->GetD(0)<AliITSReconstructor::GetRecoParam()->GetMaxDforV0dghtrForProlongation()){
	//track - can be  V0 according to TPC
      } else {	
	if (TMath::Abs(t->GetD(0))>AliITSReconstructor::GetRecoParam()->GetMaxDForProlongation()) {
	  delete t;
	  continue;
	}	
	if (TMath::Abs(vdist)>AliITSReconstructor::GetRecoParam()->GetMaxDZForProlongation()) {
	  delete t;
	  continue;
	}
	if (t->Pt()<AliITSReconstructor::GetRecoParam()->GetMinPtForProlongation()) {
	  delete t;
	  continue;
	}
	if (!CorrectForTPCtoITSDeadZoneMaterial(t)) {
	  delete t;
	  continue;
	}
      }
      t->SetReconstructed(kFALSE);
      itsTracks.AddLast(t);
      fOriginal.AddLast(t);
    }
  } /* End Read ESD tracks */

  itsTracks.Sort();
  fOriginal.Sort();
  Int_t nentr=itsTracks.GetEntriesFast();
  fTrackHypothesys.Expand(nentr);
  fBestHypothesys.Expand(nentr);
  MakeCoefficients(nentr);
  if(fUseTGeo==3 || fUseTGeo==4) MakeTrksMaterialLUT(event->GetNumberOfTracks());
  Int_t ntrk=0;
  // THE TWO TRACKING PASSES
  for (fPass=0; fPass<2; fPass++) {
     Int_t &constraint=fConstraint[fPass]; if (constraint<0) continue;
     for (fCurrentEsdTrack=0; fCurrentEsdTrack<nentr; fCurrentEsdTrack++) {
       AliITStrackMI *t=(AliITStrackMI*)itsTracks.UncheckedAt(fCurrentEsdTrack);
       if (t==0) continue;              //this track has been already tracked
       //cout<<"========== "<<fPass<<"    "<<fCurrentEsdTrack<<" =========\n";
       if (t->GetReconstructed()&&(t->GetNUsed()<1.5)) continue;  //this track was  already  "succesfully" reconstructed
       Float_t dz[2]; t->GetDZ(GetX(),GetY(),GetZ(),dz);              //I.B.
       if (fConstraint[fPass]) { 
	 if (TMath::Abs(dz[0])>AliITSReconstructor::GetRecoParam()->GetMaxDZToUseConstraint() ||
	     TMath::Abs(dz[1])>AliITSReconstructor::GetRecoParam()->GetMaxDZToUseConstraint()) continue;
       }

       Int_t tpcLabel=t->GetLabel(); //save the TPC track label       
       AliDebug(2,Form("LABEL %d pass %d",tpcLabel,fPass));
       fI = 6;
       ResetTrackToFollow(*t);
       ResetBestTrack();

       FollowProlongationTree(t,fCurrentEsdTrack,fConstraint[fPass]);
 

       SortTrackHypothesys(fCurrentEsdTrack,20,0);  //MI change
       //
       AliITStrackMI *besttrack = GetBestHypothesys(fCurrentEsdTrack,t,15);
       if (!besttrack) continue;
       besttrack->SetLabel(tpcLabel);
       //       besttrack->CookdEdx();
       CookdEdx(besttrack);
       besttrack->SetFakeRatio(1.);
       CookLabel(besttrack,0.); //For comparison only
       UpdateESDtrack(besttrack,AliESDtrack::kITSin);

       if (fConstraint[fPass]&&(!besttrack->IsGoldPrimary())) continue;  //to be tracked also without vertex constrain 

       t->SetReconstructed(kTRUE);
       ntrk++;  
       AliDebug(2,Form("TRACK! (label %d) ncls %d",besttrack->GetLabel(),besttrack->GetNumberOfClusters()));
     }
     GetBestHypothesysMIP(itsTracks); 
  } // end loop on the two tracking passes

  if(event->GetNumberOfV0s()>0) UpdateTPCV0(event);
  if(AliITSReconstructor::GetRecoParam()->GetFindV0s()) FindV02(event);
  fAfterV0 = kTRUE;
  //
  itsTracks.Delete();
  //
  Int_t entries = fTrackHypothesys.GetEntriesFast();
  for (Int_t ientry=0; ientry<entries; ientry++) {
    TObjArray * array =(TObjArray*)fTrackHypothesys.UncheckedAt(ientry);
    if (array) array->Delete();
    delete fTrackHypothesys.RemoveAt(ientry); 
  }

  fTrackHypothesys.Delete();
  fBestHypothesys.Delete();
  fOriginal.Clear();
  delete [] fCoefficients;
  fCoefficients=0;
  DeleteTrksMaterialLUT();

  Info("Clusters2Tracks","Number of prolonged tracks: %d\n",ntrk);

  fTrackingPhase="Default";
  
  return 0;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::PropagateBack(AliESDEvent *event) {
  //--------------------------------------------------------------------
  // This functions propagates reconstructed ITS tracks back
  // The clusters must be loaded !
  //--------------------------------------------------------------------
  fTrackingPhase="PropagateBack";
  Int_t nentr=event->GetNumberOfTracks();
  Info("PropagateBack", "Number of ESD tracks: %d\n", nentr);

  Int_t ntrk=0;
  for (Int_t i=0; i<nentr; i++) {
     AliESDtrack *esd=event->GetTrack(i);

     if ((esd->GetStatus()&AliESDtrack::kITSin)==0) continue;
     if (esd->GetStatus()&AliESDtrack::kITSout) continue;

     AliITStrackMI *t=0;
     try {
        t=new AliITStrackMI(*esd);
     } catch (const Char_t *msg) {
       //Warning("PropagateBack",msg);
        delete t;
        continue;
     }
     t->SetExpQ(TMath::Max(0.8*t->GetESDtrack()->GetTPCsignal(),30.));

     ResetTrackToFollow(*t);

     // propagate to vertex [SR, GSI 17.02.2003]
     // Start Time measurement [SR, GSI 17.02.2003], corrected by I.Belikov
     if (CorrectForPipeMaterial(&fTrackToFollow,"inward")) {
       if (fTrackToFollow.PropagateToVertex(event->GetVertex()))
	 fTrackToFollow.StartTimeIntegral();
       // from vertex to outside pipe
       CorrectForPipeMaterial(&fTrackToFollow,"outward");
     }

     fTrackToFollow.ResetCovariance(10.); fTrackToFollow.ResetClusters();
     if (RefitAt(AliITSRecoParam::GetrInsideITSscreen(),&fTrackToFollow,t)) {
        if (!CorrectForTPCtoITSDeadZoneMaterial(&fTrackToFollow)) {
          delete t;
          continue;
        }
        fTrackToFollow.SetLabel(t->GetLabel());
        //fTrackToFollow.CookdEdx();
        CookLabel(&fTrackToFollow,0.); //For comparison only
        fTrackToFollow.UpdateESDtrack(AliESDtrack::kITSout);
        //UseClusters(&fTrackToFollow);
        ntrk++;
     }
     delete t;
  }

  Info("PropagateBack","Number of back propagated ITS tracks: %d\n",ntrk);

  fTrackingPhase="Default";

  return 0;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::RefitInward(AliESDEvent *event) {
  //--------------------------------------------------------------------
  // This functions refits ITS tracks using the 
  // "inward propagated" TPC tracks
  // The clusters must be loaded !
  //--------------------------------------------------------------------
  fTrackingPhase="RefitInward";
  if(AliITSReconstructor::GetRecoParam()->GetFindV0s()) RefitV02(event);
  Int_t nentr=event->GetNumberOfTracks();
  Info("RefitInward", "Number of ESD tracks: %d\n", nentr);

  Int_t ntrk=0;
  for (Int_t i=0; i<nentr; i++) {
    AliESDtrack *esd=event->GetTrack(i);

    if ((esd->GetStatus()&AliESDtrack::kITSout) == 0) continue;
    if (esd->GetStatus()&AliESDtrack::kITSrefit) continue;
    if (esd->GetStatus()&AliESDtrack::kTPCout)
      if ((esd->GetStatus()&AliESDtrack::kTPCrefit)==0) continue;

    AliITStrackMI *t=0;
    try {
        t=new AliITStrackMI(*esd);
    } catch (const Char_t *msg) {
      //Warning("RefitInward",msg);
        delete t;
        continue;
    }
    t->SetExpQ(TMath::Max(0.8*t->GetESDtrack()->GetTPCsignal(),30.));
    if (!CorrectForTPCtoITSDeadZoneMaterial(t)) {
       delete t;
       continue;
    }

    ResetTrackToFollow(*t);
    fTrackToFollow.ResetClusters();

    if ((esd->GetStatus()&AliESDtrack::kTPCin)==0)
      fTrackToFollow.ResetCovariance(10.);

    //Refitting...
    Bool_t pe=AliITSReconstructor::GetRecoParam()->GetComputePlaneEff();
    AliDebug(2,Form("Refit LABEL %d  %d",t->GetLabel(),t->GetNumberOfClusters()));
    if (RefitAt(AliITSRecoParam::GetrInsideSPD1(),&fTrackToFollow,t,kTRUE,pe)) {
       AliDebug(2,"  refit OK");
       fTrackToFollow.SetLabel(t->GetLabel());
       //       fTrackToFollow.CookdEdx();
       CookdEdx(&fTrackToFollow);

       CookLabel(&fTrackToFollow,0.0); //For comparison only

       //The beam pipe
       if (CorrectForPipeMaterial(&fTrackToFollow,"inward")) {
	 fTrackToFollow.UpdateESDtrack(AliESDtrack::kITSrefit);
	 AliESDtrack  *esdTrack =fTrackToFollow.GetESDtrack();
	 //printf("                                       %d\n",esdTrack->GetITSModuleIndex(0));
	 //esdTrack->UpdateTrackParams(&fTrackToFollow,AliESDtrack::kITSrefit); //original line
	 Float_t r[3]={0.,0.,0.};
	 Double_t maxD=3.;
	 esdTrack->RelateToVertex(event->GetVertex(),GetBz(r),maxD);
	 ntrk++;
       }
    }
    delete t;
  }

  Info("RefitInward","Number of refitted tracks: %d\n",ntrk);

  fTrackingPhase="Default";

  return 0;
}
//------------------------------------------------------------------------
AliCluster *AliITStrackerMI::GetCluster(Int_t index) const {
  //--------------------------------------------------------------------
  //       Return pointer to a given cluster
  //--------------------------------------------------------------------
  Int_t l=(index & 0xf0000000) >> 28;
  Int_t c=(index & 0x0fffffff) >> 00;
  return fgLayers[l].GetCluster(c);
}
//------------------------------------------------------------------------
Bool_t AliITStrackerMI::GetTrackPoint(Int_t index, AliTrackPoint& p) const {
  //--------------------------------------------------------------------
  // Get track space point with index i
  //--------------------------------------------------------------------

  Int_t l=(index & 0xf0000000) >> 28;
  Int_t c=(index & 0x0fffffff) >> 00;
  AliITSRecPoint *cl = fgLayers[l].GetCluster(c);
  Int_t idet = cl->GetDetectorIndex();

  Float_t xyz[3];
  Float_t cov[6];
  cl->GetGlobalXYZ(xyz);
  cl->GetGlobalCov(cov);
  p.SetXYZ(xyz, cov);
  p.SetCharge(cl->GetQ());
  p.SetDriftTime(cl->GetDriftTime());
  AliGeomManager::ELayerID iLayer = AliGeomManager::kInvalidLayer; 
  switch (l) {
  case 0:
    iLayer = AliGeomManager::kSPD1;
    break;
  case 1:
    iLayer = AliGeomManager::kSPD2;
    break;
  case 2:
    iLayer = AliGeomManager::kSDD1;
    break;
  case 3:
    iLayer = AliGeomManager::kSDD2;
    break;
  case 4:
    iLayer = AliGeomManager::kSSD1;
    break;
  case 5:
    iLayer = AliGeomManager::kSSD2;
    break;
  default:
    AliWarning(Form("Wrong layer index in ITS (%d) !",l));
    break;
  };
  UShort_t volid = AliGeomManager::LayerToVolUID(iLayer,idet);
  p.SetVolumeID((UShort_t)volid);
  return kTRUE;
}
//------------------------------------------------------------------------
Bool_t AliITStrackerMI::GetTrackPointTrackingError(Int_t index, 
			AliTrackPoint& p, const AliESDtrack *t) {
  //--------------------------------------------------------------------
  // Get track space point with index i
  // (assign error estimated during the tracking)
  //--------------------------------------------------------------------

  Int_t l=(index & 0xf0000000) >> 28;
  Int_t c=(index & 0x0fffffff) >> 00;
  const AliITSRecPoint *cl = fgLayers[l].GetCluster(c);
  Int_t idet = cl->GetDetectorIndex();

  const AliITSdetector &det=fgLayers[l].GetDetector(idet);

  // tgphi and tglambda of the track in tracking frame with alpha=det.GetPhi
  Float_t detxy[2];
  detxy[0] = det.GetR()*TMath::Cos(det.GetPhi());
  detxy[1] = det.GetR()*TMath::Sin(det.GetPhi());
  Double_t alpha = t->GetAlpha();
  Double_t xdetintrackframe = detxy[0]*TMath::Cos(alpha)+detxy[1]*TMath::Sin(alpha);
  Float_t phi = TMath::ASin(t->GetSnpAt(xdetintrackframe,AliTracker::GetBz()));
  phi += alpha-det.GetPhi();
  Float_t tgphi = TMath::Tan(phi);

  Float_t tgl = t->GetTgl(); // tgl about const along track
  Float_t expQ = TMath::Max(0.8*t->GetTPCsignal(),30.);

  Float_t errlocalx,errlocalz;
  Bool_t addMisalErr=kFALSE;
  AliITSClusterParam::GetError(l,cl,tgl,tgphi,expQ,errlocalx,errlocalz,addMisalErr);

  Float_t xyz[3];
  Float_t cov[6];
  cl->GetGlobalXYZ(xyz);
  //  cl->GetGlobalCov(cov);
  Float_t pos[3] = {0.,0.,0.};
  AliCluster tmpcl((UShort_t)cl->GetVolumeId(),pos[0],pos[1],pos[2],errlocalx*errlocalx,errlocalz*errlocalz,0);
  tmpcl.GetGlobalCov(cov);

  p.SetXYZ(xyz, cov);
  p.SetCharge(cl->GetQ());
  p.SetDriftTime(cl->GetDriftTime());

  AliGeomManager::ELayerID iLayer = AliGeomManager::kInvalidLayer; 
  switch (l) {
  case 0:
    iLayer = AliGeomManager::kSPD1;
    break;
  case 1:
    iLayer = AliGeomManager::kSPD2;
    break;
  case 2:
    iLayer = AliGeomManager::kSDD1;
    break;
  case 3:
    iLayer = AliGeomManager::kSDD2;
    break;
  case 4:
    iLayer = AliGeomManager::kSSD1;
    break;
  case 5:
    iLayer = AliGeomManager::kSSD2;
    break;
  default:
    AliWarning(Form("Wrong layer index in ITS (%d) !",l));
    break;
  };
  UShort_t volid = AliGeomManager::LayerToVolUID(iLayer,idet);

  p.SetVolumeID((UShort_t)volid);
  return kTRUE;
}
//------------------------------------------------------------------------
void AliITStrackerMI::FollowProlongationTree(AliITStrackMI * otrack, Int_t esdindex, Bool_t constrain) 
{
  //--------------------------------------------------------------------
  // Follow prolongation tree
  //--------------------------------------------------------------------
  //
  Double_t xyzVtx[]={GetX(),GetY(),GetZ()};
  Double_t ersVtx[]={GetSigmaX(),GetSigmaY(),GetSigmaZ()};


  AliESDtrack * esd = otrack->GetESDtrack();
  if (esd->GetV0Index(0)>0) {
    // TEMPORARY SOLLUTION: map V0 indexes to point to proper track
    //                      mapping of ESD track is different as ITS track in Containers
    //                      Need something more stable
    //                      Indexes are set back again to the ESD track indexes in UpdateTPCV0
    for (Int_t i=0;i<3;i++){
      Int_t  index = esd->GetV0Index(i);
      if (index==0) break;
      AliESDv0 * vertex = fEsd->GetV0(index);
      if (vertex->GetStatus()<0) continue;     // rejected V0
      //
      if (esd->GetSign()>0) {
	vertex->SetIndex(0,esdindex);
      } else {
	vertex->SetIndex(1,esdindex);
      }
    }
  }
  TObjArray *bestarray = (TObjArray*)fBestHypothesys.At(esdindex);
  if (!bestarray){
    bestarray = new TObjArray(5);
    fBestHypothesys.AddAt(bestarray,esdindex);
  }

  //
  //setup tree of the prolongations
  //
  static AliITStrackMI tracks[7][100];
  AliITStrackMI *currenttrack;
  static AliITStrackMI currenttrack1;
  static AliITStrackMI currenttrack2;  
  static AliITStrackMI backuptrack;
  Int_t ntracks[7];
  Int_t nindexes[7][100];
  Float_t normalizedchi2[100];
  for (Int_t ilayer=0;ilayer<6;ilayer++) ntracks[ilayer]=0;
  otrack->SetNSkipped(0);
  new (&(tracks[6][0])) AliITStrackMI(*otrack);
  ntracks[6]=1;
  for (Int_t i=0;i<7;i++) nindexes[i][0]=0;
  Int_t modstatus = 1; // found 
  Float_t xloc,zloc;
  // 
  //
  // follow prolongations
  for (Int_t ilayer=5; ilayer>=0; ilayer--) {
    AliDebug(2,Form("FollowProlongationTree: layer %d",ilayer));
    fI = ilayer;
    //
    AliITSlayer &layer=fgLayers[ilayer];
    Double_t r = layer.GetR(); 
    ntracks[ilayer]=0;
    //
    //
    Int_t nskipped=0;
    Float_t nused =0;
    for (Int_t itrack =0; itrack<ntracks[ilayer+1]; itrack++) {
      //set current track
      if (ntracks[ilayer]>=100) break;  
      if (tracks[ilayer+1][nindexes[ilayer+1][itrack]].GetNSkipped()>0) nskipped++;
      if (tracks[ilayer+1][nindexes[ilayer+1][itrack]].GetNUsed()>2.) nused++;
      if (ntracks[ilayer]>15+ilayer){
	if (itrack>1&&tracks[ilayer+1][nindexes[ilayer+1][itrack]].GetNSkipped()>0 && nskipped>4+ilayer) continue;
	if (itrack>1&&tracks[ilayer+1][nindexes[ilayer+1][itrack]].GetNUsed()>2. && nused>3) continue;
      }

      new(&currenttrack1)  AliITStrackMI(tracks[ilayer+1][nindexes[ilayer+1][itrack]]);
  
      // material between SSD and SDD, SDD and SPD
      if (ilayer==3) 
	if(!CorrectForShieldMaterial(&currenttrack1,"SDD","inward")) continue;
      if (ilayer==1) 
	if(!CorrectForShieldMaterial(&currenttrack1,"SPD","inward")) continue;

      // detector number
      Double_t phi,z;
      if (!currenttrack1.GetPhiZat(r,phi,z)) continue;
      Int_t idet=layer.FindDetectorIndex(phi,z);

      Double_t trackGlobXYZ1[3];
      if (!currenttrack1.GetXYZ(trackGlobXYZ1)) continue;

      // Get the budget to the primary vertex for the current track being prolonged
      Double_t budgetToPrimVertex = GetEffectiveThickness();

      // check if we allow a prolongation without point
      Int_t skip = CheckSkipLayer(&currenttrack1,ilayer,idet);
      if (skip) {
	AliITStrackMI* vtrack = new (&tracks[ilayer][ntracks[ilayer]]) AliITStrackMI(currenttrack1);
	// propagate to the layer radius
	Double_t xToGo; if (!vtrack->GetLocalXat(r,xToGo)) continue;
	if(!vtrack->AliExternalTrackParam::PropagateTo(xToGo,GetBz())) continue;
	// apply correction for material of the current layer
	CorrectForLayerMaterial(vtrack,ilayer,trackGlobXYZ1,"inward");
	vtrack->SetNDeadZone(vtrack->GetNDeadZone()+1);
	vtrack->SetClIndex(ilayer,0);
	modstatus = (skip==1 ? 3 : 4); // skipped : out in z
	if(LocalModuleCoord(ilayer,idet,vtrack,xloc,zloc)) { // local module coords
	  vtrack->SetModuleIndexInfo(ilayer,idet,modstatus,xloc,zloc);
	}
	if(constrain) vtrack->Improve(budgetToPrimVertex,xyzVtx,ersVtx);
	ntracks[ilayer]++;
	continue;
      }

      // track outside layer acceptance in z
      if (idet<0) continue;
      
      //propagate to the intersection with the detector plane
      const AliITSdetector &det=layer.GetDetector(idet);
      new(&currenttrack2)  AliITStrackMI(currenttrack1);
      if (!currenttrack1.Propagate(det.GetPhi(),det.GetR())) continue;
      if (!currenttrack2.Propagate(det.GetPhi(),det.GetR())) continue;
      currenttrack1.SetDetectorIndex(idet);
      currenttrack2.SetDetectorIndex(idet);
      if(!LocalModuleCoord(ilayer,idet,&currenttrack1,xloc,zloc)) continue; // local module coords

      //***************
      // DEFINITION OF SEARCH ROAD AND CLUSTERS SELECTION
      //
      // road in global (rphi,z) [i.e. in tracking ref. system]
      Double_t zmin,zmax,ymin,ymax;
      if (!ComputeRoad(&currenttrack1,ilayer,idet,zmin,zmax,ymin,ymax)) continue;

      // select clusters in road
      layer.SelectClusters(zmin,zmax,ymin,ymax); 
      //********************

      // Define criteria for track-cluster association
      Double_t msz = currenttrack1.GetSigmaZ2() + 
	AliITSReconstructor::GetRecoParam()->GetNSigmaZLayerForRoadZ()*
	AliITSReconstructor::GetRecoParam()->GetNSigmaZLayerForRoadZ()*
	AliITSReconstructor::GetRecoParam()->GetSigmaZ2(ilayer);
      Double_t msy = currenttrack1.GetSigmaY2() + 
	AliITSReconstructor::GetRecoParam()->GetNSigmaYLayerForRoadY()*
	AliITSReconstructor::GetRecoParam()->GetNSigmaYLayerForRoadY()*
	AliITSReconstructor::GetRecoParam()->GetSigmaY2(ilayer);
      if (constrain) {
	msz *= AliITSReconstructor::GetRecoParam()->GetNSigma2RoadZC();
	msy *= AliITSReconstructor::GetRecoParam()->GetNSigma2RoadYC(); 
      }  else {
	msz *= AliITSReconstructor::GetRecoParam()->GetNSigma2RoadZNonC();
	msy *= AliITSReconstructor::GetRecoParam()->GetNSigma2RoadYNonC(); 
      }
      msz = 1./msz; // 1/RoadZ^2
      msy = 1./msy; // 1/RoadY^2

      //
      //
      // LOOP OVER ALL POSSIBLE TRACK PROLONGATIONS ON THIS LAYER
      //
      const AliITSRecPoint *cl=0; 
      Int_t clidx=-1;
      Double_t chi2trkcl=AliITSReconstructor::GetRecoParam()->GetMaxChi2(); // init with big value
      Bool_t deadzoneSPD=kFALSE;
      currenttrack = &currenttrack1;

      // check if the road contains a dead zone 
      Bool_t noClusters = kFALSE;
      if (!layer.GetNextCluster(clidx,kTRUE)) noClusters=kTRUE;
      if (noClusters) AliDebug(2,"no clusters in road");
      Double_t dz=0.5*(zmax-zmin);
      Double_t dy=0.5*(ymax-ymin);
      Int_t dead = CheckDeadZone(&currenttrack1,ilayer,idet,dz,dy,noClusters); 
      if(dead) AliDebug(2,Form("DEAD (%d)\n",dead));
      // create a prolongation without clusters (check also if there are no clusters in the road)
      if (dead || 
	  (noClusters && 
	   AliITSReconstructor::GetRecoParam()->GetAllowProlongationWithEmptyRoad())) {
	AliITStrackMI * updatetrack = new (&tracks[ilayer][ntracks[ilayer]]) AliITStrackMI(*currenttrack);
	updatetrack->SetClIndex(ilayer,0);
	if (dead==0) {
	  modstatus = 5; // no cls in road
	} else if (dead==1) {
	  modstatus = 7; // holes in z in SPD
	} else if (dead==2 || dead==3) {
	  modstatus = 2; // dead from OCDB
	}
	updatetrack->SetModuleIndexInfo(ilayer,idet,modstatus,xloc,zloc);
	// apply correction for material of the current layer
	CorrectForLayerMaterial(updatetrack,ilayer,trackGlobXYZ1,"inward");
	if (constrain) { // apply vertex constrain
	  updatetrack->SetConstrain(constrain);
	  Bool_t isPrim = kTRUE;
	  if (ilayer<4) { // check that it's close to the vertex
	    updatetrack->GetDZ(GetX(),GetY(),GetZ(),updatetrack->GetDP()); //I.B.
	    if (TMath::Abs(updatetrack->GetD(0)/(1.+ilayer)) > // y
		AliITSReconstructor::GetRecoParam()->GetMaxDZforPrimTrk() || 
		TMath::Abs(updatetrack->GetD(1)/(1.+ilayer)) > // z
		AliITSReconstructor::GetRecoParam()->GetMaxDZforPrimTrk()) isPrim=kFALSE;
	  }
	  if (isPrim) updatetrack->Improve(budgetToPrimVertex,xyzVtx,ersVtx);
	}
	if (dead) {
	  updatetrack->SetNDeadZone(updatetrack->GetNDeadZone()+1);
	  if (dead==1) { // dead zone at z=0,+-7cm in SPD
	    updatetrack->SetDeadZoneProbability(GetSPDDeadZoneProbability(updatetrack->GetZ(),TMath::Sqrt(updatetrack->GetSigmaZ2())));
	    deadzoneSPD=kTRUE;
	  }
	}
	ntracks[ilayer]++;
      }

      clidx=-1;
      // loop over clusters in the road
      while ((cl=layer.GetNextCluster(clidx))!=0) { 
	if (ntracks[ilayer]>95) break; //space for skipped clusters  
	Bool_t changedet =kFALSE;  
	if (cl->GetQ()==0 && deadzoneSPD==kTRUE) continue;
	Int_t idetc=cl->GetDetectorIndex();

	if (currenttrack->GetDetectorIndex()==idetc) { // track already on the cluster's detector
	  // take into account misalignment (bring track to real detector plane)
	  Double_t xTrOrig = currenttrack->GetX();
	  if (!currenttrack->PropagateTo(xTrOrig+cl->GetX(),0.,0.)) continue;
	  // a first cut on track-cluster distance
	  if ( (currenttrack->GetZ()-cl->GetZ())*(currenttrack->GetZ()-cl->GetZ())*msz + 
	       (currenttrack->GetY()-cl->GetY())*(currenttrack->GetY()-cl->GetY())*msy > 1. ) 
	    {  // cluster not associated to track
	      AliDebug(2,"not associated");
	      continue;
	    }
	  // bring track back to ideal detector plane
	  if (!currenttrack->PropagateTo(xTrOrig,0.,0.)) continue;
	} else {                                      // have to move track to cluster's detector
	  const AliITSdetector &detc=layer.GetDetector(idetc);
	  // a first cut on track-cluster distance
	  Double_t y;
	  if (!currenttrack2.GetProlongationFast(detc.GetPhi(),detc.GetR()+cl->GetX(),y,z)) continue;
	  if ( (z-cl->GetZ())*(z-cl->GetZ())*msz + 
	       (y-cl->GetY())*(y-cl->GetY())*msy > 1. ) 
	    continue; // cluster not associated to track
	  //
	  new (&backuptrack) AliITStrackMI(currenttrack2);
	  changedet = kTRUE;
	  currenttrack =&currenttrack2;
	  if (!currenttrack->Propagate(detc.GetPhi(),detc.GetR())) {
	    new (currenttrack) AliITStrackMI(backuptrack);
	    changedet = kFALSE;
	    continue;
	  }
	  currenttrack->SetDetectorIndex(idetc);
	  // Get again the budget to the primary vertex 
	  // for the current track being prolonged, if had to change detector 
	  //budgetToPrimVertex = GetEffectiveThickness();// not needed at the moment because anyway we take a mean material for this correction
	}

	// calculate track-clusters chi2
	chi2trkcl = GetPredictedChi2MI(currenttrack,cl,ilayer); 
	// chi2 cut
	AliDebug(2,Form("chi2 %f max %f",chi2trkcl,AliITSReconstructor::GetRecoParam()->GetMaxChi2s(ilayer)));
	if (chi2trkcl < AliITSReconstructor::GetRecoParam()->GetMaxChi2s(ilayer)) {
	  if (cl->GetQ()==0) deadzoneSPD=kTRUE; // only 1 prolongation with virtual cluster	  
	  if (ntracks[ilayer]>=100) continue;
	  AliITStrackMI * updatetrack = new (&tracks[ilayer][ntracks[ilayer]]) AliITStrackMI(*currenttrack);
	  updatetrack->SetClIndex(ilayer,0);
	  if (changedet) new (&currenttrack2) AliITStrackMI(backuptrack);

	  if (cl->GetQ()!=0) { // real cluster
	    if (!UpdateMI(updatetrack,cl,chi2trkcl,(ilayer<<28)+clidx)) {
	      AliDebug(2,"update failed");
	      continue;
	    } 
	    updatetrack->SetSampledEdx(cl->GetQ(),updatetrack->GetNumberOfClusters()-1); //b.b.
	    modstatus = 1; // found
	  } else {             // virtual cluster in dead zone
	    updatetrack->SetNDeadZone(updatetrack->GetNDeadZone()+1);
	    updatetrack->SetDeadZoneProbability(GetSPDDeadZoneProbability(updatetrack->GetZ(),TMath::Sqrt(updatetrack->GetSigmaZ2())));
	    modstatus = 7; // holes in z in SPD
	  }

	  if (changedet) {
	    Float_t xlocnewdet,zlocnewdet;
	    if(LocalModuleCoord(ilayer,idet,updatetrack,xlocnewdet,zlocnewdet)) { // local module coords
	      updatetrack->SetModuleIndexInfo(ilayer,idet,modstatus,xlocnewdet,zlocnewdet);
	    }
	  } else {
	    updatetrack->SetModuleIndexInfo(ilayer,idet,modstatus,xloc,zloc);
	  }
	  if (cl->IsUsed()) updatetrack->IncrementNUsed();

	  // apply correction for material of the current layer
	  CorrectForLayerMaterial(updatetrack,ilayer,trackGlobXYZ1,"inward");

	  if (constrain) { // apply vertex constrain
	    updatetrack->SetConstrain(constrain);
	    Bool_t isPrim = kTRUE;
	    if (ilayer<4) { // check that it's close to the vertex
              updatetrack->GetDZ(GetX(),GetY(),GetZ(),updatetrack->GetDP()); //I.B.
	      if (TMath::Abs(updatetrack->GetD(0)/(1.+ilayer)) > // y
		  AliITSReconstructor::GetRecoParam()->GetMaxDZforPrimTrk() || 
		  TMath::Abs(updatetrack->GetD(1)/(1.+ilayer)) > // z
		  AliITSReconstructor::GetRecoParam()->GetMaxDZforPrimTrk()) isPrim=kFALSE;
	    }
	    if (isPrim) updatetrack->Improve(budgetToPrimVertex,xyzVtx,ersVtx);
	  } //apply vertex constrain	  	  
	  ntracks[ilayer]++;
	}  // create new hypothesis
	else {
	  AliDebug(2,"chi2 too large");
	}

      } // loop over possible prolongations 
     
      // allow one prolongation without clusters
      if (constrain && itrack<=1 && currenttrack1.GetNSkipped()==0 && deadzoneSPD==kFALSE && ntracks[ilayer]<100) {
	AliITStrackMI* vtrack = new (&tracks[ilayer][ntracks[ilayer]]) AliITStrackMI(currenttrack1);
	// apply correction for material of the current layer
	CorrectForLayerMaterial(vtrack,ilayer,trackGlobXYZ1,"inward");
	vtrack->SetClIndex(ilayer,0);
	modstatus = 3; // skipped 
	vtrack->SetModuleIndexInfo(ilayer,idet,modstatus,xloc,zloc);
	vtrack->Improve(budgetToPrimVertex,xyzVtx,ersVtx);
	vtrack->IncrementNSkipped();
	ntracks[ilayer]++;
      }

      // allow one prolongation without clusters for tracks with |tgl|>1.1
      if (constrain && itrack==0 && TMath::Abs(currenttrack1.GetTgl())>1.1) {  //big theta - for low flux
	AliITStrackMI* vtrack = new (&tracks[ilayer][ntracks[ilayer]]) AliITStrackMI(currenttrack1);
	// apply correction for material of the current layer
	CorrectForLayerMaterial(vtrack,ilayer,trackGlobXYZ1,"inward");
	vtrack->SetClIndex(ilayer,0);
	modstatus = 3; // skipped
	vtrack->SetModuleIndexInfo(ilayer,idet,modstatus,xloc,zloc);
	vtrack->Improve(budgetToPrimVertex,xyzVtx,ersVtx);
	vtrack->SetNDeadZone(vtrack->GetNDeadZone()+1);
	ntracks[ilayer]++;
      }
     
      
    } // loop over tracks in layer ilayer+1

    //loop over track candidates for the current layer
    //
    //
    Int_t accepted=0;
    
    Int_t golden=0;
    for (Int_t itrack=0;itrack<ntracks[ilayer];itrack++){
      normalizedchi2[itrack] = NormalizedChi2(&tracks[ilayer][itrack],ilayer); 
      if (normalizedchi2[itrack] < 
	  AliITSReconstructor::GetRecoParam()->GetMaxNormChi2ForGolden(ilayer)) golden++;
      if (ilayer>4) {
	accepted++;
      } else {
	if (constrain) { // constrain
	  if (normalizedchi2[itrack]<AliITSReconstructor::GetRecoParam()->GetMaxNormChi2C(ilayer)+1) 
	    accepted++;
	} else {        // !constrain
	  if (normalizedchi2[itrack]<AliITSReconstructor::GetRecoParam()->GetMaxNormChi2NonC(ilayer)+1) 
	    accepted++;
	}
      }
    }
    // sort tracks by increasing normalized chi2
    TMath::Sort(ntracks[ilayer],normalizedchi2,nindexes[ilayer],kFALSE); 
    ntracks[ilayer] = TMath::Min(accepted,7+2*ilayer);
    if (ntracks[ilayer]<golden+2+ilayer) ntracks[ilayer]=TMath::Min(golden+2+ilayer,accepted);
    if (ntracks[ilayer]>90) ntracks[ilayer]=90; 
  } // end loop over layers


  //
  // Now select tracks to be kept
  //
  Int_t max = constrain ? 20 : 5;

  // tracks that reach layer 0 (SPD inner)
  for (Int_t i=0; i<TMath::Min(max,ntracks[0]); i++) {
    AliITStrackMI & track= tracks[0][nindexes[0][i]];
    if (track.GetNumberOfClusters()<2) continue;
    if (!constrain && track.GetNormChi2(0) >
	AliITSReconstructor::GetRecoParam()->GetMaxNormChi2NonCForHypothesis()) {
      continue;
    }
    AddTrackHypothesys(new AliITStrackMI(track), esdindex);
  }

  // tracks that reach layer 1 (SPD outer)
  for (Int_t i=0;i<TMath::Min(2,ntracks[1]);i++) {
    AliITStrackMI & track= tracks[1][nindexes[1][i]];
    if (track.GetNumberOfClusters()<4) continue;
    if (!constrain && track.GetNormChi2(1) >
	AliITSReconstructor::GetRecoParam()->GetMaxNormChi2NonCForHypothesis()) continue;
    if (constrain) track.IncrementNSkipped();
    if (!constrain) {
      track.SetD(0,track.GetD(GetX(),GetY()));   
      track.SetNSkipped(track.GetNSkipped()+4./(4.+8.*TMath::Abs(track.GetD(0))));
      if (track.GetNumberOfClusters()+track.GetNDeadZone()+track.GetNSkipped()>6) {
	track.SetNSkipped(6-track.GetNumberOfClusters()+track.GetNDeadZone());
      }
    }
    AddTrackHypothesys(new AliITStrackMI(track), esdindex);
  }

  // tracks that reach layer 2 (SDD inner), only during non-constrained pass
  if (!constrain){  
    for (Int_t i=0;i<TMath::Min(2,ntracks[2]);i++) {
      AliITStrackMI & track= tracks[2][nindexes[2][i]];
      if (track.GetNumberOfClusters()<3) continue;
      if (!constrain && track.GetNormChi2(2) >
	  AliITSReconstructor::GetRecoParam()->GetMaxNormChi2NonCForHypothesis()) continue;
      if (constrain) track.SetNSkipped(track.GetNSkipped()+2);      
      if (!constrain){
	track.SetD(0,track.GetD(GetX(),GetY()));
	track.SetNSkipped(track.GetNSkipped()+7./(7.+8.*TMath::Abs(track.GetD(0))));
	if (track.GetNumberOfClusters()+track.GetNDeadZone()+track.GetNSkipped()>6) {
	  track.SetNSkipped(6-track.GetNumberOfClusters()+track.GetNDeadZone());
	}
      }
      AddTrackHypothesys(new AliITStrackMI(track), esdindex);
    }
  }
  
  if (!constrain) {
    //
    // register best track of each layer - important for V0 finder
    //
    for (Int_t ilayer=0;ilayer<5;ilayer++){
      if (ntracks[ilayer]==0) continue;
      AliITStrackMI & track= tracks[ilayer][nindexes[ilayer][0]];
      if (track.GetNumberOfClusters()<1) continue;
      CookLabel(&track,0);
      bestarray->AddAt(new AliITStrackMI(track),ilayer);
    }
  }
  //
  // update TPC V0 information
  //
  if (otrack->GetESDtrack()->GetV0Index(0)>0){    
    Float_t fprimvertex[3]={GetX(),GetY(),GetZ()};
    for (Int_t i=0;i<3;i++){
      Int_t  index = otrack->GetESDtrack()->GetV0Index(i); 
      if (index==0) break;
      AliV0 *vertex = (AliV0*)fEsd->GetV0(index);
      if (vertex->GetStatus()<0) continue;     // rejected V0
      //
      if (otrack->GetSign()>0) {
	vertex->SetIndex(0,esdindex);
      }
      else{
	vertex->SetIndex(1,esdindex);
      }
      //find nearest layer with track info
      Double_t xrp[3]; vertex->GetXYZ(xrp[0],xrp[1],xrp[2]);  //I.B.
      Int_t nearestold  = GetNearestLayer(xrp);               //I.B.
      Int_t nearest     = nearestold; 
      for (Int_t ilayer =nearest;ilayer<8;ilayer++){
	if (ntracks[nearest]==0){
	  nearest = ilayer;
	}
      }
      //
      AliITStrackMI & track= tracks[nearest][nindexes[nearest][0]];
      if (nearestold<5&&nearest<5){
	Bool_t accept = track.GetNormChi2(nearest)<10; 
	if (accept){
	  if (track.GetSign()>0) {
	    vertex->SetParamP(track);
	    vertex->Update(fprimvertex);
	    //vertex->SetIndex(0,track.fESDtrack->GetID()); 
	    if (track.GetNumberOfClusters()>2) AddTrackHypothesys(new AliITStrackMI(track), esdindex);
	  }else{
	    vertex->SetParamN(track);
	    vertex->Update(fprimvertex);
	    //vertex->SetIndex(1,track.fESDtrack->GetID());
	    if (track.GetNumberOfClusters()>2) AddTrackHypothesys(new AliITStrackMI(track), esdindex);
	  }
	  vertex->SetStatus(vertex->GetStatus()+1);
	}else{
	  //vertex->SetStatus(-2);  // reject V0  - not enough clusters
	}
      }
    }
  } 
  
}
//------------------------------------------------------------------------
AliITStrackerMI::AliITSlayer & AliITStrackerMI::GetLayer(Int_t layer) const
{
  //--------------------------------------------------------------------
  //
  //
  return fgLayers[layer];
}
//------------------------------------------------------------------------
AliITStrackerMI::AliITSlayer::AliITSlayer():
fR(0),
fPhiOffset(0),
fNladders(0),
fZOffset(0),
fNdetectors(0),
fDetectors(0),
fN(0),
fDy5(0),
fDy10(0),
fDy20(0),
fClustersCs(0),
fClusterIndexCs(0),
fYcs(0),
fZcs(0),
fNcs(0),
fCurrentSlice(-1),
fZmax(0),
fYmin(0),
fYmax(0),
fI(0),
fImax(0),
fSkip(0),
fAccepted(0),
fRoad(0){
  //--------------------------------------------------------------------
  //default AliITSlayer constructor
  //--------------------------------------------------------------------
  for (Int_t i=0; i<AliITSRecoParam::GetMaxClusterPerLayer(); i++) {
    fClusterWeight[i]=0;
    fClusterTracks[0][i]=-1;
    fClusterTracks[1][i]=-1;
    fClusterTracks[2][i]=-1;    
    fClusterTracks[3][i]=-1;    
  }
}
//------------------------------------------------------------------------
AliITStrackerMI::AliITSlayer::
AliITSlayer(Double_t r,Double_t p,Double_t z,Int_t nl,Int_t nd):
fR(r),
fPhiOffset(p),
fNladders(nl),
fZOffset(z),
fNdetectors(nd),
fDetectors(0),
fN(0),
fDy5(0),
fDy10(0),
fDy20(0),
fClustersCs(0),
fClusterIndexCs(0),
fYcs(0),
fZcs(0),
fNcs(0),
fCurrentSlice(-1),
fZmax(0),
fYmin(0),
fYmax(0),
fI(0),
fImax(0),
fSkip(0),
fAccepted(0),
fRoad(0) {
  //--------------------------------------------------------------------
  //main AliITSlayer constructor
  //--------------------------------------------------------------------
  fDetectors=new AliITSdetector[fNladders*fNdetectors];
  fRoad=2*fR*TMath::Sqrt(TMath::Pi()/1.);//assuming that there's only one cluster
}
//------------------------------------------------------------------------
AliITStrackerMI::AliITSlayer::AliITSlayer(const AliITSlayer& layer):
fR(layer.fR),
fPhiOffset(layer.fPhiOffset),
fNladders(layer.fNladders),
fZOffset(layer.fZOffset),
fNdetectors(layer.fNdetectors),
fDetectors(layer.fDetectors),
fN(layer.fN),
fDy5(layer.fDy5),
fDy10(layer.fDy10),
fDy20(layer.fDy20),
fClustersCs(layer.fClustersCs),
fClusterIndexCs(layer.fClusterIndexCs),
fYcs(layer.fYcs),
fZcs(layer.fZcs),
fNcs(layer.fNcs),
fCurrentSlice(layer.fCurrentSlice),
fZmax(layer.fZmax),
fYmin(layer.fYmin),
fYmax(layer.fYmax),
fI(layer.fI),
fImax(layer.fImax),
fSkip(layer.fSkip),
fAccepted(layer.fAccepted),
fRoad(layer.fRoad){
  //Copy constructor
}
//------------------------------------------------------------------------
AliITStrackerMI::AliITSlayer::~AliITSlayer() {
  //--------------------------------------------------------------------
  // AliITSlayer destructor
  //--------------------------------------------------------------------
  delete [] fDetectors;
  for (Int_t i=0; i<fN; i++) delete fClusters[i];
  for (Int_t i=0; i<AliITSRecoParam::GetMaxClusterPerLayer(); i++) {
    fClusterWeight[i]=0;
    fClusterTracks[0][i]=-1;
    fClusterTracks[1][i]=-1;
    fClusterTracks[2][i]=-1;    
    fClusterTracks[3][i]=-1;    
  }
}
//------------------------------------------------------------------------
void AliITStrackerMI::AliITSlayer::ResetClusters() {
  //--------------------------------------------------------------------
  // This function removes loaded clusters
  //--------------------------------------------------------------------
  for (Int_t i=0; i<fN; i++) delete fClusters[i];
  for (Int_t i=0; i<AliITSRecoParam::GetMaxClusterPerLayer(); i++){
    fClusterWeight[i]=0;
    fClusterTracks[0][i]=-1;
    fClusterTracks[1][i]=-1;
    fClusterTracks[2][i]=-1;    
    fClusterTracks[3][i]=-1;  
  }
  
  fN=0;
  fI=0;
}
//------------------------------------------------------------------------
void AliITStrackerMI::AliITSlayer::ResetWeights() {
  //--------------------------------------------------------------------
  // This function reset weights of the clusters
  //--------------------------------------------------------------------
  for (Int_t i=0; i<AliITSRecoParam::GetMaxClusterPerLayer(); i++) {
    fClusterWeight[i]=0;
    fClusterTracks[0][i]=-1;
    fClusterTracks[1][i]=-1;
    fClusterTracks[2][i]=-1;    
    fClusterTracks[3][i]=-1;  
  }
  for (Int_t i=0; i<fN;i++) {
    AliITSRecPoint * cl = (AliITSRecPoint*)GetCluster(i);
    if (cl&&cl->IsUsed()) cl->Use();
  }

}
//------------------------------------------------------------------------
void AliITStrackerMI::AliITSlayer::ResetRoad() {
  //--------------------------------------------------------------------
  // This function calculates the road defined by the cluster density
  //--------------------------------------------------------------------
  Int_t n=0;
  for (Int_t i=0; i<fN; i++) {
     if (TMath::Abs(fClusters[i]->GetZ())<fR) n++;
  }
  if (n>1) fRoad=2*fR*TMath::Sqrt(TMath::Pi()/n);
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::AliITSlayer::InsertCluster(AliITSRecPoint *cl) {
  //--------------------------------------------------------------------
  //This function adds a cluster to this layer
  //--------------------------------------------------------------------
  if (fN==AliITSRecoParam::GetMaxClusterPerLayer()) {
    ::Error("InsertCluster","Too many clusters !\n");
    return 1;
  }
  fCurrentSlice=-1;
  fClusters[fN]=cl;
  fN++;
  AliITSdetector &det=GetDetector(cl->GetDetectorIndex());    
  if (cl->GetY()<det.GetYmin()) det.SetYmin(cl->GetY());
  if (cl->GetY()>det.GetYmax()) det.SetYmax(cl->GetY());
  if (cl->GetZ()<det.GetZmin()) det.SetZmin(cl->GetZ());
  if (cl->GetZ()>det.GetZmax()) det.SetZmax(cl->GetZ());
			     
  return 0;
}
//------------------------------------------------------------------------
void  AliITStrackerMI::AliITSlayer::SortClusters()
{
  //
  //sort clusters
  //
  AliITSRecPoint **clusters = new AliITSRecPoint*[fN];
  Float_t *z                = new Float_t[fN];
  Int_t   * index           = new Int_t[fN];
  //
  for (Int_t i=0;i<fN;i++){
    z[i] = fClusters[i]->GetZ();
  }
  TMath::Sort(fN,z,index,kFALSE);
  for (Int_t i=0;i<fN;i++){
    clusters[i] = fClusters[index[i]];
  }
  //
  for (Int_t i=0;i<fN;i++){
    fClusters[i] = clusters[i];
    fZ[i]        = fClusters[i]->GetZ();
    AliITSdetector &det=GetDetector(fClusters[i]->GetDetectorIndex());    
    Double_t y=fR*det.GetPhi() + fClusters[i]->GetY();
    if (y>2.*fR*TMath::Pi()) y -= 2.*fR*TMath::Pi();
    fY[i] = y;
  }
  delete[] index;
  delete[] z;
  delete[] clusters;
  //

  fYB[0]=10000000;
  fYB[1]=-10000000;
  for (Int_t i=0;i<fN;i++){
    if (fY[i]<fYB[0]) fYB[0]=fY[i];
    if (fY[i]>fYB[1]) fYB[1]=fY[i];
    fClusterIndex[i] = i;
  }
  //
  // fill slices
  fDy5 = (fYB[1]-fYB[0])/5.;
  fDy10 = (fYB[1]-fYB[0])/10.;
  fDy20 = (fYB[1]-fYB[0])/20.;
  for (Int_t i=0;i<6;i++)  fN5[i] =0;  
  for (Int_t i=0;i<11;i++) fN10[i]=0;  
  for (Int_t i=0;i<21;i++) fN20[i]=0;
  //  
  for (Int_t i=0;i<6;i++) {fBy5[i][0] =  fYB[0]+(i-0.75)*fDy5; fBy5[i][1] =  fYB[0]+(i+0.75)*fDy5;}
  for (Int_t i=0;i<11;i++) {fBy10[i][0] =  fYB[0]+(i-0.75)*fDy10; fBy10[i][1] =  fYB[0]+(i+0.75)*fDy10;} 
  for (Int_t i=0;i<21;i++) {fBy20[i][0] =  fYB[0]+(i-0.75)*fDy20; fBy20[i][1] =  fYB[0]+(i+0.75)*fDy20;}
  //
  //
  for (Int_t i=0;i<fN;i++)
    for (Int_t irot=-1;irot<=1;irot++){
      Float_t curY = fY[i]+irot*TMath::TwoPi()*fR; 
      // slice 5
      for (Int_t slice=0; slice<6;slice++){
	if (fBy5[slice][0]<curY && curY<fBy5[slice][1]&&fN5[slice]<AliITSRecoParam::GetMaxClusterPerLayer5()){
	  fClusters5[slice][fN5[slice]] = fClusters[i];
	  fY5[slice][fN5[slice]] = curY;
	  fZ5[slice][fN5[slice]] = fZ[i];
	  fClusterIndex5[slice][fN5[slice]]=i;
	  fN5[slice]++;
	}
      }
      // slice 10
      for (Int_t slice=0; slice<11;slice++){
	if (fBy10[slice][0]<curY && curY<fBy10[slice][1]&&fN10[slice]<AliITSRecoParam::GetMaxClusterPerLayer10()){
	  fClusters10[slice][fN10[slice]] = fClusters[i];
	  fY10[slice][fN10[slice]] = curY;
	  fZ10[slice][fN10[slice]] = fZ[i];
	  fClusterIndex10[slice][fN10[slice]]=i;
	  fN10[slice]++;
	}
      }
      // slice 20
      for (Int_t slice=0; slice<21;slice++){
	if (fBy20[slice][0]<curY && curY<fBy20[slice][1]&&fN20[slice]<AliITSRecoParam::GetMaxClusterPerLayer20()){
	  fClusters20[slice][fN20[slice]] = fClusters[i];
	  fY20[slice][fN20[slice]] = curY;
	  fZ20[slice][fN20[slice]] = fZ[i];
	  fClusterIndex20[slice][fN20[slice]]=i;
	  fN20[slice]++;
	}
      }      
    }

  //
  // consistency check
  //
  for (Int_t i=0;i<fN-1;i++){
    if (fZ[i]>fZ[i+1]){
      printf("Bug\n");
    }
  }
  //
  for (Int_t slice=0;slice<21;slice++)
  for (Int_t i=0;i<fN20[slice]-1;i++){
    if (fZ20[slice][i]>fZ20[slice][i+1]){
      printf("Bug\n");
    }
  }


}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::AliITSlayer::FindClusterIndex(Float_t z) const {
  //--------------------------------------------------------------------
  // This function returns the index of the nearest cluster 
  //--------------------------------------------------------------------
  Int_t ncl=0;
  const Float_t *zcl;  
  if (fCurrentSlice<0) {
    ncl = fN;
    zcl   = fZ;
  }
  else{
    ncl   = fNcs;
    zcl   = fZcs;;
  }
  
  if (ncl==0) return 0;
  Int_t b=0, e=ncl-1, m=(b+e)/2;
  for (; b<e; m=(b+e)/2) {
    //    if (z > fClusters[m]->GetZ()) b=m+1;
    if (z > zcl[m]) b=m+1;
    else e=m; 
  }
  return m;
}
//------------------------------------------------------------------------
Bool_t AliITStrackerMI::ComputeRoad(AliITStrackMI* track,Int_t ilayer,Int_t idet,Double_t &zmin,Double_t &zmax,Double_t &ymin,Double_t &ymax) const {
  //--------------------------------------------------------------------
  // This function computes the rectangular road for this track
  //--------------------------------------------------------------------


  AliITSdetector &det = fgLayers[ilayer].GetDetector(idet);
  // take into account the misalignment: propagate track to misaligned detector plane
  if (!track->Propagate(det.GetPhi(),det.GetRmisal())) return kFALSE;

  Double_t dz=AliITSReconstructor::GetRecoParam()->GetNSigmaRoadZ()*
                    TMath::Sqrt(track->GetSigmaZ2() + 
		    AliITSReconstructor::GetRecoParam()->GetNSigmaZLayerForRoadZ()*
		    AliITSReconstructor::GetRecoParam()->GetNSigmaZLayerForRoadZ()*
		    AliITSReconstructor::GetRecoParam()->GetSigmaZ2(ilayer));
  Double_t dy=AliITSReconstructor::GetRecoParam()->GetNSigmaRoadY()*
                    TMath::Sqrt(track->GetSigmaY2() + 
		    AliITSReconstructor::GetRecoParam()->GetNSigmaYLayerForRoadY()*
		    AliITSReconstructor::GetRecoParam()->GetNSigmaYLayerForRoadY()*
		    AliITSReconstructor::GetRecoParam()->GetSigmaY2(ilayer));
      
  // track at boundary between detectors, enlarge road
  Double_t boundaryWidth=AliITSRecoParam::GetBoundaryWidth();
  if ( (track->GetY()-dy < det.GetYmin()+boundaryWidth) || 
       (track->GetY()+dy > det.GetYmax()-boundaryWidth) || 
       (track->GetZ()-dz < det.GetZmin()+boundaryWidth) ||
       (track->GetZ()+dz > det.GetZmax()-boundaryWidth) ) {
    Float_t tgl = TMath::Abs(track->GetTgl());
    if (tgl > 1.) tgl=1.;
    Double_t deltaXNeighbDets=AliITSRecoParam::GetDeltaXNeighbDets();
    dz = TMath::Sqrt(dz*dz+deltaXNeighbDets*deltaXNeighbDets*tgl*tgl);
    Float_t snp = TMath::Abs(track->GetSnp());
    if (snp > AliITSReconstructor::GetRecoParam()->GetMaxSnp()) return kFALSE;
    dy = TMath::Sqrt(dy*dy+deltaXNeighbDets*deltaXNeighbDets*snp*snp);
  } // boundary
  
  // add to the road a term (up to 2-3 mm) to deal with misalignments
  dy = TMath::Sqrt(dy*dy + AliITSReconstructor::GetRecoParam()->GetRoadMisal()*AliITSReconstructor::GetRecoParam()->GetRoadMisal());
  dz = TMath::Sqrt(dz*dz + AliITSReconstructor::GetRecoParam()->GetRoadMisal()*AliITSReconstructor::GetRecoParam()->GetRoadMisal());

  Double_t r = fgLayers[ilayer].GetR();
  zmin = track->GetZ() - dz; 
  zmax = track->GetZ() + dz;
  ymin = track->GetY() + r*det.GetPhi() - dy;
  ymax = track->GetY() + r*det.GetPhi() + dy;

  // bring track back to idead detector plane
  if (!track->Propagate(det.GetPhi(),det.GetR())) return kFALSE;

  return kTRUE;
}
//------------------------------------------------------------------------
void AliITStrackerMI::AliITSlayer::
SelectClusters(Double_t zmin,Double_t zmax,Double_t ymin, Double_t ymax) {
  //--------------------------------------------------------------------
  // This function sets the "window"
  //--------------------------------------------------------------------
 
  Double_t circle=2*TMath::Pi()*fR;
  fYmin = ymin; fYmax =ymax;
  Float_t ymiddle = (fYmin+fYmax)*0.5;
  if (ymiddle<fYB[0]) {
    fYmin+=circle; fYmax+=circle; ymiddle+=circle;
  } else if (ymiddle>fYB[1]) {
    fYmin-=circle; fYmax-=circle; ymiddle-=circle;
  }
  
  //
  fCurrentSlice =-1;
  // defualt take all
  fClustersCs = fClusters;
  fClusterIndexCs = fClusterIndex;
  fYcs  = fY;
  fZcs  = fZ;
  fNcs  = fN;
  //
  //is in 20 slice?
  if (fCurrentSlice<0&&TMath::Abs(fYmax-fYmin)<1.49*fDy20){
    Int_t slice = int(0.5+(ymiddle-fYB[0])/fDy20);
    if (slice<0) slice=0;
    if (slice>20) slice=20;
    Bool_t isOK = (fYmin>fBy20[slice][0]&&fYmax<fBy20[slice][1]);
    if (isOK) {
      fCurrentSlice=slice;
      fClustersCs = fClusters20[fCurrentSlice];
      fClusterIndexCs = fClusterIndex20[fCurrentSlice];
      fYcs  = fY20[fCurrentSlice];
      fZcs  = fZ20[fCurrentSlice];
      fNcs  = fN20[fCurrentSlice];
    }
  }  
  //
  //is in 10 slice?
  if (fCurrentSlice<0&&TMath::Abs(fYmax-fYmin)<1.49*fDy10){
    Int_t slice = int(0.5+(ymiddle-fYB[0])/fDy10);
    if (slice<0) slice=0;
    if (slice>10) slice=10;
    Bool_t isOK = (fYmin>fBy10[slice][0]&&fYmax<fBy10[slice][1]);
    if (isOK) {
      fCurrentSlice=slice;
      fClustersCs = fClusters10[fCurrentSlice];
      fClusterIndexCs = fClusterIndex10[fCurrentSlice];
      fYcs  = fY10[fCurrentSlice];
      fZcs  = fZ10[fCurrentSlice];
      fNcs  = fN10[fCurrentSlice];
    }
  }  
  //
  //is in 5 slice?
  if (fCurrentSlice<0&&TMath::Abs(fYmax-fYmin)<1.49*fDy5){
    Int_t slice = int(0.5+(ymiddle-fYB[0])/fDy5);
    if (slice<0) slice=0;
    if (slice>5) slice=5;
    Bool_t isOK = (fYmin>fBy5[slice][0]&&fYmax<fBy5[slice][1]);
    if (isOK) {
      fCurrentSlice=slice;
      fClustersCs = fClusters5[fCurrentSlice];
      fClusterIndexCs = fClusterIndex5[fCurrentSlice];
      fYcs  = fY5[fCurrentSlice];
      fZcs  = fZ5[fCurrentSlice];
      fNcs  = fN5[fCurrentSlice];
    }
  }  
  //  
  fI=FindClusterIndex(zmin); fZmax=zmax;
  fImax = TMath::Min(FindClusterIndex(zmax)+1,fNcs);
  fSkip = 0;
  fAccepted =0;

  return;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::AliITSlayer::
FindDetectorIndex(Double_t phi, Double_t z) const {
  //--------------------------------------------------------------------
  //This function finds the detector crossed by the track
  //--------------------------------------------------------------------
  Double_t dphi;
  if (fZOffset<0)            // old geometry
    dphi = -(phi-fPhiOffset);
  else                       // new geometry
    dphi = phi-fPhiOffset;


  if      (dphi <  0) dphi += 2*TMath::Pi();
  else if (dphi >= 2*TMath::Pi()) dphi -= 2*TMath::Pi();
  Int_t np=Int_t(dphi*fNladders*0.5/TMath::Pi()+0.5);
  if (np>=fNladders) np-=fNladders;
  if (np<0)          np+=fNladders;


  Double_t dz=fZOffset-z;
  Double_t nnz = dz*(fNdetectors-1)*0.5/fZOffset+0.5;
  Int_t nz = (nnz<0 ? -1 : (Int_t)nnz);
  if (nz>=fNdetectors) return -1;
  if (nz<0)            return -1;

  // ad hoc correction for 3rd ladder of SDD inner layer,
  // which is reversed (rotated by pi around local y)
  // this correction is OK only from AliITSv11Hybrid onwards
  if (GetR()>12. && GetR()<20.) { // SDD inner
    if(np==2) { // 3rd ladder
      nz = (fNdetectors-1) - nz;
    } 
  }
  //printf("ndet %d phi %f z %f  np %d nz %d\n",fNdetectors,phi,z,np,nz);


  return np*fNdetectors + nz;
}
//------------------------------------------------------------------------
const AliITSRecPoint *AliITStrackerMI::AliITSlayer::GetNextCluster(Int_t &ci,Bool_t test)
{
  //--------------------------------------------------------------------
  // This function returns clusters within the "window" 
  //--------------------------------------------------------------------

  if (fCurrentSlice<0) {
    Double_t rpi2 = 2.*fR*TMath::Pi();
    for (Int_t i=fI; i<fImax; i++) {
      Double_t y = fY[i];
      if (fYmax<y) y -= rpi2;
      if (fYmin>y) y += rpi2;
      if (y<fYmin) continue;
      if (y>fYmax) continue;
      if (fClusters[i]->GetQ()==0&&fSkip==2) continue;
      ci=i;
      if (!test) fI=i+1;
      return fClusters[i];
    }
  } else {
    for (Int_t i=fI; i<fImax; i++) {
      if (fYcs[i]<fYmin) continue;
      if (fYcs[i]>fYmax) continue;
      if (fClustersCs[i]->GetQ()==0&&fSkip==2) continue;
      ci=fClusterIndexCs[i];
      if (!test) fI=i+1;
      return fClustersCs[i];
    }
  }
  return 0;
}
//------------------------------------------------------------------------
Double_t AliITStrackerMI::AliITSlayer::GetThickness(Double_t y,Double_t z,Double_t &x0)
const {
  //--------------------------------------------------------------------
  // This function returns the layer thickness at this point (units X0)
  //--------------------------------------------------------------------
  Double_t d=0.0085;
  x0=AliITSRecoParam::GetX0Air();
  if (43<fR&&fR<45) { //SSD2
     Double_t dd=0.0034;
     d=dd;
     if (TMath::Abs(y-0.00)>3.40) d+=dd;
     if (TMath::Abs(y-1.90)<0.45) {d+=(0.013-0.0034);}
     if (TMath::Abs(y+1.90)<0.45) {d+=(0.013-0.0034);}
     for (Int_t i=0; i<12; i++) {
       if (TMath::Abs(z-3.9*(i+0.5))<0.15) {
          if (TMath::Abs(y-0.00)>3.40) d+=dd;
          d+=0.0034; 
          break;
       }
       if (TMath::Abs(z+3.9*(i+0.5))<0.15) {
          if (TMath::Abs(y-0.00)>3.40) d+=dd;
          d+=0.0034; 
          break;
       }         
       if (TMath::Abs(z-3.4-3.9*i)<0.50) {d+=(0.016-0.0034); break;}
       if (TMath::Abs(z+0.5+3.9*i)<0.50) {d+=(0.016-0.0034); break;}
     }
  } else 
  if (37<fR&&fR<41) { //SSD1
     Double_t dd=0.0034;
     d=dd;
     if (TMath::Abs(y-0.00)>3.40) d+=dd;
     if (TMath::Abs(y-1.90)<0.45) {d+=(0.013-0.0034);}
     if (TMath::Abs(y+1.90)<0.45) {d+=(0.013-0.0034);}
     for (Int_t i=0; i<11; i++) {
       if (TMath::Abs(z-3.9*i)<0.15) {
          if (TMath::Abs(y-0.00)>3.40) d+=dd;
          d+=dd; 
          break;
       }
       if (TMath::Abs(z+3.9*i)<0.15) {
          if (TMath::Abs(y-0.00)>3.40) d+=dd;
          d+=dd; 
          break;
       }         
       if (TMath::Abs(z-1.85-3.9*i)<0.50) {d+=(0.016-0.0034); break;}
       if (TMath::Abs(z+2.05+3.9*i)<0.50) {d+=(0.016-0.0034); break;}         
     }
  } else
  if (13<fR&&fR<26) { //SDD
     Double_t dd=0.0033;
     d=dd;
     if (TMath::Abs(y-0.00)>3.30) d+=dd;

     if (TMath::Abs(y-1.80)<0.55) {
        d+=0.016;
        for (Int_t j=0; j<20; j++) {
          if (TMath::Abs(z+0.7+1.47*j)<0.12) {d+=0.08; x0=9.; break;}
          if (TMath::Abs(z-0.7-1.47*j)<0.12) {d+=0.08; x0=9.; break;}
        } 
     }
     if (TMath::Abs(y+1.80)<0.55) {
        d+=0.016;
        for (Int_t j=0; j<20; j++) {
          if (TMath::Abs(z-0.7-1.47*j)<0.12) {d+=0.08; x0=9.; break;}
          if (TMath::Abs(z+0.7+1.47*j)<0.12) {d+=0.08; x0=9.; break;}
        } 
     }

     for (Int_t i=0; i<4; i++) {
       if (TMath::Abs(z-7.3*i)<0.60) {
          d+=dd;
          if (TMath::Abs(y-0.00)>3.30) d+=dd; 
          break;
       }
       if (TMath::Abs(z+7.3*i)<0.60) {
          d+=dd; 
          if (TMath::Abs(y-0.00)>3.30) d+=dd; 
          break;
       }
     }
  } else
  if (6<fR&&fR<8) {   //SPD2
     Double_t dd=0.0063; x0=21.5;
     d=dd;
     if (TMath::Abs(y-3.08)>0.5) d+=dd;
     if (TMath::Abs(y-3.03)<0.10) d+=0.014;
  } else
  if (3<fR&&fR<5) {   //SPD1
     Double_t dd=0.0063; x0=21.5;
     d=dd;
     if (TMath::Abs(y+0.21)>0.6) d+=dd;
     if (TMath::Abs(y+0.10)<0.10) d+=0.014;
  }

  return d;
}
//------------------------------------------------------------------------
AliITStrackerMI::AliITSdetector::AliITSdetector(const AliITSdetector& det):
fR(det.fR),
fRmisal(det.fRmisal),
fPhi(det.fPhi),
fSinPhi(det.fSinPhi),
fCosPhi(det.fCosPhi),
fYmin(det.fYmin),
fYmax(det.fYmax),
fZmin(det.fZmin),
fZmax(det.fZmax),
fIsBad(det.fIsBad),
fNChips(det.fNChips),
fChipIsBad(det.fChipIsBad)
{
  //Copy constructor
}
//------------------------------------------------------------------------
void AliITStrackerMI::AliITSdetector::ReadBadDetectorAndChips(Int_t ilayer,Int_t idet,
						 AliITSDetTypeRec *detTypeRec)
{
  //--------------------------------------------------------------------
  // Read bad detectors and chips from calibration objects in AliITSDetTypeRec
  //--------------------------------------------------------------------

  // In AliITSDetTypeRec, detector numbers go from 0 to 2197
  // while in the tracker they start from 0 for each layer
  for(Int_t il=0; il<ilayer; il++) 
    idet += AliITSgeomTGeo::GetNLadders(il+1)*AliITSgeomTGeo::GetNDetectors(il+1);

  Int_t detType;
  if (ilayer==0 || ilayer==1) {        // ----------  SPD
    detType = 0;
  } else if (ilayer==2 || ilayer==3) { // ----------  SDD
    detType = 1;
  } else if (ilayer==4 || ilayer==5) { // ----------  SSD
    detType = 2;
  } else {
    printf("AliITStrackerMI::AliITSdetector::InitBadFromOCDB: Wrong layer number %d\n",ilayer);
    return;
  }

  // Get calibration from AliITSDetTypeRec
  AliITSCalibration *calib = (AliITSCalibration*)detTypeRec->GetCalibrationModel(idet);
  calib->SetModuleIndex(idet);
  AliITSCalibration *calibSPDdead = 0;
  if(detType==0) calibSPDdead = (AliITSCalibration*)detTypeRec->GetSPDDeadModel(idet); // TEMPORARY
  if (calib->IsBad() ||
      (detType==0 && calibSPDdead->IsBad())) // TEMPORARY
    {
      SetBad();
      //      printf("lay %d bad %d\n",ilayer,idet);
    }

  // Get segmentation from AliITSDetTypeRec
  AliITSsegmentation *segm = (AliITSsegmentation*)detTypeRec->GetSegmentationModel(detType);

  // Read info about bad chips
  fNChips = segm->GetMaximumChipIndex()+1;
  //printf("ilayer %d  detType %d idet %d fNChips %d %d  GetNumberOfChips %d\n",ilayer,detType,idet,fNChips,segm->GetMaximumChipIndex(),segm->GetNumberOfChips());
  if(fChipIsBad) { delete [] fChipIsBad; fChipIsBad=NULL; }
  fChipIsBad = new Bool_t[fNChips];
  for (Int_t iCh=0;iCh<fNChips;iCh++) {
    fChipIsBad[iCh] = calib->IsChipBad(iCh);
    if (detType==0 && calibSPDdead->IsChipBad(iCh)) fChipIsBad[iCh] = kTRUE; // TEMPORARY
    //if(fChipIsBad[iCh]) {printf("lay %d det %d bad chip %d\n",ilayer,idet,iCh);}
  }

  return;
}
//------------------------------------------------------------------------
Double_t AliITStrackerMI::GetEffectiveThickness()
{
  //--------------------------------------------------------------------
  // Returns the thickness between the current layer and the vertex (units X0)
  //--------------------------------------------------------------------

  if(fUseTGeo!=0) {
    if(fxOverX0Layer[0]<0) BuildMaterialLUT("Layers");
    if(fxOverX0Shield[0]<0) BuildMaterialLUT("Shields");
    if(fxOverX0Pipe<0) BuildMaterialLUT("Pipe");
  }

  // beam pipe
  Double_t dPipe = (fUseTGeo==0 ? AliITSRecoParam::GetdPipe() : fxOverX0Pipe);
  Double_t d=dPipe*AliITSRecoParam::GetrPipe()*AliITSRecoParam::GetrPipe();

  // layers
  Double_t x0=0;
  Double_t xn=fgLayers[fI].GetR();
  for (Int_t i=0; i<fI; i++) {
    Double_t xi=fgLayers[i].GetR();
    Double_t dLayer = (fUseTGeo==0 ? fgLayers[i].GetThickness(0,0,x0) : fxOverX0Layer[i]);
    d+=dLayer*xi*xi;
  }

  // shields
  if (fI>1) {
    Double_t dshieldSPD = (fUseTGeo==0 ? AliITSRecoParam::Getdshield(0) : fxOverX0Shield[0]);
    d+=dshieldSPD*AliITSRecoParam::GetrInsideShield(0)*AliITSRecoParam::GetrInsideShield(0);
  }
  if (fI>3) {
    Double_t dshieldSDD = (fUseTGeo==0 ? AliITSRecoParam::Getdshield(1) : fxOverX0Shield[1]);
    d+=dshieldSDD*AliITSRecoParam::GetrInsideShield(1)*AliITSRecoParam::GetrInsideShield(1);
  }
  return d/(xn*xn);
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::AliITSlayer::InRoad() const {
  //-------------------------------------------------------------------
  // This function returns number of clusters within the "window" 
  //--------------------------------------------------------------------
  Int_t ncl=0;
  for (Int_t i=fI; i<fN; i++) {
    const AliITSRecPoint *c=fClusters[i];
    if (c->GetZ() > fZmax) break;
    if (c->IsUsed()) continue;
    const AliITSdetector &det=GetDetector(c->GetDetectorIndex());    
    Double_t y=fR*det.GetPhi() + c->GetY();

    if (y>2.*fR*TMath::Pi()) y -= 2*fR*TMath::Pi();
    if (y>1.*fR*TMath::Pi() && fYmax<y) y -= 2*fR*TMath::Pi();

    if (y<fYmin) continue;
    if (y>fYmax) continue;
    ncl++;
  }
  return ncl;
}
//------------------------------------------------------------------------
Bool_t AliITStrackerMI::RefitAt(Double_t xx,AliITStrackMI *track,
				const AliITStrackMI *clusters,Bool_t extra, Bool_t planeeff) 
{
  //--------------------------------------------------------------------
  // This function refits the track "track" at the position "x" using
  // the clusters from "clusters"
  // If "extra"==kTRUE, 
  //    the clusters from overlapped modules get attached to "track" 
  // If "planeff"==kTRUE,
  //    special approach for plane efficiency evaluation is applyed
  //--------------------------------------------------------------------

  Int_t index[AliITSgeomTGeo::kNLayers];
  Int_t k;
  for (k=0; k<AliITSgeomTGeo::GetNLayers(); k++) index[k]=-1;
  Int_t nc=clusters->GetNumberOfClusters();
  for (k=0; k<nc; k++) { 
    Int_t idx=clusters->GetClusterIndex(k);
    Int_t ilayer=(idx&0xf0000000)>>28;
    index[ilayer]=idx; 
  }

  return RefitAt(xx,track,index,extra,planeeff); // call the method below
}
//------------------------------------------------------------------------
Bool_t AliITStrackerMI::RefitAt(Double_t xx,AliITStrackMI *track,
				const Int_t *clusters,Bool_t extra, Bool_t planeeff) 
{
  //--------------------------------------------------------------------
  // This function refits the track "track" at the position "x" using
  // the clusters from array
  // If "extra"==kTRUE, 
  //    the clusters from overlapped modules get attached to "track" 
  // If "planeff"==kTRUE,
  //    special approach for plane efficiency evaluation is applyed
  //--------------------------------------------------------------------
  Int_t index[AliITSgeomTGeo::kNLayers];
  Int_t k;
  for (k=0; k<AliITSgeomTGeo::GetNLayers(); k++) index[k]=-1;
  //
  for (k=0; k<AliITSgeomTGeo::GetNLayers(); k++) { 
    index[k]=clusters[k]; 
  }

  // special for cosmics: check which the innermost layer crossed
  // by the track
  Int_t innermostlayer=5;
  Double_t drphi = TMath::Abs(track->GetD(0.,0.));
  for(innermostlayer=0; innermostlayer<AliITSgeomTGeo::GetNLayers(); innermostlayer++) {
    if(drphi < fgLayers[innermostlayer].GetR()) break;
  }
  //printf(" drphi  %f  innermost %d\n",drphi,innermostlayer);

  Int_t modstatus=1; // found
  Float_t xloc,zloc;
  Int_t from, to, step;
  if (xx > track->GetX()) {
      from=innermostlayer; to=AliITSgeomTGeo::GetNLayers();
      step=+1;
  } else {
      from=AliITSgeomTGeo::GetNLayers()-1; to=innermostlayer-1;
      step=-1;
  }
  TString dir = (step>0 ? "outward" : "inward");

  for (Int_t ilayer = from; ilayer != to; ilayer += step) {
     AliITSlayer &layer=fgLayers[ilayer];
     Double_t r=layer.GetR();
     if (step<0 && xx>r) break;

     // material between SSD and SDD, SDD and SPD
     Double_t hI=ilayer-0.5*step; 
     if (TMath::Abs(hI-3.5)<0.01) // SDDouter
       if(!CorrectForShieldMaterial(track,"SDD",dir)) return kFALSE;
     if (TMath::Abs(hI-1.5)<0.01) // SPDouter
       if(!CorrectForShieldMaterial(track,"SPD",dir)) return kFALSE;

     // remember old position [SR, GSI 18.02.2003]
     Double_t oldX=0., oldY=0., oldZ=0.;
     if (track->IsStartedTimeIntegral() && step==1) {
        if (!track->GetGlobalXYZat(track->GetX(),oldX,oldY,oldZ)) return kFALSE;
     }
     //

     Double_t oldGlobXYZ[3];
     if (!track->GetXYZ(oldGlobXYZ)) return kFALSE;

     Double_t phi,z;
     if (!track->GetPhiZat(r,phi,z)) return kFALSE;

     Int_t idet=layer.FindDetectorIndex(phi,z);

     // check if we allow a prolongation without point for large-eta tracks
     Int_t skip = CheckSkipLayer(track,ilayer,idet);
     if (skip==2) {
       // propagate to the layer radius
       Double_t xToGo; if (!track->GetLocalXat(r,xToGo)) return kFALSE;
       if (!track->AliExternalTrackParam::PropagateTo(xToGo,GetBz())) return kFALSE;
       // apply correction for material of the current layer
       CorrectForLayerMaterial(track,ilayer,oldGlobXYZ,dir);
       modstatus = 4; // out in z
       if(LocalModuleCoord(ilayer,idet,track,xloc,zloc)) { // local module coords
	 track->SetModuleIndexInfo(ilayer,idet,modstatus,xloc,zloc);
       }
       // track time update [SR, GSI 17.02.2003]
       if (track->IsStartedTimeIntegral() && step==1) {
	 Double_t newX, newY, newZ;
	 if (!track->GetGlobalXYZat(track->GetX(),newX,newY,newZ)) return kFALSE;
	 Double_t dL2 = (oldX-newX)*(oldX-newX) + (oldY-newY)*(oldY-newY) + 
	                (oldZ-newZ)*(oldZ-newZ);
	 track->AddTimeStep(TMath::Sqrt(dL2));
       }
       continue;
     }

     if (idet<0) return kFALSE;

     const AliITSdetector &det=layer.GetDetector(idet);
     if (!track->Propagate(det.GetPhi(),det.GetR())) return kFALSE;

     track->SetDetectorIndex(idet);
     if(!LocalModuleCoord(ilayer,idet,track,xloc,zloc)) return kFALSE; // local module coords

     Double_t dz,zmin,zmax,dy,ymin,ymax;

     const AliITSRecPoint *clAcc=0;
     Double_t maxchi2=1000.*AliITSReconstructor::GetRecoParam()->GetMaxChi2();

     Int_t idx=index[ilayer];
     if (idx>=0) { // cluster in this layer
       modstatus = 6; // no refit
       const AliITSRecPoint *cl=(AliITSRecPoint *)GetCluster(idx); 
       if (cl) {
	 if (idet != cl->GetDetectorIndex()) {
	   idet=cl->GetDetectorIndex();
	   const AliITSdetector &detc=layer.GetDetector(idet);
	   if (!track->Propagate(detc.GetPhi(),detc.GetR())) return kFALSE;
	   track->SetDetectorIndex(idet);
	   if(!LocalModuleCoord(ilayer,idet,track,xloc,zloc)) return kFALSE; // local module coords
	 }
	 Int_t cllayer = (idx & 0xf0000000) >> 28;;
	 Double_t chi2=GetPredictedChi2MI(track,cl,cllayer);
	 if (chi2<maxchi2) { 
	   clAcc=cl; 
	   maxchi2=chi2; 
	   modstatus = 1; // found
	 } else {
	    return kFALSE; //
	 }
       }
     } else { // no cluster in this layer
       if (skip==1) {
	 modstatus = 3; // skipped
         // Plane Eff determination:
         if (planeeff && ilayer==AliITSReconstructor::GetRecoParam()->GetIPlanePlaneEff()) {
           if (IsOKForPlaneEff(track,clusters,ilayer))  // only adequate track for plane eff. evaluation
              UseTrackForPlaneEff(track,ilayer);
         }
       } else {
	 modstatus = 5; // no cls in road
	 // check dead
	 if (!ComputeRoad(track,ilayer,idet,zmin,zmax,ymin,ymax)) return kFALSE;
	 dz = 0.5*(zmax-zmin);
	 dy = 0.5*(ymax-ymin);
	 Int_t dead = CheckDeadZone(track,ilayer,idet,dz,dy,kTRUE);
	 if (dead==1) modstatus = 7; // holes in z in SPD
	 if (dead==2 || dead==3) modstatus = 2; // dead from OCDB
       }
     }
     
     if (clAcc) {
       if (!UpdateMI(track,clAcc,maxchi2,idx)) return kFALSE;
       track->SetSampledEdx(clAcc->GetQ(),track->GetNumberOfClusters()-1);
     }
     track->SetModuleIndexInfo(ilayer,idet,modstatus,xloc,zloc);


     if (extra) { // search for extra clusters in overlapped modules
       AliITStrackV2 tmp(*track);
       if (!ComputeRoad(track,ilayer,idet,zmin,zmax,ymin,ymax)) return kFALSE;
       layer.SelectClusters(zmin,zmax,ymin,ymax);
       
       const AliITSRecPoint *clExtra=0; Int_t ci=-1,cci=-1;
       Int_t idetExtra=-1;  
       maxchi2=1000.*AliITSReconstructor::GetRecoParam()->GetMaxChi2();
       Double_t tolerance=0.1;
       while ((clExtra=layer.GetNextCluster(ci))!=0) {
	 // only clusters in another module! (overlaps)
	 idetExtra = clExtra->GetDetectorIndex();
	 if (idet == idetExtra) continue;
	 
	 const AliITSdetector &detx=layer.GetDetector(idetExtra);
	 
	 if (!tmp.Propagate(detx.GetPhi(),detx.GetR()+clExtra->GetX())) continue;
	 if (TMath::Abs(tmp.GetZ() - clExtra->GetZ()) > tolerance) continue;
	 if (TMath::Abs(tmp.GetY() - clExtra->GetY()) > tolerance) continue;
	 if (!tmp.Propagate(detx.GetPhi(),detx.GetR())) continue;
	 
	 Double_t chi2=tmp.GetPredictedChi2(clExtra);
	 if (chi2<maxchi2) { maxchi2=chi2; cci=ci; }
       }
       if (cci>=0) {
	 track->SetExtraCluster(ilayer,(ilayer<<28)+cci);
	 track->SetExtraModule(ilayer,idetExtra);
       }
     } // end search for extra clusters in overlapped modules
     
     // Correct for material of the current layer
     if(!CorrectForLayerMaterial(track,ilayer,oldGlobXYZ,dir)) return kFALSE;
                 
     // track time update [SR, GSI 17.02.2003]
     if (track->IsStartedTimeIntegral() && step==1) {
        Double_t newX, newY, newZ;
        if (!track->GetGlobalXYZat(track->GetX(),newX,newY,newZ)) return kFALSE;
        Double_t dL2 = (oldX-newX)*(oldX-newX) + (oldY-newY)*(oldY-newY) + 
                       (oldZ-newZ)*(oldZ-newZ);
        track->AddTimeStep(TMath::Sqrt(dL2));
     }
     //

  } // end loop on layers

  if (!track->PropagateTo(xx,0.,0.)) return kFALSE;

  return kTRUE;
}
//------------------------------------------------------------------------
Double_t AliITStrackerMI::GetNormalizedChi2(AliITStrackMI * track, Int_t mode)
{
  //
  // calculate normalized chi2
  //  return NormalizedChi2(track,0);
  Float_t chi2 = 0;
  Float_t sum=0;
  Float_t *erry = GetErrY(fCurrentEsdTrack), *errz = GetErrZ(fCurrentEsdTrack);
  //  track->fdEdxMismatch=0;
  Float_t dedxmismatch =0;
  Float_t *ny = GetNy(fCurrentEsdTrack), *nz = GetNz(fCurrentEsdTrack); 
  if (mode<100){
    for (Int_t i = 0;i<6;i++){
      if (track->GetClIndex(i)>0){
	Float_t cerry, cerrz;
	if (ny[i]>0) {cerry = erry[i]; cerrz=errz[i];}
	else 
	  { cerry= track->GetSigmaY(i); cerrz = track->GetSigmaZ(i);}
	cerry*=cerry;
	cerrz*=cerrz;	
	Float_t cchi2 = (track->GetDy(i)*track->GetDy(i)/cerry)+(track->GetDz(i)*track->GetDz(i)/cerrz);
	if (i>1 && AliITSReconstructor::GetRecoParam()->GetUseAmplitudeInfo(i)) {
	  Float_t ratio = track->GetNormQ(i)/track->GetExpQ();
	  if (ratio<0.5) {
	    cchi2+=(0.5-ratio)*10.;
	    //track->fdEdxMismatch+=(0.5-ratio)*10.;
	    dedxmismatch+=(0.5-ratio)*10.;	    
	  }
	}
	if (i<2 ||i>3){
	  AliITSRecPoint * cl = (AliITSRecPoint*)GetCluster( track->GetClIndex(i));  
	  Double_t delta = cl->GetNy()+cl->GetNz()-ny[i]-nz[i];
	  if (delta>1) chi2 +=0.5*TMath::Min(delta/2,2.); 
	  if (i<2) chi2+=2*cl->GetDeltaProbability();
	}
	chi2+=cchi2;
	sum++;
      }
    }
    if (TMath::Abs(track->GetdEdxMismatch()-dedxmismatch)>0.0001){
      track->SetdEdxMismatch(dedxmismatch);
    }
  }
  else{
    for (Int_t i = 0;i<4;i++){
      if (track->GetClIndex(i)>0){
	Float_t cerry, cerrz;
	if (ny[i]>0) {cerry = erry[i]; cerrz=errz[i];}
	else { cerry= track->GetSigmaY(i); cerrz = track->GetSigmaZ(i);}
	cerry*=cerry;
	cerrz*=cerrz;
	chi2+= (track->GetDy(i)*track->GetDy(i)/cerry);
	chi2+= (track->GetDz(i)*track->GetDz(i)/cerrz);      
	sum++;
      }
    }
    for (Int_t i = 4;i<6;i++){
      if (track->GetClIndex(i)>0){	
	Float_t cerry, cerrz;
	if (ny[i]>0) {cerry = erry[i]; cerrz=errz[i];}
	else { cerry= track->GetSigmaY(i); cerrz = track->GetSigmaZ(i);}
	cerry*=cerry;
	cerrz*=cerrz;	
	Float_t cerryb, cerrzb;
	if (ny[i+6]>0) {cerryb = erry[i+6]; cerrzb=errz[i+6];}
	else { cerryb= track->GetSigmaY(i+6); cerrzb = track->GetSigmaZ(i+6);}
	cerryb*=cerryb;
	cerrzb*=cerrzb;
	chi2+= TMath::Min((track->GetDy(i+6)*track->GetDy(i+6)/cerryb),track->GetDy(i)*track->GetDy(i)/cerry);
	chi2+= TMath::Min((track->GetDz(i+6)*track->GetDz(i+6)/cerrzb),track->GetDz(i)*track->GetDz(i)/cerrz);      
	sum++;
      }
    }
  }
  if (track->GetESDtrack()->GetTPCsignal()>85){
    Float_t ratio = track->GetdEdx()/track->GetESDtrack()->GetTPCsignal();
    if (ratio<0.5) {
      chi2+=(0.5-ratio)*5.;      
    }
    if (ratio>2){
      chi2+=(ratio-2.0)*3; 
    }
  }
  //
  Double_t match = TMath::Sqrt(track->GetChi22());
  if (track->GetConstrain())  match/=track->GetNumberOfClusters();
  if (!track->GetConstrain()) { 
    if (track->GetNumberOfClusters()>2) {
      match/=track->GetNumberOfClusters()-2.;
    } else {
      match=0;
    }
  }
  if (match<0) match=0;
  Float_t deadzonefactor = (track->GetNDeadZone()>0) ? 3*(1.1-track->GetDeadZoneProbability()):0.;
  Double_t normchi2 = 2*track->GetNSkipped()+match+deadzonefactor+(1+(2*track->GetNSkipped()+deadzonefactor)/track->GetNumberOfClusters())*
    (chi2)/TMath::Max(double(sum-track->GetNSkipped()),
				1./(1.+track->GetNSkipped()));     
 
 return normchi2;
}
//------------------------------------------------------------------------
Double_t AliITStrackerMI::GetMatchingChi2(AliITStrackMI * track1, AliITStrackMI * track2)
{
  //
  // return matching chi2 between two tracks
  Double_t largeChi2=1000.;

  AliITStrackMI track3(*track2);
  if (!track3.Propagate(track1->GetAlpha(),track1->GetX())) return largeChi2;
  TMatrixD vec(5,1);
  vec(0,0)=track1->GetY()   - track3.GetY();
  vec(1,0)=track1->GetZ()   - track3.GetZ();
  vec(2,0)=track1->GetSnp() - track3.GetSnp();
  vec(3,0)=track1->GetTgl() - track3.GetTgl();
  vec(4,0)=track1->GetSigned1Pt() - track3.GetSigned1Pt();
  //
  TMatrixD cov(5,5);
  cov(0,0) = track1->GetSigmaY2()+track3.GetSigmaY2();
  cov(1,1) = track1->GetSigmaZ2()+track3.GetSigmaZ2();
  cov(2,2) = track1->GetSigmaSnp2()+track3.GetSigmaSnp2();
  cov(3,3) = track1->GetSigmaTgl2()+track3.GetSigmaTgl2();
  cov(4,4) = track1->GetSigma1Pt2()+track3.GetSigma1Pt2();
  
  cov(0,1)=cov(1,0) = track1->GetSigmaZY()+track3.GetSigmaZY();
  cov(0,2)=cov(2,0) = track1->GetSigmaSnpY()+track3.GetSigmaSnpY();
  cov(0,3)=cov(3,0) = track1->GetSigmaTglY()+track3.GetSigmaTglY();
  cov(0,4)=cov(4,0) = track1->GetSigma1PtY()+track3.GetSigma1PtY();
  //
  cov(1,2)=cov(2,1) = track1->GetSigmaSnpZ()+track3.GetSigmaSnpZ();
  cov(1,3)=cov(3,1) = track1->GetSigmaTglZ()+track3.GetSigmaTglZ();
  cov(1,4)=cov(4,1) = track1->GetSigma1PtZ()+track3.GetSigma1PtZ();
  //
  cov(2,3)=cov(3,2) = track1->GetSigmaTglSnp()+track3.GetSigmaTglSnp();
  cov(2,4)=cov(4,2) = track1->GetSigma1PtSnp()+track3.GetSigma1PtSnp();
  //
  cov(3,4)=cov(4,3) = track1->GetSigma1PtTgl()+track3.GetSigma1PtTgl();
  
  cov.Invert();
  TMatrixD vec2(cov,TMatrixD::kMult,vec);
  TMatrixD chi2(vec2,TMatrixD::kTransposeMult,vec);
  return chi2(0,0);
}
//------------------------------------------------------------------------
Double_t  AliITStrackerMI::GetSPDDeadZoneProbability(Double_t zpos, Double_t zerr)
{
  //
  //  return probability that given point (characterized by z position and error) 
  //  is in SPD dead zone
  //
  Double_t probability = 0.;
  Double_t absz = TMath::Abs(zpos);
  Double_t nearestz = (absz<2.) ? 0.5*(fSPDdetzcentre[1]+fSPDdetzcentre[2]) : 
                                  0.5*(fSPDdetzcentre[2]+fSPDdetzcentre[3]);
  if (TMath::Abs(absz-nearestz)>0.25+3.*zerr) return probability;
  Double_t zmin, zmax;   
  if (zpos<-6.) { // dead zone at z = -7
    zmin = fSPDdetzcentre[0] + 0.5*AliITSRecoParam::GetSPDdetzlength();
    zmax = fSPDdetzcentre[1] - 0.5*AliITSRecoParam::GetSPDdetzlength();
  } else if (zpos>6.) { // dead zone at z = +7
    zmin = fSPDdetzcentre[2] + 0.5*AliITSRecoParam::GetSPDdetzlength();
    zmax = fSPDdetzcentre[3] - 0.5*AliITSRecoParam::GetSPDdetzlength();
  } else if (absz<2.) { // dead zone at z = 0
    zmin = fSPDdetzcentre[1] + 0.5*AliITSRecoParam::GetSPDdetzlength();
    zmax = fSPDdetzcentre[2] - 0.5*AliITSRecoParam::GetSPDdetzlength();
  } else {
    zmin = 0.;
    zmax = 0.;
  }
  // probability that the true z is in the range [zmin,zmax] (i.e. inside 
  // dead zone)
  probability = 0.5*( TMath::Erf((zpos-zmin)/zerr/TMath::Sqrt(2.)) - 
		      TMath::Erf((zpos-zmax)/zerr/TMath::Sqrt(2.)) );
  return probability;
}
//------------------------------------------------------------------------
Double_t AliITStrackerMI::GetTruncatedChi2(AliITStrackMI * track, Float_t fac)
{
  //
  // calculate normalized chi2
  Float_t chi2[6];
  Float_t *erry = GetErrY(fCurrentEsdTrack), *errz = GetErrZ(fCurrentEsdTrack);
  Float_t ncl = 0;
  for (Int_t i = 0;i<6;i++){
    if (TMath::Abs(track->GetDy(i))>0){      
      chi2[i]= (track->GetDy(i)/erry[i])*(track->GetDy(i)/erry[i]);
      chi2[i]+= (track->GetDz(i)/errz[i])*(track->GetDz(i)/errz[i]);
      ncl++;
    }
    else{chi2[i]=10000;}
  }
  Int_t index[6];
  TMath::Sort(6,chi2,index,kFALSE);
  Float_t max = float(ncl)*fac-1.;
  Float_t sumchi=0, sumweight=0; 
  for (Int_t i=0;i<max+1;i++){
    Float_t weight = (i<max)?1.:(max+1.-i);
    sumchi+=weight*chi2[index[i]];
    sumweight+=weight;
  }
  Double_t normchi2 = sumchi/sumweight;
  return normchi2;
}
//------------------------------------------------------------------------
Double_t AliITStrackerMI::GetInterpolatedChi2(AliITStrackMI * forwardtrack, AliITStrackMI * backtrack)
{
  //
  // calculate normalized chi2
  //  if (forwardtrack->fNUsed>0.3*float(forwardtrack->GetNumberOfClusters())) return 10000;
  Int_t npoints = 0;
  Double_t res =0;
  for (Int_t i=0;i<6;i++){
    if ( (backtrack->GetSigmaY(i)<0.000000001) || (forwardtrack->GetSigmaY(i)<0.000000001)) continue;
    Double_t sy1 = forwardtrack->GetSigmaY(i);
    Double_t sz1 = forwardtrack->GetSigmaZ(i);
    Double_t sy2 = backtrack->GetSigmaY(i);
    Double_t sz2 = backtrack->GetSigmaZ(i);
    if (i<2){ sy2=1000.;sz2=1000;}
    //    
    Double_t dy0 = (forwardtrack->GetDy(i)/(sy1*sy1) +backtrack->GetDy(i)/(sy2*sy2))/(1./(sy1*sy1)+1./(sy2*sy2));
    Double_t dz0 = (forwardtrack->GetDz(i)/(sz1*sz1) +backtrack->GetDz(i)/(sz2*sz2))/(1./(sz1*sz1)+1./(sz2*sz2));
    // 
    Double_t nz0 = dz0*TMath::Sqrt((1./(sz1*sz1)+1./(sz2*sz2)));
    Double_t ny0 = dy0*TMath::Sqrt((1./(sy1*sy1)+1./(sy2*sy2)));
    //
    res+= nz0*nz0+ny0*ny0;
    npoints++;
  }
  if (npoints>1) return 
                   TMath::Max(0.3*forwardtrack->OneOverPt()-0.5,0.)+
		   //2*forwardtrack->fNUsed+
		   res/TMath::Max(double(npoints-forwardtrack->GetNSkipped()),
				  1./(1.+forwardtrack->GetNSkipped()));
  return 1000;
}
//------------------------------------------------------------------------
Float_t  *AliITStrackerMI::GetWeight(Int_t index) {
  //--------------------------------------------------------------------
  //       Return pointer to a given cluster
  //--------------------------------------------------------------------
  Int_t l=(index & 0xf0000000) >> 28;
  Int_t c=(index & 0x0fffffff) >> 00;
  return fgLayers[l].GetWeight(c);
}
//------------------------------------------------------------------------
void AliITStrackerMI::RegisterClusterTracks(AliITStrackMI* track,Int_t id)
{
  //---------------------------------------------
  // register track to the list
  //
  if (track->GetESDtrack()->GetKinkIndex(0)!=0) return;  //don't register kink tracks
  //
  //
  for (Int_t icluster=0;icluster<track->GetNumberOfClusters();icluster++){
    Int_t index = track->GetClusterIndex(icluster);
    Int_t l=(index & 0xf0000000) >> 28;
    Int_t c=(index & 0x0fffffff) >> 00;
    if (c>fgLayers[l].GetNumberOfClusters()) continue;
    for (Int_t itrack=0;itrack<4;itrack++){
      if (fgLayers[l].GetClusterTracks(itrack,c)<0){
	fgLayers[l].SetClusterTracks(itrack,c,id);
	break;
      }
    }
  }
}
//------------------------------------------------------------------------
void AliITStrackerMI::UnRegisterClusterTracks(AliITStrackMI* track, Int_t id)
{
  //---------------------------------------------
  // unregister track from the list
  for (Int_t icluster=0;icluster<track->GetNumberOfClusters();icluster++){
    Int_t index = track->GetClusterIndex(icluster);
    Int_t l=(index & 0xf0000000) >> 28;
    Int_t c=(index & 0x0fffffff) >> 00;
    if (c>fgLayers[l].GetNumberOfClusters()) continue;
    for (Int_t itrack=0;itrack<4;itrack++){
      if (fgLayers[l].GetClusterTracks(itrack,c)==id){
	fgLayers[l].SetClusterTracks(itrack,c,-1);
      }
    }
  }
}
//------------------------------------------------------------------------
Float_t AliITStrackerMI::GetNumberOfSharedClusters(AliITStrackMI* track,Int_t id, Int_t list[6], AliITSRecPoint *clist[6])
{
  //-------------------------------------------------------------
  //get number of shared clusters
  //-------------------------------------------------------------
  Float_t shared=0;
  for (Int_t i=0;i<6;i++) { list[i]=-1, clist[i]=0;}
  // mean  number of clusters
  Float_t *ny = GetNy(id), *nz = GetNz(id); 

 
  for (Int_t icluster=0;icluster<track->GetNumberOfClusters();icluster++){
    Int_t index = track->GetClusterIndex(icluster);
    Int_t l=(index & 0xf0000000) >> 28;
    Int_t c=(index & 0x0fffffff) >> 00;
    if (c>fgLayers[l].GetNumberOfClusters()) continue;
    if (ny[l]==0){
      printf("problem\n");
    }
    AliITSRecPoint *cl = (AliITSRecPoint*)GetCluster(index);
    Float_t weight=1;
    //
    Float_t deltan = 0;
    if (l>3&&cl->GetNy()+cl->GetNz()>6) continue;
    if (l>2&&AliITSReconstructor::GetRecoParam()->GetUseAmplitudeInfo(l))
      if (track->GetNormQ(l)/track->GetExpQ()>3.5) continue;
    if (l<2 || l>3){      
      deltan = (cl->GetNy()+cl->GetNz()-ny[l]-nz[l]);
    }
    else{
      deltan = (cl->GetNz()-nz[l]);
    }
    if (deltan>2.0) continue;  // extended - highly probable shared cluster
    weight = 2./TMath::Max(3.+deltan,2.);
    //
    for (Int_t itrack=0;itrack<4;itrack++){
      if (fgLayers[l].GetClusterTracks(itrack,c)>=0 && fgLayers[l].GetClusterTracks(itrack,c)!=id){
	list[l]=index;
	clist[l] = (AliITSRecPoint*)GetCluster(index);
	shared+=weight; 
	break;
      }
    }
  }
  track->SetNUsed(shared);
  return shared;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::GetOverlapTrack(AliITStrackMI *track, Int_t trackID, Int_t &shared, Int_t clusterlist[6],Int_t overlist[6])
{
  //
  // find first shared track 
  //
  // mean  number of clusters
  Float_t *ny = GetNy(trackID), *nz = GetNz(trackID); 
  //
  for (Int_t i=0;i<6;i++) overlist[i]=-1;
  Int_t sharedtrack=100000;
  Int_t tracks[24],trackindex=0;
  for (Int_t i=0;i<24;i++) {tracks[i]=-1;}
  //
  for (Int_t icluster=0;icluster<6;icluster++){
    if (clusterlist[icluster]<0) continue;
    Int_t index = clusterlist[icluster];
    Int_t l=(index & 0xf0000000) >> 28;
    Int_t c=(index & 0x0fffffff) >> 00;
    if (ny[l]==0){
      printf("problem\n");
    }
    if (c>fgLayers[l].GetNumberOfClusters()) continue;
    //if (l>3) continue;
    AliITSRecPoint *cl = (AliITSRecPoint*)GetCluster(index);
    //
    Float_t deltan = 0;
    if (l>3&&cl->GetNy()+cl->GetNz()>6) continue;
    if (l>2&&AliITSReconstructor::GetRecoParam()->GetUseAmplitudeInfo(l))
      if (track->GetNormQ(l)/track->GetExpQ()>3.5) continue;
    if (l<2 || l>3){      
      deltan = (cl->GetNy()+cl->GetNz()-ny[l]-nz[l]);
    }
    else{
      deltan = (cl->GetNz()-nz[l]);
    }
    if (deltan>2.0) continue;  // extended - highly probable shared cluster
    //
    for (Int_t itrack=3;itrack>=0;itrack--){
      if (fgLayers[l].GetClusterTracks(itrack,c)<0) continue;
      if (fgLayers[l].GetClusterTracks(itrack,c)!=trackID){
       tracks[trackindex]  = fgLayers[l].GetClusterTracks(itrack,c);
       trackindex++;
      }
    }
  }
  if (trackindex==0) return -1;
  if (trackindex==1){    
    sharedtrack = tracks[0];
  }else{
    if (trackindex==2) sharedtrack =TMath::Min(tracks[0],tracks[1]);
    else{
      //
      Int_t tracks2[24], cluster[24];
      for (Int_t i=0;i<trackindex;i++){ tracks2[i]=-1; cluster[i]=0;}
      Int_t index =0;
      //
      for (Int_t i=0;i<trackindex;i++){
	if (tracks[i]<0) continue;
	tracks2[index] = tracks[i];
	cluster[index]++;	
	for (Int_t j=i+1;j<trackindex;j++){
	  if (tracks[j]<0) continue;
	  if (tracks[j]==tracks[i]){
	    cluster[index]++;
	    tracks[j]=-1;
	  }
	}
	index++;
      }
      Int_t max=0;
      for (Int_t i=0;i<index;i++){
	if (cluster[index]>max) {
	  sharedtrack=tracks2[index];
	  max=cluster[index];
	}
      }
    }
  }
  
  if (sharedtrack>=100000) return -1;
  //
  // make list of overlaps
  shared =0;
  for (Int_t icluster=0;icluster<6;icluster++){
    if (clusterlist[icluster]<0) continue;
    Int_t index = clusterlist[icluster];
    Int_t l=(index & 0xf0000000) >> 28;
    Int_t c=(index & 0x0fffffff) >> 00;
    if (c>fgLayers[l].GetNumberOfClusters()) continue;
    AliITSRecPoint *cl = (AliITSRecPoint*)GetCluster(index);
    if (l==0 || l==1){
      if (cl->GetNy()>2) continue;
      if (cl->GetNz()>2) continue;
    }
    if (l==4 || l==5){
      if (cl->GetNy()>3) continue;
      if (cl->GetNz()>3) continue;
    }
    //
    for (Int_t itrack=3;itrack>=0;itrack--){
      if (fgLayers[l].GetClusterTracks(itrack,c)<0) continue;
      if (fgLayers[l].GetClusterTracks(itrack,c)==sharedtrack){
	overlist[l]=index;
	shared++;      
      }
    }
  }
  return sharedtrack;
}
//------------------------------------------------------------------------
AliITStrackMI *  AliITStrackerMI::GetBest2Tracks(Int_t trackID1, Int_t trackID2, Float_t th0, Float_t th1){
  //
  // try to find track hypothesys without conflicts
  // with minimal chi2;
  TClonesArray *arr1 = (TClonesArray*)fTrackHypothesys.At(trackID1);
  Int_t entries1 = arr1->GetEntriesFast();
  TClonesArray *arr2 = (TClonesArray*)fTrackHypothesys.At(trackID2);
  if (!arr2) return (AliITStrackMI*) arr1->UncheckedAt(0);
  Int_t entries2 = arr2->GetEntriesFast();
  if (entries2<=0) return (AliITStrackMI*) arr1->UncheckedAt(0);
  //
  AliITStrackMI * track10=(AliITStrackMI*) arr1->UncheckedAt(0);
  AliITStrackMI * track20=(AliITStrackMI*) arr2->UncheckedAt(0);
  if (track10->Pt()>0.5+track20->Pt()) return track10;

  for (Int_t itrack=0;itrack<entries1;itrack++){
    AliITStrackMI * track=(AliITStrackMI*) arr1->UncheckedAt(itrack);
    UnRegisterClusterTracks(track,trackID1);
  }
  //
  for (Int_t itrack=0;itrack<entries2;itrack++){
    AliITStrackMI * track=(AliITStrackMI*) arr2->UncheckedAt(itrack);
    UnRegisterClusterTracks(track,trackID2);
  }
  Int_t index1=0;
  Int_t index2=0;
  Float_t maxconflicts=6;
  Double_t maxchi2 =1000.;
  //
  // get weight of hypothesys - using best hypothesy
  Double_t w1,w2;
 
  Int_t list1[6],list2[6];
  AliITSRecPoint *clist1[6], *clist2[6] ;
  RegisterClusterTracks(track10,trackID1);
  RegisterClusterTracks(track20,trackID2);
  Float_t conflict1 = GetNumberOfSharedClusters(track10,trackID1,list1,clist1);
  Float_t conflict2 = GetNumberOfSharedClusters(track20,trackID2,list2,clist2);
  UnRegisterClusterTracks(track10,trackID1);
  UnRegisterClusterTracks(track20,trackID2);
  //
  // normalized chi2
  Float_t chi21 =0,chi22=0,ncl1=0,ncl2=0;
  Float_t nerry[6],nerrz[6];
  Float_t *erry1=GetErrY(trackID1),*errz1 = GetErrZ(trackID1);
  Float_t *erry2=GetErrY(trackID2),*errz2 = GetErrZ(trackID2);
  for (Int_t i=0;i<6;i++){
     if ( (erry1[i]>0) && (erry2[i]>0)) {
       nerry[i] = TMath::Min(erry1[i],erry2[i]);
       nerrz[i] = TMath::Min(errz1[i],errz2[i]);
     }else{
       nerry[i] = TMath::Max(erry1[i],erry2[i]);
       nerrz[i] = TMath::Max(errz1[i],errz2[i]);
     }
     if (TMath::Abs(track10->GetDy(i))>0.000000000000001){
       chi21 += track10->GetDy(i)*track10->GetDy(i)/(nerry[i]*nerry[i]);
       chi21 += track10->GetDz(i)*track10->GetDz(i)/(nerrz[i]*nerrz[i]);
       ncl1++;
     }
     if (TMath::Abs(track20->GetDy(i))>0.000000000000001){
       chi22 += track20->GetDy(i)*track20->GetDy(i)/(nerry[i]*nerry[i]);
       chi22 += track20->GetDz(i)*track20->GetDz(i)/(nerrz[i]*nerrz[i]);
       ncl2++;
     }
  }
  chi21/=ncl1;
  chi22/=ncl2;
  //
  // 
  Float_t d1 = TMath::Sqrt(track10->GetD(0)*track10->GetD(0)+track10->GetD(1)*track10->GetD(1))+0.1;
  Float_t d2 = TMath::Sqrt(track20->GetD(0)*track20->GetD(0)+track20->GetD(1)*track20->GetD(1))+0.1;
  Float_t s1 = TMath::Sqrt(track10->GetSigmaY2()*track10->GetSigmaZ2());
  Float_t s2 = TMath::Sqrt(track20->GetSigmaY2()*track20->GetSigmaZ2());
  //
  w1 = (d2/(d1+d2)+ 2*s2/(s1+s2)+
	+s2/(s1+s2)*0.5*(chi22+2.)/(chi21+chi22+4.)
	+1.*track10->Pt()/(track10->Pt()+track20->Pt())
	);
  w2 = (d1/(d1+d2)+ 2*s1/(s1+s2)+
	s1/(s1+s2)*0.5*(chi21+2.)/(chi21+chi22+4.)
	+1.*track20->Pt()/(track10->Pt()+track20->Pt())
	);

  Double_t sumw = w1+w2;
  w1/=sumw;
  w2/=sumw;
  if (w1<w2*0.5) {
    w1 = (d2+0.5)/(d1+d2+1.);
    w2 = (d1+0.5)/(d1+d2+1.);
  }
  //  Float_t maxmax       = w1*track10->fChi2MIP[0]+w2*track20->fChi2MIP[0]+w1*conflict1+w2*conflict2+1.;
  //Float_t maxconflicts0 = w1*conflict1+w2*conflict2;
  //
  // get pair of "best" hypothesys
  //  
  Float_t * ny1 = GetNy(trackID1), * nz1 = GetNz(trackID1); 
  Float_t * ny2 = GetNy(trackID2), * nz2 = GetNz(trackID2); 

  for (Int_t itrack1=0;itrack1<entries1;itrack1++){
    AliITStrackMI * track1=(AliITStrackMI*) arr1->UncheckedAt(itrack1);
    //if (track1->fFakeRatio>0) continue;
    RegisterClusterTracks(track1,trackID1);
    for (Int_t itrack2=0;itrack2<entries2;itrack2++){
      AliITStrackMI * track2=(AliITStrackMI*) arr2->UncheckedAt(itrack2);

      //      Float_t current = w1*track1->fChi2MIP[0]+w2*track2->fChi2MIP[0];
      //if (track2->fFakeRatio>0) continue;
      Float_t nskipped=0;            
      RegisterClusterTracks(track2,trackID2);
      Float_t cconflict1 = GetNumberOfSharedClusters(track1,trackID1,list1,clist1);
      Float_t cconflict2 = GetNumberOfSharedClusters(track2,trackID2,list2,clist2);
      UnRegisterClusterTracks(track2,trackID2);
      //
      if (track1->GetConstrain()) nskipped+=w1*track1->GetNSkipped();
      if (track2->GetConstrain()) nskipped+=w2*track2->GetNSkipped();
      if (nskipped>0.5) continue;
      //
      //if ( w1*conflict1+w2*conflict2>maxconflicts0) continue;
      if (conflict1+1<cconflict1) continue;
      if (conflict2+1<cconflict2) continue;
      Float_t conflict=0;
      Float_t sumchi2=0;
      Float_t sum=0;
      for (Int_t i=0;i<6;i++){
	//
	Float_t c1 =0.; // conflict coeficients
	Float_t c2 =0.; 
	if (clist1[i]&&clist2[i]){
	  Float_t deltan = 0;
	  if (i<2 || i>3){      
	    deltan = (clist1[i]->GetNy()+clist1[i]->GetNz()-TMath::Max(ny1[i],ny2[i])-TMath::Max(nz1[i],nz2[i]));
	  }
	  else{
	    deltan = (clist1[i]->GetNz()-TMath::Max(nz1[i],nz2[i]));
	  }
	  c1  = 2./TMath::Max(3.+deltan,2.);	  
	  c2  = 2./TMath::Max(3.+deltan,2.);	  
	}
	else{
	  if (clist1[i]){
	    Float_t deltan = 0;
	    if (i<2 || i>3){      
	      deltan = (clist1[i]->GetNy()+clist1[i]->GetNz()-ny1[i]-nz1[i]);
	    }
	    else{
	      deltan = (clist1[i]->GetNz()-nz1[i]);
	    }
	    c1  = 2./TMath::Max(3.+deltan,2.);	  
	    c2  = 0;
	  }

	  if (clist2[i]){
	    Float_t deltan = 0;
	    if (i<2 || i>3){      
	      deltan = (clist2[i]->GetNy()+clist2[i]->GetNz()-ny2[i]-nz2[i]);
	    }
	    else{
	      deltan = (clist2[i]->GetNz()-nz2[i]);
	    }
	    c2  = 2./TMath::Max(3.+deltan,2.);	  
	    c1  = 0;
	  }	  
	}
	//
	chi21=0;chi22=0;
	if (TMath::Abs(track1->GetDy(i))>0.) {
	  chi21 = (track1->GetDy(i)/track1->GetSigmaY(i))*(track1->GetDy(i)/track1->GetSigmaY(i))+
	    (track1->GetDz(i)/track1->GetSigmaZ(i))*(track1->GetDz(i)/track1->GetSigmaZ(i));
	  //chi21 = (track1->fDy[i]*track1->fDy[i])/(nerry[i]*nerry[i])+
	  //  (track1->GetDz(i)*track1->GetDz(i))/(nerrz[i]*nerrz[i]);
	}else{
	  if (TMath::Abs(track1->GetSigmaY(i)>0.)) c1=1;
	}
	//
	if (TMath::Abs(track2->GetDy(i))>0.) {
	  chi22 = (track2->GetDy(i)/track2->GetSigmaY(i))*(track2->GetDy(i)/track2->GetSigmaY(i))+
	    (track2->GetDz(i)/track2->GetSigmaZ(i))*(track2->GetDz(i)/track2->GetSigmaZ(i));
	  //chi22 = (track2->fDy[i]*track2->fDy[i])/(nerry[i]*nerry[i])+
	  //  (track2->fDz[i]*track2->fDz[i])/(nerrz[i]*nerrz[i]);
	}
	else{
	  if (TMath::Abs(track2->GetSigmaY(i)>0.)) c2=1;
	}
	sumchi2+=w1*(1.+c1)*(1+c1)*(chi21+c1)+w2*(1.+c2)*(1+c2)*(chi22+c2);
	if (chi21>0) sum+=w1;
	if (chi22>0) sum+=w2;
	conflict+=(c1+c2);
      }
      Double_t norm = sum-w1*track1->GetNSkipped()-w2*track2->GetNSkipped();
      if (norm<0) norm =1/(w1*track1->GetNSkipped()+w2*track2->GetNSkipped());
      Double_t normchi2 = 2*conflict+sumchi2/sum;
      if ( normchi2 <maxchi2 ){	  
	index1 = itrack1;
	index2 = itrack2;
	maxconflicts = conflict;
	maxchi2 = normchi2;
      }      
    }
    UnRegisterClusterTracks(track1,trackID1);
  }
  //
  //  if (maxconflicts<4 && maxchi2<th0){   
  if (maxchi2<th0*2.){   
    Float_t orig = track10->GetFakeRatio()*track10->GetNumberOfClusters();
    AliITStrackMI* track1=(AliITStrackMI*) arr1->UncheckedAt(index1);
    track1->SetChi2MIP(5,maxconflicts);
    track1->SetChi2MIP(6,maxchi2);
    track1->SetChi2MIP(7,0.01+orig-(track1->GetFakeRatio()*track1->GetNumberOfClusters()));
    //    track1->UpdateESDtrack(AliESDtrack::kITSin);
    track1->SetChi2MIP(8,index1);
    fBestTrackIndex[trackID1] =index1;
    UpdateESDtrack(track1, AliESDtrack::kITSin);
  }  
  else if (track10->GetChi2MIP(0)<th1){
    track10->SetChi2MIP(5,maxconflicts);
    track10->SetChi2MIP(6,maxchi2);    
    //    track10->UpdateESDtrack(AliESDtrack::kITSin);
    UpdateESDtrack(track10,AliESDtrack::kITSin);
  }   
  
  for (Int_t itrack=0;itrack<entries1;itrack++){
    AliITStrackMI * track=(AliITStrackMI*) arr1->UncheckedAt(itrack);
    UnRegisterClusterTracks(track,trackID1);
  }
  //
  for (Int_t itrack=0;itrack<entries2;itrack++){
    AliITStrackMI * track=(AliITStrackMI*) arr2->UncheckedAt(itrack);
    UnRegisterClusterTracks(track,trackID2);
  }

  if (track10->GetConstrain()&&track10->GetChi2MIP(0)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)&&track10->GetChi2MIP(1)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(1)
      &&track10->GetChi2MIP(2)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(2)&&track10->GetChi2MIP(3)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(3)){ 
    //  if (track10->fChi2MIP[0]<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)&&track10->fChi2MIP[1]<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(1)
  //    &&track10->fChi2MIP[2]<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(2)&&track10->fChi2MIP[3]<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(3)){ 
    RegisterClusterTracks(track10,trackID1);
  }
  if (track20->GetConstrain()&&track20->GetChi2MIP(0)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)&&track20->GetChi2MIP(1)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(1)
      &&track20->GetChi2MIP(2)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(2)&&track20->GetChi2MIP(3)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(3)){ 
    //if (track20->fChi2MIP[0]<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)&&track20->fChi2MIP[1]<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(1)
    //  &&track20->fChi2MIP[2]<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(2)&&track20->fChi2MIP[3]<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(3)){ 
    RegisterClusterTracks(track20,trackID2);  
  }
  return track10; 
 
}
//------------------------------------------------------------------------
void AliITStrackerMI::UseClusters(const AliKalmanTrack *t, Int_t from) const {
  //--------------------------------------------------------------------
  // This function marks clusters assigned to the track
  //--------------------------------------------------------------------
  AliTracker::UseClusters(t,from);

  AliITSRecPoint *c=(AliITSRecPoint *)GetCluster(t->GetClusterIndex(0));
  //if (c->GetQ()>2) c->Use();
  if (c->GetSigmaZ2()>0.1) c->Use();
  c=(AliITSRecPoint *)GetCluster(t->GetClusterIndex(1));
  //if (c->GetQ()>2) c->Use();
  if (c->GetSigmaZ2()>0.1) c->Use();

}
//------------------------------------------------------------------------
void AliITStrackerMI::AddTrackHypothesys(AliITStrackMI * track, Int_t esdindex)
{
  //------------------------------------------------------------------
  // add track to the list of hypothesys
  //------------------------------------------------------------------

  if (esdindex>=fTrackHypothesys.GetEntriesFast()) 
    fTrackHypothesys.Expand(TMath::Max(fTrackHypothesys.GetSize(),esdindex*2+10));
  //
  TObjArray * array = (TObjArray*) fTrackHypothesys.At(esdindex);
  if (!array) {
    array = new TObjArray(10);
    fTrackHypothesys.AddAt(array,esdindex);
  }
  array->AddLast(track);
}
//------------------------------------------------------------------------
void AliITStrackerMI::SortTrackHypothesys(Int_t esdindex, Int_t maxcut, Int_t mode)
{
  //-------------------------------------------------------------------
  // compress array of track hypothesys
  // keep only maxsize best hypothesys
  //-------------------------------------------------------------------
  if (esdindex>fTrackHypothesys.GetEntriesFast()) return;
  if (! (fTrackHypothesys.At(esdindex)) ) return;
  TObjArray * array = (TObjArray*) fTrackHypothesys.At(esdindex);
  Int_t entries = array->GetEntriesFast();
  //
  //- find preliminary besttrack as a reference
  Float_t minchi2=10000;
  Int_t maxn=0;
  AliITStrackMI * besttrack=0;
  for (Int_t itrack=0;itrack<array->GetEntriesFast();itrack++){
    AliITStrackMI * track = (AliITStrackMI*)array->At(itrack);
    if (!track) continue;
    Float_t chi2 = NormalizedChi2(track,0);
    //
    Int_t tpcLabel=track->GetESDtrack()->GetTPCLabel();
    track->SetLabel(tpcLabel);
    CookdEdx(track);
    track->SetFakeRatio(1.);
    CookLabel(track,0.); //For comparison only
    //
    //if (chi2<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)&&track->fFakeRatio==0){
    if (chi2<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)){
      if (track->GetNumberOfClusters()<maxn) continue;
      maxn = track->GetNumberOfClusters();
      if (chi2<minchi2){
	minchi2=chi2;
	besttrack=track;
      }
    }
    else{
      if (track->GetConstrain() || track->GetNumberOfClusters()>5){  //keep best short tracks - without vertex constrain
	delete array->RemoveAt(itrack);
      }	 
    }
  }
  if (!besttrack) return;
  //
  //
  //take errors of best track as a reference
  Float_t *erry = GetErrY(esdindex), *errz = GetErrZ(esdindex);
  Float_t *ny = GetNy(esdindex), *nz = GetNz(esdindex);
  for (Int_t j=0;j<6;j++) {
    if (besttrack->GetClIndex(j)>0){
      erry[j] = besttrack->GetSigmaY(j); erry[j+6] = besttrack->GetSigmaY(j+6);
      errz[j] = besttrack->GetSigmaZ(j); errz[j+6] = besttrack->GetSigmaZ(j+6);
      ny[j]   = besttrack->GetNy(j);
      nz[j]   = besttrack->GetNz(j);
    }
  }
  //
  // calculate normalized chi2
  //
  Float_t * chi2        = new Float_t[entries];
  Int_t * index         = new Int_t[entries];  
  for (Int_t i=0;i<entries;i++) chi2[i] =10000;
  for (Int_t itrack=0;itrack<entries;itrack++){
    AliITStrackMI * track = (AliITStrackMI*)array->At(itrack);
    if (track){
      track->SetChi2MIP(0,GetNormalizedChi2(track, mode));            
      if (track->GetChi2MIP(0)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)) 
	chi2[itrack] = track->GetChi2MIP(0);
      else{
	if (track->GetConstrain() || track->GetNumberOfClusters()>5){  //keep best short tracks - without vertex constrain
	  delete array->RemoveAt(itrack);	     
	}
      }
    }
  }
  //
  TMath::Sort(entries,chi2,index,kFALSE);
  besttrack = (AliITStrackMI*)array->At(index[0]);
  if (besttrack&&besttrack->GetChi2MIP(0)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)){
    for (Int_t j=0;j<6;j++){
      if (besttrack->GetClIndex(j)>0){
	erry[j] = besttrack->GetSigmaY(j); erry[j+6] = besttrack->GetSigmaY(j+6);
	errz[j] = besttrack->GetSigmaZ(j); erry[j+6] = besttrack->GetSigmaY(j+6);
	ny[j]   = besttrack->GetNy(j);
	nz[j]   = besttrack->GetNz(j);
      }
    }
  }
  //
  // calculate one more time with updated normalized errors
  for (Int_t i=0;i<entries;i++) chi2[i] =10000;  
  for (Int_t itrack=0;itrack<entries;itrack++){
    AliITStrackMI * track = (AliITStrackMI*)array->At(itrack);
    if (track){      
      track->SetChi2MIP(0,GetNormalizedChi2(track,mode));            
      if (track->GetChi2MIP(0)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)) 
	chi2[itrack] = track->GetChi2MIP(0)-0*(track->GetNumberOfClusters()+track->GetNDeadZone()); 
      else
	{
	  if (track->GetConstrain() || track->GetNumberOfClusters()>5){  //keep best short tracks - without vertex constrain
	    delete array->RemoveAt(itrack);	
	  }
	}
    }   
  }
  entries = array->GetEntriesFast();  
  //
  //
  if (entries>0){
    TObjArray * newarray = new TObjArray();  
    TMath::Sort(entries,chi2,index,kFALSE);
    besttrack = (AliITStrackMI*)array->At(index[0]);
    if (besttrack){
      //
      for (Int_t j=0;j<6;j++){
	if (besttrack->GetNz(j)>0&&besttrack->GetNy(j)>0){
	  erry[j] = besttrack->GetSigmaY(j); erry[j+6] = besttrack->GetSigmaY(j+6);
	  errz[j] = besttrack->GetSigmaZ(j); errz[j+6] = besttrack->GetSigmaZ(j+6);
	  ny[j]   = besttrack->GetNy(j);
	  nz[j]   = besttrack->GetNz(j);
	}
      }
      besttrack->SetChi2MIP(0,GetNormalizedChi2(besttrack,mode));
      minchi2 = TMath::Min(besttrack->GetChi2MIP(0)+5.+besttrack->GetNUsed(), double(AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)));
      Float_t minn = besttrack->GetNumberOfClusters()-3;
      Int_t accepted=0;
      for (Int_t i=0;i<entries;i++){
	AliITStrackMI * track = (AliITStrackMI*)array->At(index[i]);	
	if (!track) continue;
	if (accepted>maxcut) break;
	track->SetChi2MIP(0,GetNormalizedChi2(track,mode));
	if (track->GetConstrain() || track->GetNumberOfClusters()>5){  //keep best short tracks - without vertex constrain
	  if (track->GetNumberOfClusters()<6 && (track->GetChi2MIP(0)+track->GetNUsed()>minchi2)){
	    delete array->RemoveAt(index[i]);
	    continue;
	  }
	}
	Bool_t shortbest = !track->GetConstrain() && track->GetNumberOfClusters()<6;
	if ((track->GetChi2MIP(0)+track->GetNUsed()<minchi2 && track->GetNumberOfClusters()>=minn) ||shortbest){
	  if (!shortbest) accepted++;
	  //
	  newarray->AddLast(array->RemoveAt(index[i]));      
	  for (Int_t j=0;j<6;j++){
	    if (nz[j]==0){
	      erry[j] = track->GetSigmaY(j); erry[j+6] = track->GetSigmaY(j+6);
	      errz[j] = track->GetSigmaZ(j); errz[j]   = track->GetSigmaZ(j+6);
	      ny[j]   = track->GetNy(j);
	      nz[j]   = track->GetNz(j);
	    }
	  }
	}
	else{
	  delete array->RemoveAt(index[i]);
	}
      }
      array->Delete();
      delete fTrackHypothesys.RemoveAt(esdindex);
      fTrackHypothesys.AddAt(newarray,esdindex);
    }
    else{
      array->Delete();
      delete fTrackHypothesys.RemoveAt(esdindex);
    }
  }
  delete [] chi2;
  delete [] index;
}
//------------------------------------------------------------------------
AliITStrackMI * AliITStrackerMI::GetBestHypothesys(Int_t esdindex, AliITStrackMI * original, Int_t checkmax)
{
  //-------------------------------------------------------------
  // try to find best hypothesy
  // currently - minimal chi2 of track+backpropagated track+matching to the tpc track
  //-------------------------------------------------------------
  if (fTrackHypothesys.GetEntriesFast()<=esdindex) return 0;
  TObjArray * array = (TObjArray*) fTrackHypothesys.At(esdindex);
  if (!array) return 0;
  Int_t entries = array->GetEntriesFast();
  if (!entries) return 0;  
  Float_t minchi2 = 100000;
  AliITStrackMI * besttrack=0;
  //
  AliITStrackMI * backtrack    = new AliITStrackMI(*original);
  AliITStrackMI * forwardtrack = new AliITStrackMI(*original);
  Double_t xyzVtx[]={GetX(),GetY(),GetZ()};	
  Double_t ersVtx[]={GetSigmaX()/3.,GetSigmaY()/3.,GetSigmaZ()/3.};
  //
  for (Int_t i=0;i<entries;i++){    
    AliITStrackMI * track = (AliITStrackMI*)array->At(i);    
    if (!track) continue;
    Float_t sigmarfi,sigmaz;
    GetDCASigma(track,sigmarfi,sigmaz);
    track->SetDnorm(0,sigmarfi);
    track->SetDnorm(1,sigmaz);
    //
    track->SetChi2MIP(1,1000000);
    track->SetChi2MIP(2,1000000);
    track->SetChi2MIP(3,1000000);
    //
    // backtrack
    backtrack = new(backtrack) AliITStrackMI(*track); 
    if (track->GetConstrain()) {
      if (!CorrectForPipeMaterial(backtrack,"inward")) continue;
      if (!backtrack->Improve(0,xyzVtx,ersVtx))         continue;     
      backtrack->ResetCovariance(10.);      
    }else{
      backtrack->ResetCovariance(10.);
    }
    backtrack->ResetClusters();

    Double_t x = original->GetX();
    if (!RefitAt(x,backtrack,track)) continue;
    //
    track->SetChi2MIP(1,NormalizedChi2(backtrack,0));
    //for (Int_t i=2;i<6;i++){track->fDy[i]+=backtrack->fDy[i]; track->fDz[i]+=backtrack->fDz[i];}
    if (track->GetChi2MIP(1)>AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(1)*6.)  continue;
    track->SetChi22(GetMatchingChi2(backtrack,original));

    if ((track->GetConstrain()) && track->GetChi22()>90.)  continue;
    if ((!track->GetConstrain()) && track->GetChi22()>30.)  continue;
    if ( track->GetChi22()/track->GetNumberOfClusters()>11.)  continue;


    if  (!(track->GetConstrain())&&track->GetChi2MIP(1)>AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(1))  continue;
    //
    //forward track - without constraint
    forwardtrack = new(forwardtrack) AliITStrackMI(*original);
    forwardtrack->ResetClusters();
    x = track->GetX();
    RefitAt(x,forwardtrack,track);
    track->SetChi2MIP(2,NormalizedChi2(forwardtrack,0));    
    if  (track->GetChi2MIP(2)>AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(2)*6.0)  continue;
    if  (!(track->GetConstrain())&&track->GetChi2MIP(2)>AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(2))  continue;
    
    //track->fD[0] = forwardtrack->GetD(GetX(),GetY());
    //track->fD[1] = forwardtrack->GetZat(GetX())-GetZ();
    forwardtrack->GetDZ(GetX(),GetY(),GetZ(),track->GetDP());   //I.B.
    forwardtrack->SetD(0,track->GetD(0));
    forwardtrack->SetD(1,track->GetD(1));    
    {
      Int_t list[6];
      AliITSRecPoint* clist[6];
      track->SetChi2MIP(4,GetNumberOfSharedClusters(track,esdindex,list,clist));      
      if ( (!track->GetConstrain()) && track->GetChi2MIP(4)>1.0) continue;
    }
    
    track->SetChi2MIP(3,GetInterpolatedChi2(forwardtrack,backtrack));
    if  ( (track->GetChi2MIP(3)>6.*AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(3))) continue;    
    if  ( (!track->GetConstrain()) && (track->GetChi2MIP(3)>2*AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(3))) {
      track->SetChi2MIP(3,1000);
      continue; 
    }
    Double_t chi2 = track->GetChi2MIP(0)+track->GetNUsed();    
    //
    for (Int_t ichi=0;ichi<5;ichi++){
      forwardtrack->SetChi2MIP(ichi, track->GetChi2MIP(ichi));
    }
    if (chi2 < minchi2){
      //besttrack = new AliITStrackMI(*forwardtrack);
      besttrack = track;
      besttrack->SetLabel(track->GetLabel());
      besttrack->SetFakeRatio(track->GetFakeRatio());
      minchi2   = chi2;
      //original->fD[0] = forwardtrack->GetD(GetX(),GetY());
      //original->fD[1] = forwardtrack->GetZat(GetX())-GetZ();
      forwardtrack->GetDZ(GetX(),GetY(),GetZ(),original->GetDP());    //I.B.
    }    
  }
  delete backtrack;
  delete forwardtrack;
  Int_t accepted=0;
  for (Int_t i=0;i<entries;i++){    
    AliITStrackMI * track = (AliITStrackMI*)array->At(i);
   
    if (!track) continue;
    
    if (accepted>checkmax || track->GetChi2MIP(3)>AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(3)*6. || 
	(track->GetNumberOfClusters()<besttrack->GetNumberOfClusters()-1.)||
	track->GetChi2MIP(0)>besttrack->GetChi2MIP(0)+2.*besttrack->GetNUsed()+3.){
      if (track->GetConstrain() || track->GetNumberOfClusters()>5){  //keep best short tracks - without vertex constrain
	delete array->RemoveAt(i);    
	continue;
      }
    }
    else{
      accepted++;
    }
  }
  //
  array->Compress();
  SortTrackHypothesys(esdindex,checkmax,1);

  array = (TObjArray*) fTrackHypothesys.At(esdindex);
  if (!array) return 0; // PH What can be the reason? Check SortTrackHypothesys
  besttrack = (AliITStrackMI*)array->At(0);  
  if (!besttrack)  return 0;
  besttrack->SetChi2MIP(8,0);
  fBestTrackIndex[esdindex]=0;
  entries = array->GetEntriesFast();
  AliITStrackMI *longtrack =0;
  minchi2 =1000;
  Float_t minn=besttrack->GetNumberOfClusters()+besttrack->GetNDeadZone();
  for (Int_t itrack=entries-1;itrack>0;itrack--) {
    AliITStrackMI * track = (AliITStrackMI*)array->At(itrack);
    if (!track->GetConstrain()) continue;
    if (track->GetNumberOfClusters()+track->GetNDeadZone()<minn) continue;
    if (track->GetChi2MIP(0)-besttrack->GetChi2MIP(0)>0.0) continue;
    if (track->GetChi2MIP(0)>4.) continue;
    minn = track->GetNumberOfClusters()+track->GetNDeadZone();
    longtrack =track;
  }
  //if (longtrack) besttrack=longtrack;

  Int_t list[6];
  AliITSRecPoint * clist[6];
  Float_t shared = GetNumberOfSharedClusters(besttrack,esdindex,list,clist);
  if (besttrack->GetConstrain()&&besttrack->GetChi2MIP(0)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(0)&&besttrack->GetChi2MIP(1)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(1)
      &&besttrack->GetChi2MIP(2)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(2)&&besttrack->GetChi2MIP(3)<AliITSReconstructor::GetRecoParam()->GetMaxChi2PerCluster(3)){ 
    RegisterClusterTracks(besttrack,esdindex);
  }
  //
  //
  if (shared>0.0){
    Int_t nshared;
    Int_t overlist[6];
    Int_t sharedtrack = GetOverlapTrack(besttrack, esdindex, nshared, list, overlist);
    if (sharedtrack>=0){
      //
      besttrack = GetBest2Tracks(esdindex,sharedtrack,10,5.5);     
      if (besttrack){
	shared = GetNumberOfSharedClusters(besttrack,esdindex,list,clist);
      }
      else return 0;
    }
  }  
  
  if (shared>2.5) return 0;
  if (shared>1.0) return besttrack;
  //
  // Don't sign clusters if not gold track
  //
  if (!besttrack->IsGoldPrimary()) return besttrack;
  if (besttrack->GetESDtrack()->GetKinkIndex(0)!=0) return besttrack;   //track belong to kink
  //
  if (fConstraint[fPass]){
    //
    // sign clusters
    //
    Float_t *ny = GetNy(esdindex), *nz = GetNz(esdindex);
    for (Int_t i=0;i<6;i++){
      Int_t index = besttrack->GetClIndex(i);
      if (index<=0) continue; 
      Int_t ilayer =  (index & 0xf0000000) >> 28;
      if (besttrack->GetSigmaY(ilayer)<0.00000000001) continue;
      AliITSRecPoint *c = (AliITSRecPoint*)GetCluster(index);     
      if (!c) continue;
      if (ilayer>3&&c->GetNy()+c->GetNz()>6) continue;
      if ( (c->GetNy()+c->GetNz() )> ny[i]+nz[i]+0.7) continue; //shared track
      if (  c->GetNz()> nz[i]+0.7) continue; //shared track
      if ( ilayer>2&& AliITSReconstructor::GetRecoParam()->GetUseAmplitudeInfo(ilayer)) 
	if (besttrack->GetNormQ(ilayer)/besttrack->GetExpQ()>1.5) continue;
      //if (  c->GetNy()> ny[i]+0.7) continue; //shared track

      Bool_t cansign = kTRUE;
      for (Int_t itrack=0;itrack<entries; itrack++){
	AliITStrackMI * track = (AliITStrackMI*)array->At(i);   
	if (!track) continue;
	if (track->GetChi2MIP(0)>besttrack->GetChi2MIP(0)+2.*shared+1.) break;
	if ( (track->GetClIndex(ilayer)>0) && (track->GetClIndex(ilayer)!=besttrack->GetClIndex(ilayer))){
	  cansign = kFALSE;
	  break;
	}
      }
      if (cansign){
	if (TMath::Abs(besttrack->GetDy(ilayer)/besttrack->GetSigmaY(ilayer))>3.) continue;
	if (TMath::Abs(besttrack->GetDz(ilayer)/besttrack->GetSigmaZ(ilayer))>3.) continue;    
	if (!c->IsUsed()) c->Use();
      }
    }
  }
  return besttrack;
} 
//------------------------------------------------------------------------
void  AliITStrackerMI::GetBestHypothesysMIP(TObjArray &itsTracks)
{
  //
  // get "best" hypothesys
  //

  Int_t nentries = itsTracks.GetEntriesFast();
  for (Int_t i=0;i<nentries;i++){
    AliITStrackMI* track = (AliITStrackMI*)itsTracks.At(i);
    if (!track) continue;
    TObjArray * array = (TObjArray*) fTrackHypothesys.At(i);
    if (!array) continue;
    if (array->GetEntriesFast()<=0) continue;
    //
    AliITStrackMI* longtrack=0;
    Float_t minn=0;
    Float_t maxchi2=1000;
    for (Int_t j=0;j<array->GetEntriesFast();j++){
      AliITStrackMI* trackHyp = (AliITStrackMI*)array->At(j);
      if (!trackHyp) continue;
      if (trackHyp->GetGoldV0()) {
	longtrack = trackHyp;   //gold V0 track taken
	break;
      }
      if (trackHyp->GetNumberOfClusters()+trackHyp->GetNDeadZone()<minn) continue;
      Float_t chi2 = trackHyp->GetChi2MIP(0);
      if (fAfterV0){
	if (!trackHyp->GetGoldV0()&&trackHyp->GetConstrain()==kFALSE) chi2+=5;
      }
      if (trackHyp->GetNumberOfClusters()+trackHyp->GetNDeadZone()>minn) maxchi2 = trackHyp->GetChi2MIP(0);       
      //
      if (chi2 > maxchi2) continue;
      minn= trackHyp->GetNumberOfClusters()+trackHyp->GetNDeadZone();
      maxchi2 = chi2;
      longtrack=trackHyp;
    }    
    //
    //
    //
    AliITStrackMI * besttrack = (AliITStrackMI*)array->At(0);
    if (!longtrack) {longtrack = besttrack;}
    else besttrack= longtrack;
    //
    if (besttrack) {
      Int_t list[6];
      AliITSRecPoint * clist[6];
      Float_t shared = GetNumberOfSharedClusters(longtrack,i,list,clist);
      //
      track->SetNUsed(shared);      
      track->SetNSkipped(besttrack->GetNSkipped());
      track->SetChi2MIP(0,besttrack->GetChi2MIP(0));
      if (shared>0) {
	if(!AliITSReconstructor::GetRecoParam()->GetAllowSharedClusters()) continue;
	Int_t nshared;
	Int_t overlist[6]; 
	//
	Int_t sharedtrack = GetOverlapTrack(longtrack, i, nshared, list, overlist);
	//if (sharedtrack==-1) sharedtrack=0;
	if (sharedtrack>=0) {       
	  besttrack = GetBest2Tracks(i,sharedtrack,10,5.5);			  
	}
      }   
      if (besttrack&&fAfterV0) {
	UpdateESDtrack(besttrack,AliESDtrack::kITSin);
      }
      if (besttrack&&fConstraint[fPass]) 
      	UpdateESDtrack(besttrack,AliESDtrack::kITSin);
      if (besttrack->GetChi2MIP(0)+besttrack->GetNUsed()>1.5 && fConstraint[fPass]) {
	if ( TMath::Abs(besttrack->GetD(0))>0.1 || 
	     TMath::Abs(besttrack->GetD(1))>0.1 ) track->SetReconstructed(kFALSE);	
      }       

    }    
  }
} 
//------------------------------------------------------------------------
void AliITStrackerMI::CookLabel(AliITStrackMI *track,Float_t wrong) const {
  //--------------------------------------------------------------------
  //This function "cooks" a track label. If label<0, this track is fake.
  //--------------------------------------------------------------------
  Int_t tpcLabel=-1; 
     
  if ( track->GetESDtrack())   tpcLabel =  TMath::Abs(track->GetESDtrack()->GetTPCLabel());

   track->SetChi2MIP(9,0);
   Int_t nwrong=0;
   for (Int_t i=0;i<track->GetNumberOfClusters();i++){
     Int_t cindex = track->GetClusterIndex(i);
     Int_t l=(cindex & 0xf0000000) >> 28;
     AliITSRecPoint *cl = (AliITSRecPoint*)GetCluster(cindex);
     Int_t isWrong=1;
     for (Int_t ind=0;ind<3;ind++){
       if (tpcLabel>0)
	 if (cl->GetLabel(ind)==tpcLabel) isWrong=0;
     }
     track->SetChi2MIP(9,track->GetChi2MIP(9)+isWrong*(2<<l));
     nwrong+=isWrong;
   }
   Int_t nclusters = track->GetNumberOfClusters();
   if (nclusters > 0) //PH Some tracks don't have any cluster
     track->SetFakeRatio(double(nwrong)/double(nclusters));
   if (tpcLabel>0){
     if (track->GetFakeRatio()>wrong) track->SetLabel(-tpcLabel);
     else
       track->SetLabel(tpcLabel);
   }
   
}
//------------------------------------------------------------------------
void AliITStrackerMI::CookdEdx(AliITStrackMI* track)
{
  //
  //
  //  Int_t list[6];
  //AliITSRecPoint * clist[6];
  //  Int_t shared = GetNumberOfSharedClusters(track,index,list,clist);
  Float_t dedx[4];
  Int_t accepted=0;
  track->SetChi2MIP(9,0);
  for (Int_t i=0;i<track->GetNumberOfClusters();i++){
    Int_t cindex = track->GetClusterIndex(i);
    Int_t l=(cindex & 0xf0000000) >> 28;
    AliITSRecPoint *cl = (AliITSRecPoint*)GetCluster(cindex);
    Int_t lab = TMath::Abs(track->GetESDtrack()->GetTPCLabel());
    Int_t isWrong=1;
    for (Int_t ind=0;ind<3;ind++){
      if (cl->GetLabel(ind)==lab) isWrong=0;
    }
    track->SetChi2MIP(9,track->GetChi2MIP(9)+isWrong*(2<<l));
    if (l<2) continue;
    //if (l>3 && (cl->GetNy()>4) || (cl->GetNz()>4)) continue;  //shared track
    //if (l>3&& !(cl->GetType()==1||cl->GetType()==10)) continue;
    //if (l<4&& !(cl->GetType()==1)) continue;   
    dedx[accepted]= track->GetSampledEdx(i);
    //dedx[accepted]= track->fNormQ[l];
    accepted++;
  }
  if (accepted<1) {
    track->SetdEdx(0);
    return;
  }
  Int_t indexes[4];
  TMath::Sort(accepted,dedx,indexes,kFALSE);
  Double_t low=0.;
  Double_t up=0.51;    
  Double_t nl=low*accepted, nu =up*accepted;  
  Float_t sumamp = 0;
  Float_t sumweight =0;
  for (Int_t i=0; i<accepted; i++) {
    Float_t weight =1;
    if (i<nl+0.1)     weight = TMath::Max(1.-(nl-i),0.);
    if (i>nu-1)     weight = TMath::Max(nu-i,0.);
    sumamp+= dedx[indexes[i]]*weight;
    sumweight+=weight;
  }
  track->SetdEdx(sumamp/sumweight);
}
//------------------------------------------------------------------------
void AliITStrackerMI::MakeCoefficients(Int_t ntracks){
  //
  //
  if (fCoefficients) delete []fCoefficients;
  fCoefficients = new Float_t[ntracks*48];
  for (Int_t i=0;i<ntracks*48;i++) fCoefficients[i]=-1.;
}
//------------------------------------------------------------------------
Double_t AliITStrackerMI::GetPredictedChi2MI(AliITStrackMI* track, const AliITSRecPoint *cluster,Int_t layer) 
{
  //
  //
  //
  Float_t erry,errz;
  Float_t theta = track->GetTgl();
  Float_t phi   = track->GetSnp();
  phi = TMath::Sqrt(phi*phi/(1.-phi*phi));
  AliITSClusterParam::GetError(layer,cluster,theta,phi,track->GetExpQ(),erry,errz);
  AliDebug(2,Form(" chi2: tr-cl   %f  %f   tr X %f cl X %f",track->GetY()-cluster->GetY(),track->GetZ()-cluster->GetZ(),track->GetX(),cluster->GetX()));
  // Take into account the mis-alignment (bring track to cluster plane)
  Double_t xTrOrig=track->GetX();
  if (!track->PropagateTo(xTrOrig+cluster->GetX(),0.,0.)) return 1000.;
  AliDebug(2,Form(" chi2: tr-cl   %f  %f   tr X %f cl X %f",track->GetY()-cluster->GetY(),track->GetZ()-cluster->GetZ(),track->GetX(),cluster->GetX()));
  Double_t chi2 = track->GetPredictedChi2MI(cluster->GetY(),cluster->GetZ(),erry,errz);
  // Bring the track back to detector plane in ideal geometry
  // [mis-alignment will be accounted for in UpdateMI()]
  if (!track->PropagateTo(xTrOrig,0.,0.)) return 1000.;
  Float_t ny,nz;
  AliITSClusterParam::GetNTeor(layer,cluster,theta,phi,ny,nz);  
  Double_t delta = cluster->GetNy()+cluster->GetNz()-nz-ny;
  if (delta>1){
    chi2+=0.5*TMath::Min(delta/2,2.);
    chi2+=2.*cluster->GetDeltaProbability();
  }
  //
  track->SetNy(layer,ny);
  track->SetNz(layer,nz);
  track->SetSigmaY(layer,erry);
  track->SetSigmaZ(layer, errz);
  //track->fNormQ[layer] = cluster->GetQ()/TMath::Sqrt(1+theta*theta+phi*phi);
  track->SetNormQ(layer,cluster->GetQ()/TMath::Sqrt((1.+ track->GetTgl()*track->GetTgl())/(1.- track->GetSnp()*track->GetSnp())));
  return chi2;

}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::UpdateMI(AliITStrackMI* track, const AliITSRecPoint* cl,Double_t chi2,Int_t index) const 
{
  //
  //
  //
  Int_t layer = (index & 0xf0000000) >> 28;
  track->SetClIndex(layer, index);
  if (layer>1&&AliITSReconstructor::GetRecoParam()->GetUseAmplitudeInfo(layer)) {
    if (track->GetNormQ(layer)/track->GetExpQ()<0.5 ) {
      chi2+= (0.5-track->GetNormQ(layer)/track->GetExpQ())*10.;
      track->SetdEdxMismatch(track->GetdEdxMismatch()+(0.5-track->GetNormQ(layer)/track->GetExpQ())*10.);
    }
  }

  if (cl->GetQ()<=0) return 0;  // ingore the "virtual" clusters


  // Take into account the mis-alignment (bring track to cluster plane)
  Double_t xTrOrig=track->GetX();
  Float_t clxyz[3]; cl->GetGlobalXYZ(clxyz);Double_t trxyz[3]; track->GetXYZ(trxyz);
  AliDebug(2,Form("gtr %f %f %f",trxyz[0],trxyz[1],trxyz[2]));
  AliDebug(2,Form("gcl %f %f %f",clxyz[0],clxyz[1],clxyz[2]));
  AliDebug(2,Form(" xtr %f  xcl %f",track->GetX(),cl->GetX()));

  if (!track->PropagateTo(xTrOrig+cl->GetX(),0.,0.)) return 0;

  
  AliCluster c(*cl);
  c.SetSigmaY2(track->GetSigmaY(layer)*track->GetSigmaY(layer));
  c.SetSigmaZ2(track->GetSigmaZ(layer)*track->GetSigmaZ(layer));


  Int_t updated = track->UpdateMI(&c,chi2,index);

  // Bring the track back to detector plane in ideal geometry
  if (!track->PropagateTo(xTrOrig,0.,0.)) return 0;

  if(!updated) AliDebug(2,"update failed");
  return updated;
}

//------------------------------------------------------------------------
void AliITStrackerMI::GetDCASigma(AliITStrackMI* track, Float_t & sigmarfi, Float_t &sigmaz)
{
  //
  //DCA sigmas parameterization
  //to be paramterized using external parameters in future 
  //
  // 
  sigmarfi = 0.0040+1.4 *TMath::Abs(track->GetC())+332.*track->GetC()*track->GetC();
  sigmaz   = 0.0110+4.37*TMath::Abs(track->GetC());
}
//------------------------------------------------------------------------
void AliITStrackerMI::SignDeltas( TObjArray *ClusterArray, Float_t vz)
{
  //
  //  
  Int_t entries = ClusterArray->GetEntriesFast();
  if (entries<4) return;
  AliITSRecPoint* cluster = (AliITSRecPoint*)ClusterArray->At(0);
  Int_t layer = cluster->GetLayer();
  if (layer>1) return;
  Int_t index[10000];
  Int_t ncandidates=0;
  Float_t r = (layer>0)? 7:4;
  // 
  for (Int_t i=0;i<entries;i++){
    AliITSRecPoint* cl0 = (AliITSRecPoint*)ClusterArray->At(i);
    Float_t nz = 1+TMath::Abs((cl0->GetZ()-vz)/r);
    if (cl0->GetNy()+cl0->GetNz()<=5+2*layer+nz) continue;
    index[ncandidates] = i;  //candidate to belong to delta electron track
    ncandidates++;
    if (cl0->GetNy()+cl0->GetNz()>9+2*layer+nz) {
      cl0->SetDeltaProbability(1);
    }
  }
  //
  //  
  //
  for (Int_t i=0;i<ncandidates;i++){
    AliITSRecPoint* cl0 = (AliITSRecPoint*)ClusterArray->At(index[i]);
    if (cl0->GetDeltaProbability()>0.8) continue;
    // 
    Int_t ncl = 0;
    Float_t y[100],z[100],sumy,sumz,sumy2, sumyz, sumw;
    sumy=sumz=sumy2=sumyz=sumw=0.0;
    for (Int_t j=0;j<ncandidates;j++){
      if (i==j) continue;
      AliITSRecPoint* cl1 = (AliITSRecPoint*)ClusterArray->At(index[j]);
      //
      Float_t dz = cl0->GetZ()-cl1->GetZ();
      Float_t dy = cl0->GetY()-cl1->GetY();
      if (TMath::Sqrt(dz*dz+dy*dy)<0.2){
	Float_t weight = cl1->GetNy()+cl1->GetNz()-2;
	y[ncl] = cl1->GetY();
	z[ncl] = cl1->GetZ();
	sumy+= y[ncl]*weight;
	sumz+= z[ncl]*weight;
	sumy2+=y[ncl]*y[ncl]*weight;
	sumyz+=y[ncl]*z[ncl]*weight;
	sumw+=weight;
	ncl++;
      }
    }
    if (ncl<4) continue;
    Float_t det = sumw*sumy2  - sumy*sumy;
    Float_t delta=1000;
    if (TMath::Abs(det)>0.01){
      Float_t z0  = (sumy2*sumz - sumy*sumyz)/det;
      Float_t k   = (sumyz*sumw - sumy*sumz)/det;
      delta = TMath::Abs(cl0->GetZ()-(z0+k*cl0->GetY()));
    }
    else{
      Float_t z0  = sumyz/sumy;
      delta = TMath::Abs(cl0->GetZ()-z0);
    }
    if ( delta<0.05) {
      cl0->SetDeltaProbability(1-20.*delta);
    }   
  }
}
//------------------------------------------------------------------------
void AliITStrackerMI::UpdateESDtrack(AliITStrackMI* track, ULong_t flags) const
{
  //
  //
  track->UpdateESDtrack(flags);
  AliITStrackMI * oldtrack = (AliITStrackMI*)(track->GetESDtrack()->GetITStrack());
  if (oldtrack) delete oldtrack; 
  track->GetESDtrack()->SetITStrack(new AliITStrackMI(*track));
  if (TMath::Abs(track->GetDnorm(1))<0.000000001){
    printf("Problem\n");
  }
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::GetNearestLayer(const Double_t *xr) const{
  //
  // Get nearest upper layer close to the point xr.
  // rough approximation 
  //
  const Float_t kRadiuses[6]={4,6.5,15.03,24.,38.5,43.7};
  Float_t radius = TMath::Sqrt(xr[0]*xr[0]+xr[1]*xr[1]);
  Int_t res =6;
  for (Int_t i=0;i<6;i++){
    if (radius<kRadiuses[i]){
      res =i;
      break;
    }
  }
  return res;
}
//------------------------------------------------------------------------
void AliITStrackerMI::UpdateTPCV0(AliESDEvent *event){
  //
  //try to update, or reject TPC  V0s
  //
  Int_t nv0s = event->GetNumberOfV0s();
  Int_t nitstracks = fTrackHypothesys.GetEntriesFast();

  for (Int_t i=0;i<nv0s;i++){
    AliESDv0 * vertex = event->GetV0(i);
    Int_t ip = vertex->GetIndex(0);
    Int_t im = vertex->GetIndex(1);
    //
    TObjArray * arrayp = (ip<nitstracks) ? (TObjArray*)fTrackHypothesys.At(ip):0;
    TObjArray * arraym = (im<nitstracks) ? (TObjArray*)fTrackHypothesys.At(im):0;
    AliITStrackMI * trackp = (arrayp!=0) ? (AliITStrackMI*)arrayp->At(0):0;
    AliITStrackMI * trackm = (arraym!=0) ? (AliITStrackMI*)arraym->At(0):0;
    //
    //
    if (trackp){
      if (trackp->GetNumberOfClusters()+trackp->GetNDeadZone()>5.5){
	if (trackp->GetConstrain()&&trackp->GetChi2MIP(0)<3) vertex->SetStatus(-100);
	if (!trackp->GetConstrain()&&trackp->GetChi2MIP(0)<2) vertex->SetStatus(-100); 
      }
    }

    if (trackm){
      if (trackm->GetNumberOfClusters()+trackm->GetNDeadZone()>5.5){
	if (trackm->GetConstrain()&&trackm->GetChi2MIP(0)<3) vertex->SetStatus(-100);
	if (!trackm->GetConstrain()&&trackm->GetChi2MIP(0)<2) vertex->SetStatus(-100); 
      }
    }
    if (vertex->GetStatus()==-100) continue;
    //
    Double_t xrp[3]; vertex->GetXYZ(xrp[0],xrp[1],xrp[2]);  //I.B.
    Int_t clayer = GetNearestLayer(xrp);                    //I.B.
    vertex->SetNBefore(clayer);        //
    vertex->SetChi2Before(9*clayer);   //
    vertex->SetNAfter(6-clayer);       //
    vertex->SetChi2After(0);           //
    //
    if (clayer >1 ){ // calculate chi2 before vertex
      Float_t chi2p = 0, chi2m=0;  
      //
      if (trackp){
	for (Int_t ilayer=0;ilayer<clayer;ilayer++){
	  if (trackp->GetClIndex(ilayer)>0){
	    chi2p+=trackp->GetDy(ilayer)*trackp->GetDy(ilayer)/(trackp->GetSigmaY(ilayer)*trackp->GetSigmaY(ilayer))+
	      trackp->GetDz(ilayer)*trackp->GetDz(ilayer)/(trackp->GetSigmaZ(ilayer)*trackp->GetSigmaZ(ilayer));
	  }
	  else{
	    chi2p+=9;
	  }
	}
      }else{
	chi2p = 9*clayer;
      }
      //
      if (trackm){
	for (Int_t ilayer=0;ilayer<clayer;ilayer++){
	  if (trackm->GetClIndex(ilayer)>0){
	    chi2m+=trackm->GetDy(ilayer)*trackm->GetDy(ilayer)/(trackm->GetSigmaY(ilayer)*trackm->GetSigmaY(ilayer))+
	      trackm->GetDz(ilayer)*trackm->GetDz(ilayer)/(trackm->GetSigmaZ(ilayer)*trackm->GetSigmaZ(ilayer));
	  }
	  else{
	    chi2m+=9;
	  }
	}
      }else{
	chi2m = 9*clayer;
      }
      vertex->SetChi2Before(TMath::Min(chi2p,chi2m));
      if (TMath::Min(chi2p,chi2m)/Float_t(clayer)<4) vertex->SetStatus(-10);  // track exist before vertex
    }
    
    if (clayer < 5 ){ // calculate chi2 after vertex
      Float_t chi2p = 0, chi2m=0;  
      //
      if (trackp&&TMath::Abs(trackp->GetTgl())<1.){
	for (Int_t ilayer=clayer;ilayer<6;ilayer++){
	  if (trackp->GetClIndex(ilayer)>0){
	    chi2p+=trackp->GetDy(ilayer)*trackp->GetDy(ilayer)/(trackp->GetSigmaY(ilayer)*trackp->GetSigmaY(ilayer))+
	      trackp->GetDz(ilayer)*trackp->GetDz(ilayer)/(trackp->GetSigmaZ(ilayer)*trackp->GetSigmaZ(ilayer));
	  }
	  else{
	    chi2p+=9;
	  }
	}
      }else{
	chi2p = 0;
      }
      //
      if (trackm&&TMath::Abs(trackm->GetTgl())<1.){
	for (Int_t ilayer=clayer;ilayer<6;ilayer++){
	  if (trackm->GetClIndex(ilayer)>0){
	    chi2m+=trackm->GetDy(ilayer)*trackm->GetDy(ilayer)/(trackm->GetSigmaY(ilayer)*trackm->GetSigmaY(ilayer))+
	      trackm->GetDz(ilayer)*trackm->GetDz(ilayer)/(trackm->GetSigmaZ(ilayer)*trackm->GetSigmaZ(ilayer));
	  }
	  else{
	    chi2m+=9;
	  }
	}
      }else{
	chi2m = 0;
      }
      vertex->SetChi2After(TMath::Max(chi2p,chi2m));
      if (TMath::Max(chi2m,chi2p)/Float_t(6-clayer)>9) vertex->SetStatus(-20);  // track not found in ITS
    }
  }
  //
}
//------------------------------------------------------------------------
void AliITStrackerMI::FindV02(AliESDEvent *event)
{
  //
  // V0 finder
  //
  //  Cuts on DCA -  R dependent
  //          max distance DCA between 2 tracks cut 
  //          maxDist = TMath::Min(kMaxDist,kMaxDist0+pvertex->GetRr()*kMaxDist);
  //
  const Float_t kMaxDist0      = 0.1;    
  const Float_t kMaxDist1      = 0.1;     
  const Float_t kMaxDist       = 1;
  const Float_t kMinPointAngle  = 0.85;
  const Float_t kMinPointAngle2 = 0.99;
  const Float_t kMinR           = 0.5;
  const Float_t kMaxR           = 220;
  //const Float_t kCausality0Cut   = 0.19;
  //const Float_t kLikelihood01Cut = 0.25;
  //const Float_t kPointAngleCut   = 0.9996;
  const Float_t kCausality0Cut   = 0.19;
  const Float_t kLikelihood01Cut = 0.45;
  const Float_t kLikelihood1Cut  = 0.5;
  const Float_t kCombinedCut     = 0.55;

  //
  //
  TTreeSRedirector &cstream = *fDebugStreamer;
  Int_t ntracks    = event->GetNumberOfTracks(); 
  Int_t nitstracks = fTrackHypothesys.GetEntriesFast();
  fOriginal.Expand(ntracks);
  fTrackHypothesys.Expand(ntracks);
  fBestHypothesys.Expand(ntracks);
  //
  AliHelix * helixes   = new AliHelix[ntracks+2];
  TObjArray trackarray(ntracks+2);     //array with tracks - with vertex constrain
  TObjArray trackarrayc(ntracks+2);    //array of "best    tracks" - without vertex constrain
  TObjArray trackarrayl(ntracks+2);    //array of "longest tracks" - without vertex constrain
  Bool_t * forbidden   = new Bool_t [ntracks+2];
  Int_t   *itsmap      = new Int_t  [ntracks+2];
  Float_t *dist        = new Float_t[ntracks+2];
  Float_t *normdist0   = new Float_t[ntracks+2];
  Float_t *normdist1   = new Float_t[ntracks+2];
  Float_t *normdist    = new Float_t[ntracks+2];
  Float_t *norm        = new Float_t[ntracks+2];
  Float_t *maxr        = new Float_t[ntracks+2];
  Float_t *minr        = new Float_t[ntracks+2];
  Float_t *minPointAngle= new Float_t[ntracks+2];
  //
  AliV0 *pvertex      = new AliV0;
  AliITStrackMI * dummy= new AliITStrackMI;
  dummy->SetLabel(0);
  AliITStrackMI  trackat0;    //temporary track for DCA calculation
  //
  Float_t primvertex[3]={GetX(),GetY(),GetZ()};
  //
  // make ITS -  ESD map
  //
  for (Int_t itrack=0;itrack<ntracks+2;itrack++) {
    itsmap[itrack]        = -1;
    forbidden[itrack]     = kFALSE;
    maxr[itrack]          = kMaxR;
    minr[itrack]          = kMinR;
    minPointAngle[itrack] = kMinPointAngle;
  }
  for (Int_t itrack=0;itrack<nitstracks;itrack++){
    AliITStrackMI * original =   (AliITStrackMI*)(fOriginal.At(itrack));
    Int_t           esdindex =   original->GetESDtrack()->GetID();
    itsmap[esdindex]         =   itrack;
  }
  //
  // create ITS tracks from ESD tracks if not done before
  //
  for (Int_t itrack=0;itrack<ntracks;itrack++){
    if (itsmap[itrack]>=0) continue;
    AliITStrackMI * tpctrack = new AliITStrackMI(*(event->GetTrack(itrack)));
    //tpctrack->fD[0] = tpctrack->GetD(GetX(),GetY());
    //tpctrack->fD[1] = tpctrack->GetZat(GetX())-GetZ(); 
    tpctrack->GetDZ(GetX(),GetY(),GetZ(),tpctrack->GetDP());   //I.B.
    if (tpctrack->GetD(0)<20 && tpctrack->GetD(1)<20){
      // tracks which can reach inner part of ITS
      // propagate track to outer its volume - with correction for material
      CorrectForTPCtoITSDeadZoneMaterial(tpctrack);  
    }
    itsmap[itrack] = nitstracks;
    fOriginal.AddAt(tpctrack,nitstracks);
    nitstracks++;
  }
  //
  // fill temporary arrays
  //
  for (Int_t itrack=0;itrack<ntracks;itrack++){
    AliESDtrack *  esdtrack = event->GetTrack(itrack);
    Int_t          itsindex = itsmap[itrack];
    AliITStrackMI *original = (AliITStrackMI*)fOriginal.At(itsmap[itrack]);
    if (!original) continue;
    AliITStrackMI *bestConst  = 0;
    AliITStrackMI *bestLong   = 0;
    AliITStrackMI *best       = 0;    
    //
    //
    TObjArray * array    = (TObjArray*)  fTrackHypothesys.At(itsindex);
    Int_t       hentries = (array==0) ?  0 : array->GetEntriesFast();
    // Get best track with vertex constrain
    for (Int_t ih=0;ih<hentries;ih++){
      AliITStrackMI * trackh = (AliITStrackMI*)array->At(ih);
      if (!trackh->GetConstrain()) continue;
      if (!bestConst) bestConst = trackh;
      if (trackh->GetNumberOfClusters()>5.0){
	bestConst  = trackh;                         // full track -  with minimal chi2
	break;
      }
      if (trackh->GetNumberOfClusters()+trackh->GetNDeadZone()<=bestConst->GetNumberOfClusters()+bestConst->GetNDeadZone())  continue;      
      bestConst = trackh;
      break;
    }
    // Get best long track without vertex constrain and best track without vertex constrain
    for (Int_t ih=0;ih<hentries;ih++){
      AliITStrackMI * trackh = (AliITStrackMI*)array->At(ih);
      if (trackh->GetConstrain()) continue;
      if (!best)     best     = trackh;
      if (!bestLong) bestLong = trackh;
      if (trackh->GetNumberOfClusters()>5.0){
	bestLong  = trackh;                         // full track -  with minimal chi2
	break;
      }
      if (trackh->GetNumberOfClusters()+trackh->GetNDeadZone()<=bestLong->GetNumberOfClusters()+bestLong->GetNDeadZone())  continue;      
      bestLong = trackh;	
    }
    if (!best) {
      best     = original;
      bestLong = original;
    }
    //I.B. trackat0 = *bestLong;
    new (&trackat0) AliITStrackMI(*bestLong);
    Double_t xx,yy,zz,alpha; 
    if (!bestLong->GetGlobalXYZat(bestLong->GetX(),xx,yy,zz)) continue;
    alpha = TMath::ATan2(yy,xx);    
    if (!trackat0.Propagate(alpha,0)) continue;      
    // calculate normalized distances to the vertex 
    //
    Float_t ptfac  = (1.+100.*TMath::Abs(trackat0.GetC()));
    if ( bestLong->GetNumberOfClusters()>3 ){      
      dist[itsindex]      = trackat0.GetY();
      norm[itsindex]      = ptfac*TMath::Sqrt(trackat0.GetSigmaY2());
      normdist0[itsindex] = TMath::Abs(trackat0.GetY()/norm[itsindex]);
      normdist1[itsindex] = TMath::Abs((trackat0.GetZ()-primvertex[2])/(ptfac*TMath::Sqrt(trackat0.GetSigmaZ2())));
      normdist[itsindex]  = TMath::Sqrt(normdist0[itsindex]*normdist0[itsindex]+normdist1[itsindex]*normdist1[itsindex]);
      if (!bestConst){
	if (bestLong->GetNumberOfClusters()+bestLong->GetNDeadZone()<6) normdist[itsindex]*=2.;
	if (bestLong->GetNumberOfClusters()+bestLong->GetNDeadZone()<5) normdist[itsindex]*=2.;
	if (bestLong->GetNumberOfClusters()+bestLong->GetNDeadZone()<4) normdist[itsindex]*=2.;
      }else{
	if (bestConst->GetNumberOfClusters()+bestConst->GetNDeadZone()<6) normdist[itsindex]*=1.5;
	if (bestConst->GetNumberOfClusters()+bestConst->GetNDeadZone()<5) normdist[itsindex]*=1.5;
      }
    }
    else{      
      if (bestConst&&bestConst->GetNumberOfClusters()+bestConst->GetNDeadZone()>4.5){
	dist[itsindex] = bestConst->GetD(0);
	norm[itsindex] = bestConst->GetDnorm(0);
	normdist0[itsindex] = TMath::Abs(bestConst->GetD(0)/norm[itsindex]);
	normdist1[itsindex] = TMath::Abs(bestConst->GetD(0)/norm[itsindex]);
	normdist[itsindex]  = TMath::Sqrt(normdist0[itsindex]*normdist0[itsindex]+normdist1[itsindex]*normdist1[itsindex]);
      }else{
	dist[itsindex]      = trackat0.GetY();
	norm[itsindex]      = ptfac*TMath::Sqrt(trackat0.GetSigmaY2());
	normdist0[itsindex] = TMath::Abs(trackat0.GetY()/norm[itsindex]);
	normdist1[itsindex] = TMath::Abs((trackat0.GetZ()-primvertex[2])/(ptfac*TMath::Sqrt(trackat0.GetSigmaZ2())));
	normdist[itsindex]  = TMath::Sqrt(normdist0[itsindex]*normdist0[itsindex]+normdist1[itsindex]*normdist1[itsindex]);
	if (TMath::Abs(trackat0.GetTgl())>1.05){
	  if (normdist[itsindex]<3) forbidden[itsindex]=kTRUE;
	  if (normdist[itsindex]>3) {
	    minr[itsindex] = TMath::Max(Float_t(40.),minr[itsindex]);
	  }
	}
      }
    }
    //
    //-----------------------------------------------------------
    //Forbid primary track candidates - 
    //
    //treetr->SetAlias("forbidden0","Tr0.fN<4&&Tr1.fN+Tr1.fNDeadZone>4.5");
    //treetr->SetAlias("forbidden1","ND<3&&Tr1.fN+Tr1.fNDeadZone>5.5");
    //treetr->SetAlias("forbidden2","ND<2&&Tr1.fClIndex[0]>0&&Tr1.fClIndex[0]>0");
    //treetr->SetAlias("forbidden3","ND<1&&Tr1.fClIndex[0]>0");
    //treetr->SetAlias("forbidden4","ND<4&&Tr1.fNormChi2[0]<2");
    //treetr->SetAlias("forbidden5","ND<5&&Tr1.fNormChi2[0]<1");
    //-----------------------------------------------------------
    if (bestConst){
      if (bestLong->GetNumberOfClusters()<4       && bestConst->GetNumberOfClusters()+bestConst->GetNDeadZone()>4.5)               forbidden[itsindex]=kTRUE;
      if (normdist[itsindex]<3 && bestConst->GetNumberOfClusters()+bestConst->GetNDeadZone()>5.5)               forbidden[itsindex]=kTRUE;
      if (normdist[itsindex]<2 && bestConst->GetClIndex(0)>0 && bestConst->GetClIndex(1)>0 ) forbidden[itsindex]=kTRUE;
      if (normdist[itsindex]<1 && bestConst->GetClIndex(0)>0)                              forbidden[itsindex]=kTRUE;
      if (normdist[itsindex]<4 && bestConst->GetNormChi2(0)<2)                             forbidden[itsindex]=kTRUE;
      if (normdist[itsindex]<5 && bestConst->GetNormChi2(0)<1)                             forbidden[itsindex]=kTRUE;      
      if (bestConst->GetNormChi2(0)<2.5) {
	minPointAngle[itsindex]= 0.9999;
	maxr[itsindex]         = 10;
      }
    }
    //
    //forbid daughter kink candidates
    //
    if (esdtrack->GetKinkIndex(0)>0) forbidden[itsindex] = kTRUE;
    Bool_t isElectron = kTRUE;
    Bool_t isProton   = kTRUE;
    Double_t pid[5];
    esdtrack->GetESDpid(pid);
    for (Int_t i=1;i<5;i++){
      if (pid[0]<pid[i]) isElectron= kFALSE;
      if (pid[4]<pid[i]) isProton= kFALSE;
    }
    if (isElectron){
      forbidden[itsindex]=kFALSE;	
      normdist[itsindex]*=-1;
    }
    if (isProton){
      if (normdist[itsindex]>2) forbidden[itsindex]=kFALSE;	
      normdist[itsindex]*=-1;
    }

    //
    // Causality cuts in TPC volume
    //
    if (esdtrack->GetTPCdensity(0,10) >0.6)  maxr[itsindex] = TMath::Min(Float_t(110),maxr[itsindex]);
    if (esdtrack->GetTPCdensity(10,30)>0.6)  maxr[itsindex] = TMath::Min(Float_t(120),maxr[itsindex]);
    if (esdtrack->GetTPCdensity(20,40)>0.6)  maxr[itsindex] = TMath::Min(Float_t(130),maxr[itsindex]);
    if (esdtrack->GetTPCdensity(30,50)>0.6)  maxr[itsindex] = TMath::Min(Float_t(140),maxr[itsindex]);
    //
    if (esdtrack->GetTPCdensity(0,60)<0.4&&bestLong->GetNumberOfClusters()<3) minr[itsindex]=100;    
    //
    //
    if (kFALSE){
      cstream<<"Track"<<
	"Tr0.="<<best<<
	"Tr1.="<<((bestConst)? bestConst:dummy)<<
	"Tr2.="<<bestLong<<
	"Tr3.="<<&trackat0<<
	"Esd.="<<esdtrack<<
	"Dist="<<dist[itsindex]<<
	"ND0="<<normdist0[itsindex]<<
	"ND1="<<normdist1[itsindex]<<
	"ND="<<normdist[itsindex]<<
	"Pz="<<primvertex[2]<<
	"Forbid="<<forbidden[itsindex]<<
	"\n";
      //
    }
    trackarray.AddAt(best,itsindex);
    trackarrayc.AddAt(bestConst,itsindex);
    trackarrayl.AddAt(bestLong,itsindex);
    new (&helixes[itsindex]) AliHelix(*best);
  }
  //
  //
  //
  // first iterration of V0 finder
  //
  for (Int_t iesd0=0;iesd0<ntracks;iesd0++){
    Int_t itrack0 = itsmap[iesd0];
    if (forbidden[itrack0]) continue;
    AliITStrackMI * btrack0 = (AliITStrackMI*)trackarray.At(itrack0);
    if (!btrack0) continue;    
    if (btrack0->GetSign()>0) continue;
    AliITStrackMI *trackc0 = (AliITStrackMI*)trackarrayc.At(itrack0);
    //
    for (Int_t iesd1=0;iesd1<ntracks;iesd1++){
      Int_t itrack1 = itsmap[iesd1];
      if (forbidden[itrack1]) continue;

      AliITStrackMI * btrack1 = (AliITStrackMI*)trackarray.At(itrack1); 
      if (!btrack1) continue;
      if (btrack1->GetSign()<0) continue;
      Bool_t isGold = kFALSE;
      if (TMath::Abs(TMath::Abs(btrack0->GetLabel())-TMath::Abs(btrack1->GetLabel()))==1){
	isGold = kTRUE;
      }
      AliITStrackMI *trackc1 = (AliITStrackMI*)trackarrayc.At(itrack1);
      AliHelix &h1 = helixes[itrack0];
      AliHelix &h2 = helixes[itrack1];
      //
      // find linear distance
      Double_t rmin =0;
      //
      //
      //
      Double_t phase[2][2],radius[2];
      Int_t  points = h1.GetRPHIintersections(h2, phase, radius);
      if    (points==0)  continue;
      Double_t delta[2]={1000000,1000000};        
      rmin = radius[0];
      h1.ParabolicDCA(h2,phase[0][0],phase[0][1],radius[0],delta[0]);
      if (points==2){    
	if (radius[1]<rmin) rmin = radius[1];
	h1.ParabolicDCA(h2,phase[1][0],phase[1][1],radius[1],delta[1]);
      }
      rmin = TMath::Sqrt(rmin);
      Double_t distance = 0;
      Double_t radiusC  = 0;
      Int_t    iphase   = 0;
      if (points==1 || delta[0]<delta[1]){
	distance = TMath::Sqrt(delta[0]);
	radiusC  = TMath::Sqrt(radius[0]);
      }else{
	distance = TMath::Sqrt(delta[1]);
	radiusC  = TMath::Sqrt(radius[1]);
	iphase=1;
      }
      if (radiusC<TMath::Max(minr[itrack0],minr[itrack1]))    continue;
      if (radiusC>TMath::Min(maxr[itrack0],maxr[itrack1]))     continue; 
      Float_t maxDist  = TMath::Min(kMaxDist,Float_t(kMaxDist0+radiusC*kMaxDist1));      
      if (distance>maxDist) continue;
      Float_t pointAngle = h1.GetPointAngle(h2,phase[iphase],primvertex);
      if (pointAngle<TMath::Max(minPointAngle[itrack0],minPointAngle[itrack1])) continue;
      //
      //
      //      Double_t distance = TestV0(h1,h2,pvertex,rmin);
      //
      //       if (distance>maxDist)           continue;
      //       if (pvertex->GetRr()<kMinR)     continue;
      //       if (pvertex->GetRr()>kMaxR)     continue;
      AliITStrackMI * track0=btrack0;
      AliITStrackMI * track1=btrack1;
      //      if (pvertex->GetRr()<3.5){  
      if (radiusC<3.5){  
	//use longest tracks inside the pipe
	track0 = (AliITStrackMI*)trackarrayl.At(itrack0);
	track1 = (AliITStrackMI*)trackarrayl.At(itrack1);
      }      
      //
      //
      pvertex->SetParamN(*track0);
      pvertex->SetParamP(*track1);
      pvertex->Update(primvertex);
      pvertex->SetClusters(track0->ClIndex(),track1->ClIndex());  // register clusters

      if (pvertex->GetRr()<kMinR) continue;
      if (pvertex->GetRr()>kMaxR) continue;
      if (pvertex->GetV0CosineOfPointingAngle()<kMinPointAngle) continue;
//Bo:      if (pvertex->GetDist2()>maxDist) continue;
      if (pvertex->GetDcaV0Daughters()>maxDist) continue;
//Bo:        pvertex->SetLab(0,track0->GetLabel());
//Bo:        pvertex->SetLab(1,track1->GetLabel());
      pvertex->SetIndex(0,track0->GetESDtrack()->GetID());
      pvertex->SetIndex(1,track1->GetESDtrack()->GetID());
      //      
      AliITStrackMI * htrackc0 = trackc0 ? trackc0:dummy;      
      AliITStrackMI * htrackc1 = trackc1 ? trackc1:dummy;

      //
      //
      TObjArray * array0b     = (TObjArray*)fBestHypothesys.At(itrack0);
      if (!array0b&&pvertex->GetRr()<40 && TMath::Abs(track0->GetTgl())<1.1) {
	fCurrentEsdTrack = itrack0;
	FollowProlongationTree((AliITStrackMI*)fOriginal.At(itrack0),itrack0, kFALSE);
      }
      TObjArray * array1b    = (TObjArray*)fBestHypothesys.At(itrack1);
      if (!array1b&&pvertex->GetRr()<40 && TMath::Abs(track1->GetTgl())<1.1) { 
	fCurrentEsdTrack = itrack1;
	FollowProlongationTree((AliITStrackMI*)fOriginal.At(itrack1),itrack1, kFALSE);
      }
      //
      AliITStrackMI * track0b = (AliITStrackMI*)fOriginal.At(itrack0);       
      AliITStrackMI * track1b = (AliITStrackMI*)fOriginal.At(itrack1);
      AliITStrackMI * track0l = (AliITStrackMI*)fOriginal.At(itrack0);       
      AliITStrackMI * track1l = (AliITStrackMI*)fOriginal.At(itrack1);
      
      Float_t minchi2before0=16;
      Float_t minchi2before1=16;
      Float_t minchi2after0 =16;
      Float_t minchi2after1 =16;
      Double_t xrp[3]; pvertex->GetXYZ(xrp[0],xrp[1],xrp[2]);  //I.B.
      Int_t maxLayer = GetNearestLayer(xrp);                   //I.B.
      
      if (array0b) for (Int_t i=0;i<5;i++){
	// best track after vertex
	AliITStrackMI * btrack = (AliITStrackMI*)array0b->At(i);
	if (!btrack) continue;
	if (btrack->GetNumberOfClusters()>track0l->GetNumberOfClusters()) track0l = btrack;     
	//	if (btrack->fX<pvertex->GetRr()-2.-0.5/(0.1+pvertex->GetAnglep()[2])) {
	if (btrack->GetX()<pvertex->GetRr()-2.) {
	  if ( (maxLayer>i+2|| (i==0)) && btrack->GetNumberOfClusters()==(6-i)&&i<3){
	    Float_t sumchi2= 0;
	    Float_t sumn   = 0;
	    if (maxLayer<3){   // take prim vertex as additional measurement
	      if (normdist[itrack0]>0 && htrackc0){
		sumchi2 += TMath::Min((3.-maxLayer)*normdist[itrack0]*normdist[itrack0],16.);
	      }else{
		sumchi2 +=  TMath::Min((3.-maxLayer)*(3*normdist[itrack0]*normdist[itrack0]+3.),16.);
	      }
	      sumn    +=  3-maxLayer;
	    }
	    for (Int_t ilayer=i;ilayer<maxLayer;ilayer++){
	      sumn+=1.;	      
	      if (!btrack->GetClIndex(ilayer)){
		sumchi2+=25;
		continue;
	      }else{
		Int_t c=( btrack->GetClIndex(ilayer) & 0x0fffffff);
		for (Int_t itrack=0;itrack<4;itrack++){
		  if (fgLayers[ilayer].GetClusterTracks(itrack,c)>=0 && fgLayers[ilayer].GetClusterTracks(itrack,c)!=itrack0){
		    sumchi2+=18.;  //shared cluster
		    break;
		  }
		}
		sumchi2+=btrack->GetDy(ilayer)*btrack->GetDy(ilayer)/(btrack->GetSigmaY(ilayer)*btrack->GetSigmaY(ilayer));
		sumchi2+=btrack->GetDz(ilayer)*btrack->GetDz(ilayer)/(btrack->GetSigmaZ(ilayer)*btrack->GetSigmaZ(ilayer));	       
	      }
	    }
	    sumchi2/=sumn;
	    if (sumchi2<minchi2before0) minchi2before0=sumchi2; 
	  }
	  continue;   //safety space - Geo manager will give exact layer
	}
	track0b       = btrack;
	minchi2after0 = btrack->GetNormChi2(i);
	break;
      }
      if (array1b) for (Int_t i=0;i<5;i++){
	// best track after vertex
	AliITStrackMI * btrack = (AliITStrackMI*)array1b->At(i);
	if (!btrack) continue;
	if (btrack->GetNumberOfClusters()>track1l->GetNumberOfClusters()) track1l = btrack;     
	//	if (btrack->fX<pvertex->GetRr()-2-0.5/(0.1+pvertex->GetAnglep()[2])){
	if (btrack->GetX()<pvertex->GetRr()-2){
	  if ((maxLayer>i+2 || (i==0))&&btrack->GetNumberOfClusters()==(6-i)&&(i<3)){
	    Float_t sumchi2= 0;
	    Float_t sumn   = 0;
	    if (maxLayer<3){   // take prim vertex as additional measurement
	      if (normdist[itrack1]>0 && htrackc1){
		sumchi2 +=  TMath::Min((3.-maxLayer)*normdist[itrack1]*normdist[itrack1],16.);
	      }else{
		sumchi2 += TMath::Min((3.-maxLayer)*(3*normdist[itrack1]*normdist[itrack1]+3.),16.);
	      }
	      sumn    +=  3-maxLayer;
	    }
	    for (Int_t ilayer=i;ilayer<maxLayer;ilayer++){
	      sumn+=1.;
	      if (!btrack->GetClIndex(ilayer)){
		sumchi2+=30;
		continue;
	      }else{
		Int_t c=( btrack->GetClIndex(ilayer) & 0x0fffffff);
		for (Int_t itrack=0;itrack<4;itrack++){
		  if (fgLayers[ilayer].GetClusterTracks(itrack,c)>=0 && fgLayers[ilayer].GetClusterTracks(itrack,c)!=itrack1){
		    sumchi2+=18.;  //shared cluster
		    break;
		  }
		}
		sumchi2+=btrack->GetDy(ilayer)*btrack->GetDy(ilayer)/(btrack->GetSigmaY(ilayer)*btrack->GetSigmaY(ilayer));
		sumchi2+=btrack->GetDz(ilayer)*btrack->GetDz(ilayer)/(btrack->GetSigmaZ(ilayer)*btrack->GetSigmaZ(ilayer));	       
	      }
	    }
	    sumchi2/=sumn;
	    if (sumchi2<minchi2before1) minchi2before1=sumchi2; 
	  }
	  continue;   //safety space - Geo manager will give exact layer	   
	}
	track1b = btrack;
	minchi2after1 = btrack->GetNormChi2(i);
	break;
      }
      //
      // position resolution - used for DCA cut
      Float_t sigmad = track0b->GetSigmaY2()+track0b->GetSigmaZ2()+track1b->GetSigmaY2()+track1b->GetSigmaZ2()+
	(track0b->GetX()-pvertex->GetRr())*(track0b->GetX()-pvertex->GetRr())*(track0b->GetSigmaSnp2()+track0b->GetSigmaTgl2())+
	(track1b->GetX()-pvertex->GetRr())*(track1b->GetX()-pvertex->GetRr())*(track1b->GetSigmaSnp2()+track1b->GetSigmaTgl2());
      sigmad =TMath::Sqrt(sigmad)+0.04;
      if (pvertex->GetRr()>50){
	Double_t cov0[15],cov1[15];
	track0b->GetESDtrack()->GetInnerExternalCovariance(cov0);
	track1b->GetESDtrack()->GetInnerExternalCovariance(cov1);
	sigmad = cov0[0]+cov0[2]+cov1[0]+cov1[2]+
	  (80.-pvertex->GetRr())*(80.-pvertex->GetRr())*(cov0[5]+cov0[9])+
	  (80.-pvertex->GetRr())*(80.-pvertex->GetRr())*(cov1[5]+cov1[9]);
	sigmad =TMath::Sqrt(sigmad)+0.05;
      }
      //       
      AliV0 vertex2;
      vertex2.SetParamN(*track0b);
      vertex2.SetParamP(*track1b);
      vertex2.Update(primvertex);
      //Bo:      if (vertex2.GetDist2()<=pvertex->GetDist2()&&(vertex2.GetV0CosineOfPointingAngle()>=pvertex->GetV0CosineOfPointingAngle())){
      if (vertex2.GetDcaV0Daughters()<=pvertex->GetDcaV0Daughters()&&(vertex2.GetV0CosineOfPointingAngle()>=pvertex->GetV0CosineOfPointingAngle())){
	pvertex->SetParamN(*track0b);
	pvertex->SetParamP(*track1b);
	pvertex->Update(primvertex);
	pvertex->SetClusters(track0b->ClIndex(),track1b->ClIndex());  // register clusters
	pvertex->SetIndex(0,track0->GetESDtrack()->GetID());
	pvertex->SetIndex(1,track1->GetESDtrack()->GetID());
      }
      pvertex->SetDistSigma(sigmad);
      //Bo:      pvertex->SetDistNorm(pvertex->GetDist2()/sigmad);       
      pvertex->SetNormDCAPrim(normdist[itrack0],normdist[itrack1]);
      //
      // define likelihhod and causalities
      //
      Float_t pa0=1, pa1=1, pb0=0.26, pb1=0.26;      
      if (maxLayer<1){
	Float_t fnorm0 = normdist[itrack0];
	if (fnorm0<0) fnorm0*=-3;
	Float_t fnorm1 = normdist[itrack1];
	if (fnorm1<0) fnorm1*=-3;
 	if ((pvertex->GetAnglep()[2]>0.1) || ( (pvertex->GetRr()<10.5)&& pvertex->GetAnglep()[2]>0.05 ) || (pvertex->GetRr()<3)){
 	  pb0    =  TMath::Exp(-TMath::Min(fnorm0,Float_t(16.))/12.);
 	  pb1    =  TMath::Exp(-TMath::Min(fnorm1,Float_t(16.))/12.);
 	}
	pvertex->SetChi2Before(normdist[itrack0]);
	pvertex->SetChi2After(normdist[itrack1]);       
	pvertex->SetNAfter(0);
	pvertex->SetNBefore(0);
      }else{
	pvertex->SetChi2Before(minchi2before0);
	pvertex->SetChi2After(minchi2before1);
	 if (pvertex->GetAnglep()[2]>0.1 || ( pvertex->GetRr()<10.5 && pvertex->GetAnglep()[2]>0.05) || pvertex->GetRr()<3){
	   pb0    =  TMath::Exp(-TMath::Min(minchi2before0,Float_t(16))/12.);
	   pb1    =  TMath::Exp(-TMath::Min(minchi2before1,Float_t(16))/12.);
	 }
	 pvertex->SetNAfter(maxLayer);
	 pvertex->SetNBefore(maxLayer);      
      }
      if (pvertex->GetRr()<90){
	pa0  *= TMath::Min(track0->GetESDtrack()->GetTPCdensity(0,60),Double_t(1.));
	pa1  *= TMath::Min(track1->GetESDtrack()->GetTPCdensity(0,60),Double_t(1.));
      }
      if (pvertex->GetRr()<20){
	pa0  *= (0.2+TMath::Exp(-TMath::Min(minchi2after0,Float_t(16))/8.))/1.2;
	pa1  *= (0.2+TMath::Exp(-TMath::Min(minchi2after1,Float_t(16))/8.))/1.2;
      }
      //
      pvertex->SetCausality(pb0,pb1,pa0,pa1);
      //
      //  Likelihood calculations  - apply cuts
      //         
      Bool_t v0OK = kTRUE;
      Float_t p12 = pvertex->GetParamP()->GetParameter()[4]*pvertex->GetParamP()->GetParameter()[4];
      p12        += pvertex->GetParamN()->GetParameter()[4]*pvertex->GetParamN()->GetParameter()[4];
      p12         = TMath::Sqrt(p12);                             // "mean" momenta
      Float_t    sigmap0   = 0.0001+0.001/(0.1+pvertex->GetRr()); 
      Float_t    sigmap    = 0.5*sigmap0*(0.6+0.4*p12);           // "resolution: of point angle - as a function of radius and momenta

      Float_t causalityA  = (1.0-pvertex->GetCausalityP()[0])*(1.0-pvertex->GetCausalityP()[1]);
      Float_t causalityB  = TMath::Sqrt(TMath::Min(pvertex->GetCausalityP()[2],Double_t(0.7))*
					TMath::Min(pvertex->GetCausalityP()[3],Double_t(0.7)));
      //
      //Bo:      Float_t likelihood0 = (TMath::Exp(-pvertex->GetDistNorm())+0.1) *(pvertex->GetDist2()<0.5)*(pvertex->GetDistNorm()<5);
      Float_t lDistNorm = pvertex->GetDcaV0Daughters()/pvertex->GetDistSigma();
      Float_t likelihood0 = (TMath::Exp(-lDistNorm)+0.1) *(pvertex->GetDcaV0Daughters()<0.5)*(lDistNorm<5);

      Float_t likelihood1 = TMath::Exp(-(1.0001-pvertex->GetV0CosineOfPointingAngle())/sigmap)+
	0.4*TMath::Exp(-(1.0001-pvertex->GetV0CosineOfPointingAngle())/(4.*sigmap))+
	0.4*TMath::Exp(-(1.0001-pvertex->GetV0CosineOfPointingAngle())/(8.*sigmap))+
	0.1*TMath::Exp(-(1.0001-pvertex->GetV0CosineOfPointingAngle())/0.01);
      //
      if (causalityA<kCausality0Cut)                                          v0OK = kFALSE;
      if (TMath::Sqrt(likelihood0*likelihood1)<kLikelihood01Cut)              v0OK = kFALSE;
      if (likelihood1<kLikelihood1Cut)                                        v0OK = kFALSE;
      if (TMath::Power(likelihood0*likelihood1*causalityB,0.33)<kCombinedCut) v0OK = kFALSE;
      
      //
      //
      if (kFALSE){	
	Bool_t gold = TMath::Abs(TMath::Abs(track0->GetLabel())-TMath::Abs(track1->GetLabel()))==1;
	cstream<<"It0"<<
	  "Tr0.="<<track0<<                       //best without constrain
	  "Tr1.="<<track1<<                       //best without constrain  
	  "Tr0B.="<<track0b<<                     //best without constrain  after vertex
	  "Tr1B.="<<track1b<<                     //best without constrain  after vertex 
	  "Tr0C.="<<htrackc0<<                    //best with constrain     if exist
	  "Tr1C.="<<htrackc1<<                    //best with constrain     if exist
	  "Tr0L.="<<track0l<<                     //longest best           
	  "Tr1L.="<<track1l<<                     //longest best
	  "Esd0.="<<track0->GetESDtrack()<<           // esd track0 params
	  "Esd1.="<<track1->GetESDtrack()<<           // esd track1 params
	  "V0.="<<pvertex<<                       //vertex properties
	  "V0b.="<<&vertex2<<                       //vertex properties at "best" track
	  "ND0="<<normdist[itrack0]<<             //normalize distance for track0
	  "ND1="<<normdist[itrack1]<<             //normalize distance for track1
	  "Gold="<<gold<<                         //
	  //	  "RejectBase="<<rejectBase<<             //rejection in First itteration
	  "OK="<<v0OK<<
	  "rmin="<<rmin<<
	  "sigmad="<<sigmad<<
	  "\n";
      }      
      //if (rejectBase) continue;
      //
      pvertex->SetStatus(0);
      //      if (rejectBase) {
      //	pvertex->SetStatus(-100);
      //}
      if (pvertex->GetV0CosineOfPointingAngle()>kMinPointAngle2) {
	//Bo:	  pvertex->SetESDindexes(track0->GetESDtrack()->GetID(),track1->GetESDtrack()->GetID());
	pvertex->SetIndex(0,track0->GetESDtrack()->GetID());//Bo: consistency 0 for neg
	pvertex->SetIndex(1,track1->GetESDtrack()->GetID());//Bo: consistency 1 for pos
	if (v0OK){
	  //	  AliV0vertex vertexjuri(*track0,*track1);
	  //	  vertexjuri.SetESDindexes(track0->fESDtrack->GetID(),track1->fESDtrack->GetID());
	  //	  event->AddV0(&vertexjuri);
	  pvertex->SetStatus(100);
	}
        pvertex->SetOnFlyStatus(kTRUE);
        pvertex->ChangeMassHypothesis(kK0Short);
	event->AddV0(pvertex);
      }
    }
  }
  //
  //
  // delete temporary arrays
  //  
  delete[] forbidden;
  delete[] minPointAngle;
  delete[] maxr;
  delete[] minr;
  delete[] norm;
  delete[] normdist;
  delete[] normdist1;
  delete[] normdist0;
  delete[] dist;
  delete[] itsmap;
  delete[] helixes;
  delete   pvertex;
}
//------------------------------------------------------------------------
void AliITStrackerMI::RefitV02(AliESDEvent *event)
{
  //
  //try to refit  V0s in the third path of the reconstruction
  //
  TTreeSRedirector &cstream = *fDebugStreamer;
  //
  Int_t  nv0s = event->GetNumberOfV0s();
  Float_t primvertex[3]={GetX(),GetY(),GetZ()};
  AliV0 v0temp;
  for (Int_t iv0 = 0; iv0<nv0s;iv0++){
    AliV0 * v0mi = (AliV0*)event->GetV0(iv0);
    if (!v0mi) continue;
    Int_t     itrack0   = v0mi->GetIndex(0);
    Int_t     itrack1   = v0mi->GetIndex(1);
    AliESDtrack *esd0   = event->GetTrack(itrack0);
    AliESDtrack *esd1   = event->GetTrack(itrack1);
    if (!esd0||!esd1) continue;
    AliITStrackMI tpc0(*esd0);  
    AliITStrackMI tpc1(*esd1);
    Double_t x,y,z; v0mi->GetXYZ(x,y,z); //I.B. 
    Double_t alpha =TMath::ATan2(y,x);   //I.B.
    if (v0mi->GetRr()>85){
      if (tpc0.Propagate(alpha,v0mi->GetRr())&&tpc1.Propagate(alpha,v0mi->GetRr())){
	v0temp.SetParamN(tpc0);
	v0temp.SetParamP(tpc1);
	v0temp.Update(primvertex);
	if (kFALSE) cstream<<"Refit"<<
	  "V0.="<<v0mi<<
	  "V0refit.="<<&v0temp<<
	  "Tr0.="<<&tpc0<<
	  "Tr1.="<<&tpc1<<
	  "\n";
	//Bo:	if (v0temp.GetDist2()<v0mi->GetDist2() || v0temp.GetV0CosineOfPointingAngle()>v0mi->GetV0CosineOfPointingAngle()){
	if (v0temp.GetDcaV0Daughters()<v0mi->GetDcaV0Daughters() || v0temp.GetV0CosineOfPointingAngle()>v0mi->GetV0CosineOfPointingAngle()){
	  v0mi->SetParamN(tpc0);
	  v0mi->SetParamP(tpc1);
	  v0mi->Update(primvertex);
	}
      }
      continue;
    }
    if (v0mi->GetRr()>35){
      CorrectForTPCtoITSDeadZoneMaterial(&tpc0);
      CorrectForTPCtoITSDeadZoneMaterial(&tpc1);
      if (tpc0.Propagate(alpha,v0mi->GetRr())&&tpc1.Propagate(alpha,v0mi->GetRr())){
	v0temp.SetParamN(tpc0);
	v0temp.SetParamP(tpc1);
	v0temp.Update(primvertex);
	if (kFALSE) cstream<<"Refit"<<
	  "V0.="<<v0mi<<
	  "V0refit.="<<&v0temp<<
	  "Tr0.="<<&tpc0<<
	  "Tr1.="<<&tpc1<<
	  "\n";
	//Bo:	if (v0temp.GetDist2()<v0mi->GetDist2() || v0temp.GetV0CosineOfPointingAngle()>v0mi->GetV0CosineOfPointingAngle()){
	if (v0temp.GetDcaV0Daughters()<v0mi->GetDcaV0Daughters() || v0temp.GetV0CosineOfPointingAngle()>v0mi->GetV0CosineOfPointingAngle()){
	  v0mi->SetParamN(tpc0);
	  v0mi->SetParamP(tpc1);
	  v0mi->Update(primvertex);
	}	
      }
      continue;
    }
    CorrectForTPCtoITSDeadZoneMaterial(&tpc0);
    CorrectForTPCtoITSDeadZoneMaterial(&tpc1);    
    //    if (tpc0.Propagate(alpha,v0mi->GetRr())&&tpc1.Propagate(alpha,v0mi->GetRr())){
    if (RefitAt(v0mi->GetRr(),&tpc0, v0mi->GetClusters(0)) && RefitAt(v0mi->GetRr(),&tpc1, v0mi->GetClusters(1))){
      v0temp.SetParamN(tpc0);
      v0temp.SetParamP(tpc1);
      v0temp.Update(primvertex);
      if (kFALSE) cstream<<"Refit"<<
	"V0.="<<v0mi<<
	"V0refit.="<<&v0temp<<
	"Tr0.="<<&tpc0<<
	"Tr1.="<<&tpc1<<
	"\n";
      //Bo:      if (v0temp.GetDist2()<v0mi->GetDist2() || v0temp.GetV0CosineOfPointingAngle()>v0mi->GetV0CosineOfPointingAngle()){
      if (v0temp.GetDcaV0Daughters()<v0mi->GetDcaV0Daughters() || v0temp.GetV0CosineOfPointingAngle()>v0mi->GetV0CosineOfPointingAngle()){
	v0mi->SetParamN(tpc0);
	v0mi->SetParamP(tpc1);
	v0mi->Update(primvertex);
      }	
    }    
  }
}
//------------------------------------------------------------------------
void AliITStrackerMI::BuildMaterialLUT(TString material) {
  //--------------------------------------------------------------------
  // Fill a look-up table with mean material
  //--------------------------------------------------------------------

  Int_t n=1000;
  Double_t mparam[7];
  Double_t point1[3],point2[3];
  Double_t phi,cosphi,sinphi,z;
  // 0-5 layers, 6 pipe, 7-8 shields 
  Double_t rmin[9]={ 3.5, 5.5,13.0,22.0,35.0,41.0, 2.0, 8.0,25.0};
  Double_t rmax[9]={ 5.5, 8.0,17.0,26.0,41.0,47.0, 3.0,10.5,30.0};

  Int_t ifirst=0,ilast=0;  
  if(material.Contains("Pipe")) {
    ifirst=6; ilast=6;
  } else if(material.Contains("Shields")) {
    ifirst=7; ilast=8;
  } else if(material.Contains("Layers")) {
    ifirst=0; ilast=5;
  } else {
    Error("BuildMaterialLUT","Wrong layer name\n");
  }

  for(Int_t imat=ifirst; imat<=ilast; imat++) {
    Double_t param[5]={0.,0.,0.,0.,0.};
    for (Int_t i=0; i<n; i++) {
      phi = 2.*TMath::Pi()*gRandom->Rndm();
      cosphi = TMath::Cos(phi); sinphi = TMath::Sin(phi); 
      z = 14.*(-1.+2.*gRandom->Rndm()); // SPD barrel
      point1[0] = rmin[imat]*cosphi;
      point1[1] = rmin[imat]*sinphi;
      point1[2] = z;
      point2[0] = rmax[imat]*cosphi;
      point2[1] = rmax[imat]*sinphi;
      point2[2] = z;
      AliTracker::MeanMaterialBudget(point1,point2,mparam);
      for(Int_t j=0;j<5;j++) param[j]+=mparam[j];
    }
    for(Int_t j=0;j<5;j++) param[j]/=(Float_t)n;
    if(imat<=5) {
      fxOverX0Layer[imat] = param[1];
      fxTimesRhoLayer[imat] = param[0]*param[4];
    } else if(imat==6) {
      fxOverX0Pipe = param[1];
      fxTimesRhoPipe = param[0]*param[4];
    } else if(imat==7) {
      fxOverX0Shield[0] = param[1];
      fxTimesRhoShield[0] = param[0]*param[4];
    } else if(imat==8) {
      fxOverX0Shield[1] = param[1];
      fxTimesRhoShield[1] = param[0]*param[4];
    }
  }
  /*
  printf("%s\n",material.Data());
  printf("%f  %f\n",fxOverX0Pipe,fxTimesRhoPipe);
  printf("%f  %f\n",fxOverX0Shield[0],fxTimesRhoShield[0]);
  printf("%f  %f\n",fxOverX0Shield[1],fxTimesRhoShield[1]);
  printf("%f  %f\n",fxOverX0Layer[0],fxTimesRhoLayer[0]);
  printf("%f  %f\n",fxOverX0Layer[1],fxTimesRhoLayer[1]);
  printf("%f  %f\n",fxOverX0Layer[2],fxTimesRhoLayer[2]);
  printf("%f  %f\n",fxOverX0Layer[3],fxTimesRhoLayer[3]);
  printf("%f  %f\n",fxOverX0Layer[4],fxTimesRhoLayer[4]);
  printf("%f  %f\n",fxOverX0Layer[5],fxTimesRhoLayer[5]);
  */
  return;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::CorrectForPipeMaterial(AliITStrackMI *t,
					      TString direction) {
  //-------------------------------------------------------------------
  // Propagate beyond beam pipe and correct for material
  // (material budget in different ways according to fUseTGeo value)
  //-------------------------------------------------------------------

  // Define budget mode:
  // 0: material from AliITSRecoParam (hard coded)
  // 1: material from TGeo (on the fly)
  // 2: material from lut
  // 3: material from TGeo (same for all hypotheses)
  Int_t mode;
  switch(fUseTGeo) {
  case 0:
    mode=0; 
    break;    
  case 1:
    mode=1;
    break;    
  case 2:
    mode=2;
    break;
  case 3:
    if(fTrackingPhase.Contains("Clusters2Tracks")) 
      { mode=3; } else { mode=1; }
    break;
  case 4:
    if(fTrackingPhase.Contains("Clusters2Tracks")) 
      { mode=3; } else { mode=2; }
    break;
  default:
    mode=0;
    break;
  }
  if(fTrackingPhase.Contains("Default")) mode=0;

  Int_t index=fCurrentEsdTrack;

  Float_t  dir = (direction.Contains("inward") ? 1. : -1.);
  Double_t rToGo=(dir>0 ? AliITSRecoParam::GetrInsidePipe() : AliITSRecoParam::GetrOutsidePipe());
  Double_t xToGo;
  if (!t->GetLocalXat(rToGo,xToGo)) return 0;

  Double_t xOverX0,x0,lengthTimesMeanDensity;
  Bool_t anglecorr=kTRUE;

  switch(mode) {
  case 0:
    xOverX0 = AliITSRecoParam::GetdPipe();
    x0 = AliITSRecoParam::GetX0Be();
    lengthTimesMeanDensity = xOverX0*x0;
    break;
  case 1:
    if (!t->PropagateToTGeo(xToGo,1)) return 0;
    return 1;
    break;
  case 2:
    if(fxOverX0Pipe<0) BuildMaterialLUT("Pipe");  
    xOverX0 = fxOverX0Pipe;
    lengthTimesMeanDensity = fxTimesRhoPipe;
    break;
  case 3:
    if(!fxOverX0PipeTrks || index<0 || index>=fNtracks) Error("CorrectForPipeMaterial","Incorrect usage of UseTGeo option!\n");
    if(fxOverX0PipeTrks[index]<0) {
      if (!t->PropagateToTGeo(xToGo,1,xOverX0,lengthTimesMeanDensity)) return 0;
      Double_t angle=TMath::Sqrt((1.+t->GetTgl()*t->GetTgl())/
				 (1.-t->GetSnp()*t->GetSnp()));
      fxOverX0PipeTrks[index] = TMath::Abs(xOverX0)/angle;
      fxTimesRhoPipeTrks[index] = TMath::Abs(lengthTimesMeanDensity)/angle;
      return 1;
    }
    xOverX0 = fxOverX0PipeTrks[index];
    lengthTimesMeanDensity = fxTimesRhoPipeTrks[index];
    break;
  }

  lengthTimesMeanDensity *= dir;

  if (!t->AliExternalTrackParam::PropagateTo(xToGo,GetBz())) return 0;
  if (!t->CorrectForMeanMaterial(xOverX0,lengthTimesMeanDensity,anglecorr)) return 0;  

  return 1;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::CorrectForShieldMaterial(AliITStrackMI *t,
						TString shield,
						TString direction) {
  //-------------------------------------------------------------------
  // Propagate beyond SPD or SDD shield and correct for material
  // (material budget in different ways according to fUseTGeo value)
  //-------------------------------------------------------------------

  // Define budget mode:
  // 0: material from AliITSRecoParam (hard coded)
  // 1: material from TGeo (on the fly)
  // 2: material from lut
  // 3: material from TGeo (same for all hypotheses)
  Int_t mode;
  switch(fUseTGeo) {
  case 0:
    mode=0; 
    break;    
  case 1:
    mode=1;
    break;    
  case 2:
    mode=2;
    break;
  case 3:
    if(fTrackingPhase.Contains("Clusters2Tracks")) 
      { mode=3; } else { mode=1; }
    break;
  case 4:
    if(fTrackingPhase.Contains("Clusters2Tracks")) 
      { mode=3; } else { mode=2; }
    break;
  default:
    mode=0;
    break;
  }
  if(fTrackingPhase.Contains("Default")) mode=0;

  Float_t  dir = (direction.Contains("inward") ? 1. : -1.);
  Double_t rToGo;
  Int_t    shieldindex=0;
  if (shield.Contains("SDD")) { // SDDouter
    rToGo=(dir>0 ? AliITSRecoParam::GetrInsideShield(1) : AliITSRecoParam::GetrOutsideShield(1));
    shieldindex=1;
  } else if (shield.Contains("SPD")) {        // SPDouter
    rToGo=(dir>0 ? AliITSRecoParam::GetrInsideShield(0) : AliITSRecoParam::GetrOutsideShield(0)); 
    shieldindex=0;
  } else {
    Error("CorrectForShieldMaterial"," Wrong shield name\n");
    return 0;
  }
  Double_t xToGo;
  if (!t->GetLocalXat(rToGo,xToGo)) return 0;

  Int_t index=2*fCurrentEsdTrack+shieldindex;

  Double_t xOverX0,x0,lengthTimesMeanDensity;
  Bool_t anglecorr=kTRUE;

  switch(mode) {
  case 0:
    xOverX0 = AliITSRecoParam::Getdshield(shieldindex);
    x0 = AliITSRecoParam::GetX0shield(shieldindex);
    lengthTimesMeanDensity = xOverX0*x0;
    break;
  case 1:
    if (!t->PropagateToTGeo(xToGo,1)) return 0;
    return 1;
    break;
  case 2:
    if(fxOverX0Shield[shieldindex]<0) BuildMaterialLUT("Shields");  
    xOverX0 = fxOverX0Shield[shieldindex];
    lengthTimesMeanDensity = fxTimesRhoShield[shieldindex];
    break;
  case 3:
    if(!fxOverX0ShieldTrks || index<0 || index>=2*fNtracks) Error("CorrectForShieldMaterial","Incorrect usage of UseTGeo option!\n");
    if(fxOverX0ShieldTrks[index]<0) {
      if (!t->PropagateToTGeo(xToGo,1,xOverX0,lengthTimesMeanDensity)) return 0;
      Double_t angle=TMath::Sqrt((1.+t->GetTgl()*t->GetTgl())/
				 (1.-t->GetSnp()*t->GetSnp()));
      fxOverX0ShieldTrks[index] = TMath::Abs(xOverX0)/angle;
      fxTimesRhoShieldTrks[index] = TMath::Abs(lengthTimesMeanDensity)/angle;
      return 1;
    }
    xOverX0 = fxOverX0ShieldTrks[index];
    lengthTimesMeanDensity = fxTimesRhoShieldTrks[index];
    break;
  }

  lengthTimesMeanDensity *= dir;

  if (!t->AliExternalTrackParam::PropagateTo(xToGo,GetBz())) return 0;
  if (!t->CorrectForMeanMaterial(xOverX0,lengthTimesMeanDensity,anglecorr)) return 0;  

  return 1;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::CorrectForLayerMaterial(AliITStrackMI *t,
					       Int_t layerindex,
					       Double_t oldGlobXYZ[3],
					       TString direction) {
  //-------------------------------------------------------------------
  // Propagate beyond layer and correct for material
  // (material budget in different ways according to fUseTGeo value)
  //-------------------------------------------------------------------

  // Define budget mode:
  // 0: material from AliITSRecoParam (hard coded)
  // 1: material from TGeo (on the fly)
  // 2: material from lut
  // 3: material from TGeo (same for all hypotheses)
  Int_t mode;
  switch(fUseTGeo) {
  case 0:
    mode=0; 
    break;    
  case 1:
    mode=1;
    break;    
  case 2:
    mode=2;
    break;
  case 3:
    if(fTrackingPhase.Contains("Clusters2Tracks"))
      { mode=3; } else { mode=1; }
    break;
  case 4:
    if(fTrackingPhase.Contains("Clusters2Tracks")) 
      { mode=3; } else { mode=2; }
    break;
  default:
    mode=0;
    break;
  }
  if(fTrackingPhase.Contains("Default")) mode=0;

  Float_t  dir = (direction.Contains("inward") ? 1. : -1.);

  Double_t r=fgLayers[layerindex].GetR();
  Double_t deltar=(layerindex<2 ? 0.10*r : 0.05*r);

  Double_t rToGo=TMath::Sqrt(t->GetX()*t->GetX()+t->GetY()*t->GetY())-deltar*dir;
  Double_t xToGo;
  if (!t->GetLocalXat(rToGo,xToGo)) return 0;

  Int_t index=6*fCurrentEsdTrack+layerindex;

  // Bring the track beyond the material
  if (!t->AliExternalTrackParam::PropagateTo(xToGo,GetBz())) return 0;
  Double_t globXYZ[3];
  if (!t->GetXYZ(globXYZ)) return 0;

  Double_t xOverX0=0.0,x0=0.0,lengthTimesMeanDensity=0.0;
  Double_t mparam[7];
  Bool_t anglecorr=kTRUE;

  switch(mode) {
  case 0:
    xOverX0 = fgLayers[layerindex].GetThickness(t->GetY(),t->GetZ(),x0);
    lengthTimesMeanDensity = xOverX0*x0;
    break;
  case 1:
    AliTracker::MeanMaterialBudget(oldGlobXYZ,globXYZ,mparam);
    if(mparam[1]>900000) return 0;
    xOverX0=mparam[1];
    lengthTimesMeanDensity=mparam[0]*mparam[4];
    anglecorr=kFALSE;
    break;
  case 2:
    if(fxOverX0Layer[layerindex]<0) BuildMaterialLUT("Layers");  
    xOverX0 = fxOverX0Layer[layerindex];
    lengthTimesMeanDensity = fxTimesRhoLayer[layerindex];
    break;
  case 3:
    if(!fxOverX0LayerTrks || index<0 || index>=6*fNtracks) Error("CorrectForLayerMaterial","Incorrect usage of UseTGeo option!\n");
    if(fxOverX0LayerTrks[index]<0) {
      AliTracker::MeanMaterialBudget(oldGlobXYZ,globXYZ,mparam);
      if(mparam[1]>900000) return 0;
      Double_t angle=TMath::Sqrt((1.+t->GetTgl()*t->GetTgl())/
				 (1.-t->GetSnp()*t->GetSnp()));
      xOverX0=mparam[1]/angle;
      lengthTimesMeanDensity=mparam[0]*mparam[4]/angle;
      fxOverX0LayerTrks[index] = TMath::Abs(xOverX0);
      fxTimesRhoLayerTrks[index] = TMath::Abs(lengthTimesMeanDensity);
    }
    xOverX0 = fxOverX0LayerTrks[index];
    lengthTimesMeanDensity = fxTimesRhoLayerTrks[index];
    break;
  }

  lengthTimesMeanDensity *= dir;

  if (!t->CorrectForMeanMaterial(xOverX0,lengthTimesMeanDensity,anglecorr)) return 0;  

  return 1;
}
//------------------------------------------------------------------------
void AliITStrackerMI::MakeTrksMaterialLUT(Int_t ntracks) {
  //-----------------------------------------------------------------
  // Initialize LUT for storing material for each prolonged track
  //-----------------------------------------------------------------
  fxOverX0PipeTrks = new Float_t[ntracks]; 
  fxTimesRhoPipeTrks = new Float_t[ntracks]; 
  fxOverX0ShieldTrks = new Float_t[ntracks*2]; 
  fxTimesRhoShieldTrks = new Float_t[ntracks*2]; 
  fxOverX0LayerTrks = new Float_t[ntracks*6]; 
  fxTimesRhoLayerTrks = new Float_t[ntracks*6]; 

  for(Int_t i=0; i<ntracks; i++) {
    fxOverX0PipeTrks[i] = -1.;
    fxTimesRhoPipeTrks[i] = -1.;
  }
  for(Int_t j=0; j<ntracks*2; j++) {
    fxOverX0ShieldTrks[j] = -1.;
    fxTimesRhoShieldTrks[j] = -1.;
  }
  for(Int_t k=0; k<ntracks*6; k++) {
    fxOverX0LayerTrks[k] = -1.;
    fxTimesRhoLayerTrks[k] = -1.;
  }

  fNtracks = ntracks;  

  return;
}
//------------------------------------------------------------------------
void AliITStrackerMI::DeleteTrksMaterialLUT() {
  //-----------------------------------------------------------------
  // Delete LUT for storing material for each prolonged track
  //-----------------------------------------------------------------
  if(fxOverX0PipeTrks) { 
    delete [] fxOverX0PipeTrks; fxOverX0PipeTrks = 0; 
  } 
  if(fxOverX0ShieldTrks) { 
    delete [] fxOverX0ShieldTrks; fxOverX0ShieldTrks = 0; 
  } 
  
  if(fxOverX0LayerTrks) { 
    delete [] fxOverX0LayerTrks;  fxOverX0LayerTrks = 0; 
  } 
  if(fxTimesRhoPipeTrks) { 
    delete [] fxTimesRhoPipeTrks;  fxTimesRhoPipeTrks = 0; 
  } 
  if(fxTimesRhoShieldTrks) { 
    delete [] fxTimesRhoShieldTrks; fxTimesRhoShieldTrks = 0; 
  } 
  if(fxTimesRhoLayerTrks) { 
    delete [] fxTimesRhoLayerTrks; fxTimesRhoLayerTrks = 0; 
  } 
  return;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::CheckSkipLayer(AliITStrackMI *track,
				      Int_t ilayer,Int_t idet) const {
  //-----------------------------------------------------------------
  // This method is used to decide whether to allow a prolongation 
  // without clusters, because we want to skip the layer.
  // In this case the return value is > 0:
  // return 1: the user requested to skip a layer
  // return 2: track outside z acceptance of SSD/SDD and will cross both SPD
  //-----------------------------------------------------------------

  if (AliITSReconstructor::GetRecoParam()->GetLayersToSkip(ilayer)) return 1;

  if (idet<0 && ilayer>1 && AliITSReconstructor::GetRecoParam()->GetExtendedEtaAcceptance()) {
    // check if track will cross SPD outer layer
    Double_t phiAtSPD2,zAtSPD2;
    if (track->GetPhiZat(fgLayers[1].GetR(),phiAtSPD2,zAtSPD2)) {
      if (TMath::Abs(zAtSPD2)<2.*AliITSRecoParam::GetSPDdetzlength()) return 2;
    }
  }

  return 0;
}
//------------------------------------------------------------------------
Int_t AliITStrackerMI::CheckDeadZone(AliITStrackMI *track,
				     Int_t ilayer,Int_t idet,
				     Double_t dz,Double_t dy,
				     Bool_t noClusters) const {
  //-----------------------------------------------------------------
  // This method is used to decide whether to allow a prolongation 
  // without clusters, because there is a dead zone in the road.
  // In this case the return value is > 0:
  // return 1: dead zone at z=0,+-7cm in SPD
  // return 2: all road is "bad" (dead or noisy) from the OCDB
  // return 3: something "bad" (dead or noisy) from the OCDB
  //-----------------------------------------------------------------

  // check dead zones at z=0,+-7cm in the SPD
  if (ilayer<2 && !AliITSReconstructor::GetRecoParam()->GetAddVirtualClustersInDeadZone()) {
    Double_t zmindead[3]={fSPDdetzcentre[0] + 0.5*AliITSRecoParam::GetSPDdetzlength(),
			  fSPDdetzcentre[1] + 0.5*AliITSRecoParam::GetSPDdetzlength(),
			  fSPDdetzcentre[2] + 0.5*AliITSRecoParam::GetSPDdetzlength()};
    Double_t zmaxdead[3]={fSPDdetzcentre[1] - 0.5*AliITSRecoParam::GetSPDdetzlength(),
			  fSPDdetzcentre[2] - 0.5*AliITSRecoParam::GetSPDdetzlength(),
			  fSPDdetzcentre[3] - 0.5*AliITSRecoParam::GetSPDdetzlength()};
    for (Int_t i=0; i<3; i++)
      if (track->GetZ()-dz<zmaxdead[i] && track->GetZ()+dz>zmindead[i]) {
	AliDebug(2,Form("crack SPD %d",ilayer));
	return 1; 
      } 
  }

  // check bad zones from OCDB
  if (!AliITSReconstructor::GetRecoParam()->GetUseBadZonesFromOCDB()) return 0;

  if (idet<0) return 0;

  AliITSdetector &det=fgLayers[ilayer].GetDetector(idet);  

  Int_t detType=-1;
  Float_t detSizeFactorX=0.0001,detSizeFactorZ=0.0001;
  if (ilayer==0 || ilayer==1) {        // ----------  SPD
    detType = 0;
  } else if (ilayer==2 || ilayer==3) { // ----------  SDD
    detType = 1;
    detSizeFactorX *= 2.;
  } else if (ilayer==4 || ilayer==5) { // ----------  SSD
    detType = 2;
  }
  AliITSsegmentation *segm = (AliITSsegmentation*)fDetTypeRec->GetSegmentationModel(detType);
  if (detType==2) segm->SetLayer(ilayer+1);
  Float_t detSizeX = detSizeFactorX*segm->Dx(); 
  Float_t detSizeZ = detSizeFactorZ*segm->Dz(); 

  // check if the road overlaps with bad chips
  Float_t xloc,zloc;
  LocalModuleCoord(ilayer,idet,track,xloc,zloc);
  Float_t zlocmin = zloc-dz;
  Float_t zlocmax = zloc+dz;
  Float_t xlocmin = xloc-dy;
  Float_t xlocmax = xloc+dy;
  Int_t chipsInRoad[100];

  // check if road goes out of detector
  Bool_t touchNeighbourDet=kFALSE; 
  if (TMath::Abs(xlocmin)>0.5*detSizeX) {xlocmin=-0.5*detSizeX; touchNeighbourDet=kTRUE;} 
  if (TMath::Abs(xlocmax)>0.5*detSizeX) {xlocmax=+0.5*detSizeX; touchNeighbourDet=kTRUE;} 
  if (TMath::Abs(zlocmin)>0.5*detSizeZ) {zlocmin=-0.5*detSizeZ; touchNeighbourDet=kTRUE;} 
  if (TMath::Abs(zlocmax)>0.5*detSizeZ) {zlocmax=+0.5*detSizeZ; touchNeighbourDet=kTRUE;} 
  AliDebug(2,Form("layer %d det %d zmim zmax %f %f xmin xmax %f %f   %f %f",ilayer,idet,zlocmin,zlocmax,xlocmin,xlocmax,detSizeZ,detSizeX));

  // check if this detector is bad
  if (det.IsBad()) {
    AliDebug(2,Form("lay %d  bad detector %d",ilayer,idet));
    if(!touchNeighbourDet) {
      return 2; // all detectors in road are bad
    } else { 
      return 3; // at least one is bad
    }
  }

  Int_t nChipsInRoad = segm->GetChipsInLocalWindow(chipsInRoad,zlocmin,zlocmax,xlocmin,xlocmax);
  AliDebug(2,Form("lay %d nChipsInRoad %d",ilayer,nChipsInRoad));
  if (!nChipsInRoad) return 0;

  Bool_t anyBad=kFALSE,anyGood=kFALSE;
  for (Int_t iCh=0; iCh<nChipsInRoad; iCh++) {
    if (chipsInRoad[iCh]<0 || chipsInRoad[iCh]>det.GetNChips()-1) continue;
    AliDebug(2,Form("  chip %d bad %d",chipsInRoad[iCh],(Int_t)det.IsChipBad(chipsInRoad[iCh])));
    if (det.IsChipBad(chipsInRoad[iCh])) {
      anyBad=kTRUE;
    } else {
      anyGood=kTRUE;
    } 
  }

  if (!anyGood) {
    if(!touchNeighbourDet) {
      AliDebug(2,"all bad in road");
      return 2;  // all chips in road are bad
    } else {
      return 3; // at least a bad chip in road
    }
  }

  if (anyBad) {
    AliDebug(2,"at least a bad in road");
    return 3; // at least a bad chip in road
  } 


  if (!AliITSReconstructor::GetRecoParam()->GetUseSingleBadChannelsFromOCDB()
      || ilayer==4 || ilayer==5     // SSD
      || !noClusters) return 0;

  // There are no clusters in road: check if there is at least 
  // a bad SPD pixel or SDD anode 

  Int_t idetInITS=idet;
  for(Int_t l=0;l<ilayer;l++) idetInITS+=AliITSgeomTGeo::GetNLadders(l+1)*AliITSgeomTGeo::GetNDetectors(l+1);

  if (fITSChannelStatus->AnyBadInRoad(idetInITS,zlocmin,zlocmax,xlocmin,xlocmax)) {
    AliDebug(2,Form("Bad channel in det %d of layer %d\n",idet,ilayer));
    return 3;
  }
  //if (fITSChannelStatus->FractionOfBadInRoad(idet,zlocmin,zlocmax,xlocmin,xlocmax) > AliITSReconstructor::GetRecoParam()->GetMinFractionOfBadInRoad()) return 3;

  return 0;
}
//------------------------------------------------------------------------
Bool_t AliITStrackerMI::LocalModuleCoord(Int_t ilayer,Int_t idet,
				       AliITStrackMI *track,
				       Float_t &xloc,Float_t &zloc) const {
  //-----------------------------------------------------------------
  // Gives position of track in local module ref. frame
  //-----------------------------------------------------------------

  xloc=0.; 
  zloc=0.;

  if(idet<0) return kFALSE;

  Int_t ndet=AliITSgeomTGeo::GetNDetectors(ilayer+1); // layers from 1 to 6 

  Int_t lad = Int_t(idet/ndet) + 1;

  Int_t det = idet - (lad-1)*ndet + 1;

  Double_t xyzGlob[3],xyzLoc[3];

  AliITSdetector &detector = fgLayers[ilayer].GetDetector(idet);
  // take into account the misalignment: xyz at real detector plane
  if(!track->GetXYZAt(detector.GetRmisal(),GetBz(),xyzGlob)) return kFALSE;

  if(!AliITSgeomTGeo::GlobalToLocal(ilayer+1,lad,det,xyzGlob,xyzLoc)) return kFALSE;

  xloc = (Float_t)xyzLoc[0];
  zloc = (Float_t)xyzLoc[2];

  return kTRUE;
}
//------------------------------------------------------------------------
Bool_t AliITStrackerMI::IsOKForPlaneEff(AliITStrackMI* track, const Int_t *clusters, Int_t ilayer) const {
//
// Method to be optimized further: 
// Aim: decide whether a track can be used for PlaneEff evaluation
//      the decision is taken based on the track quality at the layer under study
//      no information on the clusters on this layer has to be used
//      The criterium is to reject tracks at boundaries between basic block (e.g. SPD chip)
//      the cut is done on number of sigmas from the boundaries
//
//  Input: Actual track, layer [0,5] under study
//  Output: none
//  Return: kTRUE if this is a good track
//
// it will apply a pre-selection to obtain good quality tracks.  
// Here also  you will have the possibility to put a control on the 
// impact point of the track on the basic block, in order to exclude border regions 
// this will be done by calling a proper method of the AliITSPlaneEff class.  
//
// input: AliITStrackMI* track, ilayer= layer number [0,5]
// return: Bool_t   -> kTRUE if usable track, kFALSE if not usable. 
//
  Int_t index[AliITSgeomTGeo::kNLayers];
  Int_t k;
  for (k=0; k<AliITSgeomTGeo::GetNLayers(); k++) index[k]=-1;
  //
  for (k=0; k<AliITSgeomTGeo::GetNLayers(); k++) {
    index[k]=clusters[k];
  }

  if(!fPlaneEff)
    {AliWarning("IsOKForPlaneEff: null pointer to AliITSPlaneEff"); return kFALSE;}
  AliITSlayer &layer=fgLayers[ilayer];
  Double_t r=layer.GetR();
  AliITStrackMI tmp(*track);

// require a minimal number of cluster in other layers and eventually clusters in closest layers 
  Int_t ncl=0; 
  for(Int_t lay=AliITSgeomTGeo::kNLayers-1;lay>ilayer;lay--) {
    AliDebug(2,Form("trak=%d  lay=%d  ; index=%d ESD label= %d",tmp.GetLabel(),lay,
                    tmp.GetClIndex(lay),((AliESDtrack*)tmp.GetESDtrack())->GetLabel())) ;
    if (tmp.GetClIndex(lay)>0) ncl++;
  }
  Bool_t nextout = kFALSE;
  if(ilayer==AliITSgeomTGeo::kNLayers-1) nextout=kTRUE; // you are already on the outermost layer
  else nextout = ((tmp.GetClIndex(ilayer+1)>0)? kTRUE : kFALSE );
  Bool_t nextin = kFALSE;
  if(ilayer==0) nextin=kTRUE; // you are already on the innermost layer
  else nextin = ((index[ilayer-1]>=0)? kTRUE : kFALSE );
  if(ncl<AliITSgeomTGeo::kNLayers-(ilayer+1)-AliITSReconstructor::GetRecoParam()->GetMaxMissingClustersPlaneEff()) 
     return kFALSE; 
  if(AliITSReconstructor::GetRecoParam()->GetRequireClusterInOuterLayerPlaneEff() && !nextout)  return kFALSE;
  if(AliITSReconstructor::GetRecoParam()->GetRequireClusterInInnerLayerPlaneEff() && !nextin)   return kFALSE;
  if(tmp.Pt() < AliITSReconstructor::GetRecoParam()->GetMinPtPlaneEff()) return kFALSE;
 //  if(AliITSReconstructor::GetRecoParam()->GetOnlyConstraintPlaneEff()  && !tmp.GetConstrain()) return kFALSE;

// detector number
  Double_t phi,z;
  if (!tmp.GetPhiZat(r,phi,z)) return kFALSE;
  Int_t idet=layer.FindDetectorIndex(phi,z);
  if(idet<0) { AliInfo(Form("cannot find detector"));
    return kFALSE;}

  // here check if it has good Chi Square.

  //propagate to the intersection with the detector plane
  const AliITSdetector &det=layer.GetDetector(idet);
  if (!tmp.Propagate(det.GetPhi(),det.GetR())) return kFALSE;

  Float_t locx; //
  Float_t locz; //
  if(!LocalModuleCoord(ilayer,idet,&tmp,locx,locz)) return kFALSE;
  UInt_t key=fPlaneEff->GetKeyFromDetLocCoord(ilayer,idet,locx,locz);
  if(key>fPlaneEff->Nblock()) return kFALSE;
  Float_t blockXmn,blockXmx,blockZmn,blockZmx;
  if (!fPlaneEff->GetBlockBoundaries(key,blockXmn,blockXmx,blockZmn,blockZmx)) return kFALSE;
  //***************
  // DEFINITION OF SEARCH ROAD FOR accepting a track
  //
  //For the time being they are hard-wired, later on from AliITSRecoParam
  // Double_t nsigx=AliITSRecoParam::GetNSigXFarFromBoundary();
  // Double_t nsigz=AliITSRecoParam::GetNSigZFarFromBoundary();
  Double_t nsigz=4; 
  Double_t nsigx=4; 
  Double_t dx=nsigx*TMath::Sqrt(tmp.GetSigmaY2());  // those are precisions in the tracking reference system
  Double_t dz=nsigz*TMath::Sqrt(tmp.GetSigmaZ2());  // Use it also for the module reference system, as it is
                                                // done for RecPoints

  // exclude tracks at boundary between detectors
  //Double_t boundaryWidth=AliITSRecoParam::GetBoundaryWidthPlaneEff();
  Double_t boundaryWidth=0; // for the time being hard-wired, later on from AliITSRecoParam
  AliDebug(2,Form("Tracking: track impact x=%f, y=%f, z=%f",tmp.GetX(), tmp.GetY(), tmp.GetZ()));
  AliDebug(2,Form("Local:    track impact x=%f, z=%f",locx,locz));
  AliDebug(2,Form("Search Road. Tracking: dy=%f , dz=%f",dx,dz));

  if ( (locx-dx < blockXmn+boundaryWidth) ||
       (locx+dx > blockXmx-boundaryWidth) ||
       (locz-dz < blockZmn+boundaryWidth) ||
       (locz+dz > blockZmx-boundaryWidth) ) return kFALSE;
  return kTRUE;
}
//------------------------------------------------------------------------
void AliITStrackerMI::UseTrackForPlaneEff(AliITStrackMI* track, Int_t ilayer) {
//
// This Method has to be optimized! For the time-being it uses the same criteria
// as those used in the search of extra clusters for overlapping modules.
//
// Method Purpose: estabilish whether a track has produced a recpoint or not
//                 in the layer under study (For Plane efficiency)
//
// inputs: AliITStrackMI* track  (pointer to a usable track)
// outputs: none
// side effects: update (by 1 count) the Plane Efficiency statistics of the basic block
//               traversed by this very track. In details:
//               - if a cluster can be associated to the track then call
//                  AliITSPlaneEff::UpDatePlaneEff(key,kTRUE);
//               - if not, the AliITSPlaneEff::UpDatePlaneEff(key,kFALSE) is called
//
  if(!fPlaneEff)
    {AliWarning("UseTrackForPlaneEff: null pointer to AliITSPlaneEff"); return;}
  AliITSlayer &layer=fgLayers[ilayer];
  Double_t r=layer.GetR();
  AliITStrackMI tmp(*track);

// detector number
  Double_t phi,z;
  if (!tmp.GetPhiZat(r,phi,z)) return;
  Int_t idet=layer.FindDetectorIndex(phi,z);

  if(idet<0) { AliInfo(Form("cannot find detector"));
    return;}


//propagate to the intersection with the detector plane
  const AliITSdetector &det=layer.GetDetector(idet);
  if (!tmp.Propagate(det.GetPhi(),det.GetR())) return;


//***************
// DEFINITION OF SEARCH ROAD FOR CLUSTERS SELECTION
//
  Double_t dz=AliITSReconstructor::GetRecoParam()->GetNSigmaRoadZ()*
                    TMath::Sqrt(tmp.GetSigmaZ2() +
                    AliITSReconstructor::GetRecoParam()->GetNSigmaZLayerForRoadZ()*
                    AliITSReconstructor::GetRecoParam()->GetNSigmaZLayerForRoadZ()*
                    AliITSReconstructor::GetRecoParam()->GetSigmaZ2(ilayer));
  Double_t dy=AliITSReconstructor::GetRecoParam()->GetNSigmaRoadY()*
                    TMath::Sqrt(tmp.GetSigmaY2() +
                    AliITSReconstructor::GetRecoParam()->GetNSigmaYLayerForRoadY()*
                    AliITSReconstructor::GetRecoParam()->GetNSigmaYLayerForRoadY()*
                    AliITSReconstructor::GetRecoParam()->GetSigmaY2(ilayer));

// road in global (rphi,z) [i.e. in tracking ref. system]
  Double_t zmin = tmp.GetZ() - dz;
  Double_t zmax = tmp.GetZ() + dz;
  Double_t ymin = tmp.GetY() + r*det.GetPhi() - dy;
  Double_t ymax = tmp.GetY() + r*det.GetPhi() + dy;

// select clusters in road
  layer.SelectClusters(zmin,zmax,ymin,ymax);

// Define criteria for track-cluster association
  Double_t msz = tmp.GetSigmaZ2() +
  AliITSReconstructor::GetRecoParam()->GetNSigmaZLayerForRoadZ()*
  AliITSReconstructor::GetRecoParam()->GetNSigmaZLayerForRoadZ()*
  AliITSReconstructor::GetRecoParam()->GetSigmaZ2(ilayer);
  Double_t msy = tmp.GetSigmaY2() +
  AliITSReconstructor::GetRecoParam()->GetNSigmaYLayerForRoadY()*
  AliITSReconstructor::GetRecoParam()->GetNSigmaYLayerForRoadY()*
  AliITSReconstructor::GetRecoParam()->GetSigmaY2(ilayer);
  if (tmp.GetConstrain()) {
    msz *= AliITSReconstructor::GetRecoParam()->GetNSigma2RoadZC();
    msy *= AliITSReconstructor::GetRecoParam()->GetNSigma2RoadYC();
  }  else {
    msz *= AliITSReconstructor::GetRecoParam()->GetNSigma2RoadZNonC();
    msy *= AliITSReconstructor::GetRecoParam()->GetNSigma2RoadYNonC();
  }
  msz = 1./msz; // 1/RoadZ^2
  msy = 1./msy; // 1/RoadY^2
//

  const AliITSRecPoint *cl=0; Int_t clidx=-1, ci=-1;
  Int_t idetc=-1;
  Double_t chi2trkcl=1000.*AliITSReconstructor::GetRecoParam()->GetMaxChi2();
  //Double_t  tolerance=0.2;
  /*while ((cl=layer.GetNextCluster(clidx))!=0) {
    idetc = cl->GetDetectorIndex();
    if(idet!=idetc) continue;
    //Int_t ilay = cl->GetLayer();

    if (TMath::Abs(tmp.GetZ() - cl->GetZ()) > tolerance) continue;
    if (TMath::Abs(tmp.GetY() - cl->GetY()) > tolerance) continue;

    Double_t chi2=tmp.GetPredictedChi2(cl);
    if (chi2<chi2trkcl) { chi2trkcl=chi2; ci=clidx; }
  }*/
  Float_t locx; //
  Float_t locz; //
  if(!LocalModuleCoord(ilayer,idet,&tmp,locx,locz)) return;
//
  AliDebug(2,Form("ilayer= %d, idet=%d, x= %f, z=%f",ilayer,idet,locx,locz));
  UInt_t key=fPlaneEff->GetKeyFromDetLocCoord(ilayer,idet,locx,locz);
  if(key>fPlaneEff->Nblock()) return;
  Bool_t found=kFALSE;
  //if (ci>=0) {
  Double_t chi2;
  while ((cl=layer.GetNextCluster(clidx))!=0) {
    idetc = cl->GetDetectorIndex();
    if(idet!=idetc) continue;
    // here real control to see whether the cluster can be associated to the track.
    // cluster not associated to track
    if ( (tmp.GetZ()-cl->GetZ())*(tmp.GetZ()-cl->GetZ())*msz +
         (tmp.GetY()-cl->GetY())*(tmp.GetY()-cl->GetY())*msy   > 1. ) continue;
    // calculate track-clusters chi2
    chi2 = GetPredictedChi2MI(&tmp,cl,ilayer); // note that this method change track tmp
                                               // in particular, the error associated to the cluster 
    //Double_t chi2 = tmp.GetPredictedChi(cl); // this method does not change track tmp
    // chi2 cut
    if (chi2 > AliITSReconstructor::GetRecoParam()->GetMaxChi2s(ilayer)) continue;
    found=kTRUE;
    if (chi2<chi2trkcl) { chi2trkcl=chi2; ci=clidx; } // this just to trace which cluster is selected
   // track->SetExtraCluster(ilayer,(ilayer<<28)+ci);
   // track->SetExtraModule(ilayer,idetExtra);
  }
  if(!fPlaneEff->UpDatePlaneEff(found,key))
       AliWarning(Form("UseTrackForPlaneEff: cannot UpDate PlaneEff for key=%d",key));
  if(fPlaneEff->GetCreateHistos()&&  AliITSReconstructor::GetRecoParam()->GetHistoPlaneEff()) {
    Float_t tr[4]={99999.,99999.,9999.,9999.};    // initialize to high values 
    Float_t clu[4]={-99999.,-99999.,9999.,9999.}; // (in some cases GetCov fails) 
    Int_t cltype[2]={-999,-999};

    tr[0]=locx;
    tr[1]=locz;
    tr[2]=TMath::Sqrt(tmp.GetSigmaY2());  // those are precisions in the tracking reference system
    tr[3]=TMath::Sqrt(tmp.GetSigmaZ2());  // Use it also for the module reference system, as it is

    if (found){
      clu[0]=layer.GetCluster(ci)->GetDetLocalX();
      clu[1]=layer.GetCluster(ci)->GetDetLocalZ();
      cltype[0]=layer.GetCluster(ci)->GetNy();
      cltype[1]=layer.GetCluster(ci)->GetNz();
     
     // Without the following 6 lines you would retrieve the nominal error of a cluster (e.g. for the SPD:
     //  X->50/sqrt(12)=14 micron   Z->450/sqrt(12)= 120 micron) 
     // Within AliTrackerMI/AliTrackMI the error on the cluster is associated to the AliITStrackMI (fSigmaY,Z)
     // It is computed properly by calling the method 
     // AliITStrackerMI::GetPredictedChi2MI(AliITStrackMI* track, const AliITSRecPoint *cluster,Int_t layer)
     // T
     //Double_t x=0.5*(tmp.GetX()+layer.GetCluster(ci)->GetX()); // Take into account the mis-alignment
      //if (tmp.PropagateTo(x,0.,0.)) {
        chi2=GetPredictedChi2MI(&tmp,layer.GetCluster(ci),ilayer);
        AliCluster c(*layer.GetCluster(ci));
        c.SetSigmaY2(tmp.GetSigmaY(ilayer)*tmp.GetSigmaY(ilayer));
        c.SetSigmaZ2(tmp.GetSigmaZ(ilayer)*tmp.GetSigmaZ(ilayer));
        //if (layer.GetCluster(ci)->GetGlobalCov(cov))  // by using this, instead, you got nominal cluster errors
        clu[2]=TMath::Sqrt(c.GetSigmaY2());
        clu[3]=TMath::Sqrt(c.GetSigmaZ2());
      //}
    }
    fPlaneEff->FillHistos(key,found,tr,clu,cltype);
  }
return;
}
