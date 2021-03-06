#include "TChain.h"
#include "TTree.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TObjArray.h"

#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"

#include "AliESDEvent.h"
#include "AliESDInputHandler.h"

#include "AliAnalysisTaskT0QA.h"

//#include "AliCDBMetaData.h"
//#include "AliCDBId.h"
//#include "AliCDBEntry.h"
//#include "AliCDBManager.h"
//#include "AliCDBStorage.h"

// Task should calculate channels offset 
// Authors: Alla 

ClassImp(AliAnalysisTaskT0QA)
//________________________________________________________________________
AliAnalysisTaskT0QA::AliAnalysisTaskT0QA() 
  : AliAnalysisTaskSE(),  fESD(0x0), fTzeroObject(0x0),
  fTzeroORA(0x0), fTzeroORC(0x0), fResolution(0x0), fTzeroORAplusORC(0x0),
    fRunNumber(0),fTimeVSAmplitude(0x0),fCFDVSPmtId(0x0),fSPDVertexVST0Vertex(0x0),
    fOrAvsNtracks(0), fOrCvsNtracks(0), fT0vsNtracks(0) 
{
  // Constructor

  // Define input and output slots here
  // Input slot #0 works with a TChain

// ########### NEVER define slots in the IO constructor
//  DefineInput(0,  TChain::Class());
//  DefineOutput(1, TObjArray::Class());
}


//________________________________________________________________________
AliAnalysisTaskT0QA::AliAnalysisTaskT0QA(const char *name) 
  : AliAnalysisTaskSE(name),  fESD(0x0), fTzeroObject(0x0),
  fTzeroORA(0x0), fTzeroORC(0x0), fResolution(0x0), fTzeroORAplusORC(0x0),
    fRunNumber(0),fTimeVSAmplitude(0x0),fCFDVSPmtId(0x0),fSPDVertexVST0Vertex(0x0),
    fOrAvsNtracks(0), fOrCvsNtracks(0), fT0vsNtracks(0) 
{
  // Constructor
  // Define input and output slots here
  // Input slot #0 works with a TChain
  DefineInput(0, TChain::Class());
  DefineOutput(1, TObjArray::Class());
  // Output slot #0 id reserved by the base class for AOD
  // Output slot #1 writes into a TH1 container
}

//________________________________________________________________________
AliAnalysisTaskT0QA::~AliAnalysisTaskT0QA() 
{
  // Destructor
  // printf("AliT0CalibOffsetChannels~AliT0CalibOffsetChannels() ");
  delete fTzeroORA;
  delete fTzeroORC;
  delete fResolution;
  delete fTzeroORAplusORC;
  delete [] fTimeVSAmplitude;
  delete fCFDVSPmtId;
  delete fSPDVertexVST0Vertex;
  delete  fOrAvsNtracks;
  delete fOrCvsNtracks;
  delete  fT0vsNtracks; 

  delete fTzeroObject;
}

//------------------------------------------------------------------
void AliAnalysisTaskT0QA::UserCreateOutputObjects()
{
  // Create histograms
 fTimeVSAmplitude = new TH2F*[NPMT0];

 for (Int_t i=0; i<NPMT0; i++) {
    fTimeVSAmplitude[i]= new TH2F (Form("fTimeVSAmplitude%d",i+1),"fTimeVsAmplitude",60, -10, 50,500,6000,7000);
  }

  fTzeroORAplusORC = new TH1F("fTzeroORAplusORC","ORA+ORC /2",100,-2000,2000);   //or A plus or C 
  fResolution      = new TH1F("fResolution","fResolution",100,-2000,2000);// or A minus or C spectrum
  fTzeroORA        = new TH1F("fTzeroORA","fTzeroORA",100,-2000,2000);// or A spectrum
  fTzeroORC        = new TH1F("fTzeroORC","fTzeroORC",100,-2000,2000);// or C spectrum
  fCFDVSPmtId      = new TH2F("fCFDVSPmtId","fCFDVSPmtId",24,0,24,250,6000,7000);  // 
  fSPDVertexVST0Vertex = new TH2F("fSPDVertexVST0Vertex","fSPDVertexVST0Vertex",30,-30,30,30,-30,30);
   fOrAvsNtracks = new TH2F("fAvstracks", "Avstracks",200, 0, 1000, 500, -1000, 1000);
  fOrCvsNtracks = new TH2F("fCvstracks", "Cvstracks",200, 0, 1000, 500, -1000, 1000);
   fT0vsNtracks = new TH2F("fT0ACvstrackles", "T0ACvstracks",200, 0, 1000, 500, -1000, 1000); 

  fTzeroObject     = new TObjArray(0);
  fTzeroObject->SetOwner(kTRUE);
  
  for (Int_t i=0; i<24; i++)
    fTzeroObject->AddAtAndExpand(fTimeVSAmplitude[i],i);

  fTzeroObject->AddAtAndExpand(fCFDVSPmtId,24);
  fTzeroObject->AddAtAndExpand(fSPDVertexVST0Vertex,25);
  fTzeroObject->AddAtAndExpand(fTzeroORAplusORC, 26);
  fTzeroObject->AddAtAndExpand(fResolution, 27);
  fTzeroObject->AddAtAndExpand(fTzeroORA, 28);
  fTzeroObject->AddAtAndExpand(fTzeroORC, 29);
  fTzeroObject->AddAtAndExpand(fT0vsNtracks, 30);
  fTzeroObject->AddAtAndExpand(fOrAvsNtracks,31);
  fTzeroObject->AddAtAndExpand(fOrCvsNtracks, 32);

  PostData(1, fTzeroObject);
  // Called once
}

//________________________________________________________________________
void AliAnalysisTaskT0QA::UserExec(Option_t *) 
{
  // Main loop
  // Called for each event

  // Post output data.
  fESD = dynamic_cast<AliESDEvent*>(InputEvent());
  if (!fESD) {
    printf("ERROR: fESD not available\n");
    return;
  }

  fRunNumber =  fESD->GetRunNumber() ; 

  const Double32_t* time = fESD->GetT0time();
  const Double32_t* amplitude = fESD->GetT0amplitude();

 
  for (Int_t i=0; i<24; i++) {
    if(time[i]<9999 &&TMath::Abs(time[i])>1e-8 && amplitude[i]<9999&&TMath::Abs(amplitude[i])>1e-8 )
      {
	//	cout<<"time "<<time[i]<<" amplitude "<<amplitude[i]<<endl;
	fTimeVSAmplitude[i]->Fill(amplitude[i],time[i]);
	fCFDVSPmtId->Fill(i,time[i]);
      }
  }

  const Double32_t* mean = fESD->GetT0TOF();
  Double32_t orA = mean[1];
  Double32_t orC = mean[2];
  Int_t ntracks = fESD->GetNumberOfTracks(); 

  if(orA<9999){
    fTzeroORA->Fill(orA);
    fOrAvsNtracks->Fill(ntracks, orA);
  }
  if(orC<9999) {
    fTzeroORC->Fill(orC);
    fOrCvsNtracks->Fill(ntracks, orC);
  }
  if(orA<9999 && orC<9999) {
    fResolution->Fill((orA-orC)/2.);
    fTzeroORAplusORC->Fill(mean[0]);
    fT0vsNtracks->Fill(ntracks, mean[0]);
  }
  
  Double32_t t0vertex = fESD->GetT0zVertex();
  //  cout << "t0 vertex "<<t0vertex<<endl;
  Double32_t esdzvertex;
  const AliESDVertex * esdvertex = fESD->GetPrimaryVertex();
  Int_t nofcontrib=-1;
  if(esdvertex && t0vertex<999)
    {
      nofcontrib=esdvertex->GetNContributors();
      if(nofcontrib>1)
	{
	  esdzvertex=esdvertex->GetZ();
	  //	  cout << "esd vertex "<<esdzvertex<<endl;
	  fSPDVertexVST0Vertex->Fill(t0vertex,esdzvertex);
	}
    }
  printf("%f   %f  %f\n",orA,orC,time);
  PostData(1, fTzeroObject);
}      
 //________________________________________________________________________
void AliAnalysisTaskT0QA::Terminate(Option_t *) 
{
  
   // Called once at the end of the query
}
