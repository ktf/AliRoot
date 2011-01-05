#include "AliFMDMCCorrections.h"
#include <AliESDFMD.h>
#include <TAxis.h>
#include <TList.h>
#include <TMath.h>
#include "AliForwardCorrectionManager.h"
// #include "AliFMDAnaParameters.h"
#include "AliLog.h"
#include <TH2D.h>
#include <TROOT.h>
#include <TProfile2D.h>
#include <iostream>
#include <iomanip>

ClassImp(AliFMDMCCorrections)
#if 0
; // For Emacs
#endif 


//____________________________________________________________________
AliFMDMCCorrections::~AliFMDMCCorrections()
{
  if (fComps) fComps->Clear();
  if (fFMD1i) delete fFMD1i;
  if (fFMD2i) delete fFMD2i;
  if (fFMD2o) delete fFMD2o;
  if (fFMD3i) delete fFMD3i;
  if (fFMD3o) delete fFMD3o;
}

//____________________________________________________________________
AliFMDMCCorrections&
AliFMDMCCorrections::operator=(const AliFMDMCCorrections& o)
{
  AliFMDCorrections::operator=(o);

  return *this;
}

//____________________________________________________________________
Bool_t
AliFMDMCCorrections::CorrectMC(AliForwardUtil::Histos& hists,
			       UShort_t                vtxbin)
{
  AliForwardCorrectionManager& fcm = AliForwardCorrectionManager::Instance();

  UShort_t uvb = vtxbin;
  for (UShort_t d=1; d<=3; d++) { 
    UShort_t nr = (d == 1 ? 1 : 2);
    for (UShort_t q=0; q<nr; q++) { 
      Char_t      r  = (q == 0 ? 'I' : 'O');
      TH2D*       h  = hists.Get(d,r);
      TH2D*       bg = fcm.GetSecondaryMap()->GetCorrection(d,r,uvb);
      TH2D*       ef = fcm.GetVertexBias()->GetCorrection(r, uvb);
      if (!bg) { 
	AliWarning(Form("No secondary correction for FMDM%d%c in vertex bin %d",
			d, r, uvb));
	continue;
      }
      if (!ef) { 
	AliWarning(Form("No event vertex bias correction in vertex bin %d",
			uvb));
	continue;
      }

      // Divide by primary/total ratio
      h->Divide(bg);
      
      // Divide by the event selection efficiency 
      h->Divide(ef);
    }
  }
  
  return kTRUE;
}

//____________________________________________________________________
void
AliFMDMCCorrections::Init(const TAxis& eAxis)
{
  fFMD1i = Make(1,'I',eAxis);
  fFMD2i = Make(2,'I',eAxis);
  fFMD2o = Make(2,'O',eAxis);
  fFMD3i = Make(3,'I',eAxis);
  fFMD3o = Make(3,'O',eAxis);

  fComps->Add(fFMD1i);
  fComps->Add(fFMD2i);
  fComps->Add(fFMD2o);
  fComps->Add(fFMD3i);
  fComps->Add(fFMD3o);
}

//____________________________________________________________________
TProfile2D*
AliFMDMCCorrections::Make(UShort_t d, Char_t r, 
				const TAxis& axis) const
{
  TProfile2D* ret = new TProfile2D(Form("FMD%d%c_esd_vs_mc", d, r),
				   Form("ESD/MC signal for FMD%d%c", d, r),
				   axis.GetNbins(), 
				   axis.GetXmin(),
				   axis.GetXmax(), 
				   (r == 'I' || r == 'i') ? 20 : 40,
				   0, 2*TMath::Pi());
  ret->GetXaxis()->SetTitle("#eta");
  ret->GetYaxis()->SetTitle("#varphi [degrees]");
  ret->GetZaxis()->SetTitle("#LT primary density ESD/MC#GT");
  ret->SetDirectory(0);
  return ret;
}
//____________________________________________________________________
void
AliFMDMCCorrections::Fill(UShort_t d, Char_t r, TH2* esd, TH2* mc)
{
  if (!esd || !mc) return;
  TProfile2D* p = 0;
  switch (d) { 
  case 1:  p = fFMD1i;                                   break;
  case 2:  p = (r == 'I' || r == 'i' ? fFMD2i : fFMD2o); break;
  case 3:  p = (r == 'I' || r == 'i' ? fFMD3i : fFMD3o); break;
  }
  if (!p) return;

  for (Int_t iEta = 1; iEta <= esd->GetNbinsX(); iEta++) { 
    Double_t eta = esd->GetXaxis()->GetBinCenter(iEta);
    for (Int_t iPhi = 1; iPhi <= esd->GetNbinsY(); iPhi++) { 
      Double_t phi  = esd->GetYaxis()->GetBinCenter(iPhi);
      Double_t mEsd = esd->GetBinContent(iEta,iPhi);
      Double_t mMc  = mc->GetBinContent(iEta,iPhi);
      
      p->Fill(eta, phi, (mMc > 0 ? mEsd / mMc : 0));
    }
  }
}

//____________________________________________________________________
Bool_t
AliFMDMCCorrections::CompareResults(AliForwardUtil::Histos& esd,
					  AliForwardUtil::Histos& mc)
{
  Fill(1, 'I', esd.Get(1,'I'), mc.Get(1,'I'));
  Fill(2, 'I', esd.Get(2,'I'), mc.Get(2,'I'));
  Fill(2, 'O', esd.Get(2,'O'), mc.Get(2,'O'));
  Fill(3, 'I', esd.Get(3,'I'), mc.Get(3,'I'));
  Fill(3, 'O', esd.Get(3,'O'), mc.Get(3,'O'));

  return kTRUE;
}

//____________________________________________________________________
void
AliFMDMCCorrections::DefineOutput(TList* dir)
{
  AliFMDCorrections::DefineOutput(dir);
  TList* d = static_cast<TList*>(dir->FindObject(GetName()));

  fComps = new TList;
  fComps->SetName("esd_mc_comparison");
  d->Add(fComps);
}

//____________________________________________________________________
//
// EOF
//
	  


