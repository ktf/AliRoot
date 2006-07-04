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

/*
 $Log$
 Revision 1.3  2006/06/12 09:11:16  jgrosseo
 coding conventions (Alberto)

 Revision 1.2  2006/06/06 14:26:40  jgrosseo
 o) removed files that were moved to STEER
 o) shuttle updated to follow the new interface (Alberto)

 Revision 1.1  2006/03/07 07:52:34  hristov
 New version (B.Yordanov)

 Revision 1.5  2005/11/21 09:03:48  byordano
 one more print added

 Revision 1.4  2005/11/20 10:12:37  byordano
 comments added to AliShuttleTrigger

 */


// 
// This class is to deal with DAQ LogBook and DAQ "end of run" notification.
// It has severeal two modes:
// 	1) syncrhnized - Collect(), CollectNew() and CollectAll methods
// 	2) asynchronized - Run() - starts listening for DAQ "end of run"
// 		notification by DIM service.
//

#include "AliShuttleTrigger.h"

#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TObjArray.h>
#include <TSystem.h>

#include "AliLog.h"
#include "AliCDBManager.h"
#include "AliCDBStorage.h"
#include "AliCDBEntry.h"

#include "AliDCSValue.h"
#include "AliShuttleConfig.h"
#include "AliShuttle.h"
#include "DATENotifier.h"

ClassImp(TerminateSignalHandler)

//______________________________________________________________________
TerminateSignalHandler::TerminateSignalHandler(const TerminateSignalHandler& /*other*/):
TSignalHandler()
{
// copy constructor (not implemented)

}

//______________________________________________________________________
TerminateSignalHandler &TerminateSignalHandler::operator=(const TerminateSignalHandler& /*other*/)
{
// assignment operator (not implemented)

return *this;
}

//______________________________________________________________________________________________
Bool_t TerminateSignalHandler::Notify() 
{
// Sentd terminate command to the Shuttle trigger

	AliInfo("Terminate signal received ...");
	fTrigger->Terminate();

	return kTRUE;
}

//______________________________________________________________________________________________
//______________________________________________________________________________________________

ClassImp(AliShuttleTrigger)

//______________________________________________________________________________________________
AliShuttleTrigger::AliShuttleTrigger(const AliShuttleConfig* config,
		UInt_t timeout, Int_t retries):
	fConfig(config), fShuttle(NULL),
	fNotified(kFALSE), fTerminate(kFALSE), fCondition(&fMutex),
	fQuitSignalHandler(this, kSigQuit), 
	fInterruptSignalHandler(this, kSigInterrupt)
{
	//
	// config - pointer to the AliShuttleConfig object which represents
	// the configuration
	// mainStorage - pointer to AliCDBStorage for the undelying CDBStorage
	// localStorage (local) CDB storage to be used if mainStorage is unavailable
	//

	fShuttle = new AliShuttle(config, timeout, retries);

	gSystem->AddSignalHandler(&fQuitSignalHandler);
	gSystem->AddSignalHandler(&fInterruptSignalHandler);
}



//______________________________________________________________________
AliShuttleTrigger::AliShuttleTrigger(const AliShuttleTrigger& /*other*/):
TObject()
{
// copy constructor (not implemented)

}

//______________________________________________________________________
AliShuttleTrigger &AliShuttleTrigger::operator=(const AliShuttleTrigger& /*other*/)
{
// assignment operator (not implemented)

return *this;
}





//______________________________________________________________________________________________
AliShuttleTrigger::~AliShuttleTrigger() 
{
// destructor

	gSystem->RemoveSignalHandler(&fQuitSignalHandler);
	gSystem->RemoveSignalHandler(&fInterruptSignalHandler);

	delete fShuttle;
}

//______________________________________________________________________________________________
Bool_t AliShuttleTrigger::Notify() {
	//
	// Trigger CollectNew() methods in asynchronized (listen) mode.
	// Usually called automaticly by DATENotifier on "end of run" 
	// notification event.
	//

	fMutex.Lock();

	fNotified = kTRUE;
	fCondition.Signal();

	fMutex.UnLock();

	return kTRUE;
}

//______________________________________________________________________________________________
void AliShuttleTrigger::Terminate() {
	//
	// Stop triggers listen mode and exist from Run()
	// Usually called automaticly by TerminateSignalHandler.
	//

	fTerminate = kTRUE;
	fCondition.Signal();
}

//______________________________________________________________________________________________
void AliShuttleTrigger::Run() {
	//
	// AliShuttleTrigger main loop for asynchronized (listen) mode.
	// It spawns DIM service listener and waits for DAQ "end of run"
	// notification. Calls CollectNew() on notification.
	//

	fTerminate = kFALSE;

	DATENotifier* notifier = new DATENotifier(this, "/DATE/LOGBOOK/UPDATE");

	while (1) {
	
		fMutex.Lock();

		while (!(fNotified || fTerminate)) {
			fCondition.Wait();
		}

		fNotified = kFALSE;
		
		fMutex.UnLock();

		if (fTerminate) {
			AliInfo("Terminated.");
			break;		
		}
	
		CollectNew();
	}

	delete notifier;
}

//______________________________________________________________________________________________
Bool_t AliShuttleTrigger::RetrieveDATEEntries(const char* whereClause,
		TObjArray& entries, Int_t& lastRun) 
{
// Retrieve start time and end time for all runs in the DAQ logbook
// that aren't processed yet

	TString sqlQuery;
	sqlQuery += "select run, time_start, time_end from logbook ";
	sqlQuery += whereClause;
	sqlQuery += " order by run";

	TSQLServer* aServer;
	TString logbookHost="mysql://";
	logbookHost+=fConfig->GetDAQLogBookHost();
	
	aServer = TSQLServer::Connect(logbookHost,
			fConfig->GetDAQLogBookUser(),
			fConfig->GetDAQLogBookPassword());
	if (!aServer) {
		AliError("Can't establish connection to DAQ log book DB!");
		return kFALSE;
	}
	
	aServer->GetTables("REFSYSLOG");

	TSQLResult* aResult;
	aResult = aServer->Query(sqlQuery);
	if (!aResult) {
		AliError(Form("Can't execute query <%s>!", sqlQuery.Data()));
		delete aServer;
		return kFALSE;
	}

	if (aResult->GetFieldCount() != 3) {
		AliError("Invalid SQL result field number!");
		delete aResult;
		delete aServer;
		return kFALSE;
	}

	lastRun = 0;

	TSQLRow* aRow;
	while ((aRow = aResult->Next())) {
		TString runString(aRow->GetField(0), aRow->GetFieldLength(0));
		Int_t run = runString.Atoi();

		TString startTimeString(aRow->GetField(1),
				aRow->GetFieldLength(1));
		Int_t startTime = startTimeString.Atoi();
		if (!startTime) {
			AliWarning(Form("Zero StartTime for run <%d>!", run));
			AliWarning("Going to skip this run!");
			continue;
		}

		TString endTimeString(aRow->GetField(2),
				aRow->GetFieldLength(2));
		Int_t endTime = endTimeString.Atoi();
		if (!endTime) {
			AliWarning(Form("Zero EndTime for run <%d>!", run));
			AliWarning("Going to skip this run!");
			continue;
		}

		if (startTime > endTime) {
			AliWarning(Form("StartTime bigger than EndTime for run <%d>", run));
			AliWarning("Going to skip this run!");
			continue;
		}

		entries.AddLast(new AliShuttleTriggerDATEEntry(run, startTime, endTime));
		if (lastRun < run) {
			lastRun = run;
		}
		delete aRow;
	}

	delete aResult;
	
	aServer->Shutdown();
	delete aServer;

	entries.SetOwner(1);

	return kTRUE;
}

//______________________________________________________________________________________________
Bool_t AliShuttleTrigger::RetrieveConditionsData(const TObjArray& dateEntries)
{
// Retrieve conditions data for all runs that aren't processed yet

	Bool_t hasError = kFALSE;

	TIter iter(&dateEntries);
	AliShuttleTriggerDATEEntry* anEntry;
	while ((anEntry = (AliShuttleTriggerDATEEntry*) iter.Next())) {
		if(!fShuttle->Process(anEntry->GetRun(),
				anEntry->GetStartTime(),
				anEntry->GetEndTime())) {
			hasError = kTRUE;
		}
	}

	return !hasError;
}

//______________________________________________________________________________________________
Bool_t AliShuttleTrigger::Collect(Int_t run) 
{
	//
	// Collects conditions date for the given run.
	//

	AliInfo(Form("Collecting conditions data for run <%d> ...", run));

	TString whereClause("where run = ");
	whereClause += run;

	Int_t lastRun;
	TObjArray dateEntries;
	if (!RetrieveDATEEntries(whereClause, dateEntries, lastRun)) {
		AliError("Can't retrieve entries from DAQ log book.");
		return kFALSE;
        }

	if (!dateEntries.GetEntriesFast()) {
		AliError(Form("There isn't entry for run <%d> in DAQ log book!",
			run));
		return kFALSE;
	}

	if (dateEntries.GetEntriesFast() > 1) {
		AliError(Form("There is more than one entry for run <%d> in DAQ log book", run));
		return kFALSE;
	}

	if (!RetrieveConditionsData(dateEntries)) {
		AliError("An error occured during conditions data retrieval!");
		return kFALSE;
	}

	return kTRUE;
}

//______________________________________________________________________________________________
Bool_t AliShuttleTrigger::CollectNew() 
{
	//
	// Collects conditions data for all new run written to DAQ LogBook.
	//

  // TODO revise this! last run number is ONLY allowed to be written when run was processed successfully!!!

	AliInfo("Collecting conditions data for new runs ...");

	Int_t lastRun;

	AliCDBEntry* cdbEntry = AliCDBManager::Instance()->GetStorage(AliShuttle::GetLocalURI())
				->Get("/SHUTTLE/SYSTEM/LASTRUN", 0);
	if (cdbEntry) {
		TObject* anObject = cdbEntry->GetObject();
		if (anObject == NULL ||
			anObject->IsA() != AliDCSValue::Class()) {
			AliError("Invalid last run object stored to CDB!");
			return kFALSE;
		}
		AliDCSValue* simpleValue = (AliDCSValue*) anObject;
		lastRun = simpleValue->GetInt();
		delete cdbEntry;
	} else {
		AliWarning("There isn't last run stored! Starting from run 21200");
		lastRun = 21200; // TODO maybe exit here
	}

	AliInfo(Form("Last run number <%d>", lastRun));

	TString whereClause("where run > ");
	whereClause += lastRun;

	Int_t newLastRun;
	TObjArray dateEntries;
	if (!RetrieveDATEEntries(whereClause, dateEntries, newLastRun)) {
		AliError("Can't retrieve entries from DAQ log book.");
		return kFALSE;
	}

	if (newLastRun > lastRun) {
		AliDCSValue lastRunObj(newLastRun, 0);
		AliCDBMetaData metaData;
		AliCDBId cdbID(AliCDBPath("SHUTTLE", "SYSTEM", "LASTRUN"), 0, 0);

		UInt_t result = AliCDBManager::Instance()->GetStorage(AliShuttle::GetLocalURI())
				->Put(&lastRunObj, cdbID, &metaData);

		if (!result) {
			AliError("Can't store last run to CDB!");
			return kFALSE;
		}
	}

	if (!RetrieveConditionsData(dateEntries)) {
		AliError("An error occured during conditions data retrieval!");
		return kFALSE;
	}

	return kTRUE;
}

//______________________________________________________________________________________________
Bool_t AliShuttleTrigger::CollectAll() 
{
	//
	// Collects conditions data for all run written in DAQ LogBook.
	//

	AliInfo("Collecting conditions data for all runs ...");

	Int_t lastRun;
	TObjArray dateEntries;
	if (!RetrieveDATEEntries("", dateEntries, lastRun)) {
		AliError("Can't retrieve entries from DAQ log book.");
		return kFALSE;
	}

	if (!RetrieveConditionsData(dateEntries)) {
		AliError("An error occured during conditions data retrieval!");
		return kFALSE;
	}

	return kTRUE;
}

