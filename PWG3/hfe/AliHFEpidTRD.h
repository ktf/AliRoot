/**************************************************************************
* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
*                                                                        *
* Author: The ALICE Off-line Project.                                    *
* Contributors are mentioned in the code where appropriate.              *
*                                                                        *
* Permission to use, copy, modify and distribute this software and its   *
* documentation strictly for non-commercial purposes is hereby granted   *
* without fee, provided that the above copyright notice appears in all   *
* copies and that both the copyright notice and this permission notice   *
* appear in the supporting documentation. The authors make no claims     *
* about the suitability of this software for any purpose. It is          *
* provided "as is" without express or implied warranty.                  *
**************************************************************************/
//
// TRD PID Class
// Does PID either on a x% electron efficiency basis or on dE/dx
// For more information please check the implementation file
//
#ifndef ALIHFEPIDTRD_H
#define ALIHFEPIDTRD_H

 #ifndef ALIHFEPIDBASE_H
 #include "AliHFEpidBase.h"
 #endif

class AliAODTrack;
class AliAODMCParticle;
class AliESDtrack;
class AliHFEcollection;
class AliMCParticle;
class AliVParticle;
class TList;
class TH2F;

class AliHFEpidTRD : public AliHFEpidBase{
  public:
    typedef enum{
      kLQ = 0,
      kNN = 1
    } PIDMethodTRD_t;
    enum{
      kThreshParams = 24
    };
    enum{
      kHistTRDlikeBefore = 0,
      kHistTRDlikeAfter = 1,
      kHistTRDthresholds = 2,
      kHistTRDSigV1 = 3,
      kHistTRDSigV2 = 4,
      kHistOverallSpecies = 5
    };
    AliHFEpidTRD();
    AliHFEpidTRD(const Char_t *name);
    AliHFEpidTRD(const AliHFEpidTRD &ref);
    AliHFEpidTRD& operator=(const AliHFEpidTRD &ref);
    virtual ~AliHFEpidTRD();
    
    virtual Bool_t InitializePID();
    virtual Int_t IsSelected(AliHFEpidObject *track, AliHFEpidQAmanager *pidqa);

    Double_t GetTRDSignalV1(const AliESDtrack *track);
    Double_t GetTRDSignalV2(const AliESDtrack *track);

    Bool_t IsCalculateTRDSignals() const { return TestBit(kTRDsignals); }
    void SetPIDMethod(PIDMethodTRD_t method) { fPIDMethod = method; };
    void SetElectronEfficiency(Double_t electronEfficiency) { fElectronEfficiency = electronEfficiency; }
    void SetMinP(Double_t p) { fMinP = p; }
    void CalculateTRDSignals(Bool_t docalc) { SetBit(kTRDsignals, docalc); } 

    Double_t GetTRDthresholds(Double_t electronEff, Double_t p);
  protected:
    enum{
      kTRDsignals = BIT(16)
    };
    void Copy(TObject &ref) const;
    Double_t GetElectronLikelihood(const AliVParticle *track, AliHFEpidObject::AnalysisType_t anaType);
    Double_t GetP(const AliVParticle *track, AliHFEpidObject::AnalysisType_t anaType);
    void InitParameters();
    void InitParameters1DLQ();
    void GetParameters(Double_t electronEff, Double_t *parameters);

  private:
    static const Double_t fgkVerySmall;                       // Check for 0
    Double_t fMinP;                                         // Minimum momentum above which TRD PID is applied
    Double_t fElectronEfficiency;                           // Cut on electron efficiency
    PIDMethodTRD_t fPIDMethod;                              // PID Method: 2D Likelihood or Neural Network
    Double_t fThreshParams[kThreshParams];                  // Threshold parametrisation
  ClassDef(AliHFEpidTRD, 1)     // TRD electron ID class
};

#endif
