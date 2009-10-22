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

//-----------------------------------------------------------------------------
// Analysis task to compute muon/dimuon kinematic distributions
// The output is a list of histograms.
// The macro class can run on AOD or in the train with the ESD filter.
// R. Arnaldi
//
//-----------------------------------------------------------------------------

//#ifndef ALIANALYSISTASKMUONDISTRIBUTIONS_CXX
//#define ALIANALYSISTASKMUONDISTRIBUTIONS_CXX

#include <TList.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TStyle.h>
#include <TLatex.h>

#include "AliAnalysisTaskMuonDistributions.h"
#include "AliAnalysisTaskSE.h"
#include "AliAnalysisManager.h"
#include "AliESDEvent.h"
#include "AliESD.h"
#include "AliVEvent.h"
#include "AliMCEventHandler.h"
#include "AliInputEventHandler.h"
#include "AliMCEvent.h"
#include "AliStack.h"
#include "AliLog.h"
#include "AliHeader.h"
#include "AliESDHeader.h"
#include "AliStack.h"
#include "TParticle.h"
#include "TLorentzVector.h"
#include "AliESDMuonTrack.h"
#include "AliESDInputHandler.h"
#include "AliAODEvent.h"
#include "AliAODHeader.h"
#include "AliAODHandler.h"
#include "AliAODInputHandler.h"

ClassImp(AliAnalysisTaskMuonDistributions)

//__________________________________________________________________________
AliAnalysisTaskMuonDistributions::AliAnalysisTaskMuonDistributions() :
  fBeamEnergy(0.),
  fInvMassFitLimitMin(2.),
  fInvMassFitLimitMax(5.),
  fPsiFitLimitMin(2.9),
  fPsiFitLimitMax(3.3),
  fBckFitLimitMin(2.2),
  fBckFitLimitMax(2.85),
  fInvariantMassFit(kFALSE),
  fkAnalysisType(0x0),
  fOutput(0x0)
{
}
//___________________________________________________________________________
AliAnalysisTaskMuonDistributions::AliAnalysisTaskMuonDistributions(const Char_t* name) :
  AliAnalysisTaskSE(name),
  fBeamEnergy(0.),
  fInvMassFitLimitMin(2.),
  fInvMassFitLimitMax(5.),
  fPsiFitLimitMin(2.9),
  fPsiFitLimitMax(3.3),
  fBckFitLimitMin(2.2),
  fBckFitLimitMax(2.85),
  fInvariantMassFit(kFALSE),
  fkAnalysisType(0x0),
  fOutput(0x0)
{
  //
  // Constructor. Initialization of Inputs and Outputs
  //
  Info("AliAnalysisTaskMuonDistributions","Calling Constructor");
  
  DefineOutput(1,TList::Class());

}

//___________________________________________________________________________
AliAnalysisTaskMuonDistributions& AliAnalysisTaskMuonDistributions::operator=(const AliAnalysisTaskMuonDistributions& c) 
{
  //
  // Assignment operator
  //
  if (this!=&c) {
    AliAnalysisTaskSE::operator=(c) ;
  }
  return *this;
}

//___________________________________________________________________________
AliAnalysisTaskMuonDistributions::AliAnalysisTaskMuonDistributions(const AliAnalysisTaskMuonDistributions& c) :
  AliAnalysisTaskSE(c),
  fBeamEnergy(c.fBeamEnergy),
  fInvMassFitLimitMin(c.fInvMassFitLimitMin),
  fInvMassFitLimitMax(c.fInvMassFitLimitMax),
  fPsiFitLimitMin(c.fPsiFitLimitMin),
  fPsiFitLimitMax(c.fPsiFitLimitMax),
  fBckFitLimitMin(c.fBckFitLimitMin),
  fBckFitLimitMax(c.fBckFitLimitMax),
  fInvariantMassFit(c.fInvariantMassFit),
  fkAnalysisType(c.fkAnalysisType),
  fOutput(c.fOutput)
 {
  //
  // Copy Constructor										
  //
}

//___________________________________________________________________________
AliAnalysisTaskMuonDistributions::~AliAnalysisTaskMuonDistributions() {
  //
  //destructor
  //
  Info("~AliAnalysisTaskMuonDistributions","Calling Destructor");
}

//___________________________________________________________________________
void AliAnalysisTaskMuonDistributions::UserCreateOutputObjects(){
 //
 // output objects creation
 //	 
 fOutput = new TList();
 fOutput->SetOwner(); 
 //
 // various histos
 //
 TH1D *hNumberMuonTracks = new TH1D("hNumberMuonTracks","hNumberMuonTracks;N_{#mu tracks}",10,0.,10.);
 //
 // dimuon histos
 //
 TH1D *hMassDimu   = new TH1D("hMassDimu","hMassDimu;M_{#mu#mu} (GeV/c^{2})",180,0,9.);	
 TH1D *hPtDimu  = new TH1D("hPtDimu","hPtDimu;p_{T} (GeV/c)",100,0,20);	
 TH1D *hRapidityDimu  = new TH1D("hRapidityDimu","hRapidityDimu;y",100,-5,-2);	
 TH1D *hCosThetaCSDimu  = new TH1D("hCosThetaCSDimu","hCosThetaCSDimu;cos#theta_{CS}",100,-1.,1.);	
 TH1D *hCosThetaHEDimu  = new TH1D("hCosThetaHEDimu","hCosThetaHEDimu;cos#theta_{HE}",100,-1.,1.);	
 //
 // muon histos
 //
 TH1D *hP  = new TH1D("hP","hP;p (GeV/c)",100,0,500);	
 TH1D *hPt  = new TH1D("hPt","hPt;p_{T} (GeV/c)",100,0,20);	
 TH1D *hRapidity  = new TH1D("hRapidity","hRapidity;y",100,-5,-2);	
	
 fOutput->Add(hNumberMuonTracks); 	
 fOutput->Add(hMassDimu); 	
 fOutput->Add(hPtDimu); 	
 fOutput->Add(hRapidityDimu); 	
 fOutput->Add(hCosThetaCSDimu); 	
 fOutput->Add(hCosThetaHEDimu); 	
 fOutput->Add(hP); 	
 fOutput->Add(hPt); 	
 fOutput->Add(hRapidity); 	
 fOutput->ls(); 
} 

//_________________________________________________
void AliAnalysisTaskMuonDistributions::UserExec(Option_t *)
{
//
// Execute analysis for current event
//
  AliESDEvent *esd=0x0;
  AliAODEvent *aod=0x0;
  
  if(strcmp(fkAnalysisType,"ESD")==0){
    AliESDInputHandler *esdH = dynamic_cast<AliESDInputHandler*>
        (AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler());
    esd = esdH->GetEvent();
  } else if(strcmp(fkAnalysisType,"AOD")==0){
    aod = dynamic_cast<AliAODEvent*> (InputEvent());
  }
  
  Int_t ntracks=-999;
  if(strcmp(fkAnalysisType,"ESD")==0) ntracks=esd->GetNumberOfMuonTracks();
  else if(strcmp(fkAnalysisType,"AOD")==0) ntracks=aod->GetNumberOfTracks();
  Int_t nmuontracks=0;
  
  for (Int_t j = 0; j<ntracks; j++) {
    Float_t pxmu1=-999; Float_t pymu1=-999; Float_t pzmu1=-999; Float_t ptmu1=-999; Float_t pmu1=-999;
    Float_t emu1=-999; Float_t rapiditymu1=-999;  Float_t chargemu1=-999; 
    if(strcmp(fkAnalysisType,"ESD")==0){ 
      AliESDMuonTrack* mu1 = new AliESDMuonTrack(*(esd->GetMuonTrack(j)));
      if (!mu1->ContainTrackerData()) continue;
      chargemu1 = mu1->Charge();
      pxmu1 = mu1->Px();
      pymu1 = mu1->Py();
      pzmu1 = mu1->Pz();
      emu1 = mu1->E();
      pmu1 = mu1->P();
      ptmu1 = mu1->Pt();
      rapiditymu1 = Rapidity(emu1,pzmu1);
    } else if(strcmp(fkAnalysisType,"AOD")==0){
      AliAODTrack *mu1 = aod->GetTrack(j);
      if(!mu1->IsMuonTrack()) continue;
      chargemu1 = mu1->Charge();
      pxmu1 = mu1->Px();
      pymu1 = mu1->Py();
      pzmu1 = mu1->Pz();
      emu1 = mu1->E();
      pmu1 = mu1->P();
      ptmu1 = mu1->Pt();
      rapiditymu1 = Rapidity(emu1,pzmu1);
    }
    ((TH1D*)(fOutput->FindObject("hP")))->Fill(pmu1);    
    ((TH1D*)(fOutput->FindObject("hPt")))->Fill(ptmu1);    
    ((TH1D*)(fOutput->FindObject("hRapidity")))->Fill(rapiditymu1);	  
    nmuontracks++;
    if(chargemu1<0){
      for (Int_t jj = 0; jj<ntracks; jj++) {
        Float_t pxmu2=-999; Float_t pymu2=-999; Float_t pzmu2=-999;
        Float_t emu2=-999;Float_t chargemu2=-999; 
        if(strcmp(fkAnalysisType,"ESD")==0){ 
          AliESDMuonTrack* mu2 = new AliESDMuonTrack(*(esd->GetMuonTrack(jj)));
          if (!mu2->ContainTrackerData()) continue;
	  chargemu2 = mu2->Charge();
          pxmu2 = mu2->Px();
          pymu2 = mu2->Py();
          pzmu2 = mu2->Pz();
	  emu2 = mu2->E();
        } else if(strcmp(fkAnalysisType,"AOD")==0){
          AliAODTrack *mu2 = aod->GetTrack(jj);
          if(!mu2->IsMuonTrack()) continue; 
	  chargemu2 = mu2->Charge();
          pxmu2 = mu2->Px();
          pymu2 = mu2->Py();
          pzmu2 = mu2->Pz();
	  emu2 = mu2->E();
        }
        if(chargemu2>0){
	  Float_t ptdimu = TMath::Sqrt((pxmu1+pxmu2)*(pxmu1+pxmu2)+(pymu1+pymu2)*(pymu1+pymu2));
 	  Float_t massdimu = InvMass(emu1,pxmu1,pymu1,pzmu1,emu2,pxmu2,pymu2,pzmu2);
 	  Float_t rapiditydimu = Rapidity((emu1+emu2),(pzmu1+pzmu2));
	  Double_t costhetaCSdimu = CostCS((Double_t) pxmu1,(Double_t) pymu1,(Double_t)pzmu1,(Double_t) emu1,(Double_t)chargemu1,(Double_t) pxmu2,(Double_t) pymu2,(Double_t)pzmu2,(Double_t) emu2);
	  Double_t costhetaHEdimu = CostHE((Double_t) pxmu1,(Double_t) pymu1,(Double_t)pzmu1,(Double_t) emu1,(Double_t)chargemu1,(Double_t) pxmu2,(Double_t) pymu2,(Double_t)pzmu2,(Double_t) emu2);
	  ((TH1D*)(fOutput->FindObject("hMassDimu")))->Fill(massdimu);
	  ((TH1D*)(fOutput->FindObject("hPtDimu")))->Fill(ptdimu);	
	  ((TH1D*)(fOutput->FindObject("hRapidityDimu")))->Fill(rapiditydimu);	
	  ((TH1D*)(fOutput->FindObject("hCosThetaCSDimu")))->Fill(costhetaCSdimu);	
	  ((TH1D*)(fOutput->FindObject("hCosThetaHEDimu")))->Fill(costhetaHEdimu);	
        }
        //delete mu2;
      }      // second mu Loop
    }          // mu- Selection
    //delete mu1;
  }        
  ((TH1D*)(fOutput->FindObject("hNumberMuonTracks")))->Fill(nmuontracks); 
  
  PostData(1,fOutput);
  }


//________________________________________________________________________
void AliAnalysisTaskMuonDistributions::Terminate(Option_t *) 
{
//
// Draw histos
//
  gStyle->SetCanvasColor(10);
  gStyle->SetFrameFillColor(10);
  Int_t xmin=20; 
  Int_t ymin=20;
  
  printf("Using beam Energy=%f \n",fBeamEnergy);

  TH1D *hNumberMuonTracks = dynamic_cast<TH1D*> (fOutput->FindObject("hNumberMuonTracks"));  
  TH1D *hMassDimu = dynamic_cast<TH1D*> (fOutput->FindObject("hMassDimu"));  
  TH1D *hPtDimu = dynamic_cast<TH1D*> (fOutput->FindObject("hPtDimu"));  
  TH1D *hRapidityDimu = dynamic_cast<TH1D*> (fOutput->FindObject("hRapidityDimu"));  
  TH1D *hCostCSDimu = dynamic_cast<TH1D*> (fOutput->FindObject("hCosThetaCSDimu"));  
  TH1D *hCostHEDimu = dynamic_cast<TH1D*> (fOutput->FindObject("hCosThetaHEDimu"));  
  TH1D *hP = dynamic_cast<TH1D*> (fOutput->FindObject("hP"));  
  TH1D *hPt = dynamic_cast<TH1D*> (fOutput->FindObject("hPt"));  
  TH1D *hRapidity = dynamic_cast<TH1D*> (fOutput->FindObject("hRapidity"));  

  TCanvas *c0 = new TCanvas("c0","Plots",xmin,ymin,600,600);
  c0->Divide(2,2);
  c0->cd(1);
  hNumberMuonTracks->Draw();
  
  xmin+=20; ymin+=20;
  TCanvas *c1 = new TCanvas("c1","Muon kinematic distributions Plots",xmin,ymin,600,600);
  c1->Divide(2,2);  
  c1->cd(1);
  gPad->SetLogy(1);
  hP->Draw();
  c1->cd(2);
  gPad->SetLogy(1);
  hPt->Draw();
  c1->cd(3);
  hRapidity->Draw();
  
  xmin+=20; ymin+=20;
  TCanvas *c2 = new TCanvas("c2","Dimuon kinematic distributions Plots",xmin,ymin,600,600);
  c2->Divide(2,2);  
  c2->cd(1);
  gPad->SetLogy(1);
  hPtDimu->Draw();
  c2->cd(2);
  hRapidityDimu->Draw();
  c2->cd(3);
  hCostCSDimu->Draw();
  c2->cd(4);
  hCostHEDimu->Draw();
  
  xmin+=20; ymin+=20;
  TCanvas *c3 = new TCanvas("c3","Invariant Mass Plots",xmin,ymin,600,600);
  gPad->SetLogy(1);
  hMassDimu->Draw();  
  if(fInvariantMassFit) FitInvMass(hMassDimu);
  c3->Update();

}

//________________________________________________________________________
Float_t AliAnalysisTaskMuonDistributions::InvMass(Float_t e1, Float_t px1, Float_t py1, Float_t pz1,
				   Float_t e2, Float_t px2, Float_t py2, Float_t pz2) const 
{
//
// invariant mass calculation
//
    Float_t imassrec = TMath::Sqrt((e1+e2)*(e1+e2)-((px1+px2)*(px1+px2)+
                                    (py1+py2)*(py1+py2)+(pz1+pz2)*(pz1+pz2)));
    return imassrec;
}
//________________________________________________________________________
Float_t AliAnalysisTaskMuonDistributions::Rapidity(Float_t e, Float_t pz) const 
{
//
// calculate rapidity
//
    Float_t rap;
    if(TMath::Abs(e-pz)>1e-7){
      rap = 0.5*TMath::Log((e+pz)/(e-pz));
      return rap;
    } else {
      rap = -200;
      return rap;
    }
}
//________________________________________________________________________
Double_t AliAnalysisTaskMuonDistributions::CostCS(Double_t px1, Double_t py1, Double_t pz1, Double_t e1,
Double_t charge1, Double_t px2, Double_t py2, Double_t pz2, Double_t e2)
{
//TMath
// costCS calculation
//
  TLorentzVector pMu1CM, pMu2CM, pProjCM, pTargCM, pDimuCM; // In the CM. frame
  TLorentzVector pMu1Dimu, pMu2Dimu, pProjDimu, pTargDimu; // In the dimuon rest frame
  TVector3 beta,zaxisCS;
  Double_t mp=0.93827231;
  //
  // --- Fill the Lorentz vector for projectile and target in the CM frame
  //
  pProjCM.SetPxPyPzE(0.,0.,-fBeamEnergy,TMath::Sqrt(fBeamEnergy*fBeamEnergy+mp*mp)); 
  pTargCM.SetPxPyPzE(0.,0.,fBeamEnergy,TMath::Sqrt(fBeamEnergy*fBeamEnergy+mp*mp)); 
  //
  // --- Get the muons parameters in the CM frame 
  //
  pMu1CM.SetPxPyPzE(px1,py1,pz1,e1);
  pMu2CM.SetPxPyPzE(px2,py2,pz2,e2);
  //
  // --- Obtain the dimuon parameters in the CM frame
  //
  pDimuCM=pMu1CM+pMu2CM;
  //
  // --- Translate the dimuon parameters in the dimuon rest frame
  //
  beta=(-1./pDimuCM.E())*pDimuCM.Vect();
  pMu1Dimu=pMu1CM;
  pMu2Dimu=pMu2CM;
  pProjDimu=pProjCM;
  pTargDimu=pTargCM;
  pMu1Dimu.Boost(beta);
  pMu2Dimu.Boost(beta);
  pProjDimu.Boost(beta);
  pTargDimu.Boost(beta);
  //
  // --- Determine the z axis for the CS angle 
  //
  zaxisCS=(((pProjDimu.Vect()).Unit())-((pTargDimu.Vect()).Unit())).Unit();
  //				     
  // --- Determine the CS angle (angle between mu+ and the z axis defined above)
  Double_t cost;
  
  if(charge1>0) {cost = zaxisCS.Dot((pMu1Dimu.Vect()).Unit());}
  else {cost = zaxisCS.Dot((pMu2Dimu.Vect()).Unit());}
  return cost;
}

//________________________________________________________________________
Double_t AliAnalysisTaskMuonDistributions::CostHE(Double_t px1, Double_t py1, Double_t pz1, Double_t e1,
Double_t charge1, Double_t px2, Double_t py2, Double_t pz2, Double_t e2)
{
//
// costHE calculation
//
  TLorentzVector pMu1CM, pMu2CM, pDimuCM; // In the CM frame 
  TLorentzVector pMu1Dimu, pMu2Dimu; // In the dimuon rest frame
  TVector3 beta,zaxisCS;
  //
  // --- Get the muons parameters in the CM frame
  //
  pMu1CM.SetPxPyPzE(px1,py1,pz1,e1);
  pMu2CM.SetPxPyPzE(px2,py2,pz2,e2);
  //
  // --- Obtain the dimuon parameters in the CM frame
  //
  pDimuCM=pMu1CM+pMu2CM;
  //
  // --- Translate the muon parameters in the dimuon rest frame
  //
  beta=(-1./pDimuCM.E())*pDimuCM.Vect();
  pMu1Dimu=pMu1CM;
  pMu2Dimu=pMu2CM;
  pMu1Dimu.Boost(beta);
  pMu2Dimu.Boost(beta);
  //
  // --- Determine the z axis for the calculation of the polarization angle (i.e. the direction of the dimuon in the CM system)
  //
  TVector3 zaxis;
  zaxis=(pDimuCM.Vect()).Unit();
  
  // --- Calculation of the polarization angle (angle between mu+ and the z axis defined above)
  Double_t cost;
  if(charge1>0) {cost = zaxis.Dot((pMu1Dimu.Vect()).Unit());} 
  else {cost = zaxis.Dot((pMu2Dimu.Vect()).Unit());} 
  return cost;
}  

//________________________________________________________________________
void AliAnalysisTaskMuonDistributions::FitInvMass(TH1D *histo)
{
//
// Fit to the Invariant Mass Spectrum
//
  TF1 *gau = new TF1("gau","gaus",fPsiFitLimitMin,fPsiFitLimitMax);
  TF1 *ex = new TF1("ex","expo",fBckFitLimitMin,fBckFitLimitMax);    
  TF1 *tot = new TF1("mtot","gaus(0)+expo(3)",fInvMassFitLimitMin,fInvMassFitLimitMax);
  Double_t par[5];
  Double_t binWidth= histo->GetBinWidth(1);
  gau->SetLineColor(3);
  gau->SetLineWidth(2);
  histo->Fit(gau,"RlQ"); 
  ex->SetLineColor(4);
  ex->SetLineWidth(2);
  histo->Fit(ex,"RlQ+");
  gau->GetParameters(&par[0]);
  ex->GetParameters(&par[3]);
  tot->SetParameters(par);   
  tot->SetLineColor(2);
  tot->SetLineWidth(2);
  histo->Fit(tot,"Rl+");
  histo->Draw("e");
  Double_t chi2 = tot->GetChisquare();
  Double_t ndf = tot->GetNDF();
  Float_t meanPsi= tot->GetParameter(1);
  Float_t sigPsi= tot->GetParameter(2)*1000.;
  Double_t nPsiFit = TMath::Sqrt(2*3.1415)*tot->GetParameter(0)*tot->GetParameter(2)/binWidth;
  TF1 *exfix = new TF1("exfix","expo",2.,5.);  
  exfix->SetParameter(0,tot->GetParameter(3));  
  exfix->SetParameter(1,tot->GetParameter(4));  
  Double_t nBck = exfix->Integral(2.9,3.3)/binWidth;
  
  printf("\n\n****************************************************************************\n");
  char psitext[100];
  sprintf(psitext,"N. J/#psi (2.9-3.3)=%10.0f",nPsiFit);
  printf("\nN. J/psi (2.9-3.3)=%10.0f\n",nPsiFit);
  TLatex *psilatex = new TLatex(4.5,0.85*histo->GetMaximum(),psitext);
  psilatex->SetTextColor(2);
  psilatex->SetTextSize(0.03);
  psilatex->SetTextAlign(2);
  psilatex->Draw();
  
  char psi2text[100];
  sprintf(psi2text,"J/#psi m=%4.3f GeV #sigma=%4.2f MeV",meanPsi,sigPsi);
  printf("J/psi m=%4.3f GeV sigma=%4.2f MeV\n",meanPsi,sigPsi);
  TLatex *psi2latex = new TLatex(4.5,0.425*histo->GetMaximum(),psi2text);
  psi2latex->SetTextColor(2);
  psi2latex->SetTextSize(0.03);
  psi2latex->SetTextAlign(2);
  psi2latex->Draw();
  
  char sbtext[100];
  sprintf(sbtext,"S/B (2.9-3.3)=%4.2f ",nPsiFit/nBck);
  printf("S/B (2.9-3.3)=%4.2f\n",nPsiFit/nBck);
  TLatex *sblatex = new TLatex(4.5,0.212*histo->GetMaximum(),sbtext);
  sblatex->SetTextColor(2);
  sblatex->SetTextSize(0.03);
  sblatex->SetTextAlign(2);
  sblatex->Draw();
  
  char chi2text[100];
  sprintf(chi2text,"#chi^2/ndf =%4.2f ",chi2/ndf);
  printf("chi^2/ndf =%4.2f\n",chi2/ndf);
  TLatex *chi2latex = new TLatex(4.5,0.106*histo->GetMaximum(),chi2text);
  chi2latex->SetTextColor(2);
  chi2latex->SetTextSize(0.03);
  chi2latex->SetTextAlign(2);
  chi2latex->Draw();
  printf("\n****************************************************************************\n");
  
}


