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

/* $Id$ */
 
#include <TROOT.h>
#include <TSystem.h>
#include <TInterpreter.h>
#include <TChain.h>
#include <TFile.h>
#include <TList.h>

#include "AliAnalysisTaskME.h"
#include "AliAnalysisManager.h"
#include "AliAODEvent.h"
#include "AliAODHandler.h"
#include "AliMultiAODInputHandler.h"
#include "AliLog.h"


ClassImp(AliAnalysisTaskME)

////////////////////////////////////////////////////////////////////////

AliAnalysisTaskME::AliAnalysisTaskME():
    AliAnalysisTask(),
    fDebug(0),
    fEntry(0),
    fFreshBufferOnly(kFALSE),
    fInputHandler(0x0),
    fOutputAOD(0x0),
    fTreeA(0x0)
{
  // Default constructor
}

AliAnalysisTaskME::AliAnalysisTaskME(const char* name):
    AliAnalysisTask(name, "AnalysisTaskME"),
    fDebug(0),
    fEntry(0),
    fFreshBufferOnly(kFALSE),
    fInputHandler(0x0),
    fOutputAOD(0x0),
    fTreeA(0x0)
{
  // Default constructor
    DefineInput (0, TChain::Class());
    DefineOutput(0,  TTree::Class());
}

AliAnalysisTaskME::AliAnalysisTaskME(const AliAnalysisTaskME& obj):
    AliAnalysisTask(obj),
    fDebug(0),
    fEntry(0),
    fFreshBufferOnly(kFALSE),
    fInputHandler(0x0),
    fOutputAOD(0x0),
    fTreeA(0x0)
{
// Copy constructor
    fDebug        = obj.fDebug;
    fEntry        = obj.fEntry;
    fInputHandler = obj.fInputHandler;
    fOutputAOD    = obj.fOutputAOD;
    fTreeA        = obj.fTreeA;    
}


AliAnalysisTaskME& AliAnalysisTaskME::operator=(const AliAnalysisTaskME& other)
{
// Assignment
    AliAnalysisTask::operator=(other);
    fDebug           = other.fDebug;
    fEntry           = other.fEntry;
    fFreshBufferOnly = other.fFreshBufferOnly;
    fInputHandler    = other.fInputHandler;
    fOutputAOD       = other.fOutputAOD;
    fTreeA           = other.fTreeA;    
    return *this;
}


void AliAnalysisTaskME::ConnectInputData(Option_t* /*option*/)
{
// Connect the input data
    if (fDebug > 1) printf("AnalysisTaskME::ConnectInputData() \n");
//
//  Multi AOD
//
    fInputHandler = dynamic_cast<AliMultiAODInputHandler*> 
	((AliAnalysisManager::GetAnalysisManager())->GetInputEventHandler());
    if (fInputHandler == 0) {
	AliFatal("Event Handler has to be MultiAODInputHandler !");
    } else {
	// Check that we have an event pool
	if (!fInputHandler->GetEventPool()) {
	    fInputHandler->SetEventPool(AliAnalysisManager::GetAnalysisManager()->GetEventPool());
	    if (!fInputHandler->GetEventPool()) 
		AliFatal("MultiAODInputHandler has no EventPool connected !");
	}
    }
}

void AliAnalysisTaskME::CreateOutputObjects()
{
// Create the output container
//
//  Default AOD
    if (fDebug > 1) printf("AnalysisTaskME::CreateOutPutData() \n");

    AliAODHandler* handler = (AliAODHandler*) 
	((AliAnalysisManager::GetAnalysisManager())->GetOutputEventHandler());
    
    if (handler) {
	fOutputAOD   = handler->GetAOD();
	fTreeA = handler->GetTree();
    } else {
	AliWarning("No AOD Event Handler connected.") ; 
    }
    UserCreateOutputObjects();
}

void AliAnalysisTaskME::Exec(Option_t* option)
{
//
// Exec analysis of one event

    if (fDebug > 1) AliInfo("AliAnalysisTaskME::Exec() \n");
    if( fInputHandler ) 
       fEntry = fInputHandler->GetReadEntry();
    if ( !((Entry()-1)%100) && fDebug > 0) 
         AliInfo(Form("%s ----> Processing event # %lld", CurrentFileName(), Entry()));
         
// Call the user analysis    
    if (fInputHandler->IsBufferReady()) {
	if ((fFreshBufferOnly && fInputHandler->IsFreshBuffer()) || !fFreshBufferOnly)
	{
	    UserExec(option);
	    PostData(0, fTreeA);
	}
    } else {
	AliInfo(Form("Waiting for buffer to be ready !\n"));
    }
}

const char* AliAnalysisTaskME::CurrentFileName()
{
// Returns the current file name    
    if(fInputHandler )
	return fInputHandler->GetTree()->GetCurrentFile()->GetName();
    else return "";
}

void AliAnalysisTaskME::AddAODBranch(const char* cname, void* addobj)
{
    // Add a new branch to the aod tree
    AliAODHandler* handler = (AliAODHandler*) 
	((AliAnalysisManager::GetAnalysisManager())->GetOutputEventHandler());
    if (handler) {
	handler->AddBranch(cname, addobj);
    }
}

AliAODEvent*  AliAnalysisTaskME::GetEvent(Int_t iev)
{
    // Get an event from the input handler
    return (fInputHandler->GetEvent(iev));
}

