#include "AliFMDAnalysisTaskGenerateCorrection.h"
#include "AliESDEvent.h"
#include "iostream"
#include "AliESDFMD.h"
#include "TH2F.h"
#include "AliTrackReference.h"
#include "AliStack.h"
#include "AliFMDAnaParameters.h"
#include "AliFMDStripIndex.h"
#include "AliStack.h"
#include "AliMCParticle.h"
#include "AliMCEvent.h"
//#include "AliFMDGeometry.h"
#include "TArray.h"
#include "AliGenEventHeader.h"
#include "AliMultiplicity.h"
#include "AliHeader.h"
#include "AliFMDAnaCalibBackgroundCorrection.h"
#include "AliFMDAnaCalibEventSelectionEfficiency.h"
#include "AliGenPythiaEventHeader.h"
//#include "AliCDBManager.h"
//#include "AliCDBId.h"
//#include "AliCDBMetaData.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TAxis.h"
ClassImp(AliFMDAnalysisTaskGenerateCorrection)

//_____________________________________________________________________
AliFMDAnalysisTaskGenerateCorrection::AliFMDAnalysisTaskGenerateCorrection():
AliAnalysisTaskSE(),
  fListOfHits(), 
  fListOfPrimaries(),
  fListOfCorrection(),
  fVertexBins(),
  fLastTrackByStrip(0),
  fHitsByStrip(0),
  fZvtxCut(10),
  fNvtxBins(10),
  fNbinsEta(200),
  fBackground(0),
  fEventSelectionEff(0)
{
  // Default constructor
}
//_____________________________________________________________________
AliFMDAnalysisTaskGenerateCorrection::AliFMDAnalysisTaskGenerateCorrection(const char* name):
  AliAnalysisTaskSE(name),
  fListOfHits(), 
  fListOfPrimaries(),
  fListOfCorrection(),
  fVertexBins(),
  fLastTrackByStrip(0),
  fHitsByStrip(0),
  fZvtxCut(10),
  fNvtxBins(10),
  fNbinsEta(200),
  fBackground(0),
  fEventSelectionEff(0)
{
 
  DefineOutput(1, TList::Class());
  DefineOutput(2, TList::Class());
  DefineOutput(3, TH1F::Class());
  DefineOutput(4, TList::Class());
}
//_____________________________________________________________________
void AliFMDAnalysisTaskGenerateCorrection::UserCreateOutputObjects()
{
// Create the output containers
//
  
  std::cout<<"Creating output objects"<<std::endl;
  for(Int_t v=0; v<fNvtxBins;v++) {
    
    TH2F* hSPDhits       = new TH2F(Form("hSPDhits_vtx%d",v),
				    Form("hSPDhits_vtx%d",v),
				    fNbinsEta, -4,6, 20, 0,2*TMath::Pi());
    hSPDhits->Sumw2();
    fListOfHits.Add(hSPDhits);
    
    for(Int_t iring = 0; iring<2;iring++) {
      Char_t ringChar = (iring == 0 ? 'I' : 'O');
      Int_t nSec = (iring == 1 ? 40 : 20);
      
      TH2F* hPrimary       = new TH2F(Form("hPrimary_FMD_%c_vtx%d",ringChar,v),
				      Form("hPrimary_FMD_%c_vtx%d",ringChar,v),
				      fNbinsEta, -4,6, nSec, 0,2*TMath::Pi());
      hPrimary->Sumw2();
      fListOfPrimaries.Add(hPrimary);
      
      
    }
  }
  
 
  
  
  for(Int_t det =1; det<=3;det++) {
    Int_t nRings = (det==1 ? 1 : 2);
    for(Int_t ring = 0;ring<nRings;ring++) {
      Int_t nSec = (ring == 1 ? 40 : 20);
      Char_t ringChar = (ring == 0 ? 'I' : 'O');
      TH1F* doubleHits = new TH1F(Form("DoubleHits_FMD%d%c",det,ringChar),
				  Form("DoubleHits_FMD%d%c",det,ringChar),fNbinsEta, -4,6);
      TH1F* allHits = new TH1F(Form("allHits_FMD%d%c",det,ringChar),
			       Form("allHits_FMD%d%c",det,ringChar), fNbinsEta, -4,6);
      
      doubleHits->Sumw2();
      allHits->Sumw2();
      fListOfHits.Add(allHits);
      fListOfHits.Add(doubleHits);
	
      for(Int_t v=0; v<fNvtxBins;v++) {
	TH2F* hHits = new TH2F(Form("hHits_FMD%d%c_vtx%d", det,ringChar,v),
			       Form("hHits_FMD%d%c_vtx%d", det,ringChar,v),
			       fNbinsEta, -4,6, nSec, 0, 2*TMath::Pi());
	hHits->Sumw2();
	fListOfHits.Add(hHits);
	
      } 
    }
  }
  
  TH1F* hEventsSelected  = new TH1F("EventsSelected","EventsSelected",fNvtxBins,0,fNvtxBins);
  TH1F* hEventsAll    = new TH1F("EventsAll","EventsAll",fNvtxBins,0,fNvtxBins);
  TH1F* hEventsAllNSD    = new TH1F("EventsAllNSD","EventsAllNSD",fNvtxBins,0,fNvtxBins);
  TH1F* hEventsSelectedVtx  = new TH1F("EventsSelectedVtx","EventsSelectedVtx",fNvtxBins,0,fNvtxBins);  
  TH1F* hEventsSelectedTrigger  = new TH1F("EventsSelectedTrigger","EventsSelectedTrigger",fNvtxBins,0,fNvtxBins);
  
  TH1F* hEventsSelectedNSDVtx  = new TH1F("EventsSelectedNSDVtx","EventsSelectedNSDVtx",fNvtxBins,0,fNvtxBins);  
  TH1F* hEventsSelectedNSD     = new TH1F("EventsSelectedNSD","EventsSelectedNSD",fNvtxBins,0,fNvtxBins);
  
  TH1F* hXvtx = new TH1F("hXvtx","x vertex distribution",100,-2,2);
  TH1F* hYvtx = new TH1F("hYvtx","y vertex distribution",100,-2,2);
  TH1F* hZvtx = new TH1F("hZvtx","z vertex distribution",4*fNvtxBins,-4*fZvtxCut,4*fZvtxCut);
  
  fListOfPrimaries.Add(hXvtx);
  fListOfPrimaries.Add(hYvtx);
  fListOfPrimaries.Add(hZvtx);
  
  hEventsSelected->Sumw2();
  hEventsAll->Sumw2();
  hEventsAllNSD->Sumw2();
  
  fListOfHits.Add(hEventsSelected);
  fListOfHits.Add(hEventsSelectedVtx);
  fListOfHits.Add(hEventsSelectedTrigger);
  fListOfHits.Add(hEventsSelectedNSDVtx);
  fListOfHits.Add(hEventsSelectedNSD);
  
  
  fListOfPrimaries.Add(hEventsAll);
  fListOfPrimaries.Add(hEventsAllNSD);
  
  for(Int_t v=0; v<fNvtxBins;v++) {
    
    for(Int_t iring = 0; iring<2;iring++) {
      Char_t ringChar = (iring == 0 ? 'I' : 'O');
      Int_t nSec = (iring == 1 ? 40 : 20);
      TH2F* hAnalysed       = new TH2F(Form("Analysed_FMD%c_vtx%d",ringChar,v),
				       Form("Analysed_FMD%c_vtx%d",ringChar,v),
				       fNbinsEta, -4,6, nSec, 0,2*TMath::Pi());
      
      hAnalysed->Sumw2();
      fListOfPrimaries.Add(hAnalysed);
      
      TH2F* hAnalysedNSD       = new TH2F(Form("AnalysedNSD_FMD%c_vtx%d",ringChar,v),
					  Form("AnalysedNSD_FMD%c_vtx%d",ringChar,v),
					  fNbinsEta, -4,6, nSec, 0,2*TMath::Pi());
      
      hAnalysedNSD->Sumw2();
      fListOfPrimaries.Add(hAnalysedNSD);
      
      TH2F* hInel           = new TH2F(Form("Inel_FMD%c_vtx%d",ringChar,v),
				       Form("Inel_FMD%c_vtx%d",ringChar,v),
				       fNbinsEta, -4,6, nSec, 0,2*TMath::Pi());
      
      hInel->Sumw2();
      fListOfPrimaries.Add(hInel);
      
      TH2F* hNSD           = new TH2F(Form("NSD_FMD%c_vtx%d",ringChar,v),
				      Form("NSD_FMD%c_vtx%d",ringChar,v),
				      fNbinsEta, -4,6, nSec, 0,2*TMath::Pi());
      
      hNSD->Sumw2();
      fListOfPrimaries.Add(hNSD);
      
    }
  }
  
  fVertexBins.SetName("VertexBins");
  fVertexBins.GetXaxis()->Set(fNvtxBins,-1*fZvtxCut,fZvtxCut);
  
}
//_____________________________________________________________________
void AliFMDAnalysisTaskGenerateCorrection::Init()
{
  fLastTrackByStrip.Reset(-1);
  
  
}
//_____________________________________________________________________
void AliFMDAnalysisTaskGenerateCorrection::UserExec(Option_t */*option*/)
{
  
  fLastTrackByStrip.Reset(-1);
  fHitsByStrip.Reset(0);
  AliMCEvent* mcevent = MCEvent();
  
  AliFMDAnaParameters* pars = AliFMDAnaParameters::Instance();
  
  
  AliESDEvent* esdevent = (AliESDEvent*)InputEvent();
  
  pars->SetTriggerStatus(esdevent);
  
  Double_t esdvertex[3];
  Bool_t vtxStatus =  pars->GetVertex(esdevent,esdvertex);
  
  AliMCParticle* particle = 0;
  AliStack* stack = mcevent->Stack();
  
  UShort_t det,sec,strip;
  Char_t   ring;
  
  Int_t nTracks                = mcevent->GetNumberOfTracks();
  AliHeader* header            = mcevent->Header();
  AliGenEventHeader* genHeader = header->GenEventHeader();
  
  AliGenPythiaEventHeader* pythiaGenHeader = dynamic_cast<AliGenPythiaEventHeader*>(genHeader);
  if (!pythiaGenHeader) {
    std::cout<<" no pythia header! - NSD selection unusable"<<std::endl;
    //return; 
  }
  Bool_t nsd = kTRUE;
  if(pythiaGenHeader) {
    Int_t pythiaType = pythiaGenHeader->ProcessType();
    
    if(pythiaType==92 || pythiaType==93)
      nsd = kFALSE;
  }
  //if(pythiaType==94){
  //  std::cout<<"double diffractive"<<std::endl;
  //  return;
  //}
  
  TArrayF vertex;
  genHeader->PrimaryVertex(vertex);
  
  TH1F* hXvtx = (TH1F*)fListOfPrimaries.FindObject("hXvtx");
  hXvtx->Fill(vertex.At(0));
  TH1F* hYvtx = (TH1F*)fListOfPrimaries.FindObject("hYvtx");
  hYvtx->Fill(vertex.At(1));
  TH1F* hZvtx = (TH1F*)fListOfPrimaries.FindObject("hZvtx");
  hZvtx->Fill(vertex.At(2));

  
  
  
  Double_t delta           = 2*fZvtxCut/fNvtxBins;
  Double_t vertexBinDouble = (vertex.At(2) + fZvtxCut) / delta;
  Int_t    vertexBin       = (Int_t)vertexBinDouble;
  
  // Vertex determination correction
  TH1F* hEventsSelected           = (TH1F*)fListOfHits.FindObject("EventsSelected");
  TH1F* hEventsSelectedVtx        = (TH1F*)fListOfHits.FindObject("EventsSelectedVtx");
  TH1F* hEventsSelectedTrigger    = (TH1F*)fListOfHits.FindObject("EventsSelectedTrigger");
  TH1F* hEventsSelectedNSDVtx     = (TH1F*)fListOfHits.FindObject("EventsSelectedNSDVtx");
  TH1F* hEventsSelectedNSD        = (TH1F*)fListOfHits.FindObject("EventsSelectedNSD");
  TH1F* hEventsAll                = (TH1F*)fListOfPrimaries.FindObject("EventsAll");
  TH1F* hEventsAllNSD             = (TH1F*)fListOfPrimaries.FindObject("EventsAllNSD");
  
  // TH1F* hTriggered      = (TH1F*)fListOfHits.FindObject("Triggered");
  //  TH1F* hTriggeredAll   = (TH1F*)fListOfPrimaries.FindObject("TriggeredAll");
  
  Bool_t vtxFound = kTRUE;
  if(!vtxStatus)
    vtxFound = kFALSE;
  
  //if(TMath::Abs(vertex.At(2)) > fZvtxCut) {
  //  vtxFound = kFALSE;
    
  //}
  Bool_t isTriggered    = pars->IsEventTriggered(AliFMDAnaParameters::kMB1);
  Bool_t isTriggeredNSD = pars->IsEventTriggered(AliFMDAnaParameters::kNSD);
  if(vtxFound && isTriggered) hEventsSelected->Fill(vertexBin);
  
  if(vtxFound) hEventsSelectedVtx->Fill(vertexBin);
  if(isTriggered) hEventsSelectedTrigger->Fill(vertexBin);

  if(vtxFound && isTriggeredNSD) hEventsSelectedNSDVtx->Fill(vertexBin);
  if(isTriggeredNSD) hEventsSelectedNSD->Fill(vertexBin);

  
  hEventsAll->Fill(vertexBin);
  if(nsd) hEventsAllNSD->Fill(vertexBin);
  
  //  if(!vtxFound || !isTriggered) return;
  
  if(TMath::Abs(vertex.At(2)) > fZvtxCut) {
    return;
  }
  
  for(Int_t i = 0 ;i<nTracks;i++) {
    particle = (AliMCParticle*) mcevent->GetTrack(i);
    
    if(!particle)
      continue;
        
    if(stack->IsPhysicalPrimary(i) && particle->Charge() != 0) {
      
      
      TH2F* hPrimaryInner = (TH2F*)fListOfPrimaries.FindObject( Form("hPrimary_FMD_%c_vtx%d",'I',vertexBin));
      TH2F* hPrimaryOuter = (TH2F*)fListOfPrimaries.FindObject( Form("hPrimary_FMD_%c_vtx%d",'O',vertexBin));
      hPrimaryInner->Fill(particle->Eta(),particle->Phi());
      hPrimaryOuter->Fill(particle->Eta(),particle->Phi());
      TH2F* hAnalysedInner = (TH2F*)fListOfPrimaries.FindObject( Form("Analysed_FMD%c_vtx%d",'I',vertexBin));
      TH2F* hAnalysedOuter = (TH2F*)fListOfPrimaries.FindObject( Form("Analysed_FMD%c_vtx%d",'O',vertexBin));
      TH2F* hAnalysedNSDInner = (TH2F*)fListOfPrimaries.FindObject( Form("AnalysedNSD_FMD%c_vtx%d",'I',vertexBin));
      TH2F* hAnalysedNSDOuter = (TH2F*)fListOfPrimaries.FindObject( Form("AnalysedNSD_FMD%c_vtx%d",'O',vertexBin));
      TH2F* hInelInner = (TH2F*)fListOfPrimaries.FindObject( Form("Inel_FMD%c_vtx%d",'I',vertexBin));
      TH2F* hInelOuter = (TH2F*)fListOfPrimaries.FindObject( Form("Inel_FMD%c_vtx%d",'O',vertexBin));
      TH2F* hNSDInner = (TH2F*)fListOfPrimaries.FindObject( Form("NSD_FMD%c_vtx%d",'I',vertexBin));
      TH2F* hNSDOuter = (TH2F*)fListOfPrimaries.FindObject( Form("NSD_FMD%c_vtx%d",'O',vertexBin));
      
      //if(vtxFound && isTriggered) {
      if(isTriggeredNSD) {
	hAnalysedNSDInner->Fill(particle->Eta(),particle->Phi());
	hAnalysedNSDOuter->Fill(particle->Eta(),particle->Phi());
      }
      if(isTriggered) {
	hAnalysedInner->Fill(particle->Eta(),particle->Phi());
	hAnalysedOuter->Fill(particle->Eta(),particle->Phi());
      }
      hInelInner->Fill(particle->Eta(),particle->Phi());
      hInelOuter->Fill(particle->Eta(),particle->Phi());
      
      if(nsd) {
	hNSDInner->Fill(particle->Eta(),particle->Phi());
	hNSDOuter->Fill(particle->Eta(),particle->Phi());
      }
    }
    
    for(Int_t j=0; j<particle->GetNumberOfTrackReferences();j++) {
      
      AliTrackReference* ref = particle->GetTrackReference(j);
      
      if(ref->DetectorId() != AliTrackReference::kFMD)
	continue;

      AliFMDStripIndex::Unpack(ref->UserId(),det,ring,sec,strip);
      Float_t thisStripTrack = fLastTrackByStrip(det,ring,sec,strip);
      if(particle->Charge() != 0 && i != thisStripTrack ) {
	
	Float_t phi = pars->GetPhiFromSector(det,ring,sec);
	Float_t eta = pars->GetEtaFromStrip(det,ring,sec,strip,vertex.At(2));
	
	TH2F* hHits = (TH2F*)fListOfHits.FindObject(Form("hHits_FMD%d%c_vtx%d", det,ring,vertexBin));
	hHits->Fill(eta,phi);
	Float_t nstrips = (ring =='O' ? 256 : 512);
	fHitsByStrip(det,ring,sec,strip) +=1;
	TH1F* allHits = (TH1F*)fListOfHits.FindObject(Form("allHits_FMD%d%c",det,ring));
	TH1F* doubleHits = (TH1F*)fListOfHits.FindObject(Form("DoubleHits_FMD%d%c",det,ring));
	
	if(fHitsByStrip(det,ring,sec,strip) == 1)
	  allHits->Fill(eta);
	
	doubleHits->Fill(eta);
		
	fLastTrackByStrip(det,ring,sec,strip) = (Float_t)i;
	if(strip >0)
	  fLastTrackByStrip(det,ring,sec,strip-1) = (Float_t)i;
	if(strip < (nstrips - 1))
	  fLastTrackByStrip(det,ring,sec,strip+1) = (Float_t)i;
      }
    }

  }
  
  //SPD part HHD
  TH2F* hSPDMult = (TH2F*)fListOfHits.FindObject(Form("hSPDhits_vtx%d", vertexBin));
  
  const AliMultiplicity* spdmult = esdevent->GetMultiplicity();
  for(Int_t j = 0; j< spdmult->GetNumberOfTracklets();j++) 
    hSPDMult->Fill(spdmult->GetEta(j),spdmult->GetPhi(j));
  
  for(Int_t j = 0; j< spdmult->GetNumberOfSingleClusters();j++) 
    hSPDMult->Fill(-TMath::Log(TMath::Tan(spdmult->GetThetaSingle(j)/2.)),spdmult->GetPhiSingle(j));
    
  	
  PostData(1, &fListOfHits);
  PostData(2, &fListOfPrimaries);
  PostData(3, &fVertexBins);
}
//_____________________________________________________________________
void AliFMDAnalysisTaskGenerateCorrection::Terminate(Option_t */*option*/)
{
  /*  TH1F* allHits = (TH1F*)fListOfHits.FindObject("allHits");
  TH1F* doubleHits = (TH1F*)fListOfHits.FindObject("DoubleHits");
  
  doubleHits->Divide(allHits);
  GenerateCorrection();
  PostData(1, &fListOfHits);
  PostData(4, &fListOfCorrection);*/
  
}
//_____________________________________________________________________
void AliFMDAnalysisTaskGenerateCorrection::GenerateCorrection() {
  
  fBackground         = new AliFMDAnaCalibBackgroundCorrection();
  fEventSelectionEff  = new AliFMDAnaCalibEventSelectionEfficiency();
  
  //Event selection
  TH1F* hEventsSelected           = (TH1F*)fListOfHits.FindObject("EventsSelected");
  TH1F* hEventsSelectedVtx        = (TH1F*)fListOfHits.FindObject("EventsSelectedVtx");
  TH1F* hEventsSelectedTrigger    = (TH1F*)fListOfHits.FindObject("EventsSelectedTrigger");
  TH1F* hEventsSelectedNSDVtx     = (TH1F*)fListOfHits.FindObject("EventsSelectedNSDVtx");
  TH1F* hEventsSelectedNSD        = (TH1F*)fListOfHits.FindObject("EventsSelectedNSD");
    
  TH1F* hEventsAll                = (TH1F*)fListOfPrimaries.FindObject("EventsAll");
  TH1F* hEventsAllNSD             = (TH1F*)fListOfPrimaries.FindObject("EventsAllNSD");
  TH1F* hEventsSelectedVtxDivByTr = (TH1F*)hEventsSelectedVtx->Clone("hEventsSelectedVtxDivByTr");
  TH1F* hEventsSelectedNSDVtxDivByNSD = (TH1F*)hEventsSelectedNSDVtx->Clone("hEventsSelectedNSDVtxDivByNSD");
  
  fListOfHits.Add(hEventsSelectedVtxDivByTr);
  fListOfHits.Add(hEventsSelectedNSDVtxDivByNSD);
  hEventsSelectedNSDVtxDivByNSD->Divide(hEventsSelectedNSD);
  hEventsSelectedVtxDivByTr->Divide(hEventsSelectedTrigger);
  
  for(Int_t v=0;v<fNvtxBins ; v++) {
    TH2F* hAnalysedInner = (TH2F*)fListOfPrimaries.FindObject(Form("Analysed_FMD%c_vtx%d",'I',v));
    TH2F* hAnalysedOuter = (TH2F*)fListOfPrimaries.FindObject(Form("Analysed_FMD%c_vtx%d",'O',v));
    TH2F* hAnalysedNSDInner = (TH2F*)fListOfPrimaries.FindObject( Form("AnalysedNSD_FMD%c_vtx%d",'I',v));
    TH2F* hAnalysedNSDOuter = (TH2F*)fListOfPrimaries.FindObject( Form("AnalysedNSD_FMD%c_vtx%d",'O',v));
    
    TH2F* hInelInner = (TH2F*)fListOfPrimaries.FindObject(Form("Inel_FMD%c_vtx%d",'I',v));
    TH2F* hInelOuter = (TH2F*)fListOfPrimaries.FindObject(Form("Inel_FMD%c_vtx%d",'O',v));
    TH2F* hNSDInner  = (TH2F*)fListOfPrimaries.FindObject( Form("NSD_FMD%c_vtx%d",'I',v));
    TH2F* hNSDOuter  = (TH2F*)fListOfPrimaries.FindObject( Form("NSD_FMD%c_vtx%d",'O',v));
    // hAnalysedInner->Scale((hEventsSelected->GetBinContent(v+1) == 0 ? 0 : 1/hEventsSelected->GetBinContent(v+1))); 
    //hAnalysedOuter->Scale((hEventsSelected->GetBinContent(v+1) == 0 ? 0 : 1/hEventsSelected->GetBinContent(v+1))); 

    hAnalysedInner->Scale((hEventsSelectedTrigger->GetBinContent(v+1) == 0 ? 0 : 1/hEventsSelectedTrigger->GetBinContent(v+1))); 
    
    hAnalysedOuter->Scale((hEventsSelectedTrigger->GetBinContent(v+1) == 0 ? 0 : 1/hEventsSelectedTrigger->GetBinContent(v+1))); 
    hAnalysedNSDInner->Scale((hEventsSelectedNSD->GetBinContent(v+1) == 0 ? 0 : 1/hEventsSelectedNSD->GetBinContent(v+1))); 
    
    hAnalysedNSDOuter->Scale((hEventsSelectedNSD->GetBinContent(v+1) == 0 ? 0 : 1/hEventsSelectedNSD->GetBinContent(v+1))); 
    
    

    hInelInner->Scale((hEventsAll->GetBinContent(v+1) == 0 ? 0 : 1/hEventsAll->GetBinContent(v+1))); 
    hInelOuter->Scale((hEventsAll->GetBinContent(v+1) == 0 ? 0 : 1/hEventsAll->GetBinContent(v+1)));
    
    hNSDInner->Scale((hEventsAllNSD->GetBinContent(v+1) == 0 ? 0 : 1/hEventsAllNSD->GetBinContent(v+1))); 
    hNSDOuter->Scale((hEventsAllNSD->GetBinContent(v+1) == 0 ? 0 : 1/hEventsAllNSD->GetBinContent(v+1)));
    

    
    //hAnalysedInner->Scale(1./0.84);  //numbers for real data, run104892
    //hAnalysedOuter->Scale(1./0.84);
    hAnalysedInner->Divide(hInelInner);
    hAnalysedOuter->Divide(hInelOuter);
    
    hAnalysedNSDInner->Divide(hNSDInner);
    hAnalysedNSDOuter->Divide(hNSDOuter);
    
    fEventSelectionEff->SetCorrection("INEL",v,'I',hAnalysedInner);
    fEventSelectionEff->SetCorrection("INEL",v,'O',hAnalysedOuter);
    //NSD
    fEventSelectionEff->SetCorrection("NSD",v,'I',hAnalysedNSDInner);
    fEventSelectionEff->SetCorrection("NSD",v,'O',hAnalysedNSDOuter);
    
    
}
  
  Float_t vtxEff = 1;
  if(hEventsSelectedTrigger->GetEntries())
    vtxEff = hEventsSelectedVtx->GetEntries() / hEventsSelectedTrigger->GetEntries();
  fEventSelectionEff->SetVtxToTriggerRatio(vtxEff);
  
  //  hEventsAll->Divide(hEventsAll,hEventsSelected,1,1,"B");
  hEventsSelectedVtx->Divide(hEventsAll);
  hEventsSelectedTrigger->Divide(hEventsAll);
  
  hEventsSelectedNSDVtx->Divide(hEventsAllNSD);
  hEventsSelectedNSD->Divide(hEventsAllNSD);
  
  for(Int_t i = 1; i<=hEventsSelected->GetNbinsX(); i++) {
    if(hEventsSelected->GetBinContent(i) == 0 )
      continue;
    Float_t b    = hEventsSelected->GetBinContent(i);
    // Float_t b    = hEventsSelected->GetBinContent(i)*hEventsSelectedTrigger->GetBinContent(i);
    Float_t db   = hEventsSelected->GetBinError(i);
    Float_t sum  = hEventsAll->GetBinContent(i);
    Float_t dsum = hEventsAll->GetBinError(i);
    Float_t a    = sum-b;
    Float_t da   = TMath::Sqrt(TMath::Power(db,2) + TMath::Power(dsum,2));
    
    Float_t cor  = sum / b;
    Float_t ecor = TMath::Sqrt(TMath::Power(da,2) + TMath::Power(a/(b*db),2)) / b;
    
    hEventsAll->SetBinContent(i,cor);
    hEventsAll->SetBinError(i,ecor);
    
  }
  
  //TH1F* hEventTest = (TH1F*)hEventsAll->Clone("hEventTest");
  
  
  
  fEventSelectionEff->SetCorrection(hEventsAll);
  
  for(Int_t det= 1; det <=3; det++) {
    Int_t nRings = (det==1 ? 1 : 2);
    
    for(Int_t iring = 0; iring<nRings; iring++) {
      Char_t ring = (iring == 0 ? 'I' : 'O');
      TH1F* allHits = (TH1F*)fListOfHits.FindObject(Form("allHits_FMD%d%c",det,ring));
      TH1F* doubleHits = (TH1F*)fListOfHits.FindObject(Form("DoubleHits_FMD%d%c",det,ring));
      allHits->Divide(doubleHits);
      
      fBackground->SetDoubleHitCorrection(det,ring,allHits);
      
      for(Int_t vertexBin=0;vertexBin<fNvtxBins  ;vertexBin++) {
	TH2F* hHits          = (TH2F*)fListOfHits.FindObject(Form("hHits_FMD%d%c_vtx%d", det,ring,vertexBin));
	TH2F* hPrimary  = (TH2F*)fListOfPrimaries.FindObject( Form("hPrimary_FMD_%c_vtx%d",ring,vertexBin));
	TH2F* hCorrection = (TH2F*)hHits->Clone(Form("FMD%d%c_vtxbin_%d_correction",det,ring,vertexBin));
	hCorrection->Divide(hPrimary);

	
	hCorrection->SetTitle(hCorrection->GetName());
	fListOfCorrection.Add(hCorrection);
	fBackground->SetBgCorrection(det,ring,vertexBin,hCorrection);
	
	
      }
      
    }
  }
  for(Int_t vertexBin=0;vertexBin<fNvtxBins  ;vertexBin++) {
    TH2F* hPrimary  = (TH2F*)fListOfPrimaries.FindObject( Form("hPrimary_FMD_%c_vtx%d",'I',vertexBin));
    TH2F* hSPDMult = (TH2F*)fListOfHits.FindObject(Form("hSPDhits_vtx%d", vertexBin));
    if(!hSPDMult) continue;
    
    TH2F* hCorrection = (TH2F*)hSPDMult->Clone(Form("SPD_vtxbin_%d_correction",vertexBin));
    hCorrection->SetTitle(hCorrection->GetName());
    fListOfCorrection.Add(hCorrection);
    hCorrection->Divide(hPrimary);
    fBackground->SetBgCorrection(0,'Q',vertexBin,hCorrection);
    
    TH1F* hAlive = new TH1F(Form("hAliveSPD_vtxbin%d",vertexBin),Form("hAliveSPD_vtxbin%d",vertexBin),hSPDMult->GetNbinsX(),hSPDMult->GetXaxis()->GetXmin(), hSPDMult->GetXaxis()->GetXmax());
    TH1F* hPresent = new TH1F(Form("hPresentSPD_vtxbin%d",vertexBin),Form("hPresentSPD_vtxbin%d",vertexBin),hSPDMult->GetNbinsX(),hSPDMult->GetXaxis()->GetXmin(), hSPDMult->GetXaxis()->GetXmax());
    for(Int_t xx = 1; xx <=hSPDMult->GetNbinsX(); xx++) {
      
      if(TMath::Abs(hCorrection->GetXaxis()->GetBinCenter(xx)) > 2)
	continue;
      for(Int_t yy = 1; yy <=hSPDMult->GetNbinsY(); yy++) {
	if(hCorrection->GetBinContent(xx,yy) > 0.1)
	  hAlive->Fill(hCorrection->GetXaxis()->GetBinCenter(xx));
	hPresent->Fill(hCorrection->GetXaxis()->GetBinCenter(xx));
	
      }
    }
    TH1F* hDeadCorrection = (TH1F*)hAlive->Clone(Form("hSPDDeadCorrection_vtxbin%d",vertexBin));
    hDeadCorrection->Divide(hPresent);
    fBackground->SetSPDDeadCorrection(vertexBin,hDeadCorrection);
    fListOfCorrection.Add(hDeadCorrection);
  }
  
  
  
  
  
  
  TAxis refAxis(fNvtxBins,-1*fZvtxCut,fZvtxCut);
  fBackground->SetRefAxis(&refAxis);

}
//_____________________________________________________________________
void AliFMDAnalysisTaskGenerateCorrection::ReadFromFile(const Char_t* filename, Bool_t storeInOCDB, Int_t /*runNo*/) {
  
  TFile infile(filename);
  TH1F* hVertex = (TH1F*)infile.Get("VertexBins");
  fZvtxCut = hVertex->GetXaxis()->GetXmax();
  fNvtxBins = hVertex->GetXaxis()->GetNbins();
  fVertexBins.SetName("VertexBins");
  fVertexBins.GetXaxis()->Set(fNvtxBins,-1*fZvtxCut,fZvtxCut);
  
  TList* listOfHits = (TList*)infile.Get("Hits");
  TList* listOfPrim = (TList*)infile.Get("Primaries");
  
  TH1F* hEventsSelected           = (TH1F*)listOfHits->FindObject("EventsSelected");
  TH1F* hEventsSelectedVtx        = (TH1F*)listOfHits->FindObject("EventsSelectedVtx");
  TH1F* hEventsSelectedTrigger    = (TH1F*)listOfHits->FindObject("EventsSelectedTrigger");
  TH1F* hEventsSelectedNSDVtx     = (TH1F*)listOfHits->FindObject("EventsSelectedNSDVtx");
  TH1F* hEventsSelectedNSD        = (TH1F*)listOfHits->FindObject("EventsSelectedNSD");  
  TH1F* hEventsAll                = (TH1F*)listOfPrim->FindObject("EventsAll");
  TH1F* hEventsAllNSD             = (TH1F*)listOfPrim->FindObject("EventsAllNSD");
  
  fListOfHits.Add(hEventsSelected);
  fListOfHits.Add(hEventsSelectedVtx);
  fListOfHits.Add(hEventsSelectedNSD);
  fListOfHits.Add(hEventsSelectedNSDVtx);
  fListOfHits.Add(hEventsSelectedTrigger);
  fListOfPrimaries.Add(hEventsAll);
  fListOfPrimaries.Add(hEventsAllNSD);
  
  TH1F* hXvtx = (TH1F*)listOfPrim->FindObject("hXvtx");
  TH1F* hYvtx = (TH1F*)listOfPrim->FindObject("hYvtx");
  TH1F* hZvtx = (TH1F*)listOfPrim->FindObject("hZvtx");
  fListOfPrimaries.Add(hXvtx);
  fListOfPrimaries.Add(hYvtx);
  fListOfPrimaries.Add(hZvtx);  

  for(Int_t det =1; det<=3;det++)
    {
      Int_t nRings = (det==1 ? 1 : 2);
      for(Int_t ring = 0;ring<nRings;ring++)
	{
	  Char_t ringChar = (ring == 0 ? 'I' : 'O');
	  TH1F* allHits = (TH1F*)listOfHits->FindObject(Form("allHits_FMD%d%c",det,ringChar));
	  TH1F* doubleHits = (TH1F*)listOfHits->FindObject(Form("DoubleHits_FMD%d%c",det,ringChar));
	  fListOfHits.Add(allHits);
	  fListOfHits.Add(doubleHits);
	  for(Int_t v=0; v<fNvtxBins;v++)
	    {
	      
	      TH2F* hHits          = (TH2F*)listOfHits->FindObject(Form("hHits_FMD%d%c_vtx%d", det,ringChar,v));
	      fListOfHits.Add(hHits);
	    }
	}
    }
  for(Int_t v=0; v<fNvtxBins;v++) {
    TH2F* hSPDHits          = (TH2F*)listOfHits->FindObject(Form("hSPDhits_vtx%d", v));   
    fListOfHits.Add(hSPDHits);
    
    for(Int_t iring = 0; iring<2;iring++) {
      Char_t ringChar = (iring == 0 ? 'I' : 'O');
      
      
      TH2F* hPrimary       = (TH2F*)listOfPrim->FindObject( Form("hPrimary_FMD_%c_vtx%d",ringChar,v));
      TH2F* hAnalysed      = (TH2F*)listOfPrim->FindObject(Form("Analysed_FMD%c_vtx%d",ringChar,v));
      TH2F* hAnalysedNSD   = (TH2F*)listOfPrim->FindObject(Form("AnalysedNSD_FMD%c_vtx%d",ringChar,v));
      TH2F* hInel          = (TH2F*)listOfPrim->FindObject(Form("Inel_FMD%c_vtx%d",ringChar,v));
      TH2F* hNSD           = (TH2F*)listOfPrim->FindObject(Form("NSD_FMD%c_vtx%d",ringChar,v));
      
      fListOfPrimaries.Add(hPrimary);      
      fListOfPrimaries.Add(hAnalysed);
      fListOfPrimaries.Add(hInel);      
      fListOfPrimaries.Add(hAnalysedNSD);
      fListOfPrimaries.Add(hNSD);      
    }
  }
  GenerateCorrection();
  
  TFile fout("backgroundFromFile.root","recreate");
  fListOfHits.Write();
  fListOfPrimaries.Write();
  fListOfCorrection.Write();
  fVertexBins.Write();
  
  fout.Close();
  AliFMDAnaParameters* pars = AliFMDAnaParameters::Instance();
  if(storeInOCDB) {
    TFile fbg(pars->GetPath(pars->GetBackgroundID()),"RECREATE");
    fBackground->Write(AliFMDAnaParameters::GetBackgroundID());
    fbg.Close();
    TFile feselect(pars->GetPath(pars->GetEventSelectionEffID()),"RECREATE");
    fEventSelectionEff->Write(AliFMDAnaParameters::GetEventSelectionEffID());
    feselect.Close();
    
  }
  
  
  
  
}
//_____________________________________________________________________
//
// EOF
//
