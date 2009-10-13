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
/************************************************************************
 *                                                                      *
 * PID Steering Class                                                   *
 * Interface to the user task                                           *
 *                                                                      *
 * Authors:                                                             *
 *   Markus Fasel <M.Fasel@gsi.de>                                      *
 *                                                                      *
 ************************************************************************/
#include <TClass.h>
#include <THnSparse.h>
#include <TH3F.h>
#include <TIterator.h>
#include <TList.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TString.h>

#include "AliAODTrack.h"
#include "AliESDtrack.h"
#include "AliLog.h"
#include "AliPID.h"

#include "AliHFEpid.h"
#include "AliHFEpidBase.h"
#include "AliHFEpidITS.h"
#include "AliHFEpidTPC.h"
#include "AliHFEpidTRD.h"
#include "AliHFEpidTOF.h"
#include "AliHFEpidMC.h"

ClassImp(AliHFEpid)

//____________________________________________________________
AliHFEpid::AliHFEpid():
  fEnabledDetectors(0),
  fQAlist(0x0),
  fDebugLevel(0)
{
  //
  // Default constructor
  //
  memset(fDetectorPID, 0, sizeof(AliHFEpidBase *) * kNdetectorPID);
}

//____________________________________________________________
AliHFEpid::AliHFEpid(const AliHFEpid &c):
  TObject(c),
  fEnabledDetectors(c.fEnabledDetectors),
  fQAlist(0x0),
  fDebugLevel(c.fDebugLevel)
{
  //
  // Copy Constructor
  //
  memset(fDetectorPID, 0, sizeof(AliHFEpidBase *) * kNdetectorPID);
  if(c.fDetectorPID[kMCpid])
    fDetectorPID[kMCpid] = new AliHFEpidMC(*(dynamic_cast<AliHFEpidMC *>(c.fDetectorPID[kMCpid])));
  if(c.fDetectorPID[kTPCpid])
    fDetectorPID[kTPCpid] = new AliHFEpidTPC(*(dynamic_cast<AliHFEpidTPC *>(c.fDetectorPID[kTPCpid])));
  if(c.fDetectorPID[kTRDpid])
    fDetectorPID[kTRDpid] = new AliHFEpidTRD(*(dynamic_cast<AliHFEpidTRD *>(c.fDetectorPID[kTOFpid])));
  if(c.fDetectorPID[kTOFpid])
    fDetectorPID[kTOFpid] = new AliHFEpidTOF(*(dynamic_cast<AliHFEpidTOF *>(c.fDetectorPID[kTOFpid])));
  if(c.IsQAOn()) SetQAOn();
  if(c.HasMCData()) SetHasMCData(kTRUE);
  for(Int_t idet = 0; idet < kNdetectorPID; idet++){
    if(c.IsQAOn() && fDetectorPID[idet]) fDetectorPID[idet]->SetQAOn(fQAlist);
    if(c.HasMCData() && fDetectorPID[idet]) fDetectorPID[idet]->SetHasMCData(kTRUE);
  }
}

//____________________________________________________________
AliHFEpid& AliHFEpid::operator=(const AliHFEpid &c){
  //
  // Assignment operator
  //
  TObject::operator=(c);

  if(this != &c){
    fEnabledDetectors = c.fEnabledDetectors;
    fQAlist = 0x0;
    fDebugLevel = c.fDebugLevel;
  
    memset(fDetectorPID, 0, sizeof(AliHFEpidBase *) * kNdetectorPID);
    if(c.fDetectorPID[kMCpid])
      fDetectorPID[kMCpid] = new AliHFEpidMC(*(dynamic_cast<AliHFEpidMC *>(c.fDetectorPID[kMCpid])));
    if(c.fDetectorPID[kTPCpid])
      fDetectorPID[kTPCpid] = new AliHFEpidTPC(*(dynamic_cast<AliHFEpidTPC *>(c.fDetectorPID[kTPCpid])));
    if(c.fDetectorPID[kTRDpid])
      fDetectorPID[kTRDpid] = new AliHFEpidTRD(*(dynamic_cast<AliHFEpidTRD *>(c.fDetectorPID[kTOFpid])));
    if(c.fDetectorPID[kTOFpid])
      fDetectorPID[kTOFpid] = new AliHFEpidTOF(*(dynamic_cast<AliHFEpidTOF *>(c.fDetectorPID[kTOFpid])));
    if(c.IsQAOn()) SetQAOn();
    if(c.HasMCData()) SetHasMCData(kTRUE);
    for(Int_t idet = 0; idet < kNdetectorPID; idet++){
      if(c.IsQAOn() && fDetectorPID[idet]) fDetectorPID[idet]->SetQAOn(fQAlist);
      if(c.HasMCData() && fDetectorPID[idet]) fDetectorPID[idet]->SetHasMCData();
    }
  }
  return *this; 
}

//____________________________________________________________
AliHFEpid::~AliHFEpid(){
  //
  // Destructor
  //
  for(Int_t idet = 0; idet < kNdetectorPID; idet++){
    if(fDetectorPID[idet])
      delete fDetectorPID[idet];
  } 
  if(fQAlist) delete fQAlist; fQAlist = 0x0;  // Each detector has to care about its Histograms
}

//____________________________________________________________
Bool_t AliHFEpid::InitializePID(TString detectors){
  //
  // Initializes PID Object:
  // + Defines which detectors to use
  // + Initializes Detector PID objects
  // + Handles QA
  //
  AliInfo(Form("Doing InitializePID for Detectors %s end", detectors.Data()));
  fDetectorPID[kMCpid] = new AliHFEpidMC("Monte Carlo PID"); // Always there
  fDetectorPID[kITSpid] = new AliHFEpidITS("ITS development PID");  // Development version of the ITS pid, for the moment always there
  SETBIT(fEnabledDetectors, kMCpid);
  SETBIT(fEnabledDetectors, kITSpid);
  
  TObjArray *detsEnabled = detectors.Tokenize(":");
  TIterator *detIterator = detsEnabled->MakeIterator();
  TObjString *det = 0x0;
  while((det = dynamic_cast<TObjString *>(detIterator->Next()))){
    if(det->String().CompareTo("TPC") == 0){
      AliInfo("Doing TPC PID");
      fDetectorPID[kTPCpid] = new AliHFEpidTPC("TPC PID");
      (dynamic_cast<AliHFEpidTPC *>(fDetectorPID[kTPCpid]))->SetAsymmetricTPCsigmaCut(2., 10., 0., 4.);
      SETBIT(fEnabledDetectors, kTPCpid);
    } else if(det->String().CompareTo("TRD") == 0){
      fDetectorPID[kTRDpid] = new AliHFEpidTRD("TRD PID");
      SETBIT(fEnabledDetectors, kTRDpid);
    } else if(det->String().CompareTo("TOF") == 0){
      fDetectorPID[kTOFpid] = new AliHFEpidTOF("TOF PID");
      SETBIT(fEnabledDetectors, kTOFpid);
    }
    // Here is still space for ESD PID
  }
  // Initialize PID Objects
  Bool_t status = kTRUE;
  for(Int_t idet = 0; idet < kNdetectorPID; idet++){
    if(fDetectorPID[idet]){ 
      status &= fDetectorPID[idet]->InitializePID();
      if(IsQAOn() && status) fDetectorPID[idet]->SetQAOn(fQAlist);
      if(HasMCData() && status) fDetectorPID[idet]->SetHasMCData();
    }
  }
  return status;
}

//____________________________________________________________
Bool_t AliHFEpid::IsSelected(AliHFEpidObject *track){
  //
  // Steers PID decision for single detectors respectively combined
  // PID decision
  //
  if(!track->fRecTrack){
    // MC Event
    return (TMath::Abs(fDetectorPID[kMCpid]->IsSelected(track)) == 11);
  }
  if(TESTBIT(fEnabledDetectors, kTPCpid)){
    if(IsQAOn() && fDebugLevel > 1){ 
      AliInfo("Filling QA plots");
      MakePlotsItsTpc(track);  // First fill the QA histograms
    }
    if(TESTBIT(fEnabledDetectors, kTOFpid)){
      // case TPC-TOF
      return MakePidTpcTof(track);
    } else if(TESTBIT(fEnabledDetectors, kTRDpid)){
      // case TPC-TRD with low level detector Signals
      return MakePidTpcTrd(track);
    } else
      return (TMath::Abs(fDetectorPID[kTPCpid]->IsSelected(track)) ==11);
  } else if(TESTBIT(fEnabledDetectors, kTRDpid)){
    return (TMath::Abs(fDetectorPID[kTRDpid]->IsSelected(track)) ==11);
  } else if(TESTBIT(fEnabledDetectors, kTOFpid)){
    return (TMath::Abs(fDetectorPID[kTOFpid]->IsSelected(track)) ==11);
  }
  
  return kFALSE;
}

//____________________________________________________________
Bool_t AliHFEpid::MakePidTpcTof(AliHFEpidObject *track){
  //
  // Combines TPC and TOF PID decision
  //
  if(fDetectorPID[kTOFpid]->IsSelected(track)) return fDetectorPID[kTPCpid]->IsSelected(track);
  return kFALSE;
}

//____________________________________________________________
Bool_t AliHFEpid::MakePidTpcTrd(AliHFEpidObject *track){
  //
  // Combination of TPC and TRD PID
  // Fills Histograms TPC Signal vs. TRD signal for different
  // momentum slices
  //
  if(track->fAnalysisType != AliHFEpidObject::kESDanalysis) return kFALSE; //AOD based detector PID combination not yet implemented
  AliESDtrack *esdTrack = dynamic_cast<AliESDtrack *>(track->fRecTrack);
  AliHFEpidTRD *trdPid = dynamic_cast<AliHFEpidTRD *>(fDetectorPID[kTRDpid]);
  Int_t pdg = TMath::Abs(fDetectorPID[kMCpid]->IsSelected(track));
  Int_t pid = -1;
  switch(pdg){
    case 11:    pid = AliPID::kElectron; break;
    case 13:    pid = AliPID::kMuon; break;
    case 211:   pid = AliPID::kPion; break;
    case 321:   pid = AliPID::kKaon; break;
    case 2212:  pid = AliPID::kProton; break;
    default:    pid = -1;
  };
  if(IsQAOn() && fDebugLevel > 1){
    Double_t content[10];
    content[0] = pid;
    content[1] = esdTrack->P();
    content[2] = esdTrack->GetTPCsignal();
    content[3] = trdPid->GetTRDSignalV1(esdTrack, pid);
    content[4] = trdPid->GetTRDSignalV2(esdTrack, pid);
    AliDebug(1, Form("Momentum: %f, TRD Signal: Method 1[%f], Method 2[%f]", content[1], content[3], content[4]));
    (dynamic_cast<THnSparseF *>(fQAlist->At(kTRDSignal)))->Fill(content);
  }
  Int_t trdDecision = 0;  // TRD decision 0 means outside the allowed momentum region
  return ((trdDecision = trdPid->IsSelected(track)) == 11 || trdDecision == 0) && fDetectorPID[kTPCpid]->IsSelected(track) == 11;
}

//____________________________________________________________
void AliHFEpid::MakePlotsItsTpc(AliHFEpidObject *track){
  //
  // Make a plot ITS signal - TPC signal for several momentum bins
  //
  if(track->fAnalysisType != AliHFEpidObject::kESDanalysis) return; //AOD based detector PID combination not yet implemented
  AliESDtrack * esdTrack = dynamic_cast<AliESDtrack *>(track->fRecTrack);
   // Fill My Histograms for MC PID
  Int_t pdg = TMath::Abs(fDetectorPID[kMCpid]->IsSelected(track));
  Int_t pid = -1;
  switch(pdg){
    case 11:    pid = AliPID::kElectron; break;
    case 13:    pid = AliPID::kMuon; break;
    case 211:   pid = AliPID::kPion; break;
    case 321:   pid = AliPID::kKaon; break;
    case 2212:  pid = AliPID::kProton; break;
    default:    pid = -1;
  };
  if(IsQAOn() && fDebugLevel > 0){
    Double_t content[10];
    content[0] = pid;
    content[1] = esdTrack->GetTPCInnerParam() ? esdTrack->GetTPCInnerParam()->P() : esdTrack->P();
    content[2] = (dynamic_cast<AliHFEpidITS *>(fDetectorPID[kITSpid]))->GetITSSignalV1(track->fRecTrack, pid);
    content[3] = esdTrack->GetTPCsignal();
    AliDebug(1, Form("Momentum %f, TPC Signal %f, ITS Signal %f", content[1], content[2], content[3]));
    (dynamic_cast<THnSparseF *>(fQAlist->At(kITSSignal)))->Fill(content);
  }
}

//____________________________________________________________
void AliHFEpid::SetQAOn(){
  //
  // Switch on QA
  //
  SetBit(kIsQAOn, kTRUE);
  AliInfo("QA switched on");
  if(fQAlist) return;
  fQAlist = new TList;
  fQAlist->SetName("PIDqa");
  THnSparseF *histo = NULL;

  // Prepare axis for QA histograms
  const Int_t kMomentumBins = 41;
  const Double_t kPtMin = 0.1;
  const Double_t kPtMax = 10.;
  Double_t momentumBins[kMomentumBins];
  for(Int_t ibin = 0; ibin < kMomentumBins; ibin++)
    momentumBins[ibin] = static_cast<Double_t>(TMath::Power(10,TMath::Log10(kPtMin) + (TMath::Log10(kPtMax)-TMath::Log10(kPtMin))/(kMomentumBins-1)*static_cast<Double_t>(ibin)));

  // Add Histogram for combined TPC-TRD PID
  if(fDebugLevel > 1){
    AliDebug(1, "Adding histogram for ITS-TPC investigation");
    const Int_t kDimensionsTRDsig = 5;
    Int_t kNbinsTRDsig[kDimensionsTRDsig] = {AliPID::kSPECIES + 1, kMomentumBins - 1, 200, 3000, 3000};
    Double_t binMinTRDsig[kDimensionsTRDsig] = {-1., 0.1, 0, 0, 0};
    Double_t binMaxTRDsig[kDimensionsTRDsig] = {AliPID::kSPECIES, 10., 200., 3000., 3000.};
    fQAlist->AddAt((histo = new THnSparseF("fCombTPCTRDpid", "Combined TPC-TRD PID", kDimensionsTRDsig, kNbinsTRDsig, binMinTRDsig, binMaxTRDsig)), kTRDSignal);
    histo->GetAxis(1)->Set(kMomentumBins - 1, momentumBins);
    histo->GetAxis(0)->SetTitle("Particle Species");
    histo->GetAxis(1)->SetTitle("p / GeV/c");
    histo->GetAxis(2)->SetTitle("TPC Signal / a.u.");
    histo->GetAxis(3)->SetTitle("TRD Signal / a.u.");
    histo->GetAxis(4)->SetTitle("TRD Signal / a.u.");
  }

  // Add Histogram for combined TPC-ITS PID
  if(fDebugLevel > 0){
    AliDebug(1, "Adding histogram for TPC-TRD investigation");
    const Int_t kDimensionsITSsig = 4;
    Int_t kNbinsITSsig[kDimensionsITSsig] = {AliPID::kSPECIES + 1, kMomentumBins - 1, 300, 3000};
    Double_t binMinITSsig[kDimensionsITSsig] = {-1., 0.1, 0., 0.};
    Double_t binMaxITSsig[kDimensionsITSsig] = {AliPID::kSPECIES, 10., 300., 300.};
    fQAlist->AddAt((histo = new THnSparseF("fCombTPCITSpid", "Combined TPC-ITS PID", kDimensionsITSsig, kNbinsITSsig, binMinITSsig, binMaxITSsig)), kITSSignal);
    histo->GetAxis(1)->Set(kMomentumBins - 1, momentumBins);
    histo->GetAxis(0)->SetTitle("Particle Species");
    histo->GetAxis(1)->SetTitle("p / GeV/c");
    histo->GetAxis(2)->SetTitle("ITS Signal / a.u.");
    histo->GetAxis(3)->SetTitle("TPC Signal / a.u.");
  }
}

