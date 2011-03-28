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

///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Produces the data needed to calculate the ZDC quality assurance. //
//  QA objects are 1 & 2 Dimensional histograms.                     //
//  author: C. Oppedisano                                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////

// --- ROOT system ---
#include <TClonesArray.h>
#include <TFile.h> 
#include <TH1F.h> 
#include <TH2F.h>
#include <TLine.h>
#include <TProfile.h>
#include <Riostream.h>
// --- Standard library ---

// --- AliRoot header files ---
#include "AliLog.h"
#include "AliQAChecker.h"
#include "AliZDCReco.h"
#include "AliRawReader.h"
#include "AliZDCQADataMakerRec.h"
#include "AliZDCPedestals.h"
#include "AliZDCRawStream.h"
#include "AliZDCDigit.h"
#include "AliESDZDC.h"
#include "AliESDEvent.h"

ClassImp(AliZDCQADataMakerRec)
           
//____________________________________________________________________________ 
  AliZDCQADataMakerRec::AliZDCQADataMakerRec() : 
  AliQADataMakerRec(AliQAv1::GetDetName(AliQAv1::kZDC), "ZDC Quality Assurance Data Maker"), 
  fPedCalibData(0x0)
{
  // ctor
}

//____________________________________________________________________________ 
AliZDCQADataMakerRec::AliZDCQADataMakerRec(const AliZDCQADataMakerRec& qadm) :
  AliQADataMakerRec(),      
  fPedCalibData(qadm.fPedCalibData)

{
  //copy ctor 
  SetName((const char*)qadm.GetName()); 
  SetTitle((const char*)qadm.GetTitle()); 
}

//__________________________________________________________________
AliZDCQADataMakerRec& AliZDCQADataMakerRec::operator = (const AliZDCQADataMakerRec& qadm )
{
  // Equal operator.
  this->~AliZDCQADataMakerRec();
  new(this) AliZDCQADataMakerRec(qadm);
  return *this;
}

//____________________________________________________________________________ 
AliZDCQADataMakerRec::~AliZDCQADataMakerRec()
{
  if(fPedCalibData && !(AliCDBManager::Instance()->GetCacheFlag())){
    delete fPedCalibData;
    fPedCalibData=0;
  } 
}

//____________________________________________________________________________ 
AliZDCPedestals* AliZDCQADataMakerRec::GetPedCalibData() const
{

  // Retrieving pedestal calibration object from OCDB
  AliCDBEntry  *entry = AliCDBManager::Instance()->Get("ZDC/Calib/Pedestals");
  if(!entry) AliWarning("No calibration data loaded!");  

  AliZDCPedestals *calibdata = (AliZDCPedestals*)  (entry->GetObject());
  if(!calibdata) AliFatal("Wrong calibration object in calibration  file!");

  return calibdata;

}

//____________________________________________________________________________ 
void AliZDCQADataMakerRec::InitDigits()
{
  // create Digits histograms in Digits subdir
  //
  const Bool_t expert   = kTRUE ; 
  const Bool_t image    = kTRUE ; 
  
  // ------------------- HIGH GAIN CHAIN ---------------------------
  TH1F * hDigZNCTot = new TH1F("hDigZNCTot", "Signal in ZNC;Amplitude [ADC counts];Counts", 100, 0., 6000.);
  TH1F * hDigZNATot = new TH1F("hDigZNATot", "Signal in ZNA;Amplitude [ADC counts];Counts", 100, 0., 6000.);
  TH1F * hDigZPCTot = new TH1F("hDigZPCTot", "Signal in ZPC;Amplitude [ADC counts];Counts", 100, 0., 6000.);
  TH1F * hDigZPATot = new TH1F("hDigZPATot", "Signal in ZPA;Amplitude [ADC counts];Counts", 100, 0., 6000.);
  Add2DigitsList(hDigZNCTot, 0, !expert, image);
  Add2DigitsList(hDigZNATot, 1, !expert, image);
  Add2DigitsList(hDigZPCTot, 2, !expert, image);
  Add2DigitsList(hDigZPATot, 3, !expert, image);
  //
  TH1F * hDigSumQZNC = new TH1F("hDigSumQZNC", "Signal in 4 ZNC PMQ;Amplitude [ADC counts];Counts",100, 0., 4000.);
  TH1F * hDigSumQZNA = new TH1F("hDigSumQZNA", "Signal in 4 ZNA PMQ;Amplitude [ADC counts];Counts",100, 0., 4000.);
  TH1F * hDigSumQZPC = new TH1F("hDigSumQZPC", "Signal in 4 ZPC PMQ;Amplitude [ADC counts];Counts",100, 0., 4000.);
  TH1F * hDigSumQZPA = new TH1F("hDigSumQZPA", "Signal in 4 ZPA PMQ;Amplitude [ADC counts];Counts",100, 0., 4000.);
  Add2DigitsList(hDigSumQZNC, 4, expert, !image);
  Add2DigitsList(hDigSumQZNA, 5, expert, !image);
  Add2DigitsList(hDigSumQZPC, 6, expert, !image);
  Add2DigitsList(hDigSumQZPA, 7, expert, !image);
  //
  TH1F * hDigPMCZNC = new TH1F("hDigPMCZNC", "Signal in ZNC PMC;Amplitude [ADC counts];Counts",100, 0., 4000.);
  TH1F * hDigPMCZNA = new TH1F("hDigPMCZNA", "Signal in ZNA PMC;Amplitude [ADC counts];Counts",100, 0., 4000.);
  TH1F * hDigPMCZPC = new TH1F("hDigPMCZPC", "Signal in ZPC PMC;Amplitude [ADC counts];Counts",100, 0., 4000.);
  TH1F * hDigPMCZPA = new TH1F("hDigPMCZPA", "Signal in ZPA PMC;Amplitude [ADC counts];Counts",100, 0., 4000.);
  Add2DigitsList(hDigPMCZNC, 8, expert, !image);
  Add2DigitsList(hDigPMCZNA, 9, expert, !image);
  Add2DigitsList(hDigPMCZPC, 10, expert, !image);
  Add2DigitsList(hDigPMCZPA, 11, expert, !image);
  // 
  // ------------------- LOW GAIN CHAIN ---------------------------
  TH1F * hDigZNCTotlg = new TH1F("hDigZNCTotlg", "Digit lg signal in ZNC", 100, 0., 6000.);
  TH1F * hDigZNATotlg = new TH1F("hDigZNATotlg", "Digit lg signal in ZNA", 100, 0., 6000.);
  TH1F * hDigZPCTotlg = new TH1F("hDigZPCTotlg", "Digit lg signal in ZPC", 100, 0., 6000.);
  TH1F * hDigZPATotlg = new TH1F("hDigZPATotlg", "Digit lg signal in ZPA", 100, 0., 6000.);
  Add2DigitsList(hDigZNCTotlg, 12, expert, !image);
  Add2DigitsList(hDigZNATotlg, 13, expert, !image);
  Add2DigitsList(hDigZPCTotlg, 14, expert, !image);
  Add2DigitsList(hDigZPATotlg, 15, expert, !image);
  //
  TH1F * hDigSumQZNClg = new TH1F("hDigSumQZNClg", "Signal in 4 ZNC PMQlg",100, 0., 4000.);
  TH1F * hDigSumQZNAlg = new TH1F("hDigSumQZNAlg", "Signal in 4 ZNA PMQlg",100, 0., 4000.);
  TH1F * hDigSumQZPClg = new TH1F("hDigSumQZPClg", "Signal in 4 ZPC PMQlg",100, 0., 4000.);
  TH1F * hDigSumQZPAlg = new TH1F("hDigSumQZPAlg", "Signal in 4 ZPA PMQlg",100, 0., 4000.);
  Add2DigitsList(hDigSumQZNClg, 16, expert, !image);
  Add2DigitsList(hDigSumQZNAlg, 17, expert, !image);
  Add2DigitsList(hDigSumQZPClg, 18, expert, !image);
  Add2DigitsList(hDigSumQZPAlg, 19, expert, !image);
  //
  TH1F * hDigPMCZNClg = new TH1F("hDigPMCZNClg", "Signal in ZNC PMClg",100, 0., 4000.);
  TH1F * hDigPMCZNAlg = new TH1F("hDigPMCZNAlg", "Signal in ZNA PMClg",100, 0., 4000.);
  TH1F * hDigPMCZPClg = new TH1F("hDigPMCZPClg", "Signal in ZPC PMClg",100, 0., 4000.);
  TH1F * hDigPMCZPAlg = new TH1F("hDigPMCZPAlg", "Signal in ZPA PMClg",100, 0., 4000.);
  Add2DigitsList(hDigPMCZNClg, 20, expert, !image);
  Add2DigitsList(hDigPMCZNAlg, 21, expert, !image);
  Add2DigitsList(hDigPMCZPClg, 22, expert, !image);
  Add2DigitsList(hDigPMCZPAlg, 23, expert, !image);

}

//____________________________________________________________________________
void AliZDCQADataMakerRec::InitRaws()
{
  // create Digits histograms in Digits subdir
  const Bool_t expert   = kTRUE ; 
  const Bool_t image    = kTRUE ; 

  TH1F * hZNCSpectrum = new TH1F("hZNCSpectrum","ZNC spectrum;Amplitude [ADC counts];Counts",100,0.,1200.);
  TH1F * hZPCSpectrum = new TH1F("hZPCSpectrum","ZPC spectrum;Amplitude [ADC counts];Counts",100,0.,1200.);
  TH1F * hZNASpectrum = new TH1F("hZNASpectrum","ZNA spectrum;Amplitude [ADC counts];Counts",100,0.,1200.);
  TH1F * hZPASpectrum = new TH1F("hZPASpectrum","ZPA spectrum;Amplitude [ADC counts];Counts",100,0.,1200.);
  TH1F * hZEM1Spectrum = new TH1F("hZEM1Spectrum","ZEM1 spectrum;Amplitude [ADC counts];Counts",100,8., 1208.);
  TH1F * hZEM2Spectrum = new TH1F("hZEM2Spectrum","ZEM2 spectrum;Amplitude [ADC counts];Counts",100,8., 1208.);
  Add2RawsList(hZNCSpectrum, 0, expert, !image);
  Add2RawsList(hZNASpectrum, 1, expert, !image);
  Add2RawsList(hZPCSpectrum, 2, expert, !image);
  Add2RawsList(hZPASpectrum, 3, expert, !image);
  Add2RawsList(hZEM1Spectrum, 4, expert, !image);
  Add2RawsList(hZEM2Spectrum, 5, expert, !image);
  //
  TH2F * hZNCpmCvsPMq = new TH2F("hZNCpmCvsPMq", "ZNC;PMC [ADC counts];Sum(PMQ) [ADC counts]",50,8.,1208.,50,8., 1208.);
  TH2F * hZPCpmCvsPMq = new TH2F("hZPCpmCvsPMq", "ZPC;PMC [ADC counts];Sum(PMQ) [ADC counts]",50,8.,1208.,50,8., 1208.);
  TH2F * hZNApmCvsPMq = new TH2F("hZNApmCvsPMq", "ZNA;PMC [ADC counts];Sum(PMQ) [ADC counts]",50,8.,1208.,50,8., 1208.);
  TH2F * hZPApmCvsPMq = new TH2F("hZPApmCvsPMq", "ZPA;PMC [ADC counts];Sum(PMQ) [ADC counts]",50,8.,1208.,50,8., 1208.);
  Add2RawsList(hZNCpmCvsPMq, 6, expert, !image);
  Add2RawsList(hZNApmCvsPMq, 7, expert, !image);
  Add2RawsList(hZPCpmCvsPMq, 8, expert, !image);
  Add2RawsList(hZPApmCvsPMq, 9, expert, !image);
    
  TH1F * hRawPMCZNC = new TH1F("hRawPMCZNC", "Raw ZNC PMC;Amplitude [ADC counts];Counts",100, 8., 1208.);
  TH1F * hRawPMCZNA = new TH1F("hRawPMCZNA", "Raw ZNA PMC;Amplitude [ADC counts];Counts",100, 8., 1208.);
  TH1F * hRawPMCZPC = new TH1F("hRawPMCZPC", "Raw ZPC PMC;Amplitude [ADC counts];Counts",100, 8., 1208.);
  TH1F * hRawPMCZPA = new TH1F("hRawPMCZPA", "Raw ZPA PMC;Amplitude [ADC counts];Counts",100, 8., 1208.);
  Add2RawsList(hRawPMCZNC, 10, expert, !image);
  Add2RawsList(hRawPMCZNA, 11, expert, !image);
  Add2RawsList(hRawPMCZPC, 12, expert, !image);
  Add2RawsList(hRawPMCZPA, 13, expert, !image);
  TH1F * hRawSumQZNC = new TH1F("hRawSumQZNC", "Raw sumQ ZNC;Amplitude [ADC counts];Counts",100, 0., 1200.);
  TH1F * hRawSumQZNA = new TH1F("hRawSumQZNA", "Raw sumQ ZNA;Amplitude [ADC counts];Counts",100, 0., 1200.);
  TH1F * hRawSumQZPC = new TH1F("hRawSumQZPC", "Raw sumQ ZPC;Amplitude [ADC counts];Counts",100, 0., 1200.);
  TH1F * hRawSumQZPA = new TH1F("hRawSumQZPA", "Raw sumQ ZPA;Amplitude [ADC counts];Counts",100, 0., 1200.);
  Add2RawsList(hRawSumQZNC, 14, expert, !image);
  Add2RawsList(hRawSumQZNA, 15, expert, !image);
  Add2RawsList(hRawSumQZPC, 16, expert, !image);
  Add2RawsList(hRawSumQZPA, 17, expert, !image);
  
  TH1F * hRawTDCZEM1 = new TH1F("hRawTDCZEM1", "Raw TDC ZEM1;TDC [ns]",160, -350., -310.);
  Add2RawsList(hRawTDCZEM1, 18, expert, !image);
  TH1F * hRawTDCZPC = new TH1F("hRawTDCZPC", "Raw TDC ZPC;TDC [ns]",160, -350., -310.);
  Add2RawsList(hRawTDCZPC, 19, expert, !image);
  
  TProfile * hRawADCProfs = new TProfile("hRawADCProfs", "ADC profiles;ADC id;Mean ADC values",22,-0.5,21.5,10.,1210.,"");
  Add2RawsList(hRawADCProfs, 20, expert, !image);
  TProfile * hRawTDCProfs = new TProfile("hRawTDCProfs", "TDC profiles;TDC id;Mean TDC values",6,0.5,6.5,-340.,-300.,"S");
  Add2RawsList(hRawTDCProfs, 21, expert, !image);
  
  TH1F * hRawADCs = new TH1F("hRawADCs", "ADCs;ADC id;Mean ADC values",22,-0.5,21.5);
  Add2RawsList(hRawADCs, 22, !expert, image);
 
  TH1F * hRawTDCs = new TH1F("hRawTDCs", "TDCs;TDC id;Mean TDC values",6,0.5,6.5);
  hRawTDCs->SetMaximum(-300); hRawTDCs->SetMinimum(-340);
  Add2RawsList(hRawTDCs, 23, !expert, image);
  
  TH2F *hZNCrawCentr  = new TH2F("hZNCrawCentr", "Centroid in ZNC;X (cm);Y(cm)", 100, -5.,5.,100,-5.,5.);
  Add2RawsList(hZNCrawCentr, 24, expert, !image);
  TH2F *hZNArawCentr  = new TH2F("hZNArawCentr", "Centroid in ZNA;X (cm);Y(cm)", 100, -5.,5.,100,-5.,5.);
  Add2RawsList(hZNArawCentr, 25, expert, !image);
  
  TH2F *hTimeZDC = new TH2F("fHistTimeZDC", "ZDC;TDC timing A+C vs C-A; events", 120,-30,30,120,-100,-40);
  Add2RawsList(hTimeZDC, 26, expert, !image);
}

//____________________________________________________________________________
void AliZDCQADataMakerRec::InitRecPoints()
{
  // create Digits histograms in Digits subdir
  const Bool_t expert = kTRUE ; 
  const Bool_t image  = kTRUE ; 
  //
  // ------------------- HIGH GAIN CHAIN ---------------------------
  TH1F * hRecZNCTot = new TH1F("hRecZNCTot", "Rec signal in ZNC;Amplitude [ADC counts];Counts", 100, 0., 2000.);
  TH1F * hRecZNATot = new TH1F("hRecZNATot", "Rec signal in ZNA;Amplitude [ADC counts];Counts", 100, 0., 2000.);
  TH1F * hRecZPCTot = new TH1F("hRecZPCTot", "Rec signal in ZPC;Amplitude [ADC counts];Counts", 100, 0., 8000.);
  TH1F * hRecZPATot = new TH1F("hRecZPATot", "Rec signal in ZPA;Amplitude [ADC counts];Counts", 100, 0., 8000.);
  Add2RecPointsList(hRecZNCTot, 0, !expert, image);
  Add2RecPointsList(hRecZNATot, 1, !expert, image);
  Add2RecPointsList(hRecZPCTot, 2, !expert, image);
  Add2RecPointsList(hRecZPATot, 3, !expert, image);
  //
  TH1F * hRecSumQZNC = new TH1F("hRecSumQZNC", "Rec summed 4 ZNC quadrants;Amplitude [ADC counts];Counts",100, 0., 2000.);
  TH1F * hRecSumQZNA = new TH1F("hRecSumQZNA", "Rec summed 4 ZNA quadrants;Amplitude [ADC counts];Counts",100, 0., 2000.);
  TH1F * hRecSumQZPC = new TH1F("hRecSumQZPC", "Rec summed 4 ZPC quadrants;Amplitude [ADC counts];Counts",100, 0., 2000.);
  TH1F * hRecSumQZPA = new TH1F("hRecSumQZPA", "Rec summed 4 ZPA quadrants;Amplitude [ADC counts];Counts",100, 0., 2000.);
  Add2RecPointsList(hRecSumQZNC, 4, expert, !image);
  Add2RecPointsList(hRecSumQZNA, 5, expert, !image);
  Add2RecPointsList(hRecSumQZPC, 6, expert, !image);
  Add2RecPointsList(hRecSumQZPA, 7, expert, !image);
  //
  TH1F * hRecPMCZNC = new TH1F("hRecPMCZNC", "Rec common ZNC PMT;Amplitude [ADC counts];Counts",100, 0., 2000.);
  TH1F * hRecPMCZNA = new TH1F("hRecPMCZNA", "Rec common ZNA PMT;Amplitude [ADC counts];Counts",100, 0., 2000.);
  TH1F * hRecPMCZPC = new TH1F("hRecPMCZPC", "Rec common ZPC PMT;Amplitude [ADC counts];Counts",100, 0., 2000.);
  TH1F * hRecPMCZPA = new TH1F("hRecPMCZPA", "Rec common ZPA PMT;Amplitude [ADC counts];Counts",100, 0., 2000.);
  Add2RecPointsList(hRecPMCZNC, 8 , expert, !image);
  Add2RecPointsList(hRecPMCZNA, 9 , expert, !image);
  Add2RecPointsList(hRecPMCZPC, 10, expert, !image);
  Add2RecPointsList(hRecPMCZPA, 11, expert, !image); 
}

//____________________________________________________________________________
void AliZDCQADataMakerRec::InitESDs()
{
  //Booking ESDs histograms
  //
  const Bool_t expert = kTRUE ; 
  const Bool_t image  = kTRUE ; 
  
  TH2F * hZNCcentr  = new TH2F("hZNCcentr", "Centroid in ZNC;X (cm);Y(cm)", 100, -5.,5.,100,-5.,5.);
  TH2F * hZNAcentr  = new TH2F("hZNAcentr", "Centroid in ZNA;X (cm);Y(cm)", 100, -5.,5.,100,-5.,5.);
  Add2ESDsList(hZNCcentr, 0, !expert, image);
  Add2ESDsList(hZNAcentr, 1, !expert, image);
  //
  // ------------------- HIGH GAIN CHAIN ---------------------------
  TH1F * hESDZNCTot = new TH1F("hESDZNCTot", "Energy in ZNC", 100, 0., 4000.);
  TH1F * hESDZNATot = new TH1F("hESDZNATot", "Energy in ZNA", 100, 0., 4000.);
  TH1F * hESDZPCTot = new TH1F("hESDZPCTot", "Energy in ZPC", 100, 0., 4000.);
  TH1F * hESDZPATot = new TH1F("hESDZPATot", "Energy in ZPA", 100, 0., 4000.);
  Add2ESDsList(hESDZNCTot, 2, !expert, image);
  Add2ESDsList(hESDZNATot, 3, !expert, image);
  Add2ESDsList(hESDZPCTot, 4, !expert, image);
  Add2ESDsList(hESDZPATot, 5, !expert, image);
  //
  TH1F * hESDZEM1 = new TH1F("hESDZEM1", "Energy in ZEM1", 100, 0., 2000.);
  TH1F * hESDZEM2 = new TH1F("hESDZEM2", "Energy in ZEM2", 100, 0., 2000.);
  Add2ESDsList(hESDZEM1,6, !expert, image);
  Add2ESDsList(hESDZEM2,7, !expert, image);
  //
  TH1F * hESDSumQZNC = new TH1F("hESDSumQZNC", "Sum of 4 ZNC energy",100, 0., 2000.);
  TH1F * hESDSumQZNA = new TH1F("hESDSumQZNA", "Sum of 4 ZNA energy",100, 0., 2000.);
  TH1F * hESDSumQZPC = new TH1F("hESDSumQZPC", "Sum of 4 ZPC energy",100, 0., 2000.);
  TH1F * hESDSumQZPA = new TH1F("hESDSumQZPA", "Sum of 4 ZPA energy",100, 0., 2000.);
  Add2ESDsList(hESDSumQZNC, 8, expert, !image);
  Add2ESDsList(hESDSumQZNA, 9, expert, !image);
  Add2ESDsList(hESDSumQZPC, 10, expert, !image);
  Add2ESDsList(hESDSumQZPA, 11, expert, !image);
  //
  TH1F * hESDPMCZNC = new TH1F("hESDPMCZNC", "Energy in ZNC PMC",100, 0., 2000.);
  TH1F * hESDPMCZNA = new TH1F("hESDPMCZNA", "Energy in ZNA PMC",100, 0., 2000.);
  TH1F * hESDPMCZPC = new TH1F("hESDPMCZPC", "Energy in ZPC PMC",100, 0., 2000.);
  TH1F * hESDPMCZPA = new TH1F("hESDPMCZPA", "Energy in ZPA PMC",100, 0., 2000.);
  Add2ESDsList(hESDPMCZNC, 12, expert, !image);
  Add2ESDsList(hESDPMCZNA, 13, expert, !image);
  Add2ESDsList(hESDPMCZPC, 14, expert, !image);
  Add2ESDsList(hESDPMCZPA, 15, expert, !image);
  // 
  // ------------------- LOW GAIN CHAIN ---------------------------
  TH1F * hESDSumQZNClg = new TH1F("hESDSumQZNClg", "Sum of 4 lg ZNC sectors",100, 0., 4000.);
  TH1F * hESDSumQZNAlg = new TH1F("hESDSumQZNAlg", "Sum of 4 lg ZNA sectors",100, 0., 4000.);
  TH1F * hESDSumQZPClg = new TH1F("hESDSumQZPClg", "Sum of 4 lg ZPC sectors",100, 0., 4000.);
  TH1F * hESDSumQZPAlg = new TH1F("hESDSumQZPAlg", "Sum of 4 lg ZPA sectors",100, 0., 4000.);
  Add2ESDsList(hESDSumQZNClg, 16, expert, !image);
  Add2ESDsList(hESDSumQZNAlg, 17, expert, !image);
  Add2ESDsList(hESDSumQZPClg, 18, expert, !image);
  Add2ESDsList(hESDSumQZPAlg, 19, expert, !image);
  //
  TH1F * hESDPMCZNClg = new TH1F("hESDPMCZNClg", "Signal in common ZNC lg PMT",100, 0., 4000.);
  TH1F * hESDPMCZNAlg = new TH1F("hESDPMCZNAlg", "Signal in common ZNA lg PMT",100, 0., 4000.);
  TH1F * hESDPMCZPClg = new TH1F("hESDPMCZPClg", "Signal in common ZPC lg PMT",100, 0., 4000.);
  TH1F * hESDPMCZPAlg = new TH1F("hESDPMCZPAlg", "Signal in common ZPA lg PMT",100, 0., 4000.);
  Add2ESDsList(hESDPMCZNClg, 20, expert, !image);
  Add2ESDsList(hESDPMCZNAlg, 21, expert, !image);
  Add2ESDsList(hESDPMCZPClg, 22, expert, !image);
  Add2ESDsList(hESDPMCZPAlg, 23, expert, !image);
}

//___________________________________________________________________________
void AliZDCQADataMakerRec::MakeDigits(TTree *digitTree)
{
  // makes data from Digit Tree
  if(!GetDigitsData(0)) InitDigits();

  if(!digitTree){
    AliError("Can't get ZDC digit tree!!");
    return; 
  }	
   
  TBranch * branch = digitTree->GetBranch("ZDC");
  if(!branch){
    AliError("ZDC branch in digit tree not found"); 
    return;
  } 
    
  AliZDCDigit *digit = 0x0;
  branch->SetAddress(&digit);
     
  Float_t adcSum_ZNC=0., adcSum_ZNA=0., adcSum_ZPC=0., adcSum_ZPA=0.;
  Float_t adcSumQ_ZNC=0., adcSumQ_ZNA=0., adcSumQ_ZPC=0., adcSumQ_ZPA=0.;
  Float_t adcSum_ZNC_lg=0., adcSum_ZNA_lg=0., adcSum_ZPC_lg=0., adcSum_ZPA_lg=0.;
  Float_t adcSumQ_ZNC_lg=0., adcSumQ_ZNA_lg=0., adcSumQ_ZPC_lg=0., adcSumQ_ZPA_lg=0.;
  
  Int_t ndig = digitTree->GetEntries();
  for(Int_t i=0; i<ndig; i++){
      branch->GetEntry(i);
      
      if(digit->GetSector(0)==1 && digit->GetSector(1)!=5){
	  adcSum_ZNC += digit->GetADCValue(0);
	  adcSum_ZNC_lg += digit->GetADCValue(1);
	  //
	  if(digit->GetSector(1)!=0){
	      adcSumQ_ZNC += digit->GetADCValue(0);
	      adcSumQ_ZNC_lg+= digit->GetADCValue(1);
	  }
	  else{
	      GetDigitsData(8)->Fill(digit->GetADCValue(0));
	      GetDigitsData(20)->Fill(digit->GetADCValue(1));
	  }
      }
      else if(digit->GetSector(0)==2){
	  adcSum_ZPC += digit->GetADCValue(0);
	  adcSum_ZPC_lg += digit->GetADCValue(1);
	  //
	  if(digit->GetSector(1)!=0){
	      adcSumQ_ZPC += digit->GetADCValue(0);
	      adcSumQ_ZPC_lg+= digit->GetADCValue(1);
	  }
	  else{
	      GetDigitsData(10)->Fill(digit->GetADCValue(0));
	      GetDigitsData(22)->Fill(digit->GetADCValue(1));
	  }
      }
      else if(digit->GetSector(0)==4 && digit->GetSector(1)!=5){
	  adcSum_ZNA += digit->GetADCValue(0);
	  adcSum_ZNA_lg += digit->GetADCValue(1);
	  //
	  if(digit->GetSector(1)!=0){
	      adcSumQ_ZNA += digit->GetADCValue(0);
	      adcSumQ_ZNA_lg+= digit->GetADCValue(1);
	  }
	  else{
	      GetDigitsData(9)->Fill(digit->GetADCValue(0));
	      GetDigitsData(21)->Fill(digit->GetADCValue(1));
	  }
      }
      else if(digit->GetSector(0)==5){
	  adcSum_ZPA += digit->GetADCValue(0);
	  adcSum_ZPA_lg += digit->GetADCValue(1);
	  //
	  if(digit->GetSector(1)!=0){
	      adcSumQ_ZPA += digit->GetADCValue(0);
	      adcSumQ_ZPA_lg+= digit->GetADCValue(1);
	  }
	  else{
	      GetDigitsData(11)->Fill(digit->GetADCValue(0));
	      GetDigitsData(23)->Fill(digit->GetADCValue(1));
	  }
      }
  }
  //
  GetDigitsData(0)->Fill(adcSum_ZNC);
  GetDigitsData(1)->Fill(adcSum_ZNA);
  GetDigitsData(2)->Fill(adcSum_ZPC);
  GetDigitsData(3)->Fill(adcSum_ZPA);
  //
  GetDigitsData(4)->Fill(adcSumQ_ZNC);
  GetDigitsData(5)->Fill(adcSumQ_ZNA);
  GetDigitsData(6)->Fill(adcSumQ_ZPC);
  GetDigitsData(7)->Fill(adcSumQ_ZPA);
  //
  GetDigitsData(12)->Fill(adcSum_ZNC_lg);
  GetDigitsData(13)->Fill(adcSum_ZNA_lg);
  GetDigitsData(14)->Fill(adcSum_ZPC_lg);
  GetDigitsData(15)->Fill(adcSum_ZPA_lg);
  //
  GetDigitsData(16)->Fill(adcSumQ_ZNC_lg);
  GetDigitsData(17)->Fill(adcSumQ_ZNA_lg);
  GetDigitsData(18)->Fill(adcSumQ_ZPC_lg);
  GetDigitsData(19)->Fill(adcSumQ_ZPA_lg);
  
  delete digit;
  digit=0;

}


//____________________________________________________________________________
void AliZDCQADataMakerRec::MakeRaws(AliRawReader *rawReader)
{
  // Filling Raws QA histos
  //
  // Checking the event type 
//  if (rawReader->GetType()!=7){
  
    // Check if histograms already created for this Event Specie
    if(!GetRawsData(0)) InitRaws();
  
    // Parameters for mean value pedestal subtraction
    int const kNch = 24;
    Float_t meanPed[2*kNch];    
    for(Int_t jj=0; jj<2*kNch; jj++) meanPed[jj] = fPedCalibData->GetMeanPed(jj);
    
    Float_t zncSignal=0., znaSignal=0., zpcSignal=0., zpaSignal=0.;
    Float_t zncSumQ=0., znaSumQ=0., zpcSumQ=0., zpaSumQ=0.;
    Float_t zncpmC=0., znapmC=0., zpcpmC=0., zpapmC=0.;
    Bool_t isZNCFired=kFALSE, isZPCFired=kFALSE, isZNAFired=kFALSE, isZPAFired=kFALSE;
    Int_t  indZNC=0, indZNA=0, indZPC=0, indZPA=0;
    Float_t zncTDC[10], zpcTDC[10], zem1TDC[10], zem2TDC[10], znaTDC[10], zpaTDC[10];
    Float_t zncSumTDC[10], znaSumTDC[10];
    for(Int_t i=0; i<10; i++){
       zncTDC[i]=zpcTDC[i]=zem1TDC[i]=zem2TDC[i]=znaTDC[i]=zpaTDC[i]=zncSumTDC[i]=znaSumTDC[i]=-999.;
    }
    Float_t tdcGate=-999., l0=-999.;
    Int_t iMultZNCTDC=0, iMultZPCTDC=0, iMultZEM1TDC=0, iMultZEM2TDC=0, iMultZNATDC=0, iMultZPATDC=0;
    Int_t iMultTDCC=0, iMultTDCA=0;
    
    const Float_t x[4] = {-1.75, 1.75, -1.75, 1.75};
    const Float_t y[4] = {-1.75, -1.75, 1.75, 1.75};
    const Float_t alpha=0.5;
    Float_t numXZNC=0., numYZNC=0., denZNC=0., wZNC=0.; 
    Float_t numXZNA=0., numYZNA=0., denZNA=0., wZNA=0.; 
    
    rawReader->Reset();
    AliZDCRawStream stream(rawReader);
    while(stream.Next()){

      if(stream.IsADCDataWord() && 
    	 (stream.GetADCModule()==0 || stream.GetADCModule()==1)){
       
    	 Int_t det = stream.GetSector(0);
    	 Int_t quad = stream.GetSector(1);
    	 Int_t gain = stream.GetADCGain();
    	 Int_t pedindex=0;
    	 
    	 // Stuff for pedestal subtraction
    	 if(quad != 5){ // ZDCs (not reference PTMs)
	  Float_t pedSubVal=-99.;
    	  if(det == 1){    
    	    pedindex = quad;
    	    if(gain == 0){
    	      pedSubVal = (Float_t) (stream.GetADCValue()-meanPed[pedindex]); 
    	      zncSignal  += pedSubVal; 
    	      isZNCFired = kTRUE;
    	      if(quad!=0){
	        zncSumQ += pedSubVal;
	        if(pedSubVal>0.&& zncpmC>50.){
	          wZNC = TMath::Power(pedSubVal, alpha);
	          numXZNC += x[quad-1]*wZNC;
	          numYZNC += y[quad-1]*wZNC;
	          denZNC += wZNC;
	        }
	      }
    	      else{
  		zncpmC = pedSubVal;
  		GetRawsData(10)->Fill(zncpmC);
  	      }
	      indZNC++;
	      
	      GetRawsData(20)->Fill(pedindex, pedSubVal);
  	    }
  	  }
  	  else if(det == 2){ 
  	    pedindex = quad+5;
  	    if(gain == 0){
  	      pedSubVal = (Float_t) (stream.GetADCValue()-meanPed[pedindex]); 
  	      zpcSignal += pedSubVal; 
  	      isZPCFired = kTRUE;
  	      if(quad!=0) zpcSumQ += pedSubVal;
  	      else{
  		zpcpmC = pedSubVal;
  		GetRawsData(12)->Fill(zpcpmC);
  	      }
	      indZPC++;
	      
	      GetRawsData(20)->Fill(pedindex, pedSubVal);
  	    }
  	  }
  	  else if(det == 3){ 
  	    pedindex = quad+9;
  	    if(quad==1){     
  	      if(gain == 0){
  		pedSubVal = (Float_t) (stream.GetADCValue()-meanPed[pedindex]); 
  		GetRawsData(4)->Fill(pedSubVal);
	      
	        GetRawsData(20)->Fill(pedindex, pedSubVal);
  	      }
  	    }
  	    else if(quad==2){ 
  	      if(gain == 0){
  		pedSubVal = (Float_t) (stream.GetADCValue()-meanPed[pedindex]); 
  		GetRawsData(5)->Fill(pedSubVal); 
	      
	        GetRawsData(20)->Fill(pedindex, pedSubVal);
  	      }
  	    }
  	  }
  	  else if(det == 4){	   
  	    pedindex = quad+12;
  	    if(gain == 0){
  	      pedSubVal = (Float_t) (stream.GetADCValue()-meanPed[pedindex]); 
  	      znaSignal  += pedSubVal; 
  	      isZNAFired = kTRUE;
  	      if(quad!=0){
	        znaSumQ += pedSubVal;
	        if(pedSubVal>0.&& znapmC>50.) {
	          wZNA = TMath::Power(pedSubVal, alpha);
	          numXZNA += x[quad-1]*wZNA;
	          numYZNA += y[quad-1]*wZNA;
	          denZNA += wZNA;
	        }
	      }
  	      else{
  		znapmC = pedSubVal;
  		GetRawsData(11)->Fill(znapmC);
  	      }
	      indZNA++;
	      
	      GetRawsData(20)->Fill(pedindex, pedSubVal);
	      //GetRawsData(22)->SetBinContent(pedindex+1, GetRawsData(20)->GetBinContent(pedindex+1));
	      //GetRawsData(22)->SetBinError(pedindex+1, GetRawsData(20)->GetBinError(pedindex+1));
  	    }
  	  }
  	  else if(det == 5){
  	    pedindex = quad+17;
  	    if(gain == 0){
  	      pedSubVal = (Float_t) (stream.GetADCValue()-meanPed[pedindex]); 
  	      zpaSignal  += pedSubVal; 
  	      isZPAFired = kTRUE;
  	      if(quad!=0) zpaSumQ += pedSubVal;
  	      else{
  		zpapmC = pedSubVal;
  		GetRawsData(13)->Fill(zpapmC);
  	      }
	      indZPA++;
	      
	      GetRawsData(20)->Fill(pedindex, pedSubVal);
	      //GetRawsData(22)->SetBinContent(pedindex+1, GetRawsData(20)->GetBinContent(pedindex+1));
	      //GetRawsData(22)->SetBinError(pedindex+1, GetRawsData(20)->GetBinError(pedindex+1));
  	    }
  	  }
                	 
	 }

  	 if(isZNCFired && indZNC==5){
  	   GetRawsData(0)->Fill(zncSignal);
  	   GetRawsData(6)->Fill(zncpmC, zncSumQ);
  	   GetRawsData(14)->Fill(zncSumQ); 
           //
	   Float_t xZNC, yZNC; 	         
	   if(denZNC!=0){
	     xZNC = numXZNC/denZNC;
	     yZNC = numYZNC/denZNC;
	   } 
	   else xZNC = yZNC = 999.;
	   GetRawsData(24)->Fill(xZNC, yZNC);
  	 }
  	 if(isZPCFired && indZPC==5){
  	   GetRawsData(2)->Fill(zpcSignal);
           GetRawsData(8)->Fill(zpcpmC, zpcSumQ);
           GetRawsData(16)->Fill(zpcSumQ); 
         }
         if(isZNAFired && indZNA==5){ 
           GetRawsData(1)->Fill(znaSignal);
           GetRawsData(7)->Fill(znapmC, znaSumQ);
           GetRawsData(15)->Fill(znaSumQ); 
	   //
	   Float_t xZNA, yZNA;
	   if(denZNA!=0){
	     xZNA = numXZNA/denZNA;
	     yZNA = numYZNA/denZNA;
	   } 
	   else xZNA = yZNA = 999.;
	   GetRawsData(25)->Fill(xZNA, yZNA);
         }
         if(isZPAFired && indZPA==5){ 
           GetRawsData(3)->Fill(zpaSignal);
           GetRawsData(9)->Fill(zpapmC, zpaSumQ);
           GetRawsData(17)->Fill(zpaSumQ); 
         }
	 
	 if(indZNC==5){
	   zncSignal = zncSumQ = zncpmC = 0;
	   isZNCFired=kFALSE; indZNC=0;
	 }
	 if(indZPC==5){
	   zpcSignal = zpcSumQ = zpcpmC = 0;
	   isZPCFired=kFALSE; indZPC=0;
	 }
	 if(indZNA==5){
	   znaSignal = znaSumQ = znapmC = 0;
	   isZNAFired=kFALSE; indZNA=0;
	 }
	 if(indZPA==5){
	   zpaSignal = zpaSumQ = zpapmC = 0;
	   isZPAFired=kFALSE; indZPA=0;
	 }
	 
      } //IsADCDataWord && signal ADCs
      else if(stream.IsZDCTDCDatum()){
	 if(stream.GetChannel()==1){
	    zncTDC[iMultZNCTDC] = (0.025*stream.GetZDCTDCDatum());
	    iMultZNCTDC++;
	 }
	 else if(stream.GetChannel()==3){
	    zpcTDC[iMultZPCTDC] = (0.025*stream.GetZDCTDCDatum());
	    iMultZPCTDC++;
	 }
	 else if(stream.GetChannel()==5){
	    znaTDC[iMultZNATDC] = (0.025*stream.GetZDCTDCDatum());
	    iMultZNATDC++;
	 }
	 else if(stream.GetChannel()==7){
	    zpaTDC[iMultZPATDC] = (0.025*stream.GetZDCTDCDatum());
	    iMultZPATDC++;
	 }
	 else if(stream.GetChannel()==8){
	    zem1TDC[iMultZEM1TDC] = (0.025*stream.GetZDCTDCDatum());
	    iMultZEM1TDC++;
	 }
	 else if(stream.GetChannel()==9){
	    zem2TDC[iMultZEM2TDC] = (0.025*stream.GetZDCTDCDatum());
	    iMultZEM2TDC++;
	 }
	 else if(stream.GetChannel()==10){
	    zncSumTDC[iMultZEM2TDC] = (0.025*stream.GetZDCTDCDatum());
	    iMultTDCC++;
	 }
	 else if(stream.GetChannel()==12){
	    znaSumTDC[iMultZEM2TDC] = (0.025*stream.GetZDCTDCDatum());
	    iMultTDCA++;
	 }
	 else if(stream.GetChannel()==14) tdcGate = (0.025*stream.GetZDCTDCDatum());
	 else if(stream.GetChannel()==15) l0 = (0.025*stream.GetZDCTDCDatum());
	 
	 if(stream.GetChannel()==16 && tdcGate!=-999.){
	   for(Int_t iHit=0; iHit<10; iHit++){
	      if(zncTDC[iHit]!=-999.){
	        if(zncTDC[iHit]-tdcGate>-340. && zncTDC[iHit]-tdcGate<-300.) 
		   GetRawsData(21)->Fill(1, zncTDC[iHit]-tdcGate);
	        //GetRawsData(23)->SetBinContent(1, GetRawsData(21)->GetBinContent(1));
	      }
	      if(zpcTDC[iHit]!=-999.){
	        Float_t diffZPC = zpcTDC[iHit]-tdcGate;
	        GetRawsData(19)->Fill(diffZPC);
	        if(diffZPC>-340. && diffZPC<-300.) GetRawsData(21)->Fill(2, diffZPC);
		//GetRawsData(23)->SetBinContent(2,  GetRawsData(21)->GetBinContent(3));
	      }
	      if(znaTDC[iHit]!=-999.){
	        if(znaTDC[iHit]-tdcGate>-340. && znaTDC[iHit]-tdcGate<-300.) 
	          GetRawsData(21)->Fill(3, znaTDC[iHit]-tdcGate);
	        //GetRawsData(23)->SetBinContent(3,  GetRawsData(21)->GetBinContent(5));
	      }
	      if(zpaTDC[iHit]!=-999.){
	        if(zpaTDC[iHit]-tdcGate>-340. && zpaTDC[iHit]-tdcGate<-300.) 
	          GetRawsData(21)->Fill(4, zpaTDC[iHit]-tdcGate);
	        //GetRawsData(23)->SetBinContent(4,  GetRawsData(21)->GetBinContent(7));
	      }
	      if(zem1TDC[iHit]!=-999.){
	        Float_t diffZEM1 = zem1TDC[iHit]-tdcGate;
	        GetRawsData(18)->Fill(diffZEM1);
		if(diffZEM1>-340. && diffZEM1<-300.) GetRawsData(21)->Fill(5, diffZEM1);
		//GetRawsData(23)->SetBinContent(5,  GetRawsData(21)->GetBinContent(8));
	      }
	      if(zem2TDC[iHit]!=-999.){
	        if(zem2TDC[iHit]-tdcGate>-340. && zem2TDC[iHit]-tdcGate<-300.) 
	          GetRawsData(21)->Fill(6, zem2TDC[iHit]-tdcGate);
	        //GetRawsData(23)->SetBinContent(6,  GetRawsData(21)->GetBinContent(9));
              }
	      if(zncSumTDC[iHit]!=-999.){
	         Float_t tdcC = zncSumTDC[iHit]-l0;
		 if(znaSumTDC[iHit]!=-999.){
		    Float_t tdcA = znaSumTDC[iHit]-l0;
	            //if (((tdcC-tdcA-refDelta)*(tdcC-tdcA-refDelta)/(sigmaDelta*sigmaDelta) +
	                //(tdcC+tdcA-refSum)*(tdcC+tdcA-refSum)/(sigmaSum*sigmaSum))< 1.0)
			GetRawsData(26)->Fill(tdcC-tdcA,tdcC+tdcA);
		    
		 }
              }
	   }
	   //
	   tdcGate = -999.;
           for(Int_t i=0; i<10; i++){
              zncTDC[i] = zpcTDC[i] = zem1TDC[i] = zem2TDC[i] = znaTDC[i] = zpaTDC[i] = -999.;
	      zncSumTDC[i] = znaSumTDC[i] = -999.;
           } 
	 }
      }
    
    } //stream.Next()
//  } // check on event type
//  else{
//    AliDebug(1,Form("Skipping non-physics event for QA -> event type %d \n", rawReader->GetType())); 
//  }
}

//____________________________________________________________________________
void AliZDCQADataMakerRec::MakeRecPoints(TTree * clustersTree)
{
  // Filling QA histos from RecPoints

  TBranch *branch = clustersTree->GetBranch("ZDC");
  if(!branch){ 
    AliError("Can't get the ZDC branch for rec points!");
    return;
  }
  
  if(!GetRecPointsData(0)) InitRecPoints() ;

  Float_t sum_ZNC=0., sum_ZNA=0., sum_ZPC=0., sum_ZPA=0.;
  Float_t sumQ_ZNC=0., sumQ_ZNA=0., sumQ_ZPC=0., sumQ_ZPA=0.;

  AliZDCReco reco;
  AliZDCReco* preco = &reco;
  clustersTree->SetBranchAddress("ZDC", &preco);

  clustersTree->GetEntry(0);
  for(Int_t i=0; i<5; i++){
    sum_ZNC += reco.GetZN1HREnTow(i);
    sum_ZPC += reco.GetZN2HREnTow(i);
    sum_ZNA += reco.GetZP1HREnTow(i);
    sum_ZPA += reco.GetZP2HREnTow(i);
    if(i==0){
      GetRecPointsData(8)->Fill(reco.GetZN1HREnTow(i));
      GetRecPointsData(9)->Fill(reco.GetZN2HREnTow(i));
      GetRecPointsData(10)->Fill(reco.GetZP1HREnTow(i));
      GetRecPointsData(11)->Fill(reco.GetZP2HREnTow(i));
    }
    else{
      sumQ_ZNC += reco.GetZN1HREnTow(i);
      sumQ_ZPC += reco.GetZN2HREnTow(i);
      sumQ_ZNA += reco.GetZP1HREnTow(i);
      sumQ_ZPA += reco.GetZP2HREnTow(i);
    }
  }
  
  GetRecPointsData(0)->Fill(sum_ZNC);
  GetRecPointsData(1)->Fill(sum_ZNA);
  GetRecPointsData(2)->Fill(sum_ZPC);
  GetRecPointsData(3)->Fill(sum_ZPA);
  //
  GetRecPointsData(4)->Fill(sumQ_ZNC);
  GetRecPointsData(5)->Fill(sumQ_ZNA);
  GetRecPointsData(6)->Fill(sumQ_ZPC);
  GetRecPointsData(7)->Fill(sumQ_ZPA);
  
}  

//____________________________________________________________________________
void AliZDCQADataMakerRec::MakeESDs(AliESDEvent * esd)
{
  // make QA data from ESDs
  //
  
  // Check id histograms already created for this Event Specie
  if(!GetESDsData(0)) InitESDs() ;

  AliESDZDC * zdcESD =  esd->GetESDZDC();
  //
  TString beamType = esd->GetBeamType();
  Double_t centr_ZNC[2]={999.,999}, centr_ZNA[2]={999.,999};
  if(((beamType.CompareTo("pp"))==0) || ((beamType.CompareTo("p-p"))==0)
     ||((beamType.CompareTo("PP"))==0) || ((beamType.CompareTo("P-P"))==0)){
    zdcESD->GetZNCentroidInpp(centr_ZNC, centr_ZNA);
  }
  else if((beamType.CompareTo("A-A")) == 0 || (beamType.CompareTo("Pb-Pb")) == 0){
    Float_t beamEne = esd->GetBeamEnergy();
    zdcESD->GetZNCentroidInPbPb(beamEne, centr_ZNC, centr_ZNA);
  }
  else printf("\n WARNING!!! AliZDCQADataMakerRec::MakeESDs: can't calculate centroids for beam type: %s\n\n",beamType.Data());
  GetESDsData(0)->Fill(centr_ZNC[0], centr_ZNC[1]);
  GetESDsData(1)->Fill(centr_ZNA[0], centr_ZNA[1]);

  //
  GetESDsData(2)->Fill(esd->GetZDCN1Energy());
  GetESDsData(3)->Fill(esd->GetZDCN2Energy());
  GetESDsData(4)->Fill(esd->GetZDCP1Energy());
  GetESDsData(5)->Fill(esd->GetZDCP2Energy());
  GetESDsData(6)->Fill(esd->GetZDCEMEnergy(0));
  GetESDsData(7)->Fill(esd->GetZDCEMEnergy(1));
  //
  Double_t sumQZNC=0., sumQZPC=0., sumQZNA=0., sumQZPA=0.;
  Double_t sumQZNC_lg=0., sumQZPC_lg=0., sumQZNA_lg=0., sumQZPA_lg=0.;
  //
  const Double_t *towZNC, *towZPC, *towZNA, *towZPA;
  const Double_t *towZNC_lg, *towZPC_lg, *towZNA_lg, *towZPA_lg;
  //
  towZNC = zdcESD->GetZN1TowerEnergy();
  towZPC = zdcESD->GetZP1TowerEnergy();
  towZNA = zdcESD->GetZN2TowerEnergy();
  towZPA = zdcESD->GetZP2TowerEnergy();
  //
  towZNC_lg = zdcESD->GetZN1TowerEnergyLR();
  towZPC_lg = zdcESD->GetZP1TowerEnergyLR();
  towZNA_lg = zdcESD->GetZN2TowerEnergyLR();
  towZPA_lg = zdcESD->GetZP2TowerEnergyLR();
  //
  for(Int_t i=0; i<5; i++){
     if(i==0){
       GetESDsData(12)->Fill(towZNC[i]);
       GetESDsData(13)->Fill(towZNA[i]);
       GetESDsData(14)->Fill(towZPC[i]);
       GetESDsData(15)->Fill(towZPA[i]);
       //
       GetESDsData(20)->Fill(towZNC_lg[i]);
       GetESDsData(21)->Fill(towZNA_lg[i]);
       GetESDsData(22)->Fill(towZPC_lg[i]);
       GetESDsData(23)->Fill(towZPA_lg[i]);
     }
     else{
       sumQZNC += towZNC[i];
       sumQZPC += towZPC[i];
       sumQZNA += towZNA[i];
       sumQZPA += towZPA[i];
       //
       sumQZNC_lg += towZNC_lg[i];
       sumQZPC_lg += towZPC_lg[i];
       sumQZNA_lg += towZNA_lg[i];
       sumQZPA_lg += towZPA_lg[i];
     }
  }
  GetESDsData(8)->Fill(sumQZNC);
  GetESDsData(9)->Fill(sumQZNA);
  GetESDsData(10)->Fill(sumQZPC);
  GetESDsData(11)->Fill(sumQZPA);
  //
  GetESDsData(16)->Fill(sumQZNC_lg);
  GetESDsData(17)->Fill(sumQZNA_lg);
  GetESDsData(18)->Fill(sumQZPC_lg);
  GetESDsData(19)->Fill(sumQZPA_lg);
}

//____________________________________________________________________________
void AliZDCQADataMakerRec::StartOfDetectorCycle()
{
  //Detector specific actions at start of cycle

  fPedCalibData = GetPedCalibData();
  
}

//____________________________________________________________________________ 
void AliZDCQADataMakerRec::EndOfDetectorCycle(AliQAv1::TASKINDEX_t task, TObjArray ** list)
{
  //Detector specific actions at end of cycle
  // do the QA checking
  if( task == AliQAv1::kRAWS){
     if (!GetRawsData(20) || !GetRawsData(21) || !GetRawsData(24)  || !GetRawsData(25)) {
	 printf("  WARNING!!! AliZDCQADataMaker Rec -> Not all histogram for DQM found!\n"); 
     }
     else{
       for(Int_t ibin=1; ibin<=GetRawsData(20)->GetNbinsX(); ibin++){
          GetRawsData(22)->SetBinContent(ibin, GetRawsData(20)->GetBinContent(ibin)); 
	  GetRawsData(22)->SetBinError(ibin, GetRawsData(20)->GetBinError(ibin));
          GetRawsData(22)->SetLineColor(kBlue); GetRawsData(22)->SetLineWidth(2);
       }
       for(Int_t ibin=1; ibin<=GetRawsData(21)->GetNbinsX(); ibin++){
          GetRawsData(23)->SetBinContent(ibin, GetRawsData(21)->GetBinContent(ibin)); 
	  GetRawsData(23)->SetBinError(ibin, GetRawsData(21)->GetBinError(ibin));
          GetRawsData(23)->SetLineColor(kAzure-3); GetRawsData(23)->SetLineWidth(2);
       }
     }
  }
  	
  AliQAChecker::Instance()->Run(AliQAv1::kZDC, task, list) ;  
}
