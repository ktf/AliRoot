#ifndef ALIVZEROCALIBDATA_H
#define ALIVZEROCALIBDATA_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

////////////////////////////////////////////////
//                                            // 
//  class for VZERO calibration               //
//                                            //
////////////////////////////////////////////////

#include "TNamed.h"
#include "AliVZERO.h"
#include "AliVZERODataDCS.h"

class AliVZEROCalibData: public TNamed {

 public:
  enum { kNCIUBoards = 8 };
  
  AliVZEROCalibData();
  AliVZEROCalibData(const char* name);
  AliVZEROCalibData(const AliVZEROCalibData &calibda);
  AliVZEROCalibData& operator= (const AliVZEROCalibData &calibda);
  virtual ~AliVZEROCalibData();
  void Reset();
  void FillDCSData(AliVZERODataDCS * data);

  Float_t  GetPedestal(Int_t channel)   const {return fPedestal[channel];}
  Float_t* GetPedestal()   const {return (float*)fPedestal;}
  Float_t  GetSigma(Int_t channel)   const {return fSigma[channel];}
  Float_t* GetSigma()   const {return (float*)fSigma;}
  Float_t  GetADCmean(Int_t channel)	const {return fADCmean[channel];}
  Float_t* GetADCmean()   const {return (float*)fADCmean;}
  Float_t  GetADCsigma(Int_t channel)	const {return fADCsigma[channel];}
  Float_t* GetADCsigma()   const {return (float*)fADCsigma;}
  Float_t  GetMeanHV(Int_t channel)	const {return fMeanHV[channel];}
  Float_t* GetMeanHV()   const {return (float*)fMeanHV;} 
  Float_t  GetWidthHV(Int_t channel)	const {return fWidthHV[channel];}
  Float_t* GetWidthHV()   const {return (float*)fWidthHV;}
  Bool_t   IsChannelDead(Int_t channel)	const {return fDeadChannel[channel];}
  Bool_t*  GetDeadMap()   const {return (bool*)fDeadChannel;} 
   
  Float_t  GetGain(Int_t channel)	const {return fGain[channel];}
  Float_t* GetGain()   const {return (float*)fGain;}  
  Float_t  GetTimeOffset(Int_t channel)	const {return fTimeOffset[channel];}
  Float_t* GetTimeOffset()   const {return (float*)fTimeOffset;}
  Float_t  GetTimeGain(Int_t channel)	const {return fTimeGain[channel];}
  Float_t* GetTimeGain()   const {return (float*)fTimeGain;}

  Float_t* GetTimeResolution() const {return (Float_t*) fTimeResolution;};
  Float_t  GetTimeResolution(Int_t board ) const  {return (board<kNCIUBoards?fTimeResolution[board]:0);};

  Float_t* GetWidthResolution() const {return (Float_t*) fWidthResolution;};
  Float_t  GetWidthResolution(Int_t board ) const  {return (board<kNCIUBoards?fWidthResolution[board]:0);};
    
  void     SetPedestal(Float_t val, Int_t channel) {fPedestal[channel]=val;}
  void     SetPedestal(Float_t* Pedestal);
  void     SetSigma(Float_t val, Int_t channel) {fSigma[channel]=val;}
  void     SetSigma(Float_t* Sigma);
  void 	   SetADCmean(Float_t val, Int_t channel) {fADCmean[channel]=val;}
  void 	   SetADCmean(Float_t* ADCmean);  
  void 	   SetADCsigma(Float_t val, Int_t channel) {fADCsigma[channel]=val;}
  void 	   SetADCsigma(Float_t* ADCsigma);
  void     SetMeanHV(Float_t val, Int_t channel) {fMeanHV[channel]=val;}
  void     SetMeanHV(Float_t* MeanHV);  
  void     SetWidthHV(Float_t val, Int_t channel) {fWidthHV[channel]=val;}
  void     SetWidthHV(Float_t* WidthHV); 
  void     SetDeadChannel(Bool_t val, Int_t channel) {fDeadChannel[channel]=val;}
  void     SetDeadMap(Bool_t* deadMap);  
   
  void 	   SetGain(Float_t val, Int_t channel) {fGain[channel]=val;}
  void 	   SetGain(Float_t* Gain);  
  void     SetTimeOffset(Float_t val, Int_t channel) {fTimeOffset[channel]=val;}
  void     SetTimeOffset(Float_t* TimeOffset);
  void     SetTimeGain(Float_t val, Int_t channel) {fTimeGain[channel]=val;}
  void     SetTimeGain(Float_t* TimeGain);
  
  void 	   SetParameter(TString name, Float_t val);
  void     SetTimeResolution(UShort_t *resols);
  void     SetTimeResolution(UShort_t resol, Int_t board);
  void     SetWidthResolution(UShort_t *resols);
  void     SetWidthResolution(UShort_t resol, Int_t board);

  Float_t  GetMIPperADC(Int_t channel) const;

 protected:
  Float_t  fPedestal[128];     // Mean pedestal values
  Float_t  fSigma[128];        // Sigmas of pedestal peaks
  Float_t  fADCmean[128];      // ADC mean values
  Float_t  fADCsigma[128];     // ADC sigma values
  Float_t  fMeanHV[64];        // Mean PMT HV needed to compute MIP value
  Float_t  fWidthHV[64];       // Width of the PMT HV
  
  Float_t  fGain[128];	       // Gain factor used in digitization only  
  Float_t  fTimeOffset[64];
  Float_t  fTimeGain[64];
  Bool_t   fDeadChannel[64];
  Float_t  fTimeResolution[kNCIUBoards]; // Time Resolution of the TDC (ns / channel)
  Float_t  fWidthResolution[kNCIUBoards]; // Time Width Resolution of the TDC (ns / channel)

  ClassDef(AliVZEROCalibData,4)    // VZERO Calibration data
};

#endif
