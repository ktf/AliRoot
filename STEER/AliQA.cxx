
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

//////////////////////////////////////////////////////////////////////////////
//
// Quality Assurance Object//_________________________________________________________________________
// Quality Assurance object. The QA status is held in one word per detector,
// each bit corresponds to a different status.
// bit 0-3  : QA raised during simulation      (SIM)
// bit 4-7  : QA raised during reconstruction  (REC)
// bit 8-11 : QA raised during ESD checking    (ESD)
// bit 12-15: QA raised during analysis        (ANA)
// Each of the 4 bits corresponds to a severity level of increasing importance
// from lower to higher bit (INFO, WARNING, ERROR, FATAL)
//
//*-- Yves Schutz CERN, July 2007 
//////////////////////////////////////////////////////////////////////////////


// --- ROOT system ---
#include <TFile.h>
#include <TSystem.h>
#include <TROOT.h>

// --- Standard library ---

// --- AliRoot header files ---
#include "AliCDBManager.h"
#include "AliLog.h"
#include "AliQA.h"


ClassImp(AliQA)
AliQA    * AliQA::fgQA                   = 0x0 ;
TFile    * AliQA::fgQADataFile           = 0x0 ;   
TString    AliQA::fgQADataFileName       = "QA" ;  // will transform into Det.QA.run.cycle.root  
TFile    * AliQA::fgQARefFile            = 0x0 ;   
TString    AliQA::fgQARefDirName	     = "" ; 
TString    AliQA::fgQARefFileName        = "QA.root" ;
TFile    * AliQA::fgQAResultFile         = 0x0 ;  
TString    AliQA::fgQAResultDirName      = "" ;  
TString    AliQA::fgQAResultFileName     = "QA.root" ; 
TString    AliQA::fgDetNames[]           = {"ITS", "TPC", "TRD", "TOF", "PHOS", "HMPID", "EMCAL", "MUON", "FMD",
										    "ZDC", "PMD", "T0", "VZERO", "ACORDE", "HLT", "Global"} ;   
TString       AliQA::fgTaskNames[]       = {"Raws", "Hits", "SDigits", "Digits", "RecPoints", "TrackSegments", "RecParticles", "ESDs"} ;   
const TString AliQA::fkgLabLocalFile     = "file://"  ; 
const TString AliQA::fkgLabLocalOCDB     = "local://" ;  
const TString AliQA::fkgLabAliEnOCDB     = "alien://" ;  
const TString AliQA::fkgRefFileName      = "QA.root" ; 
const TString AliQA::fkgQAName           = "QA"  ; 
const TString AliQA::fkgRefOCDBDirName   = "Ref"  ; 
TString AliQA::fkgRefDataDirName   = "Data"  ; 
const TString AliQA::fkgQARefOCDBDefault = "alien://folder=/alice/QA/20"  ; 
//____________________________________________________________________________
AliQA::AliQA() : 
  TNamed("", ""), 
  fNdet(kNDET), 
  fQA(new ULong_t[fNdet]), 
  fDet(kNULLDET),
  fTask(kNULLTASK)
  	
{
  // default constructor
  // beware singleton: not to be used
  
  for (Int_t index = 0 ; index < fNdet ; index++) 
	fQA[index] = 0 ; 
}

//____________________________________________________________________________
AliQA::AliQA(const AliQA& qa) :
  TNamed(qa),
  fNdet(qa.fNdet), 
  fQA(qa.fQA), 
  fDet(qa.fDet),
  fTask(qa.fTask)
{ 
  // cpy ctor
}

//_____________________________________________________________________________
AliQA& AliQA::operator = (const AliQA& qa)
{
// assignment operator

  this->~AliQA();
  new(this) AliQA(qa);
  return *this;
}

//_______________________________________________________________
AliQA::AliQA(const DETECTORINDEX_t det) :
  TNamed("QA", "Quality Assurance status"),
  fNdet(kNDET),  
  fQA(new ULong_t[fNdet]), 
  fDet(det),
  fTask(kNULLTASK) 
{
  // constructor to be used
  if (! CheckRange(det) ) {
    fDet = kNULLDET ; 
    return ;
  } 
  Int_t index ; 
  for (index = 0; index < fNdet; index++) 
    fQA[index] = 0 ; 
}
  
//_______________________________________________________________
AliQA::AliQA(const ALITASK_t tsk) :
  TNamed("QA", "Quality Assurance status"), 
  fNdet(kNDET),
  fQA(new ULong_t[fNdet]), 
  fDet(kNULLDET),
  fTask(tsk)
{
  // constructor to be used in the AliRoot module (SIM, REC, ESD or ANA)
  if (! CheckRange(tsk) ) {
    fTask = kNULLTASK ; 
    return ;
  } 
  Int_t index ; 
  for (index = 0; index < fNdet; index++) 
    fQA[index] = 0 ; 
}

//____________________________________________________________________________
AliQA::~AliQA() 
{
  // dtor  
  delete[] fQA ;
}

//_______________________________________________________________
void AliQA::Close() 
{
	// close the open files
	if (fgQADataFile) 
		if (fgQADataFile->IsOpen())
			fgQADataFile->Close() ; 
	if (fgQAResultFile) 
		if (fgQAResultFile->IsOpen()) 
			fgQAResultFile->Close() ;
	if (fgQARefFile)
		if (fgQARefFile->IsOpen())
			fgQARefFile->Close() ; 
} 

//_______________________________________________________________
const Bool_t AliQA::CheckFatal() const
{
  // check if any FATAL status is set
  Bool_t rv = kFALSE ;
  Int_t index ;
  for (index = 0; index < kNDET ; index++)
    rv = rv || IsSet(DETECTORINDEX_t(index), fTask, kFATAL) ;
  return rv ;
}

//_______________________________________________________________
const Bool_t AliQA::CheckRange(DETECTORINDEX_t det) const
{ 
  // check if detector is in given detector range: 0-kNDET

  Bool_t rv = ( det < 0 || det > kNDET )  ? kFALSE : kTRUE ;
  if (!rv)
    AliFatal(Form("Detector index %d is out of range: 0 <= index <= %d", det, kNDET)) ;
  return rv ;
}

//_______________________________________________________________
const Bool_t AliQA::CheckRange(ALITASK_t task) const
{ 
  // check if task is given taskk range: 0:kNTASK
  Bool_t rv = ( task < kRAW || task > kNTASK )  ? kFALSE : kTRUE ;
  if (!rv)
    AliFatal(Form("Module index %d is out of range: 0 <= index <= %d", task, kNTASK)) ;
  return rv ;
}

//_______________________________________________________________
const Bool_t AliQA::CheckRange(QABIT_t bit) const
{ 
  // check if bit is in given bit range: 0-kNBit

  Bool_t rv = ( bit < 0 || bit > kNBIT )  ? kFALSE : kTRUE ;
  if (!rv)
    AliFatal(Form("Status bit %d is out of range: 0 <= bit <= %d", bit, kNBIT)) ;
  return rv ;
}



//_______________________________________________________________
const char * AliQA::GetAliTaskName(ALITASK_t tsk)
{
  // returns the char name corresponding to module index
  TString tskName ;
  switch (tsk) {
  case kNULLTASK:
    break ; 
  case kRAW:
    tskName = "RAW" ;
    break ;  
  case kSIM:
    tskName = "SIM" ;
    break ;
  case kREC:
    tskName = "REC" ;
    break ;
  case kESD:
    tskName = "ESD" ;
    break ;
  case kANA:
    tskName = "ANA" ;
    break ;
  default:
    tsk = kNULLTASK ; 
    break ;
  }
  return tskName.Data() ;
}

//_______________________________________________________________
const AliQA::DETECTORINDEX_t AliQA::GetDetIndex(const char * name) 
{
	// returns the detector index corresponding to a given name
	TString sname(name) ; 
	DETECTORINDEX_t rv = kNULLDET ; 
	for (Int_t det = 0; det < kNDET ; det++) {
		if ( GetDetName(det) == sname ) {
			rv = DETECTORINDEX_t(det) ; 
			break ; 
		}
	}
	return rv ; 		
}

//_______________________________________________________________
const char * AliQA::GetDetName(Int_t det) 
{
	// returns the detector name corresponding to a given index (needed in a loop)
	
	if ( det >= 0 &&  det < kNDET) 
		return (fgDetNames[det]).Data() ; 
	else 
		return NULL ; 
}

//_______________________________________________________________
TFile * AliQA::GetQADataFile(const char * name, const Int_t run, const Int_t cycle) 
{
  // opens the file to store the detectors Quality Assurance Data Maker results
	const char * temp = Form("%s.%s.%d.%d.root", name, fgQADataFileName.Data(), run, cycle) ; 
	TString opt ; 
	if (! fgQADataFile ) {     
		if  (gSystem->AccessPathName(temp))
			opt = "NEW" ;
		else 
			opt = "UPDATE" ; 
		fgQADataFile = TFile::Open(temp, opt.Data()) ;
	} else {
		if ( strcmp(temp, fgQADataFile->GetName()) != 0 ) {
			fgQADataFile = dynamic_cast<TFile *>(gROOT->FindObject(temp)) ; 
			if ( !fgQADataFile ) {
				if  (gSystem->AccessPathName(temp))
					opt = "NEW" ;
				else 
					opt = "UPDATE" ; 
				fgQADataFile = TFile::Open(temp, opt.Data()) ;
			}
		}
  }
	return fgQADataFile ;
} 

//_____________________________________________________________________________
TFile * AliQA::GetQADataFile(const char * fileName)
{
  // Open if necessary the Data file and return its pointer

  if (!fgQADataFile) 
	if (!fileName) 
		fileName = AliQA::GetQADataFileName() ; 
	if  (!gSystem->AccessPathName(fileName)) {
		fgQADataFile =  TFile::Open(fileName) ;
	} else {
		printf("File %s not found", fileName) ;
		exit(1) ;  
	}
  return fgQADataFile ; 
}

//_______________________________________________________________
TFile * AliQA::GetQAResultFile() 
{
  // opens the file to store the  Quality Assurance Data Checker results
	if (fgQAResultFile) 
		fgQAResultFile->Close() ; 
	fgQAResultFile = 0x0 ; 
//	if (!fgQAResultFile) { 
		TString dirName(fgQAResultDirName) ; 
		if ( dirName.Contains(fkgLabLocalFile)) 
			dirName.ReplaceAll(fkgLabLocalFile, "") ;
		TString fileName(dirName + fgQAResultFileName) ; 
		TString opt("") ; 
		if ( !gSystem->AccessPathName(fileName) )
			opt = "UPDATE" ; 
		else { 
			if ( gSystem->AccessPathName(dirName) )
				gSystem->mkdir(dirName) ; 
			opt = "NEW" ; 
		}
		fgQAResultFile = TFile::Open(fileName, opt) ;   
//	}
	
	return fgQAResultFile ;
}

//_______________________________________________________________
const Bool_t AliQA::IsSet(DETECTORINDEX_t det, ALITASK_t tsk, QABIT_t bit) const
{
  // Checks is the requested bit is set

  CheckRange(det) ; 
  CheckRange(tsk) ;
  CheckRange(bit) ;

  ULong_t offset = Offset(tsk) ;
  ULong_t status = GetStatus(det) ;
  offset+= bit ;
  status = (status & 1 << offset) != 0 ;
  return status ;
}

//_______________________________________________________________
AliQA * AliQA::Instance()
{
  // Get an instance of the singleton.
  // Object must have been instantiated with Instance(ALITASK) first

  return fgQA ;
}

//_______________________________________________________________
AliQA * AliQA::Instance(const DETECTORINDEX_t det)
{
  // Get an instance of the singleton. The only authorized way to call the ctor
  
  if ( ! fgQA) {
    TFile * f = GetQAResultFile() ; 
	fgQA = dynamic_cast<AliQA *>(f->Get("QA")) ; 
    if ( ! fgQA ) 
		fgQA = new AliQA(det) ;
  }		
  fgQA->Set(det) ;
  return fgQA ;
}

//_______________________________________________________________
AliQA * AliQA::Instance(const ALITASK_t tsk)
{
  // get an instance of the singleton.

  if ( ! fgQA)
    switch (tsk) {
    case kNULLTASK:
      break ;
	case kRAW:
      fgQA = new AliQA(tsk) ;
      break ;
	case kSIM:
      fgQA = new AliQA(tsk) ;
      break ;
    case kREC:
      printf("fgQA = gAlice->GetQA()") ;
      break ;
    case kESD:
      printf("fgQA = dynamic_cast<AliQA *> (esdFile->Get(\"QA\")") ;
      break ;
    case kANA:
      printf("fgQA = dynamic_cast<AliQA *> (esdFile->Get(\"QA\")") ;
      break ;
    case kNTASK:
      break ;
    }
  if (fgQA) 
    fgQA->Set(tsk) ;
  return fgQA ;
}

//_______________________________________________________________
AliQA *  AliQA::Instance(const TASKINDEX_t tsk) 
{
	// get an instance of the singleton.
	
	ALITASK_t index = kNULLTASK ; 

	if ( tsk == kRAWS )
		index = kRAW ;
	else if (tsk < kDIGITS)
		index = kSIM ;
	else if (tsk < kRECPARTICLES)
		index = kREC ; 
	else if (tsk == kESDS) 
		index = kESD ; 

	return Instance(index) ; 
}

//_______________________________________________________________
const ULong_t AliQA::Offset(ALITASK_t tsk) const
{
  // Calculates the bit offset for a given module (SIM, REC, ESD, ANA)

  CheckRange(tsk) ; 

  ULong_t offset = 0 ;
  switch (tsk) {
  case kNULLTASK:
    break ;
  case kRAW:
    offset+= 0 ;
    break ;
  case kSIM:
    offset+= 4 ;
    break ;
  case kREC:
    offset+= 8 ;
    break ;
  case kESD:
    offset+= 12 ;
    break ;
  case kANA:
    offset+= 16 ;
    break ;
  case kNTASK:
    break ;
  }

  return offset ;
}

//_______________________________________________________________
void AliQA::Set(QABIT_t bit)
{
  // Set the status bit of the current detector in the current module
  
  SetStatusBit(fDet, fTask, bit) ;
}

//_____________________________________________________________________________
void AliQA::SetQARefStorage(const char * name)
{
	// Set the root directory where the QA reference data are stored

	fgQARefDirName = name ; 
	if ( fgQARefDirName.Contains(fkgLabLocalFile) )
		fgQARefFileName =  fkgRefFileName ; 
	else if ( fgQARefDirName.Contains(fkgLabLocalOCDB) )
		fgQARefFileName =  fkgQAName ; 
	else if ( fgQARefDirName.Contains(fkgLabAliEnOCDB) )
		fgQARefFileName =  fkgQAName ; 

  else {
	  printf("ERROR: %s is an invalid storage definition\n", name) ; 
	  fgQARefDirName  = "" ; 
	  fgQARefFileName = "" ; 
  }	
	TString tmp(fgQARefDirName) ; // + fgQARefFileName) ;
	printf("AliQA::SetQARefDir: QA references are in  %s\n", tmp.Data() ) ;

	AliCDBManager* man = AliCDBManager::Instance() ; 
	if ( ! man->IsDefaultStorageSet() ) { 
		man->SetDefaultStorage(AliQA::GetQARefStorage()) ; 
		man->SetSpecificStorage("*", AliQA::GetQARefStorage()) ;
	}
}

//_____________________________________________________________________________
void AliQA::SetQAResultDirName(const char * name)
{
  // Set the root directory where to store the QA status object

  fgQAResultDirName.Prepend(name) ; 
  printf("AliQA::SetQAResultDirName: QA results are in  %s\n", fgQAResultDirName.Data()) ;
  if ( fgQAResultDirName.Contains(fkgLabLocalFile)) 
    fgQAResultDirName.ReplaceAll(fkgLabLocalFile, "") ;
  fgQAResultFileName.Prepend(fgQAResultDirName) ;
}

//_______________________________________________________________
void AliQA::SetStatusBit(DETECTORINDEX_t det, ALITASK_t tsk, QABIT_t bit)
{
 // Set the status bit for a given detector and a given task

  CheckRange(det) ;
  CheckRange(tsk) ;
  CheckRange(bit) ;

  ULong_t offset = Offset(tsk) ;
  ULong_t status = GetStatus(det) ;
  offset+= bit ;
  status = status | 1 << offset ;
  SetStatus(det, status) ;
}

//_______________________________________________________________
void AliQA::ShowAll() const
{
  // dispplay the QA status word
  Int_t index ;
  for (index = 0 ; index < kNDET ; index++)
    ShowStatus(DETECTORINDEX_t(index)) ;
}

//_______________________________________________________________
void AliQA::ShowStatus(DETECTORINDEX_t det) const
{
  // Prints the full QA status of a given detector
  CheckRange(det) ;
  ULong_t status = GetStatus(det) ;
  ULong_t rawStatus = status & 0x0000f ;
  ULong_t simStatus = status & 0x000f0 ;
  ULong_t recStatus = status & 0x00f00 ;
  ULong_t esdStatus = status & 0x0f000 ;
  ULong_t anaStatus = status & 0xf0000 ;

  AliInfo(Form("QA Status for %8s raw =0x%x, sim=0x%x, rec=0x%x, esd=0x%x, ana=0x%x\n", GetDetName(det).Data(), rawStatus, simStatus, recStatus, esdStatus, anaStatus )) ;
}

//_______________________________________________________________
void AliQA::UnSet(QABIT_t bit)
{
	// UnSet the status bit of the current detector in the current module
	
	UnSetStatusBit(fDet, fTask, bit) ;
}

//_______________________________________________________________
void AliQA::UnSetStatusBit(DETECTORINDEX_t det, ALITASK_t tsk, QABIT_t bit)
{
	// UnSet the status bit for a given detector and a given task
	
	CheckRange(det) ;
	CheckRange(tsk) ;
	CheckRange(bit) ;
	
	ULong_t offset = Offset(tsk) ;
	ULong_t status = GetStatus(det) ;
	offset+= bit ;
	status = status & 0 << offset ;
	SetStatus(det, status) ;
}
