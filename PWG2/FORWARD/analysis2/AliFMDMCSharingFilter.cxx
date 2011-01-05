//
// Class to do the sharing correction of FMD ESD data
//
#include "AliFMDMCSharingFilter.h"
#include <AliESDFMD.h>
#include <AliMCEvent.h>
#include <AliTrackReference.h>
#include <AliStack.h>
#include <TAxis.h>
#include <TList.h>
#include <TH1.h>
#include <TMath.h>
#include "AliForwardCorrectionManager.h"
// #include "AliFMDAnaParameters.h"
#include "AliFMDStripIndex.h"
#include <AliLog.h>
#include <TROOT.h>
#include <iostream>
#include <iomanip>

ClassImp(AliFMDMCSharingFilter)
#if 0
; // This is for Emacs
#endif 


//____________________________________________________________________
AliFMDMCSharingFilter::AliFMDMCSharingFilter()
  : AliFMDSharingFilter(), 
    fFMD1i(0),
    fFMD2i(0),
    fFMD2o(0),
    fFMD3i(0),
    fFMD3o(0), 
    fSumEta(0)
{}

//____________________________________________________________________
AliFMDMCSharingFilter::AliFMDMCSharingFilter(const char* title)
  : AliFMDSharingFilter(title), 
    fFMD1i(0),
    fFMD2i(0),
    fFMD2o(0),
    fFMD3i(0),
    fFMD3o(0),
    fSumEta(0)
{
  fFMD1i = new TH2D("FMD1i_corr", "Merged vs MC", 21, -.5, 20.5, 100, 0, 20);
  fFMD2i = new TH2D("FMD2i_corr", "Merged vs MC", 21, -.5, 20.5, 100, 0, 20);
  fFMD2o = new TH2D("FMD2o_corr", "Merged vs MC", 21, -.5, 20.5, 100, 0, 20);
  fFMD3i = new TH2D("FMD3i_corr", "Merged vs MC", 21, -.5, 20.5, 100, 0, 20);
  fFMD3o = new TH2D("FMD3o_corr", "Merged vs MC", 21, -.5, 20.5, 100, 0, 20);
  fFMD1i->SetYTitle("#Delta E/#Delta_{mip} (ESD)");
  fFMD1i->SetXTitle("Hits (MC)");
  fFMD2i->SetYTitle("#Delta E/#Delta_{mip} (ESD)");
  fFMD2i->SetXTitle("Hits (MC)");
  fFMD2o->SetYTitle("#Delta E/#Delta_{mip} (ESD)");
  fFMD2o->SetXTitle("Hits (MC)");
  fFMD3i->SetYTitle("#Delta E/#Delta_{mip} (ESD)");
  fFMD3i->SetXTitle("Hits (MC)");
  fFMD3o->SetYTitle("#Delta E/#Delta_{mip} (ESD)");
  fFMD3o->SetXTitle("Hits (MC)");
  fFMD1i->SetDirectory(0);
  fFMD2i->SetDirectory(0);
  fFMD2o->SetDirectory(0);
  fFMD3i->SetDirectory(0);
  fFMD3o->SetDirectory(0);
  fSumEta = new TH1D("mcSumEta", "MC INEL Truth", 200, -4, 6);
  fSumEta->SetXTitle("#eta");
  fSumEta->SetYTitle("dN_{ch}/d#eta");
  fSumEta->SetDirectory(0);
  fSumEta->Sumw2();
  fSumEta->SetMarkerColor(kOrange+2);
  fSumEta->SetMarkerStyle(22);
  fSumEta->SetFillColor(0);
  fSumEta->SetFillStyle(0);
  
}

//____________________________________________________________________
AliFMDMCSharingFilter::AliFMDMCSharingFilter(const AliFMDMCSharingFilter& o)
  : AliFMDSharingFilter(o), 
    fFMD1i(o.fFMD1i),
    fFMD2i(o.fFMD2i),
    fFMD2o(o.fFMD2o),
    fFMD3i(o.fFMD3i),
    fFMD3o(o.fFMD3o),
    fSumEta(o.fSumEta)
{
}

//____________________________________________________________________
AliFMDMCSharingFilter::~AliFMDMCSharingFilter()
{
  if (fFMD1i)  delete fFMD1i;
  if (fFMD2i)  delete fFMD2i;
  if (fFMD2o)  delete fFMD2o;
  if (fFMD3i)  delete fFMD3i;
  if (fFMD3o)  delete fFMD3o;
  if (fSumEta) delete fSumEta;
}

//____________________________________________________________________
AliFMDMCSharingFilter&
AliFMDMCSharingFilter::operator=(const AliFMDMCSharingFilter& o)
{
  AliFMDSharingFilter::operator=(o);
  return *this;
}

//____________________________________________________________________
void
AliFMDMCSharingFilter::StoreParticle(UShort_t   d, 
				     Char_t     r,
				     UShort_t   s, 
				     UShort_t   t, 
				     AliESDFMD& output) const
{
  Double_t old = output.Multiplicity(d,r,s,t);
  if (old == AliESDFMD::kInvalidMult) old = 0;
  output.SetMultiplicity(d,r,s,t,old+1);
}

//____________________________________________________________________
Bool_t
AliFMDMCSharingFilter::FilterMC(const AliESDFMD&  input, 
				const AliMCEvent& event,
				Double_t          vz,
				AliESDFMD&        output)
{
  output.Clear();

  // Copy eta values to output 
  for (UShort_t ed = 1; ed <= 3; ed++) { 
    UShort_t nq = (ed == 1 ? 1 : 2);
    for (UShort_t eq = 0; eq < nq; eq++) {
      Char_t   er = (eq == 0 ? 'I' : 'O');
      UShort_t ns = (eq == 0 ?  20 :  40);
      UShort_t nt = (eq == 0 ? 512 : 256);
      for (UShort_t es = 0; es < ns; es++) 
	for (UShort_t et = 0; et < nt; et++) 
	  output.SetEta(ed, er, es, et, input.Eta(ed, er, es, et));
    }
  }
  AliStack* stack = const_cast<AliMCEvent&>(event).Stack();
  Int_t nTracks   = event.GetNumberOfTracks();
  for (Int_t iTr = 0; iTr < nTracks; iTr++) { 
    AliMCParticle* particle = 
      static_cast<AliMCParticle*>(event.GetTrack(iTr));
    
    // Check the returned particle 
    if (!particle) continue;
    
    // Check if this charged and a primary 
    Bool_t isCharged = particle->Charge() != 0;
    if (!isCharged) continue;
    Bool_t isPrimary = stack->IsPhysicalPrimary(iTr);

    // Fill 'dn/deta' histogram 
    if (isPrimary) fSumEta->Fill(particle->Eta());

    Int_t    nTrRef  = particle->GetNumberOfTrackReferences();
    Int_t    longest = -1;
    Double_t angle   = 0;
    UShort_t oD = 0, oS = 1024, oT = 1024;
    Char_t   oR = '\0';
    for (Int_t iTrRef = 0; iTrRef < nTrRef; iTrRef++) { 
      AliTrackReference* ref = particle->GetTrackReference(iTrRef);
      
      // Check existence 
      if (!ref) continue;

      // Check that we hit an FMD element 
      if (ref->DetectorId() != AliTrackReference::kFMD) 
	continue;

      // Get the detector coordinates 
      UShort_t d, s, t;
      Char_t r;
      AliFMDStripIndex::Unpack(ref->UserId(), d, r, s, t);
      if (oD > 0 && oR != '\0' && (d != oD || r != oR)) {
	longest = -1;
	StoreParticle(oD, oR, oS, oT, output);
      }

      oD = d;
      oR = r;
      oS = s;
      oT = t;

      // The longest passage is determined through the angle 
      Double_t x    = ref->X();
      Double_t y    = ref->Y();
      Double_t z    = ref->Z()-vz;
      Double_t rr   = TMath::Sqrt(x*x+y*y);
      Double_t theta= TMath::ATan2(rr,z);
      Double_t ang  = TMath::Abs(TMath::Pi()-theta);
      if (ang > angle) {
	longest = iTrRef;
	angle   = ang;
      }
    } // Loop over track references
    if (longest < 0) continue;

    // Get the reference corresponding to the longest path through the detector
    AliTrackReference* ref = particle->GetTrackReference(longest);

    // Get the detector coordinates 
    UShort_t d, s, t;
    Char_t r;
    AliFMDStripIndex::Unpack(ref->UserId(), d, r, s, t);
    
    StoreParticle(d,r,s,t,output);
  } // Loop over tracks
  return kTRUE;
}

//____________________________________________________________________
void
AliFMDMCSharingFilter::CompareResults(const AliESDFMD&  esd, 
				      const AliESDFMD&  mc)
{
  // Copy eta values to output 
  for (UShort_t d = 1; d <= 3; d++) { 
    UShort_t nq = (d == 1 ? 1 : 2);
    for (UShort_t q = 0; q < nq; q++) {
      Char_t   r  = (q == 0 ? 'I' : 'O');
      UShort_t ns = (q == 0 ?  20 :  40);
      UShort_t nt = (q == 0 ? 512 : 256);
      TH2*     co = 0;
      switch (d) { 
      case 1: co = fFMD1i; break;
      case 2: co = (q == 0 ? fFMD2i : fFMD2o); break;
      case 3: co = (q == 0 ? fFMD3i : fFMD3o); break;
      }

      for (UShort_t s = 0; s < ns; s++) {
	for (UShort_t t = 0; t < nt; t++) { 
	  Float_t mEsd = esd.Multiplicity(d, r, s, t);
	  Float_t mMc  = mc.Multiplicity(d, r, s, t);

	  co->Fill(mMc, mEsd);
	} 
      }
    }
  }
}
  
//____________________________________________________________________
void
AliFMDMCSharingFilter::DefineOutput(TList* dir)
{
  AliFMDSharingFilter::DefineOutput(dir);
  TList* d = static_cast<TList*>(dir->FindObject(GetName()));
  TList* cd = new TList;
  cd->SetName("esd_mc_comparion");
  d->Add(cd);
  cd->Add(fFMD1i);
  cd->Add(fFMD2i);
  cd->Add(fFMD2o);
  cd->Add(fFMD3i);
  cd->Add(fFMD3o);
  dir->Add(fSumEta);
}

//____________________________________________________________________
void
AliFMDMCSharingFilter::ScaleHistograms(TList* dir, Int_t nEvents)
{
  AliFMDSharingFilter::ScaleHistograms(dir, nEvents);
  TH1D* sumEta = static_cast<TH1D*>(dir->FindObject("mcSumEta"));
  if (!sumEta) { 
    AliWarning(Form("No mcSumEta histogram found in output list"));
    return;
  }
  sumEta->Scale(1. / nEvents, "width");
}

//____________________________________________________________________
void
AliFMDMCSharingFilter::Print(Option_t* option) const
{
  char ind[gROOT->GetDirLevel()+1];
  for (Int_t i = 0; i < gROOT->GetDirLevel(); i++) ind[i] = ' ';
  ind[gROOT->GetDirLevel()] = '\0';
  AliFMDSharingFilter::Print(option);
}

//____________________________________________________________________
//
// EOF
//
