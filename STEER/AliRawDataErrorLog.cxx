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

/////////////////////////////////////////////////////////////////////
//   Implementation of AliRawDataErrorLog class                    //
//                                                                 //
// class AliRawDataErrorLog                                        //
// This is a class for logging raw-data related errors.            //
// It is used to record and retrieve of the errors                 //
// during the reading and reconstruction of raw-data and ESD       //
// analysis.                                                       //
// Further description of the methods and functionality are given  //
// inline.                                                         //
//                                                                 //
// cvetan.cheshkov@cern.ch                                         //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#include "AliRawDataErrorLog.h"

ClassImp(AliRawDataErrorLog)

//_____________________________________________________________________________
AliRawDataErrorLog::AliRawDataErrorLog() :
  TNamed(),
  fEventNumber(-1),
  fDdlID(-1),
  fErrorType(AliRawDataErrorLog::kNone)
{
  // Default constructor
}

//_____________________________________________________________________________
AliRawDataErrorLog::AliRawDataErrorLog(Int_t eventNumber, Int_t ddlId,
				       ERawDataErrorType errorType,
				       const char *message) :
  TNamed(message,""),
  fEventNumber(eventNumber),
  fDdlID(ddlId),
  fErrorType(errorType)
{
  // Constructor that specifies
  // the event number, ddl id, error type and
  // custom message related to the error
}

//___________________________________________________________________________
AliRawDataErrorLog::AliRawDataErrorLog(const AliRawDataErrorLog & source) :
  TNamed(source),
  fEventNumber(source.fEventNumber),
  fDdlID(source.fDdlID),
  fErrorType(source.fErrorType)
{
  // Copy constructor
}

//___________________________________________________________________________
AliRawDataErrorLog & AliRawDataErrorLog::operator=(const AliRawDataErrorLog &source)
{
  // assignment operator
  if (this != &source) {
    TNamed::operator=(source);

    fEventNumber = source.GetEventNumber();
    fDdlID       = source.GetDdlID();
    fErrorType   = source.GetErrorType();
  }
  return *this;
}

//_____________________________________________________________________________
Int_t AliRawDataErrorLog::Compare(const TObject *obj) const
{
  // Compare the event numbers and DDL IDs
  // of two error log objects.
  // Used in the sorting of raw data error logs
  // during the raw data reading and reconstruction
  Int_t eventNumber = ((AliRawDataErrorLog*)obj)->GetEventNumber();
  Int_t ddlID = ((AliRawDataErrorLog*)obj)->GetDdlID();

  if (fEventNumber == eventNumber) {
    if (fDdlID == ddlID)
      return 0;
    else
      return ((fDdlID > ddlID) ? 1 : -1);
  }
  else
    return ((fEventNumber > eventNumber) ? 1 : -1);
}
