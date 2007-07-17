#ifndef ALIHLTPHOSRAWANALYZER_H
#define ALIHLTPHOSRAWANALYZER_H
/* Copyright(c) 1998-2004, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

//#include "Rtypes.h"

#include "AliHLTPHOSBase.h"

class AliHLTPHOSRawAnalyzer: public AliHLTPHOSBase
{
 public:
  AliHLTPHOSRawAnalyzer();
  virtual ~AliHLTPHOSRawAnalyzer();
  AliHLTPHOSRawAnalyzer(double *dataPtr, double fs);
  //  AliHLTPHOSRawAnalyzer(const AliHLTPHOSRawAnalyzer & );
  //  AliHLTPHOSRawAnalyzer & operator = (const AliHLTPHOSRawAnalyzer &)
  //   {
  //      return *this;
  //   }

  void BaselineCorrection(double *dataPtr, int N);
  void BaselineCorrection(double *dataPtr, double baselineValue);  
  int FindStartIndex(double treshold);
  float GetTiming() const;
  float GetEnergy() const;

  void SetData(double *data);
  void SetData(UInt_t *data);

  void SetSampleFreq(double freq);
  void SetStartIndex(int startIndex);
  void MakeInitialGuess();
  void MakeInitialGuess(int treshold);
  

  virtual void SetTVector(Double_t *tVector, Int_t size);
  virtual void SetAVector(Double_t *aVector, Int_t size);


  /**
   *Abstratct class documentation
   */
  virtual void Evaluate(Int_t start = 0, Int_t lenght = 100) = 0;
  //  Double_t GetMaxValue(Double_t *dta, Int_t size) const;
  //  UInt_t GetMaxValue(UInt_t *dta, Int_t size) const;

 protected:
  Double_t   *fFloatDataPtr;   /**<Float representation of data that should be fitted */
  UInt_t   *fIntDataPtr;   /**<data that should be fitted */
  double     fSampleFrequency; /**<The ADC sample frequency in MHz used under data taking */
  double     fDTofGuess;       /**<Initial guess for t0*/
  double     fDAmplGuess;      /**<Initial guess for amplitude*/
  double     fTau;	       /**<The risetime in micro seconds*/		 
  double     fDTof;            /**<Time of flight in entities of sample intervals */
  double     fDAmpl;           /**<Amplitude in entities of ADC levels*/
  int        fStartIndex;
};


#endif
