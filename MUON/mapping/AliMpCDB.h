#ifndef ALI_MP_CDB_H
#define ALI_MP_CDB_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

// $Id$ 

/// \ingroup management
/// \class AliMpCDB
/// \brief Manager class for mapping CDB IO
///
/// The mapping can be loaded in two ways:
/// - from mapping objects stored in the CDB folders Mapping, DDLStore
///   (old way)
/// - from mapping data store in a form of string map in the CDB 
///   folder MappingData (new way, now default)
///
/// To switch between these two ways:
/// - AliMpCDB::SetLoadFromData(Bool_t);
/// 
/// Now it is also possible to regenerate mapping ASCII data
/// from the string map:
/// - AliMpCDB::GenerateMpData();
///
/// \author Ivana Hrivnacova, IPN Orsay

#ifndef ROOT_TObject
#  include <TObject.h>
#endif

class AliMpSegmentation;
class AliMpDDLStore;

class AliMpCDB : public  TObject {

  public:
    // static methods
    //

    static Bool_t LoadMpSegmentation(Bool_t warn = false);
    static Bool_t LoadDDLStore(Bool_t warn = false);
    
    static Bool_t LoadMpSegmentation2(
                    const char* cdbpath = "local://$ALICE_ROOT",
                    Int_t runNumber = 0,
                    Bool_t warn = false);
    static Bool_t LoadDDLStore2(
                    const char* cdbpath = "local://$ALICE_ROOT",
                    Int_t runNumber = 0,
                    Bool_t warn = false);

    static Bool_t WriteMpData();
    static Bool_t WriteMpSegmentation(Bool_t readData = true);
    static Bool_t WriteDDLStore(Bool_t readData= true);
    
    static Bool_t GenerateMpData(
                    const char* cdbpath = "local://$ALICE_ROOT",
                    Int_t runNumber = 0);

    // Switch loading
    static void SetLoadFromData(Bool_t loadFromData);
     
  private:
    /// Not implemented
    AliMpCDB();
    /// Not implemented
    AliMpCDB(const AliMpCDB& rhs);
    /// Not implemented
    AliMpCDB& operator=(const AliMpCDB& rhs);
    

    static TObject*  GetCDBEntryObject(const char* dataPath);
    static TObject*  GetCDBEntryObject(const char* dataPath, 
                                       const char* cdbpath, 
                                       Int_t runNumber);
                                       
    /// option for loading from CDB mapping data or from CDB mapping objects 
    static Bool_t fgLoadFromData;                                       
    
  ClassDef(AliMpCDB,0)  // The factory for building mapping segmentations
};

// inline functions

inline void AliMpCDB::SetLoadFromData(Bool_t loadFromData)
{
/// Set option for loading from objects

  fgLoadFromData = loadFromData;
}  
   
#endif //ALI_MP_CDB_H















