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
/************************************************************************
 *                                                                      *
 * Class for TPC PID                                                    *
 * Implements the abstract base class AliHFEpidBase                     *
 *                                                                      *
 * Class contains TPC specific cuts and QA histograms                   *
 * Two selection strategies are offered: Selection of certain value     *
 * regions in the TPC dE/dx (by IsSelected), and likelihoods            *
 *                                                                      *
 * Authors:                                                             *
 *                                                                      *
 *   Markus Fasel <M.Fasel@gsi.de>                                      *
 *   Markus Heide <mheide@uni-muenster.de>                              *
 *                                                                      *
 *                                                                      *
 ************************************************************************/
#include <TH2I.h>
#include <TList.h>
#include <TMath.h>

#include "AliESDtrack.h"
#include "AliExternalTrackParam.h"
#include "AliLog.h"
#include "AliPID.h"
#include "AliTPCpidESD.h"
#include "AliVParticle.h"

#include "AliHFEpidTPC.h"



//___________________________________________________________________
AliHFEpidTPC::AliHFEpidTPC(const char* name) :
  // add a list here
    AliHFEpidBase(name)
  , fLineCrossingsEnabled(0)
  , fNsigmaTPC(3)
  , fPID(NULL)
  , fPIDtpcESD(NULL)
  , fQAList(NULL)
{
  //
  // default  constructor
  // 
  memset(fLineCrossingSigma, 0, sizeof(Double_t) * AliPID::kSPECIES);
  fPID = new AliPID;
  fPIDtpcESD = new AliTPCpidESD;
}

//___________________________________________________________________
AliHFEpidTPC::AliHFEpidTPC(const AliHFEpidTPC &ref) :
    AliHFEpidBase("")
  , fLineCrossingsEnabled(0)
  , fNsigmaTPC(2)
  , fPID(NULL)
  , fPIDtpcESD(NULL)
  , fQAList(NULL)
{
  //
  // Copy constructor
  //
  ref.Copy(*this);
}

//___________________________________________________________________
AliHFEpidTPC &AliHFEpidTPC::operator=(const AliHFEpidTPC &ref){
  //
  // Assignment operator
  //
  if(this != &ref){
    ref.Copy(*this);
  } 
  return *this;
}

//___________________________________________________________________
void AliHFEpidTPC::Copy(TObject &o) const{
  //
  // Copy function 
  // called in copy constructor and assigment operator
  //
  AliHFEpidTPC &target = dynamic_cast<AliHFEpidTPC &>(o);

  target.fLineCrossingsEnabled = fLineCrossingsEnabled;
  target.fNsigmaTPC = fNsigmaTPC;
  target.fPID = new AliPID(*fPID);
  target.fPIDtpcESD = new AliTPCpidESD(*fPIDtpcESD);
  target.fQAList = dynamic_cast<TList *>(fQAList->Clone());
  memcpy(target.fLineCrossingSigma, fLineCrossingSigma, sizeof(Double_t) * AliPID::kSPECIES);

  AliHFEpidBase::Copy(target);
}

//___________________________________________________________________
AliHFEpidTPC::~AliHFEpidTPC(){
  //
  // Destructor
  //
  if(fPID) delete fPID;
  if(fPIDtpcESD) delete fPIDtpcESD;
  if(fQAList){
    fQAList->Delete();
    delete fQAList;
  }
}

//___________________________________________________________________
Bool_t AliHFEpidTPC::InitializePID(){
  //
  // Add TPC dE/dx Line crossings
  //
  //AddTPCdEdxLineCrossing(AliPID::kKaon, 0.3, 0.018);
  //AddTPCdEdxLineCrossing(AliPID::kProton, 0.9, 0.054);
  return kTRUE;
}

//___________________________________________________________________
Int_t AliHFEpidTPC::IsSelected(AliVParticle *track)
{
  //
  // For the TPC pid we use the 2-sigma band around the bethe bloch curve
  // for electrons
  // exclusion of the crossing points
  //
  AliESDtrack *esdTrack = NULL;
  if(!(esdTrack = dynamic_cast<AliESDtrack *>(track))) return kFALSE;
  if(IsQAon()) FillTPChistograms(esdTrack);
  // exclude crossing points:
  // Determine the bethe values for each particle species
  Bool_t isLineCrossing = kFALSE;
  for(Int_t ispecies = 0; ispecies < AliPID::kSPECIES; ispecies++){
    if(ispecies == AliPID::kElectron) continue;
    if(!(fLineCrossingsEnabled & 1 << ispecies)) continue;
    if(fPIDtpcESD->GetNumberOfSigmas(esdTrack, (AliPID::EParticleType)ispecies) < fLineCrossingSigma[ispecies]){
      // Point in a line crossing region, no PID possible
      isLineCrossing = kTRUE;
      break;
    }
  }
  if(isLineCrossing) return 0;
  // Check whether distance from the electron line is smaller than n-sigma
  if(fPIDtpcESD->GetNumberOfSigmas(esdTrack, AliPID::kElectron) < fNsigmaTPC ) return 11;
  return 0;
}

//___________________________________________________________________
void AliHFEpidTPC::AddTPCdEdxLineCrossing(Int_t species, Double_t sigma){
  //
  // Add exclusion point for the TPC PID where a dEdx line crosses the electron line
  // Stores line center and line sigma
  //
  if(species >= AliPID::kSPECIES){
    AliError("Species doesn't exist");
    return;
  }
  fLineCrossingsEnabled |= 1 << species;
  fLineCrossingSigma[species] = sigma;
}

//___________________________________________________________________
Double_t AliHFEpidTPC::Likelihood(const AliESDtrack *track, Int_t species, Float_t rsig)
{
  //gives probability for track to come from a certain particle species;
  //no priors considered -> probabilities for equal abundances of all species!
  //optional restriction to r-sigma bands of different particle species; default: rsig = 2. (see below)

  //IMPORTANT: Tracks which are judged to be outliers get negative likelihoods -> unusable for combination with further detectors!
  
  if(!track) return -1.;
  Bool_t outlier = kTRUE;
  // Check whether distance from the respective particle line is smaller than r sigma
  for(Int_t hypo = 0; hypo < AliPID::kSPECIES; hypo++){
    if(fPIDtpcESD->GetNumberOfSigmas(track, (AliPID::EParticleType)hypo) > rsig)
      outlier = kTRUE;
    else {
	    outlier = kFALSE;
	    break;
	  }
  }
  if(outlier)
    return -2.;

  Double_t tpcProb[5];

  track->GetTPCpid(tpcProb);

  return tpcProb[species];
}

//___________________________________________________________________
Double_t  AliHFEpidTPC::Suppression(const AliESDtrack *track, Int_t species)
{
  //ratio of likelihoods to be whatever species/to be an electron;
  //as a cross-check for possible particle type suppression compared to electrons
  if(!track) return -20;
  if((Likelihood(track,species) == -2.)||(Likelihood(track,0)== -2.))
    return -30;
  if(Likelihood(track,species) == 0.)
    return -10;
  if (Likelihood(track,0) == 0.)
    return 10.;
  else
    return TMath::Log10(Likelihood(track,species)/(Likelihood(track,0)));


}

//___________________________________________________________________
void AliHFEpidTPC::FillTPChistograms(const AliESDtrack *track){
  //
  if(!track)
    return;
 
  Double_t tpcSignal = track->GetTPCsignal();
  Double_t p = track->GetInnerParam() ? track->GetInnerParam()->P() : track->P();
  if(HasMCData()){
    switch(TMath::Abs(GetPdgCode(const_cast<AliESDtrack *>(track)))){
      case 11:   (dynamic_cast<TH2I *>(fQAList->At(kHistTPCelectron)))->Fill(p, tpcSignal);
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCprobEl)))->Fill(p, Likelihood(track, 0));
	//histograms with ratio of likelihood to be electron/to be other species (a check for quality of likelihood PID);
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCenhanceElPi)))->Fill(p, -Suppression(track, 2));
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCenhanceElMu)))->Fill(p, -Suppression(track, 1));
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCenhanceElKa)))->Fill(p, -Suppression(track, 3));
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCenhanceElPro)))->Fill(p, -Suppression(track, 4));
	//___________________________________________________________________________________________
	//Likelihoods for electrons to be other particle species
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCElprobPi)))->Fill(p, Likelihood(track, 2));
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCElprobMu)))->Fill(p, Likelihood(track, 1));
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCElprobKa)))->Fill(p, Likelihood(track, 3));
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCElprobPro)))->Fill(p, Likelihood(track, 4));
	break;
	//___________________________________________________________________________________________
    case 13: (dynamic_cast<TH2I *>(fQAList->At(kHistTPCmuon)))->Fill(p, tpcSignal);
      //Likelihood of muon to be an electron
      (dynamic_cast<TH2F *>(fQAList->At(kHistTPCprobMu)))->Fill(p, Likelihood(track, 0));
      //ratio of likelihood for muon to be a muon/an electron -> indicator for quality of muon suppression
      //below functions are the same for other species
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCsuppressMu)))->Fill(p, Suppression(track, 1));
      break;
      case 211:  (dynamic_cast<TH2I *>(fQAList->At(kHistTPCpion)))->Fill(p, tpcSignal);
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCprobPi)))->Fill(p, Likelihood(track, 0));
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCsuppressPi)))->Fill(p, Suppression(track, 2));
          break;
      case 321:  (dynamic_cast<TH2I *>(fQAList->At(kHistTPCkaon)))->Fill(p, tpcSignal);
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCprobKa)))->Fill(p, Likelihood(track, 0));
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCsuppressKa)))->Fill(p, Suppression(track, 3));
          break;
      case 2212: (dynamic_cast<TH2I *>(fQAList->At(kHistTPCproton)))->Fill(p, tpcSignal);
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCprobPro)))->Fill(p, Likelihood(track, 0));
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCsuppressPro)))->Fill(p, Suppression(track, 4));
          break;
      default: (dynamic_cast<TH2I *>(fQAList->At(kHistTPCothers)))->Fill(p, tpcSignal);
	(dynamic_cast<TH2F *>(fQAList->At(kHistTPCprobOth)))->Fill(p, Likelihood(track, 0));

	break;
    }
  }
  //TPC signal and Likelihood to be electron for all tracks (independent of MC information)
  (dynamic_cast<TH2I *>(fQAList->At(kHistTPCall)))->Fill(p, tpcSignal);
 (dynamic_cast<TH2F *>(fQAList->At(kHistTPCprobAll)))->Fill(p, Likelihood(track, 0));
}

//___________________________________________________________________
void AliHFEpidTPC::AddQAhistograms(TList *qaList){
  fQAList = new TList;
  fQAList->SetName("fTPCqaHistos");

  fQAList->AddAt(new TH2I("fHistTPCelectron","TPC signal for Electrons", 200, 0, 20, 60, 0, 600), kHistTPCelectron); 
  fQAList->AddAt(new TH2I("fHistTPCmuon","TPC signal for Muons", 200, 0, 20, 60, 0, 600), kHistTPCmuon);
  fQAList->AddAt(new TH2I("fHistTPCpion","TPC signal for Pions", 200, 0, 20, 60, 0, 600), kHistTPCpion);
  fQAList->AddAt(new TH2I("fHistTPCkaon","TPC signal for Kaons", 200, 0, 20, 60, 0, 600), kHistTPCkaon);
  fQAList->AddAt(new TH2I("fHistTPCproton","TPC signal for Protons", 200, 0, 20, 60, 0, 600), kHistTPCproton);
  fQAList->AddAt(new TH2I("fHistTPCothers","TPC signal for other species", 200, 0, 20, 60, 0, 600), kHistTPCothers);
  fQAList->AddAt(new TH2I("fHistTPCall","TPC signal for all species", 200, 0, 20, 60, 0, 600), kHistTPCall);

  fQAList->AddAt(new TH2F("fHistTPCprobEl","TPC likelihood for electrons to be an electron vs. p", 200, 0.,20.,200,0.,1.), kHistTPCprobEl);
  fQAList->AddAt(new TH2F("fHistTPCprobPi","TPC likelihood for pions to be an electron vs. p",  200, 0.,20.,200, 0.,1.), kHistTPCprobPi);
  fQAList->AddAt(new TH2F("fHistTPCprobMu","TPC likelihood for muons to be an electron vs. p",  200, 0.,20.,200, 0.,1.), kHistTPCprobMu);
  fQAList->AddAt(new TH2F("fHistTPCprobKa","TPC likelihood for kaons to be an electron vs. p",  200, 0.,20.,200, 0.,1.), kHistTPCprobKa);
  fQAList->AddAt(new TH2F("fHistTPCprobPro","TPC likelihood for protons to be an electron vs. p",  200, 0.,20.,200, 0.,1.), kHistTPCprobPro);
  fQAList->AddAt(new TH2F("fHistTPCprobOth","TPC likelihood for other particles to be an electron vs. p",  200, 0.,20.,200, 0.,1.), kHistTPCprobOth);
  fQAList->AddAt(new TH2F("fHistTPCprobAll","TPC likelihood for all particles to be an electron vs. p",  200, 0.,20.,200, 0.,1.), kHistTPCprobAll);


 fQAList->AddAt(new TH2F("fHistTPCsuppressPi","log10 of TPC Likelihood(pion)/Likelihood(elec) for pions vs. p", 200, 0.,20.,200,-1.,5.8), kHistTPCsuppressPi);
 fQAList->AddAt(new TH2F("fHistTPCsuppressMu","log10 of TPC Likelihood(muon)/Likelihood(elec) for muons vs. p", 200, 0.,20.,200,-1.,5.8), kHistTPCsuppressMu);
 fQAList->AddAt(new TH2F("fHistTPCsuppressKa","log10 of TPC Likelihood(kaon)/Likelihood(elec) for kaons vs. p", 200, 0.,20.,200,-1.,5.8), kHistTPCsuppressKa);
 fQAList->AddAt(new TH2F("fHistTPCsuppressPro","log10 of TPC Likelihood(proton)/Likelihood(elec)for protons vs. p", 200, 0.,20.,200,-1.,5.8), kHistTPCsuppressPro);

 fQAList->AddAt(new TH2F("fHistTPCenhanceElPi","log10 of TPC Likelihood(elec)/Likelihood(pion) for electrons vs. p", 200, 0.,20.,200,-1.,5.8), kHistTPCsuppressPi);
 fQAList->AddAt(new TH2F("fHistTPCenhanceElMu","log10 of TPC Likelihood(elec)/Likelihood(muon) for electrons vs. p", 200, 0.,20.,200,-1.,5.8), kHistTPCsuppressMu);
 fQAList->AddAt(new TH2F("fHistTPCenhanceElKa","log10 of TPC Likelihood(elec)/Likelihood(kaon) for electrons vs. p", 200, 0.,20.,200,-1.,5.8), kHistTPCsuppressKa);
 fQAList->AddAt(new TH2F("fHistTPCenhanceElPro","log10 of TPC Likelihood(elec)/Likelihood(proton) for electrons vs. p", 200, 0.,20.,200,-1.,5.8), kHistTPCsuppressPro);

 fQAList->AddAt(new TH2F("fHistTPCElprobPi","TPC likelihood for electrons to be a pion vs. p", 200, 0.,20.,200,0.,1.), kHistTPCElprobPi);
 fQAList->AddAt(new TH2F("fHistTPCElprobMu","TPC likelihood for electrons to be a muon vs. p", 200, 0.,20.,200,0.,1.), kHistTPCElprobMu);
 fQAList->AddAt(new TH2F("fHistTPCElprobKa","TPC likelihood for electrons to be a kaon vs. p", 200, 0.,20.,200,0.,1.), kHistTPCElprobKa);
 fQAList->AddAt(new TH2F("fHistTPCElprobPro","TPC likelihood for electrons to be a proton vs. p", 200, 0.,20.,200,0.,1.), kHistTPCElprobPro);


  qaList->AddLast(fQAList);
}
