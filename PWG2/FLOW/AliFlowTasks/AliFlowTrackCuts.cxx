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

/* $Id$ */ 

// AliFlowTrackCuts:
// ESD track cuts for flow framework 
//
// origin: Mikolaj Krzewicki (mikolaj.krzewicki@cern.ch)
//
// This class gurantees consistency of cut methods, trackparameter
// selection (global tracks, TPC only, etc..) and parameter mixing
// in the flow framework. Transparently handles different input types:
// ESD, MC, AOD.
// This class works in 2 steps: first the requested track parameters are
// constructed (to be set by SetParamType() ), then cuts are applied.
// the constructed track can be requested AFTER checking the cuts by
// calling GetTrack(), in this case the cut object stays in control,
// caller does not have to delete the track.
// Additionally caller can request an AliFlowTrack object to be constructed
// according the parameter mixing scenario requested by SetParamMix().
// AliFlowTrack is made using MakeFlowTrack() method, its an 'object factory'
// so caller needs to take care of the freshly created object.

#include <limits.h>
#include <float.h>
#include <TMatrix.h>
#include "TParticle.h"
#include "TObjArray.h"
#include "AliStack.h"
#include "AliMCEvent.h"
#include "AliESDEvent.h"
#include "AliVParticle.h"
#include "AliMCParticle.h"
#include "AliESDtrack.h"
#include "AliMultiplicity.h"
#include "AliAODTrack.h"
#include "AliFlowTrack.h"
#include "AliFlowTrackCuts.h"
#include "AliLog.h"
#include "AliESDpid.h"

ClassImp(AliFlowTrackCuts)

//-----------------------------------------------------------------------
AliFlowTrackCuts::AliFlowTrackCuts():
  AliFlowTrackSimpleCuts(),
  fAliESDtrackCuts(NULL),
  fQA(NULL),
  fCutMC(kFALSE),
  fCutMCprocessType(kFALSE),
  fMCprocessType(kPNoProcess),
  fCutMCPID(kFALSE),
  fMCPID(0),
  fIgnoreSignInPID(kFALSE),
  fCutMCisPrimary(kFALSE),
  fRequireTransportBitForPrimaries(kTRUE),
  fMCisPrimary(kFALSE),
  fRequireCharge(kFALSE),
  fFakesAreOK(kTRUE),
  fCutSPDtrackletDeltaPhi(kFALSE),
  fSPDtrackletDeltaPhiMax(FLT_MAX),
  fSPDtrackletDeltaPhiMin(-FLT_MAX),
  fIgnoreTPCzRange(kFALSE),
  fIgnoreTPCzRangeMax(FLT_MAX),
  fIgnoreTPCzRangeMin(-FLT_MAX),
  fCutChi2PerClusterTPC(kFALSE),
  fMaxChi2PerClusterTPC(FLT_MAX),
  fMinChi2PerClusterTPC(-FLT_MAX),
  fCutNClustersTPC(kFALSE),
  fNClustersTPCMax(INT_MAX),
  fNClustersTPCMin(INT_MIN),  
  fParamType(kGlobal),
  fParamMix(kPure),
  fTrack(NULL),
  fTrackPhi(0.),
  fTrackEta(0.),
  fTrackWeight(0.),
  fTrackLabel(INT_MIN),
  fMCevent(NULL),
  fMCparticle(NULL),
  fEvent(NULL),
  fTPCtrack(),
  fESDpid(NULL),
  fPIDsource(kTPCTOFpid),
  fTPCpidCuts(NULL),
  fTOFpidCuts(NULL),
  fTPCTOFpidCrossOverPt(0.4),
  fAliPID(AliPID::kPion)
{
  //io constructor 
}

//-----------------------------------------------------------------------
AliFlowTrackCuts::AliFlowTrackCuts(const char* name):
  AliFlowTrackSimpleCuts(),
  fAliESDtrackCuts(new AliESDtrackCuts()),
  fQA(NULL),
  fCutMC(kFALSE),
  fCutMCprocessType(kFALSE),
  fMCprocessType(kPNoProcess),
  fCutMCPID(kFALSE),
  fMCPID(0),
  fIgnoreSignInPID(kFALSE),
  fCutMCisPrimary(kFALSE),
  fRequireTransportBitForPrimaries(kTRUE),
  fMCisPrimary(kFALSE),
  fRequireCharge(kFALSE),
  fFakesAreOK(kTRUE),
  fCutSPDtrackletDeltaPhi(kFALSE),
  fSPDtrackletDeltaPhiMax(FLT_MAX),
  fSPDtrackletDeltaPhiMin(-FLT_MAX),
  fIgnoreTPCzRange(kFALSE),
  fIgnoreTPCzRangeMax(FLT_MAX),
  fIgnoreTPCzRangeMin(-FLT_MAX),
  fCutChi2PerClusterTPC(kFALSE),
  fMaxChi2PerClusterTPC(FLT_MAX),
  fMinChi2PerClusterTPC(-FLT_MAX),
  fCutNClustersTPC(kFALSE),
  fNClustersTPCMax(INT_MAX),
  fNClustersTPCMin(INT_MIN),  
  fParamType(kGlobal),
  fParamMix(kPure),
  fTrack(NULL),
  fTrackPhi(0.),
  fTrackEta(0.),
  fTrackWeight(0.),
  fTrackLabel(INT_MIN),
  fMCevent(NULL),
  fMCparticle(NULL),
  fEvent(NULL),
  fTPCtrack(),
  fESDpid(NULL),
  fPIDsource(kTPCTOFpid),
  fTPCpidCuts(NULL),
  fTOFpidCuts(NULL),
  fTPCTOFpidCrossOverPt(0.4),
  fAliPID(AliPID::kPion)
{
  //constructor 
  SetName(name);
  SetTitle("AliFlowTrackCuts");
}

//-----------------------------------------------------------------------
AliFlowTrackCuts::AliFlowTrackCuts(const AliFlowTrackCuts& that):
  AliFlowTrackSimpleCuts(that),
  fAliESDtrackCuts(NULL),
  fQA(NULL),
  fCutMC(that.fCutMC),
  fCutMCprocessType(that.fCutMCprocessType),
  fMCprocessType(that.fMCprocessType),
  fCutMCPID(that.fCutMCPID),
  fMCPID(that.fMCPID),
  fIgnoreSignInPID(that.fIgnoreSignInPID),
  fCutMCisPrimary(that.fCutMCisPrimary),
  fRequireTransportBitForPrimaries(that.fRequireTransportBitForPrimaries),
  fMCisPrimary(that.fMCisPrimary),
  fRequireCharge(that.fRequireCharge),
  fFakesAreOK(that.fFakesAreOK),
  fCutSPDtrackletDeltaPhi(that.fCutSPDtrackletDeltaPhi),
  fSPDtrackletDeltaPhiMax(that.fSPDtrackletDeltaPhiMax),
  fSPDtrackletDeltaPhiMin(that.fSPDtrackletDeltaPhiMin),
  fIgnoreTPCzRange(that.fIgnoreTPCzRange),
  fIgnoreTPCzRangeMax(that.fIgnoreTPCzRangeMax),
  fIgnoreTPCzRangeMin(that.fIgnoreTPCzRangeMin),
  fCutChi2PerClusterTPC(that.fCutChi2PerClusterTPC),
  fMaxChi2PerClusterTPC(that.fMaxChi2PerClusterTPC),
  fMinChi2PerClusterTPC(that.fMinChi2PerClusterTPC),
  fCutNClustersTPC(that.fCutNClustersTPC),
  fNClustersTPCMax(that.fNClustersTPCMax),
  fNClustersTPCMin(that.fNClustersTPCMin),
  fParamType(that.fParamType),
  fParamMix(that.fParamMix),
  fTrack(NULL),
  fTrackPhi(0.),
  fTrackEta(0.),
  fTrackWeight(0.),
  fTrackLabel(INT_MIN),
  fMCevent(NULL),
  fMCparticle(NULL),
  fEvent(NULL),
  fTPCtrack(),
  fESDpid(that.fESDpid),
  fPIDsource(that.fPIDsource),
  fTPCpidCuts(NULL),
  fTOFpidCuts(NULL),
  fTPCTOFpidCrossOverPt(that.fTPCTOFpidCrossOverPt),
  fAliPID(that.fAliPID)
{
  //copy constructor
  if (that.fTPCpidCuts) fTPCpidCuts = new TMatrixF(*(that.fTPCpidCuts));
  if (that.fTOFpidCuts) fTOFpidCuts = new TMatrixF(*(that.fTOFpidCuts));
  if (that.fAliESDtrackCuts) fAliESDtrackCuts = new AliESDtrackCuts(*(that.fAliESDtrackCuts));
}

//-----------------------------------------------------------------------
AliFlowTrackCuts& AliFlowTrackCuts::operator=(const AliFlowTrackCuts& that)
{
  //assignment
  AliFlowTrackSimpleCuts::operator=(that);
  if (that.fAliESDtrackCuts) *fAliESDtrackCuts=*(that.fAliESDtrackCuts);
  fQA=NULL;
  fCutMC=that.fCutMC;
  fCutMCprocessType=that.fCutMCprocessType;
  fMCprocessType=that.fMCprocessType;
  fCutMCPID=that.fCutMCPID;
  fMCPID=that.fMCPID;
  fIgnoreSignInPID=that.fIgnoreSignInPID,
  fCutMCisPrimary=that.fCutMCisPrimary;
  fRequireTransportBitForPrimaries=that.fRequireTransportBitForPrimaries;
  fMCisPrimary=that.fMCisPrimary;
  fRequireCharge=that.fRequireCharge;
  fFakesAreOK=that.fFakesAreOK;
  fCutSPDtrackletDeltaPhi=that.fCutSPDtrackletDeltaPhi;
  fSPDtrackletDeltaPhiMax=that.fSPDtrackletDeltaPhiMax;
  fSPDtrackletDeltaPhiMin=that.fSPDtrackletDeltaPhiMin;
  fIgnoreTPCzRange=that.fIgnoreTPCzRange;
  fIgnoreTPCzRangeMax=that.fIgnoreTPCzRangeMax;
  fIgnoreTPCzRangeMin=that.fIgnoreTPCzRangeMin;
  fCutChi2PerClusterTPC=that.fCutChi2PerClusterTPC;
  fMaxChi2PerClusterTPC=that.fMaxChi2PerClusterTPC;
  fMinChi2PerClusterTPC=that.fMinChi2PerClusterTPC;
  fCutNClustersTPC=that.fCutNClustersTPC;
  fNClustersTPCMax=that.fNClustersTPCMax;
  fNClustersTPCMin=that.fNClustersTPCMin;  
  fParamType=that.fParamType;
  fParamMix=that.fParamMix;

  fTrack=NULL;
  fTrackPhi=0.;
  fTrackPhi=0.;
  fTrackWeight=0.;
  fTrackLabel=INT_MIN;
  fMCevent=NULL;
  fMCparticle=NULL;
  fEvent=NULL;

  fESDpid = that.fESDpid;
  fPIDsource = that.fPIDsource;

  if (that.fTPCpidCuts) fTPCpidCuts = new TMatrixF(*(that.fTPCpidCuts));
  if (that.fTOFpidCuts) fTOFpidCuts = new TMatrixF(*(that.fTOFpidCuts));
  fTPCTOFpidCrossOverPt=that.fTPCTOFpidCrossOverPt;

  fAliPID=that.fAliPID;

  return *this;
}

//-----------------------------------------------------------------------
AliFlowTrackCuts::~AliFlowTrackCuts()
{
  //dtor
  delete fAliESDtrackCuts;
  delete fTPCpidCuts;
  delete fTOFpidCuts;
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::IsSelected(TObject* obj, Int_t id)
{
  //check cuts
  AliVParticle* vparticle = dynamic_cast<AliVParticle*>(obj);
  if (vparticle) return PassesCuts(vparticle);
  AliFlowTrackSimple* flowtrack = dynamic_cast<AliFlowTrackSimple*>(obj);
  if (flowtrack) return PassesCuts(flowtrack);
  AliMultiplicity* tracklets = dynamic_cast<AliMultiplicity*>(obj);
  if (tracklets) return PassesCuts(tracklets,id);
  return kFALSE;  //default when passed wrong type of object
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::IsSelectedMCtruth(TObject* obj, Int_t id)
{
  //check cuts
  AliVParticle* vparticle = dynamic_cast<AliVParticle*>(obj);
  if (vparticle) 
  {
    return PassesMCcuts(fMCevent,vparticle->GetLabel());
  }
  AliMultiplicity* tracklets = dynamic_cast<AliMultiplicity*>(obj);
  if (tracklets)
  {
    Int_t label0 = tracklets->GetLabel(id,0);
    Int_t label1 = tracklets->GetLabel(id,1);
    Int_t label = (label0==label1)?tracklets->GetLabel(id,1):-666;
    return PassesMCcuts(fMCevent,label);
  }
  return kFALSE;  //default when passed wrong type of object
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesCuts(AliFlowTrackSimple* track)
{
  //check cuts on a flowtracksimple

  //clean up from last iteration
  fTrack = NULL;
  return AliFlowTrackSimpleCuts::PassesCuts(track);
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesCuts(AliMultiplicity* tracklet, Int_t id)
{
  //check cuts on a tracklets

  //clean up from last iteration, and init label
  fTrack = NULL;
  fMCparticle=NULL;
  fTrackLabel=-1;

  fTrackPhi = tracklet->GetPhi(id);
  fTrackEta = tracklet->GetEta(id);
  fTrackWeight = 1.0;
  if (fCutEta) {if (  fTrackEta < fEtaMin || fTrackEta >= fEtaMax ) return kFALSE;}
  if (fCutPhi) {if ( fTrackPhi < fPhiMin || fTrackPhi >= fPhiMax ) return kFALSE;}

  //check MC info if available
  //if the 2 clusters have different label track cannot be good
  //and should therefore not pass the mc cuts
  Int_t label0 = tracklet->GetLabel(id,0);
  Int_t label1 = tracklet->GetLabel(id,1);
  //if possible get label and mcparticle
  fTrackLabel = (label0==label1)?tracklet->GetLabel(id,1):-1;
  if (!fFakesAreOK && fTrackLabel<0) return kFALSE;
  if (fTrackLabel>=0 && fMCevent) fMCparticle = static_cast<AliMCParticle*>(fMCevent->GetTrack(fTrackLabel));
  //check MC cuts
  if (fCutMC && !PassesMCcuts()) return kFALSE;
  return kTRUE;
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesMCcuts(AliMCEvent* mcEvent, Int_t label)
{
  //check the MC info
  if (!mcEvent) return kFALSE;
  if (label<0) return kFALSE;//otherwise AliCMevent prints a warning before returning NULL
  AliMCParticle* mcparticle = static_cast<AliMCParticle*>(mcEvent->GetTrack(label));
  if (!mcparticle) {AliError("no MC track"); return kFALSE;}

  if (fCutMCisPrimary)
  {
    if (IsPhysicalPrimary(mcEvent,label,fRequireTransportBitForPrimaries) != fMCisPrimary) return kFALSE;
  }
  if (fCutMCPID)
  {
    Int_t pdgCode = mcparticle->PdgCode();
    if (fIgnoreSignInPID) 
    {
      if (TMath::Abs(fMCPID) != TMath::Abs(pdgCode)) return kFALSE;
    }
    else 
    {
      if (fMCPID != pdgCode) return kFALSE;
    }
  }
  if ( fCutMCprocessType )
  {
    TParticle* particle = mcparticle->Particle();
    Int_t processID = particle->GetUniqueID();
    if (processID != fMCprocessType ) return kFALSE;
  }
  return kTRUE;
}
//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesMCcuts()
{
  if (!fMCevent) return kFALSE;
  if (fTrackLabel<0) return kFALSE;//otherwise AliCMevent prints a warning before returning NULL
  fMCparticle = static_cast<AliMCParticle*>(fMCevent->GetTrack(fTrackLabel));
  return PassesMCcuts(fMCevent,fTrackLabel);
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesCuts(AliVParticle* vparticle)
{
  //check cuts for an ESD vparticle

  ////////////////////////////////////////////////////////////////
  //  start by preparing the track parameters to cut on //////////
  ////////////////////////////////////////////////////////////////
  //clean up from last iteration
  fTrack=NULL; 

  //get the label and the mc particle
  fTrackLabel = (fFakesAreOK)?TMath::Abs(vparticle->GetLabel()):vparticle->GetLabel();
  if (fMCevent) fMCparticle = static_cast<AliMCParticle*>(fMCevent->GetTrack(fTrackLabel));
  else fMCparticle=NULL;

  Bool_t isMCparticle = kFALSE; //some things are different for MC particles, check!
  AliESDtrack* esdTrack = dynamic_cast<AliESDtrack*>(vparticle);
  if (esdTrack)
    HandleESDtrack(esdTrack);
  else
  {
    HandleVParticle(vparticle);
    //now check if produced particle is MC
    isMCparticle = (dynamic_cast<AliMCParticle*>(fTrack))!=NULL;
  }
  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  if (!fTrack) return kFALSE;
  if (esdTrack) esdTrack=static_cast<AliESDtrack*>(fTrack); //because it may be different from global
  
  Bool_t pass=kTRUE;
  //check the common cuts for the current particle fTrack (MC,AOD,ESD)
  Double_t pt = fTrack->Pt();
  if (!fFakesAreOK) {if (fTrackLabel<0) pass=kFALSE;}
  if (fCutPt) {if (pt < fPtMin || pt >= fPtMax ) pass=kFALSE;}
  if (fCutEta) {if (fTrack->Eta() < fEtaMin || fTrack->Eta() >= fEtaMax ) pass=kFALSE;}
  if (fCutPhi) {if (fTrack->Phi() < fPhiMin || fTrack->Phi() >= fPhiMax ) pass=kFALSE;}
  if (fRequireCharge) {if (fTrack->Charge() == 0) pass=kFALSE;}
  if (fCutCharge && !isMCparticle) {if (fTrack->Charge() != fCharge) pass=kFALSE;}
  if (fCutCharge && isMCparticle)
  { 
    //in case of an MC particle the charge is stored in units of 1/3|e| 
    Int_t charge = TMath::Nint(fTrack->Charge()/3.0); //mc particles have charge in units of 1/3e
    if (charge!=fCharge) pass=kFALSE;
  }
  //if(fCutPID) {if (fTrack->PID() != fPID) pass=kFALSE;}

  //when additionally MC info is required
  if (fCutMC && !PassesMCcuts()) pass=kFALSE;

  //check all else for ESDs using aliesdtrackcuts
  if (esdTrack && (fParamType!=kMC) ) 
  {
    if (fIgnoreTPCzRange)
    {
      const AliExternalTrackParam* pin = esdTrack->GetOuterParam();
      const AliExternalTrackParam* pout = esdTrack->GetInnerParam();
      if (pin&&pout)
      {
        Double_t zin = pin->GetZ();
        Double_t zout = pout->GetZ();
        if (zin*zout<0) pass=kFALSE;   //reject if cross the membrane
        if (zin < fIgnoreTPCzRangeMin || zin > fIgnoreTPCzRangeMax) pass=kFALSE;
        if (zout < fIgnoreTPCzRangeMin || zout > fIgnoreTPCzRangeMax) pass=kFALSE;
      }
    }
 
    if (fAliESDtrackCuts)
    {
      if (!fAliESDtrackCuts->IsSelected(static_cast<AliESDtrack*>(fTrack))) pass=kFALSE;
    }
 
    Int_t ntpccls = ( fParamType==kESD_TPConly )?
                      esdTrack->GetTPCNclsIter1():esdTrack->GetTPCNcls();    
    if (fCutChi2PerClusterTPC)
    {
      Float_t tpcchi2 = (fParamType==kESD_TPConly)?
                         esdTrack->GetTPCchi2Iter1():esdTrack->GetTPCchi2();
      tpcchi2 = (ntpccls>0)?tpcchi2/ntpccls:-FLT_MAX;
      if (tpcchi2<fMinChi2PerClusterTPC || tpcchi2 >=fMaxChi2PerClusterTPC)
        pass=kFALSE;
    }

    if (fCutNClustersTPC)
    {
      if (ntpccls < fNClustersTPCMin || ntpccls > fNClustersTPCMax) pass=kFALSE;
    }

    if (fCutPID)
    {
      switch (fPIDsource)    
      {
        case kTPCpid:
          if (!PassesTPCpidCut(esdTrack)) pass=kFALSE;
          break;
        case kTOFpid:
          if (!PassesTOFpidCut(esdTrack)) pass=kFALSE;
          break;
        case kTPCTOFpid:
          if (pt< fTPCTOFpidCrossOverPt)
          {
            if (!PassesTPCpidCut(esdTrack)) pass=kFALSE;
          }
          else //if (pt>=fTPCTOFpidCrossOverPt)
          {
            if (!PassesTOFpidCut(esdTrack)) pass=kFALSE;
          }
          break;
        default:
          printf("AliFlowTrackCuts::PassesCuts() this should never be called!\n");
          pass=kFALSE;
          break;
      }
    }    
  }

  return pass; //true by default, if we didn't set any cuts
}

//-----------------------------------------------------------------------
void AliFlowTrackCuts::HandleVParticle(AliVParticle* track)
{
  //handle the general case
  switch (fParamType)
  {
    default:
      fTrack = track;
      break;
  }
}

//-----------------------------------------------------------------------
void AliFlowTrackCuts::HandleESDtrack(AliESDtrack* track)
{
  //handle esd track
  switch (fParamType)
  {
    case kGlobal:
      fTrack = track;
      break;
    case kESD_TPConly:
      if (!track->FillTPCOnlyTrack(fTPCtrack))
      {
        fTrack=NULL;
        fMCparticle=NULL;
        fTrackLabel=-1;
        return;
      }
      fTrack = &fTPCtrack;
      //recalculate the label and mc particle, they may differ as TPClabel != global label
      fTrackLabel = (fFakesAreOK)?TMath::Abs(fTrack->GetLabel()):fTrack->GetLabel();
      if (fMCevent) fMCparticle = static_cast<AliMCParticle*>(fMCevent->GetTrack(fTrackLabel));
      else fMCparticle=NULL;
      break;
    default:
      fTrack = track;
      break;
  }
}

//-----------------------------------------------------------------------
AliFlowTrackCuts* AliFlowTrackCuts::GetStandardTPCOnlyTrackCuts()
{
  //get standard cuts
  AliFlowTrackCuts* cuts = new AliFlowTrackCuts("standard TPConly cuts");
  delete cuts->fAliESDtrackCuts;
  cuts->fAliESDtrackCuts = AliESDtrackCuts::GetStandardTPCOnlyTrackCuts();
  cuts->SetParamType(kESD_TPConly);
  return cuts;
}

//-----------------------------------------------------------------------
AliFlowTrackCuts* AliFlowTrackCuts::GetStandardITSTPCTrackCuts2009(Bool_t selPrimaries)
{
  //get standard cuts
  AliFlowTrackCuts* cuts = new AliFlowTrackCuts("standard global track cuts 2009");
  delete cuts->fAliESDtrackCuts;
  cuts->fAliESDtrackCuts = AliESDtrackCuts::GetStandardITSTPCTrackCuts2009(selPrimaries);
  cuts->SetParamType(kGlobal);
  return cuts;
}

//-----------------------------------------------------------------------
AliFlowTrack* AliFlowTrackCuts::MakeFlowTrack() const
{
  //get a flow track constructed from whatever we applied cuts on
  //caller is resposible for deletion
  //if construction fails return NULL
  AliFlowTrack* flowtrack=NULL;
  TParticle *tmpTParticle=NULL;
  AliMCParticle* tmpAliMCParticle=NULL;
  if (fParamType==kESD_SPDtracklet)
  {
    switch (fParamMix)
    {
      case kPure:
        flowtrack = new AliFlowTrack();
        flowtrack->SetPhi(fTrackPhi);
        flowtrack->SetEta(fTrackEta);
        break;
      case kTrackWithMCkine:
        if (!fMCparticle) return NULL;
        flowtrack = new AliFlowTrack();
        flowtrack->SetPhi( fMCparticle->Phi() );
        flowtrack->SetEta( fMCparticle->Eta() );
        flowtrack->SetPt( fMCparticle->Pt() );
        break;
      case kTrackWithMCpt:
        if (!fMCparticle) return NULL;
        flowtrack = new AliFlowTrack();
        flowtrack->SetPhi(fTrackPhi);
        flowtrack->SetEta(fTrackEta);
        flowtrack->SetPt(fMCparticle->Pt());
        break;
      case kTrackWithPtFromFirstMother:
        if (!fMCparticle) return NULL;
        flowtrack = new AliFlowTrack();
        flowtrack->SetPhi(fTrackPhi);
        flowtrack->SetEta(fTrackEta);
        tmpTParticle = fMCparticle->Particle();
        tmpAliMCParticle = static_cast<AliMCParticle*>(fMCevent->GetTrack(tmpTParticle->GetFirstMother()));
        flowtrack->SetPt(tmpAliMCParticle->Pt());
        break;
      default:
        flowtrack = new AliFlowTrack();
        flowtrack->SetPhi(fTrackPhi);
        flowtrack->SetEta(fTrackEta);
        break;
    }
    flowtrack->SetSource(AliFlowTrack::kFromTracklet);
  }
  else
  {
    if (!fTrack) return NULL;
    switch(fParamMix)
    {
      case kPure:
        flowtrack = new AliFlowTrack(fTrack);
        break;
      case kTrackWithMCkine:
        flowtrack = new AliFlowTrack(fMCparticle);
        break;
      case kTrackWithMCPID:
        flowtrack = new AliFlowTrack(fTrack);
        //flowtrack->setPID(...) from mc, when implemented
        break;
      case kTrackWithMCpt:
        if (!fMCparticle) return NULL;
        flowtrack = new AliFlowTrack(fTrack);
        flowtrack->SetPt(fMCparticle->Pt());
        break;
      case kTrackWithPtFromFirstMother:
        if (!fMCparticle) return NULL;
        flowtrack = new AliFlowTrack(fTrack);
        tmpTParticle = fMCparticle->Particle();
        tmpAliMCParticle = static_cast<AliMCParticle*>(fMCevent->GetTrack(tmpTParticle->GetFirstMother()));
        flowtrack->SetPt(tmpAliMCParticle->Pt());
        break;
      default:
        flowtrack = new AliFlowTrack(fTrack);
        break;
    }
    if (fParamType==kMC) flowtrack->SetSource(AliFlowTrack::kFromMC);
    else if (dynamic_cast<AliESDtrack*>(fTrack)) flowtrack->SetSource(AliFlowTrack::kFromESD);
    else if (dynamic_cast<AliAODTrack*>(fTrack)) flowtrack->SetSource(AliFlowTrack::kFromAOD);
    else if (dynamic_cast<AliMCParticle*>(fTrack)) flowtrack->SetSource(AliFlowTrack::kFromMC);
  }
  return flowtrack;
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::IsPhysicalPrimary() const
{
  //check if current particle is a physical primary
  if (!fMCevent) return kFALSE;
  if (fTrackLabel<0) return kFALSE;
  return IsPhysicalPrimary(fMCevent, fTrackLabel, fRequireTransportBitForPrimaries);
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::IsPhysicalPrimary(AliMCEvent* mcEvent, Int_t label, Bool_t requiretransported)
{
  //check if current particle is a physical primary
  Bool_t physprim=mcEvent->IsPhysicalPrimary(label);
  AliMCParticle* track = static_cast<AliMCParticle*>(mcEvent->GetTrack(label));
  if (!track) return kFALSE;
  TParticle* particle = track->Particle();
  Bool_t transported = particle->TestBit(kTransportBit);
  //printf("label: %i prim: %s, transp: %s, pass: %s\n",label, (physprim)?"YES":"NO ",(transported)?"YES":"NO ",
        //(physprim && (transported || !requiretransported))?"YES":"NO"  );
  return (physprim && (transported || !requiretransported));
}

//-----------------------------------------------------------------------
const char* AliFlowTrackCuts::GetParamTypeName(trackParameterType type) 
{
  //return the name of the selected parameter type
  switch (type)
  {
    case kMC:
      return "MC";
    case kGlobal:
      return "ESD global";
    case kESD_TPConly:
      return "TPC only";
    case kESD_SPDtracklet:
      return "SPD tracklet";
    default:
      return "unknown";
  }
}

//-----------------------------------------------------------------------
void AliFlowTrackCuts::DefineHistograms()
{
  //define qa histograms
}

//-----------------------------------------------------------------------
Int_t AliFlowTrackCuts::GetNumberOfInputObjects() const
{
  //get the number of tracks in the input event according source
  //selection (ESD tracks, tracklets, MC particles etc.)
  AliESDEvent* esd=NULL;
  switch (fParamType)
  {
    case kESD_SPDtracklet:
      esd = dynamic_cast<AliESDEvent*>(fEvent);
      if (!esd) return 0;
      return esd->GetMultiplicity()->GetNumberOfTracklets();
    case kMC:
      if (!fMCevent) return 0;
      return fMCevent->GetNumberOfTracks();
    default:
      if (!fEvent) return 0;
      return fEvent->GetNumberOfTracks();
  }
  return 0;
}

//-----------------------------------------------------------------------
TObject* AliFlowTrackCuts::GetInputObject(Int_t i)
{
  //get the input object according the data source selection:
  //(esd tracks, traclets, mc particles,etc...)
  AliESDEvent* esd=NULL;
  switch (fParamType)
  {
    case kESD_SPDtracklet:
      esd = dynamic_cast<AliESDEvent*>(fEvent);
      if (!esd) return NULL;
      return const_cast<AliMultiplicity*>(esd->GetMultiplicity());
    case kMC:
      if (!fMCevent) return NULL;
      return fMCevent->GetTrack(i);
    default:
      if (!fEvent) return NULL;
      return fEvent->GetTrack(i);
  }
}

//-----------------------------------------------------------------------
void AliFlowTrackCuts::Clear(Option_t*)
{
  //clean up
  fTrack=NULL;
  fMCevent=NULL;
  fMCparticle=NULL;
  fTrackLabel=0;
  fTrackWeight=0.0;
  fTrackEta=0.0;
  fTrackPhi=0.0;
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesTOFpidCut(AliESDtrack* t )
{
  //check if passes PID cut using timing in TOF
  if (!fESDpid) return kFALSE;
  if (!(t && (t->GetStatus() & AliESDtrack::kTOFout) && (t->GetStatus() & AliESDtrack::kTIME)
       && (t->GetTOFsignal() > 12000) && (t->GetTOFsignal() < 100000) && (t->GetIntegratedLength() > 365)))
       return kFALSE;
  Float_t pt = t->Pt();
  Float_t p = t->GetP();
  Float_t trackT0 = fESDpid->GetTOFResponse().GetStartTime(p);
  Float_t timeTOF = t->GetTOFsignal()- trackT0; 
  //2=pion 3=kaon 4=protons
  Double_t inttimes[5] = {-1.0,-1.0,-1.0,-1.0,-1.0};
  t->GetIntegratedTimes(inttimes);
  //construct the pid index because it's screwed up in TOF
  Int_t pid = 0;
  switch (fAliPID)
  {
    case AliPID::kPion:
      pid=2;
      break;
    case AliPID::kKaon:
      pid=3;
      break;
    case AliPID::kProton:
      pid=4;
      break;
    default:
      return kFALSE;
  }
  Float_t s = timeTOF-inttimes[pid];

  Float_t* arr = fTOFpidCuts->GetMatrixArray();
  Int_t col = TMath::BinarySearch(fTOFpidCuts->GetNcols(),arr,static_cast<Float_t>(pt));
  if (col<0) return kFALSE;
  Float_t min = (*fTOFpidCuts)(1,col);
  Float_t max = (*fTOFpidCuts)(2,col);

  //printf("--------------TOF pid cut %s\n",(s>min && s<max)?"PASS":"FAIL");
  return (s>min && s<max);
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesTPCpidCut(AliESDtrack* track)
{
  //check if passes PID cut using dedx signal in the TPC
  if (!fESDpid) 
  {
    return kFALSE;
  }
  if (!fTPCpidCuts)
  {
    printf("no TPCpidCuts\n");
    return kFALSE;
  }

  const AliExternalTrackParam* tpcparam = track->GetInnerParam();
  if (!tpcparam) return kFALSE;
  Float_t sigExp = fESDpid->GetTPCResponse().GetExpectedSignal(tpcparam->GetP(), fAliPID);
  Float_t sigTPC = track->GetTPCsignal();
  Float_t s = (sigTPC-sigExp)/sigExp;
  Double_t pt = track->Pt();

  Float_t* arr = fTPCpidCuts->GetMatrixArray();
  Int_t col = TMath::BinarySearch(fTPCpidCuts->GetNcols(),arr,static_cast<Float_t>(pt));
  if (col<0) return kFALSE;
  Float_t min = (*fTPCpidCuts)(1,col);
  Float_t max = (*fTPCpidCuts)(2,col);

  //printf("------------TPC pid cut %s\n",(s>min && s<max)?"PASS":"FAIL");
  return (s>min && s<max);
}

//-----------------------------------------------------------------------
void AliFlowTrackCuts::InitPIDcuts()
{
  //init matrices with PID cuts
  TMatrixF* t = NULL;
  if (!fTPCpidCuts)
  {
    if (fAliPID==AliPID::kPion)
    {
      t = new TMatrixF(3,10);
      (*t)(0,0)  = 0.20;  (*t)(1,0)  = -0.4;  (*t)(2,0)  =   0.2;
      (*t)(0,1)  = 0.25;  (*t)(1,1)  = -0.4;  (*t)(2,1)  =   0.2;
      (*t)(0,2)  = 0.30;  (*t)(1,2)  = -0.4;  (*t)(2,2)  =  0.25;
      (*t)(0,3)  = 0.35;  (*t)(1,3)  = -0.4;  (*t)(2,3)  =  0.25;
      (*t)(0,4)  = 0.40;  (*t)(1,4)  = -0.4;  (*t)(2,4)  =   0.3;
      (*t)(0,5)  = 0.45;  (*t)(1,5)  = -0.4;  (*t)(2,5)  =   0.3;
      (*t)(0,6)  = 0.50;  (*t)(1,6)  = -0.4;  (*t)(2,6)  =  0.25;
      (*t)(0,7)  = 0.55;  (*t)(1,7)  = -0.4;  (*t)(2,7)  =  0.15;
      (*t)(0,8)  = 0.60;  (*t)(1,8)  = -0.4;  (*t)(2,8)  =   0.1;
      (*t)(0,9)  = 0.65;  (*t)(1,9)  =    0;  (*t)(2,9)  =     0;
    }
    else
    if (fAliPID==AliPID::kKaon)
    {
      t = new TMatrixF(3,7);
      (*t)(0,0)  = 0.20;  (*t)(1,0)  = -0.2;  (*t)(2,0)  = 0.4; 
      (*t)(0,1)  = 0.25;  (*t)(1,1)  =-0.15;  (*t)(2,1)  = 0.4;
      (*t)(0,2)  = 0.30;  (*t)(1,2)  = -0.1;  (*t)(2,2)  = 0.4;
      (*t)(0,3)  = 0.35;  (*t)(1,3)  = -0.1;  (*t)(2,3)  = 0.4;
      (*t)(0,4)  = 0.40;  (*t)(1,4)  = -0.1;  (*t)(2,4)  = 0.6;
      (*t)(0,5)  = 0.45;  (*t)(1,5)  = -0.1;  (*t)(2,5)  = 0.6;
      (*t)(0,6)  = 0.50;  (*t)(1,6)  =    0;  (*t)(2,6)  =   0;
    }
    else
    if (fAliPID==AliPID::kProton)
    {
      t = new TMatrixF(3,16);
      (*t)(0,0)  = 0.20;  (*t)(1,0)  =     0;  (*t)(2,0)  =    0; 
      (*t)(0,1)  = 0.25;  (*t)(1,1)  =  -0.2;  (*t)(2,1)  =  0.3; 
      (*t)(0,2)  = 0.30;  (*t)(1,2)  =  -0.2;  (*t)(2,2)  =  0.6; 
      (*t)(0,3)  = 0.35;  (*t)(1,3)  =  -0.2;  (*t)(2,3)  =  0.6; 
      (*t)(0,4)  = 0.40;  (*t)(1,4)  =  -0.2;  (*t)(2,4)  =  0.6; 
      (*t)(0,5)  = 0.45;  (*t)(1,5)  = -0.15;  (*t)(2,5)  =  0.6; 
      (*t)(0,6)  = 0.50;  (*t)(1,6)  =  -0.1;  (*t)(2,6)  =  0.6; 
      (*t)(0,7)  = 0.55;  (*t)(1,7)  = -0.05;  (*t)(2,7)  =  0.6; 
      (*t)(0,8)  = 0.60;  (*t)(1,8)  = -0.05;  (*t)(2,8)  = 0.45; 
      (*t)(0,9)  = 0.65;  (*t)(1,9)  = -0.05;  (*t)(2,9)  = 0.45; 
      (*t)(0,10) = 0.70;  (*t)(1,10) = -0.05;  (*t)(2,10) = 0.45; 
      (*t)(0,11) = 0.75;  (*t)(1,11) = -0.05;  (*t)(2,11) = 0.45; 
      (*t)(0,12) = 0.80;  (*t)(1,12) =     0;  (*t)(2,12) = 0.45; 
      (*t)(0,13) = 0.85;  (*t)(1,13) =     0;  (*t)(2,13) = 0.45; 
      (*t)(0,14) = 0.90;  (*t)(1,14) =     0;  (*t)(2,14) = 0.45;
      (*t)(0,15) = 0.95;  (*t)(1,15) =     0;  (*t)(2,15) =    0;
    }
    fTPCpidCuts=t;
  }
  t = NULL;
  if (!fTOFpidCuts)
  {
    if (fAliPID==AliPID::kPion)
    {
      t = new TMatrixF(3,27);
      (*t)(0,0)  = 0.3;   (*t)(1,0)  = -700;  (*t)(2,0)  = 700;
      (*t)(0,1)  = 0.35;  (*t)(1,1)  = -800;  (*t)(2,1)  = 800;
      (*t)(0,2)  = 0.40;  (*t)(1,2)  = -600;  (*t)(2,2)  = 800;
      (*t)(0,3)  = 0.45;  (*t)(1,3)  = -500;  (*t)(2,3)  = 700;
      (*t)(0,4)  = 0.50;  (*t)(1,4)  = -400;  (*t)(2,4)  = 700;
      (*t)(0,5)  = 0.55;  (*t)(1,5)  = -400;  (*t)(2,5)  = 700;
      (*t)(0,6)  = 0.60;  (*t)(1,6)  = -400;  (*t)(2,6)  = 700;
      (*t)(0,7)  = 0.65;  (*t)(1,7)  = -400;  (*t)(2,7)  = 700;
      (*t)(0,8)  = 0.70;  (*t)(1,8)  = -400;  (*t)(2,8)  = 700;
      (*t)(0,9)  = 0.75;  (*t)(1,9)  = -400;  (*t)(2,9)  = 700;
      (*t)(0,10) = 0.80;  (*t)(1,10) = -400;  (*t)(2,10) = 600;
      (*t)(0,11) = 0.85;  (*t)(1,11) = -400;  (*t)(2,11) = 600;
      (*t)(0,12) = 0.90;  (*t)(1,12) = -400;  (*t)(2,12) = 600;
      (*t)(0,13) = 0.95;  (*t)(1,13) = -400;  (*t)(2,13) = 600;
      (*t)(0,14) = 1.00;  (*t)(1,14) = -400;  (*t)(2,14) = 550;
      (*t)(0,15) = 1.10;  (*t)(1,15) = -400;  (*t)(2,15) = 450;
      (*t)(0,16) = 1.20;  (*t)(1,16) = -400;  (*t)(2,16) = 400;
      (*t)(0,17) = 1.30;  (*t)(1,17) = -400;  (*t)(2,17) = 300;
      (*t)(0,18) = 1.40;  (*t)(1,18) = -400;  (*t)(2,18) = 300;
      (*t)(0,19) = 1.50;  (*t)(1,19) = -400;  (*t)(2,19) = 250;
      (*t)(0,20) = 1.60;  (*t)(1,20) = -400;  (*t)(2,20) = 200;
      (*t)(0,21) = 1.70;  (*t)(1,21) = -400;  (*t)(2,21) = 150;
      (*t)(0,22) = 1.80;  (*t)(1,22) = -400;  (*t)(2,22) = 100;
      (*t)(0,23) = 1.90;  (*t)(1,23) = -400;  (*t)(2,23) =  70;
      (*t)(0,24) = 2.00;  (*t)(1,24) = -400;  (*t)(2,24) =  50;
      (*t)(0,25) = 2.10;  (*t)(1,25) = -400;  (*t)(2,25) =   0;
      (*t)(0,26) = 2.20;  (*t)(1,26) =    0;  (*t)(2,26) =   0;
    }
    else
    if (fAliPID==AliPID::kProton)
    {
      t = new TMatrixF(3,39);
      (*t)(0,0)  = 0.3;  (*t)(1,0)   = 0;     (*t)(2,0) = 0;
      (*t)(0,1)  = 0.35;  (*t)(1,1)  = 0;     (*t)(2,1) = 0;
      (*t)(0,2)  = 0.40;  (*t)(1,2)  = 0;     (*t)(2,2) = 0;
      (*t)(0,3)  = 0.45;  (*t)(1,3)  = 0;     (*t)(2,3) = 0;
      (*t)(0,4)  = 0.50;  (*t)(1,4)  = 0;     (*t)(2,4) = 0;
      (*t)(0,5)  = 0.55;  (*t)(1,5)  = -900;  (*t)(2,5)  = 600;
      (*t)(0,6)  = 0.60;  (*t)(1,6)  = -800;  (*t)(2,6)  = 600;
      (*t)(0,7)  = 0.65;  (*t)(1,7)  = -800;  (*t)(2,7)  = 600;
      (*t)(0,8)  = 0.70;  (*t)(1,8)  = -800;  (*t)(2,8)  = 600;
      (*t)(0,9)  = 0.75;  (*t)(1,9)  = -700;  (*t)(2,9)  = 500;
      (*t)(0,10) = 0.80;  (*t)(1,10) = -700;  (*t)(2,10) = 500;
      (*t)(0,11) = 0.85;  (*t)(1,11) = -700;  (*t)(2,11) = 500;
      (*t)(0,12) = 0.90;  (*t)(1,12) = -600;  (*t)(2,12) = 500;
      (*t)(0,13) = 0.95;  (*t)(1,13) = -600;  (*t)(2,13) = 500;
      (*t)(0,14) = 1.00;  (*t)(1,14) = -600;  (*t)(2,14) = 500;
      (*t)(0,15) = 1.10;  (*t)(1,15) = -600;  (*t)(2,15) = 500;
      (*t)(0,16) = 1.20;  (*t)(1,16) = -500;  (*t)(2,16) = 500;
      (*t)(0,17) = 1.30;  (*t)(1,17) = -500;  (*t)(2,17) = 500;
      (*t)(0,18) = 1.40;  (*t)(1,18) = -500;  (*t)(2,18) = 500;
      (*t)(0,19) = 1.50;  (*t)(1,19) = -500;  (*t)(2,19) = 500;
      (*t)(0,20) = 1.60;  (*t)(1,20) = -400;  (*t)(2,20) = 500;
      (*t)(0,21) = 1.70;  (*t)(1,21) = -400;  (*t)(2,21) = 500;
      (*t)(0,22) = 1.80;  (*t)(1,22) = -400;  (*t)(2,22) = 500;
      (*t)(0,23) = 1.90;  (*t)(1,23) = -400;  (*t)(2,23) = 500;
      (*t)(0,24) = 2.00;  (*t)(1,24) = -400;  (*t)(2,24) = 500;
      (*t)(0,25) = 2.10;  (*t)(1,25) = -350;  (*t)(2,25) = 500;
      (*t)(0,26) = 2.20;  (*t)(1,26) = -350;  (*t)(2,26) = 500;
      (*t)(0,27) = 2.30;  (*t)(1,27) = -300;  (*t)(2,27) = 500;
      (*t)(0,28) = 2.40;  (*t)(1,28) = -300;  (*t)(2,28) = 500;
      (*t)(0,29) = 2.50;  (*t)(1,29) = -300;  (*t)(2,29) = 500;
      (*t)(0,30) = 2.60;  (*t)(1,30) = -250;  (*t)(2,30) = 500;
      (*t)(0,31) = 2.70;  (*t)(1,31) = -200;  (*t)(2,31) = 500;
      (*t)(0,32) = 2.80;  (*t)(1,32) = -150;  (*t)(2,32) = 500;
      (*t)(0,33) = 2.90;  (*t)(1,33) = -150;  (*t)(2,33) = 500;
      (*t)(0,34) = 3.00;  (*t)(1,34) = -100;  (*t)(2,34) = 400;
      (*t)(0,35) = 3.10;  (*t)(1,35) = -100;  (*t)(2,35) = 400;
      (*t)(0,36) = 3.20;  (*t)(1,36) =    0;  (*t)(2,36) = 0;
      (*t)(0,37) = 3.30;  (*t)(1,37) =    0;  (*t)(2,37) = 0;
      (*t)(0,38) = 3.40;  (*t)(1,38) =    0;  (*t)(2,38) = 0;
    }
    else
    if (fAliPID==AliPID::kKaon)
    {
      t = new TMatrixF(3,23);
      (*t)(0,0)  = 0.3;   (*t)(1,0)  =    0;  (*t)(2,0)  =    0;
      (*t)(0,1)  = 0.35;  (*t)(1,1)  =    0;  (*t)(2,1)  =    0;
      (*t)(0,2)  = 0.40;  (*t)(1,2)  = -800;  (*t)(2,2)  =  600;
      (*t)(0,3)  = 0.45;  (*t)(1,3)  = -800;  (*t)(2,3)  =  600;
      (*t)(0,4)  = 0.50;  (*t)(1,4)  = -800;  (*t)(2,4)  =  600;
      (*t)(0,5)  = 0.55;  (*t)(1,5)  = -800;  (*t)(2,5)  =  600;
      (*t)(0,6)  = 0.60;  (*t)(1,6)  = -800;  (*t)(2,6)  =  600;
      (*t)(0,7)  = 0.65;  (*t)(1,7)  = -700;  (*t)(2,7)  =  600;
      (*t)(0,8)  = 0.70;  (*t)(1,8)  = -600;  (*t)(2,8)  =  600;
      (*t)(0,9)  = 0.75;  (*t)(1,9)  = -600;  (*t)(2,9)  =  500;
      (*t)(0,10) = 0.80;  (*t)(1,10) = -500;  (*t)(2,10) =  500;
      (*t)(0,11) = 0.85;  (*t)(1,11) = -500;  (*t)(2,11) =  500;
      (*t)(0,12) = 0.90;  (*t)(1,12) = -400;  (*t)(2,12) =  500;
      (*t)(0,13) = 0.95;  (*t)(1,13) = -400;  (*t)(2,13) =  500;
      (*t)(0,14) = 1.00;  (*t)(1,14) = -400;  (*t)(2,14) =  500;
      (*t)(0,15) = 1.10;  (*t)(1,15) = -350;  (*t)(2,15) =  450;
      (*t)(0,16) = 1.20;  (*t)(1,16) = -300;  (*t)(2,16) =  400;
      (*t)(0,17) = 1.30;  (*t)(1,17) = -300;  (*t)(2,17) =  400;
      (*t)(0,18) = 1.40;  (*t)(1,18) = -250;  (*t)(2,18) =  400;
      (*t)(0,19) = 1.50;  (*t)(1,19) = -200;  (*t)(2,19) =  400;
      (*t)(0,20) = 1.60;  (*t)(1,20) = -150;  (*t)(2,20) =  400;
      (*t)(0,21) = 1.70;  (*t)(1,21) = -100;  (*t)(2,21) =  400;
      (*t)(0,22) = 1.80;  (*t)(1,22) =    0;  (*t)(2,22) =    0;
    }
    fTOFpidCuts=t;
  }
}
