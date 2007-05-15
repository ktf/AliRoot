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

//-----------------------------------------------------------------
//   Implementation of the derived class for track residuals
//   based on Chi2 minimization
//
//-----------------------------------------------------------------

#include <TVirtualFitter.h>
#include <TGeoMatrix.h>

#include "AliAlignObj.h"
#include "AliTrackPointArray.h"
#include "AliTrackResidualsChi2.h"

void Fcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *x, Int_t iflag);


ClassImp(AliTrackResidualsChi2)


//______________________________________________________________________________
void Fcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag)
{
  // This function is called by minuit
  // The corresponding member method is called
  // using SetObjectFit/GetObjectFit methods of TMinuit
  AliTrackResidualsChi2* dummy = (AliTrackResidualsChi2 *)TVirtualFitter::GetFitter()->GetObjectFit();
  dummy->Chi2(npar, gin, f, par, iflag);
}

//______________________________________________________________________________
Bool_t AliTrackResidualsChi2::Minimize()
{
  // Implementation of Chi2 based minimization
  // of track residuala sum
  // 
  // CHOLM: Modified to use the TVirtualFitter interface only.  This
  // makes it possible other fitters than TMinuit (say TMinuit2,
  // TFumili) by simply changing ones configuration file, or setting
  // it up in a script.  Much more robust and flexible. 
  Double_t arglist[10];
  Int_t ierflg = 0;
  TVirtualFitter *fitter = TVirtualFitter::Fitter(this,6);  //initialize TMinuit
  arglist[0] = -1;
  ierflg = fitter->ExecuteCommand("SET PRINT", arglist, 1);

  fitter->SetFCN(Fcn);

  arglist[0] = 1;
  ierflg = fitter->ExecuteCommand("SET ERR", arglist ,1);

  // Set starting values and step sizes for parameters
  Double_t pars[6] = {0,0,0,0,0,0};
  Double_t step[6] = {0.0001,0.0001,0.0001,0.0001,0.0001,0.0001};
  ierflg = fitter->SetParameter(0, "dx", pars[0], step[0], 0,0);
  ierflg = fitter->SetParameter(1, "dy", pars[1], step[1], 0,0);
  ierflg = fitter->SetParameter(2, "dz", pars[2], step[2], 0,0);
  ierflg = fitter->SetParameter(3, "psi", pars[3], step[3], 0,0);
  ierflg = fitter->SetParameter(4, "theta", pars[4], step[4], 0,0);
  ierflg = fitter->SetParameter(5, "phi", pars[5], step[5], 0,0);

  // Now ready for minimization step
  arglist[0] = 500;
  arglist[1] = 1.;
  fitter->ExecuteCommand("MIGRAD", arglist ,2);

  // Print results
  Double_t amin,edm,errdef;
  Int_t nvpar,nparx,icstat;
  icstat = fitter->GetStats(amin,edm,errdef,nvpar,nparx);
  fChi2 = amin; fNdf -= nvpar;

  return kTRUE;
}

//______________________________________________________________________________
void AliTrackResidualsChi2::Chi2(Int_t & /* npar */, Double_t * /* gin */, Double_t &f, Double_t *par, Int_t /* iflag */)
{
  // Chi2 function to be minimized
  // Sums all the track residuals
  Double_t chi2 = 0;

  fAlignObj->SetPars(par[0],par[1],par[2],par[3],par[4],par[5]);

  AliTrackPoint p1,p2;

  Bool_t count = kFALSE;
  if (fNdf == 0) count = kTRUE;

  for (Int_t itrack = 0; itrack < fLast; itrack++) {
    if (!fVolArray[itrack] || !fTrackArray[itrack]) continue;
    for (Int_t ipoint = 0; ipoint < fVolArray[itrack]->GetNPoints(); ipoint++) {
      fVolArray[itrack]->GetPoint(p1,ipoint);
      fAlignObj->Transform(p1);
      fTrackArray[itrack]->GetPoint(p2,ipoint);
      Float_t residual = p2.GetResidual(p1,kTRUE);
      chi2 += residual;
      if (count) fNdf += 3;
    }
  }
  f = chi2;
}
