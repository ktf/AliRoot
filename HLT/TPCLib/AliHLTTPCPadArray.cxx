// @(#) $Id$

/**************************************************************************
 * This file is property of and copyright by the ALICE HLT Project        * 
 * ALICE Experiment at CERN, All rights reserved.                         *
 *                                                                        *
 * Authors: Kenneth Aamodt <Kenneth.Aamodt@student.uib.no>                *
 *          for The ALICE HLT Project.                                    *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/** @file   AliHLTTPCPadArray.cxx
    @author Kenneth Aamodt
    @date   
    @brief  Class containing TPC Pad objects.
*/

#if __GNUC__>= 3
using namespace std;
#endif

#include <cerrno>
#include "AliHLTTPCPadArray.h"
#include "AliHLTTPCPad.h"
#include "AliHLTStdIncludes.h"
#include "AliHLTTPCTransform.h"
#include "AliHLTTPCDigitReader.h"
#include "AliHLTTPCClusters.h"
#include <vector>
#include <sys/time.h>

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTTPCPadArray)

AliHLTTPCPadArray::AliHLTTPCPadArray()
  :
  fPatch(-1),
  fFirstRow(-1),
  fLastRow(-1),
  fThreshold(10),
  fNumberOfRows(0),
  fNumberOfPadsInRow(NULL),
  fDigitReader(NULL),
  fRowPadVector(0),
  fClusters(0)
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt
}

AliHLTTPCPadArray::AliHLTTPCPadArray(Int_t patch)
  :
  fPatch(patch),
  fFirstRow(-1),
  fLastRow(-1),
  fThreshold(10),
  fNumberOfRows(0),
  fNumberOfPadsInRow(NULL),
  fDigitReader(NULL),
  fRowPadVector(0),
  fClusters(0)
{
  // see header file for class documentation
}

AliHLTTPCPadArray::AliHLTTPCPadArray(const AliHLTTPCPadArray& srcPadArray)
  :
  fPatch(srcPadArray.fPatch),
  fFirstRow(srcPadArray.fFirstRow),
  fLastRow(srcPadArray.fLastRow),
  fThreshold(srcPadArray.fThreshold),
  fNumberOfRows(srcPadArray.fNumberOfRows),
  fNumberOfPadsInRow(srcPadArray.fNumberOfPadsInRow),
  fDigitReader(srcPadArray.fDigitReader),
  fRowPadVector(srcPadArray.fRowPadVector),
  fClusters(srcPadArray.fClusters)
{
  // see header file for class documentation
  HLTFatal("copy constructor not implemented");
}

AliHLTTPCPadArray& AliHLTTPCPadArray::operator=(const AliHLTTPCPadArray&)
{
  // see header file for class documentation
  HLTFatal("assignment operator not implemented");
  return (*this);
}

AliHLTTPCPadArray::~AliHLTTPCPadArray()
{
  // see header file for class documentation
}

Int_t AliHLTTPCPadArray::InitializeVector(){
  // see header file for class documentation

  if(fPatch>5||fPatch<0){
    HLTFatal("Patch is not set");
    return 0;
  }

  fFirstRow = AliHLTTPCTransform::GetFirstRow(fPatch);
  fLastRow = AliHLTTPCTransform::GetLastRow(fPatch);

  fNumberOfRows=fLastRow-fFirstRow+1;
  fNumberOfPadsInRow= new Int_t[fNumberOfRows];

  memset( fNumberOfPadsInRow, 0, sizeof(Int_t)*(fNumberOfRows));

  for(Int_t i=0;i<fNumberOfRows;i++){
    fNumberOfPadsInRow[i]=AliHLTTPCTransform::GetNPads(i+fFirstRow);
    fPadVector tmpRow;
    for(Int_t j=0;j<fNumberOfPadsInRow[i];j++){
      AliHLTTPCPad *tmpPad = new AliHLTTPCPad();
      tmpPad->SetID(i,j);
      tmpRow.push_back(tmpPad);
    }
    fRowPadVector.push_back(tmpRow);
  }
}

Int_t AliHLTTPCPadArray::DeInitializeVector(){
  for(Int_t i=0;i<fNumberOfRows;i++){
    for(Int_t j=0;j<fNumberOfPadsInRow[i];j++){
      delete fRowPadVector[i][j];
    }
    fRowPadVector[i].clear();
  }
  fRowPadVector.clear();
  return 1;
} 
void AliHLTTPCPadArray::SetPatch(Int_t patch){
  fPatch=patch;
}

void AliHLTTPCPadArray::SetDigitReader(AliHLTTPCDigitReader* digitReader){
  fDigitReader=digitReader;
}
Int_t AliHLTTPCPadArray::ReadData(){

  switch (fPatch){
  case 0:
    while(fDigitReader->Next()){
      fRowPadVector[fDigitReader->GetRow()-fFirstRow][fDigitReader->GetPad()]->SetDataSignal(fDigitReader->GetTime(),fDigitReader->GetSignal());
    }
    
  case 1:
    while(fDigitReader->Next()){
      fRowPadVector[fDigitReader->GetRow()-fFirstRow][fDigitReader->GetPad()]->SetDataSignal(fDigitReader->GetTime(),fDigitReader->GetSignal());      
    }
  case 2:
    while(fDigitReader->Next()){
      fRowPadVector[fDigitReader->GetRow()][fDigitReader->GetPad()]->SetDataSignal(fDigitReader->GetTime(),fDigitReader->GetSignal());
    }
  case 3:
    while(fDigitReader->Next()){
      fRowPadVector[fDigitReader->GetRow()-27][fDigitReader->GetPad()]->SetDataSignal(fDigitReader->GetTime(),fDigitReader->GetSignal());
    }
  case 4:
    while(fDigitReader->Next()){
      fRowPadVector[fDigitReader->GetRow()-54][fDigitReader->GetPad()]->SetDataSignal(fDigitReader->GetTime(),fDigitReader->GetSignal());
    }
  case 5:
    while(fDigitReader->Next()){
      fRowPadVector[fDigitReader->GetRow()-76][fDigitReader->GetPad()]->SetDataSignal(fDigitReader->GetTime(),fDigitReader->GetSignal());
    }
  }
  return 0;
}
void AliHLTTPCPadArray::FindClusters(Int_t match){
  //see header file for documentation
  Int_t nClusters=0;
  Int_t totalChargeOfPreviousCandidate=0;
  Int_t clusterChargeIsFalling=0;
  for(Int_t row=0;row<fNumberOfRows;row++){
    for(Int_t pad=0;pad<fNumberOfPadsInRow[row]-1;pad++){
      AliHLTTPCPad *tmp1=fRowPadVector[row][pad];
      AliHLTTPCPad *tmp2=fRowPadVector[row][pad+1];
      for(Int_t c1=0;c1<tmp1->fClusterCandidates.size();c1++){

	if(tmp1->fUsedClusterCandidates[c1]){
	  continue;
	}

	for(Int_t c2=0;c2<tmp2->fClusterCandidates.size();c2++){

	  if(tmp2->fUsedClusterCandidates[c2]){
	    continue;
	  }

	  Int_t diff= tmp1->fClusterCandidates[c1].fMean - tmp2->fClusterCandidates[c2].fMean;

	  if(diff < -match){
	    break;
	  }
	  if(diff <= match){

	    if((Int_t)(tmp1->fClusterCandidates[c1].fTotalCharge - tmp2->fClusterCandidates[c2].fTotalCharge)>0){
	      clusterChargeIsFalling=1;
	    }

	    tmp1->fUsedClusterCandidates[c1]=1;
	    tmp2->fUsedClusterCandidates[c2]=1;

	    AliHLTTPCClusters tmpCluster;
	    tmpCluster.fTotalCharge = tmp1->fClusterCandidates[c1].fTotalCharge;
	    tmpCluster.fPad     = tmp1->fClusterCandidates[c1].fPad;
	    tmpCluster.fPad2    = tmp1->fClusterCandidates[c1].fPad2;
	    tmpCluster.fTime    = tmp1->fClusterCandidates[c1].fTime;
	    tmpCluster.fTime2   = tmp1->fClusterCandidates[c1].fTime2;
	    tmpCluster.fRowNumber = row;

	    tmpCluster.fTotalCharge += tmp2->fClusterCandidates[c2].fTotalCharge;
	    tmpCluster.fPad     += tmp2->fClusterCandidates[c2].fPad;
	    tmpCluster.fPad2    += tmp2->fClusterCandidates[c2].fPad2;
	    tmpCluster.fTime    += tmp2->fClusterCandidates[c2].fTime;
	    tmpCluster.fTime2   += tmp2->fClusterCandidates[c2].fTime2;
	    tmpCluster.fMean         = tmp2->fClusterCandidates[c2].fMean;
	    totalChargeOfPreviousCandidate = tmp2->fClusterCandidates[c2].fTotalCharge;

	    int rowNumber=row;
	    int lastPad=pad+1;
	    nClusters++;
	    Int_t doBreak=0;
	    for(Int_t morePads=pad+2;morePads<fNumberOfPadsInRow[row];morePads++){
	      AliHLTTPCPad *tmpx=fRowPadVector[row][morePads];
	      if(morePads>lastPad+1){
		break;
	      }
	      for(Int_t cx=0;cx<tmpx->fClusterCandidates.size();cx++){
		if(tmpx->fUsedClusterCandidates[cx]){
		  continue;
		}
		Int_t diffx=tmpCluster.fMean - tmpx->fClusterCandidates[cx].fMean;
		if(diffx<-match){
		  doBreak=1;
		  break;
		}
		if(diffx <= match){
		  if((Int_t)(totalChargeOfPreviousCandidate - tmpx->fClusterCandidates[cx].fTotalCharge)>0){
		    clusterChargeIsFalling=1;
		  }

		  if(clusterChargeIsFalling&&(Int_t)(totalChargeOfPreviousCandidate - tmpx->fClusterCandidates[cx].fTotalCharge)<=0){
		    //Means we have a deconvoluted cluster.
		    totalChargeOfPreviousCandidate=0;
		    doBreak=1;
		    break;
		  }
		  
		  tmpx->fUsedClusterCandidates[cx]=1;
		  tmpCluster.fTotalCharge += tmpx->fClusterCandidates[cx].fTotalCharge;
		  tmpCluster.fPad     += tmpx->fClusterCandidates[cx].fPad;
		  tmpCluster.fPad2    += tmpx->fClusterCandidates[cx].fPad2;
		  tmpCluster.fTime    += tmpx->fClusterCandidates[cx].fTime;
		  tmpCluster.fTime2   += tmpx->fClusterCandidates[cx].fTime2;
		  tmpCluster.fMean         = tmpx->fClusterCandidates[cx].fMean;
		  lastPad=morePads;

		  totalChargeOfPreviousCandidate=tmpx->fClusterCandidates[cx].fTotalCharge;
		}
	      }
	      if(doBreak){
		break;
	      }
	    }
	    
	    if(tmpCluster.fTotalCharge<fThreshold){
	      nClusters--;
	    }
	    else{
	      //Code to look for tails, TODO insert flag.
	      UInt_t meanTime=tmpCluster.fMean;
	      if(pad>0){
		AliHLTTPCPad *tmpBefore=fRowPadVector[row][pad-1];
		//checking the fMean -1 timebin for single timebin value in the pad before the cluster
		if(meanTime>0){
		  Int_t charge =tmpBefore->GetDataSignal(meanTime-1); 
		  if(charge){
		    tmpCluster.fTotalCharge+= charge;
		    tmpCluster.fPad        += charge*(pad-1);
		    tmpCluster.fPad2       += charge*(pad-1)*(pad-1); 
		    tmpCluster.fTime       += meanTime*charge;
		    tmpCluster.fTime2      += meanTime*charge*charge;
		  } 
		}
		//checking the fMean timebin for single timebin value in the pad before the cluster
		Int_t charge2 =tmpBefore->GetDataSignal(meanTime); 
		if(charge2){
		  tmpCluster.fTotalCharge+= charge2;
		  tmpCluster.fPad        += charge2*(pad);
		  tmpCluster.fPad2       += charge2*(pad)*(pad); 
		  tmpCluster.fTime       += meanTime*charge2;
		  tmpCluster.fTime2      += meanTime*charge2*charge2;
		} 
		//checking the fMean +1 timebin for single timebin value in the pad before the cluster
		if(meanTime<AliHLTTPCTransform::GetNTimeBins()){
		  Int_t charge3 =tmpBefore->GetDataSignal(meanTime+1); 
		  if(charge3){
		    tmpCluster.fTotalCharge+= charge3;
		    tmpCluster.fPad        += charge3*(pad+1);
		    tmpCluster.fPad2       += charge3*(pad+1)*(pad+1); 
		    tmpCluster.fTime       += meanTime*charge3;
		    tmpCluster.fTime2      += meanTime*charge3*charge3;
		  } 
		}
	      }
	      
	      if(lastPad<fNumberOfPadsInRow[row]-2){
		AliHLTTPCPad *tmpAfter=fRowPadVector[row][lastPad+1];
		//checking the fMean +1 timebin for single timebin value in the pad after the cluster
		if(meanTime>0){
		  Int_t charge4 =tmpAfter->GetDataSignal(meanTime-1); 
		  if(charge4){
		    tmpCluster.fTotalCharge+= charge4;
		    tmpCluster.fPad        += charge4*(pad-1);
		    tmpCluster.fPad2       += charge4*(pad-1)*(pad-1); 
		    tmpCluster.fTime       += meanTime*charge4;
		    tmpCluster.fTime2      += meanTime*charge4*charge4;
		  } 
		}

	      
		//checking the fMean +1 timebin for single timebin value in the pad after the cluster
		Int_t charge5 =tmpAfter->GetDataSignal(meanTime); 
		if(charge5){
		  tmpCluster.fTotalCharge+= charge5;
		  tmpCluster.fPad        += charge5*(pad);
		  tmpCluster.fPad2       += charge5*(pad)*(pad); 
		  tmpCluster.fTime       += meanTime*charge5;
		  tmpCluster.fTime2      += meanTime*charge5*charge5;
		} 
		//checking the fMean +1 timebin for single timebin value in the pad after the cluster
		if(meanTime<AliHLTTPCTransform::GetNTimeBins()){
		  Int_t charge6 =tmpAfter->GetDataSignal(meanTime+1); 
		  if(charge6){
		    tmpCluster.fTotalCharge+= charge6;
		    tmpCluster.fPad        += charge6*(pad+1);
		    tmpCluster.fPad2       += charge6*(pad+1)*(pad+1); 
		    tmpCluster.fTime       += meanTime*charge6;
		    tmpCluster.fTime2      += meanTime*charge6*charge6;
		  } 
		}
	      }
	      tmpCluster.fTime= tmpCluster.fTime/tmpCluster.fTotalCharge;
	      totalChargeOfPreviousCandidate=0;
	      clusterChargeIsFalling=0;
	      tmpCluster.fRowNumber=row;
	      tmpCluster.fFirstPad=pad;

	      fClusters.push_back(tmpCluster);
	    }
	  }
	}
      }
    }
  }

  HLTInfo("Found %d clusters.",nClusters);
  // PrintClusters();
}
void AliHLTTPCPadArray::PrintClusters(){
  for(int i=0;i<fClusters.size();i++){
    cout<<"Cluster number: "<<i<<endl;
    cout<<"Row: "<<fClusters[i].fRowNumber<<" Pad: "<<fClusters[i].fFirstPad<<endl;
    cout<<"Total Charge: "<<fClusters[i].fTotalCharge<<endl;
    cout<<"PadError:     "<<fClusters[i].fPad2<<endl;
    cout<<"TimeMean:     "<<fClusters[i].fTime<<endl;
    cout<<"TimeError:    "<<fClusters[i].fTime2<<endl;
    cout<<endl;
    cout<<endl;
  }
}
