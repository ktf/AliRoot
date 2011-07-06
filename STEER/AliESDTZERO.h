
// -*- mode: C++ -*- 
#ifndef ALIESDTZERO_H
#define ALIESDTZERO_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */


//-------------------------------------------------------------------------
//                          Class AliESDTZERO
//   This is a class that summarizes the TZERO data for the ESD   
//   Origin: Christian Klein-Boesing, CERN, Christian.Klein-Boesing@cern.ch 
//-------------------------------------------------------------------------



#include <TObject.h>

class AliESDTZERO: public TObject {
public:
  AliESDTZERO();
  AliESDTZERO(const AliESDTZERO& tzero);
  AliESDTZERO& operator=(const AliESDTZERO& tzero);
  virtual void Copy(TObject &obj) const;

  Double32_t GetT0zVertex() const {return fT0zVertex;}
  void SetT0zVertex(Double32_t z) {fT0zVertex=z;}
  Double32_t GetT0() const {return fT0timeStart;}
  void SetT0(Double_t timeStart) {fT0timeStart = timeStart;}
  Float_t GetT0clock() const {return fT0clock;}
  void SetT0clock(Float_t timeStart) {fT0clock = timeStart;}
  Double32_t GetT0TOF(Int_t i) const {return fT0TOF[i];}
  const Double32_t * GetT0TOF() const {return fT0TOF;}
  void SetT0TOF(Int_t icase, Float_t time) { fT0TOF[icase] = time;}
  Int_t GetT0Trig() const {return fT0trig;}
  void SetT0Trig(Int_t tvdc) {fT0trig = tvdc;}
  Bool_t GetT0Trig(Int_t i) {return (fT0trig&(1<<i)) != 0;}
  const Double32_t * GetT0time() const {return fT0time;}
  void SetT0time(Double32_t time[24]) {
    for (Int_t i=0; i<24; i++) fT0time[i] = time[i];
  }
  const Double32_t * GetT0amplitude() const {return fT0amplitude;}
  void SetT0amplitude(Double32_t amp[24]) {
    for (Int_t i=0; i<24; i++) fT0amplitude[i] = amp[i];
  }
  Float_t GetTimeFull(Int_t ch, Int_t hit) {return fTimeFull[ch][hit];}
  Float_t GetOrA(Int_t hit) {return fOrA[hit];}
  Float_t GetOrC(Int_t hit) {return fOrC[hit];}
  Float_t GetTVDC(Int_t hit) {return fTVDC[hit];}
  
  void SetTimeFull(Int_t ch, Int_t hit, Float_t time) {fTimeFull[ch][hit] = time;}
  void SetOrA (Int_t hit, Float_t time) { fOrA[hit] = time ;}
  void SetOrC (Int_t hit, Float_t time) { fOrC[hit] = time;}
  void SetTVDC(Int_t hit, Float_t time) { fTVDC[hit] = time;}
  
  void SetMultC(Float_t mult) {fMultC = mult;}
  void SetMultA(Float_t mult) {fMultA = mult;}
  Float_t GetMultC()       const {return fMultC;}
  Float_t GetMultA()       const {return fMultA;}
  
  void    Reset();
  void    Print(const Option_t *opt=0) const;

private:

  Float_t      fT0clock;     // backward compatibility
  Double32_t   fT0TOF[3];     // interaction time in ns ( A&C, A, C)
  Double32_t   fT0zVertex;       // vertex z position estimated by the T0
  Double32_t   fT0timeStart;     // interaction time estimated by the T0
  Int_t        fT0trig;            // T0 trigger signals
  Double32_t   fT0time[24];      // best TOF on each T0 PMT
  Double32_t   fT0amplitude[24]; // number of particles(MIPs) on each T0 PMT
  Float_t fTimeFull[24][5];    // array's TDC no-correction ;centred  around 0
  Float_t fOrA[5];  //hardware OrA centred around 0
  Float_t fOrC[5];  //hardware OrC centred around 0
  Float_t fTVDC[5]; //hardware TVDC centred around 0
  Bool_t fPileup;   // pile-up flag
  Bool_t fSattelite; //sattelite flag
  Float_t fMultC; // multiplicity on the C side
  Float_t fMultA; // multiplicity on the A side
 
  ClassDef(AliESDTZERO,5)
};


#endif
