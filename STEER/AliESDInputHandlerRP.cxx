/**************************************************************************
 * Copyright(c) 1998-2007, ALICE Experiment at CERN, All rights reserved. *
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

/* $Id: AliESDInputHandler.cxx 24521 2008-03-14 16:43:54Z morsch $ */

//-------------------------------------------------------------------------
//     Event handler for ESD input reading the RecPoint Trees in parallel
//     Author: Andreas Morsch, CERN
//-------------------------------------------------------------------------

#include <TTree.h>
#include <TList.h>
#include <TFile.h>
#include <TString.h>
#include <TObjString.h>
#include <TProcessID.h>

#include "AliESDInputHandlerRP.h"
#include "AliESDEvent.h"
#include "AliESD.h"
#include "AliLog.h"

ClassImp(AliESDInputHandlerRP)

//______________________________________________________________________________
AliESDInputHandlerRP::AliESDInputHandlerRP() :
    AliESDInputHandler(),
    fRTrees(new TList()),
    fRFiles(new TList()),
    fDirR(0),
    fEventNumber(-1),
    fNEvent(-1),
    fFileNumber(0),
    fEventsPerFile(0),
    fExtension(""),
    fPathName(new TString("./"))
{
  // default constructor
}

//______________________________________________________________________________
AliESDInputHandlerRP::~AliESDInputHandlerRP() 
{
  // destructor
}

//______________________________________________________________________________
AliESDInputHandlerRP::AliESDInputHandlerRP(const char* name, const char* title):
    AliESDInputHandler(name, title),
    fRTrees(new TList()),
    fRFiles(new TList()),
    fDirR(0),
    fEventNumber(-1),
    fNEvent(-1),
    fFileNumber(0),
    fEventsPerFile(0),
    fExtension(""),
    fPathName(new TString("./"))
{
    // Constructor
}

Bool_t AliESDInputHandlerRP::Init(Option_t* opt)
{
    //
    // Initialize input
    //
    if (!(strcmp(opt, "proof")) || !(strcmp(opt, "local"))) return kTRUE;
    //
    TFile* file;
    
    file = TFile::Open(Form("%sTPC.RecPoints.root", fPathName->Data()));
    if (!file) AliFatal(Form("AliESDInputHandlerRP: TPC.RecPoints.root not found in %s ! \n", fPathName->Data()));
    fRFiles->Add(file);
    fEventsPerFile = file->GetNkeys() - file->GetNProcessIDs();
    // Reset the event number
    fEventNumber      = -1;
    fFileNumber       =  0;
    fNEvent           =  fTree->GetEntries();
    
    printf("AliESDInputHandler::Init() %d\n",__LINE__);
    return kTRUE;
}

Bool_t AliESDInputHandlerRP::BeginEvent(Long64_t entry)
{
    // Begin the next event
    // Delegate to base class
    AliESDInputHandler::BeginEvent(entry);
//
    if (entry == -1) {
	fEventNumber++;
	entry = fEventNumber;
    } else {
	fEventNumber = entry;
    }
    
    if (entry >= fNEvent) {
	AliWarning(Form("AliESDInputHandlerRP: Event number out of range %5d %5d\n", entry, fNEvent));
	return kFALSE;
    }
    return LoadEvent(entry);
}

Bool_t AliESDInputHandlerRP::LoadEvent(Int_t iev)
{
    // Load the event number iev
    //
    // Calculate the file number
    Int_t inew  = iev / fEventsPerFile;
    if (inew != fFileNumber) {
	fFileNumber = inew;
	if (!OpenFile(fFileNumber)){
	    return kFALSE;
	}
    }
    // Folder name
    char folder[20];
    sprintf(folder, "Event%d", iev);
    // Tree R
    TFile* file = (TFile*) (fRFiles->At(0));
    
    file->GetObject(folder, fDirR);
    if (!fDirR) {
	AliWarning(Form("AliESDInputHandlerRP: Event #%5d not found\n", iev));
	return kFALSE;
    }
    
    TTree* tree;
    fDirR ->GetObject("TreeR", tree);
    fRTrees->Add(tree);
    tree->ls();
    
    return kTRUE;
}

Bool_t AliESDInputHandlerRP::OpenFile(Int_t i)
{
    // Open file i
    Bool_t ok = kTRUE;
    if (i > 0) {
	fExtension = Form("%d", i);
    } else {
	fExtension = "";
    }
    
    fRFiles->Delete();
    TFile* file;
    file = TFile::Open(Form("%sTPC.RecPoints%s.root", fPathName->Data(), fExtension));
    if (!file) {
	AliFatal(Form("AliESDInputHandlerRP: TPC.RecPoints.root not found in %s ! \n", fPathName->Data()));
	ok = kFALSE;
    }
    return ok;
}

Bool_t AliESDInputHandlerRP::Notify(const char *path)
{
  // Notify about directory change
  // The directory is taken from the 'path' argument
  // Reconnect trees
    TString fileName(path);
    if(fileName.Contains("AliESDs.root")){
	fileName.ReplaceAll("AliESDs.root", "");
    }

    *fPathName = fileName;

    printf("AliESDInputHandlerRP::Notify() Path: %s\n", fPathName->Data());
    
    ResetIO();
    InitIO("");

    return kTRUE;
}

Bool_t AliESDInputHandlerRP::FinishEvent()
{
    // Clean-up after each event
    delete fDirR;  fDirR = 0;
    AliESDInputHandler::FinishEvent();
    return kTRUE;
}

void AliESDInputHandlerRP::ResetIO()
{
// Delete trees and files
    fRTrees->Delete();
    fRFiles->Delete();
    fExtension="";
}
