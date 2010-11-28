
#ifndef AlidNdPtAnalysisPbPb_H
#define AlidNdPtAnalysisPbPb_H


//------------------------------------------------------------------------------
// AlidNdPtAnalysisPbPb class used for dNdPt analysis.in PbPb collision 
// 
// Author: J.Otwinowski 04/11/2008 
//------------------------------------------------------------------------------

class iostream;

class TFile;
class TCint;
class TProfile;
class TFolder;
class TObjArray;
class TString;
class THnSparse;

class AliESDtrackCuts;
class AliVertexerTracks;
class AliESD;
class AliESDfriend;
class AliESDfriendTrack;
class AlidNdPtHelper;
class AliTriggerAnalysis;

#include "AlidNdPt.h"

class AlidNdPtAnalysisPbPb : public AlidNdPt {
public :
  AlidNdPtAnalysisPbPb(); 
  AlidNdPtAnalysisPbPb(Char_t* name, Char_t* title);
  ~AlidNdPtAnalysisPbPb();

  // Init data members
  virtual void Init();

  // Process events
  virtual void Process(AliESDEvent *const esdEvent=0, AliMCEvent *const mcEvent=0);

  // Merge output objects (needed by PROOF) 
  virtual Long64_t Merge(TCollection* const list);

  // Analyse output histograms 
  virtual void Analyse();

  // Export objects to folder
  virtual TFolder *ExportToFolder(TObjArray * const array=0);

  // Get analysis folder
  TFolder* GetAnalysisFolder() const {return fAnalysisFolder;}

  // Fill control histograms
  void SetHistogramsOn(const Bool_t histOn=kTRUE) {fHistogramsOn = histOn;}
  Bool_t IsHistogramsOn() const {return fHistogramsOn;}
    
  // Create folder for analysed histograms
  TFolder *CreateFolder(TString folder = "folderdNdPtAnalysis",TString title = "Analysed dNdPt histograms");

  // Fill histograms
  void FillHistograms(AliESDtrack *const esdTrack, AliStack *const stack, AlidNdPtHelper::TrackObject trackObj);
  void FillHistograms(AliStack *const stack, Int_t label, AlidNdPtHelper::TrackObject trackObj);
  void FillHistograms(TObjArray *const allChargedTracks,Int_t *const labelsAll,Int_t multAll,Int_t *const labelsAcc,Int_t multAcc,Int_t *const labelsRec,Int_t multRec);

  // Getters
  THnSparseF *GetTrackPtCorrelationMatrix()   const {return fTrackPtCorrelationMatrix;}
  //
  THnSparseF *GetGenEventMatrix() const {return fGenEventMatrix;}
  THnSparseF *GetTriggerEventMatrix() const {return fTriggerEventMatrix;}
  THnSparseF *GetRecEventMatrix() const {return fRecEventMatrix;}
  // 
  THnSparseF *GetGenTrackEventMatrix() const {return fGenTrackEventMatrix;}
  THnSparseF *GetTriggerTrackEventMatrix() const {return fTriggerTrackEventMatrix;}
  THnSparseF *GetRecTrackEventMatrix() const {return fRecTrackEventMatrix;}
  //
  THnSparseF *GetGenTrackMatrix() const {return fGenTrackMatrix;}
  THnSparseF *GetGenPrimTrackMatrix() const {return fGenPrimTrackMatrix;}
  THnSparseF *GetRecPrimTrackMatrix() const {return fRecPrimTrackMatrix;}

  THnSparseF *GetRecTrackMatrix() const {return fRecTrackMatrix;}
  THnSparseF *GetRecSecTrackMatrix() const {return fRecSecTrackMatrix;}
  THnSparseF *GetRecMultTrackMatrix() const {return fRecMultTrackMatrix;}
  //
  // control histograms
  //
  THnSparseF *GetMCEventHist1() const {return fMCEventHist1;}
  THnSparseF *GetRecEventHist1() const {return fRecEventHist1;}
  THnSparseF *GetRecEventHist2() const {return fRecEventHist2;}


  THnSparseF *GetRecMCEventHist1() const {return fRecMCEventHist1;}
  THnSparseF *GetRecMCTrackHist1() const {return fRecMCTrackHist1;}

  THnSparseF *GetRecMCEventHist2() const {return fRecMCEventHist2;}

  THnSparseF *GetMCTrackHist1(Int_t i) const {return fMCTrackHist1[i];}
  THnSparseF *GetMCPrimTrackHist1(Int_t i) const {return fMCPrimTrackHist1[i];}
  THnSparseF *GetMCPrimTrackHist2(Int_t i) const {return fMCPrimTrackHist2[i];}
  THnSparseF *GetMCSecTrackHist1(Int_t i) const {return fMCSecTrackHist1[i];}

  THnSparseF *GetRecTrackHist1(Int_t i) const {return fRecTrackHist1[i];}
  THnSparseF *GetRecTrackHist2(Int_t i) const {return fRecTrackHist2[i];}
  THnSparseF *GetRecTrackMultHist1(Int_t i) const {return fRecTrackMultHist1[i];}


  THnSparseF *GetMCMultRecTrackHist1() const {return fMCMultRecTrackHist1;}

  THnSparseF *GetRecTrackHist3() const {return fRecTrackHist3;}

private:

  // analysis folder 
  TFolder *fAnalysisFolder; // folder for analysed histograms
  Bool_t fHistogramsOn; // switch on/off filling of control histograms 

  // 
  // correlation matrices (histograms)
  //
  // rec. track pt vs true track pt correlation matrix for given eta
  THnSparseF *fTrackPtCorrelationMatrix; //-> Pt:mcPt:mcEta

  //
  // event level correction 
  //
  // all generated
  THnSparseF *fGenEventMatrix; //-> mcZv:multMB 

  // trigger bias corrections (fTriggerEventMatrix / fGenEventMatrix)
  THnSparseF *fTriggerEventMatrix; //-> mcZv:multMB

  // event vertex rec. eff correction (fRecEventMatrix / fTriggerEventMatrix)
  THnSparseF *fRecEventMatrix; //-> mcZv:multMB 

  // track-event level correction 
  THnSparseF *fGenTrackEventMatrix; //-> mcZv:mcPt:mcEta

  // trigger bias corrections (fTriggerTrackEventMatrix / fGenTrackEventMatrix)
  THnSparseF *fTriggerTrackEventMatrix; //-> mcZv:mcPt:mcEta

  // event vertex rec. corrections (fRecTrackEventMatrix / fTriggerTrackEventMatrix)
  THnSparseF *fRecTrackEventMatrix; //-> mcZv:Pt:mcEta

  //
  // track level correction 
  //
  // track rec. efficiency correction (fRecPrimTrackMatrix / fGenPrimTrackMatrix)
  THnSparseF *fGenTrackMatrix; //-> mcZv:mcPt:mcEta
  THnSparseF *fGenPrimTrackMatrix; //-> mcZv:mcPt:mcEta
  THnSparseF *fRecPrimTrackMatrix; //-> mcZv:mcPt:mcEta
  // secondary track contamination correction (fRecSecTrackMatrix / fRecTrackMatrix)
  THnSparseF *fRecTrackMatrix;    //-> mcZv:mcPt:mcEta
  THnSparseF *fRecSecTrackMatrix; //-> mcZv:mcPt:mcEta
  // multiple rec. track corrections (fRecMultTrackMatrix / fRecTrackMatrix)
  THnSparseF *fRecMultTrackMatrix; //-> mcZv:Pt:mcEta

  //
  // ESD and MC control analysis histograms
  //
  // THnSparse event histograms
  THnSparseF *fMCEventHist1;  //-> mcXv:mcYv:mcZv
  THnSparseF *fRecEventHist1; //-> Xv:Yv:Zv
  THnSparseF *fRecEventHist2; //-> Zv:multMB:mult
  THnSparseF *fRecMCEventHist1; //-> Xv-mcXv:Yv-mcYv:Zv-mcZv
  THnSparseF *fRecMCEventHist2; //-> Xv-mcXv:Zv-mcZv:mult

  // [0] - after charged track selection, [1] - after acceptance cuts, [2] - after esd track cuts
  THnSparseF *fMCTrackHist1[AlidNdPtHelper::kCutSteps];     //-> mcPt:mcEta:mcPhi
  THnSparseF *fMCPrimTrackHist1[AlidNdPtHelper::kCutSteps]; //-> mcPt:mcEta:pid:mech:mother
  THnSparseF *fMCPrimTrackHist2[AlidNdPtHelper::kCutSteps]; //-> pdg:mech:mother
  THnSparseF *fMCSecTrackHist1[AlidNdPtHelper::kCutSteps];  //-> mcPt:mcEta:pid:mech:mother

  THnSparseF *fRecTrackHist1[AlidNdPtHelper::kCutSteps];     //-> Pt:Eta:Phi
  THnSparseF *fRecTrackHist2[AlidNdPtHelper::kCutSteps];     //-> Zv:Pt:Eta
  THnSparseF *fRecTrackMultHist1[AlidNdPtHelper::kCutSteps]; //-> Pt:mult
  THnSparseF *fRecMCTrackHist1; //-> mcPt:mcEta:(Pt-mcPt)/mcPt:(Eta-mcEta)

  //multple reconstructed tracks
  THnSparseF *fMCMultRecTrackHist1; //-> mcPt:mcEta:pid
  // track control histograms
  THnSparseF *fRecTrackHist3;  //-> nclust:chi2:Pt:Eta:Phi

  AliTriggerAnalysis *fTriggerAnalysis; //! trigger analysis object;

  AlidNdPtAnalysisPbPb(const AlidNdPtAnalysisPbPb&); // not implemented
  AlidNdPtAnalysisPbPb& operator=(const AlidNdPtAnalysisPbPb&); // not implemented

  ClassDef(AlidNdPtAnalysisPbPb,4);
};

#endif
