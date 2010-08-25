//
// Class AliRsnValue
//
// Definition of a single value which can be computed
// from any of the defined input objects implemented
// in the resonance package.
//

#ifndef ALIRSNVALUE_H
#define ALIRSNVALUE_H

#include "TNamed.h"
#include "TArrayD.h"

class AliRsnPairDef;
class AliRsnMother;

class AliRsnValue : public TNamed
{
  public:

    enum EValueType
    {
      kTrack1P,
      kTrack2P,
      kTrack1Pt,
      kTrack2Pt,
      kTrack1Px,
      kTrack1Py,
      kTrack1Pz,
      kTrack2Px,
      kTrack2Py,
      kTrack2Pz,
      kPairInvMass,
      kPairInvMassMC,
      kPairInvMassRes,
      kPairPt,
      kPairEta,
      kPairMt,
      kPairY,
      kPairCosThetaStar,
      kPairCosThetaStar1,
      kPairCosThetaStar2,
      kPairCosThetaStarMC1,
      kPairCosThetaStarMC2,
      kAngleToLeading,
      kLeadingPt,
      kQInv,
      kEventMult,
      kValueTypes
    };

    AliRsnValue();
    AliRsnValue(const char *name, EValueType type, Int_t n = 0, Double_t min = 0.0, Double_t max = 0.0);
    AliRsnValue(const char *name, EValueType type, Double_t min, Double_t max, Double_t step);
    AliRsnValue(const char *name, EValueType type, Int_t n, Double_t *array);
    AliRsnValue(const AliRsnValue& copy) : TNamed(copy),fType(copy.fType),fValue(copy.fValue),fArray(copy.fArray) {}
    AliRsnValue& operator=(const AliRsnValue& copy) {SetName(copy.GetName());fType=copy.fType;fValue=copy.fValue;fArray=copy.fArray;return (*this);}
    virtual ~AliRsnValue() { }
    
    TArrayD     GetArray() const {return fArray;}
    Double_t    GetValue() const {return fValue;}
    EValueType  GetValueType() {return fType;}

    void        SetValueType(EValueType type) {fType = type;}
    void        SetBins(Int_t n, Double_t min, Double_t max);
    void        SetBins(Double_t min, Double_t max, Double_t step);
    void        SetBins(Int_t n, Double_t *array);
    
    Bool_t      Eval(AliRsnMother * const mother, AliRsnPairDef * const pairDef, AliRsnEvent * const event);

  private:

    EValueType fType;    // value type
    Double_t   fValue;   // computed value
    TArrayD    fArray;   // array of bins (when necessary)
    
    // ROOT dictionary
    ClassDef(AliRsnValue, 1)
};

#endif
