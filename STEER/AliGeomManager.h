#ifndef ALI_GEOM_MANAGER_H
#define ALI_GEOM_MANAGER_H

//
// Class for interfacing to the geometry; it also builds and manages two
// look-up tables for fast access to volumes :
// 1) the look-up table mapping unique volume ids to symbolic volume names
// 2) the look-up table mapping unique volume ids to TGeoPNEntries
// this allows to access directly the functionality of the physical node
// associated to a given alignable volume by means of its index
//

#include <TObject.h>
#include <TGeoManager.h>

class TObjArray;

class AliAlignObj;
class AliCDBId;
class AliCDBParam;

class AliGeomManager: public TObject {

public:
  enum ELayerID{kInvalidLayer=0,
		kFirstLayer=1,
		kSPD1=1, kSPD2=2,
		kSDD1=3, kSDD2=4,
		kSSD1=5, kSSD2=6,
		kTPC1=7, kTPC2=8,
		kTRD1=9, kTRD2=10, kTRD3=11, kTRD4=12, kTRD5=13, kTRD6=14,
		kTOF=15,
		kPHOS1=16, kPHOS2=17,
		kHMPID=18,
		kMUON=19,
		kLastLayer=20};


  static Int_t       LayerSize(Int_t layerId);
  static const char* LayerName(Int_t layerId);
  static UShort_t LayerToVolUID(ELayerID layerId, Int_t modId);
  static UShort_t LayerToVolUID(Int_t    layerId, Int_t modId);
  static ELayerID VolUIDToLayer(UShort_t voluid, Int_t &modId);
  static ELayerID VolUIDToLayer(UShort_t voluid);
  static UShort_t LayerToVolUIDSafe(ELayerID layerId, Int_t modId);
  static UShort_t LayerToVolUIDSafe(Int_t    layerId, Int_t modId);
  static ELayerID VolUIDToLayerSafe(UShort_t voluid, Int_t &modId);
  static ELayerID VolUIDToLayerSafe(UShort_t voluid);

  static const char* SymName(UShort_t voluid);
  static const char* SymName(ELayerID layerId, Int_t modId);

  static TGeoPNEntry* GetPNEntry(Int_t index);
  static TGeoPNEntry* GetPNEntry(UShort_t index);
  static TGeoPNEntry* GetPNEntry(ELayerID layerId, Int_t modId);

  static Bool_t   GetFromGeometry(const char *symname, AliAlignObj &alobj);
  static AliAlignObj* GetAlignObj(UShort_t voluid);
  static AliAlignObj* GetAlignObj(ELayerID layerId, Int_t modId);

  static TGeoHMatrix* GetMatrix(TGeoPNEntry* pne);
  static TGeoHMatrix* GetMatrix(Int_t index);
  static TGeoHMatrix* GetMatrix(const char *symname);
  static Bool_t GetTranslation(Int_t index, Double_t t[3]);
  static Bool_t GetRotation(Int_t index, Double_t r[9]);

  static Bool_t GetOrigGlobalMatrix(Int_t index, TGeoHMatrix &m);
  static Bool_t GetOrigGlobalMatrix(const char *symname, TGeoHMatrix &m);
  static Bool_t GetOrigTranslation(Int_t index, Double_t t[3]);
  static Bool_t GetOrigRotation(Int_t index, Double_t r[9]);

  static const TGeoHMatrix* GetTracking2LocalMatrix(Int_t index);
  static Bool_t GetTrackingMatrix(Int_t index, TGeoHMatrix &m);

  Bool_t         ApplyAlignObjsToGeom(TObjArray* alObjArray);

  Bool_t         ApplyAlignObjsToGeom(const char* fileName,
				      const char* clArrayName);
  Bool_t         ApplyAlignObjsToGeom(AliCDBParam* param,
				      AliCDBId& Id);
  Bool_t         ApplyAlignObjsToGeom(const char* uri, const char* path,
				      Int_t runnum, Int_t version,
				      Int_t sversion);
  Bool_t         ApplyAlignObjsToGeom(const char* detName, Int_t runnum, Int_t version,
				      Int_t sversion);

  Bool_t         ApplyAlignObjsFromCDB(const char* AlDetsList);
  Bool_t         LoadAlignObjsFromCDBSingleDet(const char* detName);

  ~AliGeomManager();
  static AliGeomManager* Instance();


 protected:

  static void        InitAlignObjFromGeometry();
  static void        InitSymNamesLUT();
  static void        InitPNEntriesLUT();

  static Int_t       fgLayerSize[kLastLayer - kFirstLayer]; // Size of layers
  static const char* fgLayerName[kLastLayer - kFirstLayer]; // Name of layers
  static TString*    fgSymName[kLastLayer - kFirstLayer]; // Symbolic volume names
  static TGeoPNEntry** fgPNEntry[kLastLayer - kFirstLayer]; // TGeoPNEntries
  static AliAlignObj** fgAlignObjs[kLastLayer - kFirstLayer]; // Alignment objects

 private:
  AliGeomManager();
  AliGeomManager(const AliGeomManager&);
  AliGeomManager& operator=(const AliGeomManager&);

  static void        ReactIfChangedGeom();
  static Bool_t      HasGeomChanged(){return fgGeometry!=gGeoManager;} 
  static TGeoManager* fgGeometry;



  static AliGeomManager* fgInstance; // the AliGeomManager singleton instance

  TObjArray* 	 fAlignObjArray;      // array with the alignment objects to be applied to the geometry

  void Init();

  ClassDef(AliGeomManager, 0);
};

#endif
