#ifndef ALIQADATAMAKER_H
#define ALIQADATAMAKER_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */


/* $Id$ */

//
//  Base Class:
//  Produces the data needed to calculate the quality assurance. 
//  All data must be mergeable objects.
//  Y. Schutz CERN July 2007
//


// --- ROOT system ---
#include <TH1.h>
#include <TObjArray.h>
#include <TNamed.h>  
class TClonesArray;
class TDirectory;
class TFile;  
class TObject; 
class TTree; 
class AliESDEvent;
class AliRawReader;
class AliDetectorRecoParam;

// --- Standard library ---

// --- AliRoot header files ---
#include "AliQA.h"

class AliQADataMaker: public TNamed {
  
public:
	
	AliQADataMaker(const char * name="", const char * title="") ;          // ctor
	AliQADataMaker(const AliQADataMaker& qadm) ;   
	virtual ~AliQADataMaker() {} // dtor
  
	virtual Int_t Add2DigitsList(TH1 * hist, const Int_t index, const Bool_t expert = kFALSE)          = 0 ; 
	virtual Int_t Add2ESDsList(TH1 * hist, const Int_t index, const Bool_t expert = kFALSE)            = 0 ; 
	virtual Int_t Add2HitsList(TH1 * hist, const Int_t index, const Bool_t expert = kFALSE)            = 0 ; 
	virtual Int_t Add2RecPointsList(TH1 * hist, const Int_t index, const Bool_t expert = kFALSE)       = 0 ; 
	virtual Int_t Add2RawsList(TH1 * hist, const Int_t index, const Bool_t expert = kFALSE, const Bool_t saveForCorr = kFALSE)            = 0 ; 
	virtual Int_t Add2SDigitsList(TH1 * hist, const Int_t index, const Bool_t expert = kFALSE)         = 0 ; 
	virtual void        Exec(AliQA::TASKINDEX_t, TObject * data)         = 0 ;
	virtual void        EndOfCycle()                                     = 0 ;
	virtual void        EndOfCycle(AliQA::TASKINDEX_t)                   = 0 ;
	void                Finish() const ; 
	virtual TH1 *       GetDigitsData(const Int_t index)                       = 0 ; 
	virtual TH1 *       GetESDsData(const Int_t index)                         = 0 ; 
	virtual TH1 *       GetHitsData(const Int_t index)                         = 0 ; 
	virtual TH1 *       GetRecPointsData(const Int_t index)                    = 0 ; 
	virtual TH1 *       GetRawsData(const Int_t index)                         = 0 ; 
	virtual TH1 *       GetSDigitsData(const Int_t index)                      = 0 ; 
	const char *        GetDetectorDirName() const { return fDetectorDirName.Data() ; }
  TList *             GetParameterList() const { return fParameterList ; }
	Int_t         Increment() { return ++fCycleCounter ; } 
	virtual TObjArray * Init(AliQA::TASKINDEX_t, Int_t cycles = -1)                              = 0 ;
	virtual void        Init(AliQA::TASKINDEX_t, TObjArray * list, Int_t run, Int_t cycles = -1) = 0 ;
	Bool_t        IsCycleDone() const { return fCycleCounter > fCycle ? kTRUE : kFALSE ; }
	void                Reset() { fCycleCounter = 0 ; }
	void                SetCycle(Int_t nevts) { fCycle = nevts ; } 
  void                SetWriteExpert() { fWriteExpert = kTRUE ; }
	virtual void        StartOfCycle(Int_t run = -1)                                                      = 0 ;
	virtual void        StartOfCycle(AliQA::TASKINDEX_t, Int_t run, const Bool_t sameCycle = kFALSE) = 0 ;
  void                UnSetWriteExpert() { fWriteExpert = kFALSE ; }
  Bool_t              WriteExpert() { return fWriteExpert ; }

	virtual void        SetRecoParam(const AliDetectorRecoParam */*param*/) { return; }
	  
protected: 

	Int_t          Add2List(TH1 * hist, const Int_t index, TObjArray * list, const Bool_t expert = kFALSE, const Bool_t saveForCorr = kFALSE) ;
	virtual void   DefaultEndOfDetectorCycle(AliQA::TASKINDEX_t task ) ; 
	virtual void   EndOfDetectorCycle(AliQA::TASKINDEX_t task, TObjArray * obj ) = 0 ; 
	TObject *      GetData(TObjArray * list, const Int_t index) ;
	virtual void   InitDigits()        = 0 ; 
	virtual void   InitESDs()          = 0 ; 
	virtual void   InitHits()          = 0 ; 
  //virtual void   InitRecParticles()  = 0 ; 
	virtual void   InitRecPoints()     = 0 ; 
	virtual void   InitRaws()          = 0 ; 
	virtual void   InitSDigits()       = 0 ; 
  //virtual void   InitTrackSegments()  = 0 ; 
	virtual void   MakeESDs(AliESDEvent * )          = 0 ; 
	virtual void   MakeHits(TClonesArray * )         = 0 ; 
	virtual void   MakeHits(TTree * )                = 0 ;  
	virtual void   MakeDigits(TClonesArray * )       = 0 ;  
	virtual void   MakeDigits(TTree * )              = 0 ; 
  //virtual void   MakeRecParticles(TClonesArray * ) = 0 ; 
	virtual void   MakeRaws(AliRawReader *)          = 0 ; 
	virtual void   MakeRecPoints(TTree * )           = 0 ; 
	virtual void   MakeSDigits(TClonesArray * )      = 0 ;  
	virtual void   MakeSDigits(TTree * )             = 0 ;  
  //virtual void   MakeTrackSegments(TTree * )		 = 0 ;  
	void           ResetCycle() { fCurrentCycle++ ; fCycleCounter = 0 ; } 
	virtual void   StartOfDetectorCycle()            = 0 ;
	
	TFile *        fOutput ;          //! output root file
	TDirectory *   fDetectorDir ;     //! directory for the given detector in the file
	TString        fDetectorDirName ; //! detector directory name in the quality assurance data file
	Int_t          fCurrentCycle ;    //! current cycle number
	Int_t          fCycle ;           //! length (# events) of the QA data acquisition cycle  
	Int_t          fCycleCounter ;    //! cycle counter
  Bool_t         fWriteExpert ;     //! flag to write or not the expert QA data
  TList *        fParameterList ;   //! list of QA data parameters
	Int_t          fRun ;             //! run number

private:
	AliQADataMaker& operator = (const AliQADataMaker& /*qadm*/); // Not implemented

  
 ClassDef(AliQADataMaker,1)  // description 

};

#endif // AliQADataMaker_H
