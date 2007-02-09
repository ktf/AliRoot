#ifndef ALIHLTPHOSRAWANALYZERPEAKFINDERCOMPONENT_H
#define ALIHLTPHOSRAWANALYZERPEAKFINDERCOMPONENT_H

/* Copyright(c) 2006, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice  */ 

#include "AliHLTPHOSRawAnalyzerComponent.h"

class AliHLTPHOSRawAnalyzerPeakFinderComponent: public AliHLTPHOSRawAnalyzerComponent
{
 public:
  AliHLTPHOSRawAnalyzerPeakFinderComponent();
  ~AliHLTPHOSRawAnalyzerPeakFinderComponent();
  virtual const char* GetComponentID();
  virtual AliHLTComponent* Spawn();

 private:
  AliHLTPHOSRawAnalyzerPeakFinderComponent(const AliHLTPHOSRawAnalyzerPeakFinderComponent & ); 
  AliHLTPHOSRawAnalyzerPeakFinderComponent & operator = (const AliHLTPHOSRawAnalyzerPeakFinderComponent)
    {
      return *this;
    };

};



#endif
