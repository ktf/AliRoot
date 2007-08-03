/**************************************************************************
 * Copyright(c) 1998-2007, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include <TClonesArray.h>
#include "AliLog.h"
#include "AliESDVertex.h"
#include "AliRunLoader.h"
#include "AliITSLoader.h"
#include "AliITSgeomTGeo.h"
#include "AliITSRecPoint.h"
#include "AliITSVertexerCosmics.h"
#include "AliStrLine.h"

//------------------------------------------------------------------------
// This class implements a method to construct a "fake" primary
// vertex for cosmic events in which the muon crosses one of 5 inner
// ITS layers. A fake primary vertex is needed for the reconstruction,
// with e.g. AliITStrackerSA, of the two tracks produced by the muon 
// in the ITS.
//   We build pairs of clusters on a given layer and define the fake vertex as
// the mid-point of the straight line joining the two clusters.
//   We use the innermost layer that has at least two clusters.
//   We reject the background by requiring at least one cluster on the outer
// layer, closer than fMaxDistOnOuterLayer to the tracklet prolongation.
//   We can reject (potentially pathological) events with the muon track
// tangential to the layer by the requiring the radial position of
// the vertex to be smaller than fMaxVtxRadius.
//   Due to background clusters, more than one vertex per event can 
// be found. We consider the first found.
//   The errors on x,y,z of the vertex are calculated as errors on the mean
// of clusters coordinates. Non-diag elements of vertex cov. mat. are set to 0.
//   The number of contributors set in the AliESDVertex object is the
// number of the layer on which the tracklet was built; if this number is -1, 
// the procedure could not find a vertex position and by default 
// the vertex coordinates are set to (0,0,0) with large errors (100,100,100)
//
// Origin: A.Dainese, andrea.dainese@lnl.infn.it
//-------------------------------------------------------------------------

ClassImp(AliITSVertexerCosmics)

//-------------------------------------------------------------------------
AliITSVertexerCosmics::AliITSVertexerCosmics():AliITSVertexer(),
fMaxDistOnOuterLayer(0),
fMinDist2Vtxs(0)
{
  // Default constructor
  SetFirstLastModules(0,0,79);
  SetFirstLastModules(1,80,239);
  SetFirstLastModules(2,240,323);
  SetFirstLastModules(3,324,499);
  SetFirstLastModules(4,500,1247);
  SetFirstLastModules(5,1248,2197);
  SetMaxVtxRadius(0,3.5);
  SetMaxVtxRadius(1,6.5);
  SetMaxVtxRadius(2,14.5);
  SetMaxVtxRadius(3,23.5);
  SetMaxVtxRadius(4,37.5);
  SetMaxVtxRadius(5,42.5);
  SetMaxDistOnOuterLayer();
  SetMinDist2Vtxs();
}
//--------------------------------------------------------------------------
AliESDVertex* AliITSVertexerCosmics::FindVertexForCurrentEvent(Int_t evnumber) 
{
  // Defines the AliESDVertex for the current event

  fCurrentVertex = 0;
  AliRunLoader *rl =AliRunLoader::GetRunLoader();
  AliITSLoader* itsLoader = (AliITSLoader*)rl->GetLoader("ITSLoader");
  itsLoader->LoadRecPoints();
  rl->GetEvent(evnumber);

  TTree *rpTree = itsLoader->TreeR();

  TClonesArray *recpoints=new TClonesArray("AliITSRecPoint",10000);
  rpTree->SetBranchAddress("ITSRecPoints",&recpoints);

  Int_t lay,lad,det; 

  // Search for innermost layer with at least two clusters 
  // on two different modules
  Int_t ilayer=0;
  while(ilayer<6) {
    Int_t nHitModules=0;
    for(Int_t imodule=fFirst[ilayer]; imodule<=fLast[ilayer]; imodule++) {
      rpTree->GetEvent(imodule);
      AliITSgeomTGeo::GetModuleId(imodule,lay,lad,det);
      lay -= 1;  // AliITSgeomTGeo gives layer from 1 to 6, we want 0 to 5
      if(lay!=ilayer) AliFatal("Layer mismatch!");
      if(recpoints->GetEntriesFast()>0) nHitModules++;
    }
    if(nHitModules>=2) break;
    ilayer++;
  }
  printf("Building tracklets on layer %d\n",ilayer);


  Float_t xclInnLay[100],yclInnLay[100],zclInnLay[100],modclInnLay[100];
  Float_t e2xclInnLay[100],e2yclInnLay[100],e2zclInnLay[100];
  Int_t nclInnLayStored=0;
  Float_t xclOutLay[100],yclOutLay[100],zclOutLay[100],modclOutLay[100];
  Int_t nclOutLayStored=0;
  Int_t nRecPoints,nRecPointsInnLay=0;

  Float_t gc[3],gcov[5];

  Float_t x[100],y[100],z[100],e2x[100],e2y[100],e2z[100];
  Double_t p1[3],p2[3],p3[3];
  Int_t nvtxs;
  Bool_t good,matchtoOutLay;
  Float_t xvtx,yvtx,zvtx,rvtx;

  // Collect clusters in the selected layer and the outer one
  for(Int_t imodule=fFirst[ilayer]; imodule<=fLast[ilayer+1]; imodule++) {
    rpTree->GetEvent(imodule);
    AliITSgeomTGeo::GetModuleId(imodule,lay,lad,det);
    lay -= 1; // AliITSgeomTGeo gives layer from 1 to 6, we want 0 to 5
    nRecPoints=recpoints->GetEntriesFast();
    if(imodule<=fLast[ilayer]) nRecPointsInnLay += nRecPoints;
    //printf("cosmics: module %d clusters %d\n",imodule,nRecPoints);
    for(Int_t irp=0; irp<nRecPoints; irp++) {
      AliITSRecPoint *rp=(AliITSRecPoint*)recpoints->UncheckedAt(irp);
      // Local coordinates of this recpoint
      rp->GetGlobalXYZ(gc);
      if(lay==ilayer) { // store InnLay clusters
	xclInnLay[nclInnLayStored]=gc[0];
	yclInnLay[nclInnLayStored]=gc[1];
	zclInnLay[nclInnLayStored]=gc[2];
	rp->GetGlobalCov(gcov);
	e2xclInnLay[nclInnLayStored]=gcov[0];
	e2yclInnLay[nclInnLayStored]=gcov[3];
	e2zclInnLay[nclInnLayStored]=gcov[5];
	modclInnLay[nclInnLayStored]=imodule;
	nclInnLayStored++;
      }
      if(lay==ilayer+1) { // store OutLay clusters
	xclOutLay[nclOutLayStored]=gc[0];
	yclOutLay[nclOutLayStored]=gc[1];
	zclOutLay[nclOutLayStored]=gc[2];
	modclOutLay[nclOutLayStored]=imodule;
	nclOutLayStored++;
      }
      if(nclInnLayStored>100 || nclOutLayStored>100) 
	AliFatal("More than 100 clusters per layer");
    }// end clusters in a module
  }// end modules

  // build fake vertices
  nvtxs=0;
  // InnLay - first cluster
  for(Int_t i1InnLay=0; i1InnLay<nclInnLayStored; i1InnLay++) { 
    p1[0]=xclInnLay[i1InnLay]; 
    p1[1]=yclInnLay[i1InnLay]; 
    p1[2]=zclInnLay[i1InnLay];
    // InnLay - second cluster
    for(Int_t i2InnLay=i1InnLay+1; i2InnLay<nclInnLayStored; i2InnLay++) { 
      if(modclInnLay[i1InnLay]==modclInnLay[i2InnLay]) continue;
      p2[0]=xclInnLay[i2InnLay]; 
      p2[1]=yclInnLay[i2InnLay]; 
      p2[2]=zclInnLay[i2InnLay];
      // look for point on OutLay
      AliStrLine InnLayline(p1,p2,kTRUE);
      matchtoOutLay = kFALSE;
      for(Int_t iOutLay=0; iOutLay<nclOutLayStored; iOutLay++) {
	p3[0]=xclOutLay[iOutLay]; 
	p3[1]=yclOutLay[iOutLay]; 
	p3[2]=zclOutLay[iOutLay];
	//printf(" %f\n",InnLayline.GetDistFromPoint(p3));
	if(InnLayline.GetDistFromPoint(p3)<fMaxDistOnOuterLayer) 
	  { matchtoOutLay = kTRUE; break; }
      }
      if(!matchtoOutLay) continue;
      xvtx = 0.5*(xclInnLay[i1InnLay]+xclInnLay[i2InnLay]);
      yvtx = 0.5*(yclInnLay[i1InnLay]+yclInnLay[i2InnLay]);
      zvtx = 0.5*(zclInnLay[i1InnLay]+zclInnLay[i2InnLay]);
      rvtx = TMath::Sqrt(xvtx*xvtx+yvtx*yvtx);
      if(rvtx>fMaxVtxRadius[ilayer]) continue;
      good = kTRUE;
      for(Int_t iv=0; iv<nvtxs; iv++) {
	if(TMath::Sqrt((xvtx- x[iv])*(xvtx- x[iv])+
		       (yvtx- y[iv])*(yvtx- y[iv])+
		       (zvtx- z[iv])*(zvtx- z[iv])) < fMinDist2Vtxs) 
	  good = kFALSE;
      }
      if(good) {
	x[nvtxs]=xvtx;
	y[nvtxs]=yvtx;
	z[nvtxs]=zvtx;
	e2x[nvtxs]=0.25*(e2xclInnLay[i1InnLay]+e2xclInnLay[i2InnLay]);
	e2y[nvtxs]=0.25*(e2yclInnLay[i1InnLay]+e2yclInnLay[i2InnLay]);
	e2z[nvtxs]=0.25*(e2zclInnLay[i1InnLay]+e2zclInnLay[i2InnLay]);
	nvtxs++;
      }
    } // InnLay - second cluster
  } // InnLay - first cluster


  Double_t pos[3]={0.,0.,0.};
  Double_t err[3]={100.,100.,100.};
  if(nvtxs) { 
    pos[0]=x[0]; 
    pos[1]=y[0]; 
    pos[2]=z[0];
    err[0]=TMath::Sqrt(e2x[0]); 
    err[1]=TMath::Sqrt(e2y[0]); 
    err[2]=TMath::Sqrt(e2z[0]);
  }
  fCurrentVertex = new AliESDVertex(pos,err,"cosmics");
  if(nvtxs) {
    fCurrentVertex->SetNContributors(ilayer);
  } else {
    fCurrentVertex->SetNContributors(-1);
  }
  fCurrentVertex->SetTitle("cosmics fake vertex");

  if(nvtxs>=0) fCurrentVertex->Print();

  delete recpoints;
  itsLoader->UnloadRecPoints();

  return fCurrentVertex;
}  
//-------------------------------------------------------------------------
void AliITSVertexerCosmics::FindVertices()
{
  // computes the vertices of the events in the range FirstEvent - LastEvent
  AliRunLoader *rl = AliRunLoader::GetRunLoader();
  AliITSLoader* itsLoader =  (AliITSLoader*) rl->GetLoader("ITSLoader");
  itsLoader->ReloadRecPoints();
  for(Int_t i=fFirstEvent;i<=fLastEvent;i++){
    //  printf("Processing event %d\n",i);
    rl->GetEvent(i);
    FindVertexForCurrentEvent(i);
    if(fCurrentVertex){
      WriteCurrentVertex();
    }
  }
}
//-------------------------------------------------------------------------
void AliITSVertexerCosmics::PrintStatus() const 
{
  // Print current status
  printf("=======================================================\n");
  printf(" fMaxDistOnOuterLayer: %f\n",fMaxDistOnOuterLayer);
  printf(" fMaxVtxRadius[0]:  %f\n",fMaxVtxRadius[0]);
  printf(" fMinDist2Vtxs:  %f\n",fMinDist2Vtxs);
  printf("=======================================================\n");
}
//-------------------------------------------------------------------------
