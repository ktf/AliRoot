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
#include "AliMCEvent.h"
#include "AliVParticle.h"
#include "AliMCParticle.h"
#include "AliESDtrack.h"
#include "AliMultiplicity.h"
#include "AliAODTrack.h"
#include "AliFlowTrack.h"
#include "AliFlowTrackCuts.h"
#include "AliLog.h"

ClassImp(AliFlowTrackCuts)

//-----------------------------------------------------------------------
AliFlowTrackCuts::AliFlowTrackCuts():
  AliFlowTrackSimpleCuts(),
  fAliESDtrackCuts(new AliESDtrackCuts()),
  fCutMCprocessType(kFALSE),
  fMCprocessType(kPNoProcess),
  fCutMCPID(kFALSE),
  fMCPID(0),
  fCutMCisPrimary(kFALSE),
  fMCisPrimary(kFALSE),
  fRequireCharge(kFALSE),
  fFakesAreOK(kTRUE),
  fParamType(kGlobal),
  fParamMix(kPure),
  fCleanupTrack(kFALSE),
  fTrack(NULL),
  fTrackPhi(0.),
  fTrackEta(0.),
  fTrackWeight(0.),
  fTrackLabel(INT_MIN),
  fMCevent(NULL),
  fMCparticle(NULL)
{
  //constructor 
}

//-----------------------------------------------------------------------
AliFlowTrackCuts::AliFlowTrackCuts(const AliFlowTrackCuts& someCuts):
  AliFlowTrackSimpleCuts(someCuts),
  fAliESDtrackCuts(new AliESDtrackCuts(*(someCuts.fAliESDtrackCuts))),
  fCutMCprocessType(someCuts.fCutMCprocessType),
  fMCprocessType(someCuts.fMCprocessType),
  fCutMCPID(someCuts.fCutMCPID),
  fMCPID(someCuts.fMCPID),
  fCutMCisPrimary(someCuts.fCutMCisPrimary),
  fMCisPrimary(someCuts.fMCisPrimary),
  fRequireCharge(someCuts.fRequireCharge),
  fFakesAreOK(someCuts.fFakesAreOK),
  fParamType(someCuts.fParamType),
  fParamMix(someCuts.fParamMix),
  fCleanupTrack(kFALSE),
  fTrack(NULL),
  fTrackPhi(someCuts.fTrackPhi),
  fTrackEta(someCuts.fTrackEta),
  fTrackWeight(someCuts.fTrackWeight),
  fTrackLabel(INT_MIN),
  fMCevent(NULL),
  fMCparticle(NULL)
{
  //copy constructor
}

//-----------------------------------------------------------------------
AliFlowTrackCuts& AliFlowTrackCuts::operator=(const AliFlowTrackCuts& someCuts)
{
  //assignment
  AliFlowTrackSimpleCuts::operator=(someCuts);
  *fAliESDtrackCuts=*(someCuts.fAliESDtrackCuts);
  fCutMCprocessType=someCuts.fCutMCprocessType;
  fMCprocessType=someCuts.fMCprocessType;
  fCutMCPID=someCuts.fCutMCPID;
  fMCPID=someCuts.fMCPID;
  fCutMCisPrimary=someCuts.fCutMCisPrimary;
  fMCisPrimary=someCuts.fMCisPrimary;
  fRequireCharge=someCuts.fRequireCharge;
  fFakesAreOK=someCuts.fFakesAreOK;
  fParamType=someCuts.fParamType;
  fParamMix=someCuts.fParamMix;

  fCleanupTrack=kFALSE;
  fTrack=NULL;
  fTrackPhi=someCuts.fTrackPhi;
  fTrackPhi=someCuts.fTrackPhi;
  fTrackWeight=someCuts.fTrackWeight;
  fTrackLabel=INT_MIN;
  fMCevent=NULL;
  fMCparticle=NULL;

  return *this;
}

//-----------------------------------------------------------------------
AliFlowTrackCuts::~AliFlowTrackCuts()
{
  //dtor
  if (fCleanupTrack) delete fTrack;
  delete fAliESDtrackCuts;
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
Bool_t AliFlowTrackCuts::PassesCuts(AliFlowTrackSimple* track)
{
  //check cuts on a flowtracksimple

  //clean up from last iteration
  if (fCleanupTrack) delete fTrack; fTrack = NULL;
  return AliFlowTrackSimpleCuts::PassesCuts(track);
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesCuts(AliMultiplicity* tracklet, Int_t id)
{
  //check cuts on a tracklets

  //clean up from last iteration
  if (fCleanupTrack) delete fTrack; fTrack = NULL;
  fMCparticle=NULL;

  fTrackPhi = tracklet->GetPhi(id);
  fTrackEta = tracklet->GetEta(id);
  fTrackWeight = 1.0;
  if (fCutEta) {if (  fTrackEta < fEtaMin || fTrackEta >= fEtaMax ) return kFALSE;}
  if (fCutPhi) {if ( fTrackPhi < fPhiMin || fTrackPhi >= fPhiMax ) return kFALSE;}

  //check MC info if available
  fTrackLabel = tracklet->GetLabel(id,1); //TODO: this can be improved
  if (!PassesMCcuts()) return kFALSE;
  return kTRUE;
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesMCcuts()
{
  //check the MC info
  if (!fMCevent) {AliError("no MC info"); return kFALSE;}
  fMCparticle = static_cast<AliMCParticle*>(fMCevent->GetTrack(fTrackLabel));
  if (!fMCparticle) {AliError("no MC info"); return kFALSE;}

  if (fCutMCisPrimary)
  {
    if (IsPhysicalPrimary() != fMCisPrimary) return kFALSE;
  }
  if (fCutMCPID)
  {
    Int_t pdgCode = fMCparticle->PdgCode();
    if (fMCPID != pdgCode) return kFALSE;
  }
  if ( fCutMCprocessType )
  {
    TParticle* particle = fMCparticle->Particle();
    Int_t processID = particle->GetUniqueID();
    if (processID != fMCprocessType ) return kFALSE;
  }
  return kTRUE;
}

//-----------------------------------------------------------------------
Bool_t AliFlowTrackCuts::PassesCuts(AliVParticle* vparticle)
{
  //check cuts for an ESD vparticle

  ////////////////////////////////////////////////////////////////
  //  start by preparing the track parameters to cut on //////////
  ////////////////////////////////////////////////////////////////
  //clean up from last iteration
  if (fCleanupTrack) delete fTrack; fTrack=NULL; 

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

  //check the common cuts for the current particle (MC,AOD,ESD)
  if (fCutPt) {if (fTrack->Pt() < fPtMin || fTrack->Pt() >= fPtMax ) return kFALSE;}
  if (fCutEta) {if (fTrack->Eta() < fEtaMin || fTrack->Eta() >= fEtaMax ) return kFALSE;}
  if (fCutPhi) {if (fTrack->Phi() < fPhiMin || fTrack->Phi() >= fPhiMax ) return kFALSE;}
  if (fRequireCharge) {if (fTrack->Charge() == 0) return kFALSE;}
  if (fCutCharge && !isMCparticle) {if (fTrack->Charge() != fCharge) return kFALSE;}
  if (fCutCharge && isMCparticle)
  { 
    //in case of an MC particle the charge is stored in units of 1/3|e| 
    Int_t charge = TMath::Nint(fTrack->Charge()/3.0); //mc particles have charge in units of 1/3e
    return (charge==fCharge);
  }
  //if(fCutPID) {if (fTrack->PID() != fPID) return kFALSE;}

  //when additionally MC info is required
  if (!PassesMCcuts()) return kFALSE;

  //check all else for ESDs using aliesdtrackcuts
  if (esdTrack && (fParamType!=kMC) ) return fAliESDtrackCuts->IsSelected(static_cast<AliESDtrack*>(fTrack));

  return kTRUE; //true by default, if we didn't set any cuts
}

//-----------------------------------------------------------------------
void AliFlowTrackCuts::HandleVParticle(AliVParticle* track)
{
  //handle the general case
  switch (fParamType)
  {
    default:
      fCleanupTrack = kFALSE;
      fTrack = track;
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
      fCleanupTrack = kFALSE;
      break;
    case kESD_TPConly:
      fTrack = new AliESDtrack();
      track->FillTPCOnlyTrack(*(static_cast<AliESDtrack*>(fTrack)));
      fCleanupTrack = kTRUE;
      //recalculate the label and mc particle, they may differ as TPClabel != global label
      fTrackLabel = (fFakesAreOK)?TMath::Abs(fTrack->GetLabel()):fTrack->GetLabel();
      if (fMCevent) fMCparticle = static_cast<AliMCParticle*>(fMCevent->GetTrack(fTrackLabel));
      else fMCparticle=NULL;
      break;
    default:
      fTrack = track;
      fCleanupTrack = kFALSE;
  }
}

//-----------------------------------------------------------------------
AliFlowTrackCuts* AliFlowTrackCuts::GetStandardTPCOnlyTrackCuts()
{
  //get standard cuts
  AliFlowTrackCuts* cuts = new AliFlowTrackCuts();
  cuts->SetName("standard TPConly cuts");
  delete cuts->fAliESDtrackCuts;
  cuts->fAliESDtrackCuts = AliESDtrackCuts::GetStandardTPCOnlyTrackCuts();
  cuts->SetParamType(kESD_TPConly);
  return cuts;
}

//-----------------------------------------------------------------------
AliFlowTrackCuts* AliFlowTrackCuts::GetStandardITSTPCTrackCuts2009(Bool_t selPrimaries)
{
  //get standard cuts
  AliFlowTrackCuts* cuts = new AliFlowTrackCuts();
  cuts->SetName("standard global track cuts 2009");
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
  AliFlowTrack* flowtrack=NULL;
  if (fParamType==kESD_SPDtracklet)
  {
    flowtrack = new AliFlowTrack();
    flowtrack->SetPhi(fTrackPhi);
    flowtrack->SetEta(fTrackEta);
    flowtrack->SetSource(AliFlowTrack::kFromTracklet);
  }
  else
  {
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
        break;
      default:
        flowtrack = new AliFlowTrack(fTrack);
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
  return fMCevent->IsPhysicalPrimary(fTrackLabel);
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
  return "unknown";
}
