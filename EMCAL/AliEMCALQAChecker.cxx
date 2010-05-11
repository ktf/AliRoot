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

/*
  Checks the quality assurance. 
  By comparing with reference data

  Based on PHOS code written by
  Y. Schutz CERN July 2007

 For the moment we only implement the checking of raw data QA.
 The checked for ESD and RecPoints will be implemented later.
 

 */


// --- ROOT system ---
#include <TClass.h>
#include <TH1.h> 
#include <TF1.h> 
#include <TH1I.h> 
#include <TH2F.h>
#include <TIterator.h> 
#include <TKey.h> 
#include <TFile.h> 
#include <TLine.h>
#include <TText.h>
#include <TPaveText.h>
#include <TMath.h>

// --- Standard library ---

// --- AliRoot header files ---
#include "AliLog.h"
#include "AliQAv1.h"
#include "AliQAChecker.h"
#include "AliEMCALQAChecker.h"

ClassImp(AliEMCALQAChecker)

//__________________________________________________________________
AliEMCALQAChecker::AliEMCALQAChecker() : 
AliQACheckerBase("EMCAL","EMCAL Quality Assurance Data Maker"),
fTextSM(new TText*[fknSM]),
fLineCol(new TLine(48.0,0.,48.0,47.0)),
fLineRow(new TLine(0.,24.,96.,24.)),
fText(new TPaveText(0.2,40.,1.6,90.,"br"))
{
  // ctor
  fLineCol->SetLineColor(1);
  fLineCol->SetLineWidth(2);
  fLineRow->SetLineColor(1);
  fLineRow->SetLineWidth(2);

  fTextSM[0]= new TText(20, 12, "SM A0");
  fTextSM[1]= new TText(20, 38, "SM A1");
  fTextSM[2]= new TText(64, 12, "SM C0");
  fTextSM[3]= new TText(64, 38, "SM C1");
}          

//__________________________________________________________________
AliEMCALQAChecker::~AliEMCALQAChecker() 
{
	/// dtor
  delete [] fTextSM ;
  delete fLineCol ;
  delete fLineRow ;
  delete fText  ; 
}

//__________________________________________________________________
AliEMCALQAChecker::AliEMCALQAChecker(const AliEMCALQAChecker& qac) : 
AliQACheckerBase(qac.GetName(), qac.GetTitle()), 
fTextSM(new TText*[fknSM]) ,
fLineCol(static_cast<TLine*>(qac.fLineCol->Clone())) , 
fLineRow(static_cast<TLine*>(qac.fLineRow->Clone())) , 
fText(new TPaveText(0.2,40.,1.6,90.,"br"))
{
   // copy ctor 
  for (Int_t sm = 0 ; sm < fknSM ; sm++){
    fTextSM[sm] = static_cast<TText *>(qac.fTextSM[sm]->Clone()) ;
  }

}   
//__________________________________________________________________
AliEMCALQAChecker& AliEMCALQAChecker::operator = (const AliEMCALQAChecker &qac) 
{
  fTextSM  = new TText*[fknSM] ;
  fLineCol = static_cast<TLine*>(qac.fLineCol->Clone()) ; 
  fLineRow = static_cast<TLine*>(qac.fLineRow->Clone()) ; 
  fText    = new TPaveText(0.2,40.,1.6,90.,"br") ;
  for (Int_t sm = 0 ; sm < fknSM ; sm++){
    fTextSM[sm] = static_cast<TText *>(qac.fTextSM[sm]->Clone()) ;
  }
  return *this ;
}

//______________________________________________________________________________
void AliEMCALQAChecker::Check(Double_t * test, AliQAv1::ALITASK_t index, TObjArray ** list, const AliDetectorRecoParam * /*recoParam*/)
{
	/// Check objects in list
	
	if ( index == AliQAv1::kRAW ) 
	{
    CheckRaws(test, list);
		printf ("checkers for task %d \n", index) ;		
	}
	
	if ( index == AliQAv1::kREC)
	{
    CheckRecPoints(test, list);
	}
	
	if ( index == AliQAv1::kESD )
	{
    CheckESD(test, list);
	}
	AliWarning(Form("Checker for task %d not implement for the moment",index));
}

//______________________________________________________________________________
TH1* 
AliEMCALQAChecker::GetHisto(TObjArray* list, const char* hname, Int_t specie) const
{
	/// Get a given histo from the list
	TH1* h = static_cast<TH1*>(list->FindObject(Form("%s_%s",AliRecoParam::GetEventSpecieName(specie),hname)));
	if (!h)
	{
		AliError(Form("Did not find expected histo %s",hname));
	}
	return h;
}

//______________________________________________________________________________
Double_t 
AliEMCALQAChecker::MarkHisto(TH1& histo, Double_t value) const
{
	/// Mark histo as originator of some QA error/warning
	
	if ( value != 1.0 )
	{
		histo.SetBit(AliQAv1::GetQABit());
	}
	
	return value;
}


//______________________________________________________________________________
void AliEMCALQAChecker::CheckRaws(Double_t * test, TObjArray ** list)
{
  //  Check RAW QA histograms	
  //  -- Yaxian Mao, CCNU/CERN/LPSC
  //adding new checking method: 25/04/2010, Yaxian Mao
  //Comparing the amplitude from current run to the reference run, if the ratio in the range [0.8, .12], count as a good tower.
  //If more than 90% towers are good, EMCAL works fine, otherwise experts should be contacted. 
  
   				
  //Float_t kThreshold = 80. ; 
  Int_t nTowersPerSM = 24*48; // number of towers in a SuperModule; 24x48
  Double_t nTot = fknSM * nTowersPerSM ;
  for (Int_t specie = 0 ; specie < AliRecoParam::kNSpecies ; specie++) {
    test[specie] = 0.0 ; 
    if ( !AliQAv1::Instance()->IsEventSpecieSet(specie)) 
      continue ; 
    if (list[specie]->GetEntries() == 0)  
      test[specie] = 0. ; // nothing to check
    else {
      TH2F * hdata  = (TH2F*)list[specie]->At(k2DRatioAmp) ; 
      TH1F * ratio = (TH1F*)list[specie]->At(kRatioDist) ;
      if(hdata->GetEntries()==0 || ratio->GetEntries()==0)
        continue;
      //adding the lines to distinguish different SMs
      if ( hdata->GetListOfFunctions()->GetEntries() == 0 ){
        hdata->GetListOfFunctions()->Add(fLineCol); 
        hdata->GetListOfFunctions()->Add(fLineRow); 
        //Now adding the text to for each SM
        for(Int_t iSM = 0 ; iSM < fknSM ; iSM++){  //number of SMs loop start
          hdata->GetListOfFunctions()->Add(fTextSM[iSM]); 
	}
      }   
      if ( ratio->GetListOfFunctions()->GetEntries() == 0 ){
        ratio->GetListOfFunctions()->Add(fText) ;
      }

      //now check the ratio histogram
      Double_t binContent = 0. ;  
      Int_t NGoodTower = 0 ;
      Double_t rv = 0. ;
      for(Int_t ix = 1; ix <= hdata->GetNbinsX(); ix++) {
        for(Int_t iy = 1; iy <= hdata->GetNbinsY(); iy++) {
          binContent = hdata->GetBinContent(ix, iy) ; 
          if (binContent < 1.2 && binContent > 0.8) 
            NGoodTower++ ;
        }
      }
      rv = NGoodTower/nTot ; 
      printf("%2.2f %% towers out of range [0.8, 1.2]\n", (1-rv)*100);
      if(fText){
        fText->Clear() ; 
      }      
      fText->AddText(Form("%2.2f %% towers out of range [0.8, 1.2]", (1-rv)*100));     
      if (rv < 0.9) {
        test[specie] = 0.9 ;
        // 2 lines text info for quality         
        fText->SetFillColor(2) ;
        fText->AddText(Form("EMCAL = NOK, CALL EXPERTS!!!")); 
      }
      else {
        test[specie] = 0.1 ;
        fText->SetFillColor(3) ;
        fText->AddText(Form("EMCAL = OK, ENJOY...")); 
      }
     } 
    } //finish the checking
}

//______________________________________________________________________________
void AliEMCALQAChecker::Init(const AliQAv1::DETECTORINDEX_t det) 
{
	/// intialises QA and QA checker settings
	AliQAv1::Instance(det) ; 
	Float_t hiValue[AliQAv1::kNBIT] ; 
	Float_t lowValue[AliQAv1::kNBIT] ;
	lowValue[AliQAv1::kINFO]      = 0.0   ; 
	hiValue[AliQAv1::kINFO]       = 0.1 ; 
	lowValue[AliQAv1::kWARNING]   = 0.1 ; 
  hiValue[AliQAv1::kWARNING]    = 0.5 ; 
	lowValue[AliQAv1::kERROR]     = 0.5   ; 
	hiValue[AliQAv1::kERROR]      = 0.8 ; 
	lowValue[AliQAv1::kFATAL]     = 0.8   ; 
	hiValue[AliQAv1::kFATAL]      = 1.0 ; 
	SetHiLo(&hiValue[0], &lowValue[0]) ; 
}

