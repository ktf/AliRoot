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

// AliFlowEventCuts:
// An event cut class for the flow framework
//
// origin: Mikolaj Krzewicki (mikolaj.krzewicki@cern.ch)

#include <limits.h>
#include <float.h>
#include "TNamed.h"
#include "AliVEvent.h"
#include "AliESDEvent.h"
#include "AliMCEvent.h"
#include "AliFlowEventCuts.h"
#include "AliESDtrackCuts.h"

ClassImp(AliFlowEventCuts)

//-----------------------------------------------------------------------
AliFlowEventCuts::AliFlowEventCuts():
  TNamed(),
  fCutNumberOfTracks(kFALSE),
  fNumberOfTracksMax(INT_MAX),
  fNumberOfTracksMin(INT_MIN),
  fCutRefMult(kFALSE),
  fRefMultMax(INT_MAX),
  fRefMultMin(INT_MIN),
  fReferenceMultiplicity(-1)
{
  //constructor 
}

//-----------------------------------------------------------------------
AliFlowEventCuts::AliFlowEventCuts(const char* name, const char* title):
  TNamed(name, title),
  fCutNumberOfTracks(kFALSE),
  fNumberOfTracksMax(INT_MAX),
  fNumberOfTracksMin(INT_MIN),
  fCutRefMult(kFALSE),
  fRefMultMax(INT_MAX),
  fRefMultMin(INT_MIN),
  fReferenceMultiplicity(-1)
{
  //constructor 
}

////-----------------------------------------------------------------------
//AliFlowEventCuts::AliFlowEventCuts(const AliFlowEventCuts& someCuts):
//  TNamed(),
//  fCutNumberOfTracks(that.fCutNumberOfTracks),
//  fNumberOfTracksMax(that.fNumberOfTracksMax),
//  fNumberOfTracksMin(that.fNumberOfTracksMin),
//{
//  //copy constructor 
//}
//
////-----------------------------------------------------------------------
//AliFlowEventCuts& AliFlowEventCuts::operator=(const AliFlowEventCuts& someCuts)
//{
//  //assignment
//  fCutNumberOfTracks=that.fCutNumberOfTracks;
//  fNumberOfTracksMax=that.fNumberOfTracksMax;
//  fNumberOfTracksMin=that.fNumberOfTracksMin;
//
//  return *this;
//}

//----------------------------------------------------------------------- 
Bool_t AliFlowEventCuts::IsSelected(const TObject* obj)
{
  //check cuts
  const AliVEvent* vevent = dynamic_cast<const AliVEvent*>(obj);
  if (vevent) return PassesCuts(vevent);
  return kFALSE;  //when passed wrong type of object
}
//----------------------------------------------------------------------- 
Bool_t AliFlowEventCuts::PassesCuts(const AliVEvent *event)
{
  ///check if event passes cuts
  if(fCutNumberOfTracks) {if (event->GetNumberOfTracks() < fNumberOfTracksMin || event->GetNumberOfTracks() >= fNumberOfTracksMax ) return kFALSE;}
  if(fCutRefMult)
  {
    //reference multiplicity still to be defined
    fReferenceMultiplicity = ReferenceMultiplicity(event);
    if (fReferenceMultiplicity < fRefMultMin || fReferenceMultiplicity >= fRefMultMax )
      return kFALSE;
  }
  return kTRUE;
}

//----------------------------------------------------------------------- 
AliFlowEventCuts* AliFlowEventCuts::StandardCuts()
{
  //make a set of standard event cuts, caller becomes owner
  AliFlowEventCuts* cuts = new AliFlowEventCuts();
  return cuts;
}

//----------------------------------------------------------------------- 
Int_t AliFlowEventCuts::ReferenceMultiplicity(const AliVEvent* event)
{
  //calculate the reference multiplicity
  const AliESDEvent* esd=dynamic_cast<const AliESDEvent*>(event);
  if (esd) return AliESDtrackCuts::GetReferenceMultiplicity(const_cast<AliESDEvent*>(esd),kTRUE);
  AliMCEvent* mc=const_cast<AliMCEvent*>(dynamic_cast<const AliMCEvent*>(event));
  if (mc) return mc->GetNumberOfPrimaries();
  return event->GetNumberOfTracks(); //default
}
