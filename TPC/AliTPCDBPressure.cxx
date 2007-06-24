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


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Class to generate TPC atmospheric pressure data base entries             //
//  Author: Haavard Helstrup                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



/**
TTimeStamp startTime(2006,10,18,0,0,0,0,kFALSE)
TTimeStamp endTime(2006,10,19,0,0,0,0,kFALSE)
Int_t run=2546
AliTPCDBPressure db
db->Init(run)
db->MakeCalib("PressureSensor.txt","DCSMap.root",startTime,endTime,firstRun,lastRun)


**/
#include "AliTPCDBPressure.h"

ClassImp(AliTPCDBPressure)

//______________________________________________________________________________________________

AliTPCDBPressure::AliTPCDBPressure(): 
   fFirstRun(0),
   fLastRun(0),
   fPressure(0),
   fStorLoc(0),
   fCalib(0),
   fMetaData(0),
   fConfTree(0)
//
//  standard constructor
//
{}

//______________________________________________________________________________________________

AliTPCDBPressure::AliTPCDBPressure(const AliTPCDBPressure& org):
  TObject(org),
  fFirstRun(org.fFirstRun),
  fLastRun(org.fLastRun),
  fPressure(0),
  fStorLoc(0),
  fCalib(0),
  fMetaData(0),
  fConfTree(0)
{
//
//  Copy constructor
//

 ((AliTPCDBPressure &) org).Copy(*this);
}

//______________________________________________________________________________________________
AliTPCDBPressure::~AliTPCDBPressure(){
//
// destructor
//
   fCalib->Terminate();
   delete fPressure;
   delete fMetaData;
   delete fConfTree;
}

//______________________________________________________________________________________________
AliTPCDBPressure& AliTPCDBPressure::operator= (const AliTPCDBPressure& org )
{
 //
 // assignment operator
 //
 if (&org == this) return *this;

 new (this) AliTPCDBPressure(org);
 return *this;
} 

//______________________________________________________________________________________________
void AliTPCDBPressure::Copy(TObject &c) const
{
  //
  // Copy function
  //

  TObject::Copy(c);
}

//______________________________________________________________________________________________
void AliTPCDBPressure::MakeCalib(const char *list, const char *mapDCS,
                             const TTimeStamp& startTime, 
			     const TTimeStamp& endTime,
			     Int_t firstRun, Int_t lastRun )
{
   // The Terminate() function is the last function to be called during
   // a query. It always runs on the client, it can be used to present
   // the results graphically or save the results to file.

   TClonesArray *arr = ReadList(list);
   AliDCSSensorArray *fPressure = new AliDCSSensorArray(arr);
   fPressure->SetStartTime(startTime);
   fPressure->SetEndTime(endTime);
   TMap* map = SetGraphFile(mapDCS);
   if (map) {
     fPressure->MakeSplineFit(map);
   }
   delete map;
   map=0;
   mapDCS=0;

   SetFirstRun(firstRun);
   SetLastRun(lastRun);   		    

   StoreObject("TPC/Calib/Pressure",fPressure, fMetaData);
}

//______________________________________________________________________________________________
void AliTPCDBPressure::MakeConfig(const char *file, Int_t firstRun, Int_t lastRun )
{
   //
   // Store Configuration file to OCDB
   //

   TTree *tree = ReadListTree(file);
   SetConfTree(tree);
   SetFirstRun(firstRun);
   SetLastRun(lastRun);   		    

   StoreObject("TPC/Config/Pressure",fConfTree, fMetaData);
}



//______________________________________________________________________________________________
AliCDBMetaData* AliTPCDBPressure::CreateMetaObject(const char* objectClassName)
{
  AliCDBMetaData *md1= new AliCDBMetaData(); 
  md1->SetObjectClassName(objectClassName);
  md1->SetResponsible("Haavard Helstrup");
  md1->SetBeamPeriod(2);
  md1->SetAliRootVersion(gSystem->Getenv("ARVERSION"));
  md1->SetComment("Pressure");
  
  return md1;
}

//______________________________________________________________________________________________
void AliTPCDBPressure::StoreObject(const char* cdbPath, TObject* object, AliCDBMetaData* metaData)
{

  AliCDBId id1(cdbPath, fFirstRun, fLastRun); 
  if (fStorLoc) fStorLoc->Put(object, id1, metaData); 
}

//______________________________________________________________________________________________
void AliTPCDBPressure::Init(Int_t run){

   Long64_t longRun;
   
   SetFirstRun(run);
   SetLastRun(run); 
       
   InitDB(run);
   fCalib = AliTPCcalibDB::Instance();    
   longRun=run;
   fCalib->SetRun(longRun);
   fPressure = fCalib->GetPressure();
     
}

//______________________________________________________________________________________________
void AliTPCDBPressure::InitDB(Int_t run)
{ 
   //   Data base generation
   
   char   *CDBpath="local:///afs/cern.ch/alice/tpctest/Calib/";

   fMetaData = CreateMetaObject("AliDCSSensorArray");
   AliCDBManager *man = AliCDBManager::Instance();
   man->SetDefaultStorage("local:///afs/cern.ch/alice/tpctest/AliRoot/HEAD"); 
   man->SetRun(run);
   man->SetSpecificStorage("TPC/*/*","local:///afs/cern.ch/alice/tpctest/Calib");
   AliCDBEntry *config = man->Get("TPC/Config/Pressure");
   if (config) fConfTree = (TTree*)config->GetObject();
   fStorLoc = man->GetStorage(CDBpath);
   if (!fStorLoc)    return;
}


//_____________________________________________________________________________
TMap* AliTPCDBPressure::SetGraphFile(const char *fname)
{
  // 
  // Read DCS maps from file given by fname 
  //
  TFile file(fname);
  TMap * map = (TMap*)file.Get("DCSMap");
  return map;
}

//______________________________________________________________________________________________

TClonesArray * AliTPCDBPressure::ReadList(const char *fname) {
  //
  // read values from ascii file
  //
  TTree* tree = new TTree("pressureConf","pressureConf");
  tree->ReadFile(fname,"");
  TClonesArray *arr = AliDCSSensor::ReadTree(tree);
  return arr;
}

//______________________________________________________________________________________________

TTree * AliTPCDBPressure::ReadListTree(const char *fname) {
  //
  // read values from ascii file
  //
  TTree* tree = new TTree("pressureConf","pressureConf");
  tree->ReadFile(fname,"");
  TClonesArray *arr = AliDCSSensor::ReadTree(tree);
  arr->Delete();
  delete arr;
  return tree;
}


