#ifndef ALIMUON_H
#define ALIMUON_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */
// Revision of includes 07/05/2004

/// \ingroup sim
/// \class AliMUON
/// \brief AliDetector class for MUON subsystem providing
/// simulation data management 
 

#include <TObjArray.h>

#include "AliDetector.h"
#include "AliMUONChamber.h"
#include "AliMUONTrigger.h"

#include <TVectorfwd.h>

class TFile;
class TTree;

class AliLoader;
class AliMUONResponse;
class AliMUONSegmentation;
class AliMUONHit;
class AliMUONGeometry;
class AliMUONGeometryTransformer;
class AliMUONGeometryBuilder;
class AliMUONRawWriter;
class AliMUONVGeometryBuilder;
class AliESD;
class AliMUONDigitMaker;
class AliMUONVHitStore;

class AliMUON : public  AliDetector 
{
  public:
    AliMUON();
    AliMUON(const char* name, const char* title);
    virtual ~AliMUON();
   
    // Geometry
    void AddGeometryBuilder(AliMUONVGeometryBuilder* geomBuilder);
    void ResetGeometryBuilder();
    
    virtual void   BuildGeometry();
    
                   /// Return geometry builder
    AliMUONGeometryBuilder*            GetGeometryBuilder() const {return fGeometryBuilder;}
    const AliMUONGeometryTransformer*  GetGeometryTransformer() const;
                   /// Return segmentation

    // MUONLoader definition
    virtual AliLoader* MakeLoader(const char* topfoldername); //builds standard getter (AliLoader type)

    virtual void   SetTreeAddress();
                  
                   /// Set split level for making branches in outfiles
    virtual void   SetSplitLevel(Int_t SplitLevel)     {fSplitLevel=SplitLevel;}

    // Digitisation 
    virtual AliDigitizer* CreateDigitizer(AliRunDigitizer* manager) const;
    virtual void   SDigits2Digits();      
    virtual void   Hits2SDigits();
    virtual void   Digits2Raw();
    virtual Bool_t Raw2SDigits(AliRawReader* rawReader);

    // Trigger
                   /// Create trigger
    virtual AliTriggerDetector* CreateTriggerDetector() const
       { return new AliMUONTrigger(); }
    

    // Configuration Methods (per station id)
    //
    // Set Signal Generation Parameters
    virtual void   SetSigmaIntegration(Int_t id, Float_t p1);
    virtual void   SetChargeSlope(Int_t id, Float_t p1);
    virtual void   SetChargeSpread(Int_t id, Float_t p1, Float_t p2);
    virtual void   SetMaxAdc(Int_t id, Int_t p1);
    // Set Response Model
    virtual void   SetResponseModel(Int_t id, const AliMUONResponse& response);

    // Set Stepping Parameters
    virtual void   SetMaxStepGas(Float_t p1);
    virtual void   SetMaxStepAlu(Float_t p1);
    virtual void   SetMaxDestepGas(Float_t p1);
    virtual void   SetMaxDestepAlu(Float_t p1);
   
    // Get Stepping Parameters
    virtual Float_t  GetMaxStepGas() const;
    virtual Float_t  GetMaxStepAlu() const;
    virtual Float_t  GetMaxDestepGas() const;
    virtual Float_t  GetMaxDestepAlu() const;
    
    // Set alignement option
    virtual void  SetAlign(Bool_t align = true);
    virtual void  SetAlign(const TString& fileName, Bool_t align = true);

                   /// Set scaler event for trigger
    virtual void  SetTriggerScalerEvent(Bool_t scaler = true){fTriggerScalerEvent = scaler;}

                  /// Set trigger response version
    virtual void  SetTriggerResponseV1(Bool_t trigResV1 = false)
	{ fTriggerResponseV1 = trigResV1; }
    virtual Bool_t GetTriggerResponseV1() const;
                  /// Set trigger coinc44
    virtual void  SetTriggerCoinc44(Int_t trigCoinc44 = 0)
	{ fTriggerCoinc44 = trigCoinc44; }
    virtual Int_t GetTriggerCoinc44() const;
                 /// Set trigger chamber efficiency by cells
    virtual void SetTriggerEffCells(Bool_t trigEffCells = false)
	{ fTriggerEffCells = trigEffCells; }
    virtual Bool_t GetTriggerEffCells() const;
                 /// Set off generation of noisy digits
    virtual void SetDigitizerWithNoise(Int_t digitizerWithNoise)
        { fDigitizerWithNoise = digitizerWithNoise; }
    virtual Int_t GetDigitizerWithNoise() const; 
    
    // Getters
                  /// Return reference to Chamber \a id
    virtual AliMUONChamber& Chamber(Int_t id)
      {return *((AliMUONChamber *) (*fChambers)[id]);}
                  /// Return reference to New Circuit \a id 

    virtual void MakeBranch(Option_t* opt=" ");
    virtual void ResetHits();
    
  protected:
    /// Not implemented
    AliMUON(const AliMUON& rMUON);
    /// Not implemented
    AliMUON& operator = (const AliMUON& rhs);

    const AliMUONGeometry* GetGeometry() const;

    Int_t                 fNCh;                ///< Number of chambers   
    Int_t                 fNTrackingCh;        ///< Number of tracking chambers*
    Int_t                 fSplitLevel;         ///< Splitlevel when making branches in outfiles.
    TObjArray*            fChambers;           ///< List of Tracking Chambers
    AliMUONGeometryBuilder*  fGeometryBuilder; ///< Geometry builder 
   
    //
    Bool_t   fAccCut;         ///<Transport acceptance cut
    Float_t  fAccMin;         ///<Minimum acceptance cut used during transport
    Float_t  fAccMax;         ///<Minimum acceptance cut used during transport
    //  
    //  Stepping Parameters
    Float_t fMaxStepGas;      ///< Maximum step size inside the chamber gas
    Float_t fMaxStepAlu;      ///< Maximum step size inside the chamber aluminum
    Float_t fMaxDestepGas;    ///< Maximum relative energy loss in gas
    Float_t fMaxDestepAlu;    ///< Maximum relative energy loss in aluminum
    
    // Pad Iterator
    Int_t fMaxIterPad;        ///< Maximum pad index
    Int_t fCurIterPad;        ///< Current pad index
   
    // Options
    Bool_t fTriggerScalerEvent; ///< Flag to generates scaler event
    Bool_t fTriggerResponseV1;  ///< Flag to select TriggerResponseV1
    Int_t  fTriggerCoinc44;     ///< Flag to select TriggerCoinc44 
    Bool_t fTriggerEffCells;    ///< Flag to select TriggerEffCells
    Int_t fDigitizerWithNoise; ///< Flag to switch on/off generation of noisy digits
    
    AliMUONRawWriter* fRawWriter; //!< Raw data writer
    
    AliMUONDigitMaker* fDigitMaker; //!< pointer to the digit maker class

    AliMUONVHitStore* fHitStore; //!< container of hits
    
    ClassDef(AliMUON,15)  // MUON Detector base class
};
#endif

