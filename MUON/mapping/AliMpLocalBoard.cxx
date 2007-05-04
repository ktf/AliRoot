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


//
// --------------------
// Class AliMpLocalBoard
// --------------------
// The class defines the properties of local board
// Author: Ch. Finck, Subatech Nantes

#include "AliMpLocalBoard.h"
#include "AliMpConstants.h"
#include "AliMpIntPair.h"

#include "AliLog.h"

#include <TString.h>
#include <Riostream.h>

/// \cond CLASSIMP
ClassImp(AliMpLocalBoard)
/// \endcond


//_____________________________________________________________________________
AliMpLocalBoard::AliMpLocalBoard(Int_t id, const Char_t* name, Int_t slot)
    : TNamed(name, "mapping trigger local board"),
      fId(id),
      fSlot(slot),
      fTC(true),
      fCrate(),
      fSwitches(false),
      fNotified(true),
      fDEId(false)

{
/// Default constructor
}

//______________________________________________________________________________
AliMpLocalBoard::AliMpLocalBoard(TRootIOCtor* /*ioCtor*/)
    : TNamed(),
      fId(),
      fSlot(),
      fTC(),
      fCrate(),
      fSwitches(),
      fNotified(),
      fDEId()
{
/// Root IO constructor
}


//_____________________________________________________________________________
AliMpLocalBoard::~AliMpLocalBoard() 
{
/// Destructor

}

//_____________________________________________________________________________
Int_t AliMpLocalBoard::GetIndex(Int_t chamberId) const
{
/// Return the index from chamver Id.
/// chamberId could range from 10-13 in absolute value
/// chamberId could also range from 0-3 in relative value

   Int_t index = chamberId;
   
   if ( chamberId >= AliMpConstants::NofTrackingChambers() && 
        chamberId <  AliMpConstants::NofChambers() )
   {
       index -= AliMpConstants::NofTrackingChambers();
   } 

   if (index < 0 || index >=  AliMpConstants::NofTriggerChambers() ) 
   {
     AliError(Form("chamber# %d not a valid trigger chamber Id, [0-3] or [10-13]", chamberId));
     return -1;
   }

   return index;
}


//______________________________________________________________________________
Bool_t AliMpLocalBoard::AddDE(Int_t detElemId)
{
/// Add detection element with given detElemId.
/// Return true if the detection element was added

 if ( HasDEId(detElemId) ) {
    AliWarningStream() 
      << "Detection element Id = " << detElemId << " already present."
      << endl;
    return false;
 }

  fDEId.Add(detElemId);
  return true;
}   


//______________________________________________________________________________
Int_t AliMpLocalBoard::GetNofDEs() const
{  
/// Return the number of detection elements connected to this crate

  return fDEId.GetSize(); 
}

//______________________________________________________________________________
Int_t  AliMpLocalBoard::GetDEId(Int_t index) const
{  
/// Return the detection element by index (in loop)

  return fDEId.GetValue(index); 
}

//______________________________________________________________________________
Int_t  AliMpLocalBoard::GetDEIdByChamber(Int_t chamberId) const
{  
/// Return the detection element by index (in loop)

  return fDEId.GetValue(GetIndex(chamberId)); 
}

//______________________________________________________________________________
Bool_t  AliMpLocalBoard::HasDEId(Int_t detElemId) const
{  
/// Return true if the detection element Id is present

  return fDEId.HasValue(detElemId); 
}

//______________________________________________________________________________
Bool_t AliMpLocalBoard::AddSwitch(Int_t swit)
{
/// Add a swicth for the given local board
/// Return true if switch was added

    if ( swit > 1 ) {
      AliWarningStream() 
	  << "Invalid value for switch = " << swit 
	  << endl;
      return false;
    }

    fSwitches.Add(swit);
    return true;
}   


//______________________________________________________________________________
Int_t AliMpLocalBoard::GetNofSwitches() const
{  
/// Return the number switches in this local board

  return fSwitches.GetSize(); 
}

//______________________________________________________________________________
Int_t  AliMpLocalBoard::GetSwitch(Int_t index) const
{
/// Return switch by index (in loop)

    if (index < fSwitches.GetSize())
	return fSwitches.GetValue(index);
    else 
	AliWarning("Switch index too large");

    return -1;
}

//______________________________________________________________________________
AliMpIntPair AliMpLocalBoard::GetPosition() const
{
/// gives position of the local board in (line, col)

    const Char_t* boardName = GetName();
    Int_t iLine = boardName[4] - '0';
    Int_t iCol = boardName[2] - '0';
    if ( iLine == 5 ) --iCol;

    return (AliMpIntPair(iLine, iCol));

}

