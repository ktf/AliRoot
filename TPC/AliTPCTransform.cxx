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

//-------------------------------------------------------
//          Implementation of the TPC transformation class
//
//   Origin: Marian Ivanov   Marian.Ivanov@cern.ch
//           Magnus Mager
//
//   Class for tranformation of the coordinate frame
//   Transformation  
//    local coordinate frame (sector, padrow, pad, timebine) ==>
//    rotated global (tracking) cooridnate frame (sector, lx,ly,lz)
//
//    Unisochronity  - (substract time0 - pad by pad)
//    Drift velocity - Currently common drift velocity - functionality of AliTPCParam
//    ExB effect     - 
//
//    Time of flight correction -
//                   - Depends on the vertex position
//                   - by default 
//                           
//    Usage:
//          AliTPCclustererMI::AddCluster
//          AliTPCtrackerMI::Transform
//    
//-------------------------------------------------------

/* To test it:
   cdb=AliCDBManager::Instance()
   cdb->SetDefaultStorage("local:///u/mmager/mycalib1")
   c=AliTPCcalibDB::Instance()
   c->SetRun(0)
   Double_t x[]={1.0,2.0,3.0}
   Int_t i[]={4}
   AliTPCTransform trafo
   trafo.Transform(x,i,0,1)
 */

/* $Id$ */

#include "AliTPCROC.h"
#include "AliTPCCalPad.h"
#include "AliTPCCalROC.h"
#include "AliTPCcalibDB.h"
#include "AliTPCParam.h"
#include "TMath.h"
#include "AliLog.h"
#include "AliTPCExB.h"
#include "TGeoMatrix.h"
#include "AliTPCRecoParam.h"
#include "AliTPCCalibVdrift.h"
#include "AliTPCTransform.h"

ClassImp(AliTPCTransform)


AliTPCTransform::AliTPCTransform():
  AliTransform(),
  fCurrentRecoParam(0),       //! current reconstruction parameters
  fCurrentRun(0),             //! current run
  fCurrentTimeStamp(0)        //! current time stamp   
{
  //
  // Speed it up a bit!
  //
  for (Int_t i=0;i<18;++i) {
    Double_t alpha=TMath::DegToRad()*(10.+20.*(i%18));
    fSins[i]=TMath::Sin(alpha);
    fCoss[i]=TMath::Cos(alpha);
  }
  fPrimVtx[0]=0;
  fPrimVtx[1]=0;
  fPrimVtx[2]=0;
}
AliTPCTransform::AliTPCTransform(const AliTPCTransform& transform):
  AliTransform(transform),
  fCurrentRecoParam(transform.fCurrentRecoParam),       //! current reconstruction parameters
  fCurrentRun(transform.fCurrentRun),             //! current run
  fCurrentTimeStamp(transform.fCurrentTimeStamp)        //! current time stamp   
{
  //
  // Speed it up a bit!
  //
  for (Int_t i=0;i<18;++i) {
    Double_t alpha=TMath::DegToRad()*(10.+20.*(i%18));
    fSins[i]=TMath::Sin(alpha);
    fCoss[i]=TMath::Cos(alpha);
  }
  fPrimVtx[0]=0;
  fPrimVtx[1]=0;
  fPrimVtx[2]=0;
}

AliTPCTransform::~AliTPCTransform() {
  //
  // Destructor
  //
}

void AliTPCTransform::SetPrimVertex(Double_t *vtx){
  //
  //
  //
  fPrimVtx[0]=vtx[0];
  fPrimVtx[1]=vtx[1];
  fPrimVtx[2]=vtx[2];
}


void AliTPCTransform::Transform(Double_t *x,Int_t *i,UInt_t /*time*/,
				Int_t /*coordinateType*/) {
  // input: x[0] - pad row
  //        x[1] - pad 
  //        x[2] - time in us
  //        i[0] - sector
  // output: x[0] - x (all in the rotated global coordinate frame)
  //         x[1] - y
  //         x[2] - z
  //
  //  primvtx     - position of the primary vertex
  //                used for the TOF correction
  //                TOF of particle calculated assuming the speed-of-light and 
  //                line approximation  
  //
  

  Int_t row=TMath::Nint(x[0]);
  Int_t pad=TMath::Nint(x[1]);
  Int_t sector=i[0];
  AliTPCcalibDB*  calib=AliTPCcalibDB::Instance();  
  //
  AliTPCCalPad * time0TPC = calib->GetPadTime0(); 
  AliTPCParam  * param    = calib->GetParameters(); 
  if (!time0TPC){
    AliFatal("Time unisochronity missing");
  }

  if (!param){
    AliFatal("Parameters missing");
  }

  Double_t xx[3];
  //  Apply Time0 correction - Pad by pad fluctuation
  //
  x[2]-=time0TPC->GetCalROC(sector)->GetValue(row,pad);
  //
  // Tranform from pad - time coordinate system to the rotated global (tracking) system
  //
  Local2RotatedGlobal(sector,x);
  //
  //
  //
  // Alignment
  //TODO:  calib->GetParameters()->GetClusterMatrix(sector)->LocalToMaster(x,xx);
  RotatedGlobal2Global(sector,x);
  //
  //
  // ExB correction
  //
  calib->GetExB()->Correct(x,xx);
  //
  // Time of flight correction
  // 
  if (fCurrentRecoParam&&fCurrentRecoParam->GetUseTOFCorrection()){
    const Int_t kNIS=param->GetNInnerSector(), kNOS=param->GetNOuterSector(); 
    Float_t sign=1;
    if (sector < kNIS) {
      sign = (sector < kNIS/2) ? 1 : -1;
    } else {
      sign = ((sector-kNIS) < kNOS/2) ? 1 : -1;
    }
    Float_t deltaDr =0;
    Float_t dist=0;
    dist+=(fPrimVtx[0]-x[0])*(fPrimVtx[0]-x[0]);
    dist+=(fPrimVtx[1]-x[1])*(fPrimVtx[1]-x[1]);
    dist+=(fPrimVtx[2]-x[2])*(fPrimVtx[2]-x[2]);
    dist = TMath::Sqrt(dist);
    // drift length correction because of TOF
    // the drift velocity is in cm/s therefore multiplication by 0.01
    deltaDr = (dist*(0.01*param->GetDriftV()))/TMath::C(); 
    xx[2]+=sign*deltaDr;
  }
  //
  //
  //

  //
  Global2RotatedGlobal(sector,xx);
  //
  x[0]=xx[0];x[1]=xx[1];x[2]=xx[2];
}

void AliTPCTransform::Local2RotatedGlobal(Int_t sector, Double_t *x) const {
  //
  //  
  // Tranform coordinate from  
  // row, pad, time to x,y,z
  //
  // Drift Velocity 
  // Current implementation - common drift velocity - for full chamber
  // TODO: use a map or parametrisation!
  //
  //  
  //
  const  Int_t kMax =60;  // cache for 60 seconds
  static Int_t lastStamp=-1;  //cached values
  static Double_t lastCorr = 1;
  //
  AliTPCcalibDB*  calib=AliTPCcalibDB::Instance();
  AliTPCParam  * param    = calib->GetParameters(); 
  AliTPCCalibVdrift *driftCalib = AliTPCcalibDB::Instance()->GetVdrift(fCurrentRun);
  Double_t driftCorr = 1.;
  if (driftCalib){
    //
    // caching drift correction - temp. fix
    // Extremally slow procedure
    if ( TMath::Abs((lastStamp)-Int_t(fCurrentTimeStamp))<kMax){
      driftCorr = lastCorr;
    }else{
      driftCorr = 1.+(driftCalib->GetPTRelative(fCurrentTimeStamp,0)+ driftCalib->GetPTRelative(fCurrentTimeStamp,1))*0.5;
      lastCorr=driftCorr;
      lastStamp=fCurrentTimeStamp;
      
    }
  }
  


  if (!param){
    AliFatal("Parameters missing");
  }
  Int_t row=TMath::Nint(x[0]);
  //  Int_t pad=TMath::Nint(x[1]);
  //
  const Int_t kNIS=param->GetNInnerSector(), kNOS=param->GetNOuterSector();
  Double_t sign = 1.;
  Double_t zwidth    = param->GetZWidth()*driftCorr;
  Double_t padWidth  = 0;
  Double_t padLength = 0;
  Double_t    maxPad    = 0;
  //
  if (sector < kNIS) {
    maxPad = param->GetNPadsLow(row);
    sign = (sector < kNIS/2) ? 1 : -1;
    padLength = param->GetPadPitchLength(sector,row);
    padWidth = param->GetPadPitchWidth(sector);
  } else {
    maxPad = param->GetNPadsUp(row);
    sign = ((sector-kNIS) < kNOS/2) ? 1 : -1;
    padLength = param->GetPadPitchLength(sector,row);
    padWidth  = param->GetPadPitchWidth(sector);
  }
  //
  // X coordinate
  x[0] = param->GetPadRowRadii(sector,row);  // padrow X position - ideal
  //
  // Y coordinate
  //
  x[1]=(x[1]-0.5*maxPad)*padWidth;
  // pads are mirrorred on C-side
  if (sector%36>17){
    x[1]*=-1;
  }
  
  //
  
  //
  // Z coordinate
  //
  x[2]*= zwidth;  // tranform time bin to the distance to the ROC
  x[2]-= 3.*param->GetZSigma() + param->GetNTBinsL1()*zwidth;
  // subtract the time offsets
  x[2] = sign*( param->GetZLength(sector) - x[2]);
}

void AliTPCTransform::RotatedGlobal2Global(Int_t sector,Double_t *x) const {
  //
  // transform possition rotated global to the global
  //
  Double_t cos,sin;
  GetCosAndSin(sector,cos,sin);
  Double_t tmp=x[0];
  x[0]= cos*tmp-sin*x[1];
  x[1]=+sin*tmp+cos*x[1];
}

void AliTPCTransform::Global2RotatedGlobal(Int_t sector,Double_t *x) const {
  //
  // tranform possition Global2RotatedGlobal
  //
  Double_t cos,sin;
  GetCosAndSin(sector,cos,sin);
  Double_t tmp=x[0];
  x[0]= cos*tmp+sin*x[1];
  x[1]= -sin*tmp+cos*x[1];
}

void AliTPCTransform::GetCosAndSin(Int_t sector,Double_t &cos,
					  Double_t &sin) const {
  cos=fCoss[sector%18];
  sin=fSins[sector%18];
}


