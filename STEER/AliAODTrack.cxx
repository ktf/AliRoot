/**************************************************************************
 * Copyright(c) 1998-2007, ALICE Experiment at CERN, All rights reserved. *
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

//-------------------------------------------------------------------------
//     AOD track implementation of AliVTrack
//     Author: Markus Oldenburg, CERN
//     Markus.Oldenburg@cern.ch
//-------------------------------------------------------------------------

#include "AliLog.h"
#include "AliAODTrack.h"

ClassImp(AliAODTrack)

//______________________________________________________________________________
AliAODTrack::AliAODTrack() : 
  AliVTrack(),
  fChi2perNDF(-999.),
  fChi2MatchTrigger(0.),
  fFlags(0),
  fLabel(-999),
  fITSMuonClusterMap(0),
  fFilterMap(0),
  fTPCClusterMap(),
  fTPCSharedMap(),
  fID(-999),
  fCharge(-99),
  fType(kUndef),
  fCovMatrix(NULL),
  fDetPid(NULL),
  fProdVertex(NULL)
{
  // default constructor

  SetP();
  SetPosition((Float_t*)NULL);
  SetXYAtDCA(-999., -999.);
  SetPxPyPzAtDCA(-999., -999., -999.);
  SetPID((Float_t*)NULL);
}

//______________________________________________________________________________
AliAODTrack::AliAODTrack(Short_t id,
			 Int_t label, 
			 Double_t p[3],
			 Bool_t cartesian,
			 Double_t x[3],
			 Bool_t isDCA,
			 Double_t covMatrix[21],
			 Short_t charge,
			 UChar_t itsClusMap,
			 Double_t pid[10],
			 AliAODVertex *prodVertex,
			 Bool_t usedForVtxFit,
			 Bool_t usedForPrimVtxFit,
			 AODTrk_t ttype,
			 UInt_t selectInfo,
			 Float_t chi2perNDF) :
  AliVTrack(),
  fChi2perNDF(chi2perNDF),
  fChi2MatchTrigger(0.),
  fFlags(0),
  fLabel(label),
  fITSMuonClusterMap(0),
  fFilterMap(selectInfo),
  fTPCClusterMap(),
  fTPCSharedMap(),
  fID(id),
  fCharge(charge),
  fType(ttype),
  fCovMatrix(NULL),
  fDetPid(NULL),
  fProdVertex(prodVertex)
{
  // constructor
 
  SetP(p, cartesian);
  SetPosition(x, isDCA);
  SetXYAtDCA(-999., -999.);
  SetPxPyPzAtDCA(-999., -999., -999.);
  SetUsedForVtxFit(usedForVtxFit);
  SetUsedForPrimVtxFit(usedForPrimVtxFit);
  if(covMatrix) SetCovMatrix(covMatrix);
  SetPID(pid);
  SetITSClusterMap(itsClusMap);
}

//______________________________________________________________________________
AliAODTrack::AliAODTrack(Short_t id,
			 Int_t label, 
			 Float_t p[3],
			 Bool_t cartesian,
			 Float_t x[3],
			 Bool_t isDCA,
			 Float_t covMatrix[21],
			 Short_t charge,
			 UChar_t itsClusMap,
			 Float_t pid[10],
			 AliAODVertex *prodVertex,
			 Bool_t usedForVtxFit,
			 Bool_t usedForPrimVtxFit,
			 AODTrk_t ttype,
			 UInt_t selectInfo,
			 Float_t chi2perNDF) :
  AliVTrack(),
  fChi2perNDF(chi2perNDF),
  fChi2MatchTrigger(0.),
  fFlags(0),
  fLabel(label),
  fITSMuonClusterMap(0),
  fFilterMap(selectInfo),
  fTPCClusterMap(),
  fTPCSharedMap(),
  fID(id),
  fCharge(charge),
  fType(ttype),
  fCovMatrix(NULL),
  fDetPid(NULL),
  fProdVertex(prodVertex)
{
  // constructor
 
  SetP(p, cartesian);
  SetPosition(x, isDCA);
  SetXYAtDCA(-999., -999.);
  SetPxPyPzAtDCA(-999., -999., -999.);
  SetUsedForVtxFit(usedForVtxFit);
  SetUsedForPrimVtxFit(usedForPrimVtxFit);
  if(covMatrix) SetCovMatrix(covMatrix);
  SetPID(pid);
  SetITSClusterMap(itsClusMap);
}

//______________________________________________________________________________
AliAODTrack::~AliAODTrack() 
{
  // destructor
  delete fCovMatrix;
  delete fDetPid;
}


//______________________________________________________________________________
AliAODTrack::AliAODTrack(const AliAODTrack& trk) :
  AliVTrack(trk),
  fChi2perNDF(trk.fChi2perNDF),
  fChi2MatchTrigger(trk.fChi2MatchTrigger),
  fFlags(trk.fFlags),
  fLabel(trk.fLabel),
  fITSMuonClusterMap(trk.fITSMuonClusterMap),
  fFilterMap(trk.fFilterMap),
  fTPCClusterMap(trk.fTPCClusterMap),
  fTPCSharedMap(trk.fTPCSharedMap),
  fID(trk.fID),
  fCharge(trk.fCharge),
  fType(trk.fType),
  fCovMatrix(NULL),
  fDetPid(NULL),
  fProdVertex(trk.fProdVertex)
{
  // Copy constructor

  trk.GetP(fMomentum);
  trk.GetPosition(fPosition);
  SetXYAtDCA(trk.XAtDCA(), trk.YAtDCA());
  SetPxPyPzAtDCA(trk.PxAtDCA(), trk.PyAtDCA(), trk.PzAtDCA());
  SetUsedForVtxFit(trk.GetUsedForVtxFit());
  SetUsedForPrimVtxFit(trk.GetUsedForPrimVtxFit());
  if(trk.fCovMatrix) fCovMatrix=new AliAODRedCov<6>(*trk.fCovMatrix);
  if(trk.fDetPid) fDetPid=new AliAODPid(*trk.fDetPid);
  SetPID(trk.fPID);
}

//______________________________________________________________________________
AliAODTrack& AliAODTrack::operator=(const AliAODTrack& trk)
{
  // Assignment operator
  if(this!=&trk) {

    AliVTrack::operator=(trk);

    trk.GetP(fMomentum);
    trk.GetPosition(fPosition);
    trk.GetPID(fPID);

    SetXYAtDCA(trk.XAtDCA(), trk.YAtDCA());
    SetPxPyPzAtDCA(trk.PxAtDCA(), trk.PyAtDCA(), trk.PzAtDCA());
    
    fChi2perNDF = trk.fChi2perNDF;
    fChi2MatchTrigger = trk.fChi2MatchTrigger;

    fFlags = trk.fFlags;
    fLabel = trk.fLabel;    
    
    fITSMuonClusterMap = trk.fITSMuonClusterMap;
    fFilterMap = trk.fFilterMap;

    fID = trk.fID;

    fCharge = trk.fCharge;
    fType = trk.fType;

    delete fCovMatrix;
    if(trk.fCovMatrix) fCovMatrix=new AliAODRedCov<6>(*trk.fCovMatrix);
    else fCovMatrix=NULL;
    fProdVertex = trk.fProdVertex;

    SetUsedForVtxFit(trk.GetUsedForVtxFit());
    SetUsedForPrimVtxFit(trk.GetUsedForPrimVtxFit());

    delete fDetPid;
    if(trk.fDetPid) fDetPid=new AliAODPid(*trk.fDetPid);
    else fDetPid=NULL;
  }

  return *this;
}

//______________________________________________________________________________
Double_t AliAODTrack::M(AODTrkPID_t pid) const
{
  // Returns the mass.
  // Masses for nuclei don't exist in the PDG tables, therefore they were put by hand.

  switch (pid) {

  case kElectron :
    return 0.000510999; //TDatabasePDG::Instance()->GetParticle(11/*::kElectron*/)->Mass();
    break;

  case kMuon :
    return 0.1056584; //TDatabasePDG::Instance()->GetParticle(13/*::kMuonMinus*/)->Mass();
    break;

  case kPion :
    return 0.13957; //TDatabasePDG::Instance()->GetParticle(211/*::kPiPlus*/)->Mass();
    break;

  case kKaon :
    return 0.4937; //TDatabasePDG::Instance()->GetParticle(321/*::kKPlus*/)->Mass();
    break;

  case kProton :
    return 0.9382720; //TDatabasePDG::Instance()->GetParticle(2212/*::kProton*/)->Mass();
    break;

  case kDeuteron :
    return 1.8756; //TDatabasePDG::Instance()->GetParticle(1000010020)->Mass();
    break;

  case kTriton :
    return 2.8089; //TDatabasePDG::Instance()->GetParticle(1000010030)->Mass();
    break;

  case kHelium3 :
    return 2.8084; //TDatabasePDG::Instance()->GetParticle(1000020030)->Mass();
    break;

  case kAlpha :
    return 3.7274; //TDatabasePDG::Instance()->GetParticle(1000020040)->Mass();
    break;

  case kUnknown :
    return -999.;
    break;

  default :
    return -999.;
  }
}

//______________________________________________________________________________
Double_t AliAODTrack::E(AODTrkPID_t pid) const
{
  // Returns the energy of the particle of a given pid.
  
  if (pid != kUnknown) { // particle was identified
    Double_t m = M(pid);
    return TMath::Sqrt(P()*P() + m*m);
  } else { // pid unknown
    return -999.;
  }
}

//______________________________________________________________________________
Double_t AliAODTrack::Y(AODTrkPID_t pid) const
{
  // Returns the rapidity of a particle of a given pid.
  
  if (pid != kUnknown) { // particle was identified
    Double_t e = E(pid);
    Double_t pz = Pz();
    if (e>=0 && e!=pz) { // energy was positive (e.g. not -999.) and not equal to pz
      return 0.5*TMath::Log((e+pz)/(e-pz));
    } else { // energy not known or equal to pz
      return -999.;
    }
  } else { // pid unknown
    return -999.;
  }
}

//______________________________________________________________________________
Double_t AliAODTrack::Y(Double_t m) const
{
  // Returns the rapidity of a particle of a given mass.
  
  if (m >= 0.) { // mass makes sense
    Double_t e = E(m);
    Double_t pz = Pz();
    if (e>=0 && e!=pz) { // energy was positive (e.g. not -999.) and not equal to pz
      return 0.5*TMath::Log((e+pz)/(e-pz));
    } else { // energy not known or equal to pz
      return -999.;
    }
  } else { // pid unknown
    return -999.;
  }
}

//______________________________________________________________________________
AliAODTrack::AODTrkPID_t AliAODTrack::GetMostProbablePID() const 
{
  // Returns the most probable PID array element.
  
  Int_t nPID = 10;
  if (fPID) {
    AODTrkPID_t loc = kUnknown;
    Double_t max = 0.;
    Bool_t allTheSame = kTRUE;
    
    for (Int_t iPID = 0; iPID < nPID; iPID++) {
      if (fPID[iPID] >= max) {
	if (fPID[iPID] > max) {
	  allTheSame = kFALSE;
	  max = fPID[iPID];
	  loc = (AODTrkPID_t)iPID;
	} else {
	  allTheSame = kTRUE;
	}
      }
    }
    
    return allTheSame ? kUnknown : loc;
  } else {
    return kUnknown;
  }
}

//______________________________________________________________________________
void AliAODTrack::ConvertAliPIDtoAODPID()
{
  // Converts AliPID array.
  // The numbering scheme is the same for electrons, muons, pions, kaons, and protons.
  // Everything else has to be set to zero.

  fPID[kDeuteron] = 0.;
  fPID[kTriton]   = 0.;
  fPID[kHelium3]  = 0.;
  fPID[kAlpha]    = 0.;
  fPID[kUnknown]  = 0.;
  
  return;
}


//______________________________________________________________________________
template <class T> void AliAODTrack::SetP(const T *p, const Bool_t cartesian) 
{
  // Set the momentum

  if (p) {
    if (cartesian) {
      Double_t pt2 = p[0]*p[0] + p[1]*p[1];
      Double_t pp  = TMath::Sqrt(pt2 + p[2]*p[2]);
      
      fMomentum[0] = TMath::Sqrt(pt2); // pt
      fMomentum[1] = (pt2 != 0.) ? TMath::Pi()+TMath::ATan2(-p[1], -p[0]) : -999; // phi
      fMomentum[2] = (pp != 0.) ? TMath::ACos(p[2] / pp) : -999.; // theta
    } else {
      fMomentum[0] = p[0];  // pt
      fMomentum[1] = p[1];  // phi
      fMomentum[2] = p[2];  // theta
    }
  } else {
    fMomentum[0] = -999.;
    fMomentum[1] = -999.;
    fMomentum[2] = -999.;
  }
}

//______________________________________________________________________________
template <class T> void AliAODTrack::SetPosition(const T *x, const Bool_t dca) 
{
  // set the position

  if (x) {
    if (!dca) {
      ResetBit(kIsDCA);

      fPosition[0] = x[0];
      fPosition[1] = x[1];
      fPosition[2] = x[2];
    } else {
      SetBit(kIsDCA);
      // don't know any better yet
      fPosition[0] = -999.;
      fPosition[1] = -999.;
      fPosition[2] = -999.;
    }
  } else {
    ResetBit(kIsDCA);

    fPosition[0] = -999.;
    fPosition[1] = -999.;
    fPosition[2] = -999.;
  }
}

//______________________________________________________________________________
void AliAODTrack::SetDCA(Double_t d, Double_t z) 
{
  // set the dca
  fPosition[0] = d;
  fPosition[1] = z;
  fPosition[2] = 0.;
  SetBit(kIsDCA);
}

//______________________________________________________________________________
void AliAODTrack::Print(Option_t* /* option */) const
{
  // prints information about AliAODTrack

  printf("Object name: %s   Track type: %s\n", GetName(), GetTitle()); 
  printf("        px = %f\n", Px());
  printf("        py = %f\n", Py());
  printf("        pz = %f\n", Pz());
  printf("        pt = %f\n", Pt());
  printf("      1/pt = %f\n", OneOverPt());
  printf("     theta = %f\n", Theta());
  printf("       phi = %f\n", Phi());
  printf("  chi2/NDF = %f\n", Chi2perNDF());
  printf("    charge = %d\n", Charge());
}

//______________________________________________________________________________
void AliAODTrack::SetMatchTrigger(Int_t matchTrig)
{
  // Set the MUON trigger information
  switch(matchTrig){
    case 0: // 0 track does not match trigger
      fITSMuonClusterMap=fITSMuonClusterMap&0x3fffffff;
      break;
    case 1: // 1 track match but does not pass pt cut
      fITSMuonClusterMap=(fITSMuonClusterMap&0x3fffffff)|0x40000000;
      break;
    case 2: // 2 track match Low pt cut
      fITSMuonClusterMap=(fITSMuonClusterMap&0x3fffffff)|0x80000000;
      break;
    case 3: // 3 track match High pt cut
      fITSMuonClusterMap=fITSMuonClusterMap|0xc0000000;
      break;
    default:
      fITSMuonClusterMap=fITSMuonClusterMap&0x3fffffff;
      AliWarning(Form("unknown case for matchTrig: %d\n",matchTrig));
  }
}

//______________________________________________________________________________
Bool_t AliAODTrack::HitsMuonChamber(Int_t MuonChamber, Int_t cathode) const
{
  // return kTRUE if the track fires the given tracking or trigger chamber.
  // If the chamber is a trigger one:
  // - if cathode = 0 or 1, the track matches the corresponding cathode
  // - if cathode = -1, the track matches both cathodes
  
  if (MuonChamber < 0) return kFALSE;
  
  if (MuonChamber < 10) return TESTBIT(GetMUONClusterMap(), MuonChamber);
  
  if (MuonChamber < 14) {
    
    if (cathode < 0) return TESTBIT(GetHitsPatternInTrigCh(), 13-MuonChamber) &&
                            TESTBIT(GetHitsPatternInTrigCh(), 13-MuonChamber+4);
    
    if (cathode < 2) return TESTBIT(GetHitsPatternInTrigCh(), 13-MuonChamber+(1-cathode)*4);
    
  }
  
  return kFALSE;
}

//______________________________________________________________________________
Bool_t AliAODTrack::MatchTriggerDigits() const
{
  // return kTRUE if the track matches a digit on both planes of at least 2 trigger chambers
  
  Int_t nMatchedChambers = 0;
  for (Int_t ich=10; ich<14; ich++) if (HitsMuonChamber(ich)) nMatchedChambers++;
  
  return (nMatchedChambers >= 2);
}

