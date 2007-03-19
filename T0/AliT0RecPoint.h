#ifndef ALIT0RECPOINT_H
#define ALIT0RECPOINT_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */
#include <TObject.h>


//___________________________________________
class AliT0RecPoint: public TObject  {
////////////////////////////////////////////////////////////////////////
 public:
    AliT0RecPoint();
    virtual ~AliT0RecPoint() {}
    Int_t  GetMeanTime() {return fTimeAverage;}
    Int_t  GetBestTimeA() {return fTimeBestA ;}
    Int_t  GetBestTimeC() {return fTimeBestC ;}
    Int_t GetMultC() {return fMultC;}
    Int_t GetMultA() {return fMultA;}
    Float_t  GetVertex() {return fVertexPosition;}


    void SetMeanTime(Int_t time) {fTimeAverage=time;}
    void SetTimeBestA( Int_t time) {fTimeBestA = time;}
    void SetTimeBestC( Int_t time) {fTimeBestC = time;}
    void SetVertex( Float_t vertex) {fVertexPosition= vertex;}
    void SetMultC(Int_t mult) {fMultC = mult;}
    void SetMultA(Int_t mult) {fMultA = mult;}

    void SetTime (Int_t ipmt, Float_t time) { fTime[ipmt] = time;}
    Float_t GetTime (Int_t ipmt) { return fTime[ipmt];}
   void SetAmp (Int_t ipmt, Float_t adc) { fADC[ipmt] = adc;}
    Float_t GetAmp (Int_t ipmt) { return fADC[ipmt];}
   void SetAmpLED (Int_t ipmt, Float_t adc) { fADCLED[ipmt] = adc;}
    Float_t AmpLED (Int_t ipmt) { return fADCLED[ipmt];}

  private: 
    Int_t fTimeAverage;     // Average time
    Float_t fVertexPosition;     // Diffrence time between C and A
    Int_t fTimeBestA;   //TOF first particle on the A
    Int_t fTimeBestC;    //TOF first particle on the C
    Int_t fMultC; // multiplicity on the 
    Int_t fMultA; // multiplicity on the 
 
    Float_t fTime[24];    // array's TDC
    Float_t fADC[24];    // array's amplitude
    Float_t fADCLED[24];    // array's LED amplitude


    ClassDef(AliT0RecPoint,3)  //Digit (Header) object for set:T0
};

typedef AliT0RecPoint AliSTARTRecPoint; // for backward compatibility

#endif



