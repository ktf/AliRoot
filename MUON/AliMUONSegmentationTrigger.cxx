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

#include <TMath.h>
//#include <TRandom.h>
//#include <TArc.h>
//#include <Riostream.h>

#include "AliMUONSegmentationTrigger.h"
#include "AliMUONTriggerConstants.h"
#include "AliRun.h"
#include "AliMUON.h"
#include "AliMUONChamber.h"

ClassImp(AliMUONSegmentationTrigger)

//------------------------------------------------------------------
AliMUONSegmentationTrigger::AliMUONSegmentationTrigger()
  : AliMUONSegmentationV0()
{
// Constructor

  fChamber=0;
}

//------------------------------------------------------------------
void AliMUONSegmentationTrigger::Init(Int_t chamber)
{
  // initialize Module geometry
  AliMUON *pMUON  = (AliMUON *) gAlice->GetModule("MUON");
  AliMUONChamber* iChamber=&(pMUON->Chamber(chamber));

  if(pMUON->GetDebug()>1) printf("%s: Initialize Trigger Chamber Module Geometry\n",ClassName());

  Float_t zPos=iChamber->Z();
  Float_t z1Pos=-1603.5;
  fZscale = zPos/z1Pos;

  Float_t y1Cmin[126];
  Float_t y1Cmax[126];

  Float_t dz=7.2;
//  Float_t z1PosPlus=z1Pos+dz/2.;
// Float_t z1PosMinus=z1Pos-dz/2.;
  Float_t z1PosPlus=z1Pos-dz/2.;
  Float_t z1PosMinus=z1Pos+dz/2.;

  Float_t z1pm=z1PosPlus/z1PosMinus;
  Float_t z1mp=z1PosMinus/z1PosPlus;

  if(pMUON->GetDebug()>1) printf("%s: fZscale = %f \n",ClassName(),fZscale);
  
// calculate yCmin and fYcmax 
  Int_t i;  
  for (i=62; i>=0; i--) {
    Int_t j=ModuleNumber(-AliMUONTriggerConstants::ModuleId(i));  // i == right, j == left 
    if (Int_t(AliMUONTriggerConstants::ModuleId(i)/10)==5) {  // start with middle chamber
      if (AliMUONTriggerConstants::ModuleId(i)==51) {         // special case (empty module)
	fYcmin[i]=fYcmax[i]=fYcmin[j]=fYcmax[j]=0.;
      } else {
	y1Cmin[i]=y1Cmin[j]=-34;
	y1Cmax[i]=y1Cmax[j]=34;
	fYcmin[i]=fYcmin[j]=-34.;

	fYcmax[i]=fYcmax[j]=34.;
      }
    } else if (Int_t(AliMUONTriggerConstants::ModuleId(i)/10)==4) { // up
      if (AliMUONTriggerConstants::ModuleId(i)!=41) {       
	y1Cmin[i]=y1Cmax[i+7]*z1pm;
	y1Cmax[i]=y1Cmin[i]+68.;
	fYcmin[i]=y1Cmin[i];
	fYcmax[i]=fYcmin[i]+68.;

	y1Cmin[j]=y1Cmax[j+7]*z1mp;
	y1Cmax[j]=y1Cmin[j]+68.;
	fYcmin[j]=y1Cmin[j];
	fYcmax[j]=fYcmin[j]+68.;
      } else { 
	y1Cmin[i]=y1Cmin[ModuleNumber(42)]+17;
	y1Cmax[i]=y1Cmin[i]+51.;
	fYcmin[i]=y1Cmin[i];
	fYcmax[i]=fYcmin[i]+51.;

	y1Cmin[j]=y1Cmin[ModuleNumber(-42)]+17;
	y1Cmax[j]=y1Cmin[j]+51.;
	fYcmin[j]=y1Cmin[j];
	fYcmax[j]=fYcmin[j]+51.;
      }
    } else if (Int_t(AliMUONTriggerConstants::ModuleId(i)/10)==3) { 
      y1Cmin[i]=y1Cmax[i+7]*z1mp;
      y1Cmax[i]=y1Cmin[i]+68.;
      fYcmin[i]=y1Cmin[i];
      fYcmax[i]=fYcmin[i]+68.;

      y1Cmin[j]=y1Cmax[j+7]*z1pm;
      y1Cmax[j]=y1Cmin[j]+68.;
      fYcmin[j]=y1Cmin[j];
      fYcmax[j]=fYcmin[j]+68.;
    } else if (Int_t(AliMUONTriggerConstants::ModuleId(i)/10)==2) {
      y1Cmin[i]=y1Cmax[i+7]*z1pm;
      y1Cmax[i]=y1Cmin[i]+68.;
      fYcmin[i]=y1Cmin[i];
      fYcmax[i]=fYcmin[i]+68.;

      y1Cmin[j]=y1Cmax[j+7]*z1mp;
      y1Cmax[j]=y1Cmin[j]+68.;
      fYcmin[j]=y1Cmin[j];
      fYcmax[j]=fYcmin[j]+68.;
    } else if (Int_t(AliMUONTriggerConstants::ModuleId(i)/10)==1) {
      y1Cmin[i]=y1Cmax[i+7]*z1mp;
      y1Cmax[i]=y1Cmin[i]+68.;
      fYcmin[i]=y1Cmin[i];
      fYcmax[i]=fYcmin[i]+68.;

      y1Cmin[j]=y1Cmax[j+7]*z1pm;
      y1Cmax[j]=y1Cmin[j]+68.;
      fYcmin[j]=y1Cmin[j];
      fYcmax[j]=fYcmin[j]+68.;
    }
  }

  for (i=0; i<63; i++) {      // second loop (fill lower part)
    Int_t j=ModuleNumber(-AliMUONTriggerConstants::ModuleId(i));  // i == right, j == left 
    if (TMath::Abs(Int_t(AliMUONTriggerConstants::ModuleId(i)/10))==6) { 
      fYcmin[i]=-fYcmax[i-14];
      fYcmax[i]=-fYcmin[i-14];
      fYcmin[j]=-fYcmax[j-14];
      fYcmax[j]=-fYcmin[j-14];
    } else if (TMath::Abs(Int_t(AliMUONTriggerConstants::ModuleId(i)/10))==7) { 
      fYcmin[i]=-fYcmax[i-28];
      fYcmax[i]=-fYcmin[i-28];
      fYcmin[j]=-fYcmax[j-28];
      fYcmax[j]=-fYcmin[j-28];
    } else if (TMath::Abs(Int_t(AliMUONTriggerConstants::ModuleId(i)/10))==8) { 
      fYcmin[i]=-fYcmax[i-42];
      fYcmax[i]=-fYcmin[i-42];
      fYcmin[j]=-fYcmax[j-42];
      fYcmax[j]=-fYcmin[j-42];
    } else if (TMath::Abs(Int_t(AliMUONTriggerConstants::ModuleId(i)/10))==9) { 
      fYcmin[i]=-fYcmax[i-56];
      fYcmax[i]=-fYcmin[i-56];
      fYcmin[j]=-fYcmax[j-56];
      fYcmax[j]=-fYcmin[j-56];
    } 
  }

  fNpx=124;
  fNpy=64;  

// Set parent chamber number
  fChamber=&(pMUON->Chamber(chamber));
  fId=chamber;
}
//------------------------------------------------------------------
void  AliMUONSegmentationTrigger::Draw(const char * /*opt*/) {}
//------------------------------------------------------------------
Int_t AliMUONSegmentationTrigger::ModuleNumber(Int_t imodule){
// returns module number (from 0 to 126) corresponding to module imodule
  Int_t imod=0;
  for (Int_t i=0; i<AliMUONTriggerConstants::Nmodule(); i++) {
    if (AliMUONTriggerConstants::ModuleId(i)==imodule) { 
      imod=i;
      break;
    }
  }
  return imod;
}
//------------------------------------------------------------------
void   AliMUONSegmentationTrigger::SetHit(Float_t xhit, Float_t yhit, Float_t /*zhit*/)
{
  SetHit(xhit, yhit);
}
//------------------------------------------------------------------
Float_t AliMUONSegmentationTrigger::StripSizeX(Int_t imodule){
// Returns x-strip size for given module imodule

  Int_t absimodule=TMath::Abs(imodule); 
  Int_t moduleNum=ModuleNumber(imodule);
  if (absimodule==51) {
    return 0; 
  } else {
    return TMath::Abs((fYcmax[moduleNum]-fYcmin[moduleNum])/
		      AliMUONTriggerConstants::NstripX(moduleNum));
  }  
}

//------------------------------------------------------------------
Float_t AliMUONSegmentationTrigger::StripSizeY(Int_t imodule, Int_t istrip){
// Returns y-strip size for given module imodule
        
  Int_t absimodule=TMath::Abs(imodule); 
  Int_t moduleNum=ModuleNumber(imodule);
  if (absimodule==51) {
      return 0;
  } else if (TMath::Abs(imodule-10*Int_t(imodule/10.))==7) {
      if (istrip<8) {
	  return 4.25;
      }
      else if (istrip>7) {	      
	  return 2.125;
      } 
      else {
	  return 0.;
      }      
  } else {
      return TMath::Abs((AliMUONTriggerConstants::XcMax(moduleNum) - 
			 AliMUONTriggerConstants::XcMin(moduleNum)) / 
			AliMUONTriggerConstants::NstripY(moduleNum));
  }
}

//------------------------------------------------------------------   
void AliMUONSegmentationTrigger::SetHit(Float_t xhit, Float_t yhit)
{
    // Sets virtual hit position, needed for evaluating pad response 
    // outside the tracking program 
    
  fXhit=xhit;
  fYhit=yhit;
}









