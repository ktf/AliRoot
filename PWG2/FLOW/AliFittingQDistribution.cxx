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

/******************************** 
 * integrated flow estimate by  *
 *   fitting q-distribution     * 
 *                              *
 * author: Ante Bilandzic       * 
 *          (anteb@nikhef.nl)   *
 *******************************/ 

#define AliFittingQDistribution_cxx

#include "Riostream.h"
#include "AliFlowCommonConstants.h"
#include "AliFlowCommonHist.h"
#include "AliFlowCommonHistResults.h"
#include "TChain.h"
#include "TFile.h"
#include "TList.h" 
#include "TParticle.h"
#include "TProfile.h"
#include "AliFlowEventSimple.h"
#include "AliFlowTrackSimple.h"
#include "AliFittingQDistribution.h"

class TH1;
class TGraph;
class TPave;
class TLatex;
class TMarker;
class TObjArray;
class TList;
class TCanvas;
class TSystem;
class TROOT;
class AliFlowVector;
class TVector;

//================================================================================================================

ClassImp(AliFittingQDistribution)

AliFittingQDistribution::AliFittingQDistribution():  
 fTrack(NULL),
 fHistList(NULL),
 fAvMultIntFlowFQD(NULL),
 fIntFlowResultsFQD(NULL),
 fCommonHists(NULL),
 fQDistributionFQD(NULL)
{
 //constructor 
 fHistList = new TList(); 
}

AliFittingQDistribution::~AliFittingQDistribution()
{
 //desctructor
 delete fHistList; 
}

//================================================================================================================

void AliFittingQDistribution::CreateOutputObjects()
{
 //various output histograms
 
 //avarage multiplicity 
 fAvMultIntFlowFQD = new TProfile("fAvMultIntFlowFQD","Average Multiplicity",1,0,1,"s");
 fAvMultIntFlowFQD->SetXTitle("");
 fAvMultIntFlowFQD->SetYTitle("");
 fAvMultIntFlowFQD->SetLabelSize(0.06);
 fAvMultIntFlowFQD->SetMarkerStyle(25);
 fAvMultIntFlowFQD->SetLabelOffset(0.01);
 (fAvMultIntFlowFQD->GetXaxis())->SetBinLabel(1,"Average Multiplicity");
 fHistList->Add(fAvMultIntFlowFQD);
 
 //final result for integrated flow 
 fIntFlowResultsFQD = new TH1D("fIntFlowResultsFQD","Integrated Flow By Fitting q-distribution",1,0,1);
 fIntFlowResultsFQD->SetXTitle("");
 fIntFlowResultsFQD->SetYTitle("");
 fIntFlowResultsFQD->SetMarkerStyle(25);
 fIntFlowResultsFQD->SetLabelSize(0.06);
 fAvMultIntFlowFQD->SetLabelOffset(0.02);
 (fIntFlowResultsFQD->GetXaxis())->SetBinLabel(1,"v_{n}");
 fHistList->Add(fIntFlowResultsFQD);
 
 //q-distribution 
 fQDistributionFQD = new TH1D("fQDistributionFQD","q-distribution",100,0,10);
 fQDistributionFQD->SetXTitle("q_{n}=Q_{n}/#sqrt{M}");
 fQDistributionFQD->SetYTitle("Counts");
 fHistList->Add(fQDistributionFQD);
  
 //common control histograms
 fCommonHists = new AliFlowCommonHist("FittingQDistribution");
 fHistList->Add(fCommonHists->GetHistList());  
 
}//end of CreateOutputObjects()

//================================================================================================================

void AliFittingQDistribution::Make(AliFlowEventSimple* anEvent)
{
 
 //Int_t nPrim = anEvent->NumberOfTracks();//total multiplicity
  
 Int_t n=2;//harmonic (to be improved)  
   
 //fill the common control histograms
 fCommonHists->FillControlHistograms(anEvent);   

 //calculating Q-vector of event
 AliFlowVector fQVector;
 fQVector.Set(0.,0.);
 fQVector.SetMult(0);
 fQVector=anEvent->GetQ(n);                                                                                  
                                                                                                                                                                      
 //multiplicity
 fAvMultIntFlowFQD->Fill(0.,fQVector.GetMult(),1.);
 
 //q = Q/sqrt(M)
 Double_t q=0.;
 
 if(fQVector.GetMult()!=0)
 {
  q = fQVector.Mod()/sqrt(fQVector.GetMult());
  fQDistributionFQD->Fill(q,1.);
 }  
}

//================================================================================================================

void AliFittingQDistribution::Finish()
{
 //not needed for the time being...
}


















