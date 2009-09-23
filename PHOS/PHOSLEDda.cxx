/*
contact: Boris.Polishchuk@cern.ch
link: see comments in the $ALICE_ROOT/PHOS/AliPHOSRcuDA1.cxx
reference run: /alice/data/2009/LHC09b_PHOS/000075883/raw/09000075883017.20.root
run type: LED
DA type: MON 
number of events needed: 1000
input files: Mod0RCU0.data Mod0RCU1.data Mod0RCU2.data Mod0RCU3.data Mod1RCU0.data Mod1RCU1.data Mod1RCU2.data Mod1RCU3.data Mod2RCU0.data Mod2RCU1.data Mod2RCU2.data Mod2RCU3.data Mod3RCU0.data Mod3RCU1.data Mod3RCU2.data Mod3RCU3.data Mod4RCU0.data Mod4RCU1.data Mod4RCU2.data Mod4RCU3.data zs.txt
Output files: PHOS_LED.root.
Trigger types used: CALIBRATION_EVENT
*/


#include "event.h"
#include "monitor.h"
extern "C" {
#include "daqDA.h"
}

#include <stdio.h>
#include <stdlib.h>

#include <TSystem.h>
#include <TROOT.h>
#include <TPluginManager.h>

#include "AliRawReader.h"
#include "AliRawReaderDate.h"
#include "AliPHOSRcuDA1.h"
#include "AliPHOSRawFitterv0.h"
#include "AliCaloAltroMapping.h"
#include "AliCaloRawStreamV3.h"
#include "AliLog.h"


/* Main routine
      Arguments: 
      1- monitoring data source
*/
int main(int argc, char **argv) {

  gROOT->GetPluginManager()->AddHandler("TVirtualStreamerInfo",
					"*",
					"TStreamerInfo",
					"RIO",
					"TStreamerInfo()");

  AliLog::SetGlobalDebugLevel(0) ;
  AliLog::SetGlobalLogLevel(AliLog::kFatal);
  
  int status;
  
  if (argc!=2) {
    printf("Wrong number of arguments\n");
    return -1;
  }

  /* Retrieve ZS parameters from DAQ DB */
  const char* zsfile = "zs.txt";
  int failZS = daqDA_DB_getFile(zsfile, zsfile);
  
  short offset,threshold;
  
  if(!failZS) {
    FILE *f = fopen(zsfile,"r");
    int scan = fscanf(f,"%d %d",&offset,&threshold);
  }
  
  /* Retrieve mapping files from DAQ DB */ 
  const char* mapFiles[20] = {
    "Mod0RCU0.data",
    "Mod0RCU1.data",
    "Mod0RCU2.data",
    "Mod0RCU3.data",
    "Mod1RCU0.data",
    "Mod1RCU1.data",
    "Mod1RCU2.data",
    "Mod1RCU3.data",
    "Mod2RCU0.data",
    "Mod2RCU1.data",
    "Mod2RCU2.data",
    "Mod2RCU3.data",
    "Mod3RCU0.data",
    "Mod3RCU1.data",
    "Mod3RCU2.data",
    "Mod3RCU3.data",
    "Mod4RCU0.data",
    "Mod4RCU1.data",
    "Mod4RCU2.data",
    "Mod4RCU3.data"
  };

  for(Int_t iFile=0; iFile<20; iFile++) {
    int failed = daqDA_DB_getFile(mapFiles[iFile], mapFiles[iFile]);
    if(failed) { 
      printf("Cannot retrieve file %s from DAQ DB. Exit.\n",mapFiles[iFile]);
      return -1;
    }
  }
  
  /* Open mapping files */
  AliAltroMapping *mapping[20];
  TString path = "./";

  path += "Mod";
  TString path2;
  TString path3;
  Int_t iMap = 0;

  for(Int_t iMod = 0; iMod < 5; iMod++) {
    path2 = path;
    path2 += iMod;
    path2 += "RCU";

    for(Int_t iRCU=0; iRCU<4; iRCU++) {
      path3 = path2;
      path3 += iRCU;
      path3 += ".data";
      mapping[iMap] = new AliCaloAltroMapping(path3.Data());
      iMap++;
    }
  }
  
  /* define data source : this is argument 1 */  
  status=monitorSetDataSource( argv[1] );
  if (status!=0) {
    printf("monitorSetDataSource() failed : %s\n",monitorDecodeError(status));
    return -1;
  }


  /* declare monitoring program */
  status=monitorDeclareMp( __FILE__ );
  if (status!=0) {
    printf("monitorDeclareMp() failed : %s\n",monitorDecodeError(status));
    return -1;
  }


  /* define wait event timeout - 1s max */
  monitorSetNowait();
  monitorSetNoWaitNetworkTimeout(1000);
  
   /* init some counters */
  int nevents_physics=0;
  int nevents_total=0;
  
  AliRawReader *rawReader = NULL;
  AliPHOSRcuDA1* dAs[5];
  
  for(Int_t iMod=0; iMod<5; iMod++) {
    dAs[iMod] = 0;
  }

  Float_t e[64][56][2];
  Float_t t[64][56][2];
  
  for(Int_t iX=0; iX<64; iX++) {
    for(Int_t iZ=0; iZ<56; iZ++) {
      for(Int_t iGain=0; iGain<2; iGain++) {
        e[iX][iZ][iGain] = 0.;
        t[iX][iZ][iGain] = 0.;
      }
    }
  }
  
  Int_t cellX    = -1;
  Int_t cellZ    = -1;
  Int_t nBunches =  0;
  Int_t nFired   = -1;
  Int_t sigStart, sigLength;
  Int_t caloFlag;
  

  TH1I fFiredCells("fFiredCells","Number of fired cells per event",100,0,1000);

  /* main loop (infinite) */
  for(;;) {
    struct eventHeaderStruct *event;
    eventTypeType eventT;
  
    /* check shutdown condition */
    if (daqDA_checkShutdown()) {break;}
    
    /* get next event (blocking call until timeout) */
    status=monitorGetEventDynamic((void **)&event);
    if (status==MON_ERR_EOF) {
      printf ("End of File detected\n");
      break; /* end of monitoring file has been reached */
    }
    
    if (status!=0) {
      printf("monitorGetEventDynamic() failed : %s\n",monitorDecodeError(status));
      break;
    }

    /* retry if got no event */
    if (event==NULL) {
      continue;
    }


    /* use event - here, just write event id to result file */
    eventT=event->eventType;
    
    if (eventT==PHYSICS_EVENT) {
      
      nFired = 0;
      
      rawReader = new AliRawReaderDate((void*)event);
      AliCaloRawStreamV3 stream(rawReader,"PHOS",mapping);
      AliPHOSRawFitterv0 fitter;
      fitter.SubtractPedestals(kTRUE); // assume that data is non-ZS

      if(!failZS) {
	fitter.SubtractPedestals(kFALSE);
	fitter.SetAmpOffset(offset);
	fitter.SetAmpThreshold(threshold);
      }
      
      while (stream.NextDDL()) {
	while (stream.NextChannel()) {
	  
	  /* Retrieve ZS parameters from data*/
	  if(failZS) {
	    short value = stream.GetAltroCFG1();
	    bool ZeroSuppressionEnabled = (value >> 15) & 0x1;
	    bool AutomaticBaselineSubtraction = (value >> 14) & 0x1;
	    if(ZeroSuppressionEnabled) {
	      offset = (value >> 10) & 0xf;
	      threshold = value & 0x3ff;
	      fitter.SubtractPedestals(kFALSE);
	      fitter.SetAmpOffset(offset);
	      fitter.SetAmpThreshold(threshold);
	    }
	  }
	  
	  cellX    = stream.GetCellX();
	  cellZ    = stream.GetCellZ();
	  caloFlag = stream.GetCaloFlag();  // 0=LG, 1=HG, 2=TRU
	  
	  if(caloFlag!=0 && caloFlag!=1) continue; //TRU data!
	  
	  // In case of oscillating signals with ZS, a channel can have several bunches
	  nBunches = 0;
	  while (stream.NextBunch()) {
	    nBunches++;
	    if (nBunches > 1) continue;
	    sigStart  = stream.GetStartTimeBin();
	    sigLength = stream.GetBunchLength();
	    fitter.SetChannelGeo(stream.GetModule(),cellX,cellZ,caloFlag);
	    fitter.Eval(stream.GetSignals(),sigStart,sigLength);
	  } // End of NextBunch()
	  
	  if(nBunches != 1) continue;
	  
	  e[cellX][cellZ][caloFlag] = fitter.GetEnergy();
	  t[cellX][cellZ][caloFlag] = fitter.GetTime();
	  
	  if(caloFlag==1 && fitter.GetEnergy()>40)
	    nFired++;
	}
	
	if(stream.GetModule()<0 || stream.GetModule()>4) continue;
	
	if(dAs[stream.GetModule()])
	  dAs[stream.GetModule()]->FillHistograms(e,t);
	else {
	  dAs[stream.GetModule()] = new AliPHOSRcuDA1(stream.GetModule(),-1,0);
	  dAs[stream.GetModule()]->FillHistograms(e,t);
	} 
	
	for(Int_t iX=0; iX<64; iX++) {
	  for(Int_t iZ=0; iZ<56; iZ++) {
	    for(Int_t iGain=0; iGain<2; iGain++) {
	      e[iX][iZ][iGain] = 0.;
	      t[iX][iZ][iGain] = 0.;
	    }
	  }
	}
	
      }
      
      fFiredCells.Fill(nFired);
      
      delete rawReader;     
      nevents_physics++;
    }
    
    nevents_total++;
    
    /* free resources */
    free(event);
    
    /* exit when last event received, no need to wait for TERM signal */
    if (eventT==END_OF_RUN) {
      printf("EOR event detected\n");
      break;
    }
  }
  
  for(Int_t i = 0; i < 4; i++) delete mapping[i];  
  
  /* Be sure that all histograms are saved */
  
  const TH2F* h2=0;
  const TH1F* h1=0;
  char localfile[128];

  Int_t nGood=0;    // >10 entries in peak
  Int_t nMax=-111;  // max. number of entries in peak
  Int_t iXmax=-1;
  Int_t iZmax=-1;
  Int_t iModMax=-1;
  
  sprintf(localfile,"PHOS_LED.root");
  TFile* f = new TFile(localfile,"recreate");

  for(Int_t iMod=0; iMod<5; iMod++) {
    if(!dAs[iMod]) continue;
  
    printf("DA1 for module %d detected.\n",iMod);
    
    for(Int_t iX=0; iX<64; iX++) {
      for(Int_t iZ=0; iZ<56; iZ++) {
	
	h1 = dAs[iMod]->GetHgLgRatioHistogram(iX,iZ); // High Gain/Low Gain ratio
	if(h1) {
	  if(h1->GetMaximum()>10.) nGood++;
	  if(h1->GetMaximum()>nMax) {
	    nMax = (Int_t)h1->GetMaximum(); iXmax=iX; iZmax=iZ; iModMax=iMod;
	  }
	  h1->Write(); 
	}
      
	for(Int_t iGain=0; iGain<2; iGain++) {
	  h2 = dAs[iMod]->GetTimeEnergyHistogram(iX,iZ,iGain); // Time vs Energy
	  if(h2) h2->Write();
	}
	
      }
    }
    
    fFiredCells.Write();    
  }
  
  f->Close();
  
  /* Store output files to the File Exchange Server */
  daqDA_FES_storeFile(localfile,"LED");
  
  printf("%d physics events of %d total processed.\n",nevents_physics,nevents_total);
  printf("%d histograms has >10 entries in maximum, max. is %d entries ",nGood,nMax);
  printf("(module,iX,iZ)=(%d,%d,%d)",iModMax,iXmax,iZmax);
  printf("\n");

  return status;
}
