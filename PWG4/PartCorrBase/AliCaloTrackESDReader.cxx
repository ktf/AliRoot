
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
/* $Id:  $ */

//_________________________________________________________________________
// Class for reading data (ESDs) in order to do prompt gamma 
// or other particle identification and correlations
//
//
//
//
//*-- Author: Gustavo Conesa (LNF-INFN) 
//////////////////////////////////////////////////////////////////////////////


// --- ROOT system ---
//#include "Riostream.h"

//---- ANALYSIS system ----
#include "AliCaloTrackESDReader.h" 
#include "AliAODEvent.h"
#include "AliMultiEventInputHandler.h"
#include "AliAnalysisManager.h"
#include "AliMixedEvent.h"
#include "AliESDVZERO.h"


ClassImp(AliCaloTrackESDReader)

//____________________________________________________________________________
AliCaloTrackESDReader::AliCaloTrackESDReader() : 
AliCaloTrackReader()
{
  //Default Ctor
  
  //Initialize parameters
  fDataType=kESD;
  fReadStack          = kTRUE;
  fReadAODMCParticles = kFALSE;

}

//____________________________________________________________________________
Double_t AliCaloTrackESDReader::GetBField() const {
  //Return magnetic field

  Double_t bfield = fInputEvent->GetMagneticField();

  return bfield;
}


//____________________________________________________________________________
void AliCaloTrackESDReader::SetInputOutputMCEvent(AliVEvent* esd, AliAODEvent* aod, AliMCEvent* mc) {
  // Connect the data pointers
  
  Bool_t tesd = kFALSE ; 

  if ( strcmp(esd->GetName(), "AliMixedEvent") == 0 ) {
    AliMultiEventInputHandler* multiEH = dynamic_cast<AliMultiEventInputHandler*>((AliAnalysisManager::GetAnalysisManager())->GetInputEventHandler());
    if(multiEH){
      if (multiEH->GetFormat() == 0 ) {
        tesd = kTRUE ; 
      }
    }
    else{
      printf("AliCaloTrackESDReader::SetInputOutputMCEvent() - MultiEventHandler is NULL");
      abort();
    }
  }
  if (strcmp(esd->GetName(),"AliESDEvent") == 0) {
    tesd = kTRUE ; 
  }
  
  if(!tesd){
    AliFatal(Form("AliCaloTrackESDReader::SetInputOutputMCEvent() - STOP ::Wrong reader, here only ESDs. Input name: %s != AliESDEvent \n",esd->GetName()));
  }
  
  SetInputEvent(esd);
  SetOutputEvent(aod);
  SetMC(mc);
  
}

//____________________________________________________________________________
void AliCaloTrackESDReader::FillInputVZERO(){
  //Fill VZERO information in data member, add all the channels information.
  AliESDVZERO* esdV0 = ((AliESDEvent*)fInputEvent)->GetVZEROData();
  //printf("Init V0: ADC (%d,%d), Multiplicity (%d,%d) \n",fV0ADC[0],fV0ADC[1],fV0Mul[0],fV0Mul[1]);

  if (esdV0) 
  {
    for (Int_t i = 0; i < 32; i++)
    {
      fV0ADC[0] += (Int_t)esdV0->GetAdcV0C(i);
      fV0ADC[1] += (Int_t)esdV0->GetAdcV0A(i);
      fV0Mul[0] += (Int_t)esdV0->GetMultiplicityV0C(i);
      fV0Mul[1] += (Int_t)esdV0->GetMultiplicityV0A(i);
    }
    if(fDebug > 0)
      printf("V0: ADC (%d,%d), Multiplicity (%d,%d) \n",fV0ADC[0],fV0ADC[1],fV0Mul[0],fV0Mul[1]);
  }
  else
  {
    printf("Cannot retrieve V0 ESD! Run w/ null V0 charges");
  }
}
