//$Id$

// Author: Anders Vestbo <mailto:vestbo@fi.uib.no>
//*-- Copyright &copy ASV

#include "AliL3StandardIncludes.h"

#include "AliL3Modeller.h"
#include "AliL3MemHandler.h"
#ifdef use_aliroot
#include "AliL3FileHandler.h"
#endif
#include "AliL3TrackArray.h"
#include "AliL3ModelTrack.h"
#include "AliL3DigitData.h"
#include "AliL3Transform.h"

#if GCCVERSION == 3
using namespace std;
#endif

//_____________________________________________________________
// AliL3Modeller
//
// Class for modeling TPC data.
// 
// This performs the cluster finding, based on track parameters.
// Basically it propagates the tracks to all padrows, and looks 
// for a corresponding cluster. For the moment only cog is calculated,
// and no deconvolution is done. 

ClassImp(AliL3Modeller)

AliL3Modeller::AliL3Modeller()
{
  fMemHandler=0;
  fTracks=0;
  fTrackThreshold=0;
  SetOverlap();
  SetTrackThreshold();
  SetSearchRange();
}


AliL3Modeller::~AliL3Modeller()
{
  if(fMemHandler)
    delete fMemHandler;
  if(fTracks)
    delete fTracks;
}

void AliL3Modeller::Init(Int_t slice,Int_t patch,Char_t *trackdata,Char_t *path,Bool_t houghtracks,Bool_t binary)
{
  fSlice = slice;
  fPatch = patch;

  sprintf(fPath,"%s",path);
  
  fTracks = new AliL3TrackArray("AliL3ModelTrack");
  
  Char_t fname[100];
  AliL3MemHandler *file = new AliL3MemHandler();
  //sprintf(fname,"%s/tracks_tr_%d_0.raw",trackdata,fSlice); //output tracks from the tracker (no merging)
  sprintf(fname,"%s/tracks_ho_%d.raw",trackdata,fSlice);
  if(!file->SetBinaryInput(fname))
    {
      cerr<<"AliL3Modeller::Init : Error opening trackfile: "<<fname<<endl;
      return;
    }
  file->Binary2TrackArray(fTracks);
  file->CloseBinaryInput();
  delete file;
  

  if(!houghtracks)
    fTracks->QSort();
  
  for(Int_t i=0; i<fTracks->GetNTracks(); i++)
    {
      AliL3ModelTrack *track = (AliL3ModelTrack*)fTracks->GetCheckedTrack(i);
      if(!track) continue;
      track->Init(fSlice,fPatch);

      //Only if the tracks has been merged across sector boundaries:
      //if(!houghtracks)
      //track->Rotate(fSlice,kTRUE); //!!!!!!!!!!!!!!!!!!!
      
      track->CalculateHelix();
    }    
  
  UInt_t ndigits=0;
  AliL3DigitRowData *digits=0;
#ifdef use_aliroot
  fMemHandler = new AliL3FileHandler();
  fMemHandler->Init(slice,patch);
  if(binary == kFALSE)
    {
      sprintf(fname,"%s/digitfile.root",fPath);
      fMemHandler->SetAliInput(fname);
      digits = fMemHandler->AliDigits2Memory(ndigits);
    }
  else
    {
      sprintf(fname,"%sdigits_%d_%d.raw",fPath,fSlice,fPatch);
      if(!fMemHandler->SetBinaryInput(fname))
	{
	  cerr<<"AliL3Modeller::Init : Error opening file "<<fname<<endl;
	  return;
	}
      digits=(AliL3DigitRowData*)fMemHandler->CompBinary2Memory(ndigits);
    }
#else
  fMemHandler = new AliL3MemHandler();
  fMemHandler->Init(slice,patch);
  if(binary == kFALSE)
    {
      cerr<<"AliL3Modeller::Init : Compile with AliROOT if you want rootfile as input"<<endl;
      return;
    }
  else
    {
      sprintf(fname,"%sdigits_%d_%d.raw",fPath,fSlice,fPatch);
      if(!fMemHandler->SetBinaryInput(fname))
	{
	  cerr<<"AliL3Modeller::Init : Error opening file "<<fname<<endl;
	  return;
	}
    }
  digits=(AliL3DigitRowData*)fMemHandler->CompBinary2Memory(ndigits);
#endif
  
  SetInputData(digits);
}

void AliL3Modeller::FindClusters()
{
  cout<<"AliL3Modeller::FindClusters : Processing slice "<<fSlice<<" patch "<<fPatch<<endl;
  if(!fTracks)
    {
      cerr<<"AliL3Modeller::Process : No tracks"<<endl;
      return;
    }
  if(!fRowData)
    {
      cerr<<"AliL3Modeller::Process : No data "<<endl;
      return;
    }
  
  AliL3DigitRowData *rowPt = fRowData;
  AliL3DigitData *digPt=0;

  Int_t ntimes = AliL3Transform::GetNTimeBins()+1;
  Int_t npads = AliL3Transform::GetNPads(AliL3Transform::GetLastRow(fPatch))+1;//Max num of pads.
  Int_t bounds = ntimes*npads;
  Digit *row = new Digit[bounds];
  
  Int_t seq_charge;
  Int_t pad,time,index;
  Short_t charge;
  Cluster cluster;

  for(Int_t i=AliL3Transform::GetFirstRow(fPatch); i<=AliL3Transform::GetLastRow(fPatch); i++)
    {
      fCurrentPadRow = i;
      memset((void*)row,0,ntimes*npads*sizeof(Digit));
      digPt = (AliL3DigitData*)rowPt->fDigitData;
      //cout<<"Loading row "<<i<<" with "<<(Int_t)rowPt->fNDigit<<" digits"<<endl;
      for(UInt_t j=0; j<rowPt->fNDigit; j++)
	{
	  pad = digPt[j].fPad;
	  time = digPt[j].fTime;
	  charge = digPt[j].fCharge;
	  row[ntimes*pad+time].fCharge = charge;
	  row[ntimes*pad+time].fUsed = kFALSE;
	  //cout<<"Row "<<i<<" pad "<<pad<<" time "<<time<<" charge "<<charge<<endl;
	}
      
      for(Int_t k=0; k<fTracks->GetNTracks(); k++)
	{
	  AliL3ModelTrack *track = (AliL3ModelTrack*)fTracks->GetCheckedTrack(k);
	  if(!track) continue;
	  
	  if(track->GetPadHit(i)<0 || track->GetTimeHit(i)<0 || track->GetOverlap(i)>=0)
	    {
	      track->SetCluster(i,0,0,0,0,0,0); //The track has left the patch.
	      continue;
	    }
	  
	  Int_t hitpad = (Int_t)rint(track->GetPadHit(i));
	  Int_t hittime = (Int_t)rint(track->GetTimeHit(i));

	  if(!CheckCluster(row,hitpad,hittime)) //Not a good cluster.
	    {
	      track->SetCluster(i,0,0,0,0,0,0); 
	      continue;
	    }

	  //cout<<"Checking track on row "<<i<<" with pad "<<hitpad<<" time "<<hittime<<endl;
	  pad = hitpad;
	  time = hittime;

	  Int_t padsign=-1;
	  Int_t timesign=-1;
	  
	  memset(&cluster,0,sizeof(Cluster));
	  //cout<<"Processing padrow "<<i<<" with hittime "<<hittime<<" hitpad "<<hitpad<<" charge "<<row[ntimes*pad + time].fCharge<<" index "<<ntimes*pad + time<<endl;
	  Int_t npads=0;
	  Int_t last_mean = hittime;
	  while(1)//Process this padrow
	    {
	      if(pad < 0 || pad >= AliL3Transform::GetNPads(i)) 
		{
		  //cout<<"Pad = "<<pad<<" on row "<<i<<endl;
		  FillCluster(track,&cluster,i,npads);
		  break;
		}
	      seq_charge=0;
	      timesign=-1;
	      time = hittime;
	      
	      while(1) //Process sequence on this pad:
		{
		  if(time < 0 || time >= AliL3Transform::GetNTimeBins()) 
		    break;
		
		  index = ntimes*pad + time;
		  if(index < 0 || index >= bounds)
		    {
		      cerr<<"AliL3Modeller::FindClusters : Index out of range : "<<index
			<<" on row "<<i<<" pad "<<pad<<" time "<<time<<endl;
		      break;
		    }
		  
		  charge = row[index].fCharge;
		  
		  if(charge==0) //Empty timebin, so try and expand the search within the limits on this pad
		    {
		      if(seq_charge==0 && abs(time-hittime) <= fTimeSearch)//No sequence yet, keep looking
			{
			  if(timesign==-1)
			    time--;
			  else
			    time++;
			  continue;
			}
		      else if(timesign==-1) //Switch search direction
			{
			  time = hittime + 1;
			  timesign = 1;
			  continue;
			}
		      else //ok, boundary reached, this pad is done.
			break;
		    }
		  
		  if(row[ntimes*pad+time].fUsed==kTRUE) //Don't use digits several times. This leads to mult. rec.tracks.
		    {
		      time += timesign;
		      continue;
		    }
		  
		  seq_charge += charge;
		  
		  //Update the cluster parameters with this timebin
		  cluster.fTime += time*charge;
		  cluster.fPad += pad*charge;
		  cluster.fCharge += charge;
		  cluster.fSigmaY2 += pad*pad*charge;
		  cluster.fSigmaZ2 += time*time*charge;
		  
		  row[ntimes*pad+time].fUsed = kTRUE;
		  time += timesign;
		}
	      
	      //Always compare with the current mean in time of the cluster under construction.
	      if(cluster.fCharge)
		last_mean = (Int_t)rint((Float_t)(cluster.fTime/cluster.fCharge));
	      
	      if(seq_charge)//There was something on this pad, so keep looking on the neighbouring pad
		{
		  pad += padsign;
		  npads++;
		}
	      else
		{
		  if(cluster.fCharge==0 && abs(pad-hitpad) <= fPadSearch && pad > 0)
		    {
		      if(padsign==-1)
			pad--;
		      else
			pad++;
		      continue;
		    }
		  else if(padsign==-1)
		    {
		      pad = hitpad + 1;
		      padsign = 1;
		      continue;
		    }
		  else
		    {
		      FillCluster(track,&cluster,i,npads);
		      break;
		    }
		}
	    }
	  //cout<<"done"<<endl;
	}
      FillZeros(rowPt,row);
      fMemHandler->UpdateRowPointer(rowPt);
    }
  delete [] row;
  //cout<<"done processing"<<endl;
  
  
  //Debug:
  for(Int_t i=0; i<fTracks->GetNTracks(); i++)
    {
      AliL3ModelTrack *track = (AliL3ModelTrack*)fTracks->GetCheckedTrack(i);
      if(!track) continue;
      if(track->GetNClusters() != AliL3Transform::GetNRows(fPatch))
	cerr<<endl<<"Mismatching hitcounts; nclusters: "<<track->GetNClusters()<<" nrows "<<AliL3Transform::GetNRows(fPatch)<<endl<<endl;
    }
  
}

Bool_t AliL3Modeller::CheckCluster(Digit *row,Int_t hitpad,Int_t hittime)
{
  Int_t ntimes = AliL3Transform::GetNTimeBins()+1;
  Bool_t seq_was_falling=kFALSE;
  Int_t last_seq_charge=0;
  Int_t cluster_charge=0;
  Int_t time,pad,charge,padsign=-1,timesign=-1;
  time = hittime;
  pad = hitpad;
  Int_t npads=0;
  //cout<<"Checking cluster "<<hitpad<<" hittime "<<hittime<<endl;
  while(1)
    {
      Bool_t last_was_falling=kFALSE;
      Int_t last_charge=0;
      Int_t seq_charge=0;
      while(1)
	{
	  
	  charge = row[(ntimes*pad+time)].fCharge;
	  //cout<<"Charge "<<charge<<" on pad "<<pad<<" time "<<time<<endl;
	  if(charge==0)
	    {
	      if(seq_charge==0 && abs(time-hittime) <= fTimeSearch)
		{
		  if(timesign==-1)
		    time--;
		  else
		    time++;
		  continue;
		}
	      else if(timesign==-1)
		{
		  last_charge = row[(ntimes*hitpad+hittime)].fCharge;
		  time = hittime+1;
		  timesign=1;
		  if(last_charge < row[(ntimes*hitpad+(hittime-1))].fCharge)
		    last_was_falling=kTRUE;
		  else
		    last_was_falling=kFALSE;
		  continue;
		}
	      else
		break;
	    }

	  if(charge > last_charge)
	    {
	      if(last_was_falling) //This is a overlapping cluster.
		return kFALSE;
	    }
	  else
	    last_was_falling = kTRUE;

	  cluster_charge+=charge;
	  seq_charge += charge;
	  last_charge = charge;
	  time += timesign;
	}
      
      if(seq_charge)
	{
	  pad += padsign;
	  npads++;
	}
      else
	{
	  if(cluster_charge==0 && abs(pad-hitpad)<=fPadSearch && pad > 0)
	    {
	      if(padsign==-1)
		pad--;
	      else
		pad++;
	      continue;
	    }
	  else if(padsign==-1)
	    {
	      pad = hitpad+1;
	      padsign = 1;
	      seq_was_falling=kFALSE;
	      continue;
	    }
	  else
	    break;
	}
      
      if(seq_charge > last_seq_charge)
	{
	  if(seq_was_falling)
	    return kFALSE;
	  else
	    seq_was_falling=kTRUE;
	}
      last_seq_charge = seq_charge;
    }
  
  if(npads < 2 || npads > 8)
    return kFALSE;
  else
    return kTRUE;
}

void AliL3Modeller::FillCluster(AliL3ModelTrack *track,Cluster *cluster,Int_t row,Int_t npads)
{
  if(cluster->fCharge==0)
    {
      track->SetCluster(row,0,0,0,0,0,0);
      return;
    }
  Float_t fcharge = (Float_t)cluster->fCharge;
  Float_t fpad = ((Float_t)cluster->fPad/fcharge);
  Float_t ftime = ((Float_t)cluster->fTime/fcharge);
  Float_t sigmaY2,sigmaZ2;
  CalcClusterWidth(cluster,sigmaY2,sigmaZ2);
  track->SetCluster(row,fpad,ftime,fcharge,sigmaY2,sigmaZ2,npads);
}

void AliL3Modeller::FillZeros(AliL3DigitRowData *rowPt,Digit *row)
{
  //Fill zero where data has been used.
  
  Int_t ntimes = AliL3Transform::GetNTimeBins()+1;
  AliL3DigitData *digPt = (AliL3DigitData*)rowPt->fDigitData;
  for(UInt_t j=0; j<rowPt->fNDigit; j++)
    {
      Int_t pad = digPt[j].fPad;
      Int_t time = digPt[j].fTime;
      if(row[ntimes*pad+time].fUsed==kTRUE)
	digPt[j].fCharge = 0;
    }
}

void AliL3Modeller::WriteRemaining()
{
  //Write remaining (nonzero) digits to file.
  
  AliL3DigitRowData *rowPt;
  rowPt = (AliL3DigitRowData*)fRowData;
  Int_t digitcount=0;
  Int_t ndigits[(AliL3Transform::GetNRows(fPatch))];
  for(Int_t i=AliL3Transform::GetFirstRow(fPatch); i<=AliL3Transform::GetLastRow(fPatch); i++)
    {
      AliL3DigitData *digPt = (AliL3DigitData*)rowPt->fDigitData;
      ndigits[(i-AliL3Transform::GetFirstRow(fPatch))]=0;
      for(UInt_t j=0; j<rowPt->fNDigit; j++)
	{
	  if(digPt[j].fCharge==0) continue;
	  digitcount++;
	  ndigits[(i-AliL3Transform::GetFirstRow(fPatch))]++;
	}
      //cout<<"Difference "<<(int)ndigits[(i-AliL3Transform::GetFirstRow(fPatch))]<<" "<<(int)rowPt->fNDigit<<endl;
      fMemHandler->UpdateRowPointer(rowPt);
    }
  
  Int_t size = digitcount*sizeof(AliL3DigitData) + AliL3Transform::GetNRows(fPatch)*sizeof(AliL3DigitRowData);
  Byte_t *data = new Byte_t[size];
  memset(data,0,size);
  AliL3DigitRowData *tempPt = (AliL3DigitRowData*)data;
  rowPt = (AliL3DigitRowData*)fRowData;
  
  for(Int_t i=AliL3Transform::GetFirstRow(fPatch); i<=AliL3Transform::GetLastRow(fPatch); i++)
    {
      Int_t localcount=0;
      tempPt->fRow = i;
      tempPt->fNDigit = ndigits[(i-AliL3Transform::GetFirstRow(fPatch))];
      AliL3DigitData *digPt = (AliL3DigitData*)rowPt->fDigitData;
      for(UInt_t j=0; j<rowPt->fNDigit; j++)
	{
	  if(digPt[j].fCharge==0) continue;
	  if(localcount >= ndigits[(i-AliL3Transform::GetFirstRow(fPatch))])
	    {
	      cerr<<"AliL3Modeller::WriteRemaining : Digitarray out of range!!"<<endl;
	      return;
	    }
	  tempPt->fDigitData[localcount].fCharge = digPt[j].fCharge;
	  tempPt->fDigitData[localcount].fPad = digPt[j].fPad;
	  tempPt->fDigitData[localcount].fTime = digPt[j].fTime;

	  localcount++;
	}
      if(ndigits[(i-AliL3Transform::GetFirstRow(fPatch))] != localcount)
	{
	  cerr<<"AliL3Modeller::WriteRemaining : Mismatch in digitcount"<<endl;
	  return;
	}
      fMemHandler->UpdateRowPointer(rowPt);
      Byte_t *tmp = (Byte_t*)tempPt;
      Int_t size = sizeof(AliL3DigitRowData) + ndigits[(i-AliL3Transform::GetFirstRow(fPatch))]*sizeof(AliL3DigitData);
      tmp += size;
      tempPt = (AliL3DigitRowData*)tmp;
    }

  Char_t fname[100];
  AliL3MemHandler *mem = new AliL3MemHandler();
  sprintf(fname,"%s/comp/remains_%d_%d.raw",fPath,fSlice,fPatch);
  mem->SetBinaryOutput(fname);
  mem->Memory2CompBinary((UInt_t)AliL3Transform::GetNRows(fPatch),(AliL3DigitRowData*)data);
  mem->CloseBinaryOutput();
  delete mem;
  delete [] data;
}


void AliL3Modeller::CalculateCrossingPoints()
{
  //cout<<"Calculating crossing points on "<<fTracks->GetNTracks()<<" tracks"<<endl;
  if(!fTracks)
    {
      cerr<<"AliL3Modeller::CalculateCrossingPoints(): No tracks"<<endl;
      return;
    }
  Float_t hit[3];
  
  Int_t sector,row;
  for(Int_t i=AliL3Transform::GetLastRow(fPatch); i>=AliL3Transform::GetFirstRow(fPatch); i--)
    {
      for(Int_t j=0; j<fTracks->GetNTracks(); j++)
	{
	  AliL3ModelTrack *track = (AliL3ModelTrack*)fTracks->GetCheckedTrack(j);
	  if(!track) continue;

	  if(!track->GetCrossingPoint(i,hit)) 
	    {
	      //cerr<<"AliL3Modeller::CalculateCrossingPoints : Track "<<j<<" does not intersect row "<<i<<" :"<<endl<<
	      //" pt "<<track->GetPt()<<
	      //" tgl "<<track->GetTgl()<<" psi "<<track->GetPsi()<<" charge "<<track->GetCharge()<<endl;
		//"Center "<<track->GetCenterX()<<" "<<track->GetCenterY()<<endl<<endl<<
		//"--------"<<endl;
	      fTracks->Remove(j);
	      continue;
	    }
	  //cout<<"X "<<hit[0]<<" Y "<<hit[1]<<" Z "<<hit[2]<<" tgl "<<track->GetTgl()<<endl;
	  
	  AliL3Transform::Slice2Sector(fSlice,i,sector,row);
	  AliL3Transform::Local2Raw(hit,sector,row);
	  //cout<<"Pad "<<hit[1]<<" time "<<hit[2]<<" in sector "<<sector<<" row "<<row<<endl;
	  if(hit[1]<0 || hit[1]>AliL3Transform::GetNPads(i) ||
	     hit[2]<0 || hit[2]>AliL3Transform::GetNTimeBins())
	    {//Track is leaving the patch, so flag the track hits (<0)
	      track->SetPadHit(i,-1);
	      track->SetTimeHit(i,-1);
	      continue;
	    }
	  
	  
	  track->SetPadHit(i,hit[1]);
	  track->SetTimeHit(i,hit[2]);
	  
	  //if(hit[1]<0 || hit[2]>445)
	  //if(hit[2]<0 || hit[2]>445)
	  //cout<<"pad "<<hit[1]<<" time "<<hit[2]<<" pt "<<track->GetPt()<<" psi "<<track->GetPsi()<<" tgl "<<track->GetTgl()<<" firstpoint "<<track->GetFirstPointX()<<" "<<track->GetFirstPointY()<<" "<<track->GetFirstPointZ()<<endl;
	  //cout<<"Crossing pad "<<hit[1]<<" time "<<hit[2]<<endl;
	}
    }
  fTracks->Compress();
  //cout<<"And there are "<<fTracks->GetNTracks()<<" tracks remaining"<<endl;
}

void AliL3Modeller::CheckForOverlaps()
{
  //Flag the tracks that overlap
  
  //cout<<"Checking for overlaps...";
  Int_t counter=0;
  for(Int_t i=0; i<fTracks->GetNTracks(); i++)
    {
      AliL3ModelTrack *track1 = (AliL3ModelTrack*)fTracks->GetCheckedTrack(i);
      if(!track1) continue;
      for(Int_t j=i+1; j<fTracks->GetNTracks(); j++)
	{
	  AliL3ModelTrack *track2 = (AliL3ModelTrack*)fTracks->GetCheckedTrack(j);
	  if(!track2) continue;
	  for(Int_t k=AliL3Transform::GetFirstRow(fPatch); k<=AliL3Transform::GetLastRow(fPatch); k++)
	    {
	      if(track1->GetPadHit(k)<0 || track1->GetTimeHit(k)<0 ||
		 track2->GetPadHit(k)<0 || track2->GetTimeHit(k)<0)
		continue;
	      
	      if(track1->GetOverlap(k)>=0 || track2->GetOverlap(k)>=0) continue;
	      
	      if(abs((Int_t)rint(track1->GetPadHit(k))-(Int_t)rint(track2->GetPadHit(k))) <= fPadOverlap &&
		 abs((Int_t)rint(track1->GetTimeHit(k))-(Int_t)rint(track2->GetTimeHit(k))) <= fTimeOverlap)
		{
		  track2->SetOverlap(k,i);
		  //track1->SetOverlap(k,j);
		  counter++;
		}
	    }
	}
    }
  //cout<<"found "<<counter<<" done"<<endl;
}


void AliL3Modeller::CalcClusterWidth(Cluster *cl,Float_t &sigmaY2,Float_t &sigmaZ2)
{
  
  Float_t padw,timew;
  
  padw = AliL3Transform::GetPadPitchWidth(fPatch);
  
  Float_t charge = (Float_t)cl->fCharge;
  Float_t pad = (Float_t)cl->fPad/charge;
  Float_t time = (Float_t)cl->fTime/charge;
  Float_t s2 = (Float_t)cl->fSigmaY2/charge - pad*pad;
  
  //Save the sigmas in pad and time:
  
  sigmaY2 = (s2);// + 1./12);//*padw*padw;
  
  /*Constants added by offline
    if(s2 != 0)
    {
    sigmaY2 = sigmaY2*0.108;
    if(fPatch<3)
    sigmaY2 = sigmaY2*2.07;
    }
  */

  s2 = (Float_t)cl->fSigmaZ2/charge - time*time;
  timew = AliL3Transform::GetZWidth();
  sigmaZ2 = (s2);// +1./12);//*timew*timew;
  

  
  /*Constants added by offline
    if(s2 != 0)
    {
    sigmaZ2 = sigmaZ2*0.169;
    if(fPatch < 3)
    sigmaZ2 = sigmaZ2*1.77;
    }
  */
}

