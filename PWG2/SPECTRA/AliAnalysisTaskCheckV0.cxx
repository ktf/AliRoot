/**************************************************************************
 * Author: Boris Hippolyte.                                               *
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
//                 AliAnalysisTaskESDCheckV0 class
//            This task is for QAing the V0s from the ESD
//              Origin: B.H. Nov2007, hippolyt@in2p3.fr
//-----------------------------------------------------------------
#include "TChain.h"
#include "TTree.h"
#include "TList.h"
#include "TH1F.h"
#include "TCanvas.h"

#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"

#include "AliESDEvent.h"
#include "AliESDInputHandler.h"

#include "AliESDv0.h"

#include "AliAnalysisTaskESDCheckV0.h"

ClassImp(AliAnalysisTaskESDCheckV0)

//________________________________________________________________________
AliAnalysisTaskESDCheckV0::AliAnalysisTaskESDCheckV0(const char *name) 
  : AliAnalysisTask(name, ""), fESD(0), fListHist(), 
    fHistTrackMultiplicity(0), fHistV0Multiplicity(0), fHistV0OnFlyStatus(0),
    fHistV0MultiplicityOff(0), fHistV0Chi2Off(0),
    fHistDcaV0DaughtersOff(0), fHistV0CosineOfPointingAngleOff(0),
    fHistV0RadiusOff(0),fHistDcaV0ToPrimVertexOff(0),
    fHistDcaPosToPrimVertexOff(0),fHistDcaNegToPrimVertexOff(0),
    fHistMassK0Off(0),fHistMassLambdaOff(0),fHistMassAntiLambdaOff(0),
    fHistV0MultiplicityOn(0), fHistV0Chi2On(0),
    fHistDcaV0DaughtersOn(0), fHistV0CosineOfPointingAngleOn(0),
    fHistV0RadiusOn(0),fHistDcaV0ToPrimVertexOn(0),
    fHistDcaPosToPrimVertexOn(0),fHistDcaNegToPrimVertexOn(0),
    fHistMassK0On(0),fHistMassLambdaOn(0),fHistMassAntiLambdaOn(0)
{
  // Constructor

  // Define input and output slots here
  // Input slot #0 works with a TChain
  DefineInput(0, TChain::Class());
  // Output slot #0 writes into a TH1 container
  DefineOutput(0, TH1F::Class());
  // Output slot #1 writes into a TList container
  DefineOutput(1, TList::Class());
}

//________________________________________________________________________
void AliAnalysisTaskESDCheckV0::ConnectInputData(Option_t *) 
{
  // Connect ESD or AOD here
  // Called once

  TTree* tree = dynamic_cast<TTree*> (GetInputData(0));
  if (!tree) {
    Printf("ERROR: Could not read chain from input slot 0");
  } else {
    // Disable all branches and enable only the needed ones
    // The next two lines are different when data produced as AliESDEvent is read
    tree->SetBranchStatus("*", kFALSE);
    tree->SetBranchStatus("fTracks.*", kTRUE);
    tree->SetBranchStatus("fV0s.*", kTRUE);

    AliESDInputHandler *esdH = dynamic_cast<AliESDInputHandler*> (AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler());

    if (!esdH) {
      Printf("ERROR: Could not get ESDInputHandler");
    } else
      fESD = esdH->GetEvent();
  }
}

//________________________________________________________________________
void AliAnalysisTaskESDCheckV0::CreateOutputObjects()
{
  // Create histograms
  // Called once

  // Distinguish Track and V0 Multiplicity !

  fListHist = new TList();

  if (!fHistTrackMultiplicity) {
    fHistTrackMultiplicity = new TH1F("fHistTrackMultiplicity", "Multiplicity distribution;Number of tracks;Events", 250, 0, 250);
    fListHist->Add(fHistTrackMultiplicity);
  }
  if (!fHistV0Multiplicity) {
    fHistV0Multiplicity = new TH1F("fHistV0Multiplicity", "Multiplicity distribution;Number of V0s;Events", 50, 0, 50);
    fListHist->Add(fHistV0Multiplicity);
  }
  if (!fHistV0OnFlyStatus) {
    fHistV0OnFlyStatus = new TH1F("fHistV0OnFlyStatus", "V0 On fly status;status;Number of V0s", 3, 0, 3);
    fListHist->Add(fHistV0OnFlyStatus);
  }

  // V0 offline distributions
  if (!fHistV0MultiplicityOff) {
    fHistV0MultiplicityOff = new TH1F("fHistV0MultiplicityOff", "Multiplicity distribution;Number of V0s;Events", 50, 0, 50);
    fListHist->Add(fHistV0MultiplicityOff);
  }
  if (!fHistV0Chi2Off) {
    fHistV0Chi2Off = new TH1F("fHistV0Chi2Off", "V0 chi2;chi2;Number of V0s", 33, 0, 33);
    fListHist->Add(fHistV0Chi2Off);
  }
  if (!fHistDcaV0DaughtersOff) {
    fHistDcaV0DaughtersOff = new TH1F("fHistDcaV0DaughtersOff", "DCA between V0 daughters;DCA (cm);Number of V0s", 300, 0, 3);
    fListHist->Add(fHistDcaV0DaughtersOff);
  }
  if (!fHistV0CosineOfPointingAngleOff) {
    fHistV0CosineOfPointingAngleOff = new TH1F("fHistV0CosineOfPointingAngleOff", "V0 Cosine of Pointing Angle;Number of V0s", 200, 0, 1);
    fListHist->Add(fHistV0CosineOfPointingAngleOff);
  }
  if (!fHistV0RadiusOff) {
    fHistV0RadiusOff = new TH1F("fHistV0RadiusOff", "V0 decay radius;Radius (cm);Number of V0s", 33, 0, 33);
    fListHist->Add(fHistV0RadiusOff);
  }
  if (!fHistDcaV0ToPrimVertexOff) {
    fHistDcaV0ToPrimVertexOff = new TH1F("fHistDcaV0ToPrimVertexOff", "DCA of V0 to Prim. Vertex;DCA (cm);Number of V0s", 300, 0, 3);
    fListHist->Add(fHistDcaV0ToPrimVertexOff);
  }
  if (!fHistDcaPosToPrimVertexOff) {
    fHistDcaPosToPrimVertexOff = new TH1F("fHistDcaPosToPrimVertexOff", "DCA of V0 neg daughter to Prim. Vertex;DCA (cm);Number of V0s", 300, 0, 3);
    fListHist->Add(fHistDcaPosToPrimVertexOff);
  }
  if (!fHistDcaNegToPrimVertexOff) {
    fHistDcaNegToPrimVertexOff = new TH1F("fHistDcaNegToPrimVertexOff", "DCA of V0 pos daughter to Prim. Vertex;DCA (cm);Number of V0s", 300, 0, 3);
    fListHist->Add(fHistDcaNegToPrimVertexOff);
  }

  if (!fHistMassK0Off) {
    fHistMassK0Off = new TH1F("fMassK0Off","K^{0} candidates;M(#pi^{+}#pi^{-}) (GeV/c^{2});Counts",100,0.4,0.6);
    fListHist->Add(fHistMassK0Off);
  }
  if (!fHistMassLambdaOff) {
    fHistMassLambdaOff = new TH1F("fMassLambdaOff","#Lambda^{0} candidates;M(p#pi^{-}) (GeV/c^{2});Counts",75,1.05,1.2);
    fListHist->Add(fHistMassLambdaOff);
  }
  if (!fHistMassAntiLambdaOff) {
    fHistMassAntiLambdaOff = new TH1F("fMassAntiLambdaOff","#bar{#Lambda}^{0} candidates;M(#bar{p}#pi^{+}) (GeV/c^{2});Counts",75,1.05,1.2);
    fListHist->Add(fHistMassAntiLambdaOff);
  }

  // V0 on-the-fly distributions
  if (!fHistV0MultiplicityOn) {
    fHistV0MultiplicityOn = new TH1F("fHistV0MultiplicityOn", "Multiplicity distribution;Number of V0s;Events", 50, 0, 50);
    fListHist->Add(fHistV0MultiplicityOn);
  }
  if (!fHistV0Chi2On) {
    fHistV0Chi2On = new TH1F("fHistV0Chi2On", "V0 chi2;chi2;Number of V0s", 33, 0, 33);
    fListHist->Add(fHistV0Chi2On);
  }
  if (!fHistDcaV0DaughtersOn) {
    fHistDcaV0DaughtersOn = new TH1F("fHistDcaV0DaughtersOn", "DCA between V0 daughters;DCA (cm);Number of V0s", 300, 0, 3);
    fListHist->Add(fHistDcaV0DaughtersOn);
  }
  if (!fHistV0CosineOfPointingAngleOn) {
    fHistV0CosineOfPointingAngleOn = new TH1F("fHistV0CosineOfPointingAngleOn", "V0 Cosine of Pointing Angle;Number of V0s", 200, 0, 1);
    fListHist->Add(fHistV0CosineOfPointingAngleOn);
  }
  if (!fHistV0RadiusOn) {
    fHistV0RadiusOn = new TH1F("fHistV0RadiusOn", "V0 decay radius;Radius (cm);Number of V0s", 33, 0, 33);
    fListHist->Add(fHistV0RadiusOn);
  }
  if (!fHistDcaV0ToPrimVertexOn) {
    fHistDcaV0ToPrimVertexOn = new TH1F("fHistDcaV0ToPrimVertexOn", "DCA of V0 to Prim. Vertex;DCA (cm);Number of V0s", 300, 0, 3);
    fListHist->Add(fHistDcaV0ToPrimVertexOn);
  }
  if (!fHistDcaPosToPrimVertexOn) {
    fHistDcaPosToPrimVertexOn = new TH1F("fHistDcaPosToPrimVertexOn", "DCA of V0 neg daughter to Prim. Vertex;DCA (cm);Number of V0s", 300, 0, 3);
    fListHist->Add(fHistDcaPosToPrimVertexOn);
  }
  if (!fHistDcaNegToPrimVertexOn) {
    fHistDcaNegToPrimVertexOn = new TH1F("fHistDcaNegToPrimVertexOn", "DCA of V0 pos daughter to Prim. Vertex;DCA (cm);Number of V0s", 300, 0, 3);
    fListHist->Add(fHistDcaNegToPrimVertexOn);
  }

  if (!fHistMassK0On) {
    fHistMassK0On = new TH1F("fMassK0On","K^{0} candidates;M(#pi^{+}#pi^{-}) (GeV/c^{2});Counts",100,0.4,0.6);
    fListHist->Add(fHistMassK0On);
  }
  if (!fHistMassLambdaOn) {
    fHistMassLambdaOn = new TH1F("fMassLambdaOn","#Lambda^{0} candidates;M(p#pi^{-}) (GeV/c^{2});Counts",75,1.05,1.2);
    fListHist->Add(fHistMassLambdaOn);
  }
  if (!fHistMassAntiLambdaOn) {
    fHistMassAntiLambdaOn = new TH1F("fMassAntiLambdaOn","#bar{#Lambda}^{0} candidates;M(#bar{p}#pi^{+}) (GeV/c^{2});Counts",75,1.05,1.2);
    fListHist->Add(fHistMassAntiLambdaOn);
  }

}

//________________________________________________________________________
void AliAnalysisTaskESDCheckV0::Exec(Option_t *) 
{
  // Main loop
  // Called for each event

  if (!fESD) {
    Printf("ERROR: fESD not available");
    return;
  }

  //  Printf("There are %d tracks in this event", fESD->GetNumberOfTracks());
  fHistTrackMultiplicity->Fill(fESD->GetNumberOfTracks());

  Int_t nv0s = 0;
  nv0s = fESD->GetNumberOfV0s();

  Int_t    lOnFlyStatus = 0, nv0sOn = 0, nv0sOff = 0;
  Double_t lChi2V0 = 0;
  Double_t lDcaV0Daughters = 0, lDcaV0ToPrimVertex = 0;
  Double_t lDcaPosToPrimVertex = 0, lDcaNegToPrimVertex = 0;
  Double_t lV0CosineOfPointingAngle = 0;
  Double_t lV0Radius = 0;
  Double_t lInvMassK0 = 0, lInvMassLambda = 0, lInvMassAntiLambda = 0;

  for (Int_t iV0 = 0; iV0 < nv0s; iV0++) 
    {// This is the begining of the V0 loop
      AliESDv0 *v0 = fESD->GetV0(iV0);
      if (!v0) continue;

      Double_t tDecayVertexV0[3]; v0->GetXYZ(tDecayVertexV0[0],tDecayVertexV0[1],tDecayVertexV0[2]); 

      lV0Radius = TMath::Sqrt(tDecayVertexV0[0]*tDecayVertexV0[0]+tDecayVertexV0[1]*tDecayVertexV0[1]);

      UInt_t lKeyPos = (UInt_t)TMath::Abs(v0->GetPindex());
      UInt_t lKeyNeg = (UInt_t)TMath::Abs(v0->GetNindex());

      Double_t lMomPos[3]; v0->GetPPxPyPz(lMomPos[0],lMomPos[1],lMomPos[2]);
      Double_t lMomNeg[3]; v0->GetNPxPyPz(lMomNeg[0],lMomNeg[1],lMomNeg[2]);

      AliESDtrack *pTrack=fESD->GetTrack(lKeyPos);
      AliESDtrack *nTrack=fESD->GetTrack(lKeyNeg);
      if (!pTrack || !nTrack) {
	Printf("ERROR: Could not retreive one of the daughter track");
	continue;
      }

      Float_t tDcaPosToPrimVertex[2];
      if(pTrack) pTrack->GetImpactParameters(tDcaPosToPrimVertex[0],tDcaPosToPrimVertex[1]);
      else { tDcaPosToPrimVertex[0]=999.;  tDcaPosToPrimVertex[1]=999.;}
      lDcaPosToPrimVertex = TMath::Sqrt(tDcaPosToPrimVertex[0]*tDcaPosToPrimVertex[0]+tDcaPosToPrimVertex[1]*tDcaPosToPrimVertex[1]);

      Float_t tDcaNegToPrimVertex[2];
      if(nTrack) nTrack->GetImpactParameters(tDcaNegToPrimVertex[0],tDcaNegToPrimVertex[1]);
      else { tDcaNegToPrimVertex[0]=999.;  tDcaNegToPrimVertex[1]=999.;}
      lDcaNegToPrimVertex = TMath::Sqrt(tDcaNegToPrimVertex[0]*tDcaNegToPrimVertex[0]+tDcaNegToPrimVertex[1]*tDcaNegToPrimVertex[1]);


      lOnFlyStatus = v0->GetOnFlyStatus();
      lChi2V0 = v0->GetChi2V0();
      lDcaV0Daughters = v0->GetDcaV0Daughters();
      lDcaV0ToPrimVertex = v0->GetD();
      lV0CosineOfPointingAngle = v0->GetV0CosineOfPointingAngle();

      // Getting invariant mass infos directly from ESD
      v0->ChangeMassHypothesis(310);
      lInvMassK0 = v0->GetEffMass();
      v0->ChangeMassHypothesis(3122);
      lInvMassLambda = v0->GetEffMass();
      v0->ChangeMassHypothesis(-3122);
      lInvMassAntiLambda = v0->GetEffMass();

      fHistV0OnFlyStatus->Fill(lOnFlyStatus);
      if(!lOnFlyStatus){
	nv0sOff++;
	fHistV0Chi2Off->Fill(lChi2V0);
	fHistDcaV0ToPrimVertexOff->Fill(lDcaV0ToPrimVertex);
	fHistDcaV0DaughtersOff->Fill(lDcaV0Daughters);
	fHistV0CosineOfPointingAngleOff->Fill(lV0CosineOfPointingAngle);

	fHistV0RadiusOff->Fill(lV0Radius);
	fHistDcaPosToPrimVertexOff->Fill(lDcaPosToPrimVertex);
	fHistDcaNegToPrimVertexOff->Fill(lDcaNegToPrimVertex);

	// Filling invariant mass histos for all candidates
	fHistMassK0Off->Fill(lInvMassK0);
	fHistMassLambdaOff->Fill(lInvMassLambda);
	fHistMassAntiLambdaOff->Fill(lInvMassAntiLambda);
      }
      else {
	nv0sOn++;
	fHistV0Chi2On->Fill(lChi2V0);
	fHistDcaV0ToPrimVertexOn->Fill(lDcaV0ToPrimVertex);
	fHistDcaV0DaughtersOn->Fill(lDcaV0Daughters);
	fHistV0CosineOfPointingAngleOn->Fill(lV0CosineOfPointingAngle);

	fHistV0RadiusOn->Fill(lV0Radius);
	fHistDcaPosToPrimVertexOn->Fill(lDcaPosToPrimVertex);
	fHistDcaNegToPrimVertexOn->Fill(lDcaNegToPrimVertex);

	// Filling invariant mass histos for all candidates
	fHistMassK0On->Fill(lInvMassK0);
	fHistMassLambdaOn->Fill(lInvMassLambda);
	fHistMassAntiLambdaOn->Fill(lInvMassAntiLambda);
      }
    }// This is the end of the V0 loop

  fHistV0Multiplicity->Fill(fESD->GetNumberOfV0s());
  fHistV0MultiplicityOff->Fill(nv0sOff);
  fHistV0MultiplicityOn->Fill(nv0sOn);

  // Post output data.
  PostData(0, fHistTrackMultiplicity);
  PostData(1, fListHist);
}      

//________________________________________________________________________
void AliAnalysisTaskESDCheckV0::Terminate(Option_t *) 
{
  // Draw result to the screen
  // Called once at the end of the query

  fHistTrackMultiplicity = dynamic_cast<TH1F*> (GetOutputData(0));
  if (!fHistTrackMultiplicity) {
    Printf("ERROR: fHistTrackMultiplicity not available");
    return;
  }
  fHistV0Multiplicity = dynamic_cast<TH1F*> (((TList*)GetOutputData(1))->FindObject("fHistV0Multiplicity"));
  if (!fHistV0Multiplicity) {
    Printf("ERROR: fHistV0Multiplicity not available");
    return;
  }
  fHistV0MultiplicityOff = dynamic_cast<TH1F*> (((TList*)GetOutputData(1))->FindObject("fHistV0MultiplicityOff"));
  if (!fHistV0MultiplicityOff) {
    Printf("ERROR: fHistV0MultiplicityOff not available");
    return;
  }
  fHistV0MultiplicityOn = dynamic_cast<TH1F*> (((TList*)GetOutputData(1))->FindObject("fHistV0MultiplicityOn"));
  if (!fHistV0MultiplicityOn) {
    Printf("ERROR: fHistV0MultiplicityOn not available");
    return;
  }
   
  TCanvas *c2 = new TCanvas("AliAnalysisTaskESDCheckV0","Ptot",10,10,510,510);
  c2->cd(1)->SetLogy();

  fHistTrackMultiplicity->SetMarkerStyle(22);
  fHistTrackMultiplicity->DrawCopy("E");
  fHistV0Multiplicity->SetMarkerStyle(26);
  fHistV0Multiplicity->DrawCopy("ESAME");
  fHistV0MultiplicityOff->SetMarkerStyle(24);
  fHistV0MultiplicityOff->DrawCopy("ESAME");
  fHistV0MultiplicityOn->SetMarkerStyle(20);
  fHistV0MultiplicityOn->DrawCopy("ESAME");
}
