#ifndef ALIITSQACHECKER_H
#define ALIITSQACHECKER_H
/* Copyright(c) 2007-2009, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */


/* $Id$ */

//
//  Checks the quality assurance. 
//  By comparing with reference data
//  INFN Torino
//  W. Ferrarese Oct 2007
//


// --- ROOT system ---
class TFile ; 
class TH2F ;  

// --- AliRoot header files ---
#include "AliQA.h"
#include "AliQACheckerBase.h"

class AliITSQASPDChecker;
class AliITSQASDDChecker;
class AliITSQASSDChecker;
class AliITSLoader ; 

class AliITSQAChecker: public AliQACheckerBase {

friend class AliITSQASPDChecker;
friend class AliITSQASDDChecker;
friend class AliITSQASSDChecker;

public:
  AliITSQAChecker(Bool_t kMode = kFALSE, Short_t subDet = 0, Short_t ldc = 0) ;         // ctor
  //AliITSQAChecker(Int_t SPDoffset, Int_t SDDoffset, Int_t SSDoffset, Bool_t kMode = kFALSE, Short_t subDet = 0, Short_t ldc = 0) ;
  AliITSQAChecker(const AliITSQAChecker& qac) : AliQACheckerBase(qac.GetName(), qac.GetTitle()), fkOnline(kFALSE), fDet(0), fLDC(0), fSPDOffset(0), fSDDOffset(0), fSSDOffset(0), fSPDChecker(0), fSDDChecker(0), fSSDChecker(0) {;} // cpy ctor   
  AliITSQAChecker& operator = (const AliITSQAChecker& qac) ; //operator =
  virtual ~AliITSQAChecker() {;} // dtor
  void SetMode(Bool_t kMode) { fkOnline = kMode; }
  void SetSubDet(Short_t subdet) { fDet = subdet; }
  void SetLDC(Short_t ldc) { fLDC = ldc; }
  Bool_t GetMode() { return fkOnline; }
  Short_t GetSubDet() { return fDet; }
  Short_t GetLDC() { return fLDC; }
  virtual void SetTaskOffset(Int_t SPDOffset, Int_t SDDOffset, Int_t SSDOffset);

 protected:

  virtual const Double_t Check(AliQA::ALITASK_t index, TObjArray * list ) ;
  virtual const Double_t Check(AliQA::ALITASK_t /*index*/, TObjArray * /*list*/, Int_t /*SubDetOffset*/) {return 0.;};

private:

  Bool_t  fkOnline;
  Short_t fDet;  
  Short_t fLDC;
  Int_t fSPDOffset; //starting point for the QACheck list
  Int_t fSDDOffset;
  Int_t fSSDOffset;

  AliITSQASPDChecker *fSPDChecker;  // SPD Checker
  AliITSQASDDChecker *fSDDChecker;  // SDD Checker
  AliITSQASSDChecker *fSSDChecker;  // SSD Checker

  ClassDef(AliITSQAChecker,3)  // description 

};

#endif // AliITSQAChecker_H
