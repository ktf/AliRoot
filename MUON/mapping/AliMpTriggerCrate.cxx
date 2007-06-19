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
// Class AliMpTriggerCrate
// --------------------
// The class defines the properties of trigger crate
// Author: Ch. Finck, Subatech Nantes

#include "AliMpTriggerCrate.h"
#include "AliMpDEManager.h"

#include "AliLog.h"

#include <Riostream.h>

/// \cond CLASSIMP
ClassImp(AliMpTriggerCrate)
/// \endcond


//______________________________________________________________________________
TString AliMpTriggerCrate::GenerateName(Int_t crateId, Int_t ddlId, Int_t nofDDLs)
{
/// Generate name

  TString name;
  // \todo parameterise this
  if (crateId == 23)
      name = "2-3";
  else 
      name = Form("%d", crateId);
 
  if (ddlId == nofDDLs)
      name.Append("R");
  else 
      name.Append("L"); 

  return name;
}  
 

//______________________________________________________________________________
AliMpTriggerCrate::AliMpTriggerCrate(const Char_t* name, Int_t ddlId)
  : TNamed(name, "mapping trigger crate"),
    fDdlId(ddlId),
    fLocalBoard(false)
 
{
/// Standard constructor
}

//______________________________________________________________________________
AliMpTriggerCrate::AliMpTriggerCrate(TRootIOCtor* /*ioCtor*/)
  : TNamed(),
    fDdlId(),
    fLocalBoard()
{
/// Root IO constructor
}

//______________________________________________________________________________
AliMpTriggerCrate::~AliMpTriggerCrate()
{
/// Destructor
}

//
// public methods
//

//______________________________________________________________________________
Bool_t AliMpTriggerCrate::AddLocalBoard(Int_t localBoardId)
{
/// Add detection element with given detElemId.
/// Return true if the detection element was added

  if ( HasLocalBoard(localBoardId) ) {
    AliWarningStream() 
      << "Local board with Id=" << localBoardId << " already present."
      << endl;
    return false;
  }    

  fLocalBoard.Add(localBoardId);
  return true;
}   


//______________________________________________________________________________
Int_t AliMpTriggerCrate::GetNofLocalBoards() const
{  
/// Return the number of local board in this crate

  return fLocalBoard.GetSize(); 
}

//______________________________________________________________________________
Int_t  AliMpTriggerCrate::GetLocalBoardId(Int_t index) const
{  
/// Return the local board by index (in loop)

  if (index >= 0 && index < fLocalBoard.GetSize())
      return fLocalBoard.GetValue(index); 
  else 
      return 0; // begin at 1
}

//______________________________________________________________________________
Bool_t  AliMpTriggerCrate::HasLocalBoard(Int_t localBoardId) const
{  
/// Return true if crate has local boardwith given localBoardId

  return fLocalBoard.HasValue(localBoardId); 
}

