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

/* $Header$ */

////////////////////////////////////////////////////////////////////////
//
// AliRunDigitizer.cxx
//
// Manager object for merging/digitization
//
// Instance of this class manages the digitization and/or merging of
// Sdigits into Digits. 
//
// Only one instance of this class is created in the macro:
//   AliRunDigitizer * manager = 
//      new AliRunDigitizer(nInputStreams,SPERB);
// where nInputStreams is number of input streams and SPERB is
// signals per background variable, which determines how combinations
// of signal and background events are generated.
// Then instances of specific detector digitizers are created:
//   AliMUONDigitizer *dMUON  = new AliMUONDigitizer(manager)
// and the I/O configured (you have to specify input files 
// and an output file). The manager connects appropriate trees from 
// the input files according a combination returned by AliMergeCombi 
// class. It creates TreeD in the output and runs once per 
// event Digitize method of all existing AliDetDigitizers 
// (without any option). AliDetDigitizers ask manager
// for a TTree with input (manager->GetInputTreeS(Int_t i),
// merge all inputs, digitize it, and save it in the TreeD 
// obtained by manager->GetTreeD(). Output events are stored with 
// numbers from 0, this default can be changed by 
// manager->SetFirstOutputEventNr(Int_t) method. The particle numbers
// in the output are shifted by MASK, which is taken from manager.
//
// The default output is to the signal file (stream 0). This can be 
// changed with the SetOutputFile(TString fn)  method.
//
// Single input file is permitted. Maximum kMaxStreamsToMerge can be merged.
// Input from the memory (on-the-fly merging) is not yet 
// supported, as well as access to the input data by invoking methods
// on the output data.
//
// Access to the some data is via gAlice for now (supposing the 
// same geometry in all input files), gAlice is taken from the first 
// input file on the first stream.
//
// Example with MUON digitizer, no merging, just digitization
//
//  AliRunDigitizer * manager = new AliRunDigitizer(1,1);
//  manager->SetInputStream(0,"galice.root");
//  AliMUONDigitizer *dMUON  = new AliMUONDigitizer(manager);
//  manager->Exec("");
//
// Example with MUON digitizer, merge all events from 
//   galice.root (signal) file with events from bgr.root 
//   (background) file. Number of merged events is
//   min(number of events in galice.root, number of events in bgr.root)
//
//  AliRunDigitizer * manager = new AliRunDigitizer(2,1);
//  manager->SetInputStream(0,"galice.root");
//  manager->SetInputStream(1,"bgr.root");
//  AliMUONDigitizer *dMUON  = new AliMUONDigitizer(manager);
//  manager->Exec("");
//
// Example with MUON digitizer, save digits in a new file digits.root,
//   process only 1 event
//
//  AliRunDigitizer * manager = new AliRunDigitizer(2,1);
//  manager->SetInputStream(0,"galice.root");
//  manager->SetInputStream(1,"bgr.root");
//  manager->SetOutputFile("digits.root");
//  AliMUONDigitizer *dMUON  = new AliMUONDigitizer(manager);
//  manager->SetNrOfEventsToWrite(1);
//  manager->Exec("");
//
//////////////////////////////////////////////////////////////////////// 

// system includes

#include <Riostream.h>

// ROOT includes

#include "TFile.h"
#include "TList.h"
#include "TParticle.h"
#include "TTree.h"

// AliROOT includes

#include "AliDigitizer.h"
#include "AliHeader.h"
#include "AliMergeCombi.h"
#include "AliRun.h"
#include "AliRunDigitizer.h"
#include "AliStream.h"

ClassImp(AliRunDigitizer)

//_______________________________________________________________________
AliRunDigitizer::AliRunDigitizer():
  fkMASKSTEP(0),
  fOutputFileName(0),
  fOutputDirName(0),
  fOutput(0),
  fEvent(0),
  fNrOfEventsToWrite(0),
  fNrOfEventsWritten(0),
  fCopyTreesFromInput(0),
  fTreeD(0),
  fTreeDTPC(0),
  fTreeDTRD(0),
  fTreeR(0),
  fNinputs(0),
  fNinputsGiven(0),
  fInputStreams(0),
  fTreeDTPCBaseName(0),
  fTreeTPCSBaseName(0),
  fCombi(0),
  fCombination(0),
  fCombinationFileName(0),
  fDebug(0)
{
  //
  // default ctor, where no new objects can be created
  // do not use this ctor, it is supplied only for root needs
  //
  for (Int_t i=0;i<kMaxStreamsToMerge;i++) {
    fArrayTreeS[i]=fArrayTreeH[i]=fArrayTreeTPCS[i]=fArrayTreeTRDS[i]=NULL;
    fInputFiles[i]=0;
  }
}

//_______________________________________________________________________
AliRunDigitizer::AliRunDigitizer(const AliRunDigitizer& dig):
  TTask(dig),
  fkMASKSTEP(0),
  fOutputFileName(0),
  fOutputDirName(0),
  fOutput(0),
  fEvent(0),
  fNrOfEventsToWrite(0),
  fNrOfEventsWritten(0),
  fCopyTreesFromInput(0),
  fTreeD(0),
  fTreeDTPC(0),
  fTreeDTRD(0),
  fTreeR(0),
  fNinputs(0),
  fNinputsGiven(0),
  fInputStreams(0),
  fTreeDTPCBaseName(0),
  fTreeTPCSBaseName(0),
  fCombi(0),
  fCombination(0),
  fCombinationFileName(0),
  fDebug(0)
{
  //
  // Copy ctor
  //
  dig.Copy(*this);
}

//_______________________________________________________________________
void AliRunDigitizer::Copy(AliRunDigitizer&) const
{
  Fatal("Copy","Not installed\n");
}


//_______________________________________________________________________
AliRunDigitizer::AliRunDigitizer(Int_t nInputStreams, Int_t sperb):
  TTask("AliRunDigitizer","The manager for Merging"),
  fkMASKSTEP(10000000),
  fOutputFileName(""),
  fOutputDirName("."),
  fOutput(0),
  fEvent(0),
  fNrOfEventsToWrite(-1),
  fNrOfEventsWritten(0),
  fCopyTreesFromInput(-1),
  fTreeD(0),
  fTreeDTPC(0),
  fTreeDTRD(0),
  fTreeR(0),
  fNinputs(nInputStreams),
  fNinputsGiven(0),
  fInputStreams(new TClonesArray("AliStream",nInputStreams)),
  fTreeDTPCBaseName("TreeD_75x40_100x60_150x60_"),
  fTreeTPCSBaseName("TreeS_75x40_100x60_150x60_"),
  fCombi(new AliMergeCombi(nInputStreams,sperb)),
  fCombination(kMaxStreamsToMerge),
  fCombinationFileName(0),
  fDebug(0)
{
  //
  // ctor which should be used to create a manager for merging/digitization
  //
  if (nInputStreams == 0) {
    Error("AliRunDigitizer","Specify nr of input streams");
    return;
  }
  for (Int_t i=0;i<kMaxStreamsToMerge;i++) {
    fArrayTreeS[i]=fArrayTreeH[i]=fArrayTreeTPCS[i]=fArrayTreeTRDS[i]=NULL;
    fCombination[i]=-1;
  }
  fkMASK[0] = 0;
  for (Int_t i=1;i<kMaxStreamsToMerge;i++) {
    fkMASK[i] = fkMASK[i-1] + fkMASKSTEP;
  }

  TClonesArray &lInputStreams = *fInputStreams;
// the first Input is open RW to be output as well
  new(lInputStreams[0]) AliStream("UPDATE");
  for (Int_t i=1;i<nInputStreams;i++) {
    new(lInputStreams[i]) AliStream("READ");
  }

  for (Int_t i=0; i<kMaxStreamsToMerge; i++) fInputFiles[i]=0;
}

//_______________________________________________________________________
AliRunDigitizer::~AliRunDigitizer() {
// dtor

// do not delete subtasks, let the creator delete them
  if (GetListOfTasks()) 
    GetListOfTasks()->Clear("nodelete");
  
    delete fInputStreams;
    delete fCombi;
}

//_______________________________________________________________________
void AliRunDigitizer::AddDigitizer(AliDigitizer *digitizer)
{
// add digitizer to the list of active digitizers
  this->Add(digitizer);
}

//_______________________________________________________________________
void AliRunDigitizer::SetInputStream(Int_t i, const char *inputFile)
{
  if (i > fInputStreams->GetLast()) {
    Error("SetInputStream","Input stream number too high");
    return;
  }
  static_cast<AliStream*>(fInputStreams->At(i))->AddFile(inputFile);
}

//_______________________________________________________________________
void AliRunDigitizer::Digitize(Option_t* option)
{
// get a new combination of inputs, connect input trees and loop 
// over all digitizers

// take gAlice from the first input file. It is needed to access
//  geometry data
// If gAlice is already in memory, use it
  if (!gAlice) {
    if (!static_cast<AliStream*>(fInputStreams->At(0))->ImportgAlice()) {
      cerr<<"gAlice object not found in the first file of "
	  <<"the 1st stream"<<endl;
      return;
    }
  }
  if (!InitGlobal()) {
    cerr<<"False from InitGlobal"<<endl;
    return;
  }
  Int_t eventsCreated = 0;
// loop until there is anything on the input in case fNrOfEventsToWrite < 0
  while ((eventsCreated++ < fNrOfEventsToWrite) || (fNrOfEventsToWrite < 0)) {
    if (!ConnectInputTrees()) break;
    InitEvent();
// loop over all registered digitizers and let them do the work
    ExecuteTasks(option);
    CleanTasks();
    FinishEvent();
  }
  FinishGlobal();
}

//_______________________________________________________________________
Bool_t AliRunDigitizer::ConnectInputTrees()
{
// fill arrays fArrayTreeS, fArrayTreeH and fArrayTreeTPCS with 
// pointers to the correct events according fCombination values
// null pointers can be in the output, AliDigitizer has to check it

  TTree *tree;
  char treeName[50];
  Int_t serialNr;
  Int_t eventNr[kMaxStreamsToMerge], delta[kMaxStreamsToMerge];
  fCombi->Combination(eventNr, delta);
  for (Int_t i=0;i<fNinputs;i++) {
    if (delta[i] == 1) {
      AliStream *iStream = static_cast<AliStream*>(fInputStreams->At(i));
      if (!iStream->NextEventInStream(serialNr)) return kFALSE;
      fInputFiles[i]=iStream->CurrentFile();
      sprintf(treeName,"TreeS%d",serialNr);
      tree = static_cast<TTree*>(iStream->CurrentFile()->Get(treeName));
      if (fArrayTreeS[i]) {
	delete fArrayTreeS[i];
	fArrayTreeS[i] = 0;
      }
      fArrayTreeS[i] = tree;
      sprintf(treeName,"TreeH%d",serialNr);
      tree = static_cast<TTree*>(iStream->CurrentFile()->Get(treeName));
      if (fArrayTreeH[i]) {
	delete fArrayTreeH[i];
	fArrayTreeH[i] = 0;
      }
      fArrayTreeH[i] = tree;
      sprintf(treeName,"%s%d",fTreeTPCSBaseName,serialNr);
      tree = static_cast<TTree*>(iStream->CurrentFile()->Get(treeName));
      if (fArrayTreeTPCS[i]) {
	delete fArrayTreeTPCS[i];
	fArrayTreeTPCS[i] = 0;
      }
      fArrayTreeTPCS[i] = tree;
      sprintf(treeName,"TreeS%d_TRD",serialNr);
      tree = static_cast<TTree*>(iStream->CurrentFile()->Get(treeName));
      if (fArrayTreeTRDS[i]) {
	delete fArrayTreeTRDS[i];
	fArrayTreeTRDS[i] = 0;
      }
      fArrayTreeTRDS[i] = tree;
    } else if (delta[i] != 0) {
      Error("ConnectInputTrees","Only delta 0 or 1 is implemented");
      return kFALSE;
    }
  }
  return kTRUE;
}

//_______________________________________________________________________
Bool_t AliRunDigitizer::InitGlobal()
{
// called once before Digitize() is called, initialize digitizers and output

  TList* subTasks = GetListOfTasks();
  if (subTasks) {
    subTasks->ForEach(AliDigitizer,Init)();
  }  
  return kTRUE;
}

//_______________________________________________________________________
void AliRunDigitizer::SetOutputFile(TString fn)
// the output will be to separate file, not to the signal file
{
  fOutputFileName = fn;
  (static_cast<AliStream*>(fInputStreams->At(0)))->ChangeMode("READ");
  InitOutputGlobal();
}

//_______________________________________________________________________
Bool_t AliRunDigitizer::InitOutputGlobal()
{
// Creates the output file, called by InitEvent()

  TString fn;
  fn = fOutputDirName + '/' + fOutputFileName;
  fOutput = new TFile(fn,"update");
  if (GetDebug()>2) {
    cerr<<"AliRunDigitizer::InitOutputGlobal(): file "<<fn.Data()<<" was opened"<<endl;
  }
  if (fOutput) return kTRUE;
  Error("InitOutputGlobal","Could not create output file.");
  return kFALSE;
}


//_______________________________________________________________________
void AliRunDigitizer::InitEvent()
{
// Creates TreeDxx in the output file, called from Digitize() once for 
//  each event. xx = fEvent

  if (GetDebug()>2) 
    cerr<<"AliRunDigitizer::InitEvent: fEvent = "<<fEvent<<endl;

// if fOutputFileName was not given, write output to signal file
  if (fOutputFileName == "") {
    fOutput = (static_cast<AliStream*>(fInputStreams->At(0)))->CurrentFile();
  }
  fOutput->cd();
  char treeName[30];
  sprintf(treeName,"TreeD%d",fEvent);
  fTreeD = static_cast<TTree*>(fOutput->Get(treeName));
  if (!fTreeD) {
    fTreeD = new TTree(treeName,"Digits");
    fTreeD->Write(0,TObject::kOverwrite);
  }

// tree for ITS fast points
  sprintf(treeName,"TreeR%d",fEvent);
  fTreeR = static_cast<TTree*>(fOutput->Get(treeName));
  if (!fTreeR) {
    fTreeR = new TTree(treeName,"Reconstruction");
    fTreeR->Write(0,TObject::kOverwrite);
  }

// special tree for TPC
  sprintf(treeName,"%s%d",fTreeDTPCBaseName,fEvent);
  fTreeDTPC = static_cast<TTree*>(fOutput->Get(treeName));
  if (!fTreeDTPC) {
    fTreeDTPC = new TTree(treeName,"TPC_Digits");
    fTreeDTPC->Write(0,TObject::kOverwrite);
  }

// special tree for TRD
  sprintf(treeName,"TreeD%d_TRD",fEvent);
  fTreeDTRD = static_cast<TTree*>(fOutput->Get(treeName));
  if (!fTreeDTRD) {
    fTreeDTRD = new TTree(treeName,"TRD_Digits");
    fTreeDTRD->Write(0,TObject::kOverwrite);
  }

}

//_______________________________________________________________________
void AliRunDigitizer::FinishEvent()
{
// called at the end of loop over digitizers

  Int_t i;
  fOutput->cd();
  if (fCopyTreesFromInput > -1) {
    char treeName[20];
    i = fCopyTreesFromInput; 
    sprintf(treeName,"TreeK%d",fCombination[i]);
    fInputFiles[i]->Get(treeName)->Clone()->Write();
    sprintf(treeName,"TreeH%d",fCombination[i]);
    fInputFiles[i]->Get(treeName)->Clone()->Write();
  }
  fEvent++;
  fNrOfEventsWritten++;
  if (fTreeD) {
    delete fTreeD;
    fTreeD = 0;
  }
  if (fTreeR) {
    delete fTreeR;
    fTreeR = 0;
  }
  if (fTreeDTPC) {
    delete fTreeDTPC;
    fTreeDTPC = 0;
  }
  if (fTreeDTRD) {
    delete fTreeDTRD;
    fTreeDTRD = 0;
  }
}

//_______________________________________________________________________
void AliRunDigitizer::FinishGlobal()
{
// called at the end of Exec
// save unique objects to the output file

  fOutput->cd();
  this->Write(0,TObject::kOverwrite);
  if (fCopyTreesFromInput > -1) {
    fInputFiles[fCopyTreesFromInput]->Get("TE")->Clone()->Write();
    gAlice->Write();
  }
  fOutput->Write();
}


//_______________________________________________________________________
Int_t  AliRunDigitizer::GetNParticles(Int_t event) const
{
// return number of particles in all input files for a given
// event (as numbered in the output file)
// return -1 if some file cannot be accessed

  Int_t sum = 0;
  Int_t sumI;
  for (Int_t i = 0; i < fNinputs; i++) {
    sumI = GetNParticles(GetInputEventNumber(event,i), i);
    if (sumI < 0) return -1;
    sum += sumI;
  }
  return sum;
}

//_______________________________________________________________________
Int_t  AliRunDigitizer::GetNParticles(Int_t /* event */, Int_t /* input */) const
{
// return number of particles in input file input for a given
// event (as numbered in this input file)
// return -1 if some error

// Must be revised in the version with AliStream

  return -1;

/*
  TFile *file = ConnectInputFile(input);
  if (!file) {
    Error("GetNParticles","Cannot open input file");
    return -1;
  }

// find the header and get Nprimaries and Nsecondaries
  TTree* tE = (TTree *)file->Get("TE") ;
  if (!tE) {
    Error("GetNParticles","input file does not contain TE");
    return -1;
  }
  AliHeader* header;
  header = 0;
  tE->SetBranchAddress("Header", &header);
  if (!tE->GetEntry(event)) {
    Error("GetNParticles","event %d not found",event);
    return -1;
  }
  if (GetDebug()>2) {
    cerr<<"Nprimary: "<< header->GetNprimary()<<endl;
    cerr<<"Nsecondary: "<<header->GetNsecondary()<<endl;
  }
  return header->GetNprimary() + header->GetNsecondary();
*/
}

//_______________________________________________________________________
Int_t* AliRunDigitizer::GetInputEventNumbers(Int_t event) const
{
// return pointer to an int array with input event numbers which were
// merged in the output event event

// simplified for now, implement later
  Int_t * a = new Int_t[kMaxStreamsToMerge];
  for (Int_t i = 0; i < fNinputs; i++) {
    a[i] = event;
  }
  return a;
}

//_______________________________________________________________________
Int_t AliRunDigitizer::GetInputEventNumber(Int_t event, Int_t /* input */) const
{
// return an event number of an eventInput from input file input
// which was merged to create output event event

// simplified for now, implement later
  return event;
}

//_______________________________________________________________________
TParticle* AliRunDigitizer::GetParticle(Int_t i, Int_t event) const
{
// return pointer to particle with index i (index with mask)

// decode the MASK
  Int_t input = i/fkMASKSTEP;
  return GetParticle(i,input,GetInputEventNumber(event,input));
}

//_______________________________________________________________________
TParticle* AliRunDigitizer::GetParticle(Int_t /* i */, Int_t /* input */, 
                                        Int_t /* event */) const
{
// return pointer to particle with index i in the input file input
// (index without mask)
// event is the event number in the file input
// return 0 i fit does not exist

// Must be revised in the version with AliStream

  return 0;
/*
  TFile *file = ConnectInputFile(input);
  if (!file) {
    Error("GetParticle","Cannot open input file");
    return 0;
  }

// find the header and get Nprimaries and Nsecondaries
  TTree* tE = (TTree *)file->Get("TE") ;
  if (!tE) {
    Error("GetParticle","input file does not contain TE");
    return 0;
  }
  AliHeader* header;
  header = 0;
  tE->SetBranchAddress("Header", &header);
  if (!tE->GetEntry(event)) {
    Error("GetParticle","event %d not found",event);
    return 0;
  }
  
// connect TreeK  
  char treeName[30];
  sprintf(treeName,"TreeK%d",event);  
  TTree* tK = static_cast<TTree*>(file->Get(treeName));
  if (!tK) {
    Error("GetParticle","input file does not contain TreeK%d",event);
    return 0;
  }
  TParticle *particleBuffer;
  particleBuffer = 0;
  tK->SetBranchAddress("Particles", &particleBuffer);


// algorithmic way of getting entry index
// (primary particles are filled after secondaries)
  Int_t entry;
  if (i<header->GetNprimary())
    entry = i+header->GetNsecondary();
  else 
    entry = i-header->GetNprimary();
  Int_t bytesRead = tK->GetEntry(entry);
//  new ((*fParticles)[nentries]) TParticle(*fParticleBuffer);
  if (bytesRead)
    return particleBuffer;
  return  0;
*/
}

//_______________________________________________________________________
void AliRunDigitizer::ExecuteTask(Option_t* option)
{
// overwrite ExecuteTask to do Digitize only

  if (!IsActive()) return;
  Digitize(option);
  fHasExecuted = kTRUE;
  return;
}

//_______________________________________________________________________
TString AliRunDigitizer::GetInputFileName(const Int_t input, const Int_t order) const 
{
// returns file name of the order-th file in the input stream input
// returns empty string if such file does not exist
// first input stream is 0
// first file in the input stream is 0
  TString fileName("");
  if (input >= fNinputs) return fileName;
  AliStream * stream = static_cast<AliStream*>(fInputStreams->At(input));
  if (order > stream->GetNInputFiles()) return fileName;
  fileName = stream->GetFileName(order);
  return fileName;
}
