#ifndef ALIRSNEVENT_H
#define ALIRSNEVENT_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */
 
////////////////////////////////////////////////////////////////////////////////
//
//  Interface to full event.
//
////////////////////////////////////////////////////////////////////////////////

#include "AliVEvent.h"
#include "AliMCEvent.h"
#include "AliESDEvent.h"
#include "AliAODEvent.h"
#include "AliRsnDaughter.h"

class AliRsnCutPID;
class AliESDtrackCuts;

class AliRsnEvent : public TObject {
public:

   AliRsnEvent(AliVEvent *ref = 0, AliVEvent *refMC = 0);
   AliRsnEvent(const AliRsnEvent& copy);
   AliRsnEvent& operator= (const AliRsnEvent& copy);
   virtual ~AliRsnEvent();

   // basic setters/getters
   void       SetRef(AliVEvent *ref)       {fRef = ref;}
   void       SetRefMC(AliVEvent *refmc)   {fRefMC = refmc;}
   void       SetLeadingIndex(Int_t i)     {fLeading = i;}
   void       SetLocalID(Int_t i)          {fLocalID = i;}
   AliVEvent* GetRef()                     {return fRef;}
   AliVEvent* GetRefMC()                   {return fRefMC;}
   Int_t      GetLeadingIndex() const      {return fLeading;}
   Int_t      GetLeadingParticleID() const {return fLeading;}
   Int_t      GetLocalID() const           {return fLocalID;}

   // getters which convert into allowed input types
   AliESDEvent* GetRefESD()   {if (classMatchRef  (AliESDEvent::Class())) return static_cast<AliESDEvent*>(fRef)  ; return 0x0;}
   AliAODEvent* GetRefAOD()   {if (classMatchRef  (AliAODEvent::Class())) return static_cast<AliAODEvent*>(fRef)  ; return 0x0;}
   AliMCEvent*  GetRefMCESD() {if (classMatchRefMC(AliMCEvent ::Class())) return static_cast<AliMCEvent *>(fRefMC); return 0x0;}
   AliAODEvent* GetRefMCAOD() {if (classMatchRefMC(AliAODEvent::Class())) return static_cast<AliAODEvent*>(fRefMC); return 0x0;}
   Bool_t       IsESD()       {return (GetRefESD() != 0x0);}
   Bool_t       IsAOD()       {return (GetRefAOD() != 0x0);}

   // advanced getters
   Double_t         GetVz();
   Int_t            GetMultiplicity(AliESDtrackCuts *cuts = 0x0);
   Int_t            GetMultiplicityMC();

   // setters for a daughter
   Bool_t           SetDaughterAbs(AliRsnDaughter &daughter, Int_t absoluteIndex);
   Bool_t           SetDaughter(AliRsnDaughter &daughter, Int_t index, AliRsnDaughter::ERefType type = AliRsnDaughter::kTrack);
   Bool_t           SetDaughterMC(AliRsnDaughter &daughter, Int_t index);
   AliRsnDaughter   GetDaughterAbs(Int_t absoluteIndex);
   AliRsnDaughter   GetDaughter(Int_t i, AliRsnDaughter::ERefType type = AliRsnDaughter::kTrack);
   AliRsnDaughter   GetDaughterMC(Int_t i);
   Int_t            GetAbsoluteSum();
   Bool_t           ConvertAbsoluteIndex(Int_t index, Int_t &realIndex, AliRsnDaughter::ERefType &type);
   Int_t            ConvertRealIndex(Int_t index, AliRsnDaughter::ERefType type);

   // leading particle stuff
   void             SetLeadingParticle(AliRsnDaughter &leading) {if (fLeading >= 0) SetDaughter(leading, fLeading);}
   Int_t            SelectLeadingParticle(Double_t ptMin = 0.0, AliRsnCutPID *cutPID = 0x0);
   Double_t         GetAverageMomentum(Int_t &count, AliRsnCutPID *cutPID = 0x0);
   Bool_t           GetAngleDistr(Double_t &angleMean, Double_t &angleRMS, AliRsnDaughter reference);

   // statig getters
   static AliRsnEvent    *GetCurrentEvent1()                                 {return fgRsnEvent1;}
   static AliRsnEvent    *GetCurrentEvent2()                                 {return fgRsnEvent2;}
   static void            SetCurrentEvent1(AliRsnEvent *event, Int_t id = 0) {fgRsnEvent1 = event; fgRsnEvent1->SetLocalID(id);}
   static void            SetCurrentEvent2(AliRsnEvent *event, Int_t id = 0) {fgRsnEvent2 = event; fgRsnEvent2->SetLocalID(id);}
   static Bool_t          IsCurrentEvent1()                                  {if (fgRsnEvent1 != 0x0) return kTRUE; return kFALSE;}
   static Bool_t          IsCurrentEvent2()                                  {if (fgRsnEvent2 != 0x0) return kTRUE; return kFALSE;}
   static Bool_t          SameEvent()                                        {if (fgRsnEvent1 == fgRsnEvent2) return kTRUE; return kFALSE;}
   
private:

   Bool_t classMatchRef  (TClass *ref) {if (fRef  ) return (fRef  ->InheritsFrom(ref)); return kFALSE;}
   Bool_t classMatchRefMC(TClass *ref) {if (fRefMC) return (fRefMC->InheritsFrom(ref)); return kFALSE;}

   Bool_t SetDaughterESDtrack(AliRsnDaughter &target, Int_t index);
   Bool_t SetDaughterAODtrack(AliRsnDaughter &target, Int_t index);
   Bool_t SetDaughterESDv0(AliRsnDaughter &target, Int_t index);
   Bool_t SetDaughterAODv0(AliRsnDaughter &target, Int_t index);
   Bool_t SetDaughterESDcascade(AliRsnDaughter &target, Int_t index);
   Bool_t SetDaughterAODcascade(AliRsnDaughter &target, Int_t index);
   Bool_t SetMCInfoESD(AliRsnDaughter &target);
   Bool_t SetMCInfoAOD(AliRsnDaughter &target);

   AliVEvent   *fRef;               //  pointer to input event
   AliVEvent   *fRefMC;             //  pointer to reference MC event (if any)
   Int_t        fLeading;           //  index of leading track
   Int_t        fLocalID;           //  identification number used locally

   static AliRsnEvent *fgRsnEvent1; //! pointer to current event #1 (default current event)
   static AliRsnEvent *fgRsnEvent2; //! pointer to current event #2 (different from the other when doing mixing)

   ClassDef(AliRsnEvent, 4);
};

#endif
