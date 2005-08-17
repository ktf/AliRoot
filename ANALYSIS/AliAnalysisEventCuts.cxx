/**************************************************************************
 * Author: Panos Christakoglou.                                           *
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

/* $Id$ */

//-----------------------------------------------------------------
//           AliAnalysisEventCuts class
//   This is the class to deal with the event and track level cuts
//   Origin: Panos Christakoglou, UOA-CERN, Panos.Christakoglou@cern.ch
//-----------------------------------------------------------------



//ROOT
#include <TROOT.h>
#include <TObject.h>
#include <TSystem.h>
#include <TObject.h>

#include <TPaveText.h>
#include <TText.h>
#include <TLine.h>
#include <TCanvas.h>

#include <TControlBar.h>
#include <TRootControlBar.h>



#include "AliLog.h"
#include "AliESD.h"

#include "AliAnalysisEventCuts.h"

ClassImp(AliAnalysisEventCuts)

//----------------------------------------//
AliAnalysisEventCuts::AliAnalysisEventCuts()
{
  Reset();
}

//----------------------------------------//
AliAnalysisEventCuts::~AliAnalysisEventCuts()
{
}

//----------------------------------------//
void AliAnalysisEventCuts::Reset()
{
  fVxMin = -1000.0;
  fVxMax = 1000.0; 
  fVyMin = -1000.0;
  fVyMax = 1000.0;  
  fVzMin = -1000.0;
  fVzMax = 1000.0;
  fMultMin = 0;
  fMultMax = 100000;
  
  fMult = 0;  
  fVx = 0;  
  fVy = 0; 
  fVz = 0; 
  fTotalEvents = 0; 
  fAcceptedEvents = 0; 

  fFlagMult = 0;  
  fFlagVx = 0;  
  fFlagVy = 0; 
  fFlagVz = 0; 
}

//----------------------------------------//
void AliAnalysisEventCuts::SetPrimaryVertexXRange(Float_t r1, Float_t r2)
{
  fVxMin = r1;
  fVxMax = r2; 
  fFlagVx = 1;  
}

//----------------------------------------//
void AliAnalysisEventCuts::SetPrimaryVertexYRange(Float_t r1, Float_t r2)
{
  fVyMin = r1;
  fVyMax = r2; 
  fFlagVy = 1;
}

//----------------------------------------//
void AliAnalysisEventCuts::SetPrimaryVertexZRange(Float_t r1, Float_t r2)
{
  fVzMin = r1;
  fVzMax = r2; 
  fFlagVz = 1;
}

//----------------------------------------//
void AliAnalysisEventCuts::SetMultiplicityRange(Int_t n1, Int_t n2)
{
  fMultMin = n1;
  fMultMax = n2; 
  fFlagMult = 1;
}


//----------------------------------------//
Bool_t AliAnalysisEventCuts::IsAccepted(AliESD *esd)
{
  fTotalEvents++;
  if((esd->GetNumberOfTracks() < fMultMin) || (esd->GetNumberOfTracks() > fMultMax))
    {
      fMult++;
      AliInfo(Form("Event rejected due to multiplicity cut"));
      return kFALSE;
    }
  const AliESDVertex *esdvertex = esd->GetVertex();
  if((esdvertex->GetXv() < fVxMin) || (esdvertex->GetXv() > fVxMax))
    {
      fVx++;
      AliInfo(Form("Event rejected due to Vx cut"));
      return kFALSE;
    }
  if((esdvertex->GetYv() < fVyMin) || (esdvertex->GetYv() > fVyMax))
    {
      fVy++;
      AliInfo(Form("Event rejected due to Vy cut"));
      return kFALSE;
    }
 if((esdvertex->GetZv() < fVzMin) || (esdvertex->GetZv() > fVzMax))
    {
      fVz++;
      AliInfo(Form("Event rejected due to Vz cut"));
      return kFALSE;
    }
 fAcceptedEvents++;

 return kTRUE;
}


//----------------------------------------//
TPaveText *AliAnalysisEventCuts::GetEventCuts()
{
  TCanvas *ccuts = new TCanvas("ccuts","Event cuts",10,10,400,400);
  ccuts->SetFillColor(10);
  ccuts->SetHighLightColor(10);

  TPaveText *pave = new TPaveText(0.01,0.01,0.98,0.98);
  pave->SetFillColor(5);
  Char_t CutName[256];
 
  TLine *l1 = pave->AddLine(0,0.78,1,0.78);
  l1->SetLineWidth(2);
  TLine *l2 = pave->AddLine(0,0.58,1,0.58);
  l2->SetLineWidth(2);
  TLine *l3 = pave->AddLine(0,0.38,1,0.38);
  l3->SetLineWidth(2);
  TLine *l4 = pave->AddLine(0,0.18,1,0.18);
  l4->SetLineWidth(2);
 
  sprintf(CutName,"Total number of events: %d",fTotalEvents);
  TText *t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
 
  sprintf(CutName,"Total number of accepted events: %d",fAcceptedEvents);
  t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
 
  sprintf(CutName,"Multiplicity range: [%d,%d]",fMultMin,fMultMax);
  t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
  sprintf(CutName,"Events rejected: %d",fMult);
  t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
 
  sprintf(CutName,"Vx range: [%f,%f]",fVxMin,fVxMax);
  t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
  sprintf(CutName,"Events rejected: %d",fVx);
  t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
 
  sprintf(CutName,"Vy range: [%f,%f]",fVyMin,fVyMax);
  t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
  sprintf(CutName,"Events rejected: %d",fVy);
  t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
 
  sprintf(CutName,"Vz range: [%f,%f]",fVzMin,fVzMax);
  t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
  sprintf(CutName,"Events rejected: %d",fVz);
  t1 = pave->AddText(CutName);
  t1->SetTextColor(4);
  t1->SetTextSize(0.04);
  t1->SetTextAlign(11);
 
  return pave;
}

//----------------------------------------//
void AliAnalysisEventCuts::GetEventStats()
{
  AliInfo(Form("Total number of events: %d",fTotalEvents));
  AliInfo(Form("Total number of accepted events: %d",fAcceptedEvents)); 
}

//----------------------------------------//
void AliAnalysisEventCuts::GetMultStats()
{
  AliInfo(Form("Multiplicity range: [%d,%d]",fMultMin,fMultMax));
  AliInfo(Form("Events rejected: %d",fMult));
}

//----------------------------------------//
void AliAnalysisEventCuts::GetVxStats()
{
  AliInfo(Form("Vx range: [%f,%f]",fVxMin,fVxMax));
  AliInfo(Form("Events rejected: %d",fVx));
}

//----------------------------------------//
void AliAnalysisEventCuts::GetVyStats()
{
  AliInfo(Form("Vy range: [%f,%f]",fVyMin,fVyMax));
  AliInfo(Form("Events rejected: %d",fVy));
}

//----------------------------------------//
void AliAnalysisEventCuts::GetVzStats()
{
  AliInfo(Form("Vz range: [%f,%f]",fVzMin,fVzMax));
  AliInfo(Form("Events rejected: %d",fVz));
}

//----------------------------------------//
void AliAnalysisEventCuts::PrintEventCuts()
{
  /*gROOT->Reset();
  TControlBar *menu1 = new TControlBar("vertical","Event Cuts",10,10);
  menu1->AddButton("Event statistics","GetEventStats()","Displays the event statistics");
  menu1->AddButton("Multipicity cut","gAlice->Run()","Events rejected due to multiplicity cut");
  menu1->AddButton("Vx cut","gAlice->RunLego()","Events rejected due to Vx cut");
  menu1->AddButton("Vy cut","DrawTopView()","Events rejected due to Vy cut");
  menu1->AddButton("Vz cut","DrawSideView()","Events rejected due to Vz cut");
  menu1->Show();
 
  gROOT->SaveContext(); */

  //GetEventCuts()->Draw();  

  AliInfo(Form("**************EVENT CUTS**************"));
  GetEventStats();
  if(fFlagMult)
    GetMultStats();
  if(fFlagVx)
    GetVxStats();
  if(fFlagVy)
    GetVyStats();
  if(fFlagVz)
    GetVzStats();
  AliInfo(Form("**************************************"));
}


     
