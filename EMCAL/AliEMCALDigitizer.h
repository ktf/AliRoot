#ifndef ALIEMCALDigitizer_H
#define ALIEMCALDigitizer_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

//_________________________________________________________________________
//  Task Class for making Digits in EMCAL      
//                  
//*-- Author: Sahal Yacoob (LBL)
// based on : AliPHOSDigit
// July 2003 Yves Schutz : NewIO 
//_________________________________________________________________________ 


// --- ROOT system ---
#include "TObjString.h"
class TArrayI ;
class TClonesArray ; 

// --- Standard library ---

// --- AliRoot header files ---
#include "AliDigitizer.h"
#include "AliConfig.h"
class AliEMCALSDigitizer ;
class AliRunDigitizer ;

class AliEMCALDigitizer: public AliDigitizer {

public:
  AliEMCALDigitizer() ;          // ctor
  AliEMCALDigitizer(const TString alirunFileNameFile, const TString eventFolderName = AliConfig::fgkDefaultEventFolderName) ;  
  AliEMCALDigitizer(const AliEMCALDigitizer & dtizer) ;
  AliEMCALDigitizer(AliRunDigitizer * manager) ;
  virtual ~AliEMCALDigitizer() ;       

  void    Digitize(const Int_t event);          // Make Digits from SDigits stored in fSDigits
  void    Exec(Option_t *option);               // Supervising method

  const Float_t GetDigitThreshold() const { return fDigitThreshold;}
  const Float_t GetPedestal()       const { return fPedestal; }
  const Float_t GetPinNoise()       const { return fPinNoise;}
  const Float_t GetSlope()          const { return fSlope; }
  const Float_t GetTimeResolution() const { return fTimeResolution ; }
  const Float_t GetECAchannel()     const { return fADCchannelEC ; }
  const Float_t GetECApedestal()    const { return fADCpedestalEC ; }
  const Float_t GetHCAchannel()     const { return fADCchannelHC ; }
  const Float_t GetHCApedestal()    const { return fADCpedestalHC ; }
  const Float_t GetPREchannel()     const { return fADCchannelPRE ; }
  const Float_t GetPREpedestal()    const { return fADCpedestalPRE ; }

  void    SetDigitThreshold(Float_t EMCThreshold)  {fDigitThreshold = EMCThreshold;}
  void    SetPinNoise(Float_t PinNoise )         {fPinNoise = PinNoise;}

  //General
  const Int_t   GetDigitsInRun()  const { return fDigitsInRun; } 
  void  MixWith(const TString alirunFileName, 
		const TString eventFolderName = AliConfig::fgkDefaultEventFolderName) ; // Add another one file to mix
  void  Print()const ;
 
  AliEMCALDigitizer & operator = (const AliEMCALDigitizer & /*rvalue*/)  {
    // assignement operator requested by coding convention but not needed
   Fatal("operator =", "not implemented") ;  
   return *this ; 
  }

private:

  Bool_t  Init();                   
  void    InitParameters() ; 
  void    PrintDigits(Option_t * option) ;
  void    Unload() ; 
  void    WriteDigits() ;         // Writes Digits the current event
  Float_t TimeOfNoise(void) ;     // Calculate time signal generated by noise

  //Calculate the time of crossing of the threshold by front edge
  Float_t FrontEdgeTime(TClonesArray * ticks) ;
  Int_t   DigitizeEnergy(Float_t energy, Int_t absId) ;

private:
  
  Bool_t  fDefaultInit;           //! Says if the task was created by defaut ctor (only parameters are initialized)
  Int_t   fDigitsInRun ;          //! Total number of digits in one run
  Bool_t  fInit ;                 //! To avoid overwriting existing files

  Int_t   fInput ;                // Number of files to merge
  TString * fInputFileNames ;     //[fInput] List of file names to merge 
  TString * fEventNames ;         //[fInput] List of event names to merge

  Float_t fDigitThreshold  ;      // Threshold for storing digits in EMC
  Int_t   fMeanPhotonElectron ;   // number of photon electrons per GeV deposited energy 
  Float_t fPedestal ;             // Calibration parameters 
  Float_t fSlope ;                // read from SDigitizer
  Float_t fPinNoise ;             // Electronics noise in EMC
  Float_t fTimeResolution ;       // Time resolution of FEE electronics
  Float_t fTimeThreshold ;        // Threshold to start timing for given crystall
  Float_t fTimeSignalLength ;     // Length of the timing signal 
  Float_t fADCchannelEC ;         // width of one ADC channel in EC section (GeV)
  Float_t fADCpedestalEC ;        //
  Int_t   fNADCEC ;               // number of channels in EC section ADC
  Float_t fADCchannelHC ;         // width of one ADC channel in HC section (GeV)
  Float_t fADCpedestalHC ;        //
  Int_t   fNADCHC ;               // number of channels in HC section ADC
  Float_t fADCchannelPRE ;        // width of one ADC channel in PRE section (GeV)
  Float_t fADCpedestalPRE ;       // 
  Int_t fNADCPRE ;                // number of channels in PRE section ADC

  TString fEventFolderName;         // skowron: name of EFN to read data from in stand alone mode

  ClassDef(AliEMCALDigitizer,4)  // description 

};


#endif // AliEMCALDigitizer_H
