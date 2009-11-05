// @(#)root/eve:$Id$
// Author: Matevz Tadel 2007

/**************************************************************************
 * Copyright(c) 1998-2008, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#include "AliEveTrackCounterEditor.h"
#include "AliEveTrackCounter.h"
#include "AliEveEventManager.h"

#include "TGedEditor.h"
#include "TVirtualPad.h"
#include "TColor.h"

// Cleanup these includes:
#include "TGLabel.h"
#include "TGNumberEntry.h"
#include "TGComboBox.h"
#include "TGMsgBox.h"

#include "TTree.h"
#include "TH1F.h"

#include "TCanvas.h"
#include "TLatex.h"
#include "TEveManager.h"

#include "TROOT.h"
#include "TSystem.h" // File input/output for track-count status.

//______________________________________________________________________________
// GUI editor for AliEveTrackCounter.
//

ClassImp(AliEveTrackCounterEditor)

//______________________________________________________________________________
AliEveTrackCounterEditor::AliEveTrackCounterEditor(const TGWindow *p, Int_t width, Int_t height,
                                               UInt_t options, Pixel_t back) :
   TGedFrame(p, width, height, options | kVerticalFrame, back),
   fM(0),
   fClickAction(0),
   fInfoLabelTracks   (0),
   fInfoLabelTracklets(0),
   fEventId(0)
{
   // Constructor.

   MakeTitle("AliEveTrackCounter");

   Int_t labelW = 42;

   { // ClickAction
      TGHorizontalFrame* f = new TGHorizontalFrame(this);
      TGLabel* lab = new TGLabel(f, "Click:");
      f->AddFrame(lab, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 1, 10, 1, 2));
      fClickAction = new TGComboBox(f);
      fClickAction->AddEntry("Print", 0);
      fClickAction->AddEntry("Toggle", 1);
      TGListBox* lb = fClickAction->GetListBox();
      lb->Resize(lb->GetWidth(), 2*16);
      fClickAction->Resize(70, 20);
      fClickAction->Connect("Selected(Int_t)", "AliEveTrackCounterEditor", this,
                            "DoClickAction(Int_t)");
      f->AddFrame(fClickAction, new TGLayoutHints(kLHintsLeft, 1, 2, 1, 1));

      AddFrame(f);
   }

   { // fInfoLabelTracks
      TGHorizontalFrame* f = new TGHorizontalFrame(this);
      TGLabel* lab = new TGLabel(f, "Tracks:");
      f->AddFrame(lab, new TGLayoutHints(kLHintsLeft, 1, 5, 1, 2));

      fInfoLabelTracks = new TGLabel(f);
      f->AddFrame(fInfoLabelTracks, new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 1, 9, 1, 2));

      AddFrame(f);
   }
   { // fInfoLabelTracklets
      TGHorizontalFrame* f = new TGHorizontalFrame(this);
      TGLabel* lab = new TGLabel(f, "Tracklets:");
      f->AddFrame(lab, new TGLayoutHints(kLHintsLeft, 1, 5, 1, 2));

      fInfoLabelTracklets = new TGLabel(f);
      f->AddFrame(fInfoLabelTracklets, new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 1, 9, 1, 2));

      AddFrame(f);
   }

   {
      TGHorizontalFrame* f = new TGHorizontalFrame(this, 210, 20, kFixedWidth);

      TGHorizontalFrame* g = new TGHorizontalFrame(f, labelW, 0, kFixedWidth);
      TGLabel* l = new TGLabel(g, "Event:");
      g->AddFrame(l, new TGLayoutHints(kLHintsLeft, 0,0,4,0));
      f->AddFrame(g);

      TGTextButton* b;

      b = new TGTextButton(f, "Prev");
      f->AddFrame(b, new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 1, 1, 0, 0));
      b->Connect("Clicked()", "AliEveTrackCounterEditor", this, "DoPrev()");

      fEventId = new TGNumberEntry(f, 0, 3, -1,TGNumberFormat::kNESInteger, TGNumberFormat::kNEANonNegative,
                                   TGNumberFormat::kNELLimitMinMax, 0, 10000);
      f->AddFrame(fEventId, new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 1, 1, 0, 0));
      fEventId->Connect("ValueSet(Long_t)", "AliEveTrackCounterEditor", this, "DoSetEvent()");

      b = new TGTextButton(f, "Next");
      f->AddFrame(b, new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 1, 1, 0, 0));
      b->Connect("Clicked()", "AliEveTrackCounterEditor", this, "DoNext()");

      AddFrame(f);
   }

   {
      TGHorizontalFrame* f = new TGHorizontalFrame(this, 210, 20, kFixedWidth);

      TGHorizontalFrame* g = new TGHorizontalFrame(f, labelW, 0, kFixedWidth);
      TGLabel* l = new TGLabel(g, "Report:");
      g->AddFrame(l, new TGLayoutHints(kLHintsLeft, 0,0,4,0));
      f->AddFrame(g);

      TGTextButton* b;

      b = new TGTextButton(f, "Print");
      f->AddFrame(b, new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 1, 1, 0, 0));
      b->Connect("Clicked()", "AliEveTrackCounterEditor", this, "DoPrintReport()");

      b = new TGTextButton(f, "File");
      f->AddFrame(b, new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 1, 1, 0, 0));
      b->Connect("Clicked()", "AliEveTrackCounterEditor", this, "DoFileReport()");

      AddFrame(f, new TGLayoutHints(kLHintsLeft, 0, 0, 4, 0));
   }
   {
      TGHorizontalFrame* f = new TGHorizontalFrame(this, 210, 20, kFixedWidth);

      TGHorizontalFrame* g = new TGHorizontalFrame(f, labelW, 0, kFixedWidth);
      TGLabel* l = new TGLabel(g, "Histos:");
      g->AddFrame(l, new TGLayoutHints(kLHintsLeft, 0,0,4,0));
      f->AddFrame(g);

      TGTextButton* b;

      b = new TGTextButton(f, "Show");
      f->AddFrame(b, new TGLayoutHints(kLHintsLeft|kLHintsExpandX, 1, 1, 0, 0));
      b->Connect("Clicked()", "AliEveTrackCounterEditor", this, "DoShowHistos()");

      AddFrame(f, new TGLayoutHints(kLHintsLeft, 0, 0, 0, 0));
   }

  AliEveEventManager::GetMaster()->Connect("NewEventLoaded()",
                        "AliEveTrackCounterEditor", this, "UpdateModel()");
}

AliEveTrackCounterEditor::~AliEveTrackCounterEditor()
{
  // Destructor.

  AliEveEventManager::GetMaster()->Disconnect("NewEventLoaded()", this);
}

/******************************************************************************/

void AliEveTrackCounterEditor::UpdateModel()
{
  if (fGedEditor && fM && fGedEditor->GetModel() == fM->GetEditorObject())
  {
    SetModel(fM->GetEditorObject());
  }
}

//______________________________________________________________________________
void AliEveTrackCounterEditor::SetModel(TObject* obj)
{
   // Set model object.

   fM = dynamic_cast<AliEveTrackCounter*>(obj);

   fClickAction->Select(fM->fClickAction, kFALSE);
   fInfoLabelTracks   ->SetText(Form("All: %3d; Primaries: %3d", fM->fAllTracks,    fM->fGoodTracks));
   fInfoLabelTracklets->SetText(Form("All: %3d; Primaries: %3d", fM->fAllTracklets, fM->fGoodTracklets));
   fEventId->SetNumber(fM->GetEventId());
}

/******************************************************************************/

//______________________________________________________________________________
void AliEveTrackCounterEditor::DoPrev()
{
   // Slot for Prev.

   AliEveEventManager::GetMaster()->PrevEvent();
}

//______________________________________________________________________________
void AliEveTrackCounterEditor::DoNext()
{
   // Slot for Next.

   AliEveEventManager::GetMaster()->NextEvent();
}

//______________________________________________________________________________
void AliEveTrackCounterEditor::DoSetEvent()
{
   // Slot for SetEvent.
   AliEveEventManager::GetMaster()->GotoEvent((Int_t) fEventId->GetNumber());
}

/******************************************************************************/

//______________________________________________________________________________
void AliEveTrackCounterEditor::DoPrintReport()
{
   // Slot for PrintReport.

   fM->PrintEventTracks();
}

//______________________________________________________________________________
void AliEveTrackCounterEditor::DoFileReport()
{
   // Slot for FileReport.

   fM->OutputEventTracks();
}

//______________________________________________________________________________
void AliEveTrackCounterEditor::DoShowHistos()
{
  // Slot for ShowHistos.

  static const TEveException kEH("AliEveTrackCounterEditor::DoShowHistos ");

  TEveUtil::Macro("show_scan_results.C");

  TTree* t = (TTree*) gDirectory->Get("SR");

  if (t == 0)
    throw kEH + "Tree 'SR' with scan results not found.";

  TCanvas *c = 0;

  //----------------------------------------------------------------------------
  // Tracks
  //----------------------------------------------------------------------------

  c = new TCanvas("Tracks", "Track Scanning Results", 800, 600);
  c->Divide(2, 3);

  c->cd(1);
  t->Draw("Sum$(T.fLabel & 1)");

  c->cd(2);
  t->Draw("T.GetSign()", "T.fLabel & 1");

  c->cd(3);
  t->Draw("T.GetSign()", "T.fLabel & 1");

  c->cd(4);
  t->Draw("T.Eta()", "T.fLabel & 1");

  c->cd(5);
  t->Draw("T.Phi()", "T.fLabel & 1");

  c->Modified();
  c->Update();

  //----------------------------------------------------------------------------
  // Trackelts
  //----------------------------------------------------------------------------

  c = new TCanvas("Tracklets", "Tracklet Scanning Results", 800, 600);
  (new TLatex(0.2, 0.4, "Not yet available"))->Draw();

  c->Modified();
  c->Update();
}

/******************************************************************************/

//______________________________________________________________________________
void AliEveTrackCounterEditor::DoClickAction(Int_t mode)
{
   // Slot for ClickAction.

   fM->SetClickAction(mode);
}
