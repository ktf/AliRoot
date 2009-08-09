#ifndef __AliAnaESDSpectraQA_hh__
#define __AliAnaESDSpectraQA_hh__

#include "AliAnalysisTask.h"

class TH1F;
class TH2F;
class TH3F;
class TList;
class TDirectory;
class AliESDEvent;
class AliESDtrackCuts;

class AliAnaESDSpectraQA: public AliAnalysisTask {

 public:
  AliAnaESDSpectraQA();
  AliAnaESDSpectraQA(const char *name);
  ~AliAnaESDSpectraQA() {;}

  virtual void   ConnectInputData(Option_t *);
  virtual void   CreateOutputObjects();
  virtual void   Exec(Option_t *option);



 private:

  void InitHistPointers();
  AliAnaESDSpectraQA(const AliAnaESDSpectraQA&);
  AliAnaESDSpectraQA& operator=(const AliAnaESDSpectraQA&);


  AliESDEvent *fESD;    //! ESD object
  AliESDtrackCuts *fTrackCuts;

  enum {kNegA,kPosA,kNegC,kPosC};
  struct hists {
    TH3F *fPhiPtNPointTPC;
    TH3F *fPhiPtNPointITS;
    TH3F *fPhiPtChisqC;
    TH3F *fPhiPtChisqTPC;
    TH3F *fPhiPtDCAR;
    TH3F *fPhiPtDCAZ;
    TH3F *fPhiPtSigmaToVertex;
  };

  hists fHists[4];  //! Internal pointers to hists, do not stream

  TH1F *fNEvent;
  TH1F *fPtAll;
  TH1F *fPtSel;

  static const Int_t fgkNPtBins;
  static const Float_t fgkPtMin;
  static const Float_t fgkPtMax;
  static const Int_t fgkNPhiBins;

  TList *fHistList;

  ClassDef(AliAnaESDSpectraQA,1) 
  
};
#endif
