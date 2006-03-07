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
#include "AliCDBStorage.h"
#include "AliCDBEntry.h"

#include "AliSimpleValue.h"
#include "AliShuttleConfig.h"
#include "AliShuttle.h"
#include "DATENotifier.h"

ClassImp(TerminateSignalHandler)
Bool_t TerminateSignalHandler::Notify() {

	AliInfo("Terminate signal received ...");
	fTrigger->Terminate();

	return kTRUE;
}

ClassImp(AliShuttleTrigger)
AliShuttleTrigger::AliShuttleTrigger(const AliShuttleConfig* config,
		AliCDBStorage* storage, UInt_t timeout, Int_t retries):
	fConfig(config), fStorage(storage), fShuttle(NULL),
	fNotified(kFALSE), fTerminate(kFALSE), fCondition(&fMutex),
	fQuitSignalHandler(this, kSigQuit), 
	fInterruptSignalHandler(this, kSigInterrupt)
{
	//
	// config - pointer to the AliShuttleConfig object which represents
	// the configuration
	// storage - pointer to AliCDBStorage for the undelying CDBStorage
	//

	fShuttle = new AliShuttle(config, storage, timeout, retries);

	gSystem->AddSignalHandler(&fQuitSignalHandler);
	gSystem->AddSignalHandler(&fInterruptSignalHandler);
}

AliShuttleTrigger::~AliShuttleTrigger() {

	gSystem->RemoveSignalHandler(&fQuitSignalHandler);
	gSystem->RemoveSignalHandler(&fInterruptSignalHandler);

	delete fShuttle;
}

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

void AliShuttleTrigger::Terminate() {
	//
	// Stop triggers listen mode and exist from Run()
	// Usually called automaticly by TerminateSignalHandler.
	//

	fTerminate = kTRUE;
	fCondition.Signal();
}

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

Bool_t AliShuttleTrigger::RetrieveDATEEntries(const char* whereClause,
		TObjArray& entries, Int_t& lastRun) {

	TString sqlQuery;
	sqlQuery += "select run, time_start, time_end from logbook ";
	sqlQuery += whereClause;
	sqlQuery += " order by run";

	TSQLServer* aServer;
	aServer = TSQLServer::Connect(fConfig->GetLogBookURI(),
			fConfig->GetLogBookUser(),
			fConfig->GetLogBookPassword());
	if (!aServer) {
		AliError("Can't establish connection to DAQ log book DB!");
		return kFALSE;
	}

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

		entries.AddLast(new DATEEntry(run, startTime, endTime));
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

Bool_t AliShuttleTrigger::RetrieveConditionsData(const TObjArray& dateEntries) {

	Bool_t hasError = kFALSE;

	TIter iter(&dateEntries);
	DATEEntry* anEntry;
	while ((anEntry = (DATEEntry*) iter.Next())) {
		if(!fShuttle->Process(anEntry->GetRun(), 
				anEntry->GetStartTime(),
				anEntry->GetEndTime())) {
			hasError = kTRUE;
		}	
	}

	return !hasError;
}

Bool_t AliShuttleTrigger::Collect(Int_t run) {
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

Bool_t AliShuttleTrigger::CollectNew() {
	//
	// Collects conditions data for all new run written to DAQ LogBook.
	//

	AliInfo("Collecting conditions data for new runs ...");

	Int_t lastRun;

	AliCDBEntry* cdbEntry = fStorage->Get("/SHUTTLE/SYSTEM/LASTRUN", 0);
	if (cdbEntry) {
		TObject* anObject = cdbEntry->GetObject();
		if (anObject == NULL || 
			anObject->IsA() != AliSimpleValue::Class()) {
			AliError("Invalid last run object stored to CDB!");
			return kFALSE;
		}
		AliSimpleValue* simpleValue = (AliSimpleValue*) anObject;
		lastRun = simpleValue->GetInt();
		delete cdbEntry;
	} else {
		AliWarning("There isn't last run stored! Starting from run 0");
		lastRun = -1;
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
		AliSimpleValue lastRunObj(newLastRun);
		AliCDBMetaData metaData;
		AliCDBId cdbID(AliCDBPath("SHUTTLE", "SYSTEM", "LASTRUN"), 0, 0);

		if (!fStorage->Put(&lastRunObj, cdbID, &metaData)) {
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

Bool_t AliShuttleTrigger::CollectAll() {
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

