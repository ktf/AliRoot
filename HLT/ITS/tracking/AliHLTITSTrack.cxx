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

//-------------------------------------------------------------------------
//                Implementation of the ITS track class
//
//          Origin: Marian Ivanov, CERN, Marian.Ivanov@cern.ch
//     dEdx analysis by: Boris Batyunya, JINR, Boris.Batiounia@cern.ch
//-------------------------------------------------------------------------

/* $Id: AliHLTITSTrack.cxx 30856 2009-02-02 11:12:50Z fca $ */

#include <TMatrixD.h>

#include <TMath.h>

#include "AliCluster.h"
#include "AliESDtrack.h"
#include "AliITSgeomTGeo.h"
#include "AliHLTITSTrack.h"
#include "AliTracker.h"
#include <TMath.h>

#include "AliCluster.h"
#include "AliESDtrack.h"
#include "AliESDVertex.h"
#include "AliITSReconstructor.h"
#include "AliITStrackV2.h"
#include "AliTracker.h"


ClassImp(AliHLTITSTrack)

//____________________________________________________________________________
AliHLTITSTrack::AliHLTITSTrack() : 
  AliKalmanTrack(),  
  fExpQ(40),
  fTPCtrackId(0)
{
  for(Int_t i=0; i<2*AliITSgeomTGeo::kNLayers; i++) {fIndex[i]=-1; }
}

//____________________________________________________________________________
AliHLTITSTrack::AliHLTITSTrack(const AliHLTITSTrack& t) : 
  AliKalmanTrack(t),  
  fExpQ(t.fExpQ),
  fTPCtrackId( t.fTPCtrackId)
{
  //------------------------------------------------------------------
  //Copy constructor
  //------------------------------------------------------------------
  Int_t i;
  for (i=0; i<2*AliITSgeomTGeo::GetNLayers(); i++) {
    fIndex[i]=t.fIndex[i];
  }
  fLab = t.fLab;
  fFakeRatio = t.fFakeRatio;
}

//____________________________________________________________________________
AliHLTITSTrack &AliHLTITSTrack::operator=(const AliHLTITSTrack& t)
{
  //------------------------------------------------------------------
  //Copy constructor
  //------------------------------------------------------------------
  *(AliKalmanTrack*)this = t;  
  fExpQ = t.fExpQ;
  fTPCtrackId = t.fTPCtrackId;
  Int_t i;
  for (i=0; i<2*AliITSgeomTGeo::GetNLayers(); i++) {
    fIndex[i]=t.fIndex[i];
  }
  fLab = t.fLab;
  fFakeRatio = t.fFakeRatio;
  return *this;
}


//____________________________________________________________________________
AliHLTITSTrack::AliHLTITSTrack(AliESDtrack& t,Bool_t c) throw (const Char_t *) :
  AliKalmanTrack(),
  fExpQ(40),
  fTPCtrackId( 0 )
{
  //------------------------------------------------------------------
  // Conversion ESD track -> ITS track.
  // If c==kTRUE, create the ITS track out of the constrained params.
  //------------------------------------------------------------------
  const AliExternalTrackParam *par=&t;
  if (c) {
    par=t.GetConstrainedParam();
    if (!par) throw "AliHLTITSTrack: conversion failed !\n";
  }
  Set(par->GetX(),par->GetAlpha(),par->GetParameter(),par->GetCovariance());

  SetLabel(t.GetLabel());
  //SetMass(t.GetMass());
  SetNumberOfClusters(t.GetITSclusters(fIndex));
}

//____________________________________________________________________________
AliHLTITSTrack::AliHLTITSTrack(AliExternalTrackParam& t ) throw (const Char_t *) :
  AliKalmanTrack(),  
  fExpQ(40),
  fTPCtrackId( 0 )
{
  //------------------------------------------------------------------
  // Conversion ESD track -> ITS track.
  // If c==kTRUE, create the ITS track out of the constrained params.
  //------------------------------------------------------------------
  const AliExternalTrackParam *par=&t;
  Set(par->GetX(),par->GetAlpha(),par->GetParameter(),par->GetCovariance());
  SetLabel(t.GetLabel());
  //SetMass(t.GetMass());
  SetNumberOfClusters(0);
}


Double_t AliHLTITSTrack::GetPredictedChi2(const AliCluster* c) const
{
  return GetPredictedChi2(c->GetY(), c->GetZ(), c->GetSigmaY2(), c->GetSigmaZ2() );
}

Double_t AliHLTITSTrack::GetPredictedChi2(Double_t cy, Double_t cz, Double_t cerr2Y, Double_t cerr2Z) const
{
  //-----------------------------------------------------------------
  // This function calculates a predicted chi2 increment.
  //-----------------------------------------------------------------
  Double_t p[2]={cy, cz};
  Double_t cov[3]={cerr2Y, 0., cerr2Z};
  return AliExternalTrackParam::GetPredictedChi2(p,cov);
}




Int_t AliHLTITSTrack::GetProlongationFast(Double_t alp, Double_t xk,Double_t &y, Double_t &z)
{
  //-----------------------------------------------------------------------------
  //get fast prolongation 
  //-----------------------------------------------------------------------------
  Double_t ca=TMath::Cos(alp-GetAlpha()), sa=TMath::Sin(alp-GetAlpha());
  Double_t cf=TMath::Sqrt((1.-GetSnp())*(1.+GetSnp()));  
  // **** rotation **********************  
  y= -GetX()*sa + GetY()*ca;
  // **** translation ******************  
  Double_t dx = xk- GetX()*ca - GetY()*sa;
  Double_t f1=GetSnp()*ca - cf*sa, f2=f1 + GetC()*dx;
  if (TMath::Abs(f2) >= 0.9999) {
    return 0;
  }
  Double_t r1=TMath::Sqrt((1.-f1)*(1.+f1)), r2=TMath::Sqrt((1.-f2)*(1.+f2));  
  y += dx*(f1+f2)/(r1+r2);
  z  = GetZ()+dx*(f1+f2)/(f1*r2 + f2*r1)*GetTgl();  
  return 1;
}



//____________________________________________________________________________
void AliHLTITSTrack::ResetClusters() {
  //------------------------------------------------------------------
  // Reset the array of attached clusters.
  //------------------------------------------------------------------
  for (Int_t i=0; i<2*AliITSgeomTGeo::kNLayers; i++) fIndex[i]=-1;
  SetChi2(0.); 
  SetNumberOfClusters(0);
} 




//____________________________________________________________________________


//____________________________________________________________________________
Bool_t AliHLTITSTrack::
GetGlobalXYZat(Double_t xloc, Double_t &x, Double_t &y, Double_t &z) const {
  //------------------------------------------------------------------
  //This function returns a track position in the global system
  //------------------------------------------------------------------
  Double_t r[3];
  Bool_t rc=GetXYZAt(xloc, GetBz(), r);
  x=r[0]; y=r[1]; z=r[2]; 
  return rc;
}


//____________________________________________________________________________
Bool_t AliHLTITSTrack::PropagateTo(Double_t xk, Double_t d, Double_t x0) {
  //------------------------------------------------------------------
  //This function propagates a track
  //------------------------------------------------------------------

  Double_t oldX=GetX(), oldY=GetY(), oldZ=GetZ();
  
  Double_t bz=GetBz();
  if (!AliExternalTrackParam::PropagateTo(xk,bz)) return kFALSE;
  Double_t xOverX0,xTimesRho; 
  xOverX0 = d; xTimesRho = d*x0;
  if (!CorrectForMeanMaterial(xOverX0,xTimesRho,kTRUE)) return kFALSE;

  Double_t x=GetX(), y=GetY(), z=GetZ();
  if (IsStartedTimeIntegral() && x>oldX) {
    Double_t l2 = (x-oldX)*(x-oldX) + (y-oldY)*(y-oldY) + (z-oldZ)*(z-oldZ);
    AddTimeStep(TMath::Sqrt(l2));
  }

  return kTRUE;
}

//____________________________________________________________________________
Bool_t AliHLTITSTrack::PropagateToTGeo(Double_t xToGo, Int_t nstep, Double_t &xOverX0, Double_t &xTimesRho, Bool_t addTime) {
  //-------------------------------------------------------------------
  //  Propagates the track to a reference plane x=xToGo in n steps.
  //  These n steps are only used to take into account the curvature.
  //  The material is calculated with TGeo. (L.Gaudichet)
  //-------------------------------------------------------------------
  
  Double_t startx = GetX(), starty = GetY(), startz = GetZ();
  Double_t sign = (startx<xToGo) ? -1.:1.;
  Double_t step = (xToGo-startx)/TMath::Abs(nstep);

  Double_t start[3], end[3], mparam[7], bz = GetBz();
  Double_t x = startx;
  
  for (Int_t i=0; i<nstep; i++) {
    
    GetXYZ(start);   //starting global position
    x += step;
    if (!GetXYZAt(x, bz, end)) return kFALSE;
    if (!AliExternalTrackParam::PropagateTo(x, bz)) return kFALSE;
    AliTracker::MeanMaterialBudget(start, end, mparam);
    xTimesRho = sign*mparam[4]*mparam[0];
    xOverX0   = mparam[1];
    if (mparam[1]<900000) {
      if (!AliExternalTrackParam::CorrectForMeanMaterial(xOverX0,
							 xTimesRho,GetMass())) return kFALSE;
    } else { // this happens when MeanMaterialBudget cannot cross a boundary
      return kFALSE;
    }
  }

  if (addTime && IsStartedTimeIntegral() && GetX()>startx) {
    Double_t l2 = ( (GetX()-startx)*(GetX()-startx) +
		    (GetY()-starty)*(GetY()-starty) +
		    (GetZ()-startz)*(GetZ()-startz) );
    AddTimeStep(TMath::Sqrt(l2));
  }

  return kTRUE;
}

//____________________________________________________________________________
Bool_t AliHLTITSTrack::Update(const AliCluster* c, Double_t chi2, Int_t index) 
{
  //------------------------------------------------------------------
  //This function updates track parameters
  //------------------------------------------------------------------
  Double_t p[2]={c->GetY(), c->GetZ()};
  Double_t cov[3]={c->GetSigmaY2(), 0., c->GetSigmaZ2()};

  if (!AliExternalTrackParam::Update(p,cov)) return kFALSE;

  Int_t n=GetNumberOfClusters();

  if (chi2<0) return kTRUE;

  fIndex[n]=index;
  SetNumberOfClusters(n+1);
  SetChi2(GetChi2()+chi2);

  return kTRUE;
}



//____________________________________________________________________________
Bool_t AliHLTITSTrack::Propagate(Double_t alp,Double_t xk) {
  //------------------------------------------------------------------
  //This function propagates a track
  //------------------------------------------------------------------
  Double_t bz=GetBz();
  if (!AliExternalTrackParam::Propagate(alp,xk,bz)) return kFALSE;

  return kTRUE;
}





//____________________________________________________________________________
Bool_t AliHLTITSTrack::
GetPhiZat(Double_t r, Double_t &phi, Double_t &z) const {
  //------------------------------------------------------------------
  // This function returns the global cylindrical (phi,z) of the track 
  // position estimated at the radius r. 
  // The track curvature is neglected.
  //------------------------------------------------------------------
  Double_t d=GetD(0.,0.);
  if (TMath::Abs(d) > r) {
    if (r>1e-1) return kFALSE;
    r = TMath::Abs(d);
  }

  Double_t rcurr=TMath::Sqrt(GetX()*GetX() + GetY()*GetY());
  if (TMath::Abs(d) > rcurr) return kFALSE;
  Double_t globXYZcurr[3]; GetXYZ(globXYZcurr); 
  Double_t phicurr=TMath::ATan2(globXYZcurr[1],globXYZcurr[0]);

  if (GetX()>=0.) {
    phi=phicurr+TMath::ASin(d/r)-TMath::ASin(d/rcurr);
  } else {
    phi=phicurr+TMath::ASin(d/r)+TMath::ASin(d/rcurr)-TMath::Pi();
  }

  // return a phi in [0,2pi[ 
  if (phi<0.) phi+=2.*TMath::Pi();
  else if (phi>=2.*TMath::Pi()) phi-=2.*TMath::Pi();
  z=GetZ()+GetTgl()*(TMath::Sqrt((r-d)*(r+d))-TMath::Sqrt((rcurr-d)*(rcurr+d)));
  return kTRUE;
}

//____________________________________________________________________________
Bool_t AliHLTITSTrack::
GetLocalXat(Double_t r,Double_t &xloc) const {
  //------------------------------------------------------------------
  // This function returns the local x of the track 
  // position estimated at the radius r. 
  // The track curvature is neglected.
  //------------------------------------------------------------------
  Double_t d=GetD(0.,0.);
  if (TMath::Abs(d) > r) { 
    if (r>1e-1) return kFALSE; 
    r = TMath::Abs(d); 
  } 

  Double_t rcurr=TMath::Sqrt(GetX()*GetX() + GetY()*GetY());
  Double_t globXYZcurr[3]; GetXYZ(globXYZcurr); 
  Double_t phicurr=TMath::ATan2(globXYZcurr[1],globXYZcurr[0]);
  Double_t phi;
  if (GetX()>=0.) {
    phi=phicurr+TMath::ASin(d/r)-TMath::ASin(d/rcurr);
  } else {
    phi=phicurr+TMath::ASin(d/r)+TMath::ASin(d/rcurr)-TMath::Pi();
  }

  xloc=r*(TMath::Cos(phi)*TMath::Cos(GetAlpha())
         +TMath::Sin(phi)*TMath::Sin(GetAlpha())); 

  return kTRUE;
}
