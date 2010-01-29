/**************************************************************************
* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
*                                                                        *
* Author: The ALICE Off-line Project.                                    *
* Contributors are mentioned in the code where appropriate.              *
*                                                                        *
* Permission to use, copy, modify and distribute this software and its   *
* documentation strictly for non-commercialf purposes is hereby granted   *
* without fee, provided that the above copyright notice appears in all   *
* copies and that both the copyright notice and this permission notice   *
* appear in the supporting documentation. The authors make no claims     *
* about the suitability of this software for any purpose. It is          *
* provided "as is" without express or implied warranty.                  *
**************************************************************************/

/* $Id: AliTRDresolution.cxx 27496 2008-07-22 08:35:45Z cblume $ */

////////////////////////////////////////////////////////////////////////////
//                                                                        //
//  TRD tracking resolution                                               //
//
// The class performs resolution and residual studies 
// of the TRD tracks for the following quantities :
//   - spatial position (y, [z])
//   - angular (phi) tracklet
//   - momentum at the track level
// 
// The class has to be used for regular detector performance checks using the official macros:
//   - $ALICE_ROOT/TRD/qaRec/run.C
//   - $ALICE_ROOT/TRD/qaRec/makeResults.C
// 
// For stand alone usage please refer to the following example: 
// {  
//   gSystem->Load("libANALYSIS.so");
//   gSystem->Load("libTRDqaRec.so");
//   AliTRDresolution *res = new AliTRDresolution();
//   //res->SetMCdata();
//   //res->SetVerbose();
//   //res->SetVisual();
//   res->Load();
//   if(!res->PostProcess()) return;
//   res->GetRefFigure(0);
// }  
//
//  Authors:                                                              //
//    Alexandru Bercuci <A.Bercuci@gsi.de>                                //
//    Markus Fasel <M.Fasel@gsi.de>                                       //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#include <TSystem.h>

#include <TROOT.h>
#include <TObjArray.h>
#include <TH3.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGaxis.h>
#include <TBox.h>
#include <TLegend.h>
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>
#include <TMath.h>
#include <TMatrixT.h>
#include <TVectorT.h>
#include <TTreeStream.h>
#include <TGeoManager.h>
#include <TDatabasePDG.h>

#include "AliPID.h"
#include "AliESDtrack.h"

#include "AliTRDresolution.h"
#include "AliTRDgeometry.h"
#include "AliTRDpadPlane.h"
#include "AliTRDcluster.h"
#include "AliTRDseedV1.h"
#include "AliTRDtrackV1.h"
#include "AliTRDReconstructor.h"
#include "AliTRDrecoParam.h"
#include "AliTRDpidUtil.h"

#include "info/AliTRDclusterInfo.h"

ClassImp(AliTRDresolution)

UChar_t const AliTRDresolution::fgNhistos[kNviews] = {
  2, 2, 5, 5,
  2, 5, 12, 2, 14
};
UChar_t const AliTRDresolution::fgNproj[kNviews] = {
  2, 2, 5, 5,
  4, 7, 12, 2, 14
};
Char_t const * AliTRDresolution::fgPerformanceName[kNviews] = {
     "Charge"
    ,"Cluster2Track"
    ,"Tracklet2Track"
    ,"Tracklet2TPC" 
    ,"Cluster2MC"
    ,"Tracklet2MC"
    ,"TPC2MC"
    ,"TOF/HMPID2MC"
    ,"TRD2MC"
};
UChar_t const AliTRDresolution::fgNcomp[kNprojs] = {
  1, 1, //2, 
  1, 1, //2, 
  1, 1, 1, 1, 1, //5, 
  1, 1, 1, 1, 1, 1, 1, //5,
// MC
  1, 1, 1, 1,    //4, 
  1, 1, 1, 1, 1, //5, 
  1, 1, 1, 1, 1, 1, 1, 1, 11, 11, 11, 11, //12, 
  1, 1, //2, 
  1, 1, 1, 1, 1, 1, 1, 1, 66, 66, 66, 66, 66, 66 //14
};
Char_t const *AliTRDresolution::fgAxTitle[kNprojs][4] = {
  // Charge
  {"Impv", "x [cm]", "I_{mpv}", "x/x_{0}"}
 ,{"dI/Impv", "x/x_{0}", "#delta I/I_{mpv}", "x[cm]"}
  // Clusters to Kalman
 ,{"Pos", "tg(#phi)", "#mu_{y}^{cl} [#mum]", "#sigma_{y}^{cl} [#mum]"}
 ,{"Pulls", "tg(#phi)", "PULL: #mu_{y}^{cl}", "PULL: #sigma_{y}^{cl}"}
  // TRD tracklet to Kalman fit
 ,{"PosY", "tg(#phi)", "#mu_{y}^{trklt} [#mum]", "#sigma_{y}^{trklt} [#mum]"}
 ,{"PullsY", "tg(#phi)", "PULL: #mu_{y}^{trklt}", "PULL: #sigma_{y}^{trklt}"}
 ,{"PosZ", "tg(#theta)", "#mu_{z}^{trklt} [#mum]", "#sigma_{z}^{trklt} [#mum]"}
 ,{"PullsZ", "tg(#theta)", "PULL: #mu_{z}^{trklt}", "PULL: #sigma_{z}^{trklt}"}
 ,{"Phi", "tg(#phi)", "#mu_{#phi}^{trklt} [mrad]", "#sigma_{#phi}^{trklt} [mrad]"}
  // TPC track 2 first TRD tracklet
 ,{"PosY", "tg(#phi)", "#mu_{y}^{TPC trklt} [#mum]", "#sigma_{y}^{TPC trklt} [#mum]"}
 ,{"PullsY", "tg(#phi)", "PULL: #mu_{y}^{TPC trklt}", "PULL: #sigma_{y}^{TPC trklt}"}
 ,{"PosZ", "tg(#theta)", "#mu_{z}^{TPC trklt} [#mum]", "#sigma_{z}^{TPC trklt} [#mum]"}
 ,{"PullsZ", "tg(#theta)", "PULL: #mu_{z}^{TPC trklt}", "PULL: #sigma_{z}^{TPC trklt}"}
 ,{"Phi", "tg(#phi)", "#mu_{#phi}^{TPC trklt} [mrad]", "#sigma_{#phi}^{TPC trklt} [mrad]"}
  // MC cluster
 ,{"Pos1", "tg(#phi)", "MC cluster (p_{t}<1 GeV/c): #mu_{y}[#mum]", "#sigma_{y}[#mum]"}
 ,{"Pos2", "tg(#phi)", "MC cluster (p_{t}<2 GeV/c): #mu_{y} [#mum]", "#sigma_{y} [#mum]"}
 ,{"Pos3", "tg(#phi)", "MC cluster (p_{t}>3 GeV/c): #mu_{y} [#mum]", "#sigma_{y}[#mum]"}
 ,{"Pulls", "tg(#phi)", "MC cluster PULL: #mu_{y}", "#sigma_{y}"}
  // MC tracklet
 ,{"PosY", "tg(#phi)", "MC tracklet (p_{t}<1 GeV/c): #mu_{y}[#mum]", "#sigma_{y}[#mum]"}
 ,{"PosY1", "tg(#phi)", "MC tracklet (p_{t}<2 GeV/c): #mu_{y}[#mum]", "#sigma_{y}[#mum]"}
 ,{"PosY2", "tg(#phi)", "MC tracklet (p_{t}>3 GeV/c): #mu_{y}[#mum]", "#sigma_{y}[#mum]"}
 ,{"PullsY", "tg(#phi)", "MC tracklet PULL: #mu_{y}", "MC PULL: #sigma_{y}"}
 ,{"PosZ", "tg(#theta)", "MC tracklet: #mu_{z}[#mum]", "#sigma_{z}[#mum]"}
 ,{"PullsZ", "tg(#theta)", "MC tracklet PULL: #mu_{z}", "#sigma_{z}"}
 ,{"Phi", "tg(#phi)", "MC tracklet: #mu_{#phi}[mrad]", "#sigma_{#phi}[mrad]"}
  // MC track TPC
 ,{"PosY", "tg(#phi)", "MC TPC2TRD0: #mu_{y}[#mum]", "#sigma_{y}[#mum]"}
 ,{"PullsY", "tg(#phi)", "MC TPC2TRD0 PULL: #mu_{y}", "#sigma_{y}"}
 ,{"PosZ", "tg(#theta)", "MC TPC2TRD0: #mu_{z}[#mum]", "#sigma_{z}[#mum]"}
 ,{"PullsZ", "tg(#theta)", "MC TPC2TRD0 PULL: #mu_{z}", "#sigma_{z}"}
 ,{"Phi", "tg(#phi)", "MC: #mu_{#phi}^{TPC} [mrad]", "MC: #sigma_{#phi}^{TPC} [mrad]"}
 ,{"PullsSNP", "tg(#phi)", "MC PULL: #mu_{snp}^{TPC}", "MC PULL: #sigma_{snp}^{TPC}"}
 ,{"Theta", "tg(#theta)", "MC: #mu_{#theta}^{TPC} [mrad]", "MC: #sigma_{#theta}^{TPC} [mrad]"}
 ,{"PullsTGL", "tg(#theta)", "MC PULL: #mu_{tgl}^{TPC}", "MC PULL: #sigma_{tgl}^{TPC}"}
 ,{"Pt", "p_{t}^{MC} [GeV/c]", "MC: #mu^{TPC}(#Deltap_{t}/p_{t}^{MC}) [%]", "MC: #sigma^{TPC}(#Deltap_{t}/p_{t}^{MC}) [%]"}
 ,{"Pulls1Pt", "1/p_{t}^{MC} [c/GeV]", "MC PULL: #mu_{1/p_{t}}^{TPC}", "MC PULL: #sigma_{1/p_{t}}^{TPC}"}
 ,{"P", "p^{MC} [GeV/c]", "MC: #mu^{TPC}(#Deltap/p^{MC}) [%]", "MC: #sigma^{TPC}(#Deltap/p^{MC}) [%]"}
 ,{"PullsP", "p^{MC} [GeV/c]", "MC PULL: #mu^{TPC}(#Deltap/#sigma_{p})", "MC PULL: #sigma^{TPC}(#Deltap/#sigma_{p})"}
  // MC track TOF
 ,{"PosZ", "tg(#theta)", "MC: #mu_{z}^{TOF} [#mum]", "MC: #sigma_{z}^{TOF} [#mum]"}
 ,{"PullsZ", "tg(#theta)", "MC PULL: #mu_{z}^{TOF}", "MC PULL: #sigma_{z}^{TOF}"}
  // MC track in TRD
 ,{"PosY", "tg(#phi)", "MC: #mu_{y}^{Trk} [#mum]", "MC: #sigma_{y}^{Trk} [#mum]"}
 ,{"PullsY", "tg(#phi)", "MC PULL: #mu_{y}^{Trk}", "MC PULL: #sigma_{y}^{Trk}"}
 ,{"PosZ", "tg(#theta)", "MC: #mu_{z}^{Trk} [#mum]", "MC: #sigma_{z}^{Trk} [#mum]"}
 ,{"PullsZ", "tg(#theta)", "MC PULL: #mu_{z}^{Trk}", "MC PULL: #sigma_{z}^{Trk}"}
 ,{"Phi", "tg(#phi)", "MC: #mu_{#phi}^{Trk} [mrad]", "MC: #sigma_{#phi}^{Trk} [mrad]"}
 ,{"PullsSNP", "tg(#phi)", "MC PULL: #mu_{snp}^{Trk}", "MC PULL: #sigma_{snp}^{Trk}"}
 ,{"Theta", "tg(#theta)", "MC: #mu_{#theta}^{Trk} [mrad]", "MC: #sigma_{#theta}^{Trk} [mrad]"}
 ,{"PullsTGL", "tg(#theta)", "MC PULL: #mu_{tgl}^{Trk}", "MC PULL: #sigma_{tgl}^{Trk}"}
 ,{"MC Pt Layer", "p_{t} [GeV/c]", "#mu(#Deltap_{t}/p_{t}^{MC}) [%]", "#sigma(#Deltap_{t}/p_{t}^{MC}) [%]"}
 ,{"MC Pulls1Pt Layer", "1/p_{t} [c/GeV]", "#mu_{1/p_{t}}", "#sigma_{1/p_{t}}"}
 ,{"MC P Layer", "p [GeV/c]", "#mu(#Deltap/p^{MC}) [%]", "#sigma(#Deltap/p^{MC}) [%]"}
 ,{"Pt - SA", "p_{t}^{MC} [GeV/c]", "MC: #mu^{Trk}(#Deltap_{t}/p_{t}^{MC}) [%]", "MC: #sigma^{Trk}(#Deltap_{t}/p_{t}^{MC}) [%]"}
 ,{"Pulls1Pt - SA", "1/p_{t}^{MC} [c/GeV]", "MC PULL: #mu_{1/p_{t}}^{Trk}", "MC PULL: #sigma_{1/p_{t}}^{Trk}"}
 ,{"P - SA", "p^{MC} [GeV/c]", "MC: #mu^{Trk}(#Deltap/p^{MC}) [%]", "MC: #sigma^{Trk}(#Deltap/p^{MC}) [%]"}
};

//________________________________________________________
AliTRDresolution::AliTRDresolution()
  :AliTRDrecoTask("resolution", "Spatial and momentum TRD resolution checker")
  ,fStatus(0)
  ,fIdxPlot(0)
  ,fIdxFrame(0)
  ,fReconstructor(NULL)
  ,fGeo(NULL)
  ,fDBPDG(NULL)
  ,fGraphS(NULL)
  ,fGraphM(NULL)
  ,fCl(NULL)
  ,fTrklt(NULL)
  ,fMCcl(NULL)
  ,fMCtrklt(NULL)
{
  //
  // Default constructor
  //

  fReconstructor = new AliTRDReconstructor();
  fReconstructor->SetRecoParam(AliTRDrecoParam::GetLowFluxParam());
  fGeo = new AliTRDgeometry();

  InitFunctorList();

  DefineOutput(1, TObjArray::Class()); // cluster2track
  DefineOutput(2, TObjArray::Class()); // tracklet2track
  DefineOutput(3, TObjArray::Class()); // cluster2mc
  DefineOutput(4, TObjArray::Class()); // tracklet2mc
}

//________________________________________________________
AliTRDresolution::~AliTRDresolution()
{
  //
  // Destructor
  //

  if(fGraphS){fGraphS->Delete(); delete fGraphS;}
  if(fGraphM){fGraphM->Delete(); delete fGraphM;}
  delete fGeo;
  delete fReconstructor;
  if(gGeoManager) delete gGeoManager;
  if(fCl){fCl->Delete(); delete fCl;}
  if(fTrklt){fTrklt->Delete(); delete fTrklt;}
  if(fMCcl){fMCcl->Delete(); delete fMCcl;}
  if(fMCtrklt){fMCtrklt->Delete(); delete fMCtrklt;}
}


//________________________________________________________
void AliTRDresolution::CreateOutputObjects()
{
  // spatial resolution
  OpenFile(0, "RECREATE");

  fContainer = Histos();

  fCl = new TObjArray();
  fCl->SetOwner(kTRUE);
  fTrklt = new TObjArray();
  fTrklt->SetOwner(kTRUE);
  fMCcl = new TObjArray();
  fMCcl->SetOwner(kTRUE);
  fMCtrklt = new TObjArray();
  fMCtrklt->SetOwner(kTRUE);
}

//________________________________________________________
void AliTRDresolution::Exec(Option_t *opt)
{
  //
  // Execution part
  //

  fCl->Delete();
  fTrklt->Delete();
  fMCcl->Delete();
  fMCtrklt->Delete();

  AliTRDrecoTask::Exec(opt);

  PostData(1, fCl);
  PostData(2, fTrklt);
  PostData(3, fMCcl);
  PostData(4, fMCtrklt);
}

//________________________________________________________
TH1* AliTRDresolution::PlotCharge(const AliTRDtrackV1 *track)
{
  //
  // Plots the charge distribution
  //

  if(track) fkTrack = track;
  if(!fkTrack){
    AliDebug(2, "No Track defined.");
    return NULL;
  }
  TObjArray *arr = NULL;
  if(!(arr = ((TObjArray*)fContainer->At(kCharge)))){
    AliWarning("No output container defined.");
    return NULL;
  }
  TH3S* h = NULL;

  AliTRDseedV1 *fTracklet = NULL;  
  AliTRDcluster *c = NULL;
  for(Int_t ily=0; ily<AliTRDgeometry::kNlayer; ily++){
    if(!(fTracklet = fkTrack->GetTracklet(ily))) continue;
    if(!fTracklet->IsOK()) continue;
    Float_t x0 = fTracklet->GetX0();
    Float_t dq, dl;
    for(Int_t itb=AliTRDseedV1::kNtb; itb--;){
      if(!(c = fTracklet->GetClusters(itb))){ 
        if(!(c = fTracklet->GetClusters(AliTRDseedV1::kNtb+itb))) continue;
      }
      dq = fTracklet->GetdQdl(itb, &dl);
      dl /= 0.15; // dl/dl0, dl0 = 1.5 mm for nominal vd
      (h = (TH3S*)arr->At(0))->Fill(dl, x0-c->GetX(), dq);
    }

//     if(!HasMCdata()) continue;
//     UChar_t s;
//     Float_t pt0, y0, z0, dydx0, dzdx0;
//     if(!fMC->GetDirections(x0, y0, z0, dydx0, dzdx0, pt0, s)) continue;

  }
  return h;
}


//________________________________________________________
TH1* AliTRDresolution::PlotCluster(const AliTRDtrackV1 *track)
{
  //
  // Plot the cluster distributions
  //

  if(track) fkTrack = track;
  if(!fkTrack){
    AliDebug(2, "No Track defined.");
    return NULL;
  }
  TObjArray *arr = NULL;
  if(!(arr = ((TObjArray*)fContainer->At(kCluster)))){
    AliWarning("No output container defined.");
    return NULL;
  }
  ULong_t status = fkESD ? fkESD->GetStatus():0;

  Double_t cov[7];
  Float_t x0, y0, z0, dy, dydx, dzdx;
  AliTRDseedV1 *fTracklet = NULL;  
  for(Int_t ily=0; ily<AliTRDgeometry::kNlayer; ily++){
    if(!(fTracklet = fkTrack->GetTracklet(ily))) continue;
    if(!fTracklet->IsOK()) continue;
    x0 = fTracklet->GetX0();

    // retrive the track angle with the chamber
    y0   = fTracklet->GetYref(0);
    z0   = fTracklet->GetZref(0);
    dydx = fTracklet->GetYref(1);
    dzdx = fTracklet->GetZref(1);
    fTracklet->GetCovRef(cov);
    Float_t tilt = fTracklet->GetTilt();
    AliTRDcluster *c = NULL;
    fTracklet->ResetClusterIter(kFALSE);
    while((c = fTracklet->PrevCluster())){
      Float_t xc = c->GetX();
      Float_t yc = c->GetY();
      Float_t zc = c->GetZ();
      Float_t dx = x0 - xc; 
      Float_t yt = y0 - dx*dydx;
      Float_t zt = z0 - dx*dzdx; 
      yc -= tilt*(zc-zt); // tilt correction
      dy = yt - yc;

      //Float_t sx2 = dydx*c->GetSX(c->GetLocalTimeBin()); sx2*=sx2;
      Float_t sy2 = c->GetSigmaY2();
      if(sy2<=0.) continue;
      ((TH2I*)arr->At(0))->Fill(dydx, dy);
      ((TH2I*)arr->At(1))->Fill(dydx, dy/TMath::Sqrt(cov[0] /*+ sx2*/ + sy2));
  
      if(DebugLevel()>=2){
        // Get z-position with respect to anode wire
        Int_t istk = fGeo->GetStack(c->GetDetector());
        AliTRDpadPlane *pp = fGeo->GetPadPlane(ily, istk);
        Float_t row0 = pp->GetRow0();
        Float_t d  =  row0 - zt + pp->GetAnodeWireOffset();
        d -= ((Int_t)(2 * d)) / 2.0;
        if (d > 0.25) d  = 0.5 - d;

        AliTRDclusterInfo *clInfo = new AliTRDclusterInfo;
        fCl->Add(clInfo);
        clInfo->SetCluster(c);
        Float_t covR[] = {cov[0], cov[1], cov[2]};
        clInfo->SetGlobalPosition(yt, zt, dydx, dzdx, covR);
        clInfo->SetResolution(dy);
        clInfo->SetAnisochronity(d);
        clInfo->SetDriftLength(dx);
        (*DebugStream()) << "ClusterREC"
          <<"status="  << status
          <<"clInfo.=" << clInfo
          << "\n";
      }
    }
  }
  return (TH2I*)arr->At(0);
}


//________________________________________________________
TH1* AliTRDresolution::PlotTracklet(const AliTRDtrackV1 *track)
{
// Plot normalized residuals for tracklets to track. 
// 
// We start from the result that if X=N(|m|, |Cov|)
// BEGIN_LATEX
// (Cov^{-1})^{1/2}X = N((Cov^{-1})^{1/2}*|m|, |1|)
// END_LATEX
// in our case X=(y_trklt - y_trk z_trklt - z_trk) and |Cov| = |Cov_trklt| + |Cov_trk| at the radial 
// reference position. 
  if(track) fkTrack = track;
  if(!fkTrack){
    AliDebug(2, "No Track defined.");
    return NULL;
  }
  TObjArray *arr = NULL;
  if(!(arr = (TObjArray*)fContainer->At(kTrackTRD ))){
    AliWarning("No output container defined.");
    return NULL;
  }

  Double_t cov[3], covR[7]/*, sqr[3], inv[3]*/;
  Float_t x, dx, dy, dz;
  AliTRDseedV1 *fTracklet = NULL;  
  for(Int_t il=AliTRDgeometry::kNlayer; il--;){
    if(!(fTracklet = fkTrack->GetTracklet(il))) continue;
    if(!fTracklet->IsOK()) continue;
    x    = fTracklet->GetX();
    dx   = fTracklet->GetX0() - x;
    // compute dy^2 and dz^2
    dy   = fTracklet->GetYref(0)-dx*fTracklet->GetYref(1) - fTracklet->GetY();
    dz   = fTracklet->GetZref(0)-dx*fTracklet->GetZref(1) - fTracklet->GetZ();
    // compute covariance matrix
    fTracklet->GetCovAt(x, cov);
    fTracklet->GetCovRef(covR);
    cov[0] += covR[0]; cov[1] += covR[1]; cov[2] += covR[2]; 
/*  // Correct PULL calculation by considering off  
    // diagonal elements in the covariance matrix
    // compute square root matrix
    if(AliTRDseedV1::GetCovInv(cov, inv)==0.) continue;
    if(AliTRDseedV1::GetCovSqrt(inv, sqr)<0.) continue;
    Double_t y = sqr[0]*dy+sqr[1]*dz;
    Double_t z = sqr[1]*dy+sqr[2]*dz;
    ((TH3*)h)->Fill(y, z, fTracklet->GetYref(1));*/

    ((TH2I*)arr->At(0))->Fill(fTracklet->GetYref(1), dy);
    ((TH2I*)arr->At(1))->Fill(fTracklet->GetYref(1), dy/TMath::Sqrt(cov[0]));
    ((TH2I*)arr->At(4))->Fill(fTracklet->GetYref(1), TMath::ATan((fTracklet->GetYref(1)-fTracklet->GetYfit(1))/(1-fTracklet->GetYref(1)*fTracklet->GetYfit(1))));
    if(!fTracklet->IsRowCross()) continue;
    ((TH2I*)arr->At(2))->Fill(fTracklet->GetZref(1), dz);
    ((TH2I*)arr->At(3))->Fill(fTracklet->GetZref(1), dz/TMath::Sqrt(cov[2]));
  }


  return (TH2I*)arr->At(0);
}


//________________________________________________________
TH1* AliTRDresolution::PlotTrackTPC(const AliTRDtrackV1 *track)
{
// Store resolution/pulls of Kalman before updating with the TRD information 
// at the radial position of the first tracklet. The following points are used 
// for comparison  
//  - the (y,z,snp) of the first TRD tracklet
//  - the (y, z, snp, tgl, pt) of the MC track reference
// 
// Additionally the momentum resolution/pulls are calculated for usage in the 
// PID calculation. 

  if(track) fkTrack = track;
  if(!fkTrack){
    AliDebug(2, "No Track defined.");
    return NULL;
  }
  AliExternalTrackParam *tin = NULL;
  if(!(tin = fkTrack->GetTrackLow())){
    AliWarning("Track did not entered TRD fiducial volume.");
    return NULL;
  }
  TH1 *h = NULL;
  
  Double_t x = tin->GetX();
  AliTRDseedV1 *tracklet = NULL;  
  for(Int_t ily=0; ily<AliTRDgeometry::kNlayer; ily++){
    if(!(tracklet = fkTrack->GetTracklet(ily))) continue;
    break;
  }
  if(!tracklet || TMath::Abs(x-tracklet->GetX())>1.e-3){
    AliWarning("Tracklet did not match TRD entrance.");
    return NULL;
  }
  const Int_t kNPAR(5);
  Double_t parR[kNPAR]; memcpy(parR, tin->GetParameter(), kNPAR*sizeof(Double_t));
  Double_t covR[3*kNPAR]; memcpy(covR, tin->GetCovariance(), 3*kNPAR*sizeof(Double_t));
  Double_t cov[3]; tracklet->GetCovAt(x, cov);

  // define sum covariances
  TMatrixDSym COV(kNPAR); TVectorD PAR(kNPAR);
  Double_t *pc = &covR[0], *pp = &parR[0];
  for(Int_t ir=0; ir<kNPAR; ir++, pp++){
    PAR(ir) = (*pp);
    for(Int_t ic = 0; ic<=ir; ic++,pc++){ 
      COV(ir,ic) = (*pc); COV(ic,ir) = (*pc);
    }
  }
  PAR[4] = TMath::Abs(PAR[4]); // remove sign of pt !!
  //COV.Print(); PAR.Print();

  //TODO Double_t dydx =  TMath::Sqrt(1.-parR[2]*parR[2])/parR[2]; 
  Double_t dy = parR[0] - tracklet->GetY(); 
  TObjArray *arr = (TObjArray*)fContainer->At(kTrackTPC);
  ((TH2I*)arr->At(0))->Fill(tracklet->GetYref(1), dy);
  ((TH2I*)arr->At(1))->Fill(tracklet->GetYref(1), dy/TMath::Sqrt(COV(0,0)+cov[0]));
  if(tracklet->IsRowCross()){
    Double_t dz = parR[1] - tracklet->GetZ(); 
    ((TH2I*)arr->At(2))->Fill(tracklet->GetZref(1), dz);
    ((TH2I*)arr->At(3))->Fill(tracklet->GetZref(1), dz/TMath::Sqrt(COV(1,1)+cov[2]));
  }
  Double_t dphi = TMath::ASin(PAR[2])-TMath::ATan(tracklet->GetYfit(1));  ((TH2I*)arr->At(4))->Fill(tracklet->GetYref(1), dphi);


  // register reference histo for mini-task
  h = (TH2I*)arr->At(0);

  if(DebugLevel()>=1){
    (*DebugStream()) << "trackIn"
      << "x="       << x
      << "P="       << &PAR
      << "C="       << &COV
      << "\n";

    Double_t y = tracklet->GetY(); 
    Double_t z = tracklet->GetZ(); 
    (*DebugStream()) << "trackletIn"
      << "y="       << y
      << "z="       << z
      << "Vy="      << cov[0]
      << "Cyz="     << cov[1]
      << "Vz="      << cov[2]
      << "\n";
  }


  if(!HasMCdata()) return h;
  UChar_t s;
  Float_t dx, pt0, x0=tracklet->GetX0(), y0, z0, dydx0, dzdx0;
  if(!fkMC->GetDirections(x0, y0, z0, dydx0, dzdx0, pt0, s)) return h;
  Int_t pdg = fkMC->GetPDG(),
        sIdx(AliTRDpidUtil::Pdg2Pid(TMath::Abs(pdg))+1), // species index
        sign(0);
  if(!fDBPDG) fDBPDG=TDatabasePDG::Instance();
  TParticlePDG *ppdg(fDBPDG->GetParticle(pdg));
  if(ppdg) sign = ppdg->Charge() > 0. ? 1 : -1;

  // translate to reference radial position
  dx = x0 - x; y0 -= dx*dydx0; z0 -= dx*dzdx0;
  Float_t norm = 1./TMath::Sqrt(1.+dydx0*dydx0); // 1/sqrt(1+tg^2(phi))
  //Fill MC info
  TVectorD PARMC(kNPAR);
  PARMC[0]=y0; PARMC[1]=z0;
  PARMC[2]=dydx0*norm; PARMC[3]=dzdx0*norm;
  PARMC[4]=1./pt0;

//   TMatrixDSymEigen eigen(COV);
//   TVectorD evals = eigen.GetEigenValues();
//   TMatrixDSym evalsm(kNPAR);
//   for(Int_t ir=0; ir<kNPAR; ir++) for(Int_t ic=0; ic<kNPAR; ic++) evalsm(ir,ic) = (ir==ic ? evals(ir): 0.);
//   TMatrixD evecs = eigen.GetEigenVectors();
//   TMatrixD sqrcov(evecs, TMatrixD::kMult, TMatrixD(evalsm, TMatrixD::kMult, evecs.T()));
  
  // fill histos
  arr = (TObjArray*)fContainer->At(kMCtrackTPC);
  // y resolution/pulls
  ((TH2I*)arr->At(0))->Fill(dydx0, PARMC[0]-PAR[0]);
  ((TH2I*)arr->At(1))->Fill(dydx0, (PARMC[0]-PAR[0])/TMath::Sqrt(COV(0,0)));
  // z resolution/pulls
  ((TH2I*)arr->At(2))->Fill(dzdx0, PARMC[1]-PAR[1]);
  ((TH2I*)arr->At(3))->Fill(dzdx0, (PARMC[1]-PAR[1])/TMath::Sqrt(COV(1,1)));
  // phi resolution/snp pulls
  ((TH2I*)arr->At(4))->Fill(dydx0, TMath::ASin(PARMC[2])-TMath::ASin(PAR[2]));
  ((TH2I*)arr->At(5))->Fill(dydx0, (PARMC[2]-PAR[2])/TMath::Sqrt(COV(2,2)));
  // theta resolution/tgl pulls
  ((TH2I*)arr->At(6))->Fill(dzdx0, TMath::ATan((PARMC[3]-PAR[3])/(1-PARMC[3]*PAR[3])));
  ((TH2I*)arr->At(7))->Fill(dzdx0, (PARMC[3]-PAR[3])/TMath::Sqrt(COV(3,3)));
  // pt resolution\\1/pt pulls\\p resolution/pull
  ((TH3S*)arr->At(8))->Fill(pt0, PARMC[4]/PAR[4]-1., sign*sIdx);
  ((TH3S*)arr->At(9))->Fill(PARMC[4], (PARMC[4]-PAR[4])/TMath::Sqrt(COV(4,4)), sign*sIdx);

  Double_t p0 = TMath::Sqrt(1.+ PARMC[3]*PARMC[3])*pt0, p;
  p = TMath::Sqrt(1.+ PAR[3]*PAR[3])*PAR[4];
  Float_t sp = 
    p*p*COV(4,4)/PAR[4]/PAR[4]
   +2.*PAR[3]*PAR[4]*COV(3,4)
   +PAR[3]*PAR[3]*PAR[4]*PAR[4]*PAR[4]*PAR[4]*COV(3,3)/p/p;
  ((TH3S*)arr->At(10))->Fill(p0, p/p0-1., sign*sIdx);
  if(sp>0.) ((TH3S*)arr->At(11))->Fill(p0, (p0-p)/TMath::Sqrt(sp), sign*sIdx);

  // fill debug for MC 
  if(DebugLevel()>=1){
    (*DebugStream()) << "trackInMC"
      << "P="   << &PARMC
      << "\n";
  }
  return h;
}

//________________________________________________________
TH1* AliTRDresolution::PlotMC(const AliTRDtrackV1 *track)
{
  //
  // Plot MC distributions
  //

  if(!HasMCdata()){ 
    AliWarning("No MC defined. Results will not be available.");
    return NULL;
  }
  if(track) fkTrack = track;
  if(!fkTrack){
    AliDebug(2, "No Track defined.");
    return NULL;
  }
  // retriev track characteristics
  Int_t pdg = fkMC->GetPDG(),
        sIdx(AliTRDpidUtil::Pdg2Pid(TMath::Abs(pdg))+1), // species index
        sign(0),
        det(-1),
        label(fkMC->GetLabel());
  if(!fDBPDG) fDBPDG=TDatabasePDG::Instance();
  TParticlePDG *ppdg(fDBPDG->GetParticle(pdg));
  if(ppdg) sign = ppdg->Charge() > 0. ? 1 : -1;
  Bool_t kBarrel = Bool_t(fkESD->GetStatus() & AliESDtrack::kTRDin);

  TObjArray *arr(NULL);TH1 *h(NULL);
  UChar_t s;
  Double_t xAnode, x, y, z, pt, dydx, dzdx, dzdl;
  Float_t pt0, x0, y0, z0, dx, dy, dz, dydx0, dzdx0;
  Double_t covR[7]/*, cov[3]*/;

  if(DebugLevel()>=1){
    Double_t dX[12], dY[12], dZ[12], dPt[12], cCOV[12][15];
    fkMC->PropagateKalman(dX, dY, dZ, dPt, cCOV);
    (*DebugStream()) << "MCkalman"
      << "pdg="  << pdg
      << "dx0="  << dX[0]
      << "dx1="  << dX[1]
      << "dx2="  << dX[2]
      << "dy0="  << dY[0]
      << "dy1="  << dY[1]
      << "dy2="  << dY[2]
      << "dz0="  << dZ[0]
      << "dz1="  << dZ[1]
      << "dz2="  << dZ[2]
      << "dpt0=" << dPt[0]
      << "dpt1=" << dPt[1]
      << "dpt2=" << dPt[2]
      << "\n";
  }

  AliTRDReconstructor rec;
  AliTRDseedV1 *fTracklet(NULL); TObjArray *clInfoArr(NULL);
  for(Int_t ily=0; ily<AliTRDgeometry::kNlayer; ily++){
    if(!(fTracklet = fkTrack->GetTracklet(ily)))/* ||
       !fTracklet->IsOK())*/ continue;

    det = fTracklet->GetDetector();
    x0  = fTracklet->GetX0();
    //radial shift with respect to the MC reference (radial position of the pad plane)
    x= fTracklet->GetX();
    if(!fkMC->GetDirections(x0, y0, z0, dydx0, dzdx0, pt0, s)) continue;
    xAnode  = fTracklet->GetX0();

    // MC track position at reference radial position
    dx  = x0 - x;
    if(DebugLevel()>=1){
      (*DebugStream()) << "MC"
        << "det="     << det
        << "pdg="     << pdg
        << "sgn="     << sign
        << "pt="      << pt0
        << "x="       << x0
        << "y="       << y0
        << "z="       << z0
        << "dydx="    << dydx0
        << "dzdx="    << dzdx0
        << "\n";
    }
    Float_t ymc = y0 - dx*dydx0;
    Float_t zmc = z0 - dx*dzdx0;
    //p = pt0*TMath::Sqrt(1.+dzdx0*dzdx0); // pt -> p

    // Kalman position at reference radial position
    dx = xAnode - x;
    dydx = fTracklet->GetYref(1);
    dzdx = fTracklet->GetZref(1);
    dzdl = fTracklet->GetTgl();
    y  = fTracklet->GetYref(0) - dx*dydx;
    dy = y - ymc;
    z  = fTracklet->GetZref(0) - dx*dzdx;
    dz = z - zmc;
    pt = TMath::Abs(fTracklet->GetPt());
    fTracklet->GetCovRef(covR);

    arr = (TObjArray*)fContainer->At(kMCtrackTRD);
    // y resolution/pulls
    ((TH2I*)arr->At(0))->Fill(dydx0, dy);
    ((TH2I*)arr->At(1))->Fill(dydx0, dy/TMath::Sqrt(covR[0]));
    // z resolution/pulls
    ((TH2I*)arr->At(2))->Fill(dzdx0, dz);
    ((TH2I*)arr->At(3))->Fill(dzdx0, dz/TMath::Sqrt(covR[2]));
    // phi resolution/ snp pulls
    Double_t dtgp = (dydx - dydx0)/(1.- dydx*dydx0);
    ((TH2I*)arr->At(4))->Fill(dydx0, TMath::ATan(dtgp));
    Double_t dsnp = dydx/TMath::Sqrt(1.+dydx*dydx) - dydx0/TMath::Sqrt(1.+dydx0*dydx0);
    ((TH2I*)arr->At(5))->Fill(dydx0, dsnp/TMath::Sqrt(covR[3]));
    // theta resolution/ tgl pulls
    Double_t dzdl0 = dzdx0/TMath::Sqrt(1.+dydx0*dydx0),
              dtgl = (dzdl - dzdl0)/(1.- dzdl*dzdl0);
    ((TH2I*)arr->At(6))->Fill(dzdl0, 
    TMath::ATan(dtgl));
    ((TH2I*)arr->At(7))->Fill(dzdl0, (dzdl - dzdl0)/TMath::Sqrt(covR[4]));
    // pt resolution  \\ 1/pt pulls \\ p resolution for PID
    Double_t p0 = TMath::Sqrt(1.+ dzdl0*dzdl0)*pt0,
             p  = TMath::Sqrt(1.+ dzdl*dzdl)*pt;
    if(kBarrel){
      ((TH3S*)((TObjArray*)arr->At(8))->At(ily))->Fill(pt0, pt/pt0-1., sign*sIdx);
      ((TH3S*)((TObjArray*)arr->At(9))->At(ily))->Fill(1./pt0, (1./pt-1./pt0)/TMath::Sqrt(covR[6]), sign*sIdx);
      ((TH3S*)((TObjArray*)arr->At(10))->At(ily))->Fill(p0, p/p0-1., sign*sIdx);
    } else {
      ((TH3S*)((TObjArray*)arr->At(11))->At(ily))->Fill(pt0, pt/pt0-1., sign*sIdx);
      ((TH3S*)((TObjArray*)arr->At(12))->At(ily))->Fill(1./pt0, (1./pt-1./pt0)/TMath::Sqrt(covR[6]), sign*sIdx);
      ((TH3S*)((TObjArray*)arr->At(13))->At(ily))->Fill(p0, p/p0-1., sign*sIdx);
    }

    // Fill Debug stream for Kalman track
    if(DebugLevel()>=1){
      (*DebugStream()) << "MCtrack"
        << "pt="      << pt
        << "x="       << x
        << "y="       << y
        << "z="       << z
        << "dydx="    << dydx
        << "dzdx="    << dzdx
        << "s2y="     << covR[0]
        << "s2z="     << covR[2]
        << "\n";
    }

    // recalculate tracklet based on the MC info
    AliTRDseedV1 tt(*fTracklet);
    tt.SetZref(0, z0 - (x0-xAnode)*dzdx0);
    tt.SetZref(1, dzdx0); 
    tt.SetReconstructor(&rec);
    tt.Fit(kTRUE, kTRUE);
    x= tt.GetX();y= tt.GetY();z= tt.GetZ();
    dydx = tt.GetYfit(1);
    dx = x0 - x;
    ymc = y0 - dx*dydx0;
    zmc = z0 - dx*dzdx0;
    Bool_t rc = tt.IsRowCross(); 
    
    // add tracklet residuals for y and dydx
    arr = (TObjArray*)fContainer->At(kMCtracklet);
    if(!rc){
      dy    = y-ymc;

      Float_t dphi  = (dydx - dydx0);
      dphi /= (1.- dydx*dydx0);

      ((TH3S*)arr->At(0))->Fill(dydx0, dy, pt0);
      if(tt.GetS2Y()>0.) ((TH2I*)arr->At(1))->Fill(dydx0, dy/TMath::Sqrt(tt.GetS2Y()));
      ((TH2I*)arr->At(4))->Fill(dydx0, TMath::ATan(dphi));
    } else {
      // add tracklet residuals for z
      dz = z-zmc;
      ((TH2I*)arr->At(2))->Fill(dzdl0, dz);
      if(tt.GetS2Z()>0.) ((TH2I*)arr->At(3))->Fill(dzdl0, dz/TMath::Sqrt(tt.GetS2Z()));
    }
  
    // Fill Debug stream for tracklet
    if(DebugLevel()>=1){
      Float_t s2y = tt.GetS2Y();
      Float_t s2z = tt.GetS2Z();
      (*DebugStream()) << "MCtracklet"
        << "rc="    << rc
        << "x="     << x
        << "y="     << y
        << "z="     << z
        << "dydx="  << dydx
        << "s2y="   << s2y
        << "s2z="   << s2z
        << "\n";
    }

    Int_t istk = AliTRDgeometry::GetStack(det); 
    AliTRDpadPlane *pp = fGeo->GetPadPlane(ily, istk);
    Float_t zr0 = pp->GetRow0() + pp->GetAnodeWireOffset();
    Float_t tilt = fTracklet->GetTilt();
    //Double_t exb = AliTRDCommonParam::Instance()->GetOmegaTau(1.5);

    arr = (TObjArray*)fContainer->At(kMCcluster);
    AliTRDcluster *c = NULL;
    fTracklet->ResetClusterIter(kFALSE);
    while((c = fTracklet->PrevCluster())){
      Float_t  q = TMath::Abs(c->GetQ());
      x = c->GetX(); y = c->GetY();z = c->GetZ();
      dx = x0 - x; 
      ymc= y0 - dx*dydx0;
      zmc= z0 - dx*dzdx0;
      dy = (y - tilt*(z-zmc)) - ymc;
      // Fill Histograms
      if(q>20. && q<250.){ 
        ((TH3S*)arr->At(0))->Fill(dydx0, dy, pt0);
        ((TH2I*)arr->At(1))->Fill(dydx0, dy/TMath::Sqrt(c->GetSigmaY2()));
      }

      // Fill calibration container
      Float_t d = zr0 - zmc;
      d -= ((Int_t)(2 * d)) / 2.0;
      if (d > 0.25) d  = 0.5 - d;
      AliTRDclusterInfo *clInfo = new AliTRDclusterInfo;
      clInfo->SetCluster(c);
      clInfo->SetMC(pdg, label);
      clInfo->SetGlobalPosition(ymc, zmc, dydx0, dzdx0);
      clInfo->SetResolution(dy);
      clInfo->SetAnisochronity(d);
      clInfo->SetDriftLength(dx-.5*AliTRDgeometry::CamHght());
      clInfo->SetTilt(tilt);
      fMCcl->Add(clInfo);
      if(DebugLevel()>=2){ 
        if(!clInfoArr) clInfoArr=new TObjArray(AliTRDseedV1::kNclusters);
        clInfoArr->Add(clInfo);
      }
    }
    // Fill Debug Tree
    if(DebugLevel()>=2 && clInfoArr){
      (*DebugStream()) << "MCcluster"
        <<"clInfo.=" << clInfoArr
        << "\n";
      clInfoArr->Clear();
    }
  }
  if(clInfoArr) delete clInfoArr;
  return h;
}


//________________________________________________________
Bool_t AliTRDresolution::GetRefFigure(Int_t ifig)
{
  //
  // Get the reference figures
  //

  Float_t xy[4] = {0., 0., 0., 0.};
  if(!gPad){
    AliWarning("Please provide a canvas to draw results.");
    return kFALSE;
  }
  TList *l = NULL; TVirtualPad *pad=NULL;
  switch(ifig){
  case kCharge:
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    ((TVirtualPad*)l->At(0))->cd();
    ((TGraphAsymmErrors*)((TObjArray*)fGraphM->At(kCharge))->At(0))->Draw("apl");
    ((TVirtualPad*)l->At(1))->cd();
    ((TGraphErrors*)((TObjArray*)fGraphS->At(kCharge))->At(0))->Draw("apl");
    break;
  case kCluster:
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0] = -.3; xy[1] = -200.; xy[2] = .3; xy[3] = 1000.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kCluster, 0)) break;
    xy[0] = -.3; xy[1] = -0.5; xy[2] = .3; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kCluster, 1)) break;
    return kTRUE;
  case kTrackTRD :
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0] = -.3; xy[1] = -500.; xy[2] = .3; xy[3] = 1500.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kTrackTRD , 0)) break;
    xy[0] = -.3; xy[1] = -0.5; xy[2] = .3; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kTrackTRD , 1)) break;
    return kTRUE;
  case 3: // kTrackTRD  z
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0] = -1.; xy[1] = -1000.; xy[2] = 1.; xy[3] = 4000.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kTrackTRD , 2)) break;
    xy[0] = -1.; xy[1] = -0.5; xy[2] = 1.; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kTrackTRD , 3)) break;
    return kTRUE;
  case 4: // kTrackTRD  phi
    xy[0] = -.3; xy[1] = -5.; xy[2] = .3; xy[3] = 50.;
    if(GetGraphPlot(&xy[0], kTrackTRD , 4)) return kTRUE;
    break;
  case 5: // kTrackTPC y
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0] = -.3; xy[1] = -500.; xy[2] = .3; xy[3] = 1500.;
    pad = ((TVirtualPad*)l->At(0)); pad->cd();
    pad->SetMargin(0.1, 0.1, 0.1, 0.01);
    if(!GetGraphPlot(&xy[0], kTrackTPC, 0)) break;
    xy[0] = -.3; xy[1] = -0.5; xy[2] = .3; xy[3] = 2.5;
    pad=((TVirtualPad*)l->At(1)); pad->cd();
    pad->SetMargin(0.1, 0.1, 0.1, 0.01);
    if(!GetGraphPlot(&xy[0], kTrackTPC, 1)) break;
    return kTRUE;
  case 6: // kTrackTPC z
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0] = -1.; xy[1] = -1000.; xy[2] = 1.; xy[3] = 4000.;
    pad = ((TVirtualPad*)l->At(0)); pad->cd();
    pad->SetMargin(0.1, 0.1, 0.1, 0.01);
    if(!GetGraphPlot(&xy[0], kTrackTPC, 2)) break;
    xy[0] = -1.; xy[1] = -0.5; xy[2] = 1.; xy[3] = 2.5;
    pad = ((TVirtualPad*)l->At(1)); pad->cd();
    pad->SetMargin(0.1, 0.1, 0.1, 0.01);
    if(!GetGraphPlot(&xy[0], kTrackTPC, 3)) break;
    return kTRUE;
  case 7: // kTrackTPC phi
    xy[0] = -.3; xy[1] = -5.; xy[2] = .3; xy[3] = 50.;
    if(GetGraphPlot(&xy[0], kTrackTPC, 4)) return kTRUE;
    break;
  case 8: // kMCcluster pt <2 GeV/c
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-.3; xy[1]=-50.; xy[2]=.3; xy[3]=650.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCcluster, 0)) break;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCcluster, 1)) break;
    return kTRUE;
  case 9: // kMCcluster pt > 3 GeV/c
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-.3; xy[1]=-50.; xy[2]=.3; xy[3]=650.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCcluster, 2)) break;
    xy[0] = -.3; xy[1] = -0.5; xy[2] = .3; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCcluster, 3)) break;
    return kTRUE;
  case 10: //kMCtracklet [y] pt < 3 GeV/c
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-.2; xy[1]=-50.; xy[2]=.2; xy[3] =250.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtracklet, 0)) break;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtracklet, 1)) break;
    return kTRUE;
  case 11: //kMCtracklet [y] pt > 3 GeV/c
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-.2; xy[1]=-50.; xy[2]=.2; xy[3] =250.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtracklet, 2)) break;
    xy[0] = -.2; xy[1] = -0.5; xy[2] = .2; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtracklet, 3)) break;
    return kTRUE;
  case 12: //kMCtracklet [z]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-1.; xy[1]=-100.; xy[2]=1.; xy[3] =2500.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtracklet, 4)) break;
    xy[0] = -1.; xy[1] = -0.5; xy[2] = 1.; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtracklet, 5)) break;
    return kTRUE;
  case 13: //kMCtracklet [phi]
    xy[0]=-.3; xy[1]=-3.; xy[2]=.3; xy[3] =25.;
    if(!GetGraphPlot(&xy[0], kMCtracklet, 6)) break;
    return kTRUE;
  case 14: //kMCtrackTRD [y]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-.2; xy[1]=-50.; xy[2]=.2; xy[3] =200.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTRD, 0)) break;
    xy[0] = -.2; xy[1] = -0.5; xy[2] = .2; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTRD, 1)) break;
    return kTRUE;
  case 15: //kMCtrackTRD [z]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-1.; xy[1]=-700.; xy[2]=1.; xy[3] =1500.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTRD, 2)) break;
    xy[0] = -1.; xy[1] = -0.5; xy[2] = 1.; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTRD, 3)) break;
    return kTRUE;
  case 16: //kMCtrackTRD [phi/snp]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-.2; xy[1]=-0.2; xy[2]=.2; xy[3] =2.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTRD, 4)) break;
    xy[0] = -.2; xy[1] = -0.5; xy[2] = .2; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTRD, 5)) break;
    return kTRUE;
  case 17: //kMCtrackTRD [theta/tgl]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-1.; xy[1]=-0.5; xy[2]=1.; xy[3] =5.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTRD, 6)) break;
    xy[0] = -.2; xy[1] = -0.5; xy[2] = .2; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTRD, 7)) break;
    return kTRUE;
  case 18: //kMCtrackTRD [pt]
    xy[0] = 0.4; xy[1] = -.7; xy[2] = 12.; xy[3] = 4.;
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    pad = (TVirtualPad*)l->At(0); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 8, 2, 7)) break;
    pad->Modified(); pad->SetLogx();
    pad = (TVirtualPad*)l->At(1); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 8, 55+2, 7, kTRUE)) break;
    pad->Modified(); pad->SetLogx();
    return kTRUE;
  case 19: //kMCtrackTRD [1/pt] pulls
    xy[0] = 0.; xy[1] = -1.5; xy[2] = 2.; xy[3] = 2.;
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    pad = (TVirtualPad*)l->At(0); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 9, 2, 7)) break;
    pad = (TVirtualPad*)l->At(1); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 9, 55+2, 7, kTRUE)) break;
    return kTRUE;
  case 20: //kMCtrackTRD [p]
    xy[0] = 0.4; xy[1] = -.7; xy[2] = 12.; xy[3] = 4.;
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    pad = (TVirtualPad*)l->At(0); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 10, 2, 7)) break;
    pad->SetLogx();
    pad = (TVirtualPad*)l->At(1); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 10, 55+2, 7, kTRUE)) break;
    pad->SetLogx();
    return kTRUE;
  case 21: //kMCtrackTRD - SA [pt]
    xy[0] = 0.; xy[1] = -5.; xy[2] = 12.; xy[3] = 7.;
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    pad = (TVirtualPad*)l->At(0); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 11, 2, 7)) break;
    pad->SetLogx();
    pad = (TVirtualPad*)l->At(1); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 11, 55+2, 7, kTRUE)) break;
    pad->SetLogx();
    return kTRUE;
  case 22: //kMCtrackTRD - SA [1/pt] pulls
    xy[0] = 0.; xy[1] = -1.5; xy[2] = 2.; xy[3] = 2.;
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    pad = (TVirtualPad*)l->At(0); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 12, 2, 7)) break;
    pad = (TVirtualPad*)l->At(1); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 12, 55+2, 7, kTRUE)) break;
    return kTRUE;
  case 23: //kMCtrackTRD - SA [p]
    xy[0] = 0.; xy[1] = -7.5; xy[2] = 12.; xy[3] = 10.5;
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    pad = (TVirtualPad*)l->At(0); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 13, 2, 7)) break;
    pad->SetLogx();
    pad = (TVirtualPad*)l->At(1); pad->cd();
    pad->SetMargin(0.07, 0.07, 0.1, 0.1);
    if(!GetGraphTrack(&xy[0], 13, 55+2, 7, kTRUE)) break;
    pad->SetLogx();
    return kTRUE;
  case 24: // kMCtrackTPC [y]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-.25; xy[1]=-50.; xy[2]=.25; xy[3] =800.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTPC, 0)) break;
    xy[0] = -.25; xy[1] = -0.5; xy[2] = .25; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTPC, 1)) break;
    return kTRUE;
  case 25: // kMCtrackTPC [z]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-1.; xy[1]=-500.; xy[2]=1.; xy[3] =800.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTPC, 2)) break;
    xy[0] = -1.; xy[1] = -0.5; xy[2] = 1.; xy[3] = 2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTPC, 3)) break;
    return kTRUE;
  case 26: // kMCtrackTPC [phi|snp]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-.25; xy[1]=-0.5; xy[2]=.25; xy[3] =2.5;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTPC, 4)) break;
    xy[0] = -.25; xy[1] = -0.5; xy[2] = .25; xy[3] = 1.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTPC, 5)) break;
    return kTRUE;
  case 27: // kMCtrackTPC [theta|tgl]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0]=-1.; xy[1]=-1.; xy[2]=1.; xy[3] =4.;
    ((TVirtualPad*)l->At(0))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTPC, 6)) break;
    xy[0] = -1.; xy[1] = -0.5; xy[2] = 1.; xy[3] = 1.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphPlot(&xy[0], kMCtrackTPC, 7)) break;
    return kTRUE;
  case 28: // kMCtrackTPC [pt]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0] = 0.4; xy[1] = -.8; xy[2] = 12.; xy[3] = 6.;
    pad=(TVirtualPad*)l->At(0); pad->cd();
    if(!GetGraphTrackTPC(xy, 8, 2, 7)) break;
    pad->SetLogx();
    xy[0]=0.; xy[1]=-0.5; xy[2]=2.; xy[3] =2.5;
    ((TVirtualPad*)l->At(1))->cd();
    if(!GetGraphTrackTPC(xy, 9, 2, 7, kTRUE)) break;
    return kTRUE;
  case 29: // kMCtrackTPC [p]
    gPad->Divide(2, 1, 1.e-5, 1.e-5); l=gPad->GetListOfPrimitives(); 
    xy[0] = 0.4; xy[1] = -.8; xy[2] = 12.; xy[3] = 6.;
    pad = ((TVirtualPad*)l->At(0));pad->cd();
    pad->SetMargin(0.12, 0.12, 0.1, 0.04);
    if(!GetGraphTrackTPC(xy, 10, 2, 7)) break;
    pad->SetLogx();
    xy[0]=0.; xy[1]=-1.5; xy[2]=12.; xy[3] =2.5;
    pad = ((TVirtualPad*)l->At(1)); pad->cd();
    pad->SetMargin(0.12, 0.12, 0.1, 0.04);
    if(!GetGraphTrackTPC(xy, 11, 2, 7, kTRUE)) break;
    return kTRUE;
  case 30:  // kMCtrackTOF [z]
    return kTRUE;
  }
  AliWarning(Form("Reference plot [%d] missing result", ifig));
  return kFALSE;
}

Char_t const *fgParticle[11]={
  " p bar", " K -", " #pi -", " #mu -", " e -",
  " No PID",
  " e +", " #mu +", " #pi +", " K +", " p",
};
//________________________________________________________
Bool_t AliTRDresolution::PostProcess()
{
  //fContainer = dynamic_cast<TObjArray*>(GetOutputData(0));
  if (!fContainer) {
    AliError("ERROR: list not available");
    return kFALSE;
  }
  Int_t nc(0);
  TGraph *gm= NULL, *gs= NULL;
  if(!fGraphS && !fGraphM){ 
    TObjArray *aM(NULL), *aS(NULL), *a(NULL);
    Int_t n = fContainer->GetEntriesFast();
    fGraphS = new TObjArray(n); fGraphS->SetOwner();
    fGraphM = new TObjArray(n); fGraphM->SetOwner();
    for(Int_t ig=0; ig<n; ig++){
      fGraphM->AddAt(aM = new TObjArray(fgNproj[ig]), ig);
      fGraphS->AddAt(aS = new TObjArray(fgNproj[ig]), ig);

      for(Int_t ic=0; ic<fgNproj[ig]; ic++){
        //printf("g[%d] c[%d] n[%d]\n", ig, ic, fgNcomp[nc]);
        if(fgNcomp[nc]>1){
          aS->AddAt(a = new TObjArray(fgNcomp[nc]), ic); 
          for(Int_t is=fgNcomp[nc]; is--;){
            a->AddAt(gs = new TGraphErrors(), is);
            if(is%11 == 4){
              gs->SetMarkerStyle(20);
              gs->SetMarkerColor(kMagenta);
              gs->SetLineColor(kMagenta);
           } else if(is%11 == 6){
              gs->SetMarkerStyle(4);
              gs->SetMarkerColor(kMagenta);
              gs->SetLineColor(kMagenta);
            } else {
              gs->SetMarkerStyle(7);
              gs->SetMarkerColor(kRed);
              gs->SetLineColor(kRed);
              gs->SetLineStyle(2);
            }
            gs->SetNameTitle(Form("s_%d%02d%02d", ig, ic, is), fgParticle[is%11]);
          }
          aM->AddAt(a = new TObjArray(fgNcomp[nc]), ic); 
          for(Int_t is=fgNcomp[nc]; is--;){
            a->AddAt(gm = new TGraphErrors(), is);
            if(is%11 == 4){
              gm->SetMarkerStyle(20);
              gm->SetMarkerColor(kBlue);
              gm->SetLineColor(kBlue);
            } else if(is%11 == 6){
              gm->SetMarkerStyle(4);
              gm->SetMarkerColor(kBlue);
              gm->SetLineColor(kBlue);
            } else {
              gm->SetMarkerStyle(7);
              gm->SetMarkerColor(kBlack);
              gm->SetLineColor(kBlack);
              gm->SetLineStyle(2);
            }
            gm->SetNameTitle(Form("m_%d%02d%02d", ig, ic, is), "");
          }
        } else {
          aS->AddAt(gs = new TGraphErrors(), ic);
          gs->SetMarkerStyle(23);
          gs->SetMarkerColor(kRed);
          gs->SetLineColor(kRed);
          gs->SetNameTitle(Form("s_%d%02d", ig, ic), "");
  
          aM->AddAt(gm = ig ? (TGraph*)new TGraphErrors() : (TGraph*)new TGraphAsymmErrors(), ic);
          gm->SetLineColor(kBlack);
          gm->SetMarkerStyle(7);
          gm->SetMarkerColor(kBlack);
          gm->SetNameTitle(Form("m_%d%02d", ig, ic), "");
        }
        nc+=fgNcomp[ic];
      }
    }
  }

/*  printf("\n\n\n"); fGraphS->ls();
  printf("\n\n\n"); fGraphM->ls();*/
  

  // DEFINE MODELS
  // simple gauss
  TF1 fg("fGauss", "gaus", -.5, .5);  
  // Landau for charge resolution
  TF1 fch("fClCh", "landau", 0., 1000.);  
  // Landau for e+- pt resolution
  TF1 fpt("fPt", "landau", -0.1, 0.2);  

  //PROCESS EXPERIMENTAL DISTRIBUTIONS
  // Charge resolution
  //Process3DL(kCharge, 0, &fl); 
  // Clusters residuals
  Process2D(kCluster, 0, &fg, 1.e4); 
  Process2D(kCluster, 1, &fg); 
  fNRefFigures = 1;
  // Tracklet residual/pulls
  Process2D(kTrackTRD , 0, &fg, 1.e4); 
  Process2D(kTrackTRD , 1, &fg); 
  Process2D(kTrackTRD , 2, &fg, 1.e4); 
  Process2D(kTrackTRD , 3, &fg); 
  Process2D(kTrackTRD , 4, &fg, 1.e3); 
  fNRefFigures = 4;
  // TPC track residual/pulls
  Process2D(kTrackTPC, 0, &fg, 1.e4); 
  Process2D(kTrackTPC, 1, &fg); 
  Process2D(kTrackTPC, 2, &fg, 1.e4); 
  Process2D(kTrackTPC, 3, &fg); 
  Process2D(kTrackTPC, 4, &fg, 1.e3); 
  fNRefFigures = 7;

  if(!HasMCdata()) return kTRUE;


  //PROCESS MC RESIDUAL DISTRIBUTIONS

  // CLUSTER Y RESOLUTION/PULLS
  Process3Drange(kMCcluster, 0, 0, &fg, 1.e4, 1, 1);
  Process3Drange(kMCcluster, 0, 1, &fg, 1.e4, 2, 2);
  Process3Drange(kMCcluster, 0, 2, &fg, 1.e4, 3, 12);
  Process2D(kMCcluster, 1, &fg, 1., 3);
  fNRefFigures = 10;

  // TRACKLET RESOLUTION/PULLS
  Process3Drange(kMCtracklet, 0, 0, &fg, 1.e4, 1, 1); // y
  Process3Drange(kMCtracklet, 0, 1, &fg, 1.e4, 2, 2); // y
  Process3Drange(kMCtracklet, 0, 2, &fg, 1.e4, 3, 12); // y
  Process2D(kMCtracklet, 1, &fg, 1., 3);       // y pulls
  Process2D(kMCtracklet, 2, &fg, 1.e4, 4); // z
  Process2D(kMCtracklet, 3, &fg, 1., 5);       // z pulls
  Process2D(kMCtracklet, 4, &fg, 1.e3, 6); // phi
  fNRefFigures = 14;

  // TRACK RESOLUTION/PULLS
  Process2D(kMCtrackTRD, 0, &fg, 1.e4);   // y
  Process2D(kMCtrackTRD, 1, &fg);         // y PULL
  Process2D(kMCtrackTRD, 2, &fg, 1.e4);   // z
  Process2D(kMCtrackTRD, 3, &fg);         // z PULL
  Process2D(kMCtrackTRD, 4, &fg, 1.e3);   // phi
  Process2D(kMCtrackTRD, 5, &fg);         // snp PULL
  Process2D(kMCtrackTRD, 6, &fg, 1.e3);   // theta
  Process2D(kMCtrackTRD, 7, &fg);         // tgl PULL
  Process4D(kMCtrackTRD, 8, &fg, 1.e2);   // pt resolution
  Process4D(kMCtrackTRD, 8, &fpt, 1.e2, 4);// pt resolution e1- @ L0
  Process4D(kMCtrackTRD, 8, &fpt, 1.e2, 6);// pt resolution e1+ @ L0
  Process4D(kMCtrackTRD, 8, &fpt, 1.e2, 55+4);// pt resolution e1- @ L5
  Process4D(kMCtrackTRD, 8, &fpt, 1.e2, 55+6);// pt resolution e1+ @ L5
  Process4D(kMCtrackTRD, 9, &fg);         // 1/pt pulls
  Process4D(kMCtrackTRD, 10, &fg, 1.e2);  // p resolution
  Process4D(kMCtrackTRD, 10, &fpt, 1.e2, 4);// p resolution e1- @ L0
  Process4D(kMCtrackTRD, 10, &fpt, 1.e2, 6);// p resolution e1+ @ L0
  Process4D(kMCtrackTRD, 10, &fpt, 1.e2, 55+4);// p resolution e1- @ L5
  Process4D(kMCtrackTRD, 10, &fpt, 1.e2, 55+6);// p resolution e1+ @ L5
  Process4D(kMCtrackTRD, 11, &fg, 1.e2);   // pt resolution stand alone
  Process4D(kMCtrackTRD, 12, &fg);         // 1/pt pulls  stand alone
  Process4D(kMCtrackTRD, 13, &fg, 1.e2);  // p resolution  stand alone
  fNRefFigures = 24;

  // TRACK TPC RESOLUTION/PULLS
  Process2D(kMCtrackTPC, 0, &fg, 1.e4);// y resolution
  Process2D(kMCtrackTPC, 1, &fg);      // y pulls
  Process2D(kMCtrackTPC, 2, &fg, 1.e4);// z resolution
  Process2D(kMCtrackTPC, 3, &fg);      // z pulls
  Process2D(kMCtrackTPC, 4, &fg, 1.e3);// phi resolution
  Process2D(kMCtrackTPC, 5, &fg);      // snp pulls
  Process2D(kMCtrackTPC, 6, &fg, 1.e3);// theta resolution
  Process2D(kMCtrackTPC, 7, &fg);      // tgl pulls
  Process3D(kMCtrackTPC, 8, &fg, 1.e2);// pt resolution
  Process3D(kMCtrackTPC, 9, &fg);      // 1/pt pulls
  Process3D(kMCtrackTPC, 10, &fg, 1.e2);// p resolution
  Process3D(kMCtrackTPC, 11, &fg);      // p pulls
  fNRefFigures = 30;

  // TRACK HMPID RESOLUTION/PULLS
  Process2D(kMCtrackTOF, 0, &fg, 1.e4); // z towards TOF
  Process2D(kMCtrackTOF, 1, &fg);       // z towards TOF
  fNRefFigures = 31;

  return kTRUE;
}


//________________________________________________________
void AliTRDresolution::Terminate(Option_t *opt)
{
  AliTRDrecoTask::Terminate(opt);
  if(HasPostProcess()) PostProcess();
}

//________________________________________________________
void AliTRDresolution::AdjustF1(TH1 *h, TF1 *f)
{
// Helper function to avoid duplication of code
// Make first guesses on the fit parameters

  // find the intial parameters of the fit !! (thanks George)
  Int_t nbinsy = Int_t(.5*h->GetNbinsX());
  Double_t sum = 0.;
  for(Int_t jbin=nbinsy-4; jbin<=nbinsy+4; jbin++) sum+=h->GetBinContent(jbin); sum/=9.;
  f->SetParLimits(0, 0., 3.*sum);
  f->SetParameter(0, .9*sum);
  Double_t rms = h->GetRMS();
  f->SetParLimits(1, -rms, rms);
  f->SetParameter(1, h->GetMean());

  f->SetParLimits(2, 0., 2.*rms);
  f->SetParameter(2, rms);
  if(f->GetNpar() <= 4) return;

  f->SetParLimits(3, 0., sum);
  f->SetParameter(3, .1*sum);

  f->SetParLimits(4, -.3, .3);
  f->SetParameter(4, 0.);

  f->SetParLimits(5, 0., 1.e2);
  f->SetParameter(5, 2.e-1);
}

//________________________________________________________
TObjArray* AliTRDresolution::Histos()
{
  //
  // Define histograms
  //

  if(fContainer) return fContainer;

  fContainer  = new TObjArray(kNviews);
  //fContainer->SetOwner(kTRUE);
  TH1 *h = NULL;
  TObjArray *arr = NULL;

  const Int_t kPbins(12); // binning in momentum range should depend on the statistics analyzed

  // cluster to track residuals/pulls
  fContainer->AddAt(arr = new TObjArray(fgNhistos[kCharge]), kCharge);
  arr->SetName("Charge");
  if(!(h = (TH3S*)gROOT->FindObject("hCharge"))){
    h = new TH3S("hCharge", "Charge Resolution", 20, 1., 2., 24, 0., 3.6, 100, 0., 500.);
    h->GetXaxis()->SetTitle("dx/dx_{0}");
    h->GetYaxis()->SetTitle("x_{d} [cm]");
    h->GetZaxis()->SetTitle("dq/dx [ADC/cm]");
  } else h->Reset();
  arr->AddAt(h, 0);

  // cluster to track residuals/pulls
  fContainer->AddAt(arr = new TObjArray(fgNhistos[kCluster]), kCluster);
  arr->SetName("Cl");
  if(!(h = (TH2I*)gROOT->FindObject("hCl"))){
    h = new TH2I("hCl", "Cluster Residuals", 21, -.33, .33, 100, -.5, .5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 0);
  if(!(h = (TH2I*)gROOT->FindObject("hClpull"))){
    h = new TH2I("hClpull", "Cluster Pulls", 21, -.33, .33, 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y/#sigma_{y}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 1);

  // tracklet to track residuals/pulls in y direction
  fContainer->AddAt(arr = new TObjArray(fgNhistos[kTrackTRD ]), kTrackTRD );
  arr->SetName("Trklt");
  if(!(h = (TH2I*)gROOT->FindObject("hTrkltY"))){
    h = new TH2I("hTrkltY", "Tracklet Y Residuals", 21, -.33, .33, 100, -.5, .5);
    h->GetXaxis()->SetTitle("#tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 0);
  if(!(h = (TH2I*)gROOT->FindObject("hTrkltYpull"))){
    h = new TH2I("hTrkltYpull", "Tracklet Y Pulls", 21, -.33, .33, 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("#tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y/#sigma_{y}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 1);
  // tracklet to track residuals/pulls in z direction
  if(!(h = (TH2I*)gROOT->FindObject("hTrkltZ"))){
    h = new TH2I("hTrkltZ", "Tracklet Z Residuals", 50, -1., 1., 100, -1.5, 1.5);
    h->GetXaxis()->SetTitle("#tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 2);
  if(!(h = (TH2I*)gROOT->FindObject("hTrkltZpull"))){
    h = new TH2I("hTrkltZpull", "Tracklet Z Pulls", 50, -1., 1., 100, -5.5, 5.5);
    h->GetXaxis()->SetTitle("#tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z/#sigma_{z}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 3);
  // tracklet to track phi residuals
  if(!(h = (TH2I*)gROOT->FindObject("hTrkltPhi"))){
    h = new TH2I("hTrkltPhi", "Tracklet #phi Residuals", 21, -.33, .33, 100, -.5, .5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta phi [rad]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 4);


  // tracklet to TPC track residuals/pulls in y direction
  fContainer->AddAt(arr = new TObjArray(fgNhistos[kTrackTPC]), kTrackTPC);
  arr->SetName("TrkTPC");
  if(!(h = (TH2I*)gROOT->FindObject("hTrkTPCY"))){
    h = new TH2I("hTrkTPCY", "Track[TPC] Y Residuals", 21, -.33, .33, 100, -.5, .5);
    h->GetXaxis()->SetTitle("#tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 0);
  if(!(h = (TH2I*)gROOT->FindObject("hTrkTPCYpull"))){
    h = new TH2I("hTrkTPCYpull", "Track[TPC] Y Pulls", 21, -.33, .33, 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("#tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y/#sigma_{y}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 1);
  // tracklet to TPC track residuals/pulls in z direction
  if(!(h = (TH2I*)gROOT->FindObject("hTrkTPCZ"))){
    h = new TH2I("hTrkTPCZ", "Track[TPC] Z Residuals", 50, -1., 1., 100, -1.5, 1.5);
    h->GetXaxis()->SetTitle("#tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 2);
  if(!(h = (TH2I*)gROOT->FindObject("hTrkTPCZpull"))){
    h = new TH2I("hTrkTPCZpull", "Track[TPC] Z Pulls", 50, -1., 1., 100, -5.5, 5.5);
    h->GetXaxis()->SetTitle("#tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z/#sigma_{z}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 3);
  // tracklet to TPC track phi residuals
  if(!(h = (TH2I*)gROOT->FindObject("hTrkTPCPhi"))){
    h = new TH2I("hTrkTPCPhi", "Track[TPC] #phi Residuals", 21, -.33, .33, 100, -.5, .5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta phi [rad]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 4);


  // Resolution histos
  if(!HasMCdata()) return fContainer;

  // cluster y resolution [0]
  fContainer->AddAt(arr = new TObjArray(fgNhistos[kMCcluster]), kMCcluster);
  arr->SetName("McCl");
  if(!(h = (TH3S*)gROOT->FindObject("hMcCl"))){
    h = new TH3S("hMcCl", "Cluster Resolution", 48, -.48, .48, 100, -.3, .3, kPbins, 0., 12.);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y [cm]");
    h->GetZaxis()->SetTitle("p_{t} [GeV/c]");
  } else h->Reset();
  arr->AddAt(h, 0);
  if(!(h = (TH2I*)gROOT->FindObject("hMcClPull"))){
    h = new TH2I("hMcClPull", "Cluster Pulls", 48, -.48, .48, 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Deltay/#sigma_{y}");
    h->GetZaxis()->SetTitle("p_{t} [GeV/c]");
  } else h->Reset();
  arr->AddAt(h, 1);


  // TRACKLET RESOLUTION
  fContainer->AddAt(arr = new TObjArray(fgNhistos[kMCtracklet]), kMCtracklet);
  arr->SetName("McTrklt");
  // tracklet y resolution
  if(!(h = (TH3S*)gROOT->FindObject("hMcTrkltY"))){
    h = new TH3S("hMcTrkltY", "Tracklet Resolution (Y)", 48, -.48, .48, 100, -.2, .2, kPbins, 0., 12.);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 0);
  // tracklet y pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkltYPull"))){
    h = new TH2I("hMcTrkltYPull", "Tracklet Pulls (Y)", 48, -.48, .48, 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y / #sigma_{y}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 1);
  // tracklet z resolution
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkltZ"))){
    h = new TH2I("hMcTrkltZ", "Tracklet Resolution (Z)", 100, -1., 1., 100, -1., 1.);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 2);
  // tracklet z pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkltZPull"))){
    h = new TH2I("hMcTrkltZPull", "Tracklet Pulls (Z)", 100, -1., 1., 100, -3.5, 3.5);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z / #sigma_{z}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 3);
  // tracklet phi resolution
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkltPhi"))){
    h = new TH2I("hMcTrkltPhi", "Tracklet Resolution (#Phi)", 48, -.48, .48, 100, -.15, .15);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta #phi [rad]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 4);


  // KALMAN TRACK RESOLUTION
  fContainer->AddAt(arr = new TObjArray(fgNhistos[kMCtrackTRD]), kMCtrackTRD);
  arr->SetName("McTrkTRD");
  // Kalman track y resolution
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkY"))){
    h = new TH2I("hMcTrkY", "Track Y Resolution", 48, -.48, .48, 100, -.2, .2);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 0);
  // Kalman track y pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkYPull"))){
    h = new TH2I("hMcTrkYPull", "Track Y Pulls", 48, -.48, .48, 100, -4., 4.);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y / #sigma_{y}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 1);
  // Kalman track Z
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkZ"))){
    h = new TH2I("hMcTrkZ", "Track Z Resolution", 100, -1., 1., 100, -1., 1.);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 2);
  // Kalman track Z pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkZPull"))){
    h = new TH2I("hMcTrkZPull", "Track Z Pulls", 100, -1., 1., 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z / #sigma_{z}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 3);
  // Kalman track SNP
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkSNP"))){
    h = new TH2I("hMcTrkSNP", "Track Phi Resolution", 60, -.3, .3, 100, -5e-3, 5e-3);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta #phi [rad]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 4);
  // Kalman track SNP pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkSNPPull"))){
    h = new TH2I("hMcTrkSNPPull", "Track SNP Pulls", 60, -.3, .3, 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta(sin(#phi)) / #sigma_{sin(#phi)}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 5);
  // Kalman track TGL
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTGL"))){
    h = new TH2I("hMcTrkTGL", "Track Theta Resolution", 100, -1., 1., 100, -5e-3, 5e-3);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta#theta [rad]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 6);
  // Kalman track TGL pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTGLPull"))){
    h = new TH2I("hMcTrkTGLPull", "Track TGL  Pulls", 100, -1., 1., 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta(tg(#theta)) / #sigma_{tg(#theta)}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 7);
  const Int_t n = 2*AliPID::kSPECIES+1;
  TObjArray *arr2 = NULL; TH3S* h3=NULL;
  // Kalman track Pt resolution
  arr->AddAt(arr2 = new TObjArray(AliTRDgeometry::kNlayer), 8);
  arr2->SetName("Pt Resolution");
  for(Int_t il=0; il<AliTRDgeometry::kNlayer; il++){
    if(!(h3 = (TH3S*)gROOT->FindObject(Form("hMcTrkPt%d", il)))){
      h3 = new TH3S(Form("hMcTrkPt%d", il), "Track Pt Resolution", kPbins, 0., 12., 150, -.1, .2, n, -5.5, 5.5);
      h3->GetXaxis()->SetTitle("p_{t} [GeV/c]");
      h3->GetYaxis()->SetTitle("#Delta p_{t}/p_{t}^{MC}");
      h3->GetZaxis()->SetTitle("SPECIES");
    } else h3->Reset();
    arr2->AddAt(h3, il);
  }
  // Kalman track Pt pulls
  arr->AddAt(arr2 = new TObjArray(AliTRDgeometry::kNlayer), 9);
  arr2->SetName("1/Pt Pulls");
  for(Int_t il=0; il<AliTRDgeometry::kNlayer; il++){
    if(!(h3 = (TH3S*)gROOT->FindObject(Form("hMcTrkPtPulls%d", il)))){
      h3 = new TH3S(Form("hMcTrkPtPulls%d", il), "Track 1/Pt Pulls", kPbins, 0., 2., 100, -4., 4., n, -5.5, 5.5);
      h3->GetXaxis()->SetTitle("1/p_{t}^{MC} [c/GeV]");
      h3->GetYaxis()->SetTitle("#Delta(1/p_{t})/#sigma(1/p_{t}) ");
      h3->GetZaxis()->SetTitle("SPECIES");
    } else h3->Reset();
    arr2->AddAt(h3, il);
  }
  // Kalman track P resolution
  arr->AddAt(arr2 = new TObjArray(AliTRDgeometry::kNlayer), 10);
  arr2->SetName("P Resolution");
  for(Int_t il=0; il<AliTRDgeometry::kNlayer; il++){
    if(!(h3 = (TH3S*)gROOT->FindObject(Form("hMcTrkP%d", il)))){
      h3 = new TH3S(Form("hMcTrkP%d", il), "Track P Resolution", kPbins, 0., 12., 150, -.15, .35, n, -5.5, 5.5);
      h3->GetXaxis()->SetTitle("p [GeV/c]");
      h3->GetYaxis()->SetTitle("#Delta p/p^{MC}");
      h3->GetZaxis()->SetTitle("SPECIES");
    } else h3->Reset();
    arr2->AddAt(h3, il);
  }
  // TRD stand-alone track Pt resolution
  arr->AddAt(arr2 = new TObjArray(AliTRDgeometry::kNlayer), 11);
  arr2->SetName("Pt Resolution [SA]");
  for(Int_t il=0; il<AliTRDgeometry::kNlayer; il++){
    if(!(h3 = (TH3S*)gROOT->FindObject(Form("hMcSATrkPt%d", il)))){
      h3 = new TH3S(Form("hMcSATrkPt%d", il), "Track Pt Resolution", kPbins, 0., 12., 150, -1., 2., n, -5.5, 5.5);
      h3->GetXaxis()->SetTitle("p_{t} [GeV/c]");
      h3->GetYaxis()->SetTitle("#Delta p_{t}/p_{t}^{MC}");
      h3->GetZaxis()->SetTitle("SPECIES");
    } else h3->Reset();
    arr2->AddAt(h3, il);
  }
  // TRD stand-alone track Pt pulls
  arr->AddAt(arr2 = new TObjArray(AliTRDgeometry::kNlayer), 12);
  arr2->SetName("1/Pt Pulls [SA]");
  for(Int_t il=0; il<AliTRDgeometry::kNlayer; il++){
    if(!(h3 = (TH3S*)gROOT->FindObject(Form("hMcSATrkPtPulls%d", il)))){
      h3 = new TH3S(Form("hMcSATrkPtPulls%d", il), "Track 1/Pt Pulls", kPbins, 0., 2., 100, -4., 4., n, -5.5, 5.5);
      h3->GetXaxis()->SetTitle("1/p_{t}^{MC} [c/GeV]");
      h3->GetYaxis()->SetTitle("#Delta(1/p_{t})/#sigma(1/p_{t}) ");
      h3->GetZaxis()->SetTitle("SPECIES");
    } else h3->Reset();
    arr2->AddAt(h3, il);
  }
  // TRD stand-alone track P resolution
  arr->AddAt(arr2 = new TObjArray(AliTRDgeometry::kNlayer), 13);
  arr2->SetName("P Resolution [SA]");
  for(Int_t il=0; il<AliTRDgeometry::kNlayer; il++){
    if(!(h3 = (TH3S*)gROOT->FindObject(Form("hMcSATrkP%d", il)))){
      h3 = new TH3S(Form("hMcSATrkP%d", il), "Track P Resolution", kPbins, 0., 12., 150, -1.5, 3.5, n, -5.5, 5.5);
      h3->GetXaxis()->SetTitle("p [GeV/c]");
      h3->GetYaxis()->SetTitle("#Delta p/p^{MC}");
      h3->GetZaxis()->SetTitle("SPECIES");
    } else h3->Reset();
    arr2->AddAt(h3, il);
  }

  // TPC TRACK RESOLUTION
  fContainer->AddAt(arr = new TObjArray(fgNhistos[kMCtrackTPC]), kMCtrackTPC);
  arr->SetName("McTrkTPC");
  // Kalman track Y
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTPCY"))){
    h = new TH2I("hMcTrkTPCY", "Track[TPC] Y Resolution", 60, -.3, .3, 100, -.5, .5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 0);
  // Kalman track Y pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTPCYPull"))){
    h = new TH2I("hMcTrkTPCYPull", "Track[TPC] Y Pulls", 60, -.3, .3, 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta y / #sigma_{y}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 1);
  // Kalman track Z
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTPCZ"))){
    h = new TH2I("hMcTrkTPCZ", "Track[TPC] Z Resolution", 100, -1., 1., 100, -1., 1.);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 2);
  // Kalman track Z pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTPCZPull"))){
    h = new TH2I("hMcTrkTPCZPull", "Track[TPC] Z Pulls", 100, -1., 1., 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z / #sigma_{z}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 3);
  // Kalman track SNP
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTPCSNP"))){
    h = new TH2I("hMcTrkTPCSNP", "Track[TPC] Phi Resolution", 60, -.3, .3, 100, -5e-3, 5e-3);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta #phi [rad]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 4);
  // Kalman track SNP pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTPCSNPPull"))){
    h = new TH2I("hMcTrkTPCSNPPull", "Track[TPC] SNP Pulls", 60, -.3, .3, 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#phi)");
    h->GetYaxis()->SetTitle("#Delta(sin(#phi)) / #sigma_{sin(#phi)}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 5);
  // Kalman track TGL
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTPCTGL"))){
    h = new TH2I("hMcTrkTPCTGL", "Track[TPC] Theta Resolution", 100, -1., 1., 100, -5e-3, 5e-3);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta#theta [rad]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 6);
  // Kalman track TGL pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTPCTGLPull"))){
    h = new TH2I("hMcTrkTPCTGLPull", "Track[TPC] TGL  Pulls", 100, -1., 1., 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta(tg(#theta)) / #sigma_{tg(#theta)}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 7);
  // Kalman track Pt resolution
  if(!(h3 = (TH3S*)gROOT->FindObject("hMcTrkTPCPt"))){
    h3 = new TH3S("hMcTrkTPCPt", "Track[TPC] Pt Resolution", kPbins, 0., 12., 100, -.1, .2, n, -5.5, 5.5);
    h3->GetXaxis()->SetTitle("p_{t} [GeV/c]");
    h3->GetYaxis()->SetTitle("#Delta p_{t}/p_{t}^{MC}");
    h3->GetZaxis()->SetTitle("SPECIES");
  } else h3->Reset();
  arr->AddAt(h3, 8);
  // Kalman track Pt pulls
  if(!(h3 = (TH3S*)gROOT->FindObject("hMcTrkTPCPtPulls"))){
    h3 = new TH3S("hMcTrkTPCPtPulls", "Track[TPC] 1/Pt Pulls", kPbins, 0., 2., 100, -4., 4., n, -5.5, 5.5);
    h3->GetXaxis()->SetTitle("1/p_{t}^{MC} [c/GeV]");
    h3->GetYaxis()->SetTitle("#Delta(1/p_{t})/#sigma(1/p_{t}) ");
    h3->GetZaxis()->SetTitle("SPECIES");
  } else h3->Reset();
  arr->AddAt(h3, 9);
  // Kalman track P resolution
  if(!(h3 = (TH3S*)gROOT->FindObject("hMcTrkTPCP"))){
    h3 = new TH3S("hMcTrkTPCP", "Track[TPC] P Resolution", kPbins, 0., 12., 100, -.15, .35, n, -5.5, 5.5);
    h3->GetXaxis()->SetTitle("p [GeV/c]");
    h3->GetYaxis()->SetTitle("#Delta p/p^{MC}");
    h3->GetZaxis()->SetTitle("SPECIES");
  } else h3->Reset();
  arr->AddAt(h3, 10);
  // Kalman track P pulls
  if(!(h3 = (TH3S*)gROOT->FindObject("hMcTrkTPCPPulls"))){
    h3 = new TH3S("hMcTrkTPCPPulls", "Track[TPC] P Pulls", kPbins, 0., 12., 100, -5., 5., n, -5.5, 5.5);
    h3->GetXaxis()->SetTitle("p^{MC} [GeV/c]");
    h3->GetYaxis()->SetTitle("#Deltap/#sigma_{p}");
    h3->GetZaxis()->SetTitle("SPECIES");
  } else h3->Reset();
  arr->AddAt(h3, 11);



  // Kalman track Z resolution [TOF]
  fContainer->AddAt(arr = new TObjArray(fgNhistos[kMCtrackTOF]), kMCtrackTOF);
  arr->SetName("McTrkTOF");
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTOFZ"))){
    h = new TH2I("hMcTrkTOFZ", "Track[TOF] Z Resolution", 100, -1., 1., 100, -1., 1.);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z [cm]");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 0);
  // Kalman track Z pulls
  if(!(h = (TH2I*)gROOT->FindObject("hMcTrkTOFZPull"))){
    h = new TH2I("hMcTrkTOFZPull", "Track[TOF] Z Pulls", 100, -1., 1., 100, -4.5, 4.5);
    h->GetXaxis()->SetTitle("tg(#theta)");
    h->GetYaxis()->SetTitle("#Delta z / #sigma_{z}");
    h->GetZaxis()->SetTitle("entries");
  } else h->Reset();
  arr->AddAt(h, 1);

  return fContainer;
}

//________________________________________________________
Bool_t AliTRDresolution::Process(TH2 * const h2, TF1 *f, Float_t k, TGraphErrors **g)
{
  //
  // Do the processing
  //

  Char_t pn[10]; sprintf(pn, "p%03d", fIdxPlot);
  Int_t n = 0;
  if((n=g[0]->GetN())) for(;n--;) g[0]->RemovePoint(n);
  if((n=g[1]->GetN())) for(;n--;) g[1]->RemovePoint(n);

  for(Int_t ibin = 1; ibin <= h2->GetNbinsX(); ibin++){
    Double_t x = h2->GetXaxis()->GetBinCenter(ibin);
    TH1D *h = h2->ProjectionY(pn, ibin, ibin);
    if(h->GetEntries()<100) continue;
    //AdjustF1(h, f);

    h->Fit(f, "QN");
    
    Int_t ip = g[0]->GetN();
    g[0]->SetPoint(ip, x, k*f->GetParameter(1));
    g[0]->SetPointError(ip, 0., k*f->GetParError(1));
    g[1]->SetPoint(ip, x, k*f->GetParameter(2));
    g[1]->SetPointError(ip, 0., k*f->GetParError(2));
    
/*  
    g[0]->SetPoint(ip, x, k*h->GetMean());
    g[0]->SetPointError(ip, 0., k*h->GetMeanError());
    g[1]->SetPoint(ip, x, k*h->GetRMS());
    g[1]->SetPointError(ip, 0., k*h->GetRMSError());*/
  }
  fIdxPlot++;
  return kTRUE;
}

//________________________________________________________
Bool_t AliTRDresolution::Process2D(ETRDresolutionPlot plot, Int_t idx, TF1 *f, Float_t k, Int_t gidx)
{
  //
  // Do the processing
  //

  if(!fContainer || !fGraphS || !fGraphM) return kFALSE;

  // retrive containers
  TH2I *h2 = idx<0 ? (TH2I*)(fContainer->At(plot)) : (TH2I*)((TObjArray*)(fContainer->At(plot)))->At(idx);
  if(!h2) return kFALSE;

  TGraphErrors *g[2];
  if(gidx<0) gidx=idx;
  if(!(g[0] = gidx<0 ? (TGraphErrors*)fGraphM->At(plot) : (TGraphErrors*)((TObjArray*)(fGraphM->At(plot)))->At(gidx))) return kFALSE;

  if(!(g[1] = gidx<0 ? (TGraphErrors*)fGraphS->At(plot) : (TGraphErrors*)((TObjArray*)(fGraphS->At(plot)))->At(gidx))) return kFALSE;

  return Process(h2, f, k, g);
}

//________________________________________________________
Bool_t AliTRDresolution::Process3D(ETRDresolutionPlot plot, Int_t idx, TF1 *f, Float_t k)
{
  //
  // Do the processing
  //

  if(!fContainer || !fGraphS || !fGraphM) return kFALSE;

  // retrive containers
  TH3S *h3 = idx<0 ? (TH3S*)(fContainer->At(plot)) : (TH3S*)((TObjArray*)(fContainer->At(plot)))->At(idx);
  if(!h3) return kFALSE;

  TObjArray *gm, *gs;
  if(!(gm = (TObjArray*)((TObjArray*)(fGraphM->At(plot)))->At(idx))) return kFALSE;
  if(!(gs = (TObjArray*)((TObjArray*)(fGraphS->At(plot)))->At(idx))) return kFALSE;
  TGraphErrors *g[2];

  TAxis *az = h3->GetZaxis();
  for(Int_t iz=1; iz<=az->GetNbins(); iz++){
    if(!(g[0] = (TGraphErrors*)gm->At(iz-1))) return kFALSE;
    if(!(g[1] = (TGraphErrors*)gs->At(iz-1))) return kFALSE;
    az->SetRange(iz, iz);
    if(!Process((TH2*)h3->Project3D("yx"), f, k, g)) return kFALSE;
  }

  return kTRUE;
}

//________________________________________________________
Bool_t AliTRDresolution::Process3Drange(ETRDresolutionPlot plot, Int_t hidx, Int_t gidx, TF1 *f, Float_t k, Int_t zbin0, Int_t zbin1)
{
  //
  // Do the processing
  //

  if(!fContainer || !fGraphS || !fGraphM) return kFALSE;

  // retrive containers
  TH3S *h3 = hidx<0 ? (TH3S*)(fContainer->At(plot)) : (TH3S*)((TObjArray*)(fContainer->At(plot)))->At(hidx);
  if(!h3) return kFALSE;

  TGraphErrors *g[2];
  if(!(g[0] = (TGraphErrors*)((TObjArray*)(fGraphM->At(plot)))->At(gidx))) return kFALSE;
  if(!(g[1] = (TGraphErrors*)((TObjArray*)(fGraphS->At(plot)))->At(gidx))) return kFALSE;

  TAxis *az = h3->GetZaxis();
  az->SetRange(zbin0, zbin1);
  if(!Process((TH2*)h3->Project3D("yx"), f, k, g)) return kFALSE;
  return kTRUE;
}

//________________________________________________________
Bool_t AliTRDresolution::Process3DL(ETRDresolutionPlot plot, Int_t idx, TF1 *f, Float_t k)
{
  //
  // Do the processing
  //

  if(!fContainer || !fGraphS || !fGraphM) return kFALSE;

  // retrive containers
  TH3S *h3 = (TH3S*)((TObjArray*)fContainer->At(plot))->At(idx);
  if(!h3) return kFALSE;

  TGraphAsymmErrors *gm; 
  TGraphErrors *gs;
  if(!(gm = (TGraphAsymmErrors*)((TObjArray*)fGraphM->At(plot))->At(0))) return kFALSE;
  if(!(gs = (TGraphErrors*)((TObjArray*)fGraphS->At(plot)))) return kFALSE;

  Float_t x, r, mpv, xM, xm;
  TAxis *ay = h3->GetYaxis();
  for(Int_t iy=1; iy<=h3->GetNbinsY(); iy++){
    ay->SetRange(iy, iy);
    x = ay->GetBinCenter(iy);
    TH2F *h2=(TH2F*)h3->Project3D("zx");
    TAxis *ax = h2->GetXaxis();
    for(Int_t ix=1; ix<=h2->GetNbinsX(); ix++){
      r = ax->GetBinCenter(ix);
      TH1D *h1 = h2->ProjectionY("py", ix, ix);
      if(h1->Integral()<50) continue;
      h1->Fit(f, "QN");

      GetLandauMpvFwhm(f, mpv, xm, xM);
      Int_t ip = gm->GetN();
      gm->SetPoint(ip, x, k*mpv);
      gm->SetPointError(ip, 0., 0., k*xm, k*xM);
      gs->SetPoint(ip, r, k*(xM-xm)/mpv);
      gs->SetPointError(ip, 0., 0.);
    }
  }

  return kTRUE;
}

//________________________________________________________
Bool_t AliTRDresolution::Process4D(ETRDresolutionPlot plot, Int_t idx, TF1 *f, Float_t k, Int_t n)
{
  //
  // Do the processing
  //

  if(!fContainer || !fGraphS || !fGraphM) return kFALSE;
  //printf("Process4D : processing plot[%d] idx[%d]\n", plot, idx);

  // retrive containers
  TObjArray *arr = (TObjArray*)((TObjArray*)(fContainer->At(plot)))->At(idx);
  if(!arr) return kFALSE;

  TObjArray *gm, *gs;
  if(!(gm = (TObjArray*)((TObjArray*)(fGraphM->At(plot)))->At(idx))) return kFALSE;
  if(!(gs = (TObjArray*)((TObjArray*)(fGraphS->At(plot)))->At(idx))) return kFALSE;

  TGraphErrors *g[2];

  TH3S *h3(NULL);
  for(Int_t ix=0, in=0; ix<arr->GetEntriesFast(); ix++){
    if(!(h3 = (TH3S*)arr->At(ix))) return kFALSE;
    TAxis *az = h3->GetZaxis();
    //printf("  process ix[%d] bins[%d] in[%d]\n", ix, az->GetNbins(), in);
    for(Int_t iz=1; iz<=az->GetNbins(); iz++, in++){
      if(n>=0 && n!=in) continue;
      if(!(g[0] = (TGraphErrors*)gm->At(in))) return kFALSE;
      if(!(g[1] = (TGraphErrors*)gs->At(in))) return kFALSE;
      //printf("    g0[%s] g1[%s]\n", g[0]->GetName(), g[1]->GetName());
      az->SetRange(iz, iz);
      if(!Process((TH2*)h3->Project3D("yx"), f, k, g)) return kFALSE;
    }
  }

  return kTRUE;
}

//________________________________________________________
Bool_t AliTRDresolution::GetGraphPlot(Float_t *bb, ETRDresolutionPlot ip, Int_t idx)
{
  //
  // Get the graphs
  //

  if(!fGraphS || !fGraphM) return kFALSE;

  //printf("plotting task[%d] gidx[%d]\n", ip, idx);
  TGraphErrors *gm = idx<0 ? (TGraphErrors*)fGraphM->At(ip) : (TGraphErrors*)((TObjArray*)(fGraphM->At(ip)))->At(idx);
  if(!gm) return kFALSE;
  TGraphErrors *gs = idx<0 ? (TGraphErrors*)fGraphS->At(ip) : (TGraphErrors*)((TObjArray*)(fGraphS->At(ip)))->At(idx);
  if(!gs) return kFALSE;
  //printf("gs[%s] gm[%s]\n", gs->GetName(), gm->GetName());
  gs->Draw("apl"); gm->Draw("pl");
  //return kTRUE;
  // titles look up
  Int_t nref = 0;
  for(Int_t jp=0; jp<(Int_t)ip; jp++) nref+=fgNproj[jp];
  UChar_t jdx = idx<0?0:idx;
  for(Int_t jc=0; jc<TMath::Min(jdx,fgNproj[ip]-1); jc++) nref++;
  const Char_t **at = fgAxTitle[nref];

  Int_t n(0);
  if((n=gm->GetN())) {
    PutTrendValue(Form("%s_%s", fgPerformanceName[ip], at[0]), gm->GetMean(2));
    PutTrendValue(Form("%s_%sRMS", fgPerformanceName[ip], at[0]), gm->GetRMS(2));
  }

  if((n=gs->GetN())){
    gs->Sort(&TGraph::CompareY);
    PutTrendValue(Form("%s_%sSigMin", fgPerformanceName[ip], at[0]), gs->GetY()[0]);
    PutTrendValue(Form("%s_%sSigMax", fgPerformanceName[ip], at[0]), gs->GetY()[n-1]);
    gs->Sort(&TGraph::CompareX); 
  }

  // axis range
  TAxis *ax = NULL;
  ax = gs->GetHistogram()->GetXaxis();
  ax->SetRangeUser(bb[0], bb[2]);
  ax->SetTitle(at[1]);ax->CenterTitle();

  ax = gs->GetHistogram()->GetYaxis();
  ax->SetRangeUser(bb[1], bb[3]);
  ax->SetTitleOffset(1.1);
  ax->SetTitle(at[2]);ax->CenterTitle();

  TGaxis *gax = NULL;
  gax = new TGaxis(bb[2], bb[1], bb[2], bb[3], bb[1], bb[3], 510, "+U");
  gax->SetLineColor(kRed);gax->SetLineWidth(2);gax->SetTextColor(kRed);
  //gax->SetVertical();
  gax->CenterTitle(); gax->SetTitleOffset(.7);
  gax->SetTitle(at[3]); gax->Draw();

  // bounding box
  TBox *b = new TBox(-.15, bb[1], .15, bb[3]);
  b->SetFillStyle(3002);b->SetLineColor(0);
  b->SetFillColor(ip<=Int_t(kMCcluster)?kGreen:kBlue);
  b->Draw();
  return kTRUE;
}

Char_t const *fgLegHeader[6]={
  "Layer 0","Layer 1","Layer 2","Layer 3","Layer 4","Layer 5"
};
//________________________________________________________
Bool_t AliTRDresolution::GetGraphTrack(Float_t *bb, Int_t idx, Int_t il, Int_t n, Bool_t kLEG)
{
  //
  // Get the graphs
  //

  if(!fGraphS || !fGraphM) return kFALSE;

  // axis titles look up
  Int_t nref(0);
  for(Int_t jp=0; jp<Int_t(kMCtrackTRD); jp++) nref+=fgNproj[jp];
  nref+=idx;
  const Char_t **at = fgAxTitle[nref];
  printf("nref[%d] ax[%s] x[%f %f] y[%f %f]\n", nref, at[0], bb[0], bb[2], bb[1], bb[3]);

  TLegend *leg(NULL);
  if(kLEG){
    leg=new TLegend(.65, .7, .95, .99);
    leg->SetHeader("Species");
    leg->SetBorderSize(1);
    leg->SetFillColor(0);
  }
  TH1S *h1(NULL);
  h1 = new TH1S(Form("h1TF_%d", fIdxFrame++), at[0], 2, bb[0], bb[2]);
  h1->SetMinimum(bb[1]);h1->SetMaximum(bb[3]);
  h1->SetLineColor(kBlack); h1->SetLineWidth(1);h1->SetLineStyle(2); 
  // axis range
  TAxis *ax = h1->GetXaxis();
  ax->SetTitle(at[1]);ax->CenterTitle();ax->SetMoreLogLabels();
  ax = h1->GetYaxis();
  ax->SetRangeUser(bb[1], bb[3]);
  ax->SetTitleOffset(.5);//ax->SetTitleSize(.06);
  ax->SetTitle(at[2]);ax->CenterTitle();
  h1->Draw();
  TGaxis *gax = NULL;
  gax = new TGaxis(bb[2], bb[1], bb[2], bb[3], bb[1], bb[3], 510, "+U");
  gax->SetLineColor(kRed);gax->SetLineWidth(2);gax->SetTextColor(kRed);
  //gax->SetVertical();
  gax->CenterTitle(); //gax->SetTitleOffset(.5);gax->SetTitleSize(.06);
  gax->SetTitle(at[3]); gax->Draw();

  TGraphErrors *gm = NULL, *gs = NULL;
  TObjArray *a0 = NULL, *a1 = NULL;
  a0 = (TObjArray*)((TObjArray*)fGraphM->At(kMCtrackTRD))->At(idx);
  a1 = (TObjArray*)((TObjArray*)fGraphS->At(kMCtrackTRD))->At(idx);
  Int_t nn(0);
  for(Int_t is=il, is0=0; is<il+n; is++, is0++){
    if(is0==3) continue;
    if(!(gs =  (TGraphErrors*)a1->At(is))) return kFALSE;
    if(!(gm =  (TGraphErrors*)a0->At(is))) return kFALSE;

    if((nn=gs->GetN())){
      gs->Draw("pl");
      if(leg) leg->AddEntry(gs, gs->GetTitle(), "p");
      //printf("entry[%d]=%s\n", il+is, gs->GetTitle());
      gs->Sort(&TGraph::CompareY);
      PutTrendValue(Form("%s_%sSigMin%s", fgPerformanceName[kMCtrackTRD], at[0], AliPID::ParticleShortName(is0)), gs->GetY()[0]);
      PutTrendValue(Form("%s_%sSigMax%s", fgPerformanceName[kMCtrackTRD], at[0], AliPID::ParticleShortName(is0)), gs->GetY()[nn-1]);
      gs->Sort(&TGraph::CompareX); 
    }
    if(gm->GetN()){
      gm->Draw("pl");
      PutTrendValue(Form("%s_%s_%s", fgPerformanceName[kMCtrackTRD], at[0], AliPID::ParticleShortName(is0)), gm->GetMean(2));
      PutTrendValue(Form("%s_%s_%sRMS", fgPerformanceName[kMCtrackTRD], at[0], AliPID::ParticleShortName(is0)), gm->GetRMS(2));
    }
  }
  if(leg) leg->Draw();
  return kTRUE;
}


//________________________________________________________
Bool_t AliTRDresolution::GetGraphTrackTPC(Float_t *bb, Int_t idx, Int_t ist, Int_t n, Bool_t kLEG)
{
  //
  // Get the graphs
  //

  if(!fGraphS || !fGraphM) return kFALSE;

  // axis titles look up
  Int_t nref = 0;
  for(Int_t jp=0; jp<Int_t(kMCtrackTPC); jp++) nref+=fgNproj[jp];
  nref+=idx;
  const Char_t **at = fgAxTitle[nref];

  TLegend *leg(NULL);
  if(kLEG){
    leg=new TLegend(.65, .7, .95, .99);
    leg->SetHeader("Species");
    leg->SetBorderSize(1);
    leg->SetFillColor(0);
  }
  TH1S *h1(NULL);
  h1 = new TH1S(Form("h1TF_%d", fIdxFrame++), at[0], 2, bb[0], bb[2]);
  h1->SetMinimum(bb[1]);h1->SetMaximum(bb[3]);
  h1->SetLineColor(kBlack); h1->SetLineWidth(1);h1->SetLineStyle(2); 
  // axis range
  TAxis *ax = h1->GetXaxis();
  ax->SetTitle(at[1]);ax->CenterTitle();ax->SetMoreLogLabels();
  ax = h1->GetYaxis();
  ax->SetRangeUser(bb[1], bb[3]);
  ax->SetTitleOffset(.5);//ax->SetTitleSize(.06);
  ax->SetTitle(at[2]);ax->CenterTitle();
  h1->Draw();
  TGaxis *gax = NULL;
  gax = new TGaxis(bb[2], bb[1], bb[2], bb[3], bb[1], bb[3], 510, "+U");
  gax->SetLineColor(kRed);gax->SetLineWidth(2);gax->SetTextColor(kRed);
  //gax->SetVertical();
  gax->CenterTitle(); //gax->SetTitleOffset(.5);gax->SetTitleSize(.06);
  gax->SetTitle(at[3]); gax->Draw();

  Int_t nn(0);
  TGraphErrors *gm = NULL, *gs = NULL;
  TObjArray *a0 = NULL, *a1 = NULL;
  a0 = (TObjArray*)((TObjArray*)fGraphM->At(kMCtrackTPC))->At(idx);
  a1 = (TObjArray*)((TObjArray*)fGraphS->At(kMCtrackTPC))->At(idx);
  for(Int_t is=ist, is0=0; is<ist+n; is++, is0++){
    if(is0==3) continue;
    if(!(gs =  (TGraphErrors*)a1->At(is))) return kFALSE;
    if(!(gm =  (TGraphErrors*)a0->At(is))) return kFALSE;
    if((nn=gs->GetN())){
      gs->Draw("pl");if(leg) leg->AddEntry(gs, gs->GetTitle(), "p");
      gs->Sort(&TGraph::CompareY);
      PutTrendValue(Form("%s_%sSigMin%s", fgPerformanceName[kMCtrackTPC], at[0], AliPID::ParticleShortName(is0)), gs->GetY()[0]);
      PutTrendValue(Form("%s_%sSigMax%s", fgPerformanceName[kMCtrackTPC], at[0], AliPID::ParticleShortName(is0)), gs->GetY()[nn-1]);
      gs->Sort(&TGraph::CompareX); 
    }
    if(gm->GetN()){
      gm->Draw("pl");
      PutTrendValue(Form("%s_%s_%s", fgPerformanceName[kMCtrackTPC], at[0], AliPID::ParticleShortName(is0)), gm->GetMean(2));
      PutTrendValue(Form("%s_%s_%sRMS", fgPerformanceName[kMCtrackTPC], at[0], AliPID::ParticleShortName(is0)), gm->GetRMS(2));
    }
  }
  if(leg) leg->Draw();
  return kTRUE;
}

//________________________________________________________
void AliTRDresolution::GetLandauMpvFwhm(TF1 * const f, Float_t &mpv, Float_t &xm, Float_t &xM)
{
  //
  // Get the most probable value and the full width half mean 
  // of a Landau distribution
  //

  const Float_t dx = 1.;
  mpv = f->GetParameter(1);
  Float_t fx, max = f->Eval(mpv);

  xm = mpv - dx;
  while((fx = f->Eval(xm))>.5*max){
    if(fx>max){ 
      max = fx;
      mpv = xm;
    }
    xm -= dx;
  }

  xM += 2*(mpv - xm);
  while((fx = f->Eval(xM))>.5*max) xM += dx;
}


//________________________________________________________
void AliTRDresolution::SetRecoParam(AliTRDrecoParam *r)
{

  fReconstructor->SetRecoParam(r);
}
