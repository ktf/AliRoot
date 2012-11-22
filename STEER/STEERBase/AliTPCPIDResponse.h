#ifndef ALITPCPIDRESPONSE_H
#define ALITPCPIDRESPONSE_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//-------------------------------------------------------
//                    TPC PID class
// A very naive design... Should be made better by the detector experts...
//   Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch 
// With many additions and modifications suggested by
//      Alexander Kalweit, GSI, alexander.philipp.kalweit@cern.ch
//      Dariusz Miskowiec, GSI, D.Miskowiec@gsi.de
// ...and some modifications by
//      Mikolaj Krzewicki, GSI, mikolaj.krzewicki@cern.ch
// ...and some modifications plus eta correction functions by
//      Benjamin Hess, University of Tuebingen, bhess@cern.ch
//-------------------------------------------------------
#include <Rtypes.h>

#include <TNamed.h>
#include <TVectorF.h>
#include <TObjArray.h>

#include "AliPID.h"
#include "AliVTrack.h"

class TH2D;
class TSpline3;

class AliTPCPIDResponse: public TNamed {
public:
  AliTPCPIDResponse();
  AliTPCPIDResponse(const Double_t *param);
  AliTPCPIDResponse(const AliTPCPIDResponse&);
  AliTPCPIDResponse& operator=(const AliTPCPIDResponse&);
  virtual ~AliTPCPIDResponse();

  enum EChamberStatus {
    kChamberOff=0,
    kChamberHighGain=1,
    kChamberLowGain=2,
    kChamberInvalid=3
  };
  
  enum ETPCgainScenario {
    kDefault= 0,
    kALLhigh = 1,
    kOROChigh = 2,
    kGainScenarioInvalid = 3
  };

  static const Int_t fgkNumberOfParticleSpecies=AliPID::kSPECIESC;
  static const Int_t fgkNumberOfGainScenarios=3;
  static const Int_t fgkNumberOfdEdxSourceScenarios=3;

  enum ETPCdEdxSource {
    kdEdxDefault=0,        // use combined dEdx from IROC+OROC (assumes ideal detector)
    kdEdxOROC=1,       // use only OROC
    kdEdxHybrid=2,   // Use IROC+OROC dEdx only where IROCS are good (high gain), otherwise fall back to OROC only
    kdEdxInvalid=3     //invalid
  };

  void SetSigma(Float_t res0, Float_t resN2);
  void SetBetheBlochParameters(Double_t kp1,
                               Double_t kp2,
                               Double_t kp3,
                               Double_t kp4,
                               Double_t kp5
                               );
  void SetMip(Float_t mip) { fMIP = mip; } // Set overall normalisation; mean dE/dx for MIP
  Double_t Bethe(Double_t bg) const;
  void SetUseDatabase(Bool_t useDatabase) { fUseDatabase = useDatabase;}
  Bool_t GetUseDatabase() const { return fUseDatabase;}
  
  void SetResponseFunction(AliPID::EParticleType type, TObject * const o) { fResponseFunctions.AddAt(o,(Int_t)type); }
  const TObject * GetResponseFunction(AliPID::EParticleType type) { return fResponseFunctions.At((Int_t)type); }
  void SetVoltage(Int_t n, Float_t v) {fVoltageMap[n]=v;}
  void SetVoltageMap(const TVectorF& a) {fVoltageMap=a;} //resets ownership, ~ will not delete contents
  Float_t GetVoltage(Int_t n) const {return fVoltageMap[n];}
  void SetLowGainIROCthreshold(Float_t v) {fLowGainIROCthreshold=v;}
  void SetBadIROCthreshold(Float_t v) {fBadIROCthreshhold=v;}
  void SetLowGainOROCthreshold(Float_t v) {fLowGainOROCthreshold=v;}
  void SetBadOROCthreshold(Float_t v) {fBadOROCthreshhold=v;}
  void SetMaxBadLengthFraction(Float_t f) {fMaxBadLengthFraction=f;}

  void SetMagField(Double_t mf) { fMagField=mf; }
  
  const TH2D* GetEtaCorrMap() const { return fhEtaCorr; };
  Bool_t SetEtaCorrMap(TH2D* hMap);
  
  Double_t GetEtaCorrection(const AliVTrack *track, AliPID::EParticleType species, ETPCdEdxSource dedxSource = kdEdxDefault) const;
  
  Double_t GetEtaCorrectedTrackdEdx(const AliVTrack *track, AliPID::EParticleType species, ETPCdEdxSource dedxSource = kdEdxDefault) const;

  const TH2D* GetSigmaPar1Map() const { return fhEtaSigmaPar1; };
  Double_t GetSigmaPar0() const { return fSigmaPar0; };
  Bool_t SetSigmaParams(TH2D* hSigmaPar1Map, Double_t sigmaPar0);
  
  Double_t GetSigmaPar1(const AliVTrack *track, AliPID::EParticleType species, ETPCdEdxSource dedxSource = kdEdxDefault) const;

  //NEW
  void SetSigma(Float_t res0, Float_t resN2, ETPCgainScenario gainScenario );
  Double_t GetExpectedSignal( const AliVTrack* track,
                              AliPID::EParticleType species,
                              ETPCdEdxSource dedxSource = kdEdxDefault,
                              Bool_t correctEta = kFALSE) const;//TODO: In future, default kTRUE
  Double_t GetExpectedSigma( const AliVTrack* track, 
                             AliPID::EParticleType species,
                             ETPCdEdxSource dedxSource = kdEdxDefault,
                             Bool_t correctEta = kFALSE) const;//TODO: In future, default kTRUE
  Float_t GetNumberOfSigmas( const AliVTrack* track,
                             AliPID::EParticleType species,
                             ETPCdEdxSource dedxSource = kdEdxDefault,
                             Bool_t correctEta = kFALSE) const;//TODO: In future, default kTRUE

  void SetResponseFunction(TObject* o,
                           AliPID::EParticleType type,
                           ETPCgainScenario gainScenario);
  void Print(Option_t* option="") const;
  TSpline3* GetResponseFunction( AliPID::EParticleType species,
                                 ETPCgainScenario gainScenario ) const;
  TSpline3* GetResponseFunction( const AliVTrack* track,
                                 AliPID::EParticleType species,
                                 ETPCdEdxSource dedxSource = kdEdxDefault) const;
  Bool_t ResponseFunctiondEdxN(const AliVTrack* track, 
                               AliPID::EParticleType species,
                               ETPCdEdxSource dedxSource,
                               Double_t& dEdx, Int_t& nPoints, ETPCgainScenario& gainScenario, TSpline3** responseFunction) const;
  Bool_t sectorNumbersInOut(Double_t* trackPositionInner,
                            Double_t* trackPositionOuter,
                            Float_t& phiIn, Float_t& phiOut, 
                            Int_t& in, Int_t& out ) const;
  AliTPCPIDResponse::EChamberStatus TrackStatus(const AliVTrack* track, Int_t layer) const;
  Float_t MaxClusterRadius(const AliVTrack* track) const;
  Bool_t TrackApex(const AliVTrack* track, Float_t magField, Double_t position[3]) const;
  static const char* GainScenarioName(Int_t n) {return fgkGainScenarioName[(n>fgkNumberOfdEdxSourceScenarios)?fgkNumberOfdEdxSourceScenarios+1:n];}
  Int_t ResponseFunctionIndex( AliPID::EParticleType species,
                               ETPCgainScenario gainScenario ) const;
  void ResetSplines();

  //OLD
  Double_t GetExpectedSignal(const Float_t mom,
                     AliPID::EParticleType n=AliPID::kKaon) const;
  Double_t GetExpectedSigma(const Float_t mom, const Int_t nPoints,
                            AliPID::EParticleType n=AliPID::kKaon) const;
  Float_t  GetNumberOfSigmas(const Float_t mom, 
                             const Float_t dEdx, 
			                       const Int_t nPoints,
                             AliPID::EParticleType n=AliPID::kKaon) const {
    //
    // Deprecated function (for backward compatibility). Please use 
    // GetNumberOfSigmas(const AliVTrack *track, AliPID::EParticleType species, ETPCdEdxSource dedxSource )
    // instead!TODO
    //
    
    Double_t bethe=GetExpectedSignal(mom,n);
    Double_t sigma=GetExpectedSigma(mom,nPoints,n);
    return (dEdx-bethe)/sigma;
  }

  Double_t GetMIP() const { return fMIP;} 
  Float_t  GetRes0()  const { return fRes0[0];  }
  Float_t  GetResN2() const { return fResN2[0]; }
  Float_t  GetRes0(ETPCgainScenario s)  const { return fRes0[s];  }
  Float_t  GetResN2(ETPCgainScenario s) const { return fResN2[s]; }

protected:
  Double_t GetExpectedSignal(const AliVTrack* track,
                             AliPID::EParticleType species,
                             Double_t dEdx,
                             const TSpline3* responseFunction,
                             Bool_t correctEta) const; 
  
  Double_t GetExpectedSigma(const AliVTrack* track, 
                            AliPID::EParticleType species,
                            ETPCgainScenario gainScenario,
                            Double_t dEdx,
                            Int_t nPoints,
                            const TSpline3* responseFunction,
                            Bool_t correctEta) const;
                             
  Double_t GetEtaCorrection(const AliVTrack *track, Double_t dEdxSplines) const;
  
  Double_t GetSigmaPar1(const AliVTrack *track, AliPID::EParticleType species,
                        Double_t dEdx, const TSpline3* responseFunction) const;
  
private:
  Float_t fMIP;          // dEdx for MIP
  Float_t fRes0[fgkNumberOfGainScenarios];  // relative dEdx resolution  rel sigma = fRes0*sqrt(1+fResN2/npoint)
  Float_t fResN2[fgkNumberOfGainScenarios]; // relative Npoint dependence rel  sigma = fRes0*sqrt(1+fResN2/npoint)

  Double_t fKp1;   // Parameters
  Double_t fKp2;   //    of
  Double_t fKp3;   // the ALEPH
  Double_t fKp4;   // Bethe-Bloch
  Double_t fKp5;   // formula

  Bool_t fUseDatabase; // flag if fine-tuned database-response or simple ALEPH BB should be used
  
  TObjArray fResponseFunctions; //! ObjArray of response functions individually for each particle
  TVectorF fVoltageMap; //!stores a map of voltages wrt nominal for all chambers
  Float_t fLowGainIROCthreshold;  //voltage threshold below which the IROC is considered low gain
  Float_t fBadIROCthreshhold;     //voltage threshold for bad IROCS
  Float_t fLowGainOROCthreshold;  //voltage threshold below which the OROC is considered low gain
  Float_t fBadOROCthreshhold;     //voltage threshold for bad OROCS
  Float_t fMaxBadLengthFraction;  //the maximum allowed fraction of track length in a bad sector.

  Int_t sectorNumber(Double_t phi) const;

  Double_t fMagField;  //! Magnetic field

  static const char* fgkGainScenarioName[fgkNumberOfGainScenarios+1];

  TH2D* fhEtaCorr; //! Map for TPC eta correction
  TH2D* fhEtaSigmaPar1; //! Map for parameter 1 of the dEdx sigma parametrisation
  
  Double_t fSigmaPar0; // Parameter 0 of the dEdx sigma parametrisation

  ClassDef(AliTPCPIDResponse,5)   // TPC PID class
};

#endif


