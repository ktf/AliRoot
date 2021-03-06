void readDigits(int nev=-1,int evStart=0)
{

  gSystem->Load("libITSUpgradeBase");
  gSystem->Load("libITSUpgradeSim");
  gROOT->SetStyle("Plain");
  const Int_t kMaxROCycleAccept=126;
  gAlice=NULL;
  AliRunLoader* runLoader = AliRunLoader::Open("galice.root");
  runLoader->LoadgAlice();

  gAlice = runLoader->GetAliRun();

  runLoader->LoadHeader();
  runLoader->LoadKinematics();
  runLoader->LoadSDigits();
  runLoader->LoadDigits();

  AliGeomManager::LoadGeometry("geometry.root");
  AliITSUGeomTGeo* gm = new AliITSUGeomTGeo(kTRUE,kTRUE);
  //
  Int_t nLayers = gm->GetNLayers();
  Int_t nChips = gm->GetNChips();
  Int_t nbins=100;
  Int_t xmin=0;
  Int_t xmax=50000;//00*1e-09;


  TH1D **hNelSDig = new TH1D*[nLayers], **hNelDig = new TH1D*[nLayers];
  //
  for(Int_t i=0; i< nLayers; i++ ) {
    hNelSDig[i] = new TH1D(Form("hNelSDig%i",i),Form("electron distribution in SDigits [ Layer %i] ",i),nbins,xmin,xmax);
    hNelSDig[i]->SetXTitle("N electrons");
    hNelDig[i] = new TH1D(Form("hNelDig%i",i),Form("electron distribution in Digits [ Layer %i] ",i),nbins,xmin,xmax);
    hNelDig[i]->SetXTitle("N electrons");
  }
  
  AliLoader *dl = runLoader->GetDetectorLoader("ITS");


  //SDIGITS INIT
  TTree * sDigTree = 0x0;
  TClonesArray *sDigArr= new TClonesArray("AliITSMFTSDigit");

  //DIGITS INIT
  TTree * digTree = 0x0;
  TClonesArray *digArr= new TClonesArray("AliITSMFTDigitPix");

  int nevTot = (Int_t)runLoader->GetNumberOfEvents();
  printf("N Events : %i \n",nevTot);
  evStart = evStart<nevTot ? evStart : nevTot-1;
  if (evStart<0) evStart = 0;
  //
  int lastEv = nev<0 ? nevTot : evStart+nev;
  if (lastEv > nevTot) lastEv = nevTot;
  //
  printf("N Events : %i \n",(Int_t)nevTot);

  for (Int_t iEvent = evStart; iEvent < lastEv; iEvent++) {
    printf("\n Event %i \n",iEvent);
    runLoader->GetEvent(iEvent);
    AliStack *stack = runLoader->Stack();
    sDigTree=dl->TreeS();
    digTree=dl->TreeD();
    //
    if (sDigTree) sDigTree->SetBranchAddress("ITS",&sDigArr);
    digTree->SetBranchAddress("ITSDigitsPix",&digArr);

    for (int imod=0;imod<nChips;imod++) {
      if (sDigTree) sDigTree->GetEntry(imod);
      digTree->GetEntry(imod);      
      int detType = gm->GetChipChipTypeID(imod);
      AliITSMFTSegmentationPix* segm = (AliITSMFTSegmentationPix*)gm->GetSegmentationByID(detType);
      int lay,sta,ssta,mod,chip;
      int nsdig = sDigArr->GetEntries();
      int ndig  = digArr->GetEntries();
      if (ndig<1) continue;
      gm->GetChipId(imod, lay,sta,ssta,mod,chip);
      printf("\nChip %3d: (chip %2d in module:%d/substave:%1d/stave:%2d/Layer:%d) |NSDigits: %4d NDigits: %4d\n",imod,chip,mod,ssta,sta,lay,nsdig,ndig);
      //
      for (int isdig=0;isdig<nsdig;isdig++) {
	AliITSMFTSDigit *pSdig = (AliITSMFTSDigit*)sDigArr->At(isdig);
	int sdinfo = pSdig->GetUniqueID();
	UInt_t row,col;
	Int_t cycle;
	AliITSMFTSensMap::GetCell(sdinfo,segm->Npz(),segm->Npx(),kMaxROCycleAccept,col,row,cycle);
	printf("#%3d Sdigit col:%4d/row:%4d/cycle:%d generated by track %5d (%s)\t",isdig, col,row,cycle,
	       pSdig->GetTrack(0),stack->Particle(pSdig->GetTrack(0))->GetName());
	pSdig->Print();
	hNelSDig[lay]->Fill(pSdig->GetSignal()); 
      }
      //
      for (int idig=0;idig<ndig;idig++) {
	AliITSMFTDigitPix *pDig = (AliITSMFTDigitPix*)digArr->At(idig);
	printf("#%3d digit, col:%4d/row:%4d ROCycle:%d signal: %.2e,  generated by tracks ",idig,pDig->GetCoord1(),pDig->GetCoord2(),
	       pDig->GetROCycle(),pDig->GetSignalPix()); 
	for (int itr=0;itr<pDig->GetNTracks();itr++) if (pDig->GetTrack(itr)>=0) printf(" %5d",pDig->GetTrack(itr)); printf("\n");
	//
	hNelDig[lay]->Fill(pDig->GetSignalPix()); 
      }
      //
    }

  }//event loop

  TCanvas *cSd = new TCanvas("cSd","Summable Digits",1000,800);
  cSd->Divide(nLayers&0x1 ?  (nLayers/2+1) : nLayers/2,2); 
  TCanvas *cD = new TCanvas("cD","Digits",1000,800);
  cD->Divide(nLayers&0x1 ?  (nLayers/2+1) : nLayers/2,2); 

  for(Int_t ip =1; ip<=nLayers; ip++){
    cSd->cd(ip);
    hNelSDig[ip-1]->Draw();
    cD->cd(ip);
    hNelDig[ip-1]->Draw();
  }


}
