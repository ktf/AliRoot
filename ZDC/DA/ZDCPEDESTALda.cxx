/*

This program reads the DAQ data files passed as argument using the monitoring library.

The program reports about its processing progress.

Messages on stdout are exported to DAQ log system.

DA for ZDC standalone pedestal runs

Contact: Chiara.Oppedisano@to.infn.it
Link: 
Run Type: STANDALONE_PEDESTAL_RUN
DA Type: LDC
Number of events needed: no constraint (tipically ~10^4)
Input Files: ZDCPedHistoLimits.dat, ZDCPedCh2FitvsOOT.dat
Output Files: ZDCPedestal.dat, ZDCChMapping.dat, ZDCPedHisto.root
Trigger Types Used: Standalone Trigger

*/
#define PEDDATA_FILE  "ZDCPedestal.dat"
#define PEDHISTO_FILE "ZDCPedHisto.root"
#define MAPDATA_FILE  "ZDCChMapping.dat"
#define HISTLIM_FILE  "ZDCPedHistoLimits.dat"
#define HIS2FIT_FILE  "ZDCPedCh2FitvsOOT.dat"

#include <stdio.h>
#include <stdlib.h>
#include <Riostream.h>
#include <math.h>

// DATE
#include <event.h>
#include <monitor.h>
#include <daqDA.h>

//ROOT
#include <TROOT.h>
#include <TPluginManager.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TF1.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TFitter.h>
#include <TMinuitMinimizer.h>
#include <TObjArray.h>

//AliRoot
#include <AliRawReaderDate.h>
#include <AliRawEventHeaderBase.h>
#include <AliZDCRawStream.h>


/* Main routine
      Arguments: list of DATE raw data files
*/
int main(int argc, char **argv) {
  

  gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
					"*",
					"TStreamerInfo",
					"RIO",
					"TStreamerInfo()"); 
  
  /* log start of process */
  printf("\n ZDC PEDESTAL program started\n");  

  /* check that we got some arguments = list of files */
  if(argc<2){
    printf("Wrong number of arguments\n");
    return -1;
  }

  //TMinuitMinimizer m; 
  gROOT->GetPluginManager()->AddHandler("ROOT::Math::Minimizer", 
  					"Minuit", 
					"TMinuitMinimizer",
      					"Minuit", 
					"TMinuitMinimizer(const char *)");
  TVirtualFitter::SetDefaultFitter("Minuit");

  //TH1::AddDirectory(0);

  int status = 0;
  int const kNModules = 9;
  int const kNChannels = 24;
  int const kNScChannels = 32;
  int kFirstADCGeo=0, kLastADCGeo=3;
      
  // *** initializations ***************************
  int iMod=-1;
  int modGeo[kNModules], modType[kNModules], modNCh[kNModules];
  for(int kl=0; kl<kNModules; kl++){
     modGeo[kl]=modType[kl]=modNCh[kl]=0;
  }
  
  int ich=0;
  int adcMod[2*kNChannels], adcCh[2*kNChannels], sigCode[2*kNChannels];
  int det[2*kNChannels], sec[2*kNChannels];
  for(int y=0; y<2*kNChannels; y++){
    adcMod[y]=adcCh[y]=sigCode[y]=det[y]=sec[y]=0;
  }
  
  int iScCh=0;
  int scMod[kNScChannels], scCh[kNScChannels], scSigCode[kNScChannels];
  int scDet[kNScChannels], scSec[kNScChannels];
  for(int y=0; y<kNScChannels; y++){
    scMod[y]=scCh[y]=scSigCode[y]=scDet[y]=scSec[y]=0;
  }
      
  int itdcCh=0;
  int tdcMod[kNScChannels], tdcCh[kNScChannels], tdcSigCode[kNScChannels];
  int tdcDet[kNScChannels], tdcSec[kNScChannels];
  for(int y=0; y<kNScChannels; y++){
    tdcMod[y]=tdcCh[y]=tdcSigCode[y]=tdcDet[y]=tdcSec[y]=-1;
  }
	

  // *** read histo limits from data file ***************************
  float rlimit[2][kNChannels];
  for(int k=0; k<kNChannels; k++){
     for(int j=0; j<2; j++) rlimit[j][k]=0;
  }
  int read = 0;
  read = daqDA_DB_getFile(HISTLIM_FILE, HISTLIM_FILE);
  if(read){
    printf("\t ERROR!!! ZDCPedHistoLimits.dat file NOT FOUND in DAQ db!  0> Exiting....\n");
    return -11;
  }
  
  FILE *fileHistLim = NULL;
  fileHistLim = fopen(HISTLIM_FILE,"r");
  if(fileHistLim==NULL) {
    printf("\t ERROR!!! Can't open ZDCPedHistoLimits.dat file!  0> Exiting....\n");
    return -12;
  }
  float readValues[2][kNChannels];
  for(int jj=0; jj<kNChannels; jj++){
     for(int ii=0; ii<2; ii++){
  	fscanf(fileHistLim,"%f",&readValues[ii][jj]);
        rlimit[ii][jj] = readValues[ii][jj];
     }
     //printf(" ch. %d limits x histo: %f %f\n",jj,rlimit[0][jj],rlimit[1][jj]);
  }
  fclose(fileHistLim);
  // *****************************************************************
  
  // *** Creating a container for the histos ***************************
  TObjArray *hList = new TObjArray(0);
  hList->SetOwner(kTRUE);

  
  // *** Creating histograms ***************************
  // --- Histograms for ADC pedestals 
  //     [22 signal channels + 2 reference PTMs]  x 2 gain chains
  //
  TH1F *hPedhg[kNChannels] = { NULL };
  TH1F *hPedOutOfTimehg[kNChannels] = { NULL };
  TH2F *hPedCorrhg[kNChannels] = { NULL };
  TH1F *hPedlg[kNChannels] = { NULL };
  TH1F *hPedOutOfTimelg[kNChannels] = { NULL };
  TH2F *hPedCorrlg[kNChannels] = { NULL };
  TProfile *hPedCorrProfhg[kNChannels] = { NULL };
  //
  TString nhistdet1  = "ZNC";
  TString nhistdet2  = "ZPC";
  TString nhistdet3  = "ZEM"; 
  TString nhistdet4  = "ZNA";
  TString nhistdet5  = "ZPA";
  TString nhistcorr  = "Corr";
  TString nhistref   = "Ref";
  TString nhistgainh = "hg";
  TString nhistgainl = "lg";
  TString nhistoot   = "OutOfTime";
  TString nprof      = "prof";

  for(int j=0; j<kNChannels; j++){
     TString hind0 = Form("%d",j);
     TString hind1 = Form("%d",j-5); 
     TString hind2 = Form("%d",j-9); 
     TString hind3 = Form("%d",j-12); 
     TString hind4 = Form("%d",j-17); 
     TString hind5 = Form("%d",j-22); 
     TString hnam0 = "Ped";
     TString hnam1 = "Ped";
     TString hnam2 = "Ped";
     TString hnam3 = "Ped";
     TString hnam4 = "Ped";
     TString hnam5 = "Ped";
     TString hnamp = "Ped";
     if(j<=4){ // ZNC
       hnam0 = nhistdet1+nhistgainh+hind0;
       hnam1 = nhistdet1+nhistgainh+nhistoot+hind0;
       hnam2 = nhistdet1+nhistgainh+nhistcorr+hind0;
       hnam3 = nhistdet1+nhistgainl+hind0;
       hnam4 = nhistdet1+nhistgainl+nhistoot+hind0;
       hnam5 = nhistdet1+nhistgainl+nhistcorr+hind0;
       hnamp = nhistdet1+nhistgainh+nhistcorr+hind0+nprof;
     }
     else if(j>=5 && j<=9){ // ZPC
       hnam0 = nhistdet2+nhistgainh+hind1;
       hnam1 = nhistdet2+nhistgainh+nhistoot+hind1;
       hnam2 = nhistdet2+nhistgainh+nhistcorr+hind1;
       hnam3 = nhistdet2+nhistgainl+hind1;
       hnam4 = nhistdet2+nhistgainl+nhistoot+hind1;
       hnam5 = nhistdet2+nhistgainl+nhistcorr+hind1;
       hnamp = nhistdet2+nhistgainh+nhistcorr+hind1+nprof;
     }
     else if(j==10 || j==11){ // ZEM
       hnam0 = nhistdet3+nhistgainh+hind2;
       hnam1 = nhistdet3+nhistgainh+nhistoot+hind2;
       hnam2 = nhistdet3+nhistgainh+nhistcorr+hind2;
       hnam3 = nhistdet3+nhistgainl+hind2;
       hnam4 = nhistdet3+nhistgainl+nhistoot+hind2;
       hnam5 = nhistdet3+nhistgainl+nhistcorr+hind2;
       hnamp = nhistdet3+nhistgainh+nhistcorr+hind2+nprof;
     }
     else if(j>=12 && j<=16){ // ZNA
       hnam0 = nhistdet4+nhistgainh+hind3;
       hnam1 = nhistdet4+nhistgainh+nhistoot+hind3;
       hnam2 = nhistdet4+nhistgainh+nhistcorr+hind3;
       hnam3 = nhistdet4+nhistgainl+hind3;
       hnam4 = nhistdet4+nhistgainl+nhistoot+hind3;
       hnam5 = nhistdet4+nhistgainl+nhistcorr+hind3;
       hnamp = nhistdet4+nhistgainh+nhistcorr+hind3+nprof;
     }
     else if(j>=17 && j<=21){ // ZPA
       hnam0 = nhistdet5+nhistgainh+hind4;
       hnam1 = nhistdet5+nhistgainh+nhistoot+hind4;
       hnam2 = nhistdet5+nhistgainh+nhistcorr+hind4;
       hnam3 = nhistdet5+nhistgainl+hind4;
       hnam4 = nhistdet5+nhistgainl+nhistoot+hind4;
       hnam5 = nhistdet5+nhistgainl+nhistcorr+hind4;
       hnamp = nhistdet5+nhistgainh+nhistcorr+hind4+nprof;
     }
     else if(j==22 || j==23){ //Reference PMs
       hnam0 = nhistref+nhistgainh+hind5;
       hnam1 = nhistref+nhistgainh+nhistoot+hind5;
       hnam2 = nhistref+nhistgainh+nhistcorr+hind5;
       hnam3 = nhistref+nhistgainl+hind5;
       hnam4 = nhistref+nhistgainl+nhistoot+hind5;
       hnam5 = nhistref+nhistgainl+nhistcorr+hind5;
       hnamp = nhistref+nhistgainh+nhistcorr+hind5+nprof;
     }
     // --- High gain chain histos
     float limit[2] = {0., 0.};
     limit[0] = rlimit[0][j];
     limit[1] = rlimit[1][j];
     //printf(" ch. %d limits x histo: %f %f\n",j,rlimit[0][j],rlimit[1][j]);
     //
     //printf("%d  creating histos %s %s %s %s %s %s %s\n", j, hnam0.Data(), hnam1.Data(), hnam2.Data(), hnam3.Data(), hnam4.Data(), hnam5.Data(), hnamp.Data());
     //
     hPedhg[j] = new TH1F(hnam0.Data(), hnam0.Data(), int(limit[1]-limit[0]), limit[0], limit[1]);
     hPedOutOfTimehg[j] = new TH1F(hnam1.Data(), hnam1.Data(), int (1.6*limit[1]-0.4*limit[0]), 0.4*limit[0], 1.6*limit[1]);
     hPedCorrhg[j] = new TH2F(hnam2.Data(), hnam2.Data(), int (1.6*limit[1]-0.4*limit[0]), 0.4*limit[0], 1.6*limit[1], int(limit[1]-limit[0]), limit[0], limit[1]);
     // --- Low gain chain histos
     hPedlg[j] = new TH1F(hnam3.Data(), hnam3.Data(), int(2*(limit[1]-limit[0])), 8.*limit[0], 8.*limit[1]);
     hPedOutOfTimelg[j] = new TH1F(hnam4.Data(), hnam4.Data(), int (2*(1.6*limit[1]-0.4*limit[0])),  8.*0.4*limit[0], 8.*1.6*limit[1]);
     hPedCorrlg[j] = new TH2F(hnam5.Data(), hnam5.Data(), int (2*(1.6*limit[1]-0.4*limit[0])),  8.*0.4*limit[0], 8.*1.6*limit[1], int(8*(limit[1]-limit[0])), 8.*limit[0], 8.*limit[1]);     
     //
     hPedCorrProfhg[j] = new TProfile(hnamp.Data(), hnamp.Data(), int (1.6*limit[1]-0.4*limit[0]), 0.4*limit[0], 1.6*limit[1]);
     
     hList->Add(hPedhg[j]);
     hList->Add(hPedOutOfTimehg[j]);
     hList->Add(hPedCorrhg[j]);
     hList->Add(hPedCorrProfhg[j]);
     hList->Add(hPedlg[j]);
     hList->Add(hPedOutOfTimelg[j]);
  }
  

  /* report progress */
  daqDA_progressReport(10);


  /* open mapping file for Shuttle */
  FILE *mapFile4Shuttle=NULL;
  mapFile4Shuttle = fopen(MAPDATA_FILE,"w");
  if(mapFile4Shuttle==NULL) {
    printf("Failed to open mapFile4Shuttle file\n");
    return -1;
  }
  
  /* open pedestal file for Shuttle */
  FILE *fileShuttle = NULL;
  fileShuttle = fopen(PEDDATA_FILE,"w");
  if(fileShuttle==NULL) {
    printf("Failed to open PEDDATA file\n");
    return -1;
  }

  /* init some counters */
  int nevents_physics=0;
  int nevents_total=0;

  /* read the data files */
  for(int n=1;n<argc;n++){
  
    struct eventHeaderStruct *event;
    eventTypeType eventT;

   
    status = monitorSetDataSource(argv[n]);
    if (status!=0) {
      printf("monitorSetDataSource() failed : %s\n",monitorDecodeError(status));
      return -1;
    }

    /* report progress */
    /* in this example, indexed on the number of files */
    daqDA_progressReport(10+80*n/argc);

    /* read the file */
    for(;;) {

      /* get next event */
      status=monitorGetEventDynamic((void **)&event);
      if(status==MON_ERR_EOF) break; /* end of monitoring file has been reached */
      if(status!=0) {
        printf("monitorGetEventDynamic() failed : %s\n",monitorDecodeError(status));
        return -1;
      }

      /* retry if got no event */
      if(event==NULL)  break;
      
      // Initalize raw-data reading and decoding
      AliRawReader *reader = new AliRawReaderDate((void*)event);
      reader->Reset();
      reader->Select("ZDC");
      //
      AliZDCRawStream *rawStreamZDC = new AliZDCRawStream(reader);
        

      /* use event - here, just write event id to result file */
      eventT=event->eventType;
      
      if(eventT==START_OF_DATA){
	
	iMod=-1; 
	ich=0; 
	iScCh=0; 
	itdcCh=0;
	  		
	rawStreamZDC->SetSODReading(kTRUE);
	
	// --------------------------------------------------------
	// --- Writing ascii data file for the Shuttle preprocessor
	if(!rawStreamZDC->Next()) printf(" \t No ZDC raw data found!! \n");
        else{
	  while((rawStreamZDC->Next())){
            if(rawStreamZDC->IsHeaderMapping()){ // mapping header
	       iMod++;
	       modGeo[iMod]  = rawStreamZDC->GetADCModule();
	       modType[iMod] = rawStreamZDC->GetModType();
	       modNCh[iMod]  = rawStreamZDC->GetADCNChannels();
	    }
            if(rawStreamZDC->IsChMapping()){ 
	      if(modType[iMod]==1){ // ADC mapping ----------------------
	        adcMod[ich]  = rawStreamZDC->GetADCModFromMap(ich);
	        adcCh[ich]   = rawStreamZDC->GetADCChFromMap(ich);
	        sigCode[ich] = rawStreamZDC->GetADCSignFromMap(ich);
	        det[ich]     = rawStreamZDC->GetDetectorFromMap(ich);
	        sec[ich]     = rawStreamZDC->GetTowerFromMap(ich);
	        ich++;
	      }
	      else if(modType[iMod]==2){ //VME scaler mapping --------------------
	        scMod[iScCh]     = rawStreamZDC->GetScalerModFromMap(iScCh);
	        scCh[iScCh]      = rawStreamZDC->GetScalerChFromMap(iScCh);
	        scSigCode[iScCh] = rawStreamZDC->GetScalerSignFromMap(iScCh);
	        scDet[iScCh]     = rawStreamZDC->GetScDetectorFromMap(iScCh);
	        scSec[iScCh]     = rawStreamZDC->GetScTowerFromMap(iScCh);
	        iScCh++;
	      }
	      else if(modType[iMod]==6 && modGeo[iMod]==4){ // ZDC TDC mapping --------------------
	        tdcMod[itdcCh]     = rawStreamZDC->GetTDCModFromMap(itdcCh);
	        tdcCh[itdcCh]      = rawStreamZDC->GetTDCChFromMap(itdcCh);
	        tdcSigCode[itdcCh] = rawStreamZDC->GetTDCSignFromMap(itdcCh);
	        itdcCh++;
	      }
	    }
 	  }
	  // Writing data on output FXS file
	  for(int is=0; is<2*kNChannels; is++){
	     fprintf(mapFile4Shuttle,"\t%d\t%d\t%d\t%d\t%d\t%d\n",
	       is,adcMod[is],adcCh[is],sigCode[is],det[is],sec[is]);
    	     //printf("  Pedestal DA -> %d ADC: mod %d ch %d, code %d det %d, sec %d\n", is,adcMod[is],adcCh[is],sigCode[is],det[is],sec[is]);
	  }
	  for(int is=0; is<kNScChannels; is++){
	     fprintf(mapFile4Shuttle,"\t%d\t%d\t%d\t%d\t%d\t%d\n",
	       is,scMod[is],scCh[is],scSigCode[is],scDet[is],scSec[is]);
 	     //printf("  Pedestal DA -> %d Scaler: mod %d ch %d, code %d det %d, sec %d\n", is,scMod[is],scCh[is],scSigCode[is],scDet[is],scSec[is]);
	  }
	  for(int is=0; is<kNScChannels; is++){
	     fprintf(mapFile4Shuttle,"\t%d\t%d\t%d\t%d\n",
	       is,tdcMod[is],tdcCh[is],tdcSigCode[is]);
 	     //if(tdcMod[is]!=-1) printf("  Mapping DA -> %d TDC: mod %d ch %d, code %d\n", is,tdcMod[is],tdcCh[is],tdcSigCode[is]);
	  }
	  for(int is=0; is<kNModules; is++){
	     fprintf(mapFile4Shuttle,"\t%d\t%d\t%d\n",
	     modGeo[is],modType[is],modNCh[is]);
	     //printf("  Pedestal DA -> Module mapping: geo %d type %d #ch %d\n", modGeo[is],modType[is],modNCh[is]);
	  }
	  
	}
        fclose(mapFile4Shuttle);
      }// SOD event
      
      else if(eventT==PHYSICS_EVENT){
	// --- Reading data header
/*        Bool_t isHeaderThere = reader->ReadHeader();
        if(isHeaderThere){
	  const AliRawDataHeader* header = reader->GetDataHeader();
          if(header){
            UChar_t message = header->GetAttributes();
            if((message & 0x20) == 0x20){ // PEDESTAL RUN
            }
            else{
              printf("WARNING!!!!!   ZDCPEDESTALda.cxx -> CDH attribute doesn't correspond to PEDESTAL RUN: %x\n", message);
              return -1;
            }
          }
          else{
            printf("\t ATTENTION! No Raw Data Header found!!!\n");
            return -1;
          }

	}
*/	
        rawStreamZDC->SetSODReading(kTRUE);

  	if(!rawStreamZDC->Next()) printf(" \t No ZDC raw data found!! \n");	
	//
	// ----- Setting ch. mapping -----
	for(int jk=0; jk<2*kNChannels; jk++){
	  //printf("ZDCPEDESTALDA.cxx ->  ch.%d mod %d, ch %d, code %d det %d, sec %d\n",
	  //    jk,adcMod[jk],adcCh[jk],sigCode[jk],det[jk],sec[jk]);
	  rawStreamZDC->SetMapADCMod(jk, adcMod[jk]);
	  rawStreamZDC->SetMapADCCh(jk, adcCh[jk]);
	  rawStreamZDC->SetMapADCSig(jk, sigCode[jk]);
	  rawStreamZDC->SetMapDet(jk, det[jk]);
	  rawStreamZDC->SetMapTow(jk, sec[jk]);
	}
	//
  	int iraw=0;
  	int RawADChg[kNChannels], RawADCoothg[kNChannels];
  	int RawADClg[kNChannels], RawADCootlg[kNChannels];
  	for(int j=0; j<kNChannels; j++){
  	   RawADChg[j]=0; RawADCoothg[j]=0;
  	   RawADClg[j]=0; RawADCootlg[j]=0;
  	}
	//
  	while(rawStreamZDC->Next()){
  	 int index=-1;
	 int detector = rawStreamZDC->GetSector(0);
	 int sector = rawStreamZDC->GetSector(1);
	 //
	 //printf(" rawData: det %d sec %d  value %d\n", detector, sector,rawStreamZDC->GetADCGain(),rawStreamZDC->GetADCValue() );
	 
  	 if((rawStreamZDC->IsADCDataWord()) && (detector!=-1) &&
            (rawStreamZDC->GetADCModule()>=kFirstADCGeo && rawStreamZDC->GetADCModule()<=kLastADCGeo)){
	  if(sector!=5){ // Physics signals
    	    if(detector==1) index = sector; // *** ZNC
	    else if(detector==2) index = sector+5; // *** ZPC
	    else if(detector==3) index = sector+9; // *** ZEM
	    else if(detector==4) index = sector+12; // *** ZNA
	    else if(detector==5) index = sector+17; // *** ZPA
	  }
	  else{ // Reference PMs
	    index = (detector-1)/3+22;
	  }
	  //
	  if(index==-1) printf("ERROR in ZDCPEDESTALda.cxx -> det %d quad %d index %d \n", 
	    detector,sector,index);
	  
	   //
	   if(iraw<2*kNChannels){ // --- In-time pedestals (1st 48 raw data)
	    if(rawStreamZDC->GetADCGain()==0){ 
	      hPedhg[index]->Fill(rawStreamZDC->GetADCValue()); 
	      RawADChg[index] = rawStreamZDC->GetADCValue();
	      //
	      //printf("\t filling histo hPedhg[%d]\n",index);
	    }
	    else{
	      hPedlg[index]->Fill(rawStreamZDC->GetADCValue()); 
	      RawADClg[index] = rawStreamZDC->GetADCValue();
	      //
	      //printf("\t filling histo hPedlg[%d]\n",index);
	    }
  	   }
  	   else{  // --- Out-of-time pedestals
  	    if(rawStreamZDC->GetADCGain()==0){
  	      hPedOutOfTimehg[index]->Fill(rawStreamZDC->GetADCValue());
  	      RawADCoothg[index] = rawStreamZDC->GetADCValue();
	      //
	      //printf("\t filling histo hPedOutOfTimehg[%d]\n",index);
  	    }
  	    else{
  	      hPedOutOfTimelg[index]->Fill(rawStreamZDC->GetADCValue());
  	      RawADCootlg[index] = rawStreamZDC->GetADCValue();
	      //
	      //printf("\t filling histo hPedOutOfTimelg[%d]\n",index);
  	    }
  	   }
  	    iraw++;
  	  }//IsADCDataWord()
        }
  	//
  	if(iraw==4*kNChannels ){ // Last ADC channel -> Filling correlation histos
  	  for(int k=0; k<kNChannels; k++){
  	    hPedCorrhg[k]->Fill(RawADCoothg[k], RawADChg[k]);
  	    hPedCorrlg[k]->Fill(RawADCootlg[k], RawADClg[k]);
  	  }
          //printf(" ev. %d -> Filling correlation histo %d\n",nevents_physics, kNChannels);
  	}
        nevents_physics++;
        //
        delete rawStreamZDC;
	rawStreamZDC=0x0;
	delete reader;
	reader=0x0;

      }//(if PHYSICS_EVENT) 
      
      /* exit when last event received, no need to wait for TERM signal */
      else if(eventT==END_OF_RUN) {
        printf(" -> EOR event detected\n\n");
        break;
      }
      
      nevents_total++;
	  
      /* free resources */
      free(event);
    
    }
  }
  
  /* Analysis of the histograms */
  //
  Double_t MeanPed[2*kNChannels], MeanPedWidth[2*kNChannels], 
   	   MeanPedOOT[2*kNChannels], MeanPedWidthOOT[2*kNChannels];
  for(int i=0; i<kNChannels; i++){
     MeanPed[i] = MeanPedWidth[i] = MeanPedOOT[i] = MeanPedWidthOOT[i] = 0.;
  }

  // --- In-time pedestals ==========================================================
  for(int i=0; i<kNChannels; i++){
     if(hPedhg[i]->GetMean() > 0){
       hPedhg[i]->Fit("gaus","Q");
       TF1 *ADCfunchg = hPedhg[i]->GetFunction("gaus");
       if(ADCfunchg){
         MeanPed[i] = ADCfunchg->GetParameter(1);
         MeanPedWidth[i] = ADCfunchg->GetParameter(2);
       }
     }
     fprintf(fileShuttle,"\t%f\t%f\n",MeanPed[i],MeanPedWidth[i]);
     //printf("\t MeanPedhg[%d] = %f\n",i, MeanPed[i]);
  }
  //
  // --- In-time pedestals ==========================================================
  for(int i=0; i<kNChannels; i++){
    if(hPedlg[i]->GetMean() > 0){
     hPedlg[i]->Fit("gaus","Q");
     TF1 *ADCfunclg = hPedlg[i]->GetFunction("gaus");
     if(ADCfunclg){
       MeanPed[i+kNChannels] = ADCfunclg->GetParameter(1);
       MeanPedWidth[i+kNChannels] = ADCfunclg->GetParameter(2);
     }
    }
    fprintf(fileShuttle,"\t%f\t%f\n",MeanPed[i+kNChannels],MeanPedWidth[i+kNChannels]);
    //printf("\t MeanPedlg[%d] = %f\n",i+kNChannels, MeanPed[i+kNChannels]);
  }
  // --- Out-of-time pedestals ==========================================================
  for(int i=0; i<kNChannels; i++){
     if(hPedOutOfTimehg[i]->GetMean() > 0){
       hPedOutOfTimehg[i]->Fit("gaus","Q");
       TF1 *ADCootfunchg = hPedOutOfTimehg[i]->GetFunction("gaus");
       if(ADCootfunchg){
         MeanPedOOT[i] = ADCootfunchg->GetParameter(1);
         MeanPedWidthOOT[i] = ADCootfunchg->GetParameter(2);
       }
     }
     fprintf(fileShuttle,"\t%f\t%f\n",MeanPedOOT[i],MeanPedWidthOOT[i]);
     //printf("\t MeanPedOOThg[%d] = %f\n",i, MeanPedOOT[i]);
  }
  //
  for(int i=0; i<kNChannels; i++){
     if(hPedOutOfTimelg[i]->GetMean() > 0){
       hPedOutOfTimelg[i]->Fit("gaus","Q");
       TF1 *ADCootfunclg = hPedOutOfTimelg[i]->GetFunction("gaus");
       if(ADCootfunclg){
         MeanPedOOT[i+kNChannels] = ADCootfunclg->GetParameter(1);
         MeanPedWidthOOT[i+kNChannels] = ADCootfunclg->GetParameter(2);
       }
     }
     fprintf(fileShuttle,"\t%f\t%f\n",MeanPedOOT[i+kNChannels],MeanPedWidthOOT[i+kNChannels]);
     //printf("\t MeanPedOOTlg[%d] = %f\n",i+kNChannels, MeanPedOOT[i+kNChannels]);
  }
  
  // --- Correlations ==========================================================
  Double_t CorrCoeff0[kNChannels], CorrCoeff1[kNChannels];
  for(int i=0; i<kNChannels;i++){
    CorrCoeff0[i] = CorrCoeff1[i] = 0.;
  }
  
  // *** checking which channels are to be fit from data file ***************************
  float is2BeFitted[kNChannels];
  for(int l=0; l<kNChannels; l++) is2BeFitted[l]=0;
  int read2 = 0;
  read2 = daqDA_DB_getFile(HIS2FIT_FILE, HIS2FIT_FILE);
  if(!read2){
    FILE *fileCh2Fit=NULL;
    fileCh2Fit = fopen(HIS2FIT_FILE,"r");
    if (fileCh2Fit==NULL) {
      printf("\t ERROR!!! Can't open ZDCPedCh2FitvsOOT.dat file!\n");
      return -10;
    }
    float readValues[2][kNChannels];
    for(int jj=0; jj<kNChannels; jj++){
       for(int ii=0; ii<2; ii++){
    	 fscanf(fileCh2Fit,"%f",&readValues[ii][jj]);
       }
       is2BeFitted[jj] = readValues[1][jj];
       //printf(" ch. %d  is2BeFitted %f\n", jj, is2BeFitted[jj]);
    }
    fclose(fileCh2Fit);
  }
  // **************************************************************************************

  for(int i=0;i<kNChannels;i++) {
    hPedCorrProfhg[i] = hPedCorrhg[i]->ProfileX(hPedCorrProfhg[i]->GetName(),-1,-1,"S");
    if(is2BeFitted[i] && (hPedCorrProfhg[i]->GetMean()>0)){
       hPedCorrProfhg[i]->Fit("pol1","FQ");
       TF1 *ffunchg = hPedCorrProfhg[i]->GetFunction("pol1");
       if(ffunchg){
         CorrCoeff0[i] = ffunchg->GetParameter(0);
         CorrCoeff1[i] = ffunchg->GetParameter(1);
       }
       //printf("\t CorrCoeff0[%d] = %f, CorrCoeff1[%d] = %f\n",i, CorrCoeff0[i], i, CorrCoeff1[i]);
     }
    fprintf(fileShuttle,"\t%d\t%f\t%f\n",i,CorrCoeff0[i],CorrCoeff1[i]);     
  }    
  //
  fclose(fileShuttle);

  //
  //gDirectory->ls("-m");
  TFile *histofile = new TFile(PEDHISTO_FILE,"RECREATE");
  histofile->cd();
  //hList->Print();
  //hList->Write("hList",TObject::kSingleKey);
  hList->Write();
    
  histofile->Close();

  /* report progress */
  daqDA_progressReport(90);
 
  /* store the result files on FES */
  // [1] File with mapping
  status = daqDA_FES_storeFile(MAPDATA_FILE, "MAPPING");
  if(status){
    printf("Failed to export mapping data file to DAQ FES\n");
    return -2;
  }

  // [2] File with pedestal data
  status = daqDA_FES_storeFile(PEDDATA_FILE, "PEDESTALDATA");
  if(status){
    printf("Failed to export pedestal data file to DAQ FES\n");
    return -3;
  }
  // [3] File with pedestal histos
  status = daqDA_FES_storeFile(PEDHISTO_FILE, "PEDESTALHISTOS");
  if(status){
    printf("Failed to export pedestal histos file to DAQ FES\n");
    return -4;
  }
  
  /* store the result files on DB */
  status = daqDA_DB_storeFile(PEDDATA_FILE, PEDDATA_FILE);  
  if(status){
    printf("Failed to store pedestal data file to DAQ DB\n");
    return -5;
  }
  
  /* report progress */
  daqDA_progressReport(100);


  return status;
}
