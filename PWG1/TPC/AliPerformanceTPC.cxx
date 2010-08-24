//------------------------------------------------------------------------------
// Implementation of AliPerformanceTPC class. It keeps information from 
// comparison of reconstructed and MC particle tracks. In addtion, 
// it keeps selection cuts used during comparison. The comparison 
// information is stored in the ROOT histograms. Analysis of these 
// histograms can be done by using Analyse() class function. The result of 
// the analysis (histograms/graphs) are stored in the folder which is
// a data member of AliPerformanceTPC.
//
// Author: J.Otwinowski 04/02/2008 
// Changes by M.Knichel 16/08/2010
//------------------------------------------------------------------------------

/*
 
  // after running comparison task, read the file, and get component
  gROOT->LoadMacro("$ALICE_ROOT/PWG1/Macros/LoadMyLibs.C");
  LoadMyLibs();

  TFile f("Output.root");
  AliPerformanceTPC * compObj = (AliPerformanceTPC*)coutput->FindObject("AliPerformanceTPC");
 
  // analyse comparison data
  compObj->Analyse();

  // the output histograms/graphs will be stored in the folder "folderTPC" 
  compObj->GetAnalysisFolder()->ls("*");

  // user can save whole comparison object (or only folder with anlysed histograms) 
  // in the seperate output file (e.g.)
  TFile fout("Analysed_TPC.root","recreate");
  compObj->Write(); // compObj->GetAnalysisFolder()->Write();
  fout.Close();

*/

#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TAxis.h"
#include "TPostScript.h"
#include "TString.h"

#include "AliPerformanceTPC.h" 
#include "AliESDEvent.h" 
#include "AliESDVertex.h"
#include "AliESDtrack.h"
#include "AliLog.h" 
#include "AliMCEvent.h" 
#include "AliHeader.h" 
#include "AliGenEventHeader.h" 
#include "AliStack.h" 
#include "AliMCInfoCuts.h" 
#include "AliRecInfoCuts.h" 
#include "AliTracker.h" 
#include "AliTreeDraw.h" 
#include "AliTPCTransform.h" 
#include "AliTPCseed.h" 
#include "AliTPCcalibDB.h" 
#include "AliESDfriend.h" 
#include "AliESDfriendTrack.h" 
#include "AliTPCclusterMI.h" 

using namespace std;

ClassImp(AliPerformanceTPC)

Bool_t AliPerformanceTPC::fgMergeTHnSparse = kFALSE;


//_____________________________________________________________________________
AliPerformanceTPC::AliPerformanceTPC():
  AliPerformanceObject("AliPerformanceTPC"),
  fTPCClustHisto(0),
  fTPCEventHisto(0),
  fTPCTrackHisto(0),
  fFolderObj(0),

  // Cuts 
  fCutsRC(0),  
  fCutsMC(0),  

  // histogram folder 
  fAnalysisFolder(0),
  
  fUseHLT(kFALSE)

{
  Init();
}


//_____________________________________________________________________________
AliPerformanceTPC::AliPerformanceTPC(Char_t* name="AliPerformanceTPC", Char_t* title="AliPerformanceTPC",Int_t analysisMode=0,Bool_t hptGenerator=kFALSE):
  AliPerformanceObject(name,title),
  fTPCClustHisto(0),
  fTPCEventHisto(0),
  fTPCTrackHisto(0),
  fFolderObj(0),

  // Cuts 
  fCutsRC(0),  
  fCutsMC(0),  

  // histogram folder 
  fAnalysisFolder(0),
  
  fUseHLT(kFALSE)

{
  // named constructor	
  // 
  SetAnalysisMode(analysisMode);
  SetHptGenerator(hptGenerator);

  Init();
}


//_____________________________________________________________________________
AliPerformanceTPC::~AliPerformanceTPC()
{
  // destructor
   
  if(fTPCClustHisto) delete fTPCClustHisto; fTPCClustHisto=0;     
  if(fTPCEventHisto) delete fTPCEventHisto; fTPCEventHisto=0;     
  if(fTPCTrackHisto) delete fTPCTrackHisto; fTPCTrackHisto=0;   
  if(fAnalysisFolder) delete fAnalysisFolder; fAnalysisFolder=0;
  if(fFolderObj) delete fFolderObj; fFolderObj=0;
}


//_____________________________________________________________________________
void AliPerformanceTPC::Init()
{
  //
  // histogram bining
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
  // 
 
  // 
  //padRow:phi:TPCSide
  Int_t binsTPCClustHisto[3] =   {160,  180,  2 };
  Double_t minTPCClustHisto[3] = {0.,   0.,   0.};
  Double_t maxTPCClustHisto[3] = {160., 2.*TMath::Pi(), 2.};

  fTPCClustHisto = new THnSparseF("fTPCClustHisto","padRow:phi:TPCSide",3,binsTPCClustHisto,minTPCClustHisto,maxTPCClustHisto);
  fTPCClustHisto->GetAxis(0)->SetTitle("padRow");
  fTPCClustHisto->GetAxis(1)->SetTitle("phi (rad)");
  fTPCClustHisto->GetAxis(2)->SetTitle("TPCSide");
  //fTPCClustHisto->Sumw2();

  // Xv:Yv:Zv:mult:multP:multN:vertStatus
  Int_t binsTPCEventHisto[7]=  {100,  100,   100,  151,   151,   151, 2   };
  Double_t minTPCEventHisto[7]={-10., -10., -30.,  -0.5,  -0.5,  -0.5, -0.5  };
  Double_t maxTPCEventHisto[7]={ 10.,  10.,  30.,  150.5, 150.5, 150.5, 1.5 };

  fTPCEventHisto = new THnSparseF("fTPCEventHisto","Xv:Yv:Zv:mult:multP:multN:vertStatus",7,binsTPCEventHisto,minTPCEventHisto,maxTPCEventHisto);
  fTPCEventHisto->GetAxis(0)->SetTitle("Xv (cm)");
  fTPCEventHisto->GetAxis(1)->SetTitle("Yv (cm)");
  fTPCEventHisto->GetAxis(2)->SetTitle("Zv (cm)");
  fTPCEventHisto->GetAxis(3)->SetTitle("mult");
  fTPCEventHisto->GetAxis(4)->SetTitle("multP");
  fTPCEventHisto->GetAxis(5)->SetTitle("multN");
  fTPCEventHisto->GetAxis(6)->SetTitle("vertStatus");
  //fTPCEventHisto->Sumw2();


  // nTPCClust:chi2PerTPCClust:nTPCClustFindRatio:DCAr:DCAz:eta:phi:pt:charge:vertStatus
   Int_t binsTPCTrackHisto[10]=  { 160,  20,  60,  30, 30,  30,   144,             nPtBins,   3, 2 };
   Double_t minTPCTrackHisto[10]={ 0.,   0.,  0., -3,  -3., -1.5, 0.,             ptMin,   -1.5, -0.5 };
   Double_t maxTPCTrackHisto[10]={ 160., 5., 1.2, 3,   3.,  1.5, 2.*TMath::Pi(), ptMax,    1.5,  1.5 };
  
  // nTPCClust:chi2PerTPCClust:nTPCClustFindRatio:DCAr:DCAz:eta:phi:pt:charge:vertStatus
//   Int_t binsTPCTrackHisto[10]=  { 160,  50,  60,  100, 100,  30,   144,            nPtBins,    3, 2 };
//   Double_t minTPCTrackHisto[10]={ 0.,   0.,  0., -10,  -10., -1.5, 0.,             ptMin,   -1.5, 0 };
//   Double_t maxTPCTrackHisto[10]={ 160., 10., 1.2, 10,   10.,  1.5, 2.*TMath::Pi(), ptMax,    1.5, 2 };



  fTPCTrackHisto = new THnSparseF("fTPCTrackHisto","nClust:chi2PerClust:nClust/nFindableClust:DCAr:DCAz:eta:phi:pt:charge:vertStatus",10,binsTPCTrackHisto,minTPCTrackHisto,maxTPCTrackHisto);
  fTPCTrackHisto->SetBinEdges(7,binsPt);

  fTPCTrackHisto->GetAxis(0)->SetTitle("nClust");
  fTPCTrackHisto->GetAxis(1)->SetTitle("chi2PerClust");
  fTPCTrackHisto->GetAxis(2)->SetTitle("nClust/nFindableClust");
  fTPCTrackHisto->GetAxis(3)->SetTitle("DCAr (cm)");
  fTPCTrackHisto->GetAxis(4)->SetTitle("DCAz (cm)");
  fTPCTrackHisto->GetAxis(5)->SetTitle("#eta");
  fTPCTrackHisto->GetAxis(6)->SetTitle("#phi (rad)");
  fTPCTrackHisto->GetAxis(7)->SetTitle("p_{T} (GeV/c)");
  fTPCTrackHisto->GetAxis(8)->SetTitle("charge");
  fTPCTrackHisto->GetAxis(9)->SetTitle("vertStatus");
  //fTPCTrackHisto->Sumw2();

  // Init cuts 
  if(!fCutsMC) 
    AliDebug(AliLog::kError, "ERROR: Cannot find AliMCInfoCuts object");
  if(!fCutsRC) 
    AliDebug(AliLog::kError, "ERROR: Cannot find AliRecInfoCuts object");

  // init folder
  fAnalysisFolder = CreateFolder("folderTPC","Analysis Resolution Folder");
  
}


//_____________________________________________________________________________
void AliPerformanceTPC::ProcessTPC(AliStack* const stack, AliESDtrack *const esdTrack, AliESDEvent *const esdEvent, Bool_t vertStatus)
{
//
// fill TPC QA info
//
  if(!esdEvent) return;
  if(!esdTrack) return;

  if( IsUseTrackVertex() ) 
  { 
    // Relate TPC inner params to prim. vertex
    const AliESDVertex *vtxESD = esdEvent->GetPrimaryVertexTracks();
    Double_t x[3]; esdTrack->GetXYZ(x);
    Double_t b[3]; AliTracker::GetBxByBz(x,b);
    Bool_t isOK = esdTrack->RelateToVertexTPCBxByBz(vtxESD, b, kVeryBig);
    if(!isOK) return;

    /*
      // JMT -- recaluclate DCA for HLT if not present
      if ( dca[0] == 0. && dca[1] == 0. ) {
        track->GetDZ( vtxESD->GetX(), vtxESD->GetY(), vtxESD->GetZ(), esdEvent->GetMagneticField(), dca );
      }
    */
  }

  // Fill TPC only resolution comparison information 
  const AliExternalTrackParam *track = esdTrack->GetTPCInnerParam();
  if(!track) return;

  Float_t dca[2], cov[3]; // dca_xy, dca_z, sigma_xy, sigma_xy_z, sigma_z
  esdTrack->GetImpactParametersTPC(dca,cov);

  Float_t q = esdTrack->Charge();
  Float_t pt = track->Pt();
  Float_t eta = track->Eta();
  Float_t phi = track->Phi();
  Int_t nClust = esdTrack->GetTPCclusters(0);
  Int_t nFindableClust = esdTrack->GetTPCNclsF();

  Float_t chi2PerCluster = 0.;
  if(nClust>0.) chi2PerCluster = esdTrack->GetTPCchi2()/Float_t(nClust);

  Float_t clustPerFindClust = 0.;
  if(nFindableClust>0.) clustPerFindClust = Float_t(nClust)/nFindableClust;
  
  //
  // select primaries
  //
  Double_t dcaToVertex = -1;
  if( fCutsRC->GetDCAToVertex2D() ) 
  {
      dcaToVertex = TMath::Sqrt(dca[0]*dca[0]/fCutsRC->GetMaxDCAToVertexXY()/fCutsRC->GetMaxDCAToVertexXY() + dca[1]*dca[1]/fCutsRC->GetMaxDCAToVertexZ()/fCutsRC->GetMaxDCAToVertexZ()); 
  }
  if(fCutsRC->GetDCAToVertex2D() && dcaToVertex > 1) return;
  if(!fCutsRC->GetDCAToVertex2D() && TMath::Abs(dca[0]) > fCutsRC->GetMaxDCAToVertexXY()) return;
  if(!fCutsRC->GetDCAToVertex2D() && TMath::Abs(dca[1]) > fCutsRC->GetMaxDCAToVertexZ()) return;

  Double_t vTPCTrackHisto[10] = {nClust,chi2PerCluster,clustPerFindClust,dca[0],dca[1],eta,phi,pt,q,vertStatus};
  fTPCTrackHisto->Fill(vTPCTrackHisto); 
 
  //
  // Fill rec vs MC information
  //
  if(!stack) return;

}


//_____________________________________________________________________________
void AliPerformanceTPC::ProcessTPCITS(AliStack* const stack, AliESDtrack *const esdTrack, AliESDEvent* const esdEvent, Bool_t vertStatus)
{
  // Fill comparison information (TPC+ITS) 
  if(!esdTrack) return;
  if(!esdEvent) return;

  if( IsUseTrackVertex() ) 
  { 
    // Relate TPC inner params to prim. vertex
    const AliESDVertex *vtxESD = esdEvent->GetPrimaryVertexTracks();
    Double_t x[3]; esdTrack->GetXYZ(x);
    Double_t b[3]; AliTracker::GetBxByBz(x,b);
    Bool_t isOK = esdTrack->RelateToVertexBxByBz(vtxESD, b, kVeryBig);
    if(!isOK) return;

    /*
      // JMT -- recaluclate DCA for HLT if not present
      if ( dca[0] == 0. && dca[1] == 0. ) {
        track->GetDZ( vtxESD->GetX(), vtxESD->GetY(), vtxESD->GetZ(), esdEvent->GetMagneticField(), dca );
      }
    */
  }

  Float_t dca[2], cov[3]; // dca_xy, dca_z, sigma_xy, sigma_xy_z, sigma_z
  esdTrack->GetImpactParameters(dca,cov);

  if ((esdTrack->GetStatus()&AliESDtrack::kITSrefit)==0) return; // ITS refit
  if ((esdTrack->GetStatus()&AliESDtrack::kTPCrefit)==0) return; // TPC refit
  if (esdTrack->GetITSclusters(0)<fCutsRC->GetMinNClustersITS()) return;  // min. nb. ITS clusters

  Float_t q = esdTrack->Charge();
  Float_t pt = esdTrack->Pt();
  Float_t eta = esdTrack->Eta();
  Float_t phi = esdTrack->Phi();
  Int_t nClust = esdTrack->GetTPCclusters(0);
  Int_t nFindableClust = esdTrack->GetTPCNclsF();

  Float_t chi2PerCluster = 0.;
  if(nClust>0.) chi2PerCluster = esdTrack->GetTPCchi2()/Float_t(nClust);

  Float_t clustPerFindClust = 0.;
  if(nFindableClust>0.) clustPerFindClust = Float_t(nClust)/nFindableClust;
  
  //
  // select primaries
  //
  Double_t dcaToVertex = -1;
  if( fCutsRC->GetDCAToVertex2D() ) 
  {
      dcaToVertex = TMath::Sqrt(dca[0]*dca[0]/fCutsRC->GetMaxDCAToVertexXY()/fCutsRC->GetMaxDCAToVertexXY() + dca[1]*dca[1]/fCutsRC->GetMaxDCAToVertexZ()/fCutsRC->GetMaxDCAToVertexZ()); 
  }
  if(fCutsRC->GetDCAToVertex2D() && dcaToVertex > 1) return;
  if(!fCutsRC->GetDCAToVertex2D() && TMath::Abs(dca[0]) > fCutsRC->GetMaxDCAToVertexXY()) return;
  if(!fCutsRC->GetDCAToVertex2D() && TMath::Abs(dca[1]) > fCutsRC->GetMaxDCAToVertexZ()) return;

  Double_t vTPCTrackHisto[10] = {nClust,chi2PerCluster,clustPerFindClust,dca[0],dca[1],eta,phi,pt,q,vertStatus};
  fTPCTrackHisto->Fill(vTPCTrackHisto); 
 
  //
  // Fill rec vs MC information
  //
  if(!stack) return;
}


//_____________________________________________________________________________
void AliPerformanceTPC::ProcessConstrained(AliStack* const /*stack*/, AliESDtrack *const /*esdTrack*/, AliESDEvent* const /*esdEvent*/)
{
  // Fill comparison information (constarained parameters) 
  AliDebug(AliLog::kWarning, "Warning: Not implemented");
}


//_____________________________________________________________________________
void AliPerformanceTPC::Exec(AliMCEvent* const mcEvent, AliESDEvent *const esdEvent, AliESDfriend *const esdFriend, const Bool_t bUseMC, const Bool_t bUseESDfriend)
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
  
  // trigger
  if(!bUseMC &&GetTriggerClass()) {
    Bool_t isEventTriggered = esdEvent->IsTriggerClassFired(GetTriggerClass());
    if(!isEventTriggered) return; 
  }

  // get TPC event vertex
  const AliESDVertex *vtxESD = NULL; 
  if(fUseTrackVertex) {
    vtxESD = esdEvent->GetPrimaryVertexTracks();
  } else {
    vtxESD = esdEvent->GetPrimaryVertexTPC();
  }
  if(!vtxESD) return;

  //  events with rec. vertex
  Int_t mult=0; Int_t multP=0; Int_t multN=0;
  
  // changed to take all events but store vertex status
//  if(vtxESD->GetStatus() >0)
//  {
  // store vertex status
  Bool_t vertStatus = vtxESD->GetStatus();
  //  Process ESD events
  for (Int_t iTrack = 0; iTrack < esdEvent->GetNumberOfTracks(); iTrack++) 
  { 
    AliESDtrack *track = esdEvent->GetTrack(iTrack);
    if(!track) continue;

    if(bUseESDfriend && esdFriend && esdFriend->TestSkipBit()==kFALSE) 
    {
      AliESDfriendTrack *friendTrack=esdFriend->GetTrack(iTrack);
      if(friendTrack) 
      {
        //
        TObject *calibObject=0;
        AliTPCseed *seed=0;
        for (Int_t j=0;(calibObject=friendTrack->GetCalibObject(j));++j) {
	    if ((seed=dynamic_cast<AliTPCseed*>(calibObject))) {
	    break;
	  }
        }

        // 
	for (Int_t irow=0;irow<159;irow++) {
	if(!seed) continue;
	  
	  AliTPCclusterMI *cluster=seed->GetClusterPointer(irow);
	  if (!cluster) continue;

	     Float_t gclf[3];
	     cluster->GetGlobalXYZ(gclf);

	     //Double_t x[3]={cluster->GetRow(),cluster->GetPad(),cluster->GetTimeBin()};
	     //Int_t i[1]={cluster->GetDetector()};
             //transform->Transform(x,i,0,1);
	     //printf("gx %f gy  %f  gz %f \n", cluster->GetX(), cluster->GetY(),cluster->GetZ());
	     //printf("gclf[0] %f gclf[1]  %f  gclf[2] %f \n", gclf[0], gclf[1],  gclf[2]);
     
             Int_t TPCside; 
	     if(gclf[2]>0.) TPCside=0; // A side 
	     else TPCside=1;

	     //
             //Double_t vTPCClust1[3] = { gclf[0], gclf[1],  TPCside };
             //fTPCClustHisto1->Fill(vTPCClust1);

             //  
	     Double_t phi = TMath::ATan2(gclf[1],gclf[0]);
	     if(phi < 0) phi += 2.*TMath::Pi();
	    
             Double_t vTPCClust[3] = { irow, phi, TPCside };
             fTPCClustHisto->Fill(vTPCClust);
        }
      }
    }

    if(GetAnalysisMode() == 0) ProcessTPC(stack,track,esdEvent,vertStatus);
    else if(GetAnalysisMode() == 1) ProcessTPCITS(stack,track,esdEvent,vertStatus);
    else if(GetAnalysisMode() == 2) ProcessConstrained(stack,track,esdEvent);
    else {
      printf("ERROR: AnalysisMode %d \n",fAnalysisMode);
      return;
    }

    // TPC only
    if(!fUseHLT){
      AliESDtrack *tpcTrack = AliESDtrackCuts::GetTPCOnlyTrack(esdEvent,iTrack);
      if(!tpcTrack) continue;
      
      // track selection
      if( fCutsRC->AcceptTrack(tpcTrack) ) { 
	mult++;
	if(tpcTrack->Charge()>0.) multP++;
	if(tpcTrack->Charge()<0.) multN++;
      }
      
      if(tpcTrack) delete tpcTrack;
    }
    else
      if( fCutsRC->AcceptTrack(track) ) { 
	//Printf("Still here for HLT");
	mult++;
	if(track->Charge()>0.) multP++;
	if(track->Charge()<0.) multN++;
      }
    
  }
//  }
  //
  
  Double_t vTPCEvent[7] = {vtxESD->GetXv(),vtxESD->GetYv(),vtxESD->GetZv(),mult,multP,multN,vtxESD->GetStatus()};
  fTPCEventHisto->Fill(vTPCEvent);
}


//_____________________________________________________________________________
void AliPerformanceTPC::Analyse()
{
    //
    // Analyse comparison information and store output histograms
    // in the folder "folderTPC"
    //
    TH1::AddDirectory(kFALSE);
    TH1::SetDefaultSumw2(kFALSE);
    TObjArray *aFolderObj = new TObjArray;
    //aFolderObj->SetOwner(); // objects are owned by fanalysisFolder
    TString selString;

    //
    // Cluster histograms
    //
    AddProjection(aFolderObj, fTPCClustHisto, 0, 1, 2);
    
    selString = "all";
    for(Int_t i=0; i <= 2; i++) {
        AddProjection(aFolderObj, fTPCClustHisto, i, &selString);
    }
    
    //fTPCClustHisto->GetAxis(2)->SetRange(1,1); // A-side
    //selString = "A_side";
    //AddProjection(aFolderObj, fTPCClustHisto, 0, 1, &selString);
    
    //fTPCClustHisto->GetAxis(2)->SetRange(2,2); // C-side
    //selString = "C_side";
    //AddProjection(aFolderObj, fTPCClustHisto, 0, 1, &selString);
    
    //reset range
    fTPCClustHisto->GetAxis(2)->SetRange(1,2); 
    
    //
    // event histograms
    //
    for(Int_t i=0; i<=6; i++) {
      AddProjection(aFolderObj, fTPCEventHisto, i);
    }    
    AddProjection(aFolderObj, fTPCEventHisto, 4, 5);
    AddProjection(aFolderObj, fTPCEventHisto, 0, 1);
    AddProjection(aFolderObj, fTPCEventHisto, 0, 3);
    AddProjection(aFolderObj, fTPCEventHisto, 1, 3);
    AddProjection(aFolderObj, fTPCEventHisto, 2, 3);

    // reconstructed vertex status > 0
    fTPCEventHisto->GetAxis(6)->SetRange(2,2);
    selString = "recVertex";
    for(Int_t i=0; i<=5; i++) {
      AddProjection(aFolderObj, fTPCEventHisto, i, &selString);
    }
    AddProjection(aFolderObj, fTPCEventHisto, 4, 5, &selString);
    AddProjection(aFolderObj, fTPCEventHisto, 0, 1, &selString);
    AddProjection(aFolderObj, fTPCEventHisto, 0, 3, &selString);
    AddProjection(aFolderObj, fTPCEventHisto, 1, 3, &selString);
    AddProjection(aFolderObj, fTPCEventHisto, 2, 3, &selString);

    // reset cuts
    fTPCEventHisto->GetAxis(6)->SetRange(1,2);

    //
    // Track histograms 
    // 
    // all with vertex
    fTPCTrackHisto->GetAxis(8)->SetRangeUser(-1.5,1.5);
    fTPCTrackHisto->GetAxis(9)->SetRangeUser(0.5,1.5);
    selString = "all_recVertex";
    for(Int_t i=0; i <= 9; i++) {
        AddProjection(aFolderObj, fTPCTrackHisto, i, &selString);        
    }
    for(Int_t i=0; i <= 4; i++) {
        AddProjection(aFolderObj, fTPCTrackHisto, i, 5, 7, &selString);        
    }    



    // Track histograms (pos with vertex)
    fTPCTrackHisto->GetAxis(8)->SetRangeUser(0,1.5);
    selString = "pos_recVertex";
    for(Int_t i=0; i <= 9; i++) {
        AddProjection(aFolderObj, fTPCTrackHisto, i, &selString);
    }
    for(Int_t i=0; i <= 4; i++) { for(Int_t j=5; j <= 5; j++) { for(Int_t k=j+1; k <= 7; k++) {
        AddProjection(aFolderObj, fTPCTrackHisto, i, j, k, &selString);
    }  }  }
    AddProjection(aFolderObj, fTPCTrackHisto, 0, 1, 2, &selString);
    AddProjection(aFolderObj, fTPCTrackHisto, 0, 1, 5, &selString);
    AddProjection(aFolderObj, fTPCTrackHisto, 0, 2, 5, &selString);
    AddProjection(aFolderObj, fTPCTrackHisto, 1, 2, 5, &selString);
    AddProjection(aFolderObj, fTPCTrackHisto, 3, 4, 5, &selString);
    AddProjection(aFolderObj, fTPCTrackHisto, 5, 6, 7, &selString);
  
    // Track histograms (neg with vertex)
    fTPCTrackHisto->GetAxis(8)->SetRangeUser(-1.5,0);
    selString = "neg_recVertex";
    for(Int_t i=0; i <= 9; i++) {
        AddProjection(aFolderObj, fTPCTrackHisto, i, &selString);
    }
    for(Int_t i=0; i <= 4; i++) { for(Int_t j=5; j <= 5; j++) { for(Int_t k=j+1; k <= 7; k++) {
        AddProjection(aFolderObj, fTPCTrackHisto, i, j, k, &selString);
    }  }  }
    AddProjection(aFolderObj, fTPCTrackHisto, 0, 1, 2, &selString);
    AddProjection(aFolderObj, fTPCTrackHisto, 0, 1, 5, &selString);
    AddProjection(aFolderObj, fTPCTrackHisto, 0, 2, 5, &selString);
    AddProjection(aFolderObj, fTPCTrackHisto, 1, 2, 5, &selString);
    AddProjection(aFolderObj, fTPCTrackHisto, 3, 4, 5, &selString);

    //restore cuts
    fTPCTrackHisto->GetAxis(8)->SetRangeUser(-1.5,1.5);
    fTPCTrackHisto->GetAxis(9)->SetRangeUser(-0.5,1.5);
  
  
    printf("exportToFolder\n");
    // export objects to analysis folder
    fAnalysisFolder = ExportToFolder(aFolderObj);
    if (fFolderObj) delete fFolderObj;
    fFolderObj = aFolderObj;
    aFolderObj=0;
}


//_____________________________________________________________________________
TFolder* AliPerformanceTPC::ExportToFolder(TObjArray * array) 
{
  // recreate folder avery time and export objects to new one
  //
  AliPerformanceTPC * comp=this;
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
Long64_t AliPerformanceTPC::Merge(TCollection* const list) 
{
  // Merge list of objects (needed by PROOF)

  if (!list)
  return 0;

  if (list->IsEmpty())
  return 1;

  TIterator* iter = list->MakeIterator();
  TObject* obj = 0;
  TObjArray* objArrayList = 0;
  objArrayList = new TObjArray();

  // collection of generated histograms
  Int_t count=0;
  while((obj = iter->Next()) != 0) 
  {
    AliPerformanceTPC* entry = dynamic_cast<AliPerformanceTPC*>(obj);
    if (entry == 0) continue; 
    if (fgMergeTHnSparse) {
        if ((fTPCClustHisto) && (entry->fTPCClustHisto)) { fTPCClustHisto->Add(entry->fTPCClustHisto); }
        if ((fTPCEventHisto) && (entry->fTPCEventHisto)) { fTPCEventHisto->Add(entry->fTPCEventHisto); }
        if ((fTPCTrackHisto) && (entry->fTPCTrackHisto)) { fTPCTrackHisto->Add(entry->fTPCTrackHisto); }
    }
    // the analysisfolder is only merged if present
    if (entry->fFolderObj) { objArrayList->Add(entry->fFolderObj); }

    count++;
  }
  if (fFolderObj) { fFolderObj->Merge(objArrayList); } 
  // to signal that track histos were not merged: reset
  if (!fgMergeTHnSparse) { fTPCTrackHisto->Reset(); fTPCClustHisto->Reset(); fTPCEventHisto->Reset(); }
  // delete
  if (objArrayList)  delete objArrayList;  objArrayList=0;
return count;
}


//_____________________________________________________________________________
TFolder* AliPerformanceTPC::CreateFolder(TString name, TString title) { 
// create folder for analysed histograms
//
TFolder *folder = 0;
  folder = new TFolder(name.Data(),title.Data());

  return folder;
}


//_____________________________________________________________________________
void AliPerformanceTPC::AddProjection(TObjArray* aFolderObj, THnSparse* hSparse, Int_t xDim, TString* selString) 
{
  TH1 *h1=0;  
  TString name = "h_tpc_" + TString(hSparse->GetName())(4,5) + '_';
  if (selString) { name += *selString + '_'; }
  name.ToLower();
  name += xDim;
  TString title = hSparse->GetAxis(xDim)->GetTitle();  
  if (selString) { title += " (" + *selString + ")"; }
  h1 = hSparse->Projection(xDim);
  h1->SetName(name.Data());
  h1->GetXaxis()->SetTitle(hSparse->GetAxis(xDim)->GetTitle());
  h1->SetTitle(title.Data());  
  aFolderObj->Add(h1);
}


//_____________________________________________________________________________
void AliPerformanceTPC::AddProjection(TObjArray* aFolderObj, THnSparse *hSparse, Int_t yDim, Int_t xDim, TString* selString)
{
  TH2 *h2=0;  
  TString name = "h_tpc_" + TString(hSparse->GetName())(4,5) + '_';
  if (selString) { name += *selString + '_'; }
  name.ToLower();
  name += yDim;
  name += '_';
  name += xDim;
  TString title = hSparse->GetAxis(yDim)->GetTitle();
  title += " vs ";
  title += hSparse->GetAxis(xDim)->GetTitle();
  if (selString) { title += " (" + *selString + ")"; }  
  h2 = hSparse->Projection(yDim,xDim);
  h2->SetName(name.Data());
  h2->GetXaxis()->SetTitle(hSparse->GetAxis(xDim)->GetTitle());
  h2->GetYaxis()->SetTitle(hSparse->GetAxis(yDim)->GetTitle());
  h2->SetTitle(title.Data());  
  aFolderObj->Add(h2);
}


//_____________________________________________________________________________
void AliPerformanceTPC::AddProjection(TObjArray* aFolderObj, THnSparse *hSparse, Int_t xDim, Int_t yDim, Int_t zDim, TString* selString)
{
  TH3 *h3=0;  
  TString name = "h_tpc_" + TString(hSparse->GetName())(4,5) + '_';
  if (selString) { name += *selString + '_'; }
  name.ToLower();
  name += xDim;
  name += '_';
  name += yDim;
  name += '_';
  name += zDim;
  TString title = hSparse->GetAxis(xDim)->GetTitle();
  title += " vs ";
  title += hSparse->GetAxis(yDim)->GetTitle();
  title += " vs ";
  title += hSparse->GetAxis(zDim)->GetTitle();
  if (selString) { title += " (" + *selString + ")"; }
  h3 = hSparse->Projection(xDim,yDim,zDim);
  h3->SetName(name.Data());
  h3->GetXaxis()->SetTitle(hSparse->GetAxis(xDim)->GetTitle());
  h3->GetYaxis()->SetTitle(hSparse->GetAxis(yDim)->GetTitle());
  h3->GetZaxis()->SetTitle(hSparse->GetAxis(zDim)->GetTitle());
  h3->SetTitle(title.Data());  
  aFolderObj->Add(h3);
}
