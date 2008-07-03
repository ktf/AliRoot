/**************************************************************************
 * This file is property of and copyright by the Experimental Nuclear     *
 * Physics Group, Dep. of Physics                                         *
 * University of Oslo, Norway, 2007                                       *
 *                                                                        * 
 * Author: Per Thomas Hille <perthi@fys.uio.no> for the ALICE HLT Project.*
 * Contributors are mentioned in the code where appropriate.              *
 * Please report bugs to perthi@fys.uio.no                                * 
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#ifndef __CINT__
# include <stdexcept>
# include <TSystem.h>
# include <TApplication.h>
# include "TStyle.h" 
#endif


#include  "AliHLTPHOSOnlineDisplay.h"
#include  "AliHLTDataTypes.h"
#include  "AliHLTPHOSRcuCellEnergyDataStruct.h"
#include  <vector>
#include  "stdio.h"
#include <string>
#include <sys/ipc.h>
#include <errno.h>
#include "TH2.h"
#include "TCanvas.h"
#include "AliHLTPHOSRcuCellAccumulatedEnergyDataStruct.h"
#include "AliHLTPHOSCommonDefs.h"
#include <iostream>
#include "AliHLTPHOSOnlineDisplayEventTab.h"
#include "AliHLTPHOSOnlineDisplayCalibTab.h"

AliHLTPHOSOnlineDisplayEventTab*     AliHLTPHOSOnlineDisplay::fgEventTabPtr       = 0;
AliHLTPHOSOnlineDisplayCalibTab*     AliHLTPHOSOnlineDisplay::fgCalibTabPtr       = 0;
AliHLTPHOSOnlineDisplay*             AliHLTPHOSOnlineDisplay::fgInstancePtr       = 0;          /**<The one an only instance of PhosOnlineDisplay*/
HOMERReader*                         AliHLTPHOSOnlineDisplay::fgHomerReaderPtr    = 0;          /**<Homer reader that fetches events from the HLT online stream*/
HOMERReader*                         AliHLTPHOSOnlineDisplay::fgHomerReadersPtr[MAX_HOSTS];     /**<Homer reader that fetches events from the HLT online stream*/
Bool_t                               AliHLTPHOSOnlineDisplay::fgAccumulate        = kFALSE ;    /**<If set to kFALSE reset fgLegoplot between event, kTRUE adds current energies to previous plot*/
Bool_t                               AliHLTPHOSOnlineDisplay::fgSyncronize        = kFALSE ;
unsigned int                         AliHLTPHOSOnlineDisplay::fgNHosts            = 0;
unsigned int                         AliHLTPHOSOnlineDisplay::fgNPorts            = 0;
char*                                AliHLTPHOSOnlineDisplay::fgHosts[MAX_HOSTS];
short unsigned int*                  AliHLTPHOSOnlineDisplay::fgPorts             = 0; 
TGTab*                               AliHLTPHOSOnlineDisplay::fTab                = 0;

using namespace std;


AliHLTPHOSOnlineDisplay*
AliHLTPHOSOnlineDisplay::Instance(int argc, char** argv) 
{
  if (!fgInstancePtr) fgInstancePtr = new AliHLTPHOSOnlineDisplay(argc, argv);
  return fgInstancePtr;
}


AliHLTPHOSOnlineDisplay::AliHLTPHOSOnlineDisplay() : AliHLTPHOSBase()
{
  cout << "ERROR ! level: FATAL, you cannot invoke the onlinedisplay without arguments" << endl;
}


AliHLTPHOSOnlineDisplay::AliHLTPHOSOnlineDisplay(int argc, char** argv) : AliHLTPHOSBase()
{
  ScanArguments(argc, argv);
  char **tmp;
  cout << "creating new PHOS Onlinedisplay" << endl;
  fgHomerReaderPtr = new  HOMERReader(fgNHosts, (const char**)fgHosts,  fgPorts);
  cout << "AliHLTPHOSOnlineDisplay::AliHLTPHOSOnlineDisplay: fgHomerReaderPtr =  " <<  fgHomerReaderPtr << endl;

  for(int i = 0; i <fgNHosts; i++)
    {
      fgHomerReadersPtr[i] =      new  HOMERReader(fgHosts[i], fgPorts[i]); 
    }
  InitDisplay();
}


AliHLTPHOSOnlineDisplay::~AliHLTPHOSOnlineDisplay()
{

}


void
AliHLTPHOSOnlineDisplay::InitDisplay()
{
  char tmpHistoName[256];
  char tmpChDtaName[256];

  gStyle->SetPalette(1);
  fTab = new TGTab(this, 100, 100);
  TGLayoutHints *fL1 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
					 kLHintsExpandY, 2, 2, 15, 1);
  fgEventTabPtr = new  AliHLTPHOSOnlineDisplayEventTab(this, fTab, fgHomerReaderPtr, fgHomerReadersPtr, fgNHosts);
  fgCalibTabPtr = new  AliHLTPHOSOnlineDisplayCalibTab(fTab, fgHomerReaderPtr, fgHomerReadersPtr, fgNHosts);

  AddFrame(fTab, fL1);
  MapSubwindows();
  Resize();
  SetWindowName("PHOS HLT OnlineDisplay");
  MapWindow();
  // MoveResize(100,100, 800,1000);
  MoveResize(100,100, 500,700);


}



int
AliHLTPHOSOnlineDisplay::GetNextEvent()
{
  fgEventTabPtr->GetNextEvent();
}



int
AliHLTPHOSOnlineDisplay::GetHistogram()
{
  fgCalibTabPtr->GetNextEvent(); 
}


int
AliHLTPHOSOnlineDisplay::ScanArguments(int argc, char** argv)
{
  for(int i=0; i< MAX_HOSTS; i++)
    {
      fgHosts[i] = new char[256];
    }

  fgPorts = new short unsigned[100];
  Bool_t hostIsSet = kFALSE;
  Bool_t portIsSet = kFALSE;
  int iResult=0;
  TString argument="";

  for (int i=0; i<argc && iResult>=0; i++) 
    {
      argument=argv[i];
      
      if (argument.IsNull()) 
	{
	  continue;
	}
      
      if (argument.CompareTo("-sync")==0)
	{
	  cout << "setting Synchronize to true" << endl;
	  fgSyncronize = kTRUE;
	}  
      
      if (argument.CompareTo("-acc")==0)
	{
	  cout << "setting Accumulate to true" << endl;
	  fgAccumulate = kTRUE;
	}  
      
      if (argument.CompareTo("-host")==0) 
	{
	  if(i+1 <= argc)
	    {
	      i++;
	      sprintf(fgHosts[fgNHosts],"%s", argv[i]);
	      fgNHosts ++; 
	      cout <<"fgNHosts set to "<< fgNHosts <<endl;
	      hostIsSet = kTRUE; 
	      if(i+1 <= argc)
		{
		  argument=argv[i+1];
		  if(argument.CompareTo("-port")==0)
		    {
		      i++;
		      if(i+1 <= argc)
			{
			  i++;
			  fgPorts[fgNPorts] = atoi(argv[i]);	
			  cout << "A setting port to   " << fgPorts[fgNPorts]  <<endl; 
			  fgNPorts ++;
			  portIsSet = kTRUE;
			}
		    }
		  else
		    {
		      fgPorts[fgNPorts] =  DEFAULT_EVENT_PORT;	
		      cout << "B setting port to   " << fgPorts[fgNPorts]  <<endl; 
		      fgNPorts ++;
		      portIsSet = kTRUE;
		    }
		}
	    }
	}
    }

  if(hostIsSet != kTRUE ||  portIsSet != kTRUE)
    {
      if(hostIsSet == kFALSE)
	{
	  printf("\nERROR: no hostname is specified\n");
	}
      
      if( portIsSet == kFALSE)
	{
	  printf("ERROR: no port spcified\n");
	}
      printf("\nYou must specify at least one host \n\n");
      printf("*****************************************************************\n");
      printf("\nUsage: ./onlinedisplay  -host  <hostname>   -port  <port>");
      printf("\n-port is optional, if not set  port 42001 will be used\n");
      printf("*****************************************************************\n\n\n");
      iResult = -1;
    }
 
 
  else
    {
      iResult = 0;
    }

  return iResult;
}//end ScanArguments



void 
AliHLTPHOSOnlineDisplay::Gain2Text(const int gain,  char *txt) const
{

  if(gain == LOW_GAIN)
    {
      
      sprintf(txt,"Low Gain");
    }

  else if(gain == HIGH_GAIN)
    {
      sprintf(txt,"High Gain");
    }

  else
    {
      sprintf(txt,"Error!! invalid gain %d", gain);
    }
  
}
