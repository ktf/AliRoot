/**************************************************************************
 * Copyright(c) 1998-2003, ALICE Experiment at CERN, All rights reserved. *
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

/* $Id$*/

////////////////////////////////////////////////////////////////////////////
//            Implementation of the ITS clusterer V2 class                //
//                                                                        //
//          Origin: Iouri Belikov, CERN, Jouri.Belikov@cern.ch            //
//                                                                        //
///////////////////////////////////////////////////////////////////////////



#include <TClonesArray.h>
#include "AliITSClusterFinderV2SDD.h"
#include "AliITSRecPoint.h"
#include "AliITSDetTypeRec.h"
#include "AliRawReader.h"
#include "AliITSRawStreamSDD.h"
#include "AliITSCalibrationSDD.h"
#include "AliITSDetTypeRec.h"
#include "AliITSsegmentationSDD.h"
#include "AliITSdigitSDD.h"
#include "AliITSgeomTGeo.h"
ClassImp(AliITSClusterFinderV2SDD)

AliITSClusterFinderV2SDD::AliITSClusterFinderV2SDD(AliITSDetTypeRec* dettyp):AliITSClusterFinderV2(dettyp)
{

  //Default constructor

}
 

void AliITSClusterFinderV2SDD::FindRawClusters(Int_t mod){

  //Find clusters V2
  SetModule(mod);
  FindClustersSDD(fDigits);

}

void AliITSClusterFinderV2SDD::FindClustersSDD(TClonesArray *digits) {
  //------------------------------------------------------------
  // Actual SDD cluster finder
  //------------------------------------------------------------
  Int_t nAnodes = GetSeg()->NpzHalf();
  Int_t nzBins = nAnodes+2;
  Int_t nTimeBins = GetSeg()->Npx();
  Int_t nxBins = nTimeBins+2;
  const Int_t kMaxBin=nzBins*(nxBins+2);

  AliBin *bins[2];
  bins[0]=new AliBin[kMaxBin];
  bins[1]=new AliBin[kMaxBin];
  AliITSCalibrationSDD* cal = (AliITSCalibrationSDD*)GetResp(fModule);
  AliITSresponseSDD* res  = (AliITSresponseSDD*)cal->GetResponse();
  const char *option=res->ZeroSuppOption();
  AliITSdigitSDD *d=0;
  Int_t i, ndigits=digits->GetEntriesFast();
  for (i=0; i<ndigits; i++) {
     d=(AliITSdigitSDD*)digits->UncheckedAt(i);
     Int_t y=d->GetCoord2()+1;   //y
     Int_t z=d->GetCoord1()+1;   //z
     Float_t gain=cal->GetChannelGain(d->GetCoord1());
     Float_t charge=d->GetSignal();

     if(!((strstr(option,"1D")) || (strstr(option,"2D")))){
       Float_t baseline = cal->GetBaseline(d->GetCoord1());
       if(charge>baseline) charge-=baseline;
       else charge=0;
     }

     if(gain>0) charge/=gain;
     if(charge<cal->GetThresholdAnode(d->GetCoord1())) continue;
     Int_t q=(Int_t)(charge+0.5);
     if (z <= nAnodes){
       bins[0][y*nzBins+z].SetQ(q);
       bins[0][y*nzBins+z].SetMask(1);
       bins[0][y*nzBins+z].SetIndex(i);
     } else {
       z-=nAnodes;
       bins[1][y*nzBins+z].SetQ(q);
       bins[1][y*nzBins+z].SetMask(1);
       bins[1][y*nzBins+z].SetIndex(i);
     }
  }
  
  FindClustersSDD(bins, kMaxBin, nzBins, digits);

  delete[] bins[0];
  delete[] bins[1];

}

void AliITSClusterFinderV2SDD::
FindClustersSDD(AliBin* bins[2], Int_t nMaxBin, Int_t nzBins, 
		TClonesArray *digits, TClonesArray *clusters) {
  //------------------------------------------------------------
  // Actual SDD cluster finder
  //------------------------------------------------------------

  const TGeoHMatrix *mT2L=AliITSgeomTGeo::GetTracking2LocalMatrix(fModule);
  AliITSCalibrationSDD* cal = (AliITSCalibrationSDD*)GetResp(fModule);
  Int_t ncl=0; 
  TClonesArray &cl=*clusters;
  for (Int_t s=0; s<2; s++)
    for (Int_t i=0; i<nMaxBin; i++) {
      if (bins[s][i].IsUsed()) continue;
      Int_t idx[32]; UInt_t msk[32]; Int_t npeaks=0;
      FindPeaks(i, nzBins, bins[s], idx, msk, npeaks);

      if (npeaks>30) continue;
      if (npeaks==0) continue;

      Int_t k,l;
      for (k=0; k<npeaks-1; k++){//mark adjacent peaks
        if (idx[k] < 0) continue; //this peak is already removed
        for (l=k+1; l<npeaks; l++) {
           if (idx[l] < 0) continue; //this peak is already removed
           Int_t ki=idx[k]/nzBins, kj=idx[k] - ki*nzBins;
           Int_t li=idx[l]/nzBins, lj=idx[l] - li*nzBins;
           Int_t di=TMath::Abs(ki - li);
           Int_t dj=TMath::Abs(kj - lj);
           if (di>1 || dj>1) continue;
           if (bins[s][idx[k]].GetQ() > bins[s][idx[l]].GetQ()) {
              msk[l]=msk[k];
              idx[l]*=-1;
           } else {
              msk[k]=msk[l];
              idx[k]*=-1;
              break;
           } 
        }
      }

      for (k=0; k<npeaks; k++) {
        MarkPeak(TMath::Abs(idx[k]), nzBins, bins[s], msk[k]);
      }
        
      for (k=0; k<npeaks; k++) {
         if (idx[k] < 0) continue; //removed peak
         AliITSRecPoint c;
         MakeCluster(idx[k], nzBins, bins[s], msk[k], c);
	 //mi change
	 Int_t milab[10];
	 for (Int_t ilab=0;ilab<10;ilab++){
	   milab[ilab]=-2;
	 }
	 Int_t maxi=0,mini=0,maxj=0,minj=0;
	 //AliBin *bmax=&bins[s][idx[k]];
	 //Float_t max = TMath::Max(TMath::Abs(bmax->GetQ())/5.,3.);
    
	 for (Int_t di=-2; di<=2;di++){
	   for (Int_t dj=-3;dj<=3;dj++){
	     Int_t index = idx[k]+di+dj*nzBins;
	     if (index<0) continue;
	     if (index>=nMaxBin) continue;
	     AliBin *b=&bins[s][index];
	     Int_t nAnode=index%nzBins-1;
	     Int_t adcSignal=b->GetQ();
	     if(adcSignal>cal->GetThresholdAnode(nAnode)){
	       if (di>maxi) maxi=di;
	       if (di<mini) mini=di;
	       if (dj>maxj) maxj=dj;
	       if (dj<minj) minj=dj;
	     }
	     //
	     if(digits) {
	       if (TMath::Abs(di)<2&&TMath::Abs(dj)<2){
		 AliITSdigitSDD* d=(AliITSdigitSDD*)digits->UncheckedAt(b->GetIndex());
		 for (Int_t itrack=0;itrack<10;itrack++){
		   Int_t track = (d->GetTracks())[itrack];
		   if (track>=0) {
		     AddLabel(milab, track); 
		   }
		 }
	       }
	     }
	   }
	 }


         Float_t y=c.GetY(),z=c.GetZ(), q=c.GetQ();
         y/=q; z/=q;


	 const Double_t kMicronTocm = 1.0e-4; 
	 Float_t timeBinCenter = y-0.5;
	 Float_t zAnode=z-0.5;
	 Float_t zdet = (zAnode*GetSeg()->Dpz(0)-GetSeg()->Dz()/2.)*kMicronTocm;
	 Float_t driftTime =  timeBinCenter*GetSeg()->Dpx(0) - cal->GetTimeOffset();
	 Float_t xdet = cal->GetDriftPath(driftTime,zAnode);
	 xdet=(xdet-GetSeg()->Dx())*kMicronTocm;
	 if (s) xdet=-xdet;
	 
	 
	 CorrectPosition(zdet,xdet);

	 Double_t loc[3]={xdet,0.,zdet},trk[3]={0.,0.,0.};
	 mT2L->MasterToLocal(loc,trk);
	 y=trk[1];
	 z=trk[2]; 

         q/=cal->GetADC2keV();  //to have MPV 1 MIP = 86.4 KeV
         Float_t hit[5] = {y, z, 0.0030*0.0030, 0.0020*0.0020, q};
         Int_t  info[3] = {maxj-minj+1, maxi-mini+1, fNlayer[fModule]};
	 if (digits) {	  
	   //	   AliBin *b=&bins[s][idx[k]];
	   //	   AliITSdigitSDD* d=(AliITSdigitSDD*)digits->UncheckedAt(b->GetIndex());
	   {
	     //Int_t lab[3];
	     //lab[0]=(d->GetTracks())[0];
	     //lab[1]=(d->GetTracks())[1];
	     //lab[2]=(d->GetTracks())[2];
	     //CheckLabels(lab);
	     CheckLabels2(milab); 
	   }
	 }
         milab[3]=fNdet[fModule];

         AliITSRecPoint cc(milab,hit,info);
	 cc.SetType(npeaks);

	 if(clusters) new (cl[ncl]) AliITSRecPoint(cc); 
	 else {
	   fDetTypeRec->AddRecPoint(cc);
	 }
	 ncl++;
      }
    }
  
}



void AliITSClusterFinderV2SDD::RawdataToClusters(AliRawReader* rawReader,TClonesArray** clusters){
    //------------------------------------------------------------
  // This function creates ITS clusters from raw data
  //------------------------------------------------------------
  rawReader->Reset();
  AliITSRawStreamSDD inputSDD(rawReader);
  /*
  AliITSCalibrationSDD* cal = (AliITSCalibrationSDD*)GetResp(240);
  printf("gain anode 10=%f\n",cal->GetDriftSpeedAtAnode(10));
  printf("drift speed anode 10=%f\n",cal->GetChannelGain(10));
  */
  AliITSDDLModuleMapSDD *ddlmap=(AliITSDDLModuleMapSDD*)fDetTypeRec->GetDDLModuleMapSDD();
  inputSDD.SetDDLModuleMap(ddlmap);
  FindClustersSDD(&inputSDD,clusters);

}

void AliITSClusterFinderV2SDD::FindClustersSDD(AliITSRawStream* input, 
					TClonesArray** clusters) 
{
  //------------------------------------------------------------
  // Actual SDD cluster finder for raw data
  //------------------------------------------------------------
  Int_t nClustersSDD = 0;
  Int_t nAnodes = GetSeg()->NpzHalf();
  Int_t nzBins = nAnodes+2;
  Int_t nTimeBins = GetSeg()->Npx();
  Int_t nxBins = nTimeBins+2;
  const Int_t kMaxBin=nzBins*(nxBins+2);
  AliBin *bins[2];
  AliBin *ddlbins[kHybridsPerDDL]; // 12 modules (=24 hybrids) of 1 DDL read "in parallel"
  for(Int_t iHyb=0;iHyb<kHybridsPerDDL;iHyb++) ddlbins[iHyb]=new AliBin[kMaxBin];
  // read raw data input stream
  while (input->Next()) {
    Int_t iModule = input->GetModuleID();
    if(iModule<0){
      AliWarning(Form("Invalid SDD module number %d\n", iModule));
      continue;
    }

    Int_t iCarlos =((AliITSRawStreamSDD*)input)->GetCarlosId();
    Int_t iSide = ((AliITSRawStreamSDD*)input)->GetChannel();
    Int_t iHybrid=iCarlos*2+iSide;
    if (input->IsCompletedModule()) {
      // when all data from a module was read, search for clusters
      if(iCarlos<0){
	AliWarning(Form("Invalid SDD carlos number %d on module %d\n", iCarlos,iModule));
	continue;
      }
      clusters[iModule] = new TClonesArray("AliITSRecPoint");
      fModule = iModule;
      bins[0]=ddlbins[iCarlos*2];   // first hybrid of the completed module
      bins[1]=ddlbins[iCarlos*2+1]; // second hybrid of the completed module
      FindClustersSDD(bins, kMaxBin, nzBins, NULL, clusters[iModule]);
      Int_t nClusters = clusters[iModule]->GetEntriesFast();
      nClustersSDD += nClusters;
      for(Int_t iBin=0;iBin<kMaxBin; iBin++){
	ddlbins[iCarlos*2][iBin].Reset();
	ddlbins[iCarlos*2+1][iBin].Reset();
      }
    }else{
    // fill the current digit into the bins array
      if(iHybrid<0 || iHybrid>=kHybridsPerDDL){ 
	AliWarning(Form("Invalid SDD hybrid number %d on module %d\n", iHybrid,iModule));
	continue;
      }
      AliITSCalibrationSDD* cal = (AliITSCalibrationSDD*)GetResp(iModule);    
      AliITSresponseSDD* res  = (AliITSresponseSDD*)cal->GetResponse();
      const char *option=res->ZeroSuppOption();
      Float_t charge=input->GetSignal();
      Int_t chan=input->GetCoord1()+nAnodes*iSide;
      Float_t gain=cal->GetChannelGain(chan);
      if(!((strstr(option,"1D")) || (strstr(option,"2D")))){
	Float_t baseline = cal->GetBaseline(chan);
	if(charge>baseline) charge-=baseline;
	else charge=0;
      }
      if(gain>0) charge/=gain;
      if(charge>=cal->GetThresholdAnode(chan)) {
	Int_t q=(Int_t)(charge+0.5);
	Int_t iz = input->GetCoord1();
	Int_t itb = input->GetCoord2();
	Int_t index = (itb+1) * nzBins + (iz+1);
	if(index<kMaxBin){
	  ddlbins[iHybrid][index].SetQ(q);
	  ddlbins[iHybrid][index].SetMask(1);
	  ddlbins[iHybrid][index].SetIndex(index);

	}else{
	  AliWarning(Form("Invalid SDD cell: Anode=%d   TimeBin=%d",iz,itb));	  
	}
      }
    }
  }
  for(Int_t iHyb=0;iHyb<kHybridsPerDDL;iHyb++) delete [] ddlbins[iHyb];
  Info("FindClustersSDD", "found clusters in ITS SDD: %d", nClustersSDD);
}


//_________________________________________________________________________
void AliITSClusterFinderV2SDD::CorrectPosition(Float_t &z, Float_t&y){

  //correction of coordinates using the maps stored in the DB

  AliITSCalibrationSDD* cal = (AliITSCalibrationSDD*)GetResp(fModule);
  static const Int_t knbint = cal->GetMapTimeNBin();
  static const Int_t knbina = cal->Chips()*cal->Channels();
  const Double_t kMicronTocm = 1.0e-4; 
  Float_t stepa = (GetSeg()->Dpz(0))*kMicronTocm; //anode pitch in cm
  Float_t stept = (GetSeg()->Dx()/cal->GetMapTimeNBin()/2.)/10.;
  
  Int_t bint = TMath::Abs((Int_t)(y/stept));
  if(y>=0) bint+=(Int_t)(knbint/2.);
  if(bint>knbint) AliError("Wrong bin number!");

  Int_t bina = TMath::Abs((Int_t)(z/stepa));
  if(z>=0) bina+=(Int_t)(knbina/2.);
  if(bina>knbina) AliError("Wrong bin number!");

  Float_t devz = cal->GetMapACell(bina,bint)*kMicronTocm;
  Float_t devx = cal->GetMapTCell(bina,bint)*kMicronTocm;
  z+=devz;
  y+=devx;


}
