/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
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


#include "AliRICHClusterFinder.h"
#include "AliRun.h"
#include "AliRICH.h"
#include "AliRICHMap.h"
#include "AliRICHSDigit.h"
#include "AliRICHDigit.h"
#include "AliRICHRawCluster.h"
#include "AliRICHParam.h"
#include <TTree.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TF1.h>
#include <TPad.h>
#include <TGraph.h> 
#include <TMinuit.h>

static AliSegmentation     *gSegmentation;
static AliRICHResponse*     gResponse;
static Int_t                gix[500];
static Int_t                giy[500];
static Float_t              gCharge[500];
static Int_t                gNbins;
static Bool_t               gFirst=kTRUE;
static TMinuit *gMyMinuit ;
void fcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t);
static Int_t                gChargeTot;

ClassImp(AliRICHClusterFinder)
//__________________________________________________________________________________________________
AliRICHClusterFinder::AliRICHClusterFinder(AliRICH *pRICH)   
{//main ctor
  Info("main ctor","Start.");
  
  fRICH=pRICH;
  
  fSegmentation=Rich()->C(1)->GetSegmentationModel();
  fResponse    =Rich()->C(1)->GetResponseModel();
    
  fDigits=0;    fNdigits=0;
  fChamber=0;
  fHitMap=0;  
  
  fCogCorr = 0;
  SetNperMax();
  SetClusterSize();
  fNPeaks=-1;
}//main ctor
//__________________________________________________________________________________________________
void AliRICHClusterFinder::Decluster(AliRICHRawCluster *cluster)
{// Decluster algorithm
  Info("Decluster","Start.");    
  if(cluster->fMultiplicity==1||cluster->fMultiplicity==2){//Nothing special for 1- and 2-clusters
    if(fNPeaks != 0) {cluster->fNcluster[0]=fNPeaks; cluster->fNcluster[1]=0;} 
    AddRawCluster(*cluster); 
    fNPeaks++;
  }else if(cluster->fMultiplicity==3){// 3-cluster, check topology
      Centered(cluster);// ok, cluster is centered and added in Centered()
  }else{//4-and more-pad clusters
    if(cluster->fMultiplicity<= fMaxClusterSize){
        SplitByLocalMaxima(cluster);
    }//if <= fClusterSize
  }//if multiplicity 
}//Decluster()
//__________________________________________________________________________________________________
Bool_t AliRICHClusterFinder::Centered(AliRICHRawCluster *cluster)
{//Is the cluster centered?

  AliRICHDigit* dig;
  dig= (AliRICHDigit*)fDigits->UncheckedAt(cluster->fIndexMap[0]);
  Int_t x[kMaxNeighbours], y[kMaxNeighbours], xN[kMaxNeighbours], yN[kMaxNeighbours];
  Int_t nn=Rich()->Param()->PadNeighbours(dig->PadX(),dig->PadY(),x,y);
    
  
  Int_t nd=0;
  for (Int_t i=0; i<nn; i++){//neighbours loop
    if(fHitMap->TestHit(x[i],y[i]) == kUsed){
      xN[nd]=x[i];
      yN[nd]=y[i];
      nd++;
    }
  }//neighbours loop
    
  if(nd==2){// cluster is centered !
	if (fNPeaks != 0) {
            cluster->fNcluster[0]=fNPeaks;
            cluster->fNcluster[1]=0;
        }  
	cluster->fCtype=0;
	AddRawCluster(*cluster);
	fNPeaks++;
	return kTRUE;
    } else if (nd ==1) {
// Highest signal on an edge, split cluster into 2+1
// who is the neighbour ?            
	Int_t nind=fHitMap->GetHitIndex(xN[0], yN[0]);
	Int_t i1= (nind==cluster->fIndexMap[1]) ? 1:2;
	Int_t i2= (nind==cluster->fIndexMap[1]) ? 2:1;    
// 2-cluster
	AliRICHRawCluster cnew;
	if (fNPeaks == 0) {
            cnew.fNcluster[0]=-1;
            cnew.fNcluster[1]=fNRawClusters;
        } else {
            cnew.fNcluster[0]=fNPeaks;
            cnew.fNcluster[1]=0;
        }
	cnew.fMultiplicity=2;
	cnew.fIndexMap[0]=cluster->fIndexMap[0];
	cnew.fIndexMap[1]=cluster->fIndexMap[i1];
	FillCluster(&cnew);
	cnew.fClusterType=cnew.PhysicsContribution();
	AddRawCluster(cnew);
        fNPeaks++;
// 1-cluster
	cluster->fMultiplicity=1;
	cluster->fIndexMap[0]=cluster->fIndexMap[i2];
	cluster->fIndexMap[1]=0;
	cluster->fIndexMap[2]=0;	
	FillCluster(cluster);
        if (fNPeaks != 0) {
            cluster->fNcluster[0]=fNPeaks;
            cluster->fNcluster[1]=0;
        }  
	cluster->fClusterType=cluster->PhysicsContribution();
	AddRawCluster(*cluster);
	fNPeaks++;
	return kFALSE;
    } else {
	Warning("Centered","\n Completely screwed up %d !! \n",nd);
	
    }    
  return kFALSE;
}//Centered()
//__________________________________________________________________________________________________
void AliRICHClusterFinder::SplitByLocalMaxima(AliRICHRawCluster *c)
{// Split the cluster according to the number of maxima inside
  Info("SplitbyLocalMaxima","Start.");
  
      AliRICHDigit* dig[100], *digt;
    Int_t ix[100], iy[100], q[100];
    Double_t x[100], y[100];
    Int_t i; // loops over digits
    Int_t j; // loops over local maxima
    Int_t mul=c->fMultiplicity;
//  dump digit information into arrays
  for (i=0; i<mul; i++){
    dig[i]= (AliRICHDigit*)fDigits->UncheckedAt(c->fIndexMap[i]);
    ix[i]= dig[i]->PadX();
    iy[i]= dig[i]->PadY();
    q[i] = dig[i]->Signal();
    AliRICHParam::Pad2Loc(ix[i], iy[i], x[i], y[i]);
  }
//  Find local maxima
    Bool_t isLocal[100];
    Int_t nLocal=0;
    Int_t associatePeak[100];
    Int_t indLocal[100];
    Int_t nn;
    Int_t xNei[kMaxNeighbours], yNei[kMaxNeighbours];
    for (i=0; i<mul; i++) {
	fSegmentation->Neighbours(ix[i], iy[i], &nn, xNei, yNei);
	isLocal[i]=kTRUE;
	for (j=0; j<nn; j++) {
	    if (fHitMap->TestHit(xNei[j], yNei[j])==kEmpty) continue;
	    digt=(AliRICHDigit*) fHitMap->GetHit(xNei[j], yNei[j]);
	    if (digt->Signal() > q[i]) {
		isLocal[i]=kFALSE;
		break;
// handle special case of neighbouring pads with equal signal
	    } else if (digt->Signal() == q[i]) {
		if (nLocal >0) {
		    for (Int_t k=0; k<nLocal; k++) {
			if (xNei[j]==ix[indLocal[k]] && yNei[j]==iy[indLocal[k]]){
			    isLocal[i]=kFALSE;
			}
		    }
		}
	    } 
	} // loop over next neighbours
	// Maxima should not be on the edge
	if (isLocal[i]) {
	    indLocal[nLocal]=i;
	    nLocal++;
	} 
    } // loop over all digits
// If only one local maximum found but multiplicity is high take global maximum from the list of digits.    
    if (nLocal==1 && mul>5) {
	Int_t nnew=0;
	for (i=0; i<mul; i++) {
	    if (!isLocal[i]) {
		indLocal[nLocal]=i;
		isLocal[i]=kTRUE;
		nLocal++;
		nnew++;
	    }
	    if (nnew==1) break;
	}
    }
    if(nLocal==2) {// If number of local maxima is 2 try to fit a double gaussian

//  Initialise global variables for fit
	gFirst=kTRUE;
	gSegmentation=fSegmentation;
	gResponse    =fResponse;
	gNbins=mul;
	
	for (i=0; i<mul; i++) {
	    gix[i]=ix[i];
	    giy[i]=iy[i];
	    gCharge[i]=Float_t(q[i]);
	}
	if (gFirst)    gMyMinuit = new TMinuit(5);
	
	gMyMinuit->SetFCN(fcn);
	gMyMinuit->mninit(5,10,7);
	Double_t arglist[20];
	arglist[0]=1;
// Set starting values 
	static Double_t vstart[5];
	vstart[0]=x[indLocal[0]];
	vstart[1]=y[indLocal[0]];	
	vstart[2]=x[indLocal[1]];
	vstart[3]=y[indLocal[1]];	
	vstart[4]=Float_t(q[indLocal[0]])/Float_t(q[indLocal[0]]+q[indLocal[1]]);
// lower and upper limits
	static Double_t lower[5], upper[5];
	lower[0]=vstart[0]-AliRICHParam::PadSizeX()/2;
	lower[1]=vstart[1]-AliRICHParam::PadSizeY()/2;
	
	upper[0]=vstart[0]+AliRICHParam::PadSizeX()/2;
	upper[1]=vstart[1]+AliRICHParam::PadSizeY()/2;
	
	lower[2]=vstart[2]-AliRICHParam::PadSizeX()/2;
	lower[3]=vstart[3]-AliRICHParam::PadSizeY()/2;
	
	upper[2]=vstart[2]+AliRICHParam::PadSizeX()/2;
	upper[3]=vstart[3]+AliRICHParam::PadSizeY()/2;
	
	lower[4]=0.;
	upper[4]=1.;
// step sizes
	static Double_t step[5]={0.005, 0.03, 0.005, 0.03, 0.01};
	Int_t iErr;
	
	gMyMinuit->mnparm(0,"x1",vstart[0],step[0],lower[0],upper[0],iErr);
	gMyMinuit->mnparm(1,"y1",vstart[1],step[1],lower[1],upper[1],iErr);
	gMyMinuit->mnparm(2,"x2",vstart[2],step[2],lower[2],upper[2],iErr);
	gMyMinuit->mnparm(3,"y2",vstart[3],step[3],lower[3],upper[3],iErr);
	gMyMinuit->mnparm(4,"a0",vstart[4],step[4],lower[4],upper[4],iErr);
// ready for minimisation	
	gMyMinuit->SetPrintLevel(-1);
	gMyMinuit->mnexcm("SET OUT", arglist, 0, iErr);
	arglist[0]= -1;
	arglist[1]= 0;
	
	gMyMinuit->mnexcm("SET NOGR", arglist, 0, iErr);
	gMyMinuit->mnexcm("SIMPLEX", arglist, 0, iErr);
	gMyMinuit->mnexcm("MIGRAD", arglist, 0, iErr);
	gMyMinuit->mnexcm("EXIT" , arglist, 0, iErr);

	Double_t xrec[2], yrec[2], qfrac;
	TString chname;
	Double_t epxz, b1, b2;
	gMyMinuit->mnpout(0, chname, xrec[0], epxz, b1, b2, iErr);	
	gMyMinuit->mnpout(1, chname, yrec[0], epxz, b1, b2, iErr);	
	gMyMinuit->mnpout(2, chname, xrec[1], epxz, b1, b2, iErr);	
	gMyMinuit->mnpout(3, chname, yrec[1], epxz, b1, b2, iErr);	
	gMyMinuit->mnpout(4, chname, qfrac,   epxz, b1, b2, iErr);	
        
        cout<<"xrex[0]="<<xrec[0]<<"yrec[0]="<<yrec[0]<<"xrec[1]="<<xrec[1]<<"yrec[1]="<<yrec[1]<<"qfrac="<<qfrac<<endl;
	for (j=0; j<2; j++) { // One cluster for each maximum
	    AliRICHRawCluster cnew;
	    if (fNPeaks == 0) {
		cnew.fNcluster[0]=-1;
		cnew.fNcluster[1]=fNRawClusters;
	    } else {
		cnew.fNcluster[0]=fNPeaks;
		cnew.fNcluster[1]=0;
	    }
	    cnew.fMultiplicity=0;
	    cnew.fX=Float_t(xrec[j]);
	    cnew.fY=Float_t(yrec[j]);
	    if (j==0) {
		cnew.fQ=Int_t(gChargeTot*qfrac);
	    } else {
		cnew.fQ=Int_t(gChargeTot*(1-qfrac));
	    }
	    for (i=0; i<mul; i++) {
		cnew.fIndexMap[cnew.fMultiplicity]=c->fIndexMap[i];
                TVector3 x3(xrec[j],yrec[j],0);
		cnew.fContMap[cnew.fMultiplicity]=AliRICHParam::Loc2PadFrac(x3,gix[i], giy[i]);
		cnew.fMultiplicity++;
	    }
	    FillCluster(&cnew,0);
	    cnew.fClusterType=cnew.PhysicsContribution();
	    AddRawCluster(cnew);
	    fNPeaks++;
	}
    }//if 2 maximum in cluster
    Bool_t fitted=kTRUE;

    if (nLocal >2 || !fitted) {
	// Check if enough local clusters have been found, if not add global maxima to the list 
	Int_t nPerMax;
	if (nLocal!=0) {
	    nPerMax=mul/nLocal;
	} else {
	    Warning("SplitByLocalMaxima","no local maximum found");
	    nPerMax=fNperMax+1;
	}
	
	if (nPerMax > fNperMax) {
	    Int_t nGlob=mul/fNperMax-nLocal+1;
	    if (nGlob > 0) {
		Int_t nnew=0;
		for (i=0; i<mul; i++) {
		    if (!isLocal[i]) {
			indLocal[nLocal]=i;
			isLocal[i]=kTRUE;
			nLocal++;
			nnew++;
		    }
		    if (nnew==nGlob) break;
		}
	    }
	}
	for (i=0; i<mul; i++) {	// Associate hits to peaks
	    Float_t dmin=1.E10;
	    Float_t qmax=0;
	    if (isLocal[i]) continue;
	    for (j=0; j<nLocal; j++) {
		Int_t il=indLocal[j];
		Float_t d=TMath::Sqrt((x[i]-x[il])*(x[i]-x[il])
				      +(y[i]-y[il])*(y[i]-y[il]));
		Float_t ql=q[il];
		if (d<dmin) {		// Select nearest peak
		    dmin=d;
		    qmax=ql;
		    associatePeak[i]=j;
		} else if (d==dmin) {		    // If more than one take highest peak
		    if (ql>qmax) {
			dmin=d;
			qmax=ql;
			associatePeak[i]=j;
		    }
		}
	    }
	}	
 // One cluster for each maximum
	for (j=0; j<nLocal; j++) {
	    AliRICHRawCluster cnew;
	    if (fNPeaks == 0) {
		cnew.fNcluster[0]=-1;
		cnew.fNcluster[1]=fNRawClusters;
	    } else {
		cnew.fNcluster[0]=fNPeaks;
		cnew.fNcluster[1]=0;
	    }
	    cnew.fIndexMap[0]=c->fIndexMap[indLocal[j]];
	    cnew.fMultiplicity=1;
	    for (i=0; i<mul; i++) {
		if (isLocal[i]) continue;
		if (associatePeak[i]==j) {
		    cnew.fIndexMap[cnew.fMultiplicity]=c->fIndexMap[i];
		    cnew.fMultiplicity++;
		}
	    }
	    FillCluster(&cnew);
	    cnew.fClusterType=cnew.PhysicsContribution();
	    AddRawCluster(cnew);
	    fNPeaks++;
	}
    }
}//SplitByLocalMaxima(AliRICHRawCluster *c)
//__________________________________________________________________________________________________
void  AliRICHClusterFinder::FillCluster(AliRICHRawCluster* c, Int_t flag) 
{//  Completes cluster information starting from list of digits
    AliRICHDigit* dig;
    Double_t x, y;
    Int_t  ix, iy;
    Float_t fraction=0;
    
    c->fPeakSignal=0;
    if (flag) {
	c->fX=0;
	c->fY=0;
	c->fQ=0;
    }
 

    for (Int_t i=0; i<c->fMultiplicity; i++){
	dig= (AliRICHDigit*)fDigits->UncheckedAt(c->fIndexMap[i]);
	ix=dig->PadX();
	iy=dig->PadY();
	Int_t q=dig->Signal();
	if (dig->Physics() >= dig->Signal()) {
	  c->fPhysicsMap[i]=2;
	} else if (dig->Physics() == 0) {
	  c->fPhysicsMap[i]=0;
	} else  c->fPhysicsMap[i]=1;
// peak signal and track list
	if (flag) {
	   if (q>c->fPeakSignal) {
	      c->fPeakSignal=q;
	    c->fTracks[0]=dig->Hit();
	    c->fTracks[1]=dig->Track(0);
	    c->fTracks[2]=dig->Track(1);
	   }
	} else {
	   if (c->fContMap[i] > fraction) {
              fraction=c->fContMap[i];
	      c->fPeakSignal=q;
	    c->fTracks[0]=dig->Hit();
	    c->fTracks[1]=dig->Track(0);
	    c->fTracks[2]=dig->Track(1);
	   }
	}
	if (flag) {
	    AliRICHParam::Pad2Loc(ix,iy,x,y);
	    c->fX += q*x;
	    c->fY += q*y;
	    c->fQ += q;
	}

    } // loop over digits

 if (flag) {
     
     c->fX/=c->fQ;
     c->fX=fSegmentation->GetAnod(c->fX);
     c->fY/=c->fQ; 
//  apply correction to the coordinate along the anode wire
     x=c->fX;   
     y=c->fY;
     AliRICHParam::Loc2Pad(x,y,ix,iy);
     AliRICHParam::Pad2Loc(ix,iy,x,y);
     Int_t isec=fSegmentation->Sector(ix,iy);
     TF1* cogCorr = fSegmentation->CorrFunc(isec-1);
     
     if (cogCorr) {
	 Float_t yOnPad=(c->fY-y)/fSegmentation->Dpy(isec);
	 c->fY=c->fY-cogCorr->Eval(yOnPad, 0, 0);
     }
 }
}//FillCluster() 
//__________________________________________________________________________________________________
void  AliRICHClusterFinder::AddDigit2Cluster(Int_t i, Int_t j, AliRICHRawCluster &c)
{//Find clusters Add i,j as element of the cluster  
  Info("AddDigit2Cluster","Start with digit(%i,%i)",i,j);
  
  Int_t idx = fHitMap->GetHitIndex(i,j);
  AliRICHDigit* dig = (AliRICHDigit*) fHitMap->GetHit(i,j);
  Int_t q=dig->Signal();
  if(q>TMath::Abs(c.fPeakSignal)){
	c.fPeakSignal=q;
	c.fTracks[0]=dig->Hit();
	c.fTracks[1]=dig->Track(0);
	c.fTracks[2]=dig->Track(1);
    }
//  Make sure that list of digits is ordered 
    Int_t mu=c.fMultiplicity;
    c.fIndexMap[mu]=idx;

    if (dig->Physics() >= dig->Signal()) {
        c.fPhysicsMap[mu]=2;
    } else if (dig->Physics() == 0) {
        c.fPhysicsMap[mu]=0;
    } else  c.fPhysicsMap[mu]=1;

    if (mu > 0) {
	for (Int_t ind=mu-1; ind>=0; ind--) {
	    Int_t ist=(c.fIndexMap)[ind];
	    Int_t ql=((AliRICHDigit*)fDigits->UncheckedAt(ist))->Signal();
	    if (q>ql) {
		c.fIndexMap[ind]=idx;
		c.fIndexMap[ind+1]=ist;
	    } else {
		break;
	    }
	}
    }
    
  c.fMultiplicity++;    
    if (c.fMultiplicity >= 50 ) {
	Info("AddDigit2CLuster","multiplicity >50  %d \n",c.fMultiplicity);
	c.fMultiplicity=49;
    }
  Double_t x,y;// Prepare center of gravity calculation
  AliRICHParam::Pad2Loc(i,j,x,y);
  c.fX+=q*x;    c.fY+=q*y;    c.fQ += q;
  fHitMap->FlagHit(i,j);// Flag hit as taken  


  Int_t xList[4], yList[4];    //  Now look recursively for all neighbours
  for (Int_t iNei=0;iNei<Rich()->Param()->PadNeighbours(i,j,xList,yList);iNei++)
    if(fHitMap->TestHit(xList[iNei],yList[iNei])==kUnused) AddDigit2Cluster(xList[iNei],yList[iNei],c);    
}//AddDigit2Cluster()
//__________________________________________________________________________________________________
void AliRICHClusterFinder::FindRawClusters()
{//finds neighbours and fill the tree with raw clusters
  Info("FindRawClusters","Start for Chamber %i.",fChamber);
  
  if(!fNdigits)return;

  fHitMap=new AliRICHMap(fDigits);

  for(Int_t iDigN=0;iDigN<fNdigits;iDigN++){//digits loop
    AliRICHDigit *dig=(AliRICHDigit*)fDigits->UncheckedAt(iDigN);
    Int_t i=dig->PadX();   Int_t j=dig->PadY();
    if(fHitMap->TestHit(i,j)==kUsed||fHitMap->TestHit(i,j)==kEmpty) continue;
	
    AliRICHRawCluster c;
    c.fMultiplicity=0; c.fPeakSignal=dig->Signal();
    c.fTracks[0]=dig->Hit();c.fTracks[1]=dig->Track(0);c.fTracks[2]=dig->Track(1);        
    c.fNcluster[0]=-1;// tag the beginning of cluster list in a raw cluster
    
    AddDigit2Cluster(i,j,c);//form initial cluster
	
    c.fX /= c.fQ;	// center of gravity
    //c.fX=fSegmentation->GetAnod(c.fX);
    c.fY /= c.fQ;
    //AddRawCluster(c);
    
//    Int_t ix,iy;//  apply correction to the coordinate along the anode wire
//    Float_t x=c.fX, y=c.fY;	
//    Rich()->Param()->Loc2Pad(x,y,ix,iy);
//    Rich()->Param()->Pad2Loc(ix,iy,x,y);
//    Int_t isec=fSegmentation->Sector(ix,iy);
//    TF1* cogCorr=fSegmentation->CorrFunc(isec-1);
//    if(cogCorr){
//      Float_t yOnPad=(c.fY-y)/fSegmentation->Dpy(isec);
//      c.fY=c.fY-cogCorr->Eval(yOnPad,0,0);
//    }

    c.fNcluster[1]=fNRawClusters; c.fClusterType=c.PhysicsContribution();
    
    Decluster(&c);
    
    fNPeaks=0;

    c.fMultiplicity=0; for(int k=0;k<c.fMultiplicity;k++) c.fIndexMap[k]=0;//reset cluster object    
  }//digits loop
  delete fHitMap;
  Info("FindRawClusters","Stop.");
}//FindRawClusters()
//__________________________________________________________________________________________________
void AliRICHClusterFinder::CalibrateCOG()
{// Calibration

    Float_t x[5];
    Float_t y[5];
    Int_t n, i;
    if (fSegmentation) {
        TF1 *func;
	fSegmentation->GiveTestPoints(n, x, y);
	for (i=0; i<n; i++) {
            func = 0;
	    Float_t xtest=x[i];
	    Float_t ytest=y[i];	    
	    SinoidalFit(xtest, ytest, func);
	    if (func) fSegmentation->SetCorrFunc(i, new TF1(*func));
	}
    }
}//CalibrateCOG()
//__________________________________________________________________________________________________
void AliRICHClusterFinder::SinoidalFit(Double_t x, Double_t y, TF1 *func)
{//Sinoidal fit
  static Int_t count=0;
    
    count++;

    const Int_t kNs=101;
    Float_t xg[kNs], yg[kNs], xrg[kNs], yrg[kNs];
    Float_t xsig[kNs], ysig[kNs];
   
    Int_t ix,iy;
    AliRICHParam::Loc2Pad(x,y,ix,iy);   
    AliRICHParam::Pad2Loc(ix,iy,x,y);   
    Int_t isec=fSegmentation->Sector(ix,iy);
// Pad Limits    
    Float_t xmin = x-Rich()->Param()->PadSizeX()/2;
    Float_t ymin = y-Rich()->Param()->PadSizeY()/2;
//      	
//      Integration Limits 
    Float_t dxI=Rich()->Param()->MathiensonDeltaX();
    Float_t dyI=Rich()->Param()->MathiensonDeltaY();

//
//  Scanning
//
    Int_t i;
    Float_t qp=0;

//  y-position
    Float_t yscan=ymin;
    Float_t dy=Rich()->Param()->PadSizeY()/(kNs-1);

    for (i=0; i<kNs; i++) {//      Pad Loop
	Float_t sum=0;
	Float_t qcheck=0;
	fSegmentation->SigGenInit(x, yscan, 0);
	
	for (fSegmentation->FirstPad(x, yscan,0, dxI, dyI); 
	     fSegmentation->MorePads(); 
	     fSegmentation->NextPad()) 
	{
	    qp=fResponse->IntXY(fSegmentation);
	    qp=TMath::Abs(qp);
	    if (qp > 1.e-4) {
		qcheck+=qp;
		Int_t ixs=fSegmentation->Ix();
		Int_t iys=fSegmentation->Iy();
		Double_t xs,ys;
		AliRICHParam::Pad2Loc(ixs,iys,xs,ys);
		sum+=qp*ys;
	    }
	} // Pad loop
	Float_t ycog=sum/qcheck;
	yg[i]=(yscan-y)/fSegmentation->Dpy(isec);
	yrg[i]=(ycog-y)/fSegmentation->Dpy(isec);
	ysig[i]=ycog-yscan;
	yscan+=dy;
    } // scan loop
//  x-position
    Float_t xscan=xmin;
    Float_t dx=fSegmentation->Dpx(isec)/(kNs-1);

    for (i=0; i<kNs; i++) {//      Pad Loop
	Float_t sum=0;
	Float_t qcheck=0;
	fSegmentation->SigGenInit(xscan, y, 0);
	
	for (fSegmentation->FirstPad(xscan, y, 0, dxI, dyI); 
	     fSegmentation->MorePads(); 
	     fSegmentation->NextPad()) 
	{
	    qp=fResponse->IntXY(fSegmentation);
	    qp=TMath::Abs(qp);
	    if (qp > 1.e-2) {
		qcheck+=qp;
		Int_t ixs=fSegmentation->Ix();
		Int_t iys=fSegmentation->Iy();
		Double_t xs,ys;
		AliRICHParam::Pad2Loc(ixs,iys,xs,ys);
		sum+=qp*xs;
	    }
	} // Pad loop
	Float_t xcog=sum/qcheck;
	xcog=fSegmentation->GetAnod(xcog);
	
	xg[i]=(xscan-x)/fSegmentation->Dpx(isec);
	xrg[i]=(xcog-x)/fSegmentation->Dpx(isec);
	xsig[i]=xcog-xscan;
	xscan+=dx;
    }
// Creates a Root function based on function sinoid above and perform the fit
    TGraph *graphyr= new TGraph(kNs,yrg,ysig);
    Double_t sinoid(Double_t *x, Double_t *par);
    new TF1("sinoidf",sinoid,0.5,0.5,5);
    graphyr->Fit("sinoidf","Q");
    func = (TF1*)graphyr->GetListOfFunctions()->At(0);
}//SinoidalFit()
//__________________________________________________________________________________________________
Double_t sinoid(Double_t *x, Double_t *par)
{// Sinoid function

    Double_t arg = -2*TMath::Pi()*x[0];
    Double_t fitval= par[0]*TMath::Sin(arg)+
	par[1]*TMath::Sin(2*arg)+
	par[2]*TMath::Sin(3*arg)+
	par[3]*TMath::Sin(4*arg)+
	par[4]*TMath::Sin(5*arg);
    return fitval;
}//sinoid()
//__________________________________________________________________________________________________
Double_t DoubleGauss(Double_t *x, Double_t *par)
{//Double gaussian function
  Double_t arg1 = (x[0]-par[1])/0.18;
  Double_t arg2 = (x[0]-par[3])/0.18;
  return par[0]*TMath::Exp(-arg1*arg1/2)+par[2]*TMath::Exp(-arg2*arg2/2);
}
//__________________________________________________________________________________________________
Float_t DiscrCharge(Int_t i,Double_t *par) 
{
// par[0]    x-position of first  cluster
// par[1]    y-position of first  cluster
// par[2]    x-position of second cluster
// par[3]    y-position of second cluster
// par[4]    charge fraction of first  cluster
// 1-par[4]  charge fraction of second cluster

    static Float_t qtot;
    if (gFirst) {
	qtot=0;
	for (Int_t jbin=0; jbin<gNbins; jbin++) {
	    qtot+=gCharge[jbin];
	}
	gFirst=kFALSE;
	gChargeTot=Int_t(qtot);
	
    }
    TVector3 x3(par[0],par[1],0);
    Float_t q1=AliRICHParam::Loc2PadFrac(x3,gix[i],giy[i]);
    x3.SetX(par[2]);x3.SetY(par[3]);
    Float_t q2=AliRICHParam::Loc2PadFrac(x3,gix[i],giy[i]);
//    cout<<"qtot="<<gChargeTot<<" q1="<<q1<<" q2="<<q2<<" px="<<gix[i]<<" py="<<giy[i]<<endl;
    
    Float_t value = qtot*(par[4]*q1+(1.-par[4])*q2);
    return value;
}//DiscrCharge(Int_t i,Double_t *par) 
//__________________________________________________________________________________________________
void fcn(Int_t &npar, Double_t */*gin*/, Double_t &f, Double_t *par, Int_t)
{// Minimisation function
  npar=1;
    Int_t i;
    Float_t delta;
    Float_t chisq=0;
    Float_t qcont=0;
    Float_t qtot=0;
    
    for (i=0; i<gNbins; i++) {
	Float_t q0=gCharge[i];
	Float_t q1=DiscrCharge(i,par);
	delta=(q0-q1)/TMath::Sqrt(q0);
	chisq+=delta*delta;
	qcont+=q1;
	qtot+=q0;
    }
//    chisq=chisq+=(qtot-qcont)*(qtot-qcont)*0.5;
    f=chisq;
}//
//__________________________________________________________________________________________________
void AliRICHClusterFinder::Exec()
{
  Info("Exec","Start.");
  
  Rich()->GetLoader()->LoadDigits(); 
  
  for(Int_t iEventN=0;iEventN<gAlice->GetEventsPerRun();iEventN++){//events loop
    gAlice->GetRunLoader()->GetEvent(iEventN);
    
    Rich()->GetLoader()->MakeTree("R");  Rich()->MakeBranch("R");
    Rich()->ResetDigitsOld();  Rich()->ResetRawClusters();
    
    Rich()->GetLoader()->TreeD()->GetEntry(0);
    for(fChamber=1;fChamber<=kNCH;fChamber++){//chambers loop
      fDigits=Rich()->DigitsOld(fChamber); fNdigits=fDigits->GetEntries();
      
      FindRawClusters();
        
    }//chambers loop
    
    Rich()->GetLoader()->TreeR()->Fill();
    Rich()->GetLoader()->WriteRecPoints("OVERWRITE");
  }//events loop  
  Rich()->GetLoader()->UnloadDigits(); Rich()->GetLoader()->UnloadRecPoints();  
  Rich()->ResetDigitsOld();  Rich()->ResetRawClusters();
  Info("Exec","Stop.");      
}//Exec()
//__________________________________________________________________________________________________
