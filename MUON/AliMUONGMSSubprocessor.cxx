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

// $Id$
//
// Class AliMUONGMSSubprocessor
// -----------------------------
// The shuttle subprocessor for GMS data
// Author: Ivana Hrivnacova, IPN Orsay
// 16/09/2006

#include "AliMUONGMSSubprocessor.h"
#include "AliMUONPreprocessor.h"

#include "AliCDBMetaData.h"
#include "AliLog.h"

#include <TTimeStamp.h>
#include <TFile.h>
#include <TClonesArray.h>
#include <TObjString.h>
#include <Riostream.h>

ClassImp(AliMUONGMSSubprocessor)

const Int_t    AliMUONGMSSubprocessor::fgkSystem = AliPreprocessor::kDAQ;
const TString  AliMUONGMSSubprocessor::fgkDataId = "GMS";
const TString  AliMUONGMSSubprocessor::fgkMatrixArrayName = "GMSarray";

//______________________________________________________________________________
AliMUONGMSSubprocessor::AliMUONGMSSubprocessor(AliMUONPreprocessor* master) 
  : AliMUONVSubprocessor(master, "GMS", "Upload GMS matrices to OCDB"),
    fTransformer(true)
{
/// Constructor
  fTransformer.ReadGeometryData("volpath.dat", "transform.dat");
}

//______________________________________________________________________________
AliMUONGMSSubprocessor::~AliMUONGMSSubprocessor()
{
/// Destructor
}


//
// private methods
//


//______________________________________________________________________________
UInt_t AliMUONGMSSubprocessor::ProcessFile(const TString& fileName)
{
/// Convert TGeoHMatrix to AliAlignObjMatrix and fill them into AliTestDataDCS object

  AliInfoStream() << "Processing file " << fileName << endl;

  // Open root file
  TFile f(fileName.Data());
  if ( ! f.IsOpen() ) {
    AliErrorStream() << "Cannot open file " << fileName << endl;
    return 1;
  }  
  
  // Get array with matrices
  TClonesArray* array = (TClonesArray*)f.Get(fgkMatrixArrayName);
  if ( ! array ) {
    AliErrorStream() << "TClonesArray not found in file " 
       << fileName << endl;
    return 1;
  }  
  
  // Convert matrices into Alice alignment objects
  for (Int_t i=0; i<array->GetEntriesFast(); i++ ) {
    TGeoHMatrix* matrix = (TGeoHMatrix*)array->At(i);
    fTransformer.AddMisAlignModule(matrix->GetUniqueID(), *matrix);
  }  
  TObject* data = const_cast< TClonesArray*>(fTransformer.GetMisAlignmentData());
  
  //Now we have to store the final CDB file
  AliCDBMetaData metaData;
  metaData.SetBeamPeriod(0);
  metaData.SetResponsible("");
  metaData.SetComment("This preprocessor fills GMS alignment objects.");

  UInt_t result = Master()->Store("SHUTTLE", "GMS", data, &metaData, 0, 0);

  // Clear MisAlignArray in transformer
  fTransformer.ClearMisAlignmentData();

  return result;
}  

//
// public methods
//


//______________________________________________________________________________
UInt_t AliMUONGMSSubprocessor::Process(TMap* /*dcsAliasMap*/)
{
/// Process GMS alignment files

  UInt_t result = 0;
  TList* sources = Master()->GetFileSources(fgkSystem, fgkDataId);
  TIter next(sources);
  TObjString* o(0x0);
  while ( ( o = static_cast<TObjString*>(next()) ) ) {
    TString fileName(Master()->GetFile(fgkSystem, fgkDataId, o->GetName()));
    result += ProcessFile(fileName);
  }
  delete sources;

  return result;
}

