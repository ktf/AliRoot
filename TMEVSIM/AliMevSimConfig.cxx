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
//__________________________________________________________________
////////////////////////////////////////////////////////////////////
//
// class AliMevSimConfig
//
// Class containing configuation inforamtion for MeVSim generator
// --------------------------------------------
// --------------------------------------------
// --------------------------------------------
// author: radomski@if.pw.edu.pl
//
////////////////////////////////////////////////////////////////////

#include "AliMevSimConfig.h"

ClassImp(AliMevSimConfig)


//////////////////////////////////////////////////////////////////////////////////////////////////

AliMevSimConfig::AliMevSimConfig() {
//def ctor
  Init();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

AliMevSimConfig::AliMevSimConfig(Int_t modelType) {
//ctor
  Init();
  SetModelType(modelType);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

AliMevSimConfig::~AliMevSimConfig() {
//dtor
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AliMevSimConfig::Init() {
  // Default Values

  fModelType = 1;
  fReacPlaneCntrl = 4;
  fPsiRMean = fPsiRStDev = 0;

  fMultFacMean  = 1.0;
  fMultFacStDev = 0.0;

  fNStDevMult = fNStDevTemp = fNStDevSigma = 3.0;
  fNStDevExpVel = fNStdDevPSIr = fNStDevVn = fNStDevMultFac = 3.0;
  
  fNIntegPts = fNScanPts = 100;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
 
void AliMevSimConfig::SetModelType(Int_t modelType) {
//Sets type of the model
  if (modelType < 0 || modelType > fgkMAX_MODEL)
    Error("SetModelType","Wrog Model Type indentifier (%d)",modelType);

  fModelType = modelType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AliMevSimConfig::SetRectPlane(Int_t ctrl, Float_t psiRMean, Float_t psiRStDev) {
//Sets reaction plane parameters
  if (ctrl < 0 || ctrl > fgkMAX_CTRL)
    Error("SetReactPlane","Wrong Control Parameter (%d)", ctrl);

  fReacPlaneCntrl = ctrl;
  fPsiRMean = psiRMean;
  fPsiRStDev = psiRStDev;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AliMevSimConfig::SetMultFac(Float_t mean, Float_t stDev) {
  //Sets multiplicity mean and variance
  fMultFacMean = mean;
  fMultFacStDev = stDev;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AliMevSimConfig::SetStDev(Float_t mult, Float_t temp, Float_t sigma,
  Float_t expVel, Float_t psiR, Float_t Vn, Float_t multFac) {
//sets Dev parameters (whatever Dev is)
  fNStDevMult = mult;
  fNStDevTemp = temp;
  fNStDevSigma = sigma;
  fNStDevExpVel = expVel;
  fNStdDevPSIr = psiR;
  fNStDevVn = Vn;
  fNStDevMultFac =multFac;

}
void AliMevSimConfig::GetStDev(Float_t& mult, Float_t& temp, Float_t& sigma,
                Float_t& expVel, Float_t& psiR, Float_t& Vn, Float_t& multFac) const
{
 //returns dev parameters
   mult  = fNStDevMult;
   temp  = fNStDevTemp;
   sigma  = fNStDevSigma;
   expVel  = fNStDevExpVel;
   psiR  = fNStdDevPSIr;
   Vn  = fNStDevVn;
   multFac  = fNStDevMultFac;
 
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void AliMevSimConfig::SetGrid(Int_t integr, Int_t scan) {
//Sets grid 
  fNIntegPts = integr;
  fNScanPts = scan;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
 
