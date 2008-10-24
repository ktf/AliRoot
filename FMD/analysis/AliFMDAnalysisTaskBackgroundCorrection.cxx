 
#include <TROOT.h>
#include <TSystem.h>
#include <TInterpreter.h>
#include <TChain.h>
#include <TFile.h>
#include <TList.h>
#include <iostream>
#include "TH2F.h"
#include "AliFMDAnalysisTaskBackgroundCorrection.h"
#include "AliAnalysisManager.h"
#include "AliESDFMD.h"
#include "AliESDEvent.h"
#include "AliAODEvent.h"
#include "AliAODHandler.h"
#include "AliMCEventHandler.h"
#include "AliStack.h"
#include "AliESDVertex.h"
#include "TMath.h"
#include "AliFMDAnaParameters.h"
#include "AliFMDGeometry.h"

ClassImp(AliFMDAnalysisTaskBackgroundCorrection)


AliFMDAnalysisTaskBackgroundCorrection::AliFMDAnalysisTaskBackgroundCorrection()
: fDebug(0),
  fChain(0x0),
  fOutputList(0),
  fArray(0),
  fInputArray(0)
{
  // Default constructor
  DefineInput (0, TList::Class());
  DefineOutput(0,TList::Class());
}

AliFMDAnalysisTaskBackgroundCorrection::AliFMDAnalysisTaskBackgroundCorrection(const char* name):
    AliAnalysisTask(name, "Density"),
    fDebug(0),
    fChain(0x0),
    fOutputList(0),
    fArray(0),
    fInputArray(0)
{
  DefineInput (0, TList::Class());
  DefineOutput(0, TList::Class());
}

void AliFMDAnalysisTaskBackgroundCorrection::CreateOutputObjects()
{
  AliFMDAnaParameters* pars = AliFMDAnaParameters::Instance();
  fOutputList = new TList();
  
  fArray     = new TObjArray();
  fArray->SetName("FMD");
  fArray->SetOwner();
  
  TH2F* hMult = 0;
  
  Int_t nVtxbins = pars->GetNvtxBins();
  
  for(Int_t det =1; det<=3;det++)
    {
      TObjArray* detArray = new TObjArray();
      detArray->SetName(Form("FMD%d",det));
      fArray->AddAtAndExpand(detArray,det);
      Int_t nRings = (det==1 ? 1 : 2);
      for(Int_t ring = 0;ring<nRings;ring++)
	{
	  Char_t ringChar = (ring == 0 ? 'I' : 'O');
	  Int_t  nSec     = (ring == 0 ? 20 : 40);
	  
	  TObjArray* vtxArray = new TObjArray();
	  vtxArray->SetName(Form("FMD%d%c",det,ringChar));
	  detArray->AddAtAndExpand(vtxArray,ring);
	  for(Int_t i = 0; i< nVtxbins; i++) {
	    TH2F* hBg = pars->GetBackgroundCorrection(det, ringChar, i);
	    hMult  = new TH2F(Form("mult_FMD%d%c_vtxbin%d",det,ringChar,i),Form("mult_FMD%d%c_vtxbin%d",det,ringChar,i),
			      hBg->GetNbinsX(),
			      hBg->GetXaxis()->GetXmin(),
			      hBg->GetXaxis()->GetXmax(),
			      nSec, 0, 2*TMath::Pi());
	    vtxArray->AddAtAndExpand(hMult,i);
	    
	  }
	} 
    }
    
  fOutputList->Add(fArray);
   
  
}

void AliFMDAnalysisTaskBackgroundCorrection::ConnectInputData(Option_t */*option*/)
{

  TList* list = (TList*)GetInputData(0);
  fInputArray = (TObjArray*)list->At(0);
    
}

void AliFMDAnalysisTaskBackgroundCorrection::Exec(Option_t */*option*/)
{
  AliFMDAnaParameters* pars = AliFMDAnaParameters::Instance();
  
  Int_t nVtxbins = pars->GetNvtxBins();
  
    
  for(UShort_t det=1;det<=3;det++) {
    TObjArray* detInputArray = (TObjArray*)fInputArray->At(det);
    TObjArray* detArray = (TObjArray*)fArray->At(det);
    Int_t nRings = (det==1 ? 1 : 2);
    for (UShort_t ir = 0; ir < nRings; ir++) {
      Char_t ringChar = (ir == 0 ? 'I' : 'O');
      TObjArray* vtxInputArray = (TObjArray*)detInputArray->At(ir);
      TObjArray* vtxArray = (TObjArray*)detArray->At(ir);
      for(Int_t i =0; i<nVtxbins; i++) {
	TH2F* hMultTotal = (TH2F*)vtxArray->At(i);
	TH2F* hMult      = (TH2F*)vtxInputArray->At(i);
	TH2F* hBg        = pars->GetBackgroundCorrection(det, ringChar, i);
	
	TH2F* hTmp       = (TH2F*)hMult->Clone("hMult_from_event");
	/*for(Int_t j=1; j<=hMult->GetNbinsX();j++) {
	  for(Int_t k=1; k<=hMult->GetNbinsY();k++) {
	    if(hBg->GetBinContent(j,k) != 0) {
	      // std::cout<<hMult->GetBinContent(j,k)<<"    "<<hBg->GetBinContent(j,k)<<std::endl;
	      hTmp->SetBinContent(j,k,hMult->GetBinContent(j,k)/hBg->GetBinContent(j,k));
	    }
	  }
	  }*/
	hTmp->Divide(hTmp,hBg,1,1,"B");
	
	hMultTotal->Add(hTmp);
	
      }
    }
  }
  
  PostData(0, fOutputList); 
  
}

