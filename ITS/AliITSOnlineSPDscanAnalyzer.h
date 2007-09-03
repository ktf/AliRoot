#ifndef ALIITSONLINESPDSCANANALYZER_H
#define ALIITSONLINESPDSCANANALYZER_H
/* Copyright(c) 2007-2009, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

////////////////////////////////////////////////////////////
// Author: Henrik Tydesjo                                 //
// This class is used in the detector algorithm framework //
// to process the data stored in special container files  //
// (see AliITSOnlineSPDscan). For instance, minimum       //
// threshold values can be calculated.                    //
////////////////////////////////////////////////////////////

#include <TString.h>

class AliITSOnlineSPDscan;
class AliITSOnlineCalibrationSPDhandler;
class TGraph;
class TH2F;

class AliITSOnlineSPDscanAnalyzer {

 public:
  AliITSOnlineSPDscanAnalyzer(const Char_t *fileName);
  AliITSOnlineSPDscanAnalyzer(const AliITSOnlineSPDscanAnalyzer& handle);
  ~AliITSOnlineSPDscanAnalyzer();

  AliITSOnlineSPDscanAnalyzer& operator=(const AliITSOnlineSPDscanAnalyzer& handle);

  void       SetParam(const Char_t *pname, const Char_t *pval);

  UInt_t     GetType() const {return fType;}
  UInt_t     GetDacId() const {return fDacId;}
  UInt_t     GetRouterNr() const {return fRouterNr;}

  Int_t      GetDelay(UInt_t hs, UInt_t chipNr);
  Int_t      GetMinTh(UInt_t hs, UInt_t chipNr);
  
  Int_t      GetNrNoisyUnima(UInt_t hs, UInt_t chipNr);

  Bool_t     ProcessDeadPixels(Char_t *oldCalibDir);
  Bool_t     ProcessNoisyPixels(Char_t *oldCalibDir);
  Bool_t     SaveDeadNoisyPixels(UInt_t module, Char_t *calibDir);

  Bool_t     ProcessNrTriggers();

  AliITSOnlineCalibrationSPDhandler* GetOnlineCalibrationHandler(UInt_t module);
  AliITSOnlineSPDscan* GetOnlineScan() {return fScanObj;}
  UInt_t     GetRouterNr();
  Bool_t     GetHalfStavePresent(UInt_t hs);

  TGraph*    GetNrTriggersG();
  TGraph*    GetMeanMultiplicityG(UInt_t hs, UInt_t chipNr);
  TGraph*    GetHitEventEfficiencyG(UInt_t hs, UInt_t chipNr);
  TH2F*      GetHitMapTot(UInt_t step);


 private:
  UInt_t               fType;           // calib type
  UInt_t               fDacId;          // dac id
  UInt_t               fRouterNr;       // router nr
  TString              fFileName;       // container file name
  enum                 calibvals{kMINTH,kMEANTH,kDAC,kUNIMA,kNOISE,kDELAY};  // calib types

  AliITSOnlineSPDscan               *fScanObj;      // container obj
  AliITSOnlineCalibrationSPDhandler *fHandler[240]; // calib help objs

  TGraph*    fMeanMultiplicity[6][11];   // mean mult graphs
  TGraph*    fHitEventEfficiency[6][11]; // hit event graphs
  TGraph*    fTriggers;                  // trigger graph

  void       Init();                     // init

  Bool_t     ProcessMeanMultiplicity();  // process mean mult
  Bool_t     ProcessHitEventEfficiency();// process hit event eff

  Int_t      FindLastMinThDac(UInt_t hs, UInt_t chipNr);  // dac value where fMinIncreaseFromBaseLine reached
  Int_t      FindClosestLowerStep(Float_t dacValueInput); // step closest (lower) to a dacvalue 
  Float_t    GetCompareLine(UInt_t step, UInt_t hs, UInt_t chipNr, Float_t basePar2); // line to compare mean mult with

  // dead noisy parameters:
  Bool_t     fOverWrite;          // overWrite old dead/noisy or just add new ones to it
  // noise scan parameters:
  Float_t    fNoiseThreshold;     // ratio of allowed hits/triggers
  UInt_t     fNoiseMinimumEvents; // minimum events required to find noisy pixels
  // min th scan parameters:
  UInt_t     fMinNrStepsBeforeIncrease; // min nr of steps required before fMinIncreaseFromBaseLine reached
  Float_t    fMinIncreaseFromBaseLine;  // min increase of mean mult from base line
  UInt_t     fStepDownDacSafe;          // nr of steps down to put threshold result (to be on the safe side)
  Float_t    fMaxBaseLineLevel;         // maximum value for the base line to compare with

};

#endif
