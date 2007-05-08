#ifndef ALI_SURVEY_OBJ_H
#define ALI_SURVEY_OBJ_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/////////////////////////////////////////////////////////////////////
//                                                                 //
//  class AliSurveyObj						   //
//  Retrieve and Convert survey data into ROOT Objects   	   //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#include <TObject.h>
//#include <sstream>

#include <TString.h>
#include <TObjArray.h>

//#include "AliLog.h"
#include "AliSurveyPoint.h"

class AliSurveyObj: public TObject {

 public:
  AliSurveyObj();
  ~AliSurveyObj();
  Bool_t GetFromLocalFile(const Char_t* filename);
  Bool_t Get(TString detector, Int_t reportNumber, Int_t reportVersion);

  
  Int_t GetEntries() const {return fDataPoints->GetEntries();};
  Bool_t IsValid() const {return fIsValid;};

  
 private:
  TString fTitle;     // Report Title
  TString fDate;      // Report Date
  TString fDetector;  // Subdetector (or structure) surveyed
  TString fURL;       // Report URL in EDMS
  Int_t fReportNr;    // Report Number
  Int_t fVersion;     // Report Version
  TString fObs;       // General observations / comments
  TString fCoordSys;  // Measurements coordinate system
  TString fUnits;     // Measurements units
  Int_t fNrColumns;   // Number of columns in data values
  TString fColNames;  // Column names sepparated by commas
  Bool_t fIsValid;    // Is the data valid? (sucessfully parsed)
  
  TObjArray *fDataPoints;	// Actual Data
  
  Bool_t Connect(const char *gridUrl, const char *user);
  Bool_t OpenFile(TString openString);
  TString &Sanitize(TString str);
  Bool_t ParseBuffer(const Char_t* buf);
  AliSurveyObj (const AliSurveyObj& surveyObj);            // copy constructor
  AliSurveyObj& operator=(const AliSurveyObj& surveyObj);  // assignment operator 
  void AddPoint(AliSurveyPoint* point);
  ClassDef(AliSurveyObj, 1);
};

#endif
