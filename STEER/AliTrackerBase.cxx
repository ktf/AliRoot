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

/* $Id: AliTrackerBase.cxx 38069 2009-12-24 16:56:18Z belikov $ */

//-------------------------------------------------------------------------
//               Implementation of the AliTrackerBase class
//  that is the base for AliTPCtracker, AliITStrackerV2 and AliTRDtracker    
//        Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch
//-------------------------------------------------------------------------
#include <TClass.h>
#include <TMath.h>
#include <TGeoManager.h>

#include "AliLog.h"
#include "AliTrackerBase.h"
#include "AliExternalTrackParam.h"

extern TGeoManager *gGeoManager;

ClassImp(AliTrackerBase)

AliTrackerBase::AliTrackerBase():
  TObject(),
  fX(0),
  fY(0),
  fZ(0),
  fSigmaX(0.005),
  fSigmaY(0.005),
  fSigmaZ(0.010)
{
  //--------------------------------------------------------------------
  // The default constructor.
  //--------------------------------------------------------------------
  if (!TGeoGlobalMagField::Instance()->GetField())
    AliWarning("Field map is not set.");
}

//__________________________________________________________________________
AliTrackerBase::AliTrackerBase(const AliTrackerBase &atr):
  TObject(atr),
  fX(atr.fX),
  fY(atr.fY),
  fZ(atr.fZ),
  fSigmaX(atr.fSigmaX),
  fSigmaY(atr.fSigmaY),
  fSigmaZ(atr.fSigmaZ)
{
  //--------------------------------------------------------------------
  // The default constructor.
  //--------------------------------------------------------------------
  if (!TGeoGlobalMagField::Instance()->GetField())
    AliWarning("Field map is not set.");
}

//__________________________________________________________________________
Double_t AliTrackerBase::GetBz()
{
  AliMagF* fld = (AliMagF*)TGeoGlobalMagField::Instance()->GetField();
  if (!fld) return 0.5*kAlmost0Field;
  Double_t bz = fld->SolenoidField();
  return TMath::Sign(0.5*kAlmost0Field,bz) + bz;
}

//__________________________________________________________________________
Double_t AliTrackerBase::GetBz(const Double_t *r) {
  //------------------------------------------------------------------
  // Returns Bz (kG) at the point "r" .
  //------------------------------------------------------------------
  AliMagF* fld = (AliMagF*)TGeoGlobalMagField::Instance()->GetField();
  if (!fld) return  0.5*kAlmost0Field;
  Double_t bz = fld->GetBz(r);
  return  TMath::Sign(0.5*kAlmost0Field,bz) + bz;
}

//__________________________________________________________________________
void AliTrackerBase::GetBxByBz(const Double_t r[3], Double_t b[3]) {
  //------------------------------------------------------------------
  // Returns Bx, By and Bz (kG) at the point "r" .
  //------------------------------------------------------------------
  AliMagF* fld = (AliMagF*)TGeoGlobalMagField::Instance()->GetField();
  if (!fld) {
     b[0] = b[1] = 0.;
     b[2] = 0.5*kAlmost0Field;
     return;
  }

  if (fld->IsUniform()) {
     b[0] = b[1] = 0.;
     b[2] = fld->SolenoidField();
  }  else {
     fld->Field(r,b);
  }
  b[2] = (TMath::Sign(0.5*kAlmost0Field,b[2]) + b[2]);
  return;
}

Double_t AliTrackerBase::MeanMaterialBudget(const Double_t *start, const Double_t *end, Double_t *mparam)
{
  // 
  // Calculate mean material budget and material properties between 
  //    the points "start" and "end".
  //
  // "mparam" - parameters used for the energy and multiple scattering
  //  corrections: 
  //
  // mparam[0] - mean density: sum(x_i*rho_i)/sum(x_i) [g/cm3]
  // mparam[1] - equivalent rad length fraction: sum(x_i/X0_i) [adimensional]
  // mparam[2] - mean A: sum(x_i*A_i)/sum(x_i) [adimensional]
  // mparam[3] - mean Z: sum(x_i*Z_i)/sum(x_i) [adimensional]
  // mparam[4] - length: sum(x_i) [cm]
  // mparam[5] - Z/A mean: sum(x_i*Z_i/A_i)/sum(x_i) [adimensional]
  // mparam[6] - number of boundary crosses
  //
  //  Origin:  Marian Ivanov, Marian.Ivanov@cern.ch
  //
  //  Corrections and improvements by
  //        Andrea Dainese, Andrea.Dainese@lnl.infn.it,
  //        Andrei Gheata,  Andrei.Gheata@cern.ch
  //

  mparam[0]=0; mparam[1]=1; mparam[2] =0; mparam[3] =0;
  mparam[4]=0; mparam[5]=0; mparam[6]=0;
  //
  Double_t bparam[6]; // total parameters
  Double_t lparam[6]; // local parameters

  for (Int_t i=0;i<6;i++) bparam[i]=0;

  if (!gGeoManager) {
    AliErrorClass("No TGeo\n");
    return 0.;
  }
  //
  Double_t length;
  Double_t dir[3];
  length = TMath::Sqrt((end[0]-start[0])*(end[0]-start[0])+
                       (end[1]-start[1])*(end[1]-start[1])+
                       (end[2]-start[2])*(end[2]-start[2]));
  mparam[4]=length;
  if (length<TGeoShape::Tolerance()) return 0.0;
  Double_t invlen = 1./length;
  dir[0] = (end[0]-start[0])*invlen;
  dir[1] = (end[1]-start[1])*invlen;
  dir[2] = (end[2]-start[2])*invlen;

  // Initialize start point and direction
  TGeoNode *currentnode = 0;
  TGeoNode *startnode = gGeoManager->InitTrack(start, dir);
  if (!startnode) {
    AliErrorClass(Form("start point out of geometry: x %f, y %f, z %f",
		  start[0],start[1],start[2]));
    return 0.0;
  }
  TGeoMaterial *material = startnode->GetVolume()->GetMedium()->GetMaterial();
  lparam[0]   = material->GetDensity();
  lparam[1]   = material->GetRadLen();
  lparam[2]   = material->GetA();
  lparam[3]   = material->GetZ();
  lparam[4]   = length;
  lparam[5]   = lparam[3]/lparam[2];
  if (material->IsMixture()) {
    TGeoMixture * mixture = (TGeoMixture*)material;
    lparam[5] =0;
    Double_t sum =0;
    for (Int_t iel=0;iel<mixture->GetNelements();iel++){
      sum  += mixture->GetWmixt()[iel];
      lparam[5]+= mixture->GetZmixt()[iel]*mixture->GetWmixt()[iel]/mixture->GetAmixt()[iel];
    }
    lparam[5]/=sum;
  }

  // Locate next boundary within length without computing safety.
  // Propagate either with length (if no boundary found) or just cross boundary
  gGeoManager->FindNextBoundaryAndStep(length, kFALSE);
  Double_t step = 0.0; // Step made
  Double_t snext = gGeoManager->GetStep();
  // If no boundary within proposed length, return current density
  if (!gGeoManager->IsOnBoundary()) {
    mparam[0] = lparam[0];
    mparam[1] = lparam[4]/lparam[1];
    mparam[2] = lparam[2];
    mparam[3] = lparam[3];
    mparam[4] = lparam[4];
    return lparam[0];
  }
  // Try to cross the boundary and see what is next
  Int_t nzero = 0;
  while (length>TGeoShape::Tolerance()) {
    currentnode = gGeoManager->GetCurrentNode();
    if (snext<2.*TGeoShape::Tolerance()) nzero++;
    else nzero = 0;
    if (nzero>3) {
      // This means navigation has problems on one boundary
      // Try to cross by making a small step
      AliErrorClass("Cannot cross boundary\n");
      mparam[0] = bparam[0]/step;
      mparam[1] = bparam[1];
      mparam[2] = bparam[2]/step;
      mparam[3] = bparam[3]/step;
      mparam[5] = bparam[5]/step;
      mparam[4] = step;
      mparam[0] = 0.;             // if crash of navigation take mean density 0
      mparam[1] = 1000000;        // and infinite rad length
      return bparam[0]/step;
    }
    mparam[6]+=1.;
    step += snext;
    bparam[1]    += snext/lparam[1];
    bparam[2]    += snext*lparam[2];
    bparam[3]    += snext*lparam[3];
    bparam[5]    += snext*lparam[5];
    bparam[0]    += snext*lparam[0];

    if (snext>=length) break;
    if (!currentnode) break;
    length -= snext;
    material = currentnode->GetVolume()->GetMedium()->GetMaterial();
    lparam[0] = material->GetDensity();
    lparam[1]  = material->GetRadLen();
    lparam[2]  = material->GetA();
    lparam[3]  = material->GetZ();
    lparam[5]   = lparam[3]/lparam[2];
    if (material->IsMixture()) {
      TGeoMixture * mixture = (TGeoMixture*)material;
      lparam[5]=0;
      Double_t sum =0;
      for (Int_t iel=0;iel<mixture->GetNelements();iel++){
        sum+= mixture->GetWmixt()[iel];
        lparam[5]+= mixture->GetZmixt()[iel]*mixture->GetWmixt()[iel]/mixture->GetAmixt()[iel];
      }
      lparam[5]/=sum;
    }
    gGeoManager->FindNextBoundaryAndStep(length, kFALSE);
    snext = gGeoManager->GetStep();
  }
  mparam[0] = bparam[0]/step;
  mparam[1] = bparam[1];
  mparam[2] = bparam[2]/step;
  mparam[3] = bparam[3]/step;
  mparam[5] = bparam[5]/step;
  return bparam[0]/step;
}


Bool_t 
AliTrackerBase::PropagateTrackTo(AliExternalTrackParam *track, Double_t xToGo, 
			     Double_t mass, Double_t maxStep, Bool_t rotateTo, Double_t maxSnp, Double_t sign){
  //----------------------------------------------------------------
  //
  // Propagates the track to the plane X=xk (cm) using the magnetic field map 
  // and correcting for the crossed material.
  //
  // mass     - mass used in propagation - used for energy loss correction
  // maxStep  - maximal step for propagation
  //
  //  Origin: Marian Ivanov,  Marian.Ivanov@cern.ch
  //
  //----------------------------------------------------------------
  const Double_t kEpsilon = 0.00001;
  Double_t xpos     = track->GetX();
  Double_t dir      = (xpos<xToGo) ? 1.:-1.;
  //
  while ( (xToGo-xpos)*dir > kEpsilon){
    Double_t step = dir*TMath::Min(TMath::Abs(xToGo-xpos), maxStep);
    Double_t x    = xpos+step;
    Double_t xyz0[3],xyz1[3],param[7];
    track->GetXYZ(xyz0);   //starting global position

    Double_t bz=GetBz(xyz0); // getting the local Bz

    if (!track->GetXYZAt(x,bz,xyz1)) return kFALSE;   // no prolongation
    xyz1[2]+=kEpsilon; // waiting for bug correction in geo

    if (TMath::Abs(track->GetSnpAt(x,bz)) >= maxSnp) return kFALSE;
    if (!track->PropagateTo(x,bz))  return kFALSE;

    MeanMaterialBudget(xyz0,xyz1,param);	
    Double_t xrho=param[0]*param[4]*sign, xx0=param[1];

    if (!track->CorrectForMeanMaterial(xx0,xrho,mass)) return kFALSE;
    if (rotateTo){
      if (TMath::Abs(track->GetSnp()) >= maxSnp) return kFALSE;
      track->GetXYZ(xyz0);   // global position
      Double_t alphan = TMath::ATan2(xyz0[1], xyz0[0]); 
      //
      Double_t ca=TMath::Cos(alphan-track->GetAlpha()), 
               sa=TMath::Sin(alphan-track->GetAlpha());
      Double_t sf=track->GetSnp(), cf=TMath::Sqrt((1.-sf)*(1.+sf));
      Double_t sinNew =  sf*ca - cf*sa;
      if (TMath::Abs(sinNew) >= maxSnp) return kFALSE;
      if (!track->Rotate(alphan)) return kFALSE;
    }
    xpos = track->GetX();
  }
  return kTRUE;
}

Bool_t 
AliTrackerBase::PropagateTrackToBxByBz(AliExternalTrackParam *track,
Double_t xToGo, 
				   Double_t mass, Double_t maxStep, Bool_t rotateTo, Double_t maxSnp,Double_t sign){
  //----------------------------------------------------------------
  //
  // Propagates the track to the plane X=xk (cm)
  // taking into account all the three components of the magnetic field 
  // and correcting for the crossed material.
  //
  // mass     - mass used in propagation - used for energy loss correction
  // maxStep  - maximal step for propagation
  //
  //  Origin: Marian Ivanov,  Marian.Ivanov@cern.ch
  //
  //----------------------------------------------------------------
  const Double_t kEpsilon = 0.00001;
  Double_t xpos     = track->GetX();
  Double_t dir      = (xpos<xToGo) ? 1.:-1.;
  //
  while ( (xToGo-xpos)*dir > kEpsilon){
    Double_t step = dir*TMath::Min(TMath::Abs(xToGo-xpos), maxStep);
    Double_t x    = xpos+step;
    Double_t xyz0[3],xyz1[3],param[7];
    track->GetXYZ(xyz0);   //starting global position

    Double_t b[3]; GetBxByBz(xyz0,b); // getting the local Bx, By and Bz

    if (!track->GetXYZAt(x,b[2],xyz1)) return kFALSE;   // no prolongation
    xyz1[2]+=kEpsilon; // waiting for bug correction in geo

    if (TMath::Abs(track->GetSnpAt(x,b[2])) >= maxSnp) return kFALSE;
    if (!track->PropagateToBxByBz(x,b))  return kFALSE;

    MeanMaterialBudget(xyz0,xyz1,param);	
    Double_t xrho=param[0]*param[4]*sign, xx0=param[1];

    if (!track->CorrectForMeanMaterial(xx0,xrho,mass)) return kFALSE;
    if (rotateTo){
      if (TMath::Abs(track->GetSnp()) >= maxSnp) return kFALSE;
      track->GetXYZ(xyz0);   // global position
      Double_t alphan = TMath::ATan2(xyz0[1], xyz0[0]); 
      //
      Double_t ca=TMath::Cos(alphan-track->GetAlpha()), 
               sa=TMath::Sin(alphan-track->GetAlpha());
      Double_t sf=track->GetSnp(), cf=TMath::Sqrt((1.-sf)*(1.+sf));
      Double_t sinNew =  sf*ca - cf*sa;
      if (TMath::Abs(sinNew) >= maxSnp) return kFALSE;
      if (!track->Rotate(alphan)) return kFALSE;
    }
    xpos = track->GetX();
  }
  return kTRUE;
}

Double_t AliTrackerBase::GetTrackPredictedChi2(AliExternalTrackParam *track,
                                           Double_t mass, Double_t step,
			             const AliExternalTrackParam *backup) {
  //
  // This function brings the "track" with particle "mass" [GeV] 
  // to the same local coord. system and the same reference plane as 
  // of the "backup", doing it in "steps" [cm].
  // Then, it calculates the 5D predicted Chi2 for these two tracks
  //
  Double_t chi2=kVeryBig;
  Double_t alpha=backup->GetAlpha();
  if (!track->Rotate(alpha)) return chi2;

  Double_t xb=backup->GetX();
  Double_t sign=(xb < track->GetX()) ? 1. : -1.;
  if (!PropagateTrackTo(track,xb,mass,step,kFALSE,kAlmost1,sign)) return chi2;

  chi2=track->GetPredictedChi2(backup);

  return chi2;
}
