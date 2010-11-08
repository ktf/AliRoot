#ifndef ALIANALYSISTASKTRIGGERSTUDY_H
#define ALIANALYSISTASKTRIGGERSTUDY_H

#include "AliAnalysisTaskSE.h"

//-------------------------------------------------------------------------
//                      AliAnalysisTaskTriggerStudy
// 
// 
//
//
// Author: Michele Floris, CERN
//-------------------------------------------------------------------------


class AliESDEvent;
class AliESDtrackCuts;
class AliHistoListWrapper;
class AliTriggerAnalysis;
class AliAnalysisTaskTriggerStudy : public AliAnalysisTaskSE {

  // offline trigger enum
  enum {kC0MBS1,kC0MBS2,kC0MBS3,kC0MBS4,kC0MBS5,kC0VBA,kC0VBC,kC0OM2,kCO0M3};
  // enum for triggers to be included in the venn-like histogram
  //  enum {kVDC0MBS1,kVDC0MBS2,kVDC0VBA,kVDC0VBC,kVDC0OM2,kNVDEntries};
  enum {kVDC0MBS1,kVDC0MBS2,kVDC0VBA,kVDC0VBC,kNVDEntries};
    
public:

  AliAnalysisTaskTriggerStudy();
  AliAnalysisTaskTriggerStudy(const char * name);
  AliAnalysisTaskTriggerStudy(const AliAnalysisTaskTriggerStudy& obj) ;
  ~AliAnalysisTaskTriggerStudy();

  void SetIsMC(Bool_t flag=kTRUE) { fIsMC = flag;}
  AliHistoListWrapper * GetHistoList() { return fHistoList;}

  void SetNTrackletsCut(Int_t cut) { fNTrackletsCut = cut;}

  TH1 * GetHistoTracklets   (const char * name, const char * title);
  void  FillTriggerOverlaps (const char * name, const char * title, Bool_t * vdArray) ;

  virtual void   UserCreateOutputObjects();
  virtual void   UserExec(Option_t *option);
  virtual void   Terminate(Option_t *);
  
  

private:

  //
  AliESDEvent *  fESD;    //! ESD object  AliVEvent*     fEvent;
  AliHistoListWrapper  * fHistoList; // wrapper for the list, takes care of merging + histo booking and getters  
  Bool_t fIsMC; // true if processing montecarlo
  
  AliTriggerAnalysis * fTriggerAnalysis; // trigger analysis object, to get the offline triggers
  TString fHistoSuffix; // suffix appended to all histos, set in the user exec.

  Int_t fNTrackletsCut; // max number of tracklets

  static const char * kVDNames[];       // names of the venn hist
  AliAnalysisTaskTriggerStudy& operator=(const AliAnalysisTaskTriggerStudy& task);
  
  ClassDef(AliAnalysisTaskTriggerStudy, 2)


};

#endif
