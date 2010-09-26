/**************************************************************************
 * Copyright(c) 1998-2010, ALICE Experiment at CERN, All rights reserved. *
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

//***********************************************************************
// Class AliHFPtSpectrum
// Base class for feed-down corrections on heavy-flavour decays
// computes the cross-section via one of the three implemented methods:
//   0) Consider no feed-down prediction 
//   1) Subtract the feed-down with the "fc" method 
//       Yield = Reco * fc;  where fc = 1 / ( 1 + (eff_b/eff_c)*(N_b/N_c) ) ;
//   2) Subtract the feed-down with the "Nb" method
//       Yield = Reco - Feed-down (exact formula on the function implementation)
//
//  (the corrected yields per bin are divided by the bin-width)
//
// Author: Z.Conesa, zconesa@in2p3.fr
//***********************************************************************

#include <Riostream.h>

#include "TMath.h"
#include "TH1.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TNamed.h"

#include "AliLog.h"
#include "AliHFPtSpectrum.h"

ClassImp(AliHFPtSpectrum)

//_________________________________________________________________________________________________________
AliHFPtSpectrum::AliHFPtSpectrum(const char* name, const char* title, Int_t option):
  TNamed(name,title),
  fhDirectMCpt(),
  fhFeedDownMCpt(),
  fhDirectMCptMax(),
  fhDirectMCptMin(),
  fhFeedDownMCptMax(),
  fhFeedDownMCptMin(),
  fhDirectEffpt(),
  fhFeedDownEffpt(),
  fhRECpt(),
  fLuminosity(),
  fTrigEfficiency(),
  fhFc(),
  fhFcMax(),
  fhFcMin(),
  fgFc(),
  fhYieldCorr(),
  fhYieldCorrMax(),
  fhYieldCorrMin(),
  fgYieldCorr(),
  fhSigmaCorr(),
  fhSigmaCorrMax(),
  fhSigmaCorrMin(),
  fgSigmaCorr(),
  fFeedDownOption(option),
  fAsymUncertainties(kTRUE)
{
  //
  // Default constructor
  //

  fLuminosity[0]=1.;  fLuminosity[1]=0.;  
  fTrigEfficiency[0]=1.; fTrigEfficiency[1]=0.; 

}

//_________________________________________________________________________________________________________
AliHFPtSpectrum::AliHFPtSpectrum(const AliHFPtSpectrum &rhs):
  TNamed(rhs),
  fhDirectMCpt(rhs.fhDirectMCpt),
  fhFeedDownMCpt(rhs.fhFeedDownMCpt),
  fhDirectMCptMax(rhs.fhDirectMCptMax),
  fhDirectMCptMin(rhs.fhDirectMCptMin),
  fhFeedDownMCptMax(rhs.fhFeedDownMCptMax),
  fhFeedDownMCptMin(rhs.fhFeedDownMCptMin),
  fhDirectEffpt(rhs.fhDirectEffpt),
  fhFeedDownEffpt(rhs.fhFeedDownEffpt),
  fhRECpt(rhs.fhRECpt),
  fLuminosity(),
  fTrigEfficiency(),
  fhFc(rhs.fhFc),
  fhFcMax(rhs.fhFcMax),
  fhFcMin(rhs.fhFcMin),
  fgFc(rhs.fgFc),
  fhYieldCorr(rhs.fhYieldCorr),
  fhYieldCorrMax(rhs.fhYieldCorrMax),
  fhYieldCorrMin(rhs.fhYieldCorrMin),
  fgYieldCorr(rhs.fgYieldCorr),
  fhSigmaCorr(rhs.fhSigmaCorr),
  fhSigmaCorrMax(rhs.fhSigmaCorrMax),
  fhSigmaCorrMin(rhs.fhSigmaCorrMin),
  fgSigmaCorr(rhs.fgSigmaCorr),
  fFeedDownOption(rhs.fFeedDownOption),
  fAsymUncertainties(rhs.fAsymUncertainties)
{
  //
  // Copy constructor
  //

  for(Int_t i=0; i<2; i++){
    fLuminosity[i] = rhs.fLuminosity[i];
    fTrigEfficiency[i] = rhs.fTrigEfficiency[i];
  }

}

//_________________________________________________________________________________________________________
AliHFPtSpectrum &AliHFPtSpectrum::operator=(const AliHFPtSpectrum &source){
  //
  // Assignment operator
  //

  if (&source == this) return *this;
  
  fhDirectMCpt = source.fhDirectMCpt;
  fhFeedDownMCpt = source.fhFeedDownMCpt;
  fhDirectMCptMax = source.fhDirectMCptMax;
  fhDirectMCptMin = source.fhDirectMCptMin;
  fhFeedDownMCptMax = source.fhFeedDownMCptMax;
  fhFeedDownMCptMin = source.fhFeedDownMCptMin;
  fhDirectEffpt = source.fhDirectEffpt;
  fhFeedDownEffpt = source.fhFeedDownEffpt;
  fhRECpt = source.fhRECpt;
  fhFc = source.fhFc;
  fhFcMax = source.fhFcMax;
  fhFcMin = source.fhFcMin;
  fgFc = source.fgFc;
  fhYieldCorr = source.fhYieldCorr;
  fhYieldCorrMax = source.fhYieldCorrMax;
  fhYieldCorrMin = source.fhYieldCorrMin;
  fgYieldCorr = source.fgYieldCorr;
  fhSigmaCorr = source.fhSigmaCorr;
  fhSigmaCorrMax = source.fhSigmaCorrMax;
  fhSigmaCorrMin = source.fhSigmaCorrMin;
  fgSigmaCorr = source.fgSigmaCorr;
  fFeedDownOption = source.fFeedDownOption;
  fAsymUncertainties = source.fAsymUncertainties;
  
  for(Int_t i=0; i<2; i++){
    fLuminosity[i] = source.fLuminosity[i];
    fTrigEfficiency[i] = source.fTrigEfficiency[i];
  }

  return *this;
}

//_________________________________________________________________________________________________________
AliHFPtSpectrum::~AliHFPtSpectrum(){
  //
  // Destructor
  //
  ;
}
  

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::SetMCptSpectra(TH1 *hDirect, TH1 *hFeedDown){
  //
  // Set the MonteCarlo or Theoretical spectra
  //  both for direct and feed-down contributions
  //
  
  if (!hDirect || !hFeedDown || !fhRECpt) {
    AliError("One or both (direct, feed-down) spectra or the reconstructed spectra don't exist");
    return;
  }

  Bool_t areconsistent = kTRUE;
  areconsistent = CheckHistosConsistency(hDirect,hFeedDown);
  if (!areconsistent) {
    AliInfo("Histograms are not consistent (bin width, bounds)"); 
    return;
  }

  //
  // Get the reconstructed spectra bins & limits
  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t *limits = new Double_t[nbins+1];
  Double_t xlow=0., binwidth=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
    //    cout <<" bin = " << i <<", bin width = "<< binwidth << ", low edge = " << xlow << ", high edge = " << xlow+binwidth <<endl;
  }
  limits[nbins] = xlow + binwidth;

  //
  // If the predictions shape do not have the same
  //  bin width (check via number of bins) as the reconstructed spectra, change them 
  Int_t nbinsMC = hDirect->GetNbinsX();
  if (hDirect->GetBinWidth(1) == fhRECpt->GetBinWidth(1)) {

    fhDirectMCpt = hDirect;
    fhFeedDownMCpt = hFeedDown;

  } 
  else {

    fhDirectMCpt = new TH1D("fhDirectMCpt"," direct theoretical prediction",nbins,limits);
    fhFeedDownMCpt = new TH1D("fhFeedDownMCpt"," feed-down theoretical prediction",nbins,limits);

    Double_t sumd[nbins], sumf[nbins], itemsd[nbins], itemsf[nbins];
    for (Int_t ibinrec=0; ibinrec<=nbins; ibinrec++){
      sumd[ibinrec]=0.; sumf[ibinrec]=0.; itemsd[ibinrec]=0.; itemsf[ibinrec]=0.;
    }
    for (Int_t ibin=0; ibin<=nbinsMC; ibin++){

      for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++){
	if( hDirect->GetBinCenter(ibin)>limits[ibinrec] && 
	    hDirect->GetBinCenter(ibin)<limits[ibinrec+1]){
	  sumd[ibinrec]+=hDirect->GetBinContent(ibin);
	  itemsd[ibinrec]+=1.;
	  //	  cout << " binrec="<< ibinrec << " sumd="<< sumd[ibinrec] << endl;
	}
	if( hFeedDown->GetBinCenter(ibin)>limits[ibinrec] && 
	    hFeedDown->GetBinCenter(ibin)<limits[ibinrec+1]){
	  sumf[ibinrec]+=hFeedDown->GetBinContent(ibin);
	  itemsf[ibinrec]+=1.;
	  //	  cout << " binrec="<< ibinrec << ", sumf=" << sumf[ibinrec] << endl;
	}
      }

    }

    // set the theoretical rebinned spectra to ( sum-bins / n-bins ) per new bin
    for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++) {
      fhDirectMCpt->SetBinContent(ibinrec+1,sumd[ibinrec]/itemsd[ibinrec]);
      fhFeedDownMCpt->SetBinContent(ibinrec+1,sumf[ibinrec]/itemsf[ibinrec]);
      //      cout << " Setting: binrec="<< ibinrec <<", at x="<<fhDirectMCpt->GetBinCenter(ibinrec)<< " sumd="<< sumd[ibinrec] << ", sumf=" << sumf[ibinrec] << endl;
    }

    // control plot
    TCanvas *cTheoryRebin = new TCanvas("cTheoryRebin","control the theoretical spectra rebin");
    cTheoryRebin->Divide(1,2);
    cTheoryRebin->cd(1);
    hDirect->Draw("");
    fhDirectMCpt->SetLineColor(2);
    fhDirectMCpt->Draw("same");
    cTheoryRebin->cd(2);
    hFeedDown->Draw("");
    fhFeedDownMCpt->SetLineColor(2);
    fhFeedDownMCpt->Draw("same");
    cTheoryRebin->Update();
  }

}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::SetFeedDownMCptSpectra(TH1 *hFeedDown){
  //
  // Set the MonteCarlo or Theoretical spectra
  //  for feed-down contribution
  //
  
  if (!hFeedDown || !fhRECpt) {
    AliError("Feed-down or reconstructed spectra don't exist");
    return;
  }

  //
  // Get the reconstructed spectra bins & limits
  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t *limits = new Double_t[nbins+1];
  Double_t xlow=0., binwidth=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
  }
  limits[nbins] = xlow + binwidth;

  //
  // If the predictions shape do not have the same
  //  bin width (check via number of bins) as the reconstructed spectra, change them 
  Int_t nbinsMC = hFeedDown->GetNbinsX();
  if (hFeedDown->GetBinWidth(1) == fhRECpt->GetBinWidth(1)) {

    fhFeedDownMCpt = hFeedDown;

  } 
  else {

    fhFeedDownMCpt = new TH1D("fhFeedDownMCpt"," feed-down theoretical prediction",nbins,limits);

    Double_t sumf[nbins], itemsf[nbins];
    for (Int_t ibin=0; ibin<=nbinsMC; ibin++){

      for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++){
	if( hFeedDown->GetBinCenter(ibin)>limits[ibinrec] && 
	    hFeedDown->GetBinCenter(ibin)<limits[ibinrec+1]){
	  sumf[ibinrec]+=hFeedDown->GetBinContent(ibin);
	  itemsf[ibinrec]+=1.;
	}
      }

    }

    // set the theoretical rebinned spectra to ( sum-bins / n-bins ) per new bin
    for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++) {
      fhFeedDownMCpt->SetBinContent(ibinrec+1,sumf[ibinrec]/itemsf[ibinrec]);
    }

    // control plot
    TCanvas *cTheoryRebin = new TCanvas("cTheoryRebin","control the theoretical spectra rebin");
    hFeedDown->Draw("");
    fhFeedDownMCpt->SetLineColor(2);
    fhFeedDownMCpt->Draw("same");
    cTheoryRebin->Update();
  }

}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::SetMCptDistributionsBounds(TH1 *hDirectMax, TH1 *hDirectMin, TH1 *hFeedDownMax, TH1 *hFeedDownMin){
  //
  // Set the maximum and minimum MonteCarlo or Theoretical spectra
  //  both for direct and feed-down contributions
  // used in case uncertainties are asymmetric and ca not be on the "basic histograms"
  //

  if (!hDirectMax || !hDirectMin || !hFeedDownMax|| !hFeedDownMin || !fhRECpt) {
    AliError("One or all of the max/min direct/feed-down or the reconstructed spectra don't exist");
    return;
  }

  Bool_t areconsistent = kTRUE; 
  areconsistent &= CheckHistosConsistency(hDirectMax,hDirectMin);
  areconsistent &= CheckHistosConsistency(hFeedDownMax,hFeedDownMin);
  areconsistent &= CheckHistosConsistency(hDirectMax,hFeedDownMax);
  if (!areconsistent) {
    AliInfo("Histograms are not consistent (bin width, bounds)"); 
    return;
  }

  //
  // Get the reconstructed spectra bins & limits
  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t *limits = new Double_t[nbins+1];
  Double_t xlow=0., binwidth=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
  }
  limits[nbins] = xlow + binwidth;

  //
  // If the predictions shape do not have the same
  //  bin width (check via number of bins) as the reconstructed spectra, change them 
  Int_t nbinsMC =  hDirectMax->GetNbinsX();
  if (hFeedDownMax->GetBinWidth(1) == fhRECpt->GetBinWidth(1)) {
    
    fhDirectMCptMax = hDirectMax;
    fhDirectMCptMin = hDirectMin;
    fhFeedDownMCptMax = hFeedDownMax;
    fhFeedDownMCptMin = hFeedDownMin; 

  } 
  else {

    fhDirectMCptMax = new TH1D("fhDirectMCptMax"," maximum direct theoretical prediction",nbins,limits);
    fhDirectMCptMin = new TH1D("fhDirectMCptMin"," minimum direct theoretical prediction",nbins,limits);
    fhFeedDownMCptMax = new TH1D("fhFeedDownMCptMax"," maximum feed-down theoretical prediction",nbins,limits);
    fhFeedDownMCptMin = new TH1D("fhFeedDownMCptMin"," minimum feed-down theoretical prediction",nbins,limits);

    Double_t sumdmax[nbins], sumfmax[nbins], itemsdmax[nbins], itemsfmax[nbins];
    Double_t sumdmin[nbins], sumfmin[nbins], itemsdmin[nbins], itemsfmin[nbins];
    for (Int_t ibin=0; ibin<=nbinsMC; ibin++){

      for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++){
	if( hDirectMax->GetBinCenter(ibin)>limits[ibinrec] && 
	    hDirectMax->GetBinCenter(ibin)<limits[ibinrec+1]){
	  sumdmax[ibinrec]+=hDirectMax->GetBinContent(ibin);
	  itemsdmax[ibinrec]+=1.;
	}
	if( hDirectMin->GetBinCenter(ibin)>limits[ibinrec] && 
	    hDirectMin->GetBinCenter(ibin)<limits[ibinrec+1]){
	  sumdmin[ibinrec]+=hDirectMin->GetBinContent(ibin);
	  itemsdmin[ibinrec]+=1.;
	}
	if( hFeedDownMax->GetBinCenter(ibin)>limits[ibinrec] && 
	    hFeedDownMax->GetBinCenter(ibin)<limits[ibinrec+1]){
	  sumfmax[ibinrec]+=hFeedDownMax->GetBinContent(ibin);
	  itemsfmax[ibinrec]+=1.;
	}
	if( hFeedDownMin->GetBinCenter(ibin)>limits[ibinrec] && 
	    hFeedDownMin->GetBinCenter(ibin)<limits[ibinrec+1]){
	  sumfmin[ibinrec]+=hFeedDownMin->GetBinContent(ibin);
	  itemsfmin[ibinrec]+=1.;
	}
      }

    }

    // set the theoretical rebinned spectra to ( sum-bins / n-bins ) per new bin
    for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++) {
      fhDirectMCptMax->SetBinContent(ibinrec+1,sumdmax[ibinrec]/itemsdmax[ibinrec]);
      fhDirectMCptMin->SetBinContent(ibinrec+1,sumdmin[ibinrec]/itemsdmin[ibinrec]);
      fhFeedDownMCptMax->SetBinContent(ibinrec+1,sumfmax[ibinrec]/itemsfmax[ibinrec]);
      fhFeedDownMCptMin->SetBinContent(ibinrec+1,sumfmin[ibinrec]/itemsfmin[ibinrec]);
    }

    // control plot
    TCanvas *cTheoryRebinLimits = new TCanvas("cTheoryRebinLimits","control the theoretical spectra limits rebin");
    cTheoryRebinLimits->Divide(1,2);
    cTheoryRebinLimits->cd(1);
    hDirectMax->Draw("");
    fhDirectMCptMax->SetLineColor(2);
    fhDirectMCptMax->Draw("same");
    hDirectMin->Draw("same");
    fhDirectMCptMin->SetLineColor(2);
    fhDirectMCptMin->Draw("same");
    cTheoryRebinLimits->cd(2);
    hFeedDownMax->Draw("");
    fhFeedDownMCptMax->SetLineColor(2);
    fhFeedDownMCptMax->Draw("same");
    hFeedDownMin->Draw("same");
    fhFeedDownMCptMin->SetLineColor(2);
    fhFeedDownMCptMin->Draw("same");
    cTheoryRebinLimits->Update();
  }

}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::SetFeedDownMCptDistributionsBounds(TH1 *hFeedDownMax, TH1 *hFeedDownMin){
  //
  // Set the maximum and minimum MonteCarlo or Theoretical spectra
  //   for feed-down contributions
  // used in case uncertainties are asymmetric and can not be on the "basic histogram"
  //

  if (!hFeedDownMax || !hFeedDownMin || !fhRECpt) {
    AliError("One or all of the max/min direct/feed-down spectra don't exist");
    return;
  }

  Bool_t areconsistent = kTRUE; 
  areconsistent &= CheckHistosConsistency(hFeedDownMax,hFeedDownMin);
  if (!areconsistent) {
    AliInfo("Histograms are not consistent (bin width, bounds)"); 
    return;
  }

  //
  // Get the reconstructed spectra bins & limits
  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t *limits = new Double_t[nbins+1];
  Double_t xlow=0., binwidth=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
  }
  limits[nbins] = xlow + binwidth;

  //
  // If the predictions shape do not have the same
  //  bin width (check via number of bins) as the reconstructed spectra, change them 
  Int_t nbinsMC = hFeedDownMax->GetNbinsX();
  if (hFeedDownMax->GetBinWidth(1) == fhRECpt->GetBinWidth(1)) {
    
    fhFeedDownMCptMax = hFeedDownMax;
    fhFeedDownMCptMin = hFeedDownMin; 

  } 
  else {

    fhFeedDownMCptMax = new TH1D("fhFeedDownMCptMax"," maximum feed-down theoretical prediction",nbins,limits);
    fhFeedDownMCptMin = new TH1D("fhFeedDownMCptMin"," minimum feed-down theoretical prediction",nbins,limits);

    Double_t sumfmax[nbins], itemsfmax[nbins];
    Double_t sumfmin[nbins], itemsfmin[nbins];
    for (Int_t ibin=0; ibin<=nbinsMC; ibin++){

      for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++){
	if( hFeedDownMax->GetBinCenter(ibin)>limits[ibinrec] && 
	    hFeedDownMax->GetBinCenter(ibin)<limits[ibinrec+1]){
	  sumfmax[ibinrec]+=hFeedDownMax->GetBinContent(ibin);
	  itemsfmax[ibinrec]+=1.;
	}
	if( hFeedDownMin->GetBinCenter(ibin)>limits[ibinrec] && 
	    hFeedDownMin->GetBinCenter(ibin)<limits[ibinrec+1]){
	  sumfmin[ibinrec]+=hFeedDownMin->GetBinContent(ibin);
	  itemsfmin[ibinrec]+=1.;
	}
      }

    }

    // set the theoretical rebinned spectra to ( sum-bins / n-bins ) per new bin
    for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++) {
      fhFeedDownMCptMax->SetBinContent(ibinrec+1,sumfmax[ibinrec]/itemsfmax[ibinrec]);
      fhFeedDownMCptMin->SetBinContent(ibinrec+1,sumfmin[ibinrec]/itemsfmin[ibinrec]);
    }

    // control plot
    TCanvas *cTheoryRebinLimits = new TCanvas("cTheoryRebinLimits","control the theoretical spectra limits rebin");
    hFeedDownMax->Draw("");
    fhFeedDownMCptMax->SetLineColor(2);
    fhFeedDownMCptMax->Draw("same");
    hFeedDownMin->Draw("same");
    fhFeedDownMCptMin->SetLineColor(2);
    fhFeedDownMCptMin->Draw("same");
    cTheoryRebinLimits->Update();
  }

}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::SetDirectAccEffCorrection(TH1 *hDirectEff){
  //
  // Set the Acceptance and Efficiency corrections 
  //   for the direct contribution
  //
  
  if (!hDirectEff) {
    AliError("The direct acceptance and efficiency corrections doesn't exist");
    return;
  }

  fhDirectEffpt = hDirectEff;
}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::SetAccEffCorrection(TH1 *hDirectEff, TH1 *hFeedDownEff){
  //
  // Set the Acceptance and Efficiency corrections 
  //  both for direct and feed-down contributions
  //
  
  if (!hDirectEff || !hFeedDownEff) {
    AliError("One or both (direct, feed-down) acceptance and efficiency corrections don't exist");
    return;
  }

  Bool_t areconsistent=kTRUE;
  areconsistent = CheckHistosConsistency(hDirectEff,hFeedDownEff);
  if (!areconsistent) {
    AliInfo("Histograms are not consistent (bin width, bounds)"); 
    return;
  }

  fhDirectEffpt = hDirectEff;
  fhFeedDownEffpt = hFeedDownEff;
}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::SetReconstructedSpectrum(TH1 *hRec) {
  //
  // Set the reconstructed spectrum
  //
  
  if (!hRec) {
    AliError("The reconstructed spectrum doesn't exist");
    return;
  }

  fhRECpt = hRec;
}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::ComputeHFPtSpectrum(Double_t deltaY, Double_t branchingRatioC, Double_t branchingRatioBintoFinalDecay) {
  //
  // Main function to compute the corrected cross-section:
  // variables : analysed delta_y, BR for the final correction,
  //             BR b --> D --> decay (relative to the input theoretical prediction)
  //
  //   Sigma = ( 1. / (lumi * delta_y * BR_c * eff_trig * eff_c ) ) * spectra (corrected for feed-down)
  //
  // Uncertainties: delta_sigma = sigma * sqrt ( (delta_reco/reco)^2 + (delta_lumi/lumi)^2 + (delta_eff_trig/eff_trig)^2  )

  //
  // First: Initialization
  //
  Bool_t areHistosOk = Initialize();
  if (!areHistosOk) {
    AliInfo(" Histos not properly initialized. Check : inconsistent binning ? missing histos ?");
    return;
  }

  //
  // Second: Correct for feed-down
  //
  if (fFeedDownOption==1) {
    // Compute the feed-down correction via fc-method
    CalculateFeedDownCorrectionFc(); 
    // Correct the yield for feed-down correction via fc-method
    CalculateFeedDownCorrectedSpectrumFc(); 
  }
  else if (fFeedDownOption==2) {
    // Correct the yield for feed-down correction via Nb-method
    CalculateFeedDownCorrectedSpectrumNb(deltaY,branchingRatioBintoFinalDecay); 
  }
  else if (fFeedDownOption==0) { 
    // If there is no need for feed-down correction,
    //    the "corrected" yield is equal to the raw yield
    fhYieldCorr = fhRECpt;
    fhYieldCorr->SetNameTitle("fhYieldCorr","un-corrected yield");
    fhYieldCorrMax = fhRECpt;
    fhYieldCorrMin = fhRECpt;
    fhYieldCorrMax->SetNameTitle("fhYieldCorrMax","un-corrected yield");
    fhYieldCorrMin->SetNameTitle("fhYieldCorrMin","un-corrected yield");
    fAsymUncertainties=kFALSE;
  }
  else { 
    AliInfo(" Are you sure the feed-down correction option is right ?"); 
  }

  // Print out information
  printf("\n\n     Correcting the spectra with : \n   luminosity = %2.2e +- %2.2e, trigger efficiency = %2.2e +- %2.2e, \n    delta_y = %2.2f, BR_c = %2.2e, BR_b_decay = %2.2e \n\n",fLuminosity[0],fLuminosity[1],fTrigEfficiency[0],fTrigEfficiency[1],deltaY,branchingRatioC,branchingRatioBintoFinalDecay);

  //
  // Finally: Correct from yields to cross-section
  //
  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t binwidth = fhRECpt->GetBinWidth(1);
  Double_t *limits = new Double_t[nbins+1]; 
  Double_t xlow=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
  }
  limits[nbins] = xlow + binwidth;

  
  // declare the output histograms
  TH1D *hSigmaCorr = new TH1D("hSigmaCorr","corrected sigma",nbins,limits);
  TH1D *hSigmaCorrMax = new TH1D("hSigmaCorrMax","max corrected sigma",nbins,limits);
  TH1D *hSigmaCorrMin = new TH1D("hSigmaCorrMin","min corrected sigma",nbins,limits);
  // and the output TGraphAsymmErrors
  if (fAsymUncertainties & !fgSigmaCorr) fgSigmaCorr = new TGraphAsymmErrors(nbins);

  // protect against null denominator
  if (deltaY==0. || fLuminosity[0]==0. || fTrigEfficiency[0]==0. || branchingRatioC==0.) {
    AliError(" Hey you ! Why luminosity or trigger-efficiency or the c-BR or delta_y are set to zero ?! ");
    return ;
  }

  Double_t value=0, valueMax=0., valueMin=0.;
  for(Int_t ibin=0; ibin<=nbins; ibin++){

    //   Sigma = ( 1. / (lumi * delta_y * BR_c * eff_trig * eff_c ) ) * spectra (corrected for feed-down)
    value = (fhDirectEffpt->GetBinContent(ibin) && fhDirectEffpt->GetBinContent(ibin)!=0.) ? 
      ( fhYieldCorr->GetBinContent(ibin) / ( deltaY * branchingRatioC * fLuminosity[0] * fTrigEfficiency[0] * fhDirectEffpt->GetBinContent(ibin) ) )
      : 0. ;
    //    cout << " bin="<< ibin << " yield-corr="<< fhYieldCorr->GetBinContent(ibin) <<", eff_D="<< fhDirectEffpt->GetBinContent(ibin) <<", value="<<value<<endl;
    
    // Uncertainties: delta_sigma = sigma * sqrt ( (delta_reco/reco)^2 + (delta_lumi/lumi)^2 + (delta_eff_trig/eff_trig)^2  )
    if (fAsymUncertainties) {
      valueMax = value * TMath::Sqrt( (fgYieldCorr->GetErrorYhigh(ibin)/fhYieldCorr->GetBinContent(ibin))* (fgYieldCorr->GetErrorYhigh(ibin)/fhYieldCorr->GetBinContent(ibin)) + 
				       (fLuminosity[1]/fLuminosity[0])*(fLuminosity[1]/fLuminosity[0]) + 
				       (fTrigEfficiency[1]/fTrigEfficiency[0])*(fTrigEfficiency[1]/fTrigEfficiency[0])  );
      valueMin = value * TMath::Sqrt( (fgYieldCorr->GetErrorYlow(ibin)/fhYieldCorr->GetBinContent(ibin))* (fgYieldCorr->GetErrorYlow(ibin)/fhYieldCorr->GetBinContent(ibin)) + 
				       (fLuminosity[1]/fLuminosity[0])*(fLuminosity[1]/fLuminosity[0]) + 
				       (fTrigEfficiency[1]/fTrigEfficiency[0])*(fTrigEfficiency[1]/fTrigEfficiency[0])  );
    }
    else {
      // protect against null denominator
      valueMax = (value!=0.) ?
	value * TMath::Sqrt( (fhYieldCorr->GetBinError(ibin)/fhYieldCorr->GetBinContent(ibin))* (fhYieldCorr->GetBinError(ibin)/fhYieldCorr->GetBinContent(ibin)) + 
			     (fLuminosity[1]/fLuminosity[0])*(fLuminosity[1]/fLuminosity[0]) + 
			     (fTrigEfficiency[1]/fTrigEfficiency[0])*(fTrigEfficiency[1]/fTrigEfficiency[0])  )
	: 0. ;
      valueMin = valueMax;
    }
    
    // Fill the histograms
    hSigmaCorr->SetBinContent(ibin,value);
    hSigmaCorr->SetBinError(ibin,valueMax);
    hSigmaCorrMax->SetBinContent(ibin,valueMax);
    hSigmaCorrMin->SetBinContent(ibin,valueMin);
    // Fill the TGraphAsymmErrors
    if (fAsymUncertainties) {
      Double_t x = fhYieldCorr->GetBinCenter(ibin);
      fgSigmaCorr->SetPoint(ibin,x,value); // i,x,y
      fgSigmaCorr->SetPointError(ibin,(binwidth/2.),(binwidth/2.),valueMin,valueMax); // i,xl,xh,yl,yh
    }
    
  }

  fhSigmaCorr = hSigmaCorr ;
  fhSigmaCorrMax = hSigmaCorrMax ;
  fhSigmaCorrMin = hSigmaCorrMin ;
}

//_________________________________________________________________________________________________________
TH1 * AliHFPtSpectrum::EstimateAndSetDirectEfficiencyRecoBin(TH1 *hSimu, TH1 *hReco) {
  //
  // Function that computes the Direct  acceptance and efficiency correction
  //  based on the simulated and reconstructed spectra
  //  and using the reconstructed spectra bin width
  //
  //  eff = reco/sim ; err_eff = sqrt( eff*(1-eff) )/ sqrt( sim )
  // 

  if(!fhRECpt){
    AliInfo("Hey, the reconstructed histogram was not set yet !"); 
    return NULL;
  }

  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t *limits = new Double_t[nbins+1];
  Double_t xlow=0.,binwidth=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
  }
  limits[nbins] = xlow + binwidth;

  fhDirectEffpt = new TH1D("fhDirectEffpt"," direct acceptance #times efficiency",nbins,limits);
  
  Double_t sumSimu[nbins], sumReco[nbins];
  for (Int_t ibin=0; ibin<=hSimu->GetNbinsX(); ibin++){

    for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++){
      if ( hSimu->GetBinCenter(ibin)>limits[ibinrec] && 
	   hSimu->GetBinCenter(ibin)<limits[ibinrec+1] ) {
	sumSimu[ibinrec]+=hSimu->GetBinContent(ibin);
      }
      if ( hReco->GetBinCenter(ibin)>limits[ibinrec] && 
	   hReco->GetBinCenter(ibin)<limits[ibinrec+1] ) {
	sumReco[ibinrec]+=hReco->GetBinContent(ibin);
      }
    }
    
  }

  // the efficiency is computed as reco/sim (in each bin)
  //  its uncertainty is err_eff = sqrt( eff*(1-eff) )/ sqrt( sim )
  Double_t eff=0., erreff=0.;
  for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++) {
    eff = sumReco[ibinrec] / sumSimu[ibinrec] ;
    erreff = TMath::Sqrt( eff * (1.0 - eff) ) / TMath::Sqrt( sumSimu[ibinrec] );
    fhDirectEffpt->SetBinContent(ibinrec+1,eff);
    fhDirectEffpt->SetBinError(ibinrec+1,erreff);
  }

  return (TH1*)fhDirectEffpt;
}

//_________________________________________________________________________________________________________
TH1 * AliHFPtSpectrum::EstimateAndSetFeedDownEfficiencyRecoBin(TH1 *hSimu, TH1 *hReco) {
  //
  // Function that computes the Feed-Down acceptance and efficiency correction
  //  based on the simulated and reconstructed spectra
  //  and using the reconstructed spectra bin width
  //
  //  eff = reco/sim ; err_eff = sqrt( eff*(1-eff) )/ sqrt( sim )
  // 
  
  if(!fhRECpt){
    AliInfo("Hey, the reconstructed histogram was not set yet !"); 
    return NULL;
  }

  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t *limits = new Double_t[nbins+1];
  Double_t xlow=0.,binwidth=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
  }
  limits[nbins] = xlow + binwidth;

  fhFeedDownEffpt = new TH1D("fhFeedDownEffpt"," feed-down acceptance #times efficiency",nbins,limits);
  
  Double_t sumSimu[nbins], sumReco[nbins];
  for (Int_t ibin=0; ibin<=hSimu->GetNbinsX(); ibin++){

    for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++){
      if ( hSimu->GetBinCenter(ibin)>limits[ibinrec] && 
	   hSimu->GetBinCenter(ibin)<limits[ibinrec+1] ) {
	sumSimu[ibinrec]+=hSimu->GetBinContent(ibin);
      }
      if ( hReco->GetBinCenter(ibin)>limits[ibinrec] && 
	   hReco->GetBinCenter(ibin)<limits[ibinrec+1] ) {
	sumReco[ibinrec]+=hReco->GetBinContent(ibin);
      }
    }
    
  }
  
  // the efficiency is computed as reco/sim (in each bin)
  //  its uncertainty is err_eff = sqrt( eff*(1-eff) )/ sqrt( sim )
  Double_t eff=0., erreff=0.;
  for (Int_t ibinrec=0; ibinrec<nbins; ibinrec++) {
    eff = sumReco[ibinrec] / sumSimu[ibinrec] ;
    erreff = TMath::Sqrt( eff * (1.0 - eff) ) / TMath::Sqrt( sumSimu[ibinrec] );
    fhFeedDownEffpt->SetBinContent(ibinrec+1,eff);
    fhFeedDownEffpt->SetBinError(ibinrec+1,erreff);
  }

  return (TH1*)fhFeedDownEffpt;
}

//_________________________________________________________________________________________________________
Bool_t AliHFPtSpectrum::Initialize(){
  //
  // Initialization of the variables (histograms)
  //

  if (fFeedDownOption==0) { 
    AliInfo("Getting ready for the corrections without feed-down consideration");
  } else if (fFeedDownOption==1) { 
    AliInfo("Getting ready for the fc feed-down correction calculation");
  } else if (fFeedDownOption==2) {
    AliInfo("Getting ready for the Nb feed-down correction calculation");
  } else { AliError("The calculation option must be <=2"); return kFALSE; }

  // Start checking the input histograms consistency
  Bool_t areconsistent=kTRUE;

  // General checks 
  if (!fhDirectEffpt || !fhRECpt) {
    AliError(" Reconstructed spectra and/or the Nc efficiency distributions are not defined");
    return kFALSE;
  }
  areconsistent &= CheckHistosConsistency(fhRECpt,fhDirectEffpt);
  if (!areconsistent) {
    AliInfo("Histograms required for Nb correction are not consistent (bin width, bounds)"); 
    return kFALSE;
  }
  if (fFeedDownOption==0) return kTRUE;

  //
  // Common checks for options 1 (fc) & 2(Nb)
  if (!fhFeedDownMCpt || !fhFeedDownEffpt) {
    AliError(" Theoretical Nb and/or the Nb efficiency distributions are not defined");
    return kFALSE;
  }
  areconsistent &= CheckHistosConsistency(fhRECpt,fhFeedDownMCpt);
  areconsistent &= CheckHistosConsistency(fhFeedDownMCpt,fhFeedDownEffpt);
  if (fAsymUncertainties) {
    if (!fhFeedDownMCptMax || !fhFeedDownMCptMin) {
      AliError(" Max/Min theoretical Nb distributions are not defined");
      return kFALSE;
    }
    areconsistent &= CheckHistosConsistency(fhFeedDownMCpt,fhFeedDownMCptMax);
  }
  if (!areconsistent) {
    AliInfo("Histograms required for Nb correction are not consistent (bin width, bounds)"); 
    return kFALSE;
  }
  if (fFeedDownOption>1) return kTRUE;  

  //
  // Now checks for option 1 (fc correction) 
  if (!fhDirectMCpt) {
    AliError("Theoretical Nc distributions is not defined");
    return kFALSE;
  }
  areconsistent &= CheckHistosConsistency(fhDirectMCpt,fhFeedDownMCpt);
  areconsistent &= CheckHistosConsistency(fhDirectMCpt,fhDirectEffpt);
  if (fAsymUncertainties) {
    if (!fhDirectMCptMax || !fhDirectMCptMin) {
      AliError(" Max/Min theoretical Nc distributions are not defined");
      return kFALSE;
    }
    areconsistent &= CheckHistosConsistency(fhDirectMCpt,fhDirectMCptMax);
  }
  if (!areconsistent) {
    AliInfo("Histograms required for fc correction are not consistent (bin width, bounds)"); 
    return kFALSE;
  }

  return kTRUE;
}

//_________________________________________________________________________________________________________
Bool_t AliHFPtSpectrum::CheckHistosConsistency(TH1 *h1, TH1 *h2){
  //
  // Check the histograms consistency (bins, limits)
  //

  if (!h1 || !h2) {
    AliError("One or both histograms don't exist");
    return kFALSE;
  }

  Double_t binwidth1 = h1->GetBinWidth(1);
  Double_t binwidth2 = h2->GetBinWidth(1);
  Double_t min1 = h1->GetBinCenter(1) - (binwidth1/2.) ; 
//   Double_t max1 = h1->GetBinCenter(nbins1) + (binwidth1/2.) ;
  Double_t min2 = h2->GetBinCenter(1) - (binwidth2/2.) ;
//   Double_t max2 = h2->GetBinCenter(nbins2) + (binwidth2/2.) ;

  if (binwidth1!=binwidth2) {
    AliInfo(" histograms with different bin width");
    return kFALSE;
  }
  if (min1!=min2) {
    AliInfo(" histograms with different minimum");
    return kFALSE;
  }
//   if (max1!=max2) {
//     AliInfo(" histograms with different maximum");
//     return kFALSE;
//   }

  return kTRUE;
}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::CalculateFeedDownCorrectionFc(){ 
  //
  // Compute fc factor and its uncertainties bin by bin
  //   fc = 1 / ( 1 + (eff_b/eff_c)*(N_b/N_c) ) 
  //
  
  // define the variables
  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t binwidth = fhRECpt->GetBinWidth(1);
  Double_t *limits = new Double_t[nbins+1];
  Double_t xlow=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
  }
  limits[nbins] = xlow + binwidth;

  Double_t correction=1.;
  Double_t correctionMax=1., correctionMin=1.;
  Double_t theoryRatio=1.;
  Double_t effRatio=1.; 
  
  // declare the output histograms
  TH1D *hfc = new TH1D("hfc","fc correction factor",nbins,limits);
  TH1D *hfcMax = new TH1D("hfcMax","max fc correction factor",nbins,limits);
  TH1D *hfcMin = new TH1D("hfcMin","min fc correction factor",nbins,limits);
  // two local control histograms
  TH1D *hTheoryRatio = new TH1D("hTheoryRatio","Theoretical B-->D over c-->D (feed-down/direct) ratio",nbins,limits);
  TH1D *hEffRatio = new TH1D("hEffRatio","Efficiency B-->D over c-->D (feed-down/direct) ratio",nbins,limits);
  // and the output TGraphAsymmErrors
  if (fAsymUncertainties & !fgFc) fgFc = new TGraphAsymmErrors(nbins);

  //
  // Compute fc
  //
  for (Int_t ibin=0; ibin<=nbins; ibin++) {

    //  theory_ratio = (N_b/N_c) 
    theoryRatio = (fhDirectMCpt->GetBinContent(ibin) && fhDirectMCpt->GetBinContent(ibin)!=0.) ? fhFeedDownMCpt->GetBinContent(ibin) / fhDirectMCpt->GetBinContent(ibin) : 1.0 ;

    //  eff_ratio = (eff_b/eff_c)
    effRatio = (fhDirectEffpt->GetBinContent(ibin) && fhDirectEffpt->GetBinContent(ibin)!=0.) ? fhFeedDownEffpt->GetBinContent(ibin) / fhDirectEffpt->GetBinContent(ibin) : 1.0 ;

    //   fc = 1 / ( 1 + (eff_b/eff_c)*(N_b/N_c) ) 
    correction = (effRatio && theoryRatio) ? ( 1. / ( 1 + ( effRatio * theoryRatio ) ) ) : 1.0 ;

    // Calculate the uncertainty [ considering only the theoretical uncertainties on Nb & Nc for now !!! ]
    // delta_fc = fc^2 * (Nb/Nc) * sqrt ( (delta_Nb/Nb)^2 + (delta_Nc/Nc)^2 ) 
    Double_t deltaNbMax = fhFeedDownMCptMax->GetBinContent(ibin) - fhFeedDownMCpt->GetBinContent(ibin) ;
    Double_t deltaNbMin = fhFeedDownMCpt->GetBinContent(ibin) - fhFeedDownMCptMin->GetBinContent(ibin) ;
    Double_t deltaNcMax = fhDirectMCptMax->GetBinContent(ibin) - fhDirectMCpt->GetBinContent(ibin) ;
    Double_t deltaNcMin = fhDirectMCpt->GetBinContent(ibin) - fhDirectMCptMin->GetBinContent(ibin) ;

    // Protect against null denominator. If so, define uncertainty as null
    if (fhFeedDownMCpt->GetBinContent(ibin) && fhFeedDownMCpt->GetBinContent(ibin)!=0. && 
	fhDirectMCpt->GetBinContent(ibin) && fhDirectMCpt->GetBinContent(ibin)!=0. ) {
      correctionMax = correction*correction*theoryRatio * 
	TMath::Sqrt( 
		    (deltaNbMax/fhFeedDownMCpt->GetBinContent(ibin))*(deltaNbMax/fhFeedDownMCpt->GetBinContent(ibin)) + 
		    (deltaNcMax/fhDirectMCpt->GetBinContent(ibin))*(deltaNcMax/fhDirectMCpt->GetBinContent(ibin)) 
		     );
      correctionMin = correction*correction*theoryRatio * 
	TMath::Sqrt( 
		    (deltaNbMin/fhFeedDownMCpt->GetBinContent(ibin))*(deltaNbMin/fhFeedDownMCpt->GetBinContent(ibin)) + 
		    (deltaNcMin/fhDirectMCpt->GetBinContent(ibin))*(deltaNcMin/fhDirectMCpt->GetBinContent(ibin)) 
		     );
    }
    else { correctionMax = 0.; correctionMin = 0.; }


    // Fill in the histograms
    hTheoryRatio->SetBinContent(ibin,theoryRatio);
    hEffRatio->SetBinContent(ibin,effRatio);
    hfc->SetBinContent(ibin,correction);
    hfcMax->SetBinContent(ibin,correction+correctionMax);
    hfcMin->SetBinContent(ibin,correction-correctionMin);
    if (fAsymUncertainties) {
      Double_t x = fhDirectMCpt->GetBinCenter(ibin);
      fgFc->SetPoint(ibin,x,correction); // i,x,y
      fgFc->SetPointError(ibin,(binwidth/2.),(binwidth/2.),correctionMin,correctionMax); // i,xl,xh,yl,yh
    }

  }

  fhFc = hfc;
  fhFcMax = hfcMax;
  fhFcMin = hfcMin;
}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::CalculateFeedDownCorrectedSpectrumFc(){
  //
  // Compute the feed-down corrected spectrum if feed-down correction is done via fc factor (bin by bin)
  //    physics = reco * fc / bin-width
  //
  //    uncertainty: delta_physics = physics * sqrt ( (delta_reco/reco)^2 + (delta_fc/fc)^2 )
  //
  //    ( Calculation done bin by bin )

  if (!fhFc || !fhRECpt) {
    AliError(" Reconstructed or fc distributions are not defined");
    return;
  }

  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t value = 0., valueDmax= 0., valueDmin= 0.;
  Double_t binwidth = fhRECpt->GetBinWidth(1);
  Double_t *limits = new Double_t[nbins+1];
  Double_t xlow=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
  }
 limits[nbins] = xlow + binwidth;
  
  // declare the output histograms
  TH1D *hYield = new TH1D("hYield","corrected yield (by fc)",nbins,limits);
  TH1D *hYieldMax = new TH1D("hYieldMax","max corrected yield (by fc)",nbins,limits);
  TH1D *hYieldMin = new TH1D("hYieldMin","min corrected yield (by fc)",nbins,limits);
  // and the output TGraphAsymmErrors
  if (fAsymUncertainties & !fgYieldCorr) fgYieldCorr = new TGraphAsymmErrors(nbins);
  
  //
  // Do the calculation
  // 
  for (Int_t ibin=0; ibin<=nbins; ibin++) {

    // calculate the value 
    value = fhRECpt->GetBinContent(ibin) * fhFc->GetBinContent(ibin) ;
    value /= fhRECpt->GetBinWidth(ibin) ;
    //    cout << " bin="<< ibin << " reco="<<fhRECpt->GetBinContent(ibin) <<", fc="<< fhFc->GetBinContent(ibin) <<", value="<<value<<endl;

    // calculate the value uncertainty
    // Protect against null denominator. If so, define uncertainty as null
    if (fhRECpt->GetBinContent(ibin) && fhRECpt->GetBinContent(ibin)!=0.) {

      if (fAsymUncertainties) {

	if (fhFc->GetBinContent(ibin) && fhFc->GetBinContent(ibin)!=0.) {
	  valueDmax = value * TMath::Sqrt( ( (fhRECpt->GetBinError(ibin)/fhRECpt->GetBinContent(ibin))*(fhRECpt->GetBinError(ibin)/fhRECpt->GetBinContent(ibin)) ) + ( (fgFc->GetErrorYhigh(ibin)/fhFc->GetBinContent(ibin))*(fgFc->GetErrorYhigh(ibin)/fhFc->GetBinContent(ibin)) )  );
	  valueDmin = value * TMath::Sqrt( ( (fhRECpt->GetBinError(ibin)/fhRECpt->GetBinContent(ibin))*(fhRECpt->GetBinError(ibin)/fhRECpt->GetBinContent(ibin)) ) + ( (fgFc->GetErrorYlow(ibin)/fhFc->GetBinContent(ibin))*(fgFc->GetErrorYlow(ibin)/fhFc->GetBinContent(ibin)) ) );
	}
	else { valueDmax = 0.; valueDmin = 0.; }

      }
      else { // Don't consider fc uncertainty in this case [ to be tested!!! ]
	valueDmax = value * (fhRECpt->GetBinError(ibin)/fhRECpt->GetBinContent(ibin)) ; 
	valueDmin = value * (fhRECpt->GetBinError(ibin)/fhRECpt->GetBinContent(ibin)) ;
      }

    }
    else { valueDmax = 0.; valueDmin = 0.; }
    
    // fill in the histograms
    hYield->SetBinContent(ibin,value);
    hYield->SetBinError(ibin,valueDmax);
    hYieldMax->SetBinContent(ibin,value+valueDmax); 
    hYieldMin->SetBinContent(ibin,value-valueDmin);
    if (fAsymUncertainties) {
      Double_t center = hYield->GetBinCenter(ibin);
      fgYieldCorr->SetPoint(ibin,center,value); // i,x,y
      fgYieldCorr->SetPointError(ibin,(binwidth/2.),(binwidth/2.),valueDmin,valueDmax); // i,xl,xh,yl,yh
    }

  }
  
  fhYieldCorr =  hYield;
  fhYieldCorrMax = hYieldMax; 
  fhYieldCorrMin = hYieldMin; 
}

//_________________________________________________________________________________________________________
void AliHFPtSpectrum::CalculateFeedDownCorrectedSpectrumNb(Double_t deltaY, Double_t branchingRatioBintoFinalDecay) {
  //
  // Compute the feed-down corrected spectrum if feed-down correction is done via Nb (bin by bin)
  //    physics =  [ reco  - (lumi * delta_y * BR_b * eff_trig * eff_b * Nb_th) ] / bin-width
  //
  //    uncertainty: delta_physics = sqrt ( (delta_reco)^2 + (k*delta_lumi/lumi)^2 + 
  //                                        (k*delta_eff_trig/eff_trig)^2 + (k*delta_Nb/Nb)^2 )
  //                    where k = lumi * delta_y * BR_b * eff_trig * eff_b * Nb_th
  //

  Int_t nbins = fhRECpt->GetNbinsX();
  Double_t binwidth = fhRECpt->GetBinWidth(1);
  Double_t value = 0., valueDmax = 0., valueDmin = 0., kfactor = 0.;
  Double_t *limits = new Double_t[nbins+1];
  Double_t xlow=0.;
  for (Int_t i=0; i<=nbins; i++) {
    binwidth = fhRECpt->GetBinWidth(i);
    xlow = fhRECpt->GetBinLowEdge(i);
    limits[i-1] = xlow;
  }
 limits[nbins] = xlow + binwidth;
  
  // declare the output histograms
  TH1D *hYield = new TH1D("hYield","corrected yield (by Nb)",nbins,limits);
  TH1D *hYieldMax = new TH1D("hYieldMax","max corrected yield (by Nb)",nbins,limits);
  TH1D *hYieldMin = new TH1D("hYieldMin","min corrected yield (by Nb)",nbins,limits);
  // and the output TGraphAsymmErrors
  if (fAsymUncertainties & !fgYieldCorr) fgYieldCorr = new TGraphAsymmErrors(nbins);

  //
  // Do the calculation
  // 
  for (Int_t ibin=0; ibin<=nbins; ibin++) {
    
    // calculate the value
    value = fhRECpt->GetBinContent(ibin) - (deltaY*branchingRatioBintoFinalDecay*fLuminosity[0]*fTrigEfficiency[0]*fhFeedDownEffpt->GetBinContent(ibin)*fhFeedDownMCpt->GetBinContent(ibin) );
    value /= fhRECpt->GetBinWidth(ibin);

    kfactor = deltaY*branchingRatioBintoFinalDecay*fLuminosity[0]*fTrigEfficiency[0]*fhFeedDownEffpt->GetBinContent(ibin)*fhFeedDownMCpt->GetBinContent(ibin) ;

    // calculate the value uncertainty
    if (fAsymUncertainties) {
      Double_t Nb =  fhFeedDownMCpt->GetBinContent(ibin);
      Double_t NbDmax = fhFeedDownMCptMax->GetBinContent(ibin) - fhFeedDownMCpt->GetBinContent(ibin);
      Double_t NbDmin = fhFeedDownMCpt->GetBinContent(ibin) - fhFeedDownMCptMin->GetBinContent(ibin);
      valueDmax = TMath::Sqrt( ( fhRECpt->GetBinError(ibin)*fhRECpt->GetBinError(ibin) ) +
			       ( (kfactor*fLuminosity[1]/fLuminosity[0])*(kfactor*fLuminosity[1]/fLuminosity[0]) ) +
			       ( (kfactor*fTrigEfficiency[1]/fTrigEfficiency[0])*(kfactor*fTrigEfficiency[1]/fTrigEfficiency[0]) ) +
			       ( (kfactor*NbDmax/Nb)*(kfactor*NbDmax/Nb) ) 	);
      valueDmin =  TMath::Sqrt( ( fhRECpt->GetBinError(ibin)*fhRECpt->GetBinError(ibin) ) +
				( (kfactor*fLuminosity[1]/fLuminosity[0])*(kfactor*fLuminosity[1]/fLuminosity[0]) ) +
				( (kfactor*fTrigEfficiency[1]/fTrigEfficiency[0])*(kfactor*fTrigEfficiency[1]/fTrigEfficiency[0]) ) +
				( (kfactor*NbDmin/Nb)*(kfactor*NbDmin/Nb) ) 	);
    }
    else{ // Don't consider Nb uncertainty in this case [ to be tested!!! ]
      valueDmax =  TMath::Sqrt( ( fhRECpt->GetBinError(ibin)*fhRECpt->GetBinError(ibin) ) +
				( (kfactor*fLuminosity[1]/fLuminosity[0])*(kfactor*fLuminosity[1]/fLuminosity[0]) ) +
				( (kfactor*fTrigEfficiency[1]/fTrigEfficiency[0])*(kfactor*fTrigEfficiency[1]/fTrigEfficiency[0]) ) 	);
      valueDmin =  valueDmax ;
    }
    
    // fill in histograms
    hYield->SetBinContent(ibin,value);
    hYield->SetBinError(ibin,valueDmax);
    hYieldMax->SetBinContent(ibin,value+valueDmax); 
    hYieldMin->SetBinContent(ibin,value-valueDmin);
    if (fAsymUncertainties) {
      Double_t x = hYield->GetBinCenter(ibin);
      fgYieldCorr->SetPoint(ibin,x,value); // i,x,y
      fgYieldCorr->SetPointError(ibin,(binwidth/2.),(binwidth/2.),valueDmin,valueDmax); // i,xl,xh,yl,yh
    }

  }
  
  fhYieldCorr =  hYield;
  fhYieldCorrMax = hYieldMax; 
  fhYieldCorrMin = hYieldMin; 
}


//_________________________________________________________________________________________________________
TH1 * AliHFPtSpectrum::ReweightHisto(TH1 *hToReweight, TH1 *hReference){
  //
  // Function to  reweight histograms for testing purposes: 
  // This function takes the histo hToReweight and reweights 
  //  it (its pt shape) with respect to hReference 
  // 

  // check histograms consistency
  Bool_t areconsistent=kTRUE;
  areconsistent &= CheckHistosConsistency(hToReweight,hReference);
  if (!areconsistent) {
    AliInfo("the histograms to reweight are not consistent (bin width, bounds)"); 
    return NULL;
  }

  // define a new empty histogram
  TH1 *hReweighted = (TH1*)hToReweight->Clone("hReweighted");
  hReweighted->Reset();
  Double_t weight=1.0;
  Double_t yvalue=1.0; 
  Double_t integralRef = hReference->Integral();
  Double_t integralH = hToReweight->Integral();

  // now reweight the spectra
  //
  // the weight is the relative probability of the given pt bin in the reference histo
  //  divided by its relative probability (to normalize it) on the histo to re-weight
  for (Int_t i=0; i<=hToReweight->GetNbinsX(); i++) {
    weight = (hReference->GetBinContent(i)/integralRef) / (hToReweight->GetBinContent(i)/integralH) ;
    yvalue = hToReweight->GetBinContent(i);
    hReweighted->SetBinContent(i,yvalue*weight);
  }

  return (TH1*)hReweighted;
}

//_________________________________________________________________________________________________________
TH1 * AliHFPtSpectrum::ReweightRecHisto(TH1 *hRecToReweight, TH1 *hMCToReweight, TH1 *hMCReference){
  //
  // Function to  reweight histograms for testing purposes: 
  // This function takes the histo hToReweight and reweights 
  //  it (its pt shape) with respect to hReference /hMCToReweight
  // 

  // check histograms consistency
  Bool_t areconsistent=kTRUE;
  areconsistent &= CheckHistosConsistency(hMCToReweight,hMCReference);
  areconsistent &= CheckHistosConsistency(hRecToReweight,hMCReference);
  if (!areconsistent) {
    AliInfo("the histograms to reweight are not consistent (bin width, bounds)"); 
    return NULL;
  }

  // define a new empty histogram
  TH1 *hReweighted = (TH1*)hMCToReweight->Clone("hReweighted");
  hReweighted->Reset();
  TH1 *hRecReweighted = (TH1*)hRecToReweight->Clone("hRecReweighted");
  hRecReweighted->Reset();
  Double_t weight=1.0;
  Double_t yvalue=1.0, yrecvalue=1.0; 
  Double_t integralRef = hMCReference->Integral();
  Double_t integralH = hMCToReweight->Integral();

  // now reweight the spectra
  //
  // the weight is the relative probability of the given pt bin 
  //  that should be applied in the MC histo to get the reference histo shape
  //  Probabilities are properly normalized.
  for (Int_t i=0; i<=hMCToReweight->GetNbinsX(); i++) {
    weight = (hMCReference->GetBinContent(i)/integralRef) / (hMCToReweight->GetBinContent(i)/integralH) ;
    yvalue = hMCToReweight->GetBinContent(i);
    hReweighted->SetBinContent(i,yvalue*weight);
    yrecvalue = hRecToReweight->GetBinContent(i);
    hRecReweighted->SetBinContent(i,yrecvalue*weight);
  }

  return (TH1*)hRecReweighted;
}

