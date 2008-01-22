#ifndef ALIZDCRECONSTRUCTOR_H
#define ALIZDCRECONSTRUCTOR_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// class for ZDC reconstruction                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "AliReconstructor.h"
#include "AliCDBManager.h"
#include "AliCDBStorage.h"
#include "AliZDCPedestals.h"
#include "AliZDCCalib.h"
#include "AliZDCRecParam.h"
#include "AliLog.h"

class TF1;
class AliLoader;

class AliZDCReconstructor: public AliReconstructor {
public:
  AliZDCReconstructor();
  virtual ~AliZDCReconstructor();

  virtual Bool_t HasDigitConversion() const {return kFALSE;};

  virtual void Reconstruct(TTree* digitsTree, TTree* clustersTree) const; 
  virtual void Reconstruct(AliRawReader* rawReader, TTree* clustersTree) const;

  virtual void FillESD(TTree* /*digitsTree*/, TTree* clustersTree, AliESDEvent* esd) const 
  	        {FillZDCintoESD(clustersTree,esd);}
  virtual void FillESD(AliRawReader* /*rawReader*/, TTree* clustersTree, AliESDEvent* esd) const 
  	        {FillZDCintoESD(clustersTree,esd);}
    
  AliCDBStorage   *SetStorage(const char* uri);
  AliZDCPedestals *GetPedData() const; 
  AliZDCCalib     *GetECalibData() const; 
  AliZDCRecParam  *GetRecParams() const; 
  
private:
  AliZDCReconstructor(const AliZDCReconstructor&);
  AliZDCReconstructor& operator =(const AliZDCReconstructor&);

  void   ReconstructEvent(TTree *clustersTree, 
  	    Float_t* ZN1ADCCorr, Float_t* ZP1ADCCorr, Float_t* ZN2ADCCorr, Float_t* ZP2ADCCorr,
	    Float_t* ZEM1ADCCorr, Float_t* ZEM2ADCCorr, Float_t* PMRef1, Float_t* PMRef2) const;
  void   FillZDCintoESD(TTree *clustersTree, AliESDEvent*esd) const;

  TF1*   fZNCen;     //! Nspectator n true vs. EZN
  TF1*   fZNPer;     //! Nspectator n true vs. EZN
  TF1*   fZPCen;     //! Nspectator p true vs. EZP
  TF1*   fZPPer;     //! Nspectator p true vs. EZP
  TF1*   fZDCCen;    //! Nspectators true vs. EZDC
  TF1*   fZDCPer;    //! Nspectators true vs. EZDC
  TF1*   fbCen;      //! b vs. EZDC
  TF1*   fbPer;      //! b vs. EZDC
  TF1*   fZEMn;      //! Nspectators n from ZEM energy
  TF1*   fZEMp;      //! Nspectators p from ZEM energy
  TF1*   fZEMsp;     //! Nspectators from ZEM energy
  TF1*   fZEMb;      //! b from ZEM energy
  
  AliZDCPedestals *fPedData; 	//! pedestal calibration data
  AliZDCCalib     *fECalibData; //! energy and equalization calibration data
  AliZDCRecParam  *fRecParam; 	//! reconstruction parameters

  ClassDef(AliZDCReconstructor, 3)   // class for the ZDC reconstruction
};

#endif
