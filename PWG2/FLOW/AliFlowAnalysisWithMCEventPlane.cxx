/*************************************************************************
* Copyright(c) 1998-2008, ALICE Experiment at CERN, All rights reserved. *
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

#define AliFlowAnalysisWithMCEventPlane_cxx
 
#include "Riostream.h"  //needed as include
#include "TFile.h"      //needed as include
#include "TProfile.h"   //needed as include
#include "TComplex.h"   //needed as include
#include "TList.h"

class TH1F;

#include "AliFlowCommonConstants.h"    //needed as include
#include "AliFlowEventSimple.h"
#include "AliFlowTrackSimple.h"
#include "AliFlowCommonHist.h"
#include "AliFlowCommonHistResults.h"
#include "AliFlowAnalysisWithMCEventPlane.h"

class AliFlowVector;

// AliFlowAnalysisWithMCEventPlane:
// Description: Maker to analyze Flow from the generated MC reaction plane.
//              This class is used to get the real value of the flow 
//              to compare the other methods to when analysing simulated events
// author: N. van der Kolk (kolk@nikhef.nl)

ClassImp(AliFlowAnalysisWithMCEventPlane)

  //-----------------------------------------------------------------------
 
 AliFlowAnalysisWithMCEventPlane::AliFlowAnalysisWithMCEventPlane():
   fQsum(NULL),
   fQ2sum(0),
   fEventNumber(0),
   fDebug(kFALSE),
   fHistList(NULL),
   fCommonHists(NULL),
   fCommonHistsRes(NULL),
   fHistProFlow(NULL),
   fHistRP(NULL)

{

  // Constructor.
  fHistList = new TList();

  fQsum = new TVector2;        // flow vector sum
}

 
 //-----------------------------------------------------------------------


 AliFlowAnalysisWithMCEventPlane::~AliFlowAnalysisWithMCEventPlane() 
 {
   //destructor
   delete fHistList;
   delete fQsum;
 }
 

//-----------------------------------------------------------------------
void AliFlowAnalysisWithMCEventPlane::Init() {

  //Define all histograms
  cout<<"---Analysis with the real MC Event Plane---"<<endl;

  Int_t iNbinsPt = AliFlowCommonConstants::GetNbinsPt();
  Double_t  dPtMin = AliFlowCommonConstants::GetPtMin();	     
  Double_t  dPtMax = AliFlowCommonConstants::GetPtMax();

  fCommonHists = new AliFlowCommonHist("AliFlowCommonHistMCEP");
  fHistList->Add(fCommonHists);
  fCommonHistsRes = new AliFlowCommonHistResults("AliFlowCommonHistResultsMCEP");
  fHistList->Add(fCommonHistsRes);
  
  fHistProFlow = new TProfile("FlowPro_VPt_MCEP","FlowPro_VPt_MCEP",iNbinsPt,dPtMin,dPtMax);
  fHistProFlow->SetXTitle("Pt");
  fHistProFlow->SetYTitle("v2 (%)");
  fHistList->Add(fHistProFlow);

  fHistRP = new TH1F("Flow_RP_MCEP","Flow_RP_MCEP",100,0.,3.14);
  fHistRP->SetXTitle("Reaction Plane Angle");
  fHistRP->SetYTitle("Counts");
  fHistList->Add(fHistRP);

 
  fEventNumber = 0;  //set number of events to zero
      
} 
 
//-----------------------------------------------------------------------
 
void AliFlowAnalysisWithMCEventPlane::Make(AliFlowEventSimple* anEvent, Double_t aRP) {

  //Calculate v2 from the MC reaction plane
  if (anEvent) {
         
    //fill control histograms     
    fCommonHists->FillControlHistograms(anEvent);

    //get the Q vector from the FlowEvent
    AliFlowVector vQ = anEvent->GetQ(); 
    //cout<<"vQ.Mod() = " << vQ.Mod() << endl;
    //for chi calculation:
    *fQsum += vQ;
    //cout<<"fQsum.Mod() = "<<fQsum.Mod()<<endl;
    fQ2sum += vQ.Mod2();
    //cout<<"fQ2sum = "<<fQ2sum<<endl;
        
    fHistRP->Fill(aRP);   
              
    //calculate flow
    //loop over the tracks of the event
    Int_t iNumberOfTracks = anEvent->NumberOfTracks(); 
    for (Int_t i=0;i<iNumberOfTracks;i++) 
      {
	AliFlowTrackSimple* pTrack = anEvent->GetTrack(i) ; 
	if (pTrack){
	  if (pTrack->UseForDifferentialFlow()) {
	    Double_t dPhi = pTrack->Phi();
	    //if (dPhi<0.) dPhi+=2*TMath::Pi();
	    //calculate flow v2:
	    Double_t dv2 = TMath::Cos(2*(dPhi-aRP));
	    Double_t dPt = pTrack->Pt();
	    //fill histogram
	    fHistProFlow->Fill(dPt,100*dv2);  
	  }  
	}//track selected
      }//loop over tracks
	  
    fEventNumber++;
    cout<<"@@@@@ "<<fEventNumber<<" events processed"<<endl;
  }
}

  //--------------------------------------------------------------------    
void AliFlowAnalysisWithMCEventPlane::Finish() {
   
  //*************make histograms etc. 
  if (fDebug) cout<<"AliFlowAnalysisWithMCEventPlane::Terminate()"<<endl;
     
  Int_t iNbinsPt = AliFlowCommonConstants::GetNbinsPt();
   
  
  TH1F* fHistPtDiff = fCommonHists->GetHistPtDiff();
  Double_t dV = 0.;
  Double_t dErrV = 0.;
  Double_t dSum = 0.;
  for(Int_t b=0;b<iNbinsPt;b++){
    Double_t dv2pro = 0.;
    Double_t dErrdifcomb = 0.; 
    if(fHistProFlow) {
      dv2pro = fHistProFlow->GetBinContent(b);
      dErrdifcomb = fHistProFlow->GetBinError(b); //in case error from profile is correct
      //fill TH1D
      fCommonHistsRes->FillDifferentialFlow(b, dv2pro, dErrdifcomb); 
      if (fHistPtDiff){
	//integrated flow
	Double_t dYield = fHistPtDiff->GetBinContent(b);
	dV += dv2pro/100*dYield ;
	dSum += dYield;
	//error on integrated flow
	dErrV += dYield*dYield*(dErrdifcomb/100)*(dErrdifcomb/100);
      }
    } else { cout<<"fHistProFlow is NULL"<<endl; }
  }
  if (dSum != 0. ) {
    dV /= dSum;  //because pt distribution should be normalised
    dErrV /= dSum*dSum;
    dErrV = TMath::Sqrt(dErrV); }
  cout<<"dV is "<<dV<<" +- "<<dErrV<<endl;
  fCommonHistsRes->FillIntegratedFlow(dV,dErrV); 
      	  
  cout<<".....finished"<<endl;
}

 
 
