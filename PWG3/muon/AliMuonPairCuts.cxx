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

#include "TMath.h"
#include "TList.h"
#include "TLorentzVector.h"

#include "AliLog.h"
#include "AliVParticle.h"

#include "AliMuonTrackCuts.h"
#include "AliMuonPairCuts.h"

/// \cond CLASSIMP
ClassImp(AliMuonPairCuts) // Class implementation in ROOT context
/// \endcond

//________________________________________________________________________
AliMuonPairCuts::AliMuonPairCuts() :
  AliAnalysisCuts(),
  fMuonTrackCuts()
{
  /// Default ctor.
}

//________________________________________________________________________
AliMuonPairCuts::AliMuonPairCuts(const char* name, const char* title, Bool_t isESD) :
  AliAnalysisCuts(name, title),
  fMuonTrackCuts("muonTrackCutInPair","muonTrackCutInPair", isESD)
{
  /// Constructor
  SetDefaultFilterMask();
}


//________________________________________________________________________
AliMuonPairCuts::AliMuonPairCuts(const char* name, const char* title, const AliMuonTrackCuts& trackCuts) :
AliAnalysisCuts(name, title),
fMuonTrackCuts(trackCuts)
{
  /// Test Constructor
  SetDefaultFilterMask();
}



//________________________________________________________________________
AliMuonPairCuts::AliMuonPairCuts(const AliMuonPairCuts& obj) :
  AliAnalysisCuts(obj),
  fMuonTrackCuts(obj.fMuonTrackCuts)
{
  /// Copy constructor
}


//________________________________________________________________________
AliMuonPairCuts::~AliMuonPairCuts()
{
  /// Destructor
}

//________________________________________________________________________
Bool_t AliMuonPairCuts::SetRun( Int_t runNumber )
{
  /// Get parameters from OADB for runNumber
  return fMuonTrackCuts.SetRun(runNumber);
}


//________________________________________________________________________
Bool_t AliMuonPairCuts::IsSelected( TObject* /*obj*/ )
{
  /// Not implemented
  AliError("Requires a list of two AliVParticle");
  return kFALSE;
}

//________________________________________________________________________
Bool_t AliMuonPairCuts::IsSelected( TList* list )
{
  /// Pair is selected
  UInt_t filterMask = GetFilterMask();
  UInt_t selectionMask = GetSelectionMask(list);
  
  return ( ( selectionMask & filterMask ) == filterMask );
}


//________________________________________________________________________
UInt_t AliMuonPairCuts::GetSelectionMask( const TObject* obj )
{
  /// Get selection mask (overloaded function)
  const TList* list = static_cast<const TList*> ( obj );
  if ( list->GetEntries() < 2 ) {
    AliError("Requires a list of two AliVParticle");
    return 0;
  }

  return GetSelectionMask(list->At(0), list->At(1));

}


//________________________________________________________________________
UInt_t AliMuonPairCuts::GetSelectionMask( const TObject* track1, const TObject* track2 )
{
  /// Get selection mask from AliVParticles
  
  UInt_t selectionMask = 0;
    
  UInt_t maskTrack1 = fMuonTrackCuts.GetSelectionMask(track1);
  UInt_t maskTrack2 = fMuonTrackCuts.GetSelectionMask(track2);
  
  UInt_t maskAND = maskTrack1 & maskTrack2;
  UInt_t maskOR = maskTrack1 | maskTrack2;
  if ( maskAND & AliMuonTrackCuts::kMuEta ) selectionMask |= kBothMuEta;
  if ( maskAND & AliMuonTrackCuts::kMuThetaAbs ) selectionMask |= kBothMuThetaAbs;
  if ( maskAND & AliMuonTrackCuts::kMuPdca ) selectionMask |= kBothMuPdca;
  if ( maskAND & AliMuonTrackCuts::kMuTrackChiSquare ) selectionMask |= kBothMuTrackChiSquare;
  if ( maskAND & AliMuonTrackCuts::kMuMatchApt ) selectionMask |= kBothMuMatchApt;
  if ( maskAND & AliMuonTrackCuts::kMuMatchLpt ) selectionMask |= kBothMuMatchLpt;
  if ( maskAND & AliMuonTrackCuts::kMuMatchHpt ) selectionMask |= kBothMuMatchHpt;
  if ( maskOR & AliMuonTrackCuts::kMuMatchApt ) selectionMask |= kOneMuMatchApt;
  if ( maskOR & AliMuonTrackCuts::kMuMatchLpt ) selectionMask |= kOneMuMatchLpt;
  if ( maskOR & AliMuonTrackCuts::kMuMatchHpt ) selectionMask |= kOneMuMatchHpt;
  
  TLorentzVector vec[2];
  Double_t chargeProduct = 1.;
  for ( Int_t itrack=0; itrack<2; ++itrack ) {
    const AliVParticle* track = static_cast<const AliVParticle*> ( ( itrack == 0 ) ? track1 : track2 );
    chargeProduct *= track->Charge();
    Double_t trackP = track->P();
    Double_t energy = TMath::Sqrt(trackP*trackP + MuonMass2());
    vec[itrack].SetPxPyPzE(track->Px(), track->Py(), track->Pz(), energy);
  }
  
  if ( chargeProduct < 0. ) selectionMask |= kDimuUnlikeSign;
  
  TLorentzVector vecPair = vec[0] + vec[1];
  Double_t rapidity = vecPair.Rapidity();
  if ( rapidity > -4. && rapidity < -2.5 ) selectionMask |= kDimuRapidity; 
  
  return selectionMask;
}


//_____________________________________________________________________________
Double_t AliMuonPairCuts::MuonMass2() const
{
  /// A usefull constant
  static Double_t m2 = 1.11636129640000012e-02; // using a constant here as the line below is a problem for CINT...
  return m2;
}


//________________________________________________________________________
void AliMuonPairCuts::SetDefaultFilterMask ()
{
  /// Standard cuts for muon pair
  SetFilterMask ( kBothMuEta | kBothMuThetaAbs | kBothMuMatchLpt | kDimuUnlikeSign | kDimuRapidity );
}  

//________________________________________________________________________
void AliMuonPairCuts::SetIsMC ( Bool_t isMC )
{
  /// Set Is MC
  fMuonTrackCuts.SetIsMC(isMC);
}

//________________________________________________________________________
void AliMuonPairCuts::Print(Option_t* option) const
{
  //
  /// Print info
  //
  
  TString sopt(option);
  sopt.ToLower();
  if ( sopt.IsNull() || sopt.Contains("*") || sopt.Contains("all") ) sopt += " pair track";
  if ( sopt.Contains("pair") ) {
    UInt_t filterMask = GetFilterMask();
    printf(" *** Muon pair filter mask: *** \n");
    printf("  0x%x\n", filterMask);
    if ( filterMask & kBothMuEta ) printf("  mu1 && mu2 pass eta cut\n");
    if ( filterMask & kBothMuThetaAbs ) printf("  mu1 && mu2 pass theta_abs cut\n");
    if ( filterMask & kBothMuPdca ) printf("  mu1 && mu2 pass pxDCA cut\n");
    if ( filterMask & kBothMuTrackChiSquare ) printf("  Chi2 cut on track\n");
    if ( filterMask & kBothMuMatchApt ) printf("  mu1 && mu2 match Apt\n");
    if ( filterMask & kBothMuMatchLpt ) printf("  mu1 && mu2 match Lpt\n");
    if ( filterMask & kBothMuMatchHpt ) printf("  mu1 && mu2 match Hpt\n");
    if ( filterMask & kBothMuMatchApt ) printf("  mu1 || mu2 match Apt\n");
    if ( filterMask & kBothMuMatchLpt ) printf("  mu1 || mu2 match Lpt\n");
    if ( filterMask & kBothMuMatchHpt ) printf("  mu1 || mu2 match Hpt\n");
    if ( filterMask & kDimuUnlikeSign ) printf("  Unlike sign\n");
    if ( filterMask & kDimuRapidity ) printf("  -4 < y_{mumu} < -2.5\n");
    printf(" ******************** \n");
  }
  
  if ( sopt.Contains("track") )
    fMuonTrackCuts.Print(sopt.Data());
}
