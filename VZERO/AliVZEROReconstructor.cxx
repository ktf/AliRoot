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

///////////////////////////////////////////////////////////////////////////////
///                                                                          //
/// class for VZERO reconstruction                                           //
///                                                                          //
///////////////////////////////////////////////////////////////////////////////


#include "AliVZEROReconstructor.h"

ClassImp(AliVZEROReconstructor)

//_____________________________________________________________________________
AliVZEROReconstructor:: AliVZEROReconstructor()
{
  // Default constructor  
  
  // Get calibration data
  
  fCalibData = GetCalibData(); 
}


//_____________________________________________________________________________
AliVZEROReconstructor& AliVZEROReconstructor::operator = 
  (const AliVZEROReconstructor& /*reconstructor*/)
{
// assignment operator

  Fatal("operator =", "assignment operator not implemented");
  return *this;
}

//_____________________________________________________________________________
AliVZEROReconstructor::~AliVZEROReconstructor()
{
// destructor

}

  
//_____________________________________________________________________________
AliCDBStorage* AliVZEROReconstructor::SetStorage(const char *uri) 
{
  
  Bool_t deleteManager = kFALSE;
  
  AliCDBManager *manager = AliCDBManager::Instance();
  AliCDBStorage *defstorage = manager->GetDefaultStorage();
  
  if(!defstorage || !(defstorage->Contains("VZERO"))){ 
     AliWarning("No default storage set or default storage doesn't contain VZERO!");
     manager->SetDefaultStorage(uri);
     deleteManager = kTRUE;
  }
 
  AliCDBStorage *storage = manager->GetDefaultStorage();

  if(deleteManager){
    AliCDBManager::Instance()->UnsetDefaultStorage();
    defstorage = 0;   // the storage is killed by AliCDBManager::Instance()->Destroy()
  }

  return storage; 
}

//_____________________________________________________________________________
AliVZEROCalibData* AliVZEROReconstructor::GetCalibData() const
{

  // Getting calibration object for VZERO set

  AliCDBEntry  *entry = AliCDBManager::Instance()->Get("VZERO/Calib/Data");
  AliVZEROCalibData *calibdata = (AliVZEROCalibData*) entry->GetObject();

  if (!calibdata)  AliWarning("No calibration data from calibration database !");

  return calibdata;
}

