/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *;
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

////////////////////////////////////////////////
//  RecPoints classes for set ZDC             //
//  This class reconstructs the space         //
//  points from digits                        //
//  for the ZDC calorimeter                   //
////////////////////////////////////////////////


#include "AliZDCReco.h"

ClassImp(AliZDCReco)
  

//_____________________________________________________________________________
AliZDCReco::AliZDCReco() :
	
  TObject(),
  fZN1Energy(0),
  fZP1Energy(0),
  fZN2Energy(0),
  fZP2Energy(0),
  //
  fZEMsignal(0),
  //
  fNDetSpecNLeft(0),
  fNDetSpecPLeft(0),
  fNDetSpecNRight(0),
  fNDetSpecPRight(0),
  fNTrueSpecNLeft(0),
  fNTrueSpecPLeft(0),
  fNTrueSpecLeft(0),
  fNTrueSpecNRight(0),
  fNTrueSpecPRight(0),
  fNTrueSpecRight(0),
  fNPartLeft(0),
  fNPartRight(0),
  fImpPar(0)

{ 
  //
  // Default constructor
  //
  for(Int_t i=0; i<5; i++){
     fZN1EnTow[i] = 0;
     fZP1EnTow[i] = 0;
     fZN2EnTow[i] = 0;
     fZP2EnTow[i] = 0;
     fZN1SigLowRes[i] = 0;
     fZP1SigLowRes[i] = 0;
     fZN2SigLowRes[i] = 0;
     fZP2SigLowRes[i] = 0;
     
  }
}
  

//_____________________________________________________________________________
AliZDCReco::AliZDCReco(Float_t ezn1, Float_t ezp1, Float_t ezn2, Float_t ezp2,  
	     //
	     Float_t* ezn1tow, Float_t* ezp1tow,
	     Float_t* ezn2tow, Float_t* ezp2tow, 
	     Float_t* ezn1siglr, Float_t* ezp1siglr,
	     Float_t* ezn2siglr, Float_t* ezp2siglr,
	     Float_t ezem, 
	     //	   
	     Int_t detspnLeft,  Int_t detsppLeft, Int_t detspnRight,
	     Int_t detsppRight,  Int_t trspnLeft, Int_t trsppLeft, 
	     Int_t trspLeft, Int_t partLeft, Int_t trspnRight, 
	     Int_t trsppRight, Int_t trspRight, Int_t partRight,  
	     Float_t b) :
	
  TObject(),
  fZN1Energy(ezn1),
  fZP1Energy(ezp1),
  fZN2Energy(ezn2),
  fZP2Energy(ezp2),
  //
  fZEMsignal(ezem),
  //
  fNDetSpecNLeft(detspnLeft),
  fNDetSpecPLeft(detsppLeft),
  fNDetSpecNRight(detspnRight),
  fNDetSpecPRight(detsppRight),
  fNTrueSpecNLeft(trspnLeft),
  fNTrueSpecPLeft(trsppLeft),
  fNTrueSpecLeft(trspLeft),
  fNTrueSpecNRight(trspnRight),
  fNTrueSpecPRight(trsppRight),
  fNTrueSpecRight(trspRight),
  fNPartLeft(partLeft),
  fNPartRight(partRight),
  fImpPar(b)

{ 
  //
  // Constructor
  //
  for(Int_t j=0; j<5; j++){
     fZN1EnTow[j] =  ezn1tow[j];
     fZP1EnTow[j] =  ezp1tow[j];
     fZN2EnTow[j] =  ezn2tow[j];
     fZP2EnTow[j] =  ezp2tow[j];
     fZN1SigLowRes[j] = ezn1siglr[j];
     fZP1SigLowRes[j] = ezp1siglr[j];
     fZN2SigLowRes[j] = ezn2siglr[j];
     fZP2SigLowRes[j] = ezp2siglr[j];
  }
  
}

//______________________________________________________________________________
AliZDCReco::AliZDCReco(const AliZDCReco &oldreco) :

  TObject()
{
  // Copy constructor

  fZN1Energy  = oldreco.GetZN1Energy();
  fZP1Energy  = oldreco.GetZP1Energy();            
  fZN2Energy  = oldreco.GetZN2Energy();	   
  fZP2Energy  = oldreco.GetZP2Energy(); 	   
  //
  for(Int_t i=0; i<5; i++){	  
     fZN1EnTow[i]  = oldreco.GetZN1EnTow(i);
     fZP1EnTow[i]  = oldreco.GetZP1EnTow(i);
     fZN2EnTow[i]  = oldreco.GetZN2EnTow(i);
     fZP2EnTow[i]  = oldreco.GetZP2EnTow(i);
     fZN1SigLowRes[i] = oldreco.GetZN1SigLowRes(i);
     fZP1SigLowRes[i] = oldreco.GetZP1SigLowRes(i);
     fZN2SigLowRes[i] = oldreco.GetZN2SigLowRes(i);
     fZP2SigLowRes[i] = oldreco.GetZP2SigLowRes(i);
  }
  //
  fZEMsignal = oldreco.GetZEMsignal();	
  //   
  fNDetSpecNLeft = oldreco.GetNDetSpecNLeft();	
  fNDetSpecPLeft = oldreco.GetNDetSpecPLeft();	
  fNDetSpecNRight = oldreco.GetNDetSpecNRight();	
  fNDetSpecPRight = oldreco.GetNDetSpecPRight();	
  fNTrueSpecNLeft = oldreco.GetNTrueSpecNLeft();	  
  fNTrueSpecPLeft = oldreco.GetNTrueSpecPLeft();	  
  fNTrueSpecLeft = oldreco.GetNTrueSpecLeft();	  
  fNTrueSpecNRight = oldreco.GetNTrueSpecNRight();	  
  fNTrueSpecPRight = oldreco.GetNTrueSpecPRight();	  
  fNTrueSpecRight = oldreco.GetNTrueSpecRight();	  
  fNPartLeft = oldreco.GetNPartLeft();			 
  fNPartRight = oldreco.GetNPartRight();			 
  fImpPar = oldreco.GetImpPar();			 
}

//______________________________________________________________________________
void AliZDCReco::Print(Option_t *) const {
  //
  // Printing Reconstruction Parameters
  //
  printf(" \t ---   Reconstruction -> EZN = %f TeV, EZP = %f TeV,  EZEM = %f GeV \n "		
	 " \t NDetSpecNLeft = %d, NDetSpecPLeft = %d, NspecnLeft = %d,"
	 " NspecpLeft = %d, NpartLeft = %d"
	 " \t NDetSpecNRight = %d, NDetSpecPRight = %d, NspecnRight = %d,"
	 " NspecpRight = %d, NpartRight = %d"
	 " \t b = %f fm\n ", 
	 fZN1Energy,fZP1Energy,fZEMsignal,
	 fNDetSpecNLeft,fNDetSpecPLeft,fNTrueSpecNLeft,fNTrueSpecPLeft,fNPartLeft,
	 fNDetSpecNRight,fNDetSpecPRight,fNTrueSpecNRight,fNTrueSpecPRight,fNPartRight,
	 fImpPar);
}
