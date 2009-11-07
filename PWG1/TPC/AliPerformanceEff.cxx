//------------------------------------------------------------------------------
// Implementation of AliPerformanceEff class. It keeps information from 
// comparison of reconstructed and MC particle tracks. In addtion, 
// it keeps selection cuts used during comparison. The comparison 
// information is stored in the ROOT histograms. Analysis of these 
// histograms can be done by using Analyse() class function. The result of 
// the analysis (histograms/graphs) are stored in the folder which is 
// a data member of AliPerformanceEff.
// 
// Author: J.Otwinowski 04/02/2008 
//------------------------------------------------------------------------------

/*
 
  // after running comparison task, read the file, and get component
  gROOT->LoadMacro("$ALICE_ROOT/PWG1/Macros/LoadMyLibs.C");
  LoadMyLibs();
  TFile f("Output.root");
  AliPerformanceEff * compObj = (AliPerformanceEff*)coutput->FindObject("AliPerformanceEff");

  // Analyse comparison data
  compObj->Analyse();

  // the output histograms/graphs will be stored in the folder "folderEff" 
  compObj->GetAnalysisFolder()->ls("*");

  // user can save whole comparison object (or only folder with anlysed histograms) 
  // in the seperate output file (e.g.)
  TFile fout("Analysed_Eff.root","recreate");
  compObj->Write(); // compObj->GetAnalysisFolder()->Write();
  fout.Close();

*/

#include <TAxis.h>
#include <TH1D.h>
#include "THnSparse.h"

// 
#include "AliESDtrack.h"
#include "AliRecInfoCuts.h" 
#include "AliMCInfoCuts.h" 
#include "AliLog.h" 
#include "AliESDVertex.h" 
#include "AliExternalTrackParam.h" 
#include "AliTracker.h" 
#include "AliESDEvent.h" 
#include "AliMCEvent.h" 
#include "AliMCParticle.h" 
#include "AliHeader.h" 
#include "AliGenEventHeader.h" 
#include "AliStack.h" 
#include "AliPerformanceEff.h" 

using namespace std;

ClassImp(AliPerformanceEff)

//_____________________________________________________________________________
AliPerformanceEff::AliPerformanceEff():
  AliPerformanceObject("AliPerformanceEff"),

  // histograms
  fEffHisto(0),
  fEffSecHisto(0),

  // Cuts 
  fCutsRC(0), 
  fCutsMC(0),

  // histogram folder 
  fAnalysisFolder(0)
{
  // default consttructor	
  Init();
}

//_____________________________________________________________________________
AliPerformanceEff::AliPerformanceEff(Char_t* name="AliPerformanceEff",Char_t*title="AliPerformanceEff",Int_t analysisMode=0, Bool_t hptGenerator=kFALSE):
  AliPerformanceObject(name,title),

  // histograms
  fEffHisto(0),
  fEffSecHisto(0),

  // Cuts 
  fCutsRC(0), 
  fCutsMC(0),

  // histogram folder 
  fAnalysisFolder(0)
{
  // named constructor
  //
  SetAnalysisMode(analysisMode);
  SetHptGenerator(hptGenerator);

  Init();
}


//_____________________________________________________________________________
AliPerformanceEff::~AliPerformanceEff()
{
// destructor

  if(fEffHisto)  delete  fEffHisto; fEffHisto=0;
  if(fEffSecHisto)  delete  fEffSecHisto; fEffSecHisto=0;
  if(fAnalysisFolder) delete fAnalysisFolder; fAnalysisFolder=0;
}

//_____________________________________________________________________________
void AliPerformanceEff::Init()
{
  // Init histograms
  //
  // set pt bins
  Int_t nPtBins = 50;
  Double_t ptMin = 1.e-2, ptMax = 10.;

  Double_t *binsPt = 0;
  if (IsHptGenerator())  { 
    nPtBins = 100; ptMax = 100.;
    binsPt = CreateLogAxis(nPtBins,ptMin,ptMax);
  } else {
    binsPt = CreateLogAxis(nPtBins,ptMin,ptMax);
  }

  /*
  Int_t nPtBins = 31;
  Double_t binsPt[32] = {0.,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.5,0.55,0.6,0.7,0.8,0.9,1.0,1.2,1.4,1.6,1.8,2.0,2.25,2.5,2.75,3.,3.5,4.,5.,6.,8.,10.};
  Double_t ptMin = 0., ptMax = 10.;

  if(IsHptGenerator() == kTRUE) {
    nPtBins = 100;
    ptMin = 0.; ptMax = 100.;
  }
  */

  //mceta:mcphi:mcpt:pid:recStatus:findable
  Int_t binsEffHisto[6]={30,90,nPtBins,5,2,2};
  Double_t minEffHisto[6]={-1.5,0.,ptMin,0.,0.,0.};
  Double_t maxEffHisto[6]={ 1.5,2.*TMath::Pi(), ptMax,5.,2.,2.};

  fEffHisto = new THnSparseF("fEffHisto","mceta:mcphi:mcpt:pid:recStatus:findable",6,binsEffHisto,minEffHisto,maxEffHisto);
  fEffHisto->SetBinEdges(2,binsPt);

  fEffHisto->GetAxis(0)->SetTitle("#eta_{mc}");
  fEffHisto->GetAxis(1)->SetTitle("#phi_{mc} (rad)");
  fEffHisto->GetAxis(2)->SetTitle("p_{Tmc} (GeV/c)");
  fEffHisto->GetAxis(3)->SetTitle("pid");
  fEffHisto->GetAxis(4)->SetTitle("recStatus");
  fEffHisto->GetAxis(5)->SetTitle("findable");
  fEffHisto->Sumw2();

  //mceta:mcphi:mcpt:pid:recStatus:findable:mcR:mother_phi:mother_eta
  Int_t binsEffSecHisto[9]={30,60,nPtBins,5,2,2,100,60,30};
  Double_t minEffSecHisto[9]={-1.5,0.,ptMin,0.,0.,0.,0.,0.,-1.5};
  Double_t maxEffSecHisto[9]={ 1.5,2.*TMath::Pi(), ptMax,5.,2.,2.,200,2.*TMath::Pi(),1.5};

  fEffSecHisto = new THnSparseF("fEffSecHisto","mceta:mcphi:mcpt:pid:recStatus:findable:mcR:mother_phi:mother_eta",9,binsEffSecHisto,minEffSecHisto,maxEffSecHisto);
  fEffSecHisto->SetBinEdges(2,binsPt);

  fEffSecHisto->GetAxis(0)->SetTitle("#eta_{mc}");
  fEffSecHisto->GetAxis(1)->SetTitle("#phi_{mc} (rad)");
  fEffSecHisto->GetAxis(2)->SetTitle("p_{Tmc} (GeV/c)");
  fEffSecHisto->GetAxis(3)->SetTitle("pid");
  fEffSecHisto->GetAxis(4)->SetTitle("recStatus");
  fEffSecHisto->GetAxis(5)->SetTitle("findable");
  fEffSecHisto->GetAxis(6)->SetTitle("mcR (cm)");
  fEffSecHisto->GetAxis(7)->SetTitle("mother_phi (rad)");
  fEffSecHisto->GetAxis(8)->SetTitle("mother_eta");
  fEffSecHisto->Sumw2();

  // init cuts
  if(!fCutsMC) 
    AliDebug(AliLog::kError, "ERROR: Cannot find AliMCInfoCuts object");
  if(!fCutsRC) 
    AliDebug(AliLog::kError, "ERROR: Cannot find AliRecInfoCuts object");

  // init folder
  fAnalysisFolder = CreateFolder("folderEff","Analysis Efficiency Folder");
}

//_____________________________________________________________________________
void AliPerformanceEff::ProcessTPC(AliMCEvent* const mcEvent, AliESDEvent *const esdEvent)
{
  // Fill TPC only efficiency comparison information 
  Int_t *labelsRec =  new Int_t[esdEvent->GetNumberOfTracks()];
  if(!labelsRec) 
     AliDebug(AliLog::kError, "Cannot create labelsRec");

  Int_t *labelsAllRec =  new Int_t[esdEvent->GetNumberOfTracks()];
  if(!labelsAllRec) 
     AliDebug(AliLog::kError, "Cannot create labelsAllRec");

  // loop over rec. tracks
  AliESDtrack *track=0;
  for (Int_t iTrack = 0; iTrack < esdEvent->GetNumberOfTracks(); iTrack++) 
  { 
    track = esdEvent->GetTrack(iTrack);
    if(!track) continue;
    if(track->Charge()==0) continue;
    Int_t label = TMath::Abs(track->GetLabel()); 
    labelsAllRec[iTrack]=label;

    // TPC only
    if(IsRecTPC(track) != 0) 
      labelsRec[iTrack]=label;
  }

  // 
  // MC histograms for efficiency studies
  //
  if(!mcEvent) return;
 
  AliStack *stack = mcEvent->Stack();
  if (!stack) {
    AliDebug(AliLog::kError, "Stack not available");
    return;
  }

  //Int_t nPart  = stack->GetNtrack();
  Int_t nPart  = stack->GetNprimary();
  for (Int_t iMc = 0; iMc < nPart; ++iMc) 
  {
    TParticle* particle = stack->Particle(iMc);
    if (!particle) continue;
    if (particle->GetPDG()->Charge() == 0.0) continue;
      
    // physical primary
    Bool_t prim = stack->IsPhysicalPrimary(iMc);
    if(!prim) continue;

    Bool_t findable = kFALSE;
    for(Int_t iRec=0; iRec<esdEvent->GetNumberOfTracks(); ++iRec) 
    {
      // check findable
      if(iMc == labelsAllRec[iRec]) 
      {
        findable = IsFindable(mcEvent,iMc);
	break;
      }
    }  

    Bool_t recStatus = kFALSE;
    for(Int_t iRec=0; iRec<esdEvent->GetNumberOfTracks(); ++iRec) 
    {
      // check reconstructed
      if(iMc == labelsRec[iRec]) 
      {
        recStatus = kTRUE;
        break;
      }
    }

    // Only 5 charged particle species (e,mu,pi,K,p)
    if (fCutsMC->IsPdgParticle(TMath::Abs(particle->GetPdgCode())) == kFALSE) continue; 

    // transform Pdg to Pid
    Int_t pid = TransformToPID(particle);

    Float_t mceta =  particle->Eta();
    Float_t mcphi =  particle->Phi();
    if(mcphi<0) mcphi += 2.*TMath::Pi();
    Float_t mcpt = particle->Pt();


    // Fill histograms
    Double_t vEffHisto[6] = {mceta, mcphi, mcpt, pid, recStatus, findable}; 
    fEffHisto->Fill(vEffHisto);
  }

  if(labelsRec) delete [] labelsRec; labelsRec = 0;
  if(labelsAllRec) delete [] labelsAllRec; labelsAllRec = 0;
}

//_____________________________________________________________________________
void AliPerformanceEff::ProcessTPCSec(AliMCEvent* const mcEvent, AliESDEvent *const esdEvent)
{
  // Fill TPC only efficiency comparison information for secondaries
  Int_t *labelsRec =  new Int_t[esdEvent->GetNumberOfTracks()];
  if(!labelsRec) 
     AliDebug(AliLog::kError, "Cannot create labelsRec");

  Int_t *labelsAllRec =  new Int_t[esdEvent->GetNumberOfTracks()];
  if(!labelsAllRec) 
     AliDebug(AliLog::kError, "Cannot create labelsAllRec");

  // loop over rec. tracks
  AliESDtrack *track=0;
  Int_t multAll=0, multRec=0;
  for (Int_t iTrack = 0; iTrack < esdEvent->GetNumberOfTracks(); iTrack++) 
  { 
    track = esdEvent->GetTrack(iTrack);
    if(!track) continue;
    if(track->Charge()==0) continue;
    Int_t label = TMath::Abs(track->GetLabel()); 
    labelsAllRec[multAll]=label;
    multAll++;

    // TPC only
    if(IsRecTPC(track) != 0) {
      labelsRec[multRec]=label;
      multRec++;
    }
  }

  // 
  // MC histograms for efficiency studies
  //
  if(!mcEvent) return;
 
  AliStack *stack = mcEvent->Stack();
  if (!stack) {
    AliDebug(AliLog::kError, "Stack not available");
    return;
  }

  Int_t nPart  = stack->GetNtrack();
  //Int_t nPart  = stack->GetNprimary();
  for (Int_t iMc = 0; iMc < nPart; ++iMc) 
  {
    TParticle* particle = stack->Particle(iMc);
    if (!particle) continue;
    if (particle->GetPDG()->Charge() == 0.0) continue;
      
    // physical primary
    Bool_t prim = stack->IsPhysicalPrimary(iMc);

    // only secondaries which can be reconstructed at TPC
    if(prim) continue;

    //Float_t radius = TMath::Sqrt(particle->Vx()*particle->Vx()+particle->Vy()*particle->Vy()+particle->Vz()*particle->Vz());
    //if(radius > fCutsMC->GetMaxR()) continue;

    // only secondary electrons from gamma conversion
    //if( TMath::Abs(particle->GetPdgCode())!=fCutsMC->GetEM() ||   particle->GetUniqueID() != 5) continue;

    Bool_t findable = kFALSE;
    for(Int_t iRec=0; iRec<multAll; ++iRec) 
    {
      // check findable
      if(iMc == labelsAllRec[iRec]) 
      {
        findable = IsFindable(mcEvent,iMc);
	break;
      }
    }  

    Bool_t recStatus = kFALSE;
    for(Int_t iRec=0; iRec<multRec; ++iRec) 
    {
      // check reconstructed
      if(iMc == labelsRec[iRec]) 
      {
        recStatus = kTRUE;
        break;
      }
    }

    // Only 5 charged particle species (e,mu,pi,K,p)
    if (fCutsMC->IsPdgParticle(TMath::Abs(particle->GetPdgCode())) == kFALSE) continue; 

    // transform Pdg to Pid
    Int_t pid = TransformToPID(particle);

    Float_t mceta =  particle->Eta();
    Float_t mcphi =  particle->Phi();
    if(mcphi<0) mcphi += 2.*TMath::Pi();
    Float_t mcpt = particle->Pt();
    Float_t mcR = particle->R();

    // get info about mother
    Int_t motherLabel = particle->GetMother(0);
    if(motherLabel < 0) continue;
    TParticle *mother = stack->Particle(motherLabel);
    if(!mother) continue; 

    Float_t mother_eta = mother->Eta();
    Float_t mother_phi = mother->Phi();
    if(mother_phi<0) mother_phi += 2.*TMath::Pi();

    // Fill histograms
    Double_t vEffSecHisto[9] = { mceta, mcphi, mcpt, pid, recStatus, findable, mcR, mother_phi, mother_eta }; 
    fEffSecHisto->Fill(vEffSecHisto);
  }

  if(labelsRec) delete [] labelsRec; labelsRec = 0;
  if(labelsAllRec) delete [] labelsAllRec; labelsAllRec = 0;
}




//_____________________________________________________________________________
void AliPerformanceEff::ProcessTPCITS(AliMCEvent* const mcEvent, AliESDEvent *const esdEvent)
{
  // Fill efficiency comparison information
  Int_t *labelsRec =  new Int_t[esdEvent->GetNumberOfTracks()];
  if(!labelsRec) 
     AliDebug(AliLog::kError, "Cannot create labelsRec");

  Int_t *labelsAllRec =  new Int_t[esdEvent->GetNumberOfTracks()];
  if(!labelsAllRec) 
     AliDebug(AliLog::kError, "Cannot create labelsAllRec");

  // loop over rec. tracks
  AliESDtrack *track=0;
  for (Int_t iTrack = 0; iTrack < esdEvent->GetNumberOfTracks(); iTrack++) 
  { 
    track = esdEvent->GetTrack(iTrack);
    if(!track) continue;
    if(track->Charge()==0) continue;
    Int_t label = TMath::Abs(track->GetLabel()); 
    labelsAllRec[iTrack]=label;

    // iTPC+ITS
    if(IsRecTPCITS(track) != 0) 
      labelsRec[iTrack]=label;
  }

  // 
  // MC histograms for efficiency studies
  //
  if(!mcEvent) return;
 
  AliStack *stack = mcEvent->Stack();
  if (!stack) {
    AliDebug(AliLog::kError, "Stack not available");
    return;
  }

  //Int_t nPart  = stack->GetNtrack();
  Int_t nPart  = stack->GetNprimary();
  for (Int_t iMc = 0; iMc < nPart; ++iMc) 
  {
    TParticle* particle = stack->Particle(iMc);
    if (!particle) continue;
    if (particle->GetPDG()->Charge() == 0.0) continue;
      
    // physical primary
    Bool_t prim = stack->IsPhysicalPrimary(iMc);
    if(!prim) continue;

    Bool_t findable = kFALSE;
    for(Int_t iRec=0; iRec<esdEvent->GetNumberOfTracks(); ++iRec) 
    {
      // check findable
      if(iMc == labelsAllRec[iRec]) 
      {
        findable = IsFindable(mcEvent,iMc);
	break;
      }
    }  

    Bool_t recStatus = kFALSE;
    for(Int_t iRec=0; iRec<esdEvent->GetNumberOfTracks(); ++iRec) 
    {
      // check reconstructed
      if(iMc == labelsRec[iRec]) 
      {
        recStatus = kTRUE;
        break;
      }
    }

    // Only 5 charged particle species (e,mu,pi,K,p)
    if (fCutsMC->IsPdgParticle(TMath::Abs(particle->GetPdgCode())) == kFALSE) continue; 

    // transform Pdg to Pid
    Int_t pid = TransformToPID(particle);

    Float_t mceta =  particle->Eta();
    Float_t mcphi =  particle->Phi();
    if(mcphi<0) mcphi += 2.*TMath::Pi();
    Float_t mcpt = particle->Pt();

    // Fill histograms
    Double_t vEffHisto[6] = { mceta, mcphi, mcpt, pid, recStatus, findable}; 
    fEffHisto->Fill(vEffHisto);
  }

  if(labelsRec) delete [] labelsRec; labelsRec = 0;
  if(labelsAllRec) delete [] labelsAllRec; labelsAllRec = 0;
}

//_____________________________________________________________________________
void AliPerformanceEff::ProcessConstrained(AliMCEvent* const mcEvent, AliESDEvent *const esdEvent)
{
  // Process comparison information 
  Int_t *labelsRec =  new Int_t[esdEvent->GetNumberOfTracks()];
  if(!labelsRec) 
     AliDebug(AliLog::kError, "Cannot create labelsRec");

  Int_t *labelsAllRec =  new Int_t[esdEvent->GetNumberOfTracks()];
  if(!labelsAllRec) 
     AliDebug(AliLog::kError, "Cannot create labelsAllRec");

  // loop over rec. tracks
  AliESDtrack *track=0;
  for (Int_t iTrack = 0; iTrack < esdEvent->GetNumberOfTracks(); iTrack++) 
  { 
    track = esdEvent->GetTrack(iTrack);
    if(!track) continue;
    if(track->Charge()==0) continue;
    Int_t label = TMath::Abs(track->GetLabel()); 
    labelsAllRec[iTrack]=label;

    // Constrained
    if(IsRecConstrained(track) != 0) 
      labelsRec[iTrack]=label;

  }

  // 
  // MC histograms for efficiency studies
  //
  if(!mcEvent) return;
 
  AliStack *stack = mcEvent->Stack();
  if (!stack) {
    AliDebug(AliLog::kError, "Stack not available");
    return;
  }

  //Int_t nPart  = stack->GetNtrack();
  Int_t nPart  = stack->GetNprimary();
  for (Int_t iMc = 0; iMc < nPart; ++iMc) 
  {
    TParticle* particle = stack->Particle(iMc);
    if (!particle) continue;
    if (particle->GetPDG()->Charge() == 0.0) continue;
      
    // physical primary
    Bool_t prim = stack->IsPhysicalPrimary(iMc);
    if(!prim) continue;

    Bool_t findable = kFALSE;
    for(Int_t iRec=0; iRec<esdEvent->GetNumberOfTracks(); ++iRec) 
    {
      // check findable
      if(iMc == labelsAllRec[iRec]) 
      {
        findable = IsFindable(mcEvent,iMc);
	break;
      }
    }  

    Bool_t recStatus = kFALSE;
    for(Int_t iRec=0; iRec<esdEvent->GetNumberOfTracks(); ++iRec) 
    {
      // check reconstructed
      if(iMc == labelsRec[iRec]) 
      {
        recStatus = kTRUE;
        break;
      }
    }

    // Only 5 charged particle species (e,mu,pi,K,p)
    if (fCutsMC->IsPdgParticle(TMath::Abs(particle->GetPdgCode())) == kFALSE) continue; 

    // transform Pdg to Pid
    Int_t pid = TransformToPID(particle);

    Float_t mceta =  particle->Eta();
    Float_t mcphi =  particle->Phi();
    if(mcphi<0) mcphi += 2.*TMath::Pi();
    Float_t mcpt = particle->Pt();

    // Fill histograms
    Double_t vEffHisto[6] = { mceta, mcphi, mcpt, pid, recStatus, findable}; 
    fEffHisto->Fill(vEffHisto);
  }

  if(labelsRec) delete [] labelsRec; labelsRec = 0;
  if(labelsAllRec) delete [] labelsAllRec; labelsAllRec = 0;
}

//_____________________________________________________________________________
void AliPerformanceEff::Exec(AliMCEvent* const mcEvent, AliESDEvent *const esdEvent, AliESDfriend *const esdFriend, const Bool_t bUseMC, const Bool_t bUseESDfriend)
{
  // Process comparison information 
  //
  if(!esdEvent) 
  {
    Error("Exec","esdEvent not available");
    return;
  }
  AliHeader* header = 0;
  AliGenEventHeader* genHeader = 0;
  AliStack* stack = 0;
  TArrayF vtxMC(3);
  
  if(bUseMC)
  {
    if(!mcEvent) {
      Error("Exec","mcEvent not available");
      return;
    }
    // get MC event header
    header = mcEvent->Header();
    if (!header) {
      Error("Exec","Header not available");
      return;
    }
    // MC particle stack
    stack = mcEvent->Stack();
    if (!stack) {
      Error("Exec","Stack not available");
      return;
    }
    // get MC vertex
    genHeader = header->GenEventHeader();
    if (!genHeader) {
      Error("Exec","Could not retrieve genHeader from Header");
      return;
    }
    genHeader->PrimaryVertex(vtxMC);
  } 
  else {
    Error("Exec","MC information required!");
    return;
  } 

  // use ESD friends
  if(bUseESDfriend) {
    if(!esdFriend) {
      Error("Exec","esdFriend not available");
      return;
    }
  }

  //
  //  Process events
  //
  if(GetAnalysisMode() == 0) ProcessTPC(mcEvent,esdEvent);
  else if(GetAnalysisMode() == 1) ProcessTPCITS(mcEvent,esdEvent);
  else if(GetAnalysisMode() == 2) ProcessConstrained(mcEvent,esdEvent);
  else if(GetAnalysisMode() == 5) ProcessTPCSec(mcEvent,esdEvent);
  else {
    printf("ERROR: AnalysisMode %d \n",fAnalysisMode);
    return;
  }
}

//_____________________________________________________________________________
Int_t AliPerformanceEff::TransformToPID(TParticle *particle) 
{
// transform Pdg to Pid
// Pdg convension is different for hadrons and leptons 
// (e.g. K+/K- = 321/-321; e+/e- = -11/11 ) 

  Int_t pid = -1;
  if( TMath::Abs(particle->GetPdgCode())==fCutsMC->GetEM() ) pid = 0; 
  if( TMath::Abs(particle->GetPdgCode())==fCutsMC->GetMuM() ) pid = 1; 
  if( TMath::Abs(particle->GetPdgCode())==fCutsMC->GetPiP() ) pid = 2; 
  if( TMath::Abs(particle->GetPdgCode())==fCutsMC->GetKP() ) pid = 3; 
  if( TMath::Abs(particle->GetPdgCode())==fCutsMC->GetProt() ) pid = 4; 

return pid;
}

//_____________________________________________________________________________
Bool_t AliPerformanceEff::IsFindable(const AliMCEvent *mcEvent, Int_t label) 
{
//
// Findfindable tracks
//
if(!mcEvent) return kFALSE;

  AliMCParticle *mcParticle = (AliMCParticle*) mcEvent->GetTrack(label);
  if(!mcParticle) return kFALSE;

  Int_t counter; 
  Float_t tpcTrackLength = mcParticle->GetTPCTrackLength(AliTracker::GetBz(),0.05,counter,3.0); 
  //printf("tpcTrackLength %f \n", tpcTrackLength);

return (tpcTrackLength>fCutsMC->GetMinTrackLength());    
}

//_____________________________________________________________________________
Bool_t AliPerformanceEff::IsRecTPC(AliESDtrack *esdTrack) 
{
//
// Check whether track is reconstructed in TPC
//
if(!esdTrack) return kFALSE;

  const AliExternalTrackParam *track = esdTrack->GetTPCInnerParam();
  if(!track) return kFALSE;

  Float_t dca[2], cov[3]; // dca_xy, dca_z, sigma_xy, sigma_xy_z, sigma_z
  esdTrack->GetImpactParametersTPC(dca,cov);

  Bool_t recStatus = kFALSE;
  if(esdTrack->GetTPCNcls()>fCutsRC->GetMinNClustersTPC()) recStatus = kTRUE; 

  /*
  if( TMath::Abs(dca[0])<fCutsRC->GetMaxDCAToVertexXY() && 
      TMath::Abs(dca[1])<fCutsRC->GetMaxDCAToVertexZ())
  {
    recStatus = kTRUE;
  }
  */

return recStatus;
}

//_____________________________________________________________________________
Bool_t AliPerformanceEff::IsRecTPCITS(AliESDtrack *esdTrack) 
{
//
// Check whether track is reconstructed in TPCITS
//
if(!esdTrack) return kFALSE;

  Float_t dca[2], cov[3]; // dca_xy, dca_z, sigma_xy, sigma_xy_z, sigma_z
  esdTrack->GetImpactParameters(dca,cov);

  Bool_t recStatus = kFALSE;

  if ((esdTrack->GetStatus()&AliESDtrack::kTPCrefit)==0) return kFALSE; // TPC refit
  if (esdTrack->GetTPCNcls()<fCutsRC->GetMinNClustersTPC()) return kFALSE; // min. nb. TPC clusters
  //if ((esdTrack->GetStatus()&AliESDtrack::kITSrefit)==0) return kFALSE; // ITS refit
  //Int_t clusterITS[200];
  //if(esdTrack->GetITSclusters(clusterITS)<fCutsRC->GetMinNClustersITS()) return kFALSE;  // min. nb. ITS clusters

  recStatus = kTRUE;
  /*
  if(TMath::Abs(dca[0])<fCutsRC->GetMaxDCAToVertexXY() && 
     TMath::Abs(dca[1])<fCutsRC->GetMaxDCAToVertexZ())
  {
    recStatus = kTRUE;
  }
  */

return recStatus;
}

//_____________________________________________________________________________
Bool_t AliPerformanceEff::IsRecConstrained(AliESDtrack *esdTrack) 
{
//
// Check whether track is reconstructed in IsRecConstrained
//
  if(!esdTrack) return kFALSE;

  const AliExternalTrackParam * track = esdTrack->GetConstrainedParam();
  if(!track) return kFALSE;

  Float_t dca[2], cov[3]; // dca_xy, dca_z, sigma_xy, sigma_xy_z, sigma_z
  esdTrack->GetImpactParameters(dca,cov);
  //Int_t label = TMath::Abs(esdTrack->GetLabel()); 

  Bool_t recStatus = kFALSE;

  if ((esdTrack->GetStatus()&AliESDtrack::kTPCrefit)==0) return kFALSE; // TPC refit
  if (esdTrack->GetTPCNcls()<fCutsRC->GetMinNClustersTPC()) return kFALSE; // min. nb. TPC clusters
  Int_t clusterITS[200];
  if(esdTrack->GetITSclusters(clusterITS)<fCutsRC->GetMinNClustersITS()) return kFALSE;  // min. nb. ITS clusters

  recStatus = kTRUE;

  /*
  if(TMath::Abs(dca[0])<fCutsRC->GetMaxDCAToVertexXY() && 
     TMath::Abs(dca[1])<fCutsRC->GetMaxDCAToVertexZ())
  {
    recStatus = kTRUE;
  }
  */

return recStatus;
}

//_____________________________________________________________________________
Long64_t AliPerformanceEff::Merge(TCollection* const list) 
{
  // Merge list of objects (needed by PROOF)

  if (!list)
  return 0;

  if (list->IsEmpty())
  return 1;

  TIterator* iter = list->MakeIterator();
  TObject* obj = 0;

  // collection of generated histograms

  Int_t count=0;
  while((obj = iter->Next()) != 0) 
  {
    AliPerformanceEff* entry = dynamic_cast<AliPerformanceEff*>(obj);
    if (entry == 0) continue; 
  
     fEffHisto->Add(entry->fEffHisto);
     fEffSecHisto->Add(entry->fEffSecHisto);
  count++;
  }

return count;
}
 
//_____________________________________________________________________________
void AliPerformanceEff::Analyse() 
{
  // Analyse comparison information and store output histograms
  // in the folder "folderEff" 
  //
  TH1::AddDirectory(kFALSE);
  TObjArray *aFolderObj = new TObjArray;
  char title[256];

  //
  // efficiency vs pt
  //

  if(GetAnalysisMode() != 5) {

  fEffHisto->GetAxis(0)->SetRangeUser(-0.9,0.9); // eta range
  fEffHisto->GetAxis(2)->SetRangeUser(0.1,10.); // pt range

  // rec efficiency vs pt
  TH1D *ptAll = fEffHisto->Projection(2);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.);  // reconstructed 
  TH1D *ptRec = fEffHisto->Projection(2);
  TH1D *ptRecc = (TH1D*)ptRec->Clone();
  ptRecc->Divide(ptRec,ptAll,1,1,"B");
  ptRecc->SetName("ptRecEff");

  ptRecc->GetXaxis()->SetTitle(fEffHisto->GetAxis(2)->GetTitle());
  ptRecc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency",fEffHisto->GetAxis(2)->GetTitle());
  ptRecc->SetTitle(title);

  ptRecc->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecc);

  // rec efficiency vs pid vs pt

  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(3)->SetRangeUser(2.,2.); // pions

  TH1D *ptAllPi = fEffHisto->Projection(2);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *ptRecPi = fEffHisto->Projection(2);
  TH1D *ptRecPic = (TH1D*)ptRecPi->Clone();
  ptRecPic->Divide(ptRecPi,ptAllPi,1,1,"B");
  ptRecPic->SetName("ptRecEffPi");

  ptRecPic->GetXaxis()->SetTitle(fEffHisto->GetAxis(2)->GetTitle());
  ptRecPic->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (pions)",fEffHisto->GetAxis(2)->GetTitle());
  ptRecPic->SetTitle(title);

  ptRecPic->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecPic);

  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(3)->SetRangeUser(3.,3.); // kaons
  TH1D *ptAllK = fEffHisto->Projection(2);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *ptRecK = fEffHisto->Projection(2);

  TH1D *ptRecKc = (TH1D*)ptRecK->Clone();
  ptRecKc->Divide(ptRecK,ptAllK,1,1,"B");
  ptRecKc->SetName("ptRecEffK");

  ptRecKc->GetXaxis()->SetTitle(fEffHisto->GetAxis(2)->GetTitle());
  ptRecKc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (kaons)",fEffHisto->GetAxis(2)->GetTitle());
  ptRecKc->SetTitle(title);


  ptRecKc->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecKc);

  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(3)->SetRangeUser(4.,4.); // protons
  TH1D *ptAllP = fEffHisto->Projection(2);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *ptRecP = fEffHisto->Projection(2);
  TH1D *ptRecPc = (TH1D*)ptRecP->Clone();
  ptRecPc->Divide(ptRecP,ptAllP,1,1,"B");
  ptRecPc->SetName("ptRecEffP");

  ptRecPc->GetXaxis()->SetTitle(fEffHisto->GetAxis(2)->GetTitle());
  ptRecPc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (protons)",fEffHisto->GetAxis(2)->GetTitle());
  ptRecPc->SetTitle(title);

  ptRecPc->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecPc);

  // findable efficiency vs pt

  fEffHisto->GetAxis(3)->SetRangeUser(0.,4.); 
  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(5)->SetRangeUser(1.,1.); // findable
  TH1D *ptAllF = fEffHisto->Projection(2);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.);
  fEffHisto->GetAxis(5)->SetRangeUser(1.,1.);

  TH1D *ptRecF = fEffHisto->Projection(2); // rec findable
  TH1D *ptRecFc = (TH1D*)ptRecF->Clone();
  ptRecFc->Divide(ptRecF,ptAllF,1,1,"B");
  ptRecFc->SetName("ptRecEffF");

  ptRecFc->GetXaxis()->SetTitle(fEffHisto->GetAxis(2)->GetTitle());
  ptRecFc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (findable)",fEffHisto->GetAxis(2)->GetTitle());
  ptRecFc->SetTitle(title);

  ptRecFc->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecFc);

  //
  // efficiency vs eta
  //

  fEffHisto->GetAxis(0)->SetRangeUser(-1.5,1.5); // eta range
  fEffHisto->GetAxis(2)->SetRangeUser(0.2,10.); // pt range
  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.);   // all
  fEffHisto->GetAxis(5)->SetRangeUser(0.,1.);   // all

  TH1D *etaAll = fEffHisto->Projection(0);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.);  // reconstructed 
  TH1D *etaRec = fEffHisto->Projection(0);
  TH1D *etaRecc = (TH1D*)etaRec->Clone();
  etaRecc->Divide(etaRec,etaAll,1,1,"B");
  etaRecc->SetName("etaRecEff");

  etaRecc->GetXaxis()->SetTitle(fEffHisto->GetAxis(0)->GetTitle());
  etaRecc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency",fEffHisto->GetAxis(0)->GetTitle());
  etaRecc->SetTitle(title);

  aFolderObj->Add(etaRecc);

  // rec efficiency vs pid vs eta
  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(3)->SetRangeUser(2.,2.); // pions

  TH1D *etaAllPi = fEffHisto->Projection(0);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *etaRecPi = fEffHisto->Projection(0);
  TH1D *etaRecPic = (TH1D*)etaRecPi->Clone();
  etaRecPic->Divide(etaRecPi,etaAllPi,1,1,"B");
  etaRecPic->SetName("etaRecEffPi");

  etaRecPic->GetXaxis()->SetTitle(fEffHisto->GetAxis(0)->GetTitle());
  etaRecPic->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (pions)",fEffHisto->GetAxis(0)->GetTitle());
  etaRecPic->SetTitle(title);

  aFolderObj->Add(etaRecPic);

  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(3)->SetRangeUser(3.,3.); // kaons
  TH1D *etaAllK = fEffHisto->Projection(0);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *etaRecK = fEffHisto->Projection(0);

  TH1D *etaRecKc = (TH1D*)etaRecK->Clone();
  etaRecKc->Divide(etaRecK,etaAllK,1,1,"B");
  etaRecKc->SetName("etaRecEffK");

  etaRecKc->GetXaxis()->SetTitle(fEffHisto->GetAxis(0)->GetTitle());
  etaRecKc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (kaons)",fEffHisto->GetAxis(0)->GetTitle());
  etaRecKc->SetTitle(title);


  aFolderObj->Add(etaRecKc);

  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(3)->SetRangeUser(4.,4.); // protons
  TH1D *etaAllP = fEffHisto->Projection(0);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *etaRecP = fEffHisto->Projection(0);
  TH1D *etaRecPc = (TH1D*)etaRecP->Clone();
  etaRecPc->Divide(etaRecP,etaAllP,1,1,"B");
  etaRecPc->SetName("etaRecEffP");

  etaRecPc->GetXaxis()->SetTitle(fEffHisto->GetAxis(0)->GetTitle());
  etaRecPc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (protons)",fEffHisto->GetAxis(0)->GetTitle());
  etaRecPc->SetTitle(title);

  aFolderObj->Add(etaRecPc);

  // findable efficiency vs eta

  fEffHisto->GetAxis(3)->SetRangeUser(0.,4.); 
  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(5)->SetRangeUser(1.,1.); // findable
  TH1D *etaAllF = fEffHisto->Projection(0);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.);
  fEffHisto->GetAxis(5)->SetRangeUser(1.,1.);

  TH1D *etaRecF = fEffHisto->Projection(0); // rec findable
  TH1D *etaRecFc = (TH1D*)etaRecF->Clone();
  etaRecFc->Divide(etaRecF,etaAllF,1,1,"B");
  etaRecFc->SetName("etaRecEffF");

  etaRecFc->GetXaxis()->SetTitle(fEffHisto->GetAxis(0)->GetTitle());
  etaRecFc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (findable)",fEffHisto->GetAxis(0)->GetTitle());
  etaRecFc->SetTitle(title);

  aFolderObj->Add(etaRecFc);

  //
  // efficiency vs phi
  //

  fEffHisto->GetAxis(0)->SetRangeUser(-0.9,0.9); // eta range
  fEffHisto->GetAxis(2)->SetRangeUser(0.2,10.); // pt range
  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.);   // all
  fEffHisto->GetAxis(5)->SetRangeUser(0.,1.);   // all

  TH1D *phiAll = fEffHisto->Projection(1);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.);  // reconstructed 
  TH1D *phiRec = fEffHisto->Projection(1);
  TH1D *phiRecc = (TH1D*)phiRec->Clone();
  phiRecc->Divide(phiRec,phiAll,1,1,"B");
  phiRecc->SetName("phiRecEff");

  phiRecc->GetXaxis()->SetTitle(fEffHisto->GetAxis(1)->GetTitle());
  phiRecc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency",fEffHisto->GetAxis(1)->GetTitle());
  phiRecc->SetTitle(title);

  aFolderObj->Add(phiRecc);

  // rec efficiency vs pid vs phi
  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(3)->SetRangeUser(2.,2.); // pions

  TH1D *phiAllPi = fEffHisto->Projection(1);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *phiRecPi = fEffHisto->Projection(1);
  TH1D *phiRecPic = (TH1D*)phiRecPi->Clone();
  phiRecPic->Divide(phiRecPi,phiAllPi,1,1,"B");
  phiRecPic->SetName("phiRecEffPi");

  phiRecPic->GetXaxis()->SetTitle(fEffHisto->GetAxis(1)->GetTitle());
  phiRecPic->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (pions)",fEffHisto->GetAxis(1)->GetTitle());
  phiRecPic->SetTitle(title);

  aFolderObj->Add(phiRecPic);

  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(3)->SetRangeUser(3.,3.); // kaons
  TH1D *phiAllK = fEffHisto->Projection(1);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *phiRecK = fEffHisto->Projection(1);

  TH1D *phiRecKc = (TH1D*)phiRecK->Clone();
  phiRecKc->Divide(phiRecK,phiAllK,1,1,"B");
  phiRecKc->SetName("phiRecEffK");

  phiRecKc->GetXaxis()->SetTitle(fEffHisto->GetAxis(1)->GetTitle());
  phiRecKc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (kaons)",fEffHisto->GetAxis(1)->GetTitle());
  phiRecKc->SetTitle(title);


  aFolderObj->Add(phiRecKc);

  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(3)->SetRangeUser(4.,4.); // protons
  TH1D *phiAllP = fEffHisto->Projection(1);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *phiRecP = fEffHisto->Projection(1);
  TH1D *phiRecPc = (TH1D*)phiRecP->Clone();
  phiRecPc->Divide(phiRecP,phiAllP,1,1,"B");
  phiRecPc->SetName("phiRecEffP");

  phiRecPc->GetXaxis()->SetTitle(fEffHisto->GetAxis(1)->GetTitle());
  phiRecPc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (protons)",fEffHisto->GetAxis(1)->GetTitle());
  phiRecPc->SetTitle(title);

  aFolderObj->Add(phiRecPc);

  // findable efficiency vs phi

  fEffHisto->GetAxis(3)->SetRangeUser(0.,4.); 
  fEffHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffHisto->GetAxis(5)->SetRangeUser(1.,1.); // findable
  TH1D *phiAllF = fEffHisto->Projection(1);

  fEffHisto->GetAxis(4)->SetRangeUser(1.,1.);
  fEffHisto->GetAxis(5)->SetRangeUser(1.,1.);

  TH1D *phiRecF = fEffHisto->Projection(1); // rec findable
  TH1D *phiRecFc = (TH1D*)phiRecF->Clone();
  phiRecFc->Divide(phiRecF,phiAllF,1,1,"B");
  phiRecFc->SetName("phiRecEffF");

  phiRecFc->GetXaxis()->SetTitle(fEffHisto->GetAxis(1)->GetTitle());
  phiRecFc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (findable)",fEffHisto->GetAxis(1)->GetTitle());
  phiRecFc->SetTitle(title);

  aFolderObj->Add(phiRecFc);
  }
  else {
  // 
  Float_t minEta=-1.5, maxEta=1.5;
  Float_t minR=0.0, maxR=150.0;
  Float_t minPt=0.15, maxPt=100.0;

  // mother eta range
  fEffSecHisto->GetAxis(8)->SetRangeUser(minEta,maxEta);

  // particle creation radius range 
  fEffSecHisto->GetAxis(6)->SetRangeUser(minR,maxR);

  //
  fEffSecHisto->GetAxis(0)->SetRangeUser(minEta,maxEta);
  fEffSecHisto->GetAxis(2)->SetRangeUser(minPt,maxPt);

  // rec efficiency vs pt
  TH1D *ptAll = fEffSecHisto->Projection(2);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.);  // reconstructed 
  TH1D *ptRec = fEffSecHisto->Projection(2);
  TH1D *ptRecc = (TH1D*)ptRec->Clone();
  ptRecc->Divide(ptRec,ptAll,1,1,"B");
  ptRecc->SetName("ptRecEff");

  ptRecc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency",fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecc->SetTitle(title);

  ptRecc->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecc);

  // rec efficiency vs pid vs pt
  
  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(0.,0.); // electrons

  TH1D *ptAllEle = fEffSecHisto->Projection(2);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *ptRecEle = fEffSecHisto->Projection(2);
  TH1D *ptRecElec = (TH1D*)ptRecEle->Clone();
  ptRecElec->Divide(ptRecEle,ptAllEle,1,1,"B");
  ptRecElec->SetName("ptRecEffEle");

  ptRecElec->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecElec->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (electrons)",fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecElec->SetTitle(title);

  ptRecElec->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecElec);

  //

  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(2.,2.); // pions

  TH1D *ptAllPi = fEffSecHisto->Projection(2);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *ptRecPi = fEffSecHisto->Projection(2);
  TH1D *ptRecPic = (TH1D*)ptRecPi->Clone();
  ptRecPic->Divide(ptRecPi,ptAllPi,1,1,"B");
  ptRecPic->SetName("ptRecEffPi");

  ptRecPic->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecPic->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (pions)",fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecPic->SetTitle(title);

  ptRecPic->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecPic);

  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(3.,3.); // kaons
  TH1D *ptAllK = fEffSecHisto->Projection(2);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *ptRecK = fEffSecHisto->Projection(2);

  TH1D *ptRecKc = (TH1D*)ptRecK->Clone();
  ptRecKc->Divide(ptRecK,ptAllK,1,1,"B");
  ptRecKc->SetName("ptRecEffK");

  ptRecKc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecKc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (kaons)",fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecKc->SetTitle(title);


  ptRecKc->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecKc);

  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(4.,4.); // protons
  TH1D *ptAllP = fEffSecHisto->Projection(2);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *ptRecP = fEffSecHisto->Projection(2);
  TH1D *ptRecPc = (TH1D*)ptRecP->Clone();
  ptRecPc->Divide(ptRecP,ptAllP,1,1,"B");
  ptRecPc->SetName("ptRecEffP");

  ptRecPc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecPc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (protons)",fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecPc->SetTitle(title);

  ptRecPc->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecPc);

  // findable efficiency vs pt

  fEffSecHisto->GetAxis(3)->SetRangeUser(0.,4.); 
  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(5)->SetRangeUser(1.,1.); // findable
  TH1D *ptAllF = fEffSecHisto->Projection(2);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.);
  fEffSecHisto->GetAxis(5)->SetRangeUser(1.,1.);

  TH1D *ptRecF = fEffSecHisto->Projection(2); // rec findable
  TH1D *ptRecFc = (TH1D*)ptRecF->Clone();
  ptRecFc->Divide(ptRecF,ptAllF,1,1,"B");
  ptRecFc->SetName("ptRecEffF");

  ptRecFc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecFc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (findable)",fEffSecHisto->GetAxis(2)->GetTitle());
  ptRecFc->SetTitle(title);

  ptRecFc->SetBit(TH1::kLogX);
  aFolderObj->Add(ptRecFc);

  //
  // efficiency vs eta
  //
  fEffSecHisto->GetAxis(2)->SetRangeUser(minPt,maxPt);
  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.);   // all
  fEffSecHisto->GetAxis(5)->SetRangeUser(0.,1.);   // all

  TH1D *etaAll = fEffSecHisto->Projection(0);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.);  // reconstructed 
  TH1D *etaRec = fEffSecHisto->Projection(0);
  TH1D *etaRecc = (TH1D*)etaRec->Clone();
  etaRecc->Divide(etaRec,etaAll,1,1,"B");
  etaRecc->SetName("etaRecEff");

  etaRecc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency",fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecc->SetTitle(title);

  aFolderObj->Add(etaRecc);

  // rec efficiency vs pid vs eta
  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(0.,0.); // electrons

  TH1D *etaAllEle = fEffSecHisto->Projection(0);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *etaRecEle = fEffSecHisto->Projection(0);
  TH1D *etaRecElec = (TH1D*)etaRecEle->Clone();
  etaRecElec->Divide(etaRecEle,etaAllEle,1,1,"B");
  etaRecElec->SetName("etaRecEffEle");

  etaRecElec->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecElec->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (electrons)",fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecElec->SetTitle(title);

  aFolderObj->Add(etaRecElec);

  //
  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(2.,2.); // pions

  TH1D *etaAllPi = fEffSecHisto->Projection(0);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *etaRecPi = fEffSecHisto->Projection(0);
  TH1D *etaRecPic = (TH1D*)etaRecPi->Clone();
  etaRecPic->Divide(etaRecPi,etaAllPi,1,1,"B");
  etaRecPic->SetName("etaRecEffPi");

  etaRecPic->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecPic->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (pions)",fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecPic->SetTitle(title);

  aFolderObj->Add(etaRecPic);

  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(3.,3.); // kaons
  TH1D *etaAllK = fEffSecHisto->Projection(0);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *etaRecK = fEffSecHisto->Projection(0);

  TH1D *etaRecKc = (TH1D*)etaRecK->Clone();
  etaRecKc->Divide(etaRecK,etaAllK,1,1,"B");
  etaRecKc->SetName("etaRecEffK");

  etaRecKc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecKc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (kaons)",fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecKc->SetTitle(title);


  aFolderObj->Add(etaRecKc);

  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(4.,4.); // protons
  TH1D *etaAllP = fEffSecHisto->Projection(0);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *etaRecP = fEffSecHisto->Projection(0);
  TH1D *etaRecPc = (TH1D*)etaRecP->Clone();
  etaRecPc->Divide(etaRecP,etaAllP,1,1,"B");
  etaRecPc->SetName("etaRecEffP");

  etaRecPc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecPc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (protons)",fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecPc->SetTitle(title);

  aFolderObj->Add(etaRecPc);

  // findable efficiency vs eta

  fEffSecHisto->GetAxis(3)->SetRangeUser(0.,4.); 
  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(5)->SetRangeUser(1.,1.); // findable
  TH1D *etaAllF = fEffSecHisto->Projection(0);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.);
  fEffSecHisto->GetAxis(5)->SetRangeUser(1.,1.);

  TH1D *etaRecF = fEffSecHisto->Projection(0); // rec findable
  TH1D *etaRecFc = (TH1D*)etaRecF->Clone();
  etaRecFc->Divide(etaRecF,etaAllF,1,1,"B");
  etaRecFc->SetName("etaRecEffF");

  etaRecFc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecFc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (findable)",fEffSecHisto->GetAxis(0)->GetTitle());
  etaRecFc->SetTitle(title);

  aFolderObj->Add(etaRecFc);

  //
  // efficiency vs phi
  //

  fEffSecHisto->GetAxis(0)->SetRangeUser(minEta,maxEta);
  fEffSecHisto->GetAxis(2)->SetRangeUser(minPt,maxPt);

  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.);   // all
  fEffSecHisto->GetAxis(5)->SetRangeUser(0.,1.);   // all

  TH1D *phiAll = fEffSecHisto->Projection(1);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.);  // reconstructed 
  TH1D *phiRec = fEffSecHisto->Projection(1);
  TH1D *phiRecc = (TH1D*)phiRec->Clone();
  phiRecc->Divide(phiRec,phiAll,1,1,"B");
  phiRecc->SetName("phiRecEff");

  phiRecc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency",fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecc->SetTitle(title);

  aFolderObj->Add(phiRecc);

  // rec efficiency vs pid vs phi
  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(2.,2.); // pions

  TH1D *phiAllEle = fEffSecHisto->Projection(1);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *phiRecEle = fEffSecHisto->Projection(1);
  TH1D *phiRecElec = (TH1D*)phiRecEle->Clone();
  phiRecElec->Divide(phiRecEle,phiAllEle,1,1,"B");
  phiRecElec->SetName("phiRecEffEle");

  phiRecElec->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecElec->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (electrons)",fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecElec->SetTitle(title);

  aFolderObj->Add(phiRecElec);

  //
  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(2.,2.); // pions

  TH1D *phiAllPi = fEffSecHisto->Projection(1);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *phiRecPi = fEffSecHisto->Projection(1);
  TH1D *phiRecPic = (TH1D*)phiRecPi->Clone();
  phiRecPic->Divide(phiRecPi,phiAllPi,1,1,"B");
  phiRecPic->SetName("phiRecEffPi");

  phiRecPic->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecPic->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (pions)",fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecPic->SetTitle(title);

  aFolderObj->Add(phiRecPic);

  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(3.,3.); // kaons
  TH1D *phiAllK = fEffSecHisto->Projection(1);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *phiRecK = fEffSecHisto->Projection(1);

  TH1D *phiRecKc = (TH1D*)phiRecK->Clone();
  phiRecKc->Divide(phiRecK,phiAllK,1,1,"B");
  phiRecKc->SetName("phiRecEffK");

  phiRecKc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecKc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (kaons)",fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecKc->SetTitle(title);


  aFolderObj->Add(phiRecKc);

  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(3)->SetRangeUser(4.,4.); // protons
  TH1D *phiAllP = fEffSecHisto->Projection(1);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.); // reconstructed
  TH1D *phiRecP = fEffSecHisto->Projection(1);
  TH1D *phiRecPc = (TH1D*)phiRecP->Clone();
  phiRecPc->Divide(phiRecP,phiAllP,1,1,"B");
  phiRecPc->SetName("phiRecEffP");

  phiRecPc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecPc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (protons)",fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecPc->SetTitle(title);

  aFolderObj->Add(phiRecPc);

  // findable efficiency vs phi

  fEffSecHisto->GetAxis(3)->SetRangeUser(0.,4.); 
  fEffSecHisto->GetAxis(4)->SetRangeUser(0.,1.); 
  fEffSecHisto->GetAxis(5)->SetRangeUser(1.,1.); // findable
  TH1D *phiAllF = fEffSecHisto->Projection(1);

  fEffSecHisto->GetAxis(4)->SetRangeUser(1.,1.);
  fEffSecHisto->GetAxis(5)->SetRangeUser(1.,1.);

  TH1D *phiRecF = fEffSecHisto->Projection(1); // rec findable
  TH1D *phiRecFc = (TH1D*)phiRecF->Clone();
  phiRecFc->Divide(phiRecF,phiAllF,1,1,"B");
  phiRecFc->SetName("phiRecEffF");

  phiRecFc->GetXaxis()->SetTitle(fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecFc->GetYaxis()->SetTitle("efficiency");
  sprintf(title,"%s vs %s","rec. efficiency (findable)",fEffSecHisto->GetAxis(1)->GetTitle());
  phiRecFc->SetTitle(title);

  aFolderObj->Add(phiRecFc);
  }

  // export objects to analysis folder
  fAnalysisFolder = ExportToFolder(aFolderObj);

  // delete only TObjArray
  if(aFolderObj) delete aFolderObj;
}

//_____________________________________________________________________________
TFolder* AliPerformanceEff::ExportToFolder(TObjArray * array) 
{
  // recreate folder avery time and export objects to new one
  //
  AliPerformanceEff * comp=this;
  TFolder *folder = comp->GetAnalysisFolder();

  TString name, title;
  TFolder *newFolder = 0;
  Int_t i = 0;
  Int_t size = array->GetSize();

  if(folder) { 
     // get name and title from old folder
     name = folder->GetName();  
     title = folder->GetTitle();  

	 // delete old one
     delete folder;

	 // create new one
     newFolder = CreateFolder(name.Data(),title.Data());
     newFolder->SetOwner();

	 // add objects to folder
     while(i < size) {
	   newFolder->Add(array->At(i));
	   i++;
	 }
  }

return newFolder;
}


//_____________________________________________________________________________
TFolder* AliPerformanceEff::CreateFolder(TString name,TString title) { 
// create folder for analysed histograms
//
TFolder *folder = 0;
  folder = new TFolder(name.Data(),title.Data());

  return folder;
}
