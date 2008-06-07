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

#include "Riostream.h"                 //needed as include
#include "AliFlowCommonConstants.h"    //needed as include
#include "AliFlowCommonHistResults.h"

#include "TString.h" 
#include "TH1D.h"   //needed as include
#include "TMath.h"  //needed as include

class TH1F;
class AliFlowVector;
class AliFlowCommonHist;

// AliFlowCommonHistResults:
// Class to organize the common histograms for Flow Analysis
// Holds v2(pt), integrated v2 and chi (resolution)
//
// authors: N.K A.B. R.S

ClassImp(AliFlowCommonHistResults)

//-----------------------------------------------------------------------

  AliFlowCommonHistResults::AliFlowCommonHistResults(TString input): TObject(),
  fHistIntFlow(0),
  fHistDiffFlow(0),
  fHistChi(0)
  {
  //constructor creating histograms 
  Int_t fNbinsPt = AliFlowCommonConstants::GetNbinsPt();
  TString name;

  Double_t  fPtMin = AliFlowCommonConstants::GetPtMin();	     
  Double_t  fPtMax = AliFlowCommonConstants::GetPtMax();
  
  //integrated flow
  name = "Flow_Integrated_";
  name +=input;
  fHistIntFlow = new TH1D(name.Data(), name.Data(),1,0.5,1.5);
  fHistIntFlow ->SetXTitle("");
  fHistIntFlow ->SetYTitle("Integrated Flow value (%)");

  //differential flow
  name = "Flow_Differential_Pt_";
  name +=input;
  fHistDiffFlow = new TH1D(name.Data(), name.Data(),fNbinsPt,fPtMin,fPtMax);
  fHistDiffFlow ->SetXTitle("Pt");
  fHistDiffFlow ->SetYTitle("v (%)");
  
  //Chi (needed for rebinning later on)
  name = "Flow_Chi_";
  name +=input;
  fHistChi = new TH1D(name.Data(), name.Data(),1,0.5,1.5);
  fHistChi ->SetXTitle("");
  fHistChi ->SetYTitle("Chi");
  }

//----------------------------------------------------------------------- 

AliFlowCommonHistResults::~AliFlowCommonHistResults()
{
  //deletes histograms
  delete fHistIntFlow;
  delete fHistDiffFlow;
  delete fHistChi;
}

//----------------------------------------------------------------------- 

Bool_t AliFlowCommonHistResults::FillIntegratedFlow(Double_t fV, Double_t fError)
{
  //Fill fHistIntFlow
  fHistIntFlow -> SetBinContent(1,fV);
  fHistIntFlow -> SetBinError(1,fError);

  return kTRUE; 
}

//----------------------------------------------------------------------- 

Bool_t AliFlowCommonHistResults::FillDifferentialFlow(Int_t fBin, Double_t fv, Double_t fError)
{
  //Fill fHistDiffFlow
  fHistDiffFlow ->SetBinContent(fBin,fv);
  fHistDiffFlow ->SetBinError(fBin,fError);

  return kTRUE; 
}

//----------------------------------------------------------------------- 

Bool_t AliFlowCommonHistResults::FillChi(Double_t fChi)
{
  //Fill fHistChi
  fHistChi -> SetBinContent(1,fChi);
  
  return kTRUE; 
}

//----------------------------------------------------------------------- 


