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
// Class AliMUONGeometrySVMap
// ------------------------------------ 
// As the detection element frame is different from the
// frame of the sensitive volume(s) defined in Geant,
// the sensitive volumes have to be mapped to the detection 
// elements. In the map, fSVMap, the sensitive voolumes are specified
// by the full path in the volume hierarchy, defined as:
//  /volname.copyNo/volName.copyNo1/...
//
// The array of global positions of sensitive volumes fSVPositions
// is included to make easier the verification of the assignements 
// in the fSVMap.
//
// Author: Ivana Hrivnacova, IPN Orsay

#include <Riostream.h>
#include <TGeoMatrix.h>
#include <TObjString.h>

#include "AliMUONGeometrySVMap.h"
#include "AliLog.h"

ClassImp(AliMUONGeometrySVMap)

//______________________________________________________________________________
AliMUONGeometrySVMap::AliMUONGeometrySVMap(Int_t initSize)
 : TObject(),
   fSVMap(),
   fSVPositions(initSize)
{ 
/// Standard constructor
  
  fSVPositions.SetOwner(true);
}

//______________________________________________________________________________
AliMUONGeometrySVMap::AliMUONGeometrySVMap()
 : TObject(),
   fSVMap(),
   fSVPositions()
{
/// Default constructor
}

//______________________________________________________________________________
AliMUONGeometrySVMap::AliMUONGeometrySVMap(const AliMUONGeometrySVMap& rhs)
  : TObject(rhs)
{
/// Protected copy constructor

  AliFatal("Copy constructor is not implemented.");
}

//______________________________________________________________________________
AliMUONGeometrySVMap::~AliMUONGeometrySVMap() 
{
/// Destructor

  fSVPositions.Delete();
}

//______________________________________________________________________________
AliMUONGeometrySVMap& 
AliMUONGeometrySVMap::operator = (const AliMUONGeometrySVMap& rhs) 
{
/// Protected assignement operator

  // check assignement to self
  if (this == &rhs) return *this;

  AliFatal("Assignment operator is not implemented.");
    
  return *this;  
}

//
// private methods
//

//______________________________________________________________________________
const TGeoCombiTrans* 
AliMUONGeometrySVMap::FindByName(const TString& name) const
{
/// Find TGeoCombiTrans in the array of positions by name 

  for (Int_t i=0; i<fSVPositions.GetEntriesFast(); i++) { 
     TGeoCombiTrans* transform = (TGeoCombiTrans*) fSVPositions.At(i);
     if ( transform && TString(transform->GetTitle()) == name )
       return transform;
  }     
       
  return 0;
}  


//
// public methods
//

//______________________________________________________________________________
void AliMUONGeometrySVMap::Add(const TString& volumePath, 
                               Int_t detElemId)
{
/// Add the specified sensitive volume path and the detElemId 
/// to the map
 
  fSVMap.Add(volumePath, detElemId);
}		          
    
//______________________________________________________________________________
void AliMUONGeometrySVMap::AddPosition(const TString& volumePath, 
                              const TGeoTranslation& globalPosition)
{
/// Add global position for the sensitive volume specified by volumePath  
/// in the array of transformations if this volumePath is not yet present. 
 
  TGeoTranslation* newTransform = new TGeoTranslation(globalPosition);
  Int_t detElemId = fSVMap.Get(volumePath);

  TString detElemIdString("");
  detElemIdString += detElemId;

  newTransform->SetName(detElemIdString);
  newTransform->SetTitle(volumePath);
  
  // cout << ".. adding " << volumePath << "  " << detElemId << endl;

  // Add to the map  
  if ( !FindByName(volumePath )) {
  
    newTransform->SetUniqueID(detElemId);
      // Set detector element id as unique id
 
    fSVPositions.Add(newTransform);
  } 
}		      
    
//______________________________________________________________________________
void AliMUONGeometrySVMap::Clear(Option_t* /*option*/)
{
// Clears the sensitive volumes map

  fSVMap.Clear();
}  

//______________________________________________________________________________
void AliMUONGeometrySVMap::ClearPositions()
{
/// Clear the array of transformations

  fSVPositions.Delete();
}  

//______________________________________________________________________________
void AliMUONGeometrySVMap::SortPositions()
{
/// Sort the array of positions by names.

  fSVPositions.Sort(fSVPositions.GetEntriesFast());
}
  
//______________________________________________________________________________
void  AliMUONGeometrySVMap::Print(const char* option) const
{    
/// Print the map of sensitive volumes and detector elements 

  fSVMap.Print(option);
}  

//______________________________________________________________________________
void  AliMUONGeometrySVMap::PrintPositions() const
{
/// Print the sensitive volumes global positions

  for (Int_t i=0; i<fSVPositions.GetEntriesFast(); i++) {
    
    TGeoTranslation* matrix = (TGeoTranslation*)fSVPositions.At(i);

    cout << "DetElemId: " << matrix->GetUniqueID();
    cout << "  name: " << matrix->GetTitle() << endl;

    const double* translation = matrix->GetTranslation();
    cout << "   translation: "
#if defined (__DECCXX)
         << translation[0] << ", " 
         << translation[1] << ", "
         << translation[2] << endl;
#else
         << std::fixed
         << std::setw(7) << std::setprecision(4) << translation[0] << ", " 
         << std::setw(7) << std::setprecision(4) << translation[1] << ", "
         << std::setw(7) << std::setprecision(4) << translation[2] << endl;
#endif
  }
}     

//______________________________________________________________________________
void  AliMUONGeometrySVMap::WriteMap(ofstream& out) const
{    
/// Print the map of sensitive volumes and detector elements 
/// into specified stream

  fSVMap.Print("SV", out);
}  

//______________________________________________________________________________
Int_t  AliMUONGeometrySVMap::GetDetElemId(const TString& volumePath) const
{
/// Return detection element Id for the sensitive volume specified by path

  return fSVMap.Get(volumePath);
}  
