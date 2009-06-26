#include "AliVZEROPreprocessor.h"
#include "AliVZEROCalibData.h"
#include "AliVZEROTriggerData.h"
#include "AliCDBMetaData.h"
#include "AliCDBEntry.h"
#include "AliDCSValue.h"
#include "AliLog.h"
#include <TFile.h>
#include <TTimeStamp.h>
#include <TObjString.h>
#include <TSystem.h>
#include <TList.h>

//
// This class is  a simple preprocessor for V0.
//

ClassImp(AliVZEROPreprocessor)

//______________________________________________________________________________________________
AliVZEROPreprocessor::AliVZEROPreprocessor(AliShuttleInterface* shuttle) :
	AliPreprocessor("V00", shuttle),
	fData(0),
	fFEEData(0)
 
{
  // constructor  
  
  AddRunType("STANDALONE");
  AddRunType("PHYSICS");
    
}

//______________________________________________________________________________________________
AliVZEROPreprocessor::~AliVZEROPreprocessor()
{
  // destructor
	delete fFEEData;
	delete fData;
	
}

//______________________________________________________________________________________________
void AliVZEROPreprocessor::Initialize(Int_t run, UInt_t startTime,
	UInt_t endTime)
{
  // Creates AliVZERODataDCS object

   AliPreprocessor::Initialize(run, startTime, endTime);
  
   Log(Form("\n\tRun %d \n\tStartTime %s \n\tEndTime %s", run,
		TTimeStamp(startTime).AsString(),
		TTimeStamp(endTime).AsString()));

   fRun       = run;
   // fStartTime = startTime;
   // fEndTime   = endTime;
   fStartTime = GetStartTimeDCSQuery ();
   fEndTime   = GetEndTimeDCSQuery ();

	fData      = new AliVZERODataDCS(fRun, fStartTime, fEndTime);
	fFEEData   = new AliVZERODataFEE(fRun, fStartTime, fEndTime);		
   
}

//______________________________________________________________________________________________
UInt_t AliVZEROPreprocessor::Process(TMap* dcsAliasMap)
{
  // Fills data retrieved from DCS and DAQ into a AliVZEROCalibData object and 
  // stores it into CalibrationDB


  // *** GET RUN TYPE ***
  TString runType = GetRunType();


  // *** REFERENCE DATA *** 
  
  TString fileName; 
  AliVZEROCalibData *calibData = new AliVZEROCalibData();
  
  // *************** HV From DCS ******************
  // Fills data into a AliVZERODataDCS object
  if(!dcsAliasMap) return 1;

 	// The Processing of the DCS input data is forwarded to AliVZERODataDCS

	fData->ProcessData(*dcsAliasMap);
	//fData->Draw(""); 		// Draws the HV values as a function of time
	//dcsAliasMap->Print("");	// Prints out the HV values

	// Writes VZERO PMs HV values into VZERO calibration object
	calibData->SetMeanHV(fData->GetMeanHV());
	calibData->SetWidthHV(fData->GetWidthHV());
    
   // *************** From DAQ ******************
   
	TString SourcesId = "V00da_results";

	TList* sourceList = GetFileSources(kDAQ, SourcesId.Data());
  	if (!sourceList)  {
		Log(Form("No sources found for id %s", SourcesId.Data()));      		
      		return 1; }
	Log(Form("The following sources produced files with the id %s",SourcesId.Data()));
	sourceList->Print();    

  	TIter iter(sourceList);
  	TObjString *source = 0;
		
	while((source=dynamic_cast<TObjString*> (iter.Next()))){
  		fileName = GetFile(kDAQ, SourcesId.Data(), source->GetName());
  		if (fileName.Length() > 0)
    		Log(Form("Got the file %s, now we can extract some values.", fileName.Data()));
		FILE *file;
		if((file = fopen(fileName.Data(),"r")) == NULL){
            	                   Log(Form("Cannot open file %s",fileName.Data()));
	    	  	           return 1;}
		Float_t PEDmean[128], PEDsigma[128], ADCmean[128], ADCsigma[128] ;
		for(Int_t j=0; j<128; j++)fscanf(file,"%f %f %f %f",
			                  &PEDmean[j], &PEDsigma[j], &ADCmean[j], &ADCsigma[j]);
		fclose(file);
	    	
		calibData->SetPedestal(PEDmean);
		calibData->SetSigma(PEDsigma);			
		calibData->SetGain(ADCmean); 
		calibData->SetADCsigma(ADCsigma);
		}				

	delete source;      
  
  // Check that everything was properly transmitted

//   for(Int_t j=0; j<128; j++){printf("Pedestal[%d] -> %f \n",j,calibData->GetPedestal(j));}
//   for(Int_t j=0; j<128; j++){printf("PedSigma[%d] -> %f \n",j,calibData->GetSigma(j));}
//   for(Int_t j=0; j<128; j++){printf("Gain[%d] -> %f \n",j,calibData->GetGain(j));}
//   for(Int_t j=0; j<128; j++){printf("ADCsigma[%d] -> %f \n",j,calibData->GetADCsigma(j));}
//   for(Int_t j=0; j<64; j++){printf("MeanHV[%d] -> %f \n",j,calibData->GetMeanHV(j));}
//   for(Int_t j=0; j<64; j++){printf("WidthHV[%d] -> %f \n",j,calibData->GetWidthHV(j));}
  
  // Now we store the VZERO Calibration Object into CalibrationDB

  Bool_t resECal=kTRUE;
  
  Bool_t result = 0;
//  if(sourceList && sourceList->GetEntries()>0)
//  {
  AliCDBMetaData metaData;
  metaData.SetBeamPeriod(0);
  metaData.SetResponsible("Brigitte Cheynis");
  metaData.SetComment("This preprocessor fills an AliVZEROCalibData object");

  resECal = Store("Calib", "Data", calibData, &metaData, 0, kTRUE);
//  }
  if(resECal==kFALSE ) result = 1;
  

  delete calibData;
  delete sourceList; 

 // -----------------------------------------------------------------------
 // Retrieve Front End Electronics Parameters from the DCS
 // -----------------------------------------------------------------------
	AliVZEROTriggerData *triggerData = new AliVZEROTriggerData();

 	// The processing of the DCS input data is forwarded to AliVZERODataFEE
	fFEEData->ProcessData(*dcsAliasMap);

	// Writes VZERO FEE parameters values into VZERO  Trigger parametrization object
	triggerData->FillData(fFEEData);

	// Stores the VZERO Trigger Object into CalibrationDB
	
	resECal=kTRUE;
	
	result = 0;
	metaData.SetBeamPeriod(0);
	metaData.SetResponsible("Brigitte Cheynis");
	metaData.SetComment("This preprocessor fills an AliVZEROTriggerData object");
	
	resECal = Store("Trigger", "Data", triggerData, &metaData, 0, kTRUE);
	if(resECal==kFALSE ) result = 1;
	
	
  return result;
}

