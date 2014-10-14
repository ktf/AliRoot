#ifndef ALIAODPWG4PARTICLE_H
#define ALIAODPWG4PARTICLE_H
/* Copyright(c) 1998-2007, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id: AliAODPWG4Particle.h  $ */

//-------------------------------------------------------------------------
//     AOD objects class in use in the CaloTrackCorrelations
//     analysis pacackge ($ALICE_ROOT/PWGGA/CaloTrackCorrelations)
//
//     Author: Gustavo Conesa Balbastre - CNRS-LPSC-Grenoble
//-------------------------------------------------------------------------

//-- ROOT system --
#include <TLorentzVector.h>
class TString;

//-- Analysis system
#include "AliVParticle.h"

class AliAODPWG4Particle : public AliVParticle {
  
 public:
  AliAODPWG4Particle();
  AliAODPWG4Particle(Double_t px, Double_t py, Double_t pz, Double_t e);
  AliAODPWG4Particle(TLorentzVector & p);
  
  virtual ~AliAODPWG4Particle();
  virtual void Clear(const Option_t* /*opt*/);

  AliAODPWG4Particle (           const AliAODPWG4Particle& photon);
  AliAODPWG4Particle & operator=(const AliAODPWG4Particle& photon);

  // Main methods to recover kinematics or PID
  TLorentzVector * Momentum() const                { return fMomentum ; }
  virtual void     SetMomentum(TLorentzVector *lv) { fMomentum = lv   ; }
  
  Bool_t IsPIDOK(const Int_t ipid, const Int_t pdgwanted) const;
  Double_t GetPairMass(AliAODPWG4Particle * p)const{ return (*(p->fMomentum)+*fMomentum).M(); }
  
  // AliVParticle methods
  virtual Double_t Px()         const { return fMomentum->Px();      }
  virtual Double_t Py()         const { return fMomentum->Py();      }
  virtual Double_t Pz()         const { return fMomentum->Pz();      }
  virtual Double_t Pt()         const { return fMomentum->Pt();      }
  virtual Double_t P()          const { return fMomentum->P();       }
  virtual Bool_t   PxPyPz(Double_t p[3]) const { p[0] = Px(); p[1] = Py(); p[2] = Pz(); return kTRUE; }
  virtual Double_t OneOverPt()  const { return 1. / fMomentum->Pt(); }
  virtual Double_t Phi()        const;
  virtual Double_t Theta()      const { return fMomentum->Theta();   }
  virtual Double_t E()          const { return fMomentum->E();       }
  virtual Double_t M()          const { return fMomentum->M();       }
  virtual Double_t Eta()        const { return fMomentum->Eta();     }
  virtual Double_t Y()          const { return fMomentum->Rapidity();}
  virtual Double_t Xv()         const { return -999.;} // put reasonable values here
  virtual Double_t Yv()         const { return -999.;} //
  virtual Double_t Zv()         const { return -999.;} //
  virtual Bool_t   XvYvZv(Double_t x[3]) const { x[0] = Xv(); x[1] = Yv(); x[2] = Zv(); return kTRUE; }  
  virtual void     Print(Option_t* /*option*/) const;
  
  //
  //Dummy
  virtual Short_t Charge()      const { return 0;}
  virtual const Double_t* PID() const { return NULL;}
  Int_t   PdgCode() const {return 0;}
  //

  //
  // Specific getters
  virtual Int_t   GetIdentifiedParticleType() const { return fPdg     ; }

  virtual Int_t   GetLabel()             const { return fLabel        ; }
  virtual Int_t   GetCaloLabel (Int_t i) const { return fCaloLabel[i] ; }
  virtual Int_t   GetTrackLabel(Int_t i) const { return fTrackLabel[i]; }
  virtual TString GetDetector()          const { return fDetector     ; }
  virtual UInt_t  GetDetectorTag()       const { return fDetectorTag  ; }
  virtual Bool_t  GetDispBit()           const { return fDisp         ; }
  virtual Bool_t  GetTOFBit()            const { return fTof          ; }
  virtual Bool_t  GetChargedBit()        const { return fCharged      ; }
  virtual Int_t   DistToBad()            const { return fBadDist      ; }
  virtual Int_t   GetInputFileIndex()    const { return fInputFileIndex ; }
  virtual Int_t   GetFiducialArea()      const { return fFidArea      ; }

  // Tags
  virtual Int_t   GetTag()               const { return fTag          ; }
  virtual Bool_t  IsTagged()             const { return fTagged       ; }
  virtual Int_t   DecayTag()             const { return fDecayTag     ; }
  virtual Bool_t  IsIsolated()           const { return fIsolated     ; }
  virtual Bool_t  IsLeadingParticle()    const { return fLeadingParticle ; }

  // Calorimeter specific param
  virtual Int_t   GetNLM()               const { return fNLM          ; }
  virtual Float_t GetM02()               const { return fM02          ; }
  
  //
  // Specific setters
  virtual void SetIdentifiedParticleType(Int_t pdg) { fPdg = pdg ; }

  virtual void SetLabel(Int_t l)         { fLabel = l ; }
  virtual void SetCaloLabel (Int_t a, Int_t b) { fCaloLabel [0] = a; fCaloLabel [1] = b  ; }
  virtual void SetTrackLabel(Int_t a, Int_t b) { fTrackLabel[0] = a; fTrackLabel[1] = b  ; }
  virtual void SetTrackLabel(Int_t a, Int_t b, Int_t c, Int_t d) 
  { fTrackLabel[0] = a; fTrackLabel[1] = b  ; fTrackLabel[2] = c; fTrackLabel[3] = d; }
  
  virtual void SetDetector(TString   d)  { fDetector    = d    ; }
  virtual void SetDetectorTag(UInt_t d)  { fDetectorTag = d    ; }
  virtual void SetDispBit(Bool_t disp)   { fDisp        = disp ; }
  virtual void SetTOFBit(Bool_t tof)     { fTof         = tof  ; }
  virtual void SetChargedBit(Bool_t ch)  { fCharged     = ch   ; }
  virtual void SetDistToBad(Int_t dist)  { fBadDist     = dist ; }
  virtual void SetInputFileIndex(Int_t i){ fInputFileIndex = i ; }
  virtual void SetFiducialArea(Int_t a)  { fFidArea     = a    ; }

  // Tags
  virtual void SetTag(Int_t tag)         { fTag         = tag  ; }
  virtual void SetTagged(Bool_t tag)     { fTagged      = tag  ; }
  virtual void SetDecayTag(Int_t tag)    { fDecayTag    = tag  ; }
  virtual void SetIsolated(Bool_t iso)   { fIsolated    = iso  ; }
  virtual void SetLeadingParticle(Bool_t l) { fLeadingParticle = l ; }
  
  // Calorimeter specific param
  virtual void SetNLM(UInt_t  nlm)       { fNLM         = nlm  ; }
  virtual void SetM02(Float_t m02)       { fM02         = m02  ; }
  
  //
  // BTagging
  // enumerated type for various b-tags of electrons
  enum btagTypes {kDVMTag0, kDVMTag1, kDVMTag2, kTransverseIPTag, kUnknownTag};
  
  virtual void  SetBtag(Int_t tag)      { fBtag        = tag  ; }
  virtual Int_t GetBtag()         const { return fBtag        ; }

  void SetBTagBit(Int_t &tag, const UInt_t set) const {
    // Set bit of type set (btagTypes) in tag
    tag |= (1<<set) ;
  }
  
  Bool_t CheckBTagBit(const Int_t tag, const UInt_t test) const {
    // Check if in fBtag the bit test (btagTypes) is set.
    if (tag & (1<<test) ) return  kTRUE ;
    else return kFALSE ;
  }
  
 private:
  TLorentzVector* fMomentum; // Photon 4-momentum vector
  Int_t      fPdg ;          // type of identified particle, same code as PDG, but this is not a MonteCarlo particle 
  Int_t      fTag ;          // tag of particle (decay, fragment, prompt photon), MC
  Int_t      fLabel ;        // MC label
  Int_t      fCaloLabel[2];  // CaloCluster index, 1 for photons, 2 for pi0.
  Int_t      fTrackLabel[4]; // Track lable, 1 for pions, 2 for conversion photons 
  TString    fDetector ;     // Detector where particle was measured.
  UInt_t     fDetectorTag ;  // Detector where particle was measured, integer
  Bool_t     fDisp ;         // Dispersion bit
  Bool_t     fTof ;          // TOF bit
  Bool_t     fCharged ;      // Charged bit
  Int_t      fDecayTag;      // Tag the photon as decay from, pi0, eta, pi0 side band, eta side band
  Int_t      fBadDist ;      // Distance to bad module in module units
  UInt_t     fNLM ;          // Store the number of local maxima
  Float_t    fM02 ;          // Store the main axis of the calorimeter shower shape
  Bool_t     fIsolated ;     // Particle is isolated or not
  Bool_t     fLeadingParticle ; //Particle is leading or not

  // Not in use currently ...
  Bool_t     fTagged ;       // If photon tagged (pi0 decay), not used anymore, replace by fDecayTag
  Int_t      fFidArea ;      // Type of fiducial area hit by this photon
  Int_t      fInputFileIndex;// 0, standard input, 1 first input added. Only possible one for now, not really used.
  Int_t      fBtag;          // tag particle from B.

  
  ClassDef(AliAODPWG4Particle, 5);
};

inline Double_t AliAODPWG4Particle::Phi() const
{
  // Return phi
  Double_t phi = fMomentum->Phi();
  if (phi < 0.) phi += TMath::TwoPi();
  return phi;
}

#endif //ALIAODPWG4PARTICLE_H
