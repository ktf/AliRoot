/* $Id$ */

// ------------------------------------------------------
//
// Class to handle corrections.
//
// ------------------------------------------------------
//

#include <TFile.h>
#include <TCanvas.h>
#include <TH2F.h>

#include <AliLog.h>

#include "AliCorrectionMatrix.h"

//____________________________________________________________________
ClassImp(AliCorrectionMatrix)

//____________________________________________________________________
AliCorrectionMatrix::AliCorrectionMatrix() : TNamed(),
  fhMeas(0),
  fhGene(0),
  fhCorr(0)
{
  // default constructor
}

//____________________________________________________________________
AliCorrectionMatrix::AliCorrectionMatrix(const Char_t* name, const Char_t* title) : TNamed(name, title),
  fhMeas(0),
  fhGene(0),
  fhCorr(0)
{
  // constructor initializing tnamed
}

//____________________________________________________________________
AliCorrectionMatrix::AliCorrectionMatrix(const AliCorrectionMatrix& c) : TNamed(c),
  fhMeas(0),
  fhGene(0),
  fhCorr(0)
{
  // copy constructor
  ((AliCorrectionMatrix &)c).Copy(*this);
}

//____________________________________________________________________
AliCorrectionMatrix::~AliCorrectionMatrix()
{
  //
  // destructor
  //

  if (fhMeas)
  {
    delete fhMeas;
    fhMeas = 0;
  }

  if (fhGene)
  {
    delete fhGene;
    fhGene = 0;
  }

  if (fhCorr)
  {
    delete fhCorr;
    fhCorr = 0;
  }
}

//____________________________________________________________________
AliCorrectionMatrix &AliCorrectionMatrix::operator=(const AliCorrectionMatrix &c)
{
  // assigment operator

  if (this != &c)
    ((AliCorrectionMatrix &) c).Copy(*this);

  return *this;
}

//____________________________________________________________________
void AliCorrectionMatrix::Copy(TObject& c) const
{
  // copy function

  AliCorrectionMatrix& target = (AliCorrectionMatrix &) c;

  if (fhMeas)
    target.fhMeas = dynamic_cast<TH1*> (fhMeas->Clone());

  if (fhGene)
    target.fhGene = dynamic_cast<TH1*> (fhGene->Clone());

  if (fhCorr)
    target.fhCorr = dynamic_cast<TH1*> (fhCorr->Clone());
}

//________________________________________________________________________
void AliCorrectionMatrix::SetAxisTitles(const Char_t* titleX, const Char_t* titleY, const Char_t* titleZ)
{
  //
  // method for setting the axis titles of the histograms
  //

  fhMeas ->SetXTitle(titleX);  fhMeas ->SetYTitle(titleY);  fhMeas ->SetZTitle(titleZ);
  fhGene ->SetXTitle(titleX);  fhGene ->SetYTitle(titleY);  fhGene ->SetZTitle(titleZ);
  fhCorr ->SetXTitle(titleX);  fhCorr ->SetYTitle(titleY);  fhCorr ->SetZTitle(titleZ);
}

//____________________________________________________________________
Long64_t AliCorrectionMatrix::Merge(TCollection* list)
{
  // Merge a list of AliCorrectionMatrix objects with this (needed for
  // PROOF). 
  // Returns the number of merged objects (including this).

  if (!list)
    return 0;
  
  if (list->IsEmpty())
    return 1;

  TIterator* iter = list->MakeIterator();
  TObject* obj;

  // collections of measured and generated histograms
  TList* collectionMeas = new TList;
  TList* collectionGene = new TList;
  
  Int_t count = 0;
  while ((obj = iter->Next())) {
    
    AliCorrectionMatrix* entry = dynamic_cast<AliCorrectionMatrix*> (obj);
    if (entry == 0) 
      continue;

    collectionMeas->Add(entry->GetMeasuredHistogram());
    collectionGene->Add(entry->GetGeneratedHistogram());

    count++;
  }
  fhMeas->Merge(collectionMeas);
  fhGene->Merge(collectionGene);

  delete collectionMeas;
  delete collectionGene;

  return count+1;
}

//____________________________________________________________________
void AliCorrectionMatrix::Divide()
{
  //
  // divides generated by measured to get the correction
  //

//   if (!fhCorr) {
//     fhCorr = (TH1*)fhGene->Clone("correction");
//     fhCorr->SetTitle(Form("%s correction",GetTitle()));
//     fhCorr->Reset();
//   }

  if (!fhMeas || !fhGene || !fhCorr) {
    AliDebug(AliLog::kError, "measured or generated histograms not available");
    return;
  }

  fhCorr->Divide(fhGene, fhMeas, 1, 1, "B");

  Int_t emptyBins = 0;
  for (Int_t x=1; x<=fhCorr->GetNbinsX(); ++x)
    for (Int_t y=1; y<=fhCorr->GetNbinsY(); ++y)
      for (Int_t z=1; z<=fhCorr->GetNbinsZ(); ++z)
        if (fhCorr->GetBinContent(x, y, z) == 0)
          ++emptyBins;

  if (emptyBins > 0)
    printf("INFO: In %s we have %d empty bins (of %d) in the correction map\n", GetTitle(), emptyBins, fhCorr->GetNbinsX() * fhCorr->GetNbinsY() * fhCorr->GetNbinsZ());
}

//____________________________________________________________________
void AliCorrectionMatrix::Multiply()
{
  //
  // multiplies measured with correction to get the generated
  //

  if (!fhMeas || !fhGene || !fhCorr)
    return;

  fhGene->Multiply(fhMeas, fhCorr, 1, 1, "B");
}

//____________________________________________________________________
void AliCorrectionMatrix::Add(AliCorrectionMatrix* aMatrixToAdd, Float_t c) {
  //
  // adds the measured and generated of aMatrixToAdd to measured and generated of this
  // 
  // NB: the correction will naturally stay the same
  
  fhMeas->Add(aMatrixToAdd->GetMeasuredHistogram(), c);
  fhGene->Add(aMatrixToAdd->GetGeneratedHistogram(), c);
}


//____________________________________________________________________
Bool_t AliCorrectionMatrix::LoadHistograms(const Char_t* dir)
{
  //
  // loads the histograms from a file
  // if dir is empty a directory with the name of this object is taken (like in SaveHistogram)
  //

  if (!dir)
    dir = GetName();

  if (!gDirectory->cd(dir))
    return kFALSE;

  if (fhGene)
  {
    delete fhGene;
    fhGene=0;
  }

  if (fhCorr)
  {
    delete fhCorr;
    fhCorr=0;
  }

  if (fhMeas)
  {
    delete fhMeas;
    fhMeas=0;
  }

  fhMeas  = dynamic_cast<TH1*> (gDirectory->Get("measured"));
  if (!fhMeas)
    Info("LoadHistograms", "No measured hist available");

  fhGene  = dynamic_cast<TH1*> (gDirectory->Get("generated"));
  if (!fhGene)
    Info("LoadHistograms", "No generated hist available");

  fhCorr  = dynamic_cast<TH1*> (gDirectory->Get("correction"));

  Bool_t success = kTRUE;
  if (!fhCorr)
  {
    Info("LoadHistograms", "No correction hist available");
    success = kFALSE;
  }

  gDirectory->cd("..");

  return success;
}

//____________________________________________________________________
void AliCorrectionMatrix::SaveHistograms()
{
  //
  // saves the histograms
  //

  gDirectory->mkdir(GetName());
  gDirectory->cd(GetName());

  if (fhMeas)
    fhMeas ->Write();

  if (fhGene)
    fhGene ->Write();

  if (fhCorr)
    fhCorr->Write();

  gDirectory->cd("..");
}

//____________________________________________________________________
void AliCorrectionMatrix::DrawHistograms(const Char_t* canvasName)
{
  //
  // draws all histograms on one TCanvas
  // if canvasName is 0 the name of this object is taken
  //

  if (!canvasName)
    canvasName = Form("%s_canvas", GetName());

  TCanvas* canvas = new TCanvas(canvasName, GetTitle(), 1200, 400);
  canvas->Divide(3, 1);

  canvas->cd(1);
  if (fhMeas)
    fhMeas->Draw("COLZ");

  canvas->cd(2);
  if (fhGene)
  {
    // work around ROOT bug #22011
    if (fhGene->GetEntries() == 0)
      fhGene->SetEntries(1);
    fhGene->Draw("COLZ");
  }

  canvas->cd(3);
  if (fhCorr)
    fhCorr->Draw("COLZ");
}

//____________________________________________________________________
void AliCorrectionMatrix::ReduceInformation()
{
  // this function deletes the measured and generated histograms to reduce the amount of data
  // in memory

  if (fhMeas)
  {
    delete fhMeas;
    fhMeas = 0;
  }

  if (fhGene)
  {
    delete fhGene;
    fhGene = 0;
  }
}

//____________________________________________________________________
void AliCorrectionMatrix::Reset(Option_t* option)
{
  // resets the histograms

  if (fhGene)
    fhGene->Reset(option);

  if (fhMeas)
    fhMeas->Reset(option);

  if (fhCorr)
    fhCorr->Reset(option);
}

//____________________________________________________________________
void AliCorrectionMatrix::SetCorrectionToUnity()
{
  // sets the correction matrix to unity

  if (!fhCorr)
    return;

  for (Int_t x=1; x<=fhCorr->GetNbinsX(); ++x)
    for (Int_t y=1; y<=fhCorr->GetNbinsY(); ++y)
      for (Int_t z=1; z<=fhCorr->GetNbinsZ(); ++z)
      {
        fhCorr->SetBinContent(x, y, z, 1);
        fhCorr->SetBinError(x, y, z, 0);
      }
}

//____________________________________________________________________
void AliCorrectionMatrix::Scale(Double_t factor)
{
  // scales the generated and measured histogram with the given factor

  Printf("Scaling histograms with %f", factor);

  fhMeas->Scale(factor);
  fhGene->Scale(factor);
}
