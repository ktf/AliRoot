// --
// --
// Implementation for TTree output in PHOS DA
// for calibrating energy by pi0 and MIP.
// --
// -- Author: Hisayuki Torii (Hiroshima Univ.)
// --
#ifndef AliPHOSDApi0mip_H
#define AliPHOSDApi0mip_H

#include <time.h>

#include "TNamed.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TFile.h"
#include "TTree.h"
#include "AliPHOSDATreeEvent.h"

class AliPHOSDApi0mip : public TNamed {
 public:
  AliPHOSDApi0mip(int module,int iterid=0,char* fopt="RECREATE");
  AliPHOSDApi0mip(const AliPHOSDApi0mip& da);
  ~AliPHOSDApi0mip();
  
  void NewEvent();
  void FillDigit(float adc,int row,int col);
  void SetTime(time_t& intime){fTime=intime;};
  time_t GetTime(){return fTime;};
  void FillTree(AliPHOSDATreeEvent* event=0);
  void FillHist(AliPHOSDATreeEvent* event=0);
  void Print(char* opt="");

 private:
  bool CreateTree();
  bool CreateHist();
  bool fCreateTree;           //! Flag of tree initialization
  bool fCreateHist;           //! Flag of hist initialization
  int fMod;                   // Module ID [0-4] ([2-4] for 2009)
  int fIterId;                // Iteration step [0-*]
  //
  TFile* fTFile;              //! output file
  TTree* fTTree;              //! output TTree
  AliPHOSDATreeEvent* fEvent; //! Contents of TTree
  bool fEventClustered;       //! Flag for
  time_t fTime;                 // time
  TH1I* fH1Time;                // x:bin1=StartTime bin2=EndTime
  TH1F* fH1DigitNum;            // x:Number of digits
  TH1F* fH1ClusterNum;          // x:Number of clusters
  TH2F* fH2EneDigitId;          // x:DigitId[0-3583] y:Digit Energy
  TH2F* fH2MipDigitId;          // x:DigitId[0-3583] y:Cluster Energy
  TH2F* fH2Pi0DigitId;          // x:DigitId[0-3583] y:Cluster Pair Mass
  TH3F* fH3Pi0AsymPt;           // x:asym y:pT(GeV/c) z:Cluster Pair Mass

  ClassDef(AliPHOSDApi0mip,1)
};
#endif
