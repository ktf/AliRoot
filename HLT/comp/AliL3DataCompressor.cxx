// @(#) $Id$

// Author: Anders Vestbo <mailto:vestbo@fi.uib.no>
//*-- Copyright &copy ALICE HLT Group

#include "AliL3StandardIncludes.h"

#include "AliL3Logging.h"
#include "AliL3RootTypes.h"
#include "AliL3Transform.h"
#include "AliL3MemHandler.h"
#include "AliL3SpacePointData.h"
#include "AliL3Compress.h"
#include "AliL3TrackArray.h"
#include "AliL3ModelTrack.h"
#include "AliL3Benchmark.h"
#include "AliL3ClusterFitter.h"

#ifdef use_aliroot
#include "AliL3FileHandler.h"

#include <AliKalmanTrack.h>
#include <AliTPCtracker.h>
#include <AliTPCcluster.h>
#include <AliTPCParamSR.h>
#include <AliTPCDigitsArray.h>
#include <AliTPCClustersArray.h>
#include <AliTPCcluster.h>
#include <AliTPCClustersRow.h>
#include <AliSimDigits.h>
#include <AliTPC.h>
#include <AliTPCv2.h>
#include <AliRun.h>
#endif

#ifdef use_root
#include <TFile.h>
#include <TMath.h>
#include <TDirectory.h>
#include <TSystem.h>
#include <TH2F.h>
#endif

#include "AliL3DataCompressor.h"

#if GCCVERSION == 3
using namespace std;
#endif

//_____________________________________________________________
//
//  AliL3DataCompression
//
// Interface class; binary <-> AliROOT handling of TPC data compression classes.
//


ClassImp(AliL3DataCompressor)

Int_t AliL3DataCompressor::fNumTimeBits = 12;
Int_t AliL3DataCompressor::fNumPadBits = 12;
Int_t AliL3DataCompressor::fNumChargeBits = 14;
Int_t AliL3DataCompressor::fNumShapeBits = 14;
Float_t AliL3DataCompressor::fXYResidualStep = 0.03;
Float_t AliL3DataCompressor::fZResidualStep = 0.05;
Float_t AliL3DataCompressor::fXYWidthStep = 0.005;
Float_t AliL3DataCompressor::fZWidthStep = 0.005;
Int_t AliL3DataCompressor::fClusterCharge = 100;

AliL3DataCompressor::AliL3DataCompressor()
{
  fBenchmark=0;
  fInputTracks=0;
  fKeepRemaining=kTRUE;
  fEvent=0;
  fWriteClusterShape=kFALSE;
  fOutputFile=0;
  fCompRatioFile=0;
  memset(fClusters,0,36*6*sizeof(AliL3SpacePointData*));
}

AliL3DataCompressor::AliL3DataCompressor(Char_t *path,Bool_t keep,Bool_t writeshape)
{
  strcpy(fPath,path);
  fBenchmark = new AliL3Benchmark();
  fInputTracks=0;
  fKeepRemaining=keep;
  fWriteClusterShape = writeshape;
  fEvent=0;
  fOutputFile=0;
  memset(fClusters,0,36*6*sizeof(AliL3SpacePointData*));
#ifdef use_root
  Char_t name[1024];
  sprintf(name,"rm -f %s/comp/*",path);//Clean the directory
  gSystem->Exec(name);
#endif
  OpenOutputFile();
}

AliL3DataCompressor::~AliL3DataCompressor()
{
  if(fInputTracks)
    delete fInputTracks;
  if(fBenchmark)
    delete fBenchmark;
  if(fClusters)
    {
      for(Int_t i=0; i<36; i++)
	for(Int_t j=0; j<6; j++)
	  if(fClusters[i][j])
	    delete fClusters[i][j];
    }
  CloseOutputFile();
}

void AliL3DataCompressor::DoBench(Char_t *fname)
{
  fBenchmark->Analyze(fname);
}

void AliL3DataCompressor::SetBitNumbers(Int_t pad,Int_t time,Int_t charge,Int_t shape)
{
  fNumPadBits = pad;
  fNumTimeBits = time;
  fNumChargeBits = charge;
  fNumShapeBits = shape;
}

void AliL3DataCompressor::SetResolutions(Float_t xyresidual,Float_t zresidual,Int_t clustercharge,Float_t xywidth,Float_t zwidth)
{
  fXYResidualStep = xyresidual;
  fZResidualStep = zresidual;
  fXYWidthStep = xywidth;
  fZWidthStep = zwidth;
  fClusterCharge = clustercharge;
}

void AliL3DataCompressor::OpenOutputFile()
{
#ifndef use_aliroot
   LOG(AliL3Log::kError,"AliL3DataCompressor::OpenOutputFile","Version")
     <<"You have to compile with use_aliroot flag in order to use this function"<<ENDLOG;
#else
  Char_t filename[1024];
  
  sprintf(filename,"%s/comp/comprates.txt",fPath);
  fCompRatioFile = new ofstream(filename);
  
  if(fOutputFile)
    if(fOutputFile->IsOpen())
      fOutputFile->Close();

  sprintf(filename,"%s/alirunfile.root",fPath);
  TFile *f = TFile::Open(filename);
  AliTPCParam *param = (AliTPCParam*)f->Get(AliL3Transform::GetParamName());
  sprintf(filename,"%s/comp/AliTPCclusters.root",fPath);
  fOutputFile = TFile::Open(filename,"RECREATE");
  param->Write(param->GetTitle());
  f->Close();
#endif
}

void AliL3DataCompressor::CloseOutputFile()
{
  if(fCompRatioFile)
    {
      fCompRatioFile->close();
      delete fCompRatioFile;
    }
  
  if(!fOutputFile)
    return;
#ifdef use_root
  if(!fOutputFile->IsOpen())
    return;
  fOutputFile->Close();
#else
  fclose(fOutputFile);
#endif
  fOutputFile=0;
}

void AliL3DataCompressor::LoadData(Int_t event,Bool_t sp)
{
  fSinglePatch=sp;
  fEvent=event;
  AliL3MemHandler *clusterfile[36][6];
  Char_t fname[1024];
  for(Int_t s=0; s<=35; s++)
    {
      for(Int_t p=0; p<6; p++)
	{
	  if(fClusters[s][p])
	    delete fClusters[s][p];
	  fClusters[s][p] = 0;
	  clusterfile[s][p] = new AliL3MemHandler();
	  if(fSinglePatch)
	    sprintf(fname,"%s/cf/points_%d_%d_%d.raw",fPath,fEvent,s,-1);
	  else
	    sprintf(fname,"%s/cf/points_%d_%d_%d.raw",fPath,fEvent,s,p);
	  clusterfile[s][p]->SetBinaryInput(fname);
	  
	  fClusters[s][p] = (AliL3SpacePointData*)clusterfile[s][p]->Allocate();
	  clusterfile[s][p]->Binary2Memory(fNcl[s][p],fClusters[s][p]);
	  clusterfile[s][p]->CloseBinaryInput();
	  
	  if(fSinglePatch)
	    break;
	}
    }
  
  sprintf(fname,"%s/cf/tracks_%d.raw",fPath,fEvent);
  AliL3MemHandler *tfile = new AliL3MemHandler();
  tfile->SetBinaryInput(fname);
  
  if(fInputTracks)
    delete fInputTracks;
  fInputTracks = new AliL3TrackArray();
  tfile->Binary2TrackArray(fInputTracks);
  tfile->CloseBinaryInput();
  delete tfile;
}

void AliL3DataCompressor::FillData(Int_t min_hits,Bool_t expand)
{
  
  //Fill the track data into track and cluster structures, and write to file.
  //Preparation for compressing it.
  
  cout<<"Filling data; "<<fInputTracks->GetNTracks()<<" tracks"<<endl;
  AliL3TrackArray *comptracks = new AliL3TrackArray("AliL3ModelTrack");
  fInputTracks->QSort();
  for(Int_t i=0; i<fInputTracks->GetNTracks(); i++)
    {
      AliL3Track *intrack = fInputTracks->GetCheckedTrack(i);
      if(!intrack) continue;

      if(intrack->GetNHits()<min_hits) break;

      intrack->CalculateHelix();
      
      AliL3ModelTrack *outtrack = (AliL3ModelTrack*)comptracks->NextTrack();
      outtrack->SetNHits(intrack->GetNHits());
      outtrack->SetRowRange(intrack->GetFirstRow(),intrack->GetLastRow());
      outtrack->SetFirstPoint(intrack->GetFirstPointX(),intrack->GetFirstPointY(),intrack->GetFirstPointZ());
      outtrack->SetLastPoint(intrack->GetLastPointX(),intrack->GetLastPointY(),intrack->GetLastPointZ());
      outtrack->SetPt(intrack->GetPt());
      outtrack->SetPsi(intrack->GetPsi());
      outtrack->SetTgl(intrack->GetTgl());
      outtrack->SetCharge(intrack->GetCharge());
      outtrack->CalculateHelix();
      Int_t nhits = intrack->GetNHits();
      UInt_t *hitids = intrack->GetHitNumbers();
      Int_t origslice = (hitids[nhits-1]>>25)&0x7f;
      outtrack->Init(origslice,-1);
      for(Int_t j=nhits-1; j>=0; j--)
	{
	  UInt_t id=hitids[j];
	  Int_t slice = (id>>25)&0x7f;
	  Int_t patch = (id>>22)&0x7;
	  UInt_t pos = id&0x3fffff;	     

	  //UInt_t size;
	  AliL3SpacePointData *points = fClusters[slice][patch];//->GetDataPointer(size);
	  Float_t xyz[3] = {points[pos].fX,points[pos].fY,points[pos].fZ};
	  Int_t padrow = points[pos].fPadRow;

	  //Calculate the crossing point between track and padrow
	  Float_t angle = 0; //Perpendicular to padrow in local coordinates
	  AliL3Transform::Local2GlobalAngle(&angle,slice);
	  if(!intrack->CalculateReferencePoint(angle,AliL3Transform::Row2X(padrow)))
	    {
	      cerr<<"AliL3DataCompressor::FillData : Error in crossing point calc on slice "<<slice<<" row "<<padrow<<endl;
	      outtrack->Print(kFALSE);
	      exit(5);
	    }
	  
	  Float_t xyz_cross[3] = {intrack->GetPointX(),intrack->GetPointY(),intrack->GetPointZ()};
	  
	  Int_t sector,row;
	  AliL3Transform::Slice2Sector(slice,padrow,sector,row);
	  AliL3Transform::Global2Raw(xyz_cross,sector,row);
	  AliL3Transform::Global2Raw(xyz,sector,row);
	  
	  outtrack->SetPadHit(padrow,xyz_cross[1]);
	  outtrack->SetTimeHit(padrow,xyz_cross[2]);

	  if(fWriteClusterShape)
	    {
	      Float_t angle = intrack->GetCrossingAngle(padrow,slice);
	      outtrack->SetCrossingAngleLUT(padrow,angle);
	      outtrack->CalculateClusterWidths(padrow,kTRUE);
	      Int_t patch = AliL3Transform::GetPatch(padrow);
	      Float_t sigmaY2 = points[pos].fSigmaY2 / pow(AliL3Transform::GetPadPitchWidth(patch),2);
	      Float_t sigmaZ2 = points[pos].fSigmaZ2 / pow(AliL3Transform::GetZWidth(),2);
	      outtrack->SetCluster(padrow,xyz[1],xyz[2],points[pos].fCharge,sigmaY2,sigmaZ2,3);
	    }
	  else
	    outtrack->SetCluster(padrow,xyz[1],xyz[2],points[pos].fCharge,0,0,3);
	  
	  //IMPORTANT: Set the slice in which cluster is, you need it in AliL3ModelTrack::FillTrack!
	  outtrack->GetClusterModel(padrow)->fSlice=slice;
	  points[pos].fCharge = 0;//Mark this cluster as used.
	}
      if(!expand)
	outtrack->SetNClusters(AliL3Transform::GetNRows(-1));
    }
  
  if(expand)
    ExpandTrackData(comptracks);
  
  cout<<"Writing "<<comptracks->GetNTracks()<<" tracks to file"<<endl;
  AliL3Compress *comp = new AliL3Compress(-1,-1,fPath,fWriteClusterShape,fEvent);
  comp->WriteFile(comptracks);
  delete comp;
  delete comptracks;
  
}

void AliL3DataCompressor::ExpandTrackData(AliL3TrackArray *tracks)
{
  //Loop over tracks and try to assign unused clusters.
  //Only clusters which are closer than the max. residual are taken.
  
  cout<<"Expanding "<<tracks->GetNTracks()<<" tracks"<<endl;
  for(Int_t i=0; i<tracks->GetNTracks(); i++)
    {
      AliL3ModelTrack *track = (AliL3ModelTrack*)tracks->GetCheckedTrack(i);
      if(!track) continue;
      if(track->GetNHits() == AliL3Transform::GetNRows()) continue;
      
      Int_t nhits = track->GetNHits();
      //cout<<"Expanding track with "<<nhits<<" clusters"<<endl;
      
      Int_t last_slice=-1;
      for(Int_t padrow=AliL3Transform::GetNRows()-1; padrow>=0; padrow--)
	{
	  if(track->IsPresent(padrow))
	    {
	      last_slice = track->GetClusterModel(padrow)->fSlice;
	      continue;
	    }
	  
	  if(last_slice < 0) //the outer cluster is missing, so skip it - it will be written anyhow.
	    continue;
	  
	  //Check the slice of the next padrow:
	  Int_t next_padrow = padrow-1;
	  Int_t next_slice = -1;
	  while(next_padrow >=0)
	    {
	      if(track->IsPresent(next_padrow))
		{
		  next_slice = track->GetClusterModel(next_padrow)->fSlice;
		  break;
		}
	      next_padrow--;
	    }
	  if(next_slice>=0)
	    if(next_slice != last_slice)//The track crosses a slice boundary here
	      continue;
	  
 	  //UInt_t size;
	  AliL3SpacePointData *points = fClusters[last_slice][0];//->GetDataPointer(size);
	  
	  Float_t angle = 0;
	  AliL3Transform::Local2GlobalAngle(&angle,last_slice);
	  if(!track->CalculateReferencePoint(angle,AliL3Transform::Row2X(padrow)))
	    continue;
	  Float_t xyz_cross[3] = {track->GetPointX(),track->GetPointY(),track->GetPointZ()};
	  AliL3Transform::Global2Local(xyz_cross,last_slice,kTRUE);
	  Float_t mindist = 123456789;
	  AliL3SpacePointData *closest=0;
	  for(UInt_t j=0; j<fNcl[last_slice][0]; j++)
	    {
	      if(points[j].fCharge == 0) continue;// || points[j].fPadRow != padrow) continue;
	      if(points[j].fPadRow < padrow) continue;
	      if(points[j].fPadRow > padrow) break;
	      Float_t xyz[3] = {points[j].fX,points[j].fY,points[j].fZ};
	      AliL3Transform::Global2Local(xyz,last_slice,kTRUE);
	      
	      //Check for overflow:
	      Int_t temp = (Int_t)rint((xyz_cross[1]-xyz[1])/GetXYResidualStep());
	      if( abs(temp) > 1<<(GetNPadBits()-1))
		continue;
	      
	      temp = (Int_t)rint((xyz_cross[2]-xyz[2])/GetZResidualStep());
	      if( abs(temp) > 1<<(GetNTimeBits()-1))
		continue;
	      
	      Float_t dist = sqrt( pow(xyz_cross[1]-xyz[1],2) + pow(xyz_cross[2]-xyz[2],2) );
	      if(dist < mindist)
		{
		  closest = &points[j];
		  mindist = dist;
		}
	    }
	  if(closest) //there was a cluster assigned
	    {
	      Int_t sector,row;
	      Float_t xyz[3] = {closest->fX,closest->fY,closest->fZ};
	      AliL3Transform::Slice2Sector(last_slice,padrow,sector,row);
	      AliL3Transform::Local2Raw(xyz_cross,sector,row);
	      AliL3Transform::Global2Raw(xyz,sector,row);
	      
	      track->SetPadHit(padrow,xyz_cross[1]);
	      track->SetTimeHit(padrow,xyz_cross[2]);
	      
	      if(fWriteClusterShape)
		{
		  Float_t angle = track->GetCrossingAngle(padrow,last_slice);
		  track->SetCrossingAngleLUT(padrow,angle);
		  track->CalculateClusterWidths(padrow,kTRUE);
		  Int_t patch = AliL3Transform::GetPatch(padrow);
		  Float_t sigmaY2 = closest->fSigmaY2 / pow(AliL3Transform::GetPadPitchWidth(patch),2);
		  Float_t sigmaZ2 = closest->fSigmaZ2 / pow(AliL3Transform::GetZWidth(),2);
		  track->SetCluster(padrow,xyz[1],xyz[2],closest->fCharge,sigmaY2,sigmaZ2,3);
		}
	      else
		track->SetCluster(padrow,xyz[1],xyz[2],closest->fCharge,0,0,3);
	      nhits++;
	      
	      //IMPORTANT: Set the slice in which cluster is, you need it in AliL3ModelTrack::FillTrack!
	      track->GetClusterModel(padrow)->fSlice=last_slice;
	      closest->fCharge = 0;//Mark this cluster as used.
	    }
	}
      track->SetNClusters(AliL3Transform::GetNRows());
      //cout<<"Track was assigned "<<nhits<<" clusters"<<endl;
    }
  
}

void AliL3DataCompressor::WriteRemaining(Bool_t select)
{
  //Write remaining clusters (not assigned to any tracks) to file

  
  if(!fKeepRemaining)
    return;
  
  if(select)
    SelectRemainingClusters();
  
  Char_t filename[1024];
  
  if(!fSinglePatch)
    {
      cerr<<"AliL3Compressor::WriteRemaining : You have to modify this function when not running singlepatch"<<endl;
      return;
    }

  cout<<"Writing remaining clusters "<<endl;
  Int_t nrows = AliL3Transform::GetNRows();
  Int_t *npoints = new Int_t[nrows];
  for(Int_t i=0; i<=35; i++)
    {
      for(Int_t patch=0; patch < 1; patch++)
	{
	  sprintf(filename,"%s/comp/remains_%d_%d_%d.raw",fPath,fEvent,i,-1);
	  FILE *outfile = fopen(filename,"w");
	  if(!outfile)
	    {
	      cerr<<"AliL3DataCompressor::WriteRemaining : Cannot open file "<<filename<<endl;
	      exit(5);
	    }
	  //UInt_t dummy;
	  AliL3SpacePointData *points = fClusters[i][patch];//->GetDataPointer(dummy);
	  
	  memset(npoints,0,nrows*sizeof(Int_t));
	  
	  for(UInt_t j=0; j<fNcl[i][patch]; j++)
	    {
	      if(points[j].fCharge == 0) continue; //has been used
	      npoints[points[j].fPadRow]++;
	    }
	  Int_t size =0;
	  Byte_t *data = 0;
	  AliL3RemainingRow *tempPt=0;
	  
	  Int_t last_row = -2;
	  Int_t localcounter=0;
	  
	  for(UInt_t j=0; j<fNcl[i][patch]; j++)
	    {
	      if(points[j].fCharge == 0) continue; //has been used
	      
	      Int_t padrow = points[j].fPadRow;
	      if(padrow != last_row)
		{
		  if(last_row != -2)
		    {
		      if(!tempPt)
			{
			  cerr<<"AliL3DataCompressor::WriteRemaining : Zero row pointer "<<endl;
			  exit(5);
			}
		      if(localcounter != tempPt->fNClusters)
			{
			  cerr<<"AliL3DataCompressor::WriteRemaining : Mismatching clustercounter "<<localcounter<<" "
			      <<(Int_t)tempPt->fNClusters<<endl;
			  exit(5);
			}
		      //cout<<"Writing row "<<(int)tempPt->fPadRow<<" with "<<(int)tempPt->fNClusters<<" clusters"<<endl;
		      fwrite(tempPt,size,1,outfile);
		    }
		  if(data)
		    delete [] data;
		  size = sizeof(AliL3RemainingRow) + npoints[padrow]*sizeof(AliL3RemainingCluster);
		  data = new Byte_t[size];
		  tempPt = (AliL3RemainingRow*)data;
		  
		  localcounter=0;
		  tempPt->fPadRow = padrow;
		  tempPt->fNClusters = npoints[padrow];
		  last_row = padrow;
		}
	      if(localcounter >= npoints[padrow])
		{
		  cerr<<"AliL3DataCompressor::WriteRemaining : Cluster counter out of range: "
		      <<localcounter<<" "<<npoints[padrow]<<endl;
		  exit(5);
		}
	      
	      Float_t xyz[3] = {points[j].fX,points[j].fY,points[j].fZ};
	      AliL3Transform::Global2Local(xyz,i,kTRUE);
	      
	      tempPt->fClusters[localcounter].fY = xyz[1];
	      tempPt->fClusters[localcounter].fZ = xyz[2];
	      tempPt->fClusters[localcounter].fCharge = points[j].fCharge;
	      tempPt->fClusters[localcounter].fSigmaY2 = points[j].fSigmaY2;
	      tempPt->fClusters[localcounter].fSigmaZ2 = points[j].fSigmaZ2;
	      localcounter++;
	    }
	  //Write the last row:
	  //cout<<"Writing row "<<(int)tempPt->fPadRow<<" with "<<(int)tempPt->fNClusters<<" clusters"<<endl;
	  fwrite(tempPt,size,1,outfile);
	  if(data)
	    delete [] data;
	  fclose(outfile);
	}
    }
  delete [] npoints;
}

void AliL3DataCompressor::SelectRemainingClusters()
{
  //Select which remaining clusters to write in addition to the compressed data.

  cout<<"Cleaning up clusters"<<endl;
  Int_t nrows = AliL3Transform::GetNRows();
  Int_t gap=(Int_t)(0.125*nrows), shift=(Int_t)(0.5*gap);
  
  for(Int_t slice=0; slice<36; slice++)
    {
      //UInt_t dummy;
      AliL3SpacePointData *points = fClusters[slice][0];//->GetDataPointer(dummy);
      for(UInt_t i=0; i<fNcl[slice][0]; i++)
	{
	  if(points[i].fCharge == 0) continue; //Already removed
	  Int_t padrow = (Int_t)points[i].fPadRow;
	  
	  Float_t xyz[3] = {points[i].fX,points[i].fY,points[i].fZ};
	  Int_t sector,row;
	  AliL3Transform::Slice2Sector(slice,padrow,sector,row);
	  AliL3Transform::Global2Raw(xyz,sector,row);
	  
	  if(padrow >= nrows-1-gap-shift) continue;
	  //if(padrow >= nrows-1-shift) continue;

	  //Save the clusters at the borders:
	  if(xyz[1] < 3 || xyz[1] >= AliL3Transform::GetNPads(padrow)-4)
	    continue;

	  //Save clusters on padrows used for offline seeding:
	  if(padrow == nrows - 1 || padrow == nrows - 1 - gap ||                 //First seeding
	     padrow == nrows - 1 - shift || padrow == nrows - 1 - gap - shift)   //Second seeding
	    continue;
	  
	  //Cluster did not meet any of the above criteria, so disregard it:
	  points[i].fCharge = 0;
	}
    }
  
}

void AliL3DataCompressor::CompressAndExpand()
{
  //Read tracks/clusters from file, compress data and uncompress it. Write compression rates to file.
  cout<<"Compressing and expanding data"<<endl;
  AliL3Compress *comp = new AliL3Compress(-1,-1,fPath,fWriteClusterShape,fEvent);
  comp->CompressFile();
  comp->ExpandFile();
  comp->PrintCompRatio(fCompRatioFile);
  delete comp;
}


void AliL3DataCompressor::RestoreData()
{
  //Restore the uncompressed data together with the remaining clusters,
  //and write to a final cluster file which serves as an input to the
  //final offline tracker.
  
#ifndef use_aliroot
   LOG(AliL3Log::kError,"AliL3DataCompressor::RestoreData","Version")
     <<"You have to compile with use_aliroot flag in order to use this function"<<ENDLOG;
#else

  cout<<"Restoring data"<<endl;
  
  const Int_t maxpoints=500000;
  TempCluster **clusters = new TempCluster*[36];
  Int_t *ncl = new Int_t[36];
  for(Int_t i=0; i<36; i++)
    {
      ncl[i]=0;
      clusters[i] = new TempCluster[maxpoints];
    }
  
  ReadUncompressedData(clusters,ncl,maxpoints);
  
  if(fKeepRemaining)
    ReadRemaining(clusters,ncl,maxpoints);
  
  Char_t filename[1024];
  sprintf(filename,"%s/digitfile.root",fPath);
  TFile *rootfile = TFile::Open(filename);
  rootfile->cd();
  AliTPCParam *param = (AliTPCParam*)rootfile->Get(AliL3Transform::GetParamName());

  AliTPCDigitsArray *darray = new AliTPCDigitsArray();
  darray->Setup(param);
  darray->SetClass("AliSimDigits");
  sprintf(filename,"TreeD_%s_%d",AliL3Transform::GetParamName(),fEvent);
  Bool_t ok = darray->ConnectTree(filename);
  if(!ok)
    {
      cerr<<"AliL3DataCompressor::RestoreData : Problems connecting tree"<<endl;
      return;
    }

  fOutputFile->cd();
    
  AliTPCClustersArray *carray = new AliTPCClustersArray();
  carray->Setup(param);
  carray->SetClusterType("AliTPCcluster");
  carray->MakeTree();
  
  Int_t totcounter=0;
  for(Int_t slice=0; slice<=35; slice++)
    {
      TempCluster **clPt = new TempCluster*[maxpoints];
      cout<<"Sorting "<<ncl[slice]<<" clusters in slice "<<slice<<endl;
      for(Int_t i=0; i<ncl[slice]; i++)
	clPt[i] = &clusters[slice][i];
      
      QSort(clPt,0,ncl[slice]);
      
      //cout<<"padrow "<<clPt[i]->padrow<<" pad "<<clPt[i]->pad<<" time "<<clPt[i]->time<<endl;

      Int_t falseid=0;
      Int_t counter=0;
      for(Int_t padrow=AliL3Transform::GetFirstRow(-1); padrow<=AliL3Transform::GetLastRow(-1); padrow++)
	{
	  Int_t sec,row;
	  AliL3Transform::Slice2Sector(slice,padrow,sec,row);
	  AliTPCClustersRow *clrow=carray->CreateRow(sec,row);
	  AliSimDigits *digits = (AliSimDigits*)darray->LoadRow(sec,row);
	  digits->ExpandBuffer();
	  digits->ExpandTrackBuffer();
	  Int_t patch = AliL3Transform::GetPatch(padrow);
	  while(counter < ncl[slice] && clPt[counter]->padrow == padrow)
	    {
	      Float_t temp[3];
	      AliL3Transform::Raw2Local(temp,sec,row,clPt[counter]->pad,clPt[counter]->time);
	      
	      AliTPCcluster *c = new AliTPCcluster();
	      c->SetY(temp[1]);
	      c->SetZ(temp[2]);
	      c->SetQ(clPt[counter]->charge);
	      
	      c->SetSigmaY2(clPt[counter]->sigmaY2*pow(AliL3Transform::GetPadPitchWidth(patch),2));
	      c->SetSigmaZ2(clPt[counter]->sigmaZ2*pow(AliL3Transform::GetZWidth(),2));
	      Int_t pad = TMath::Nint(clPt[counter]->pad);
	      Int_t time = TMath::Nint(clPt[counter]->time);
	      
	      if(pad < 0)
		pad=0;
	      if(pad >= AliL3Transform::GetNPads(padrow))
		pad = AliL3Transform::GetNPads(padrow)-1;
	      if(time < 0 || time >= AliL3Transform::GetNTimeBins())
		cerr<<"row "<<padrow<<" pad "<<pad<<" time "<<time<<endl;
	      
	      for(Int_t lab=0; lab<3; lab++)
		{
		  Int_t label = digits->GetTrackIDFast(time,pad,lab);
		  if(label > 1)
		    c->SetLabel(label-2,lab);
		  else if(label==0)
		    c->SetLabel(-2,lab);
		  else
		    c->SetLabel(-1,lab);
		  if(lab==0 && c->GetLabel(0) < 0)
		    {
		      falseid++;
		      //AliL3Transform::Local2Global(temp,slice);
		      //cout<<"slice "<<slice<<" padrow "<<padrow<<" y "<<temp[1]<<" z "<<temp[2]<<" label "<<c->GetLabel(0)<<endl;
		    }
		}
	      //cout<<"row "<<padrow<<" pad "<<clPt[counter]->pad<<" time "<<clPt[counter]->time<<" sigmaY2 "<<c->GetSigmaY2()<<" sigmaZ2 "<<c->GetSigmaZ2()<<endl;
	      clrow->InsertCluster(c);
	      delete c;
	      counter++;
	      totcounter++;
	    }
	  carray->StoreRow(sec,row);
	  carray->ClearRow(sec,row);
	  darray->ClearRow(sec,row);
	}
      //cerr<<"Slice "<<slice<<" nclusters "<<counter<<" falseones "<<falseid<<endl;
      if(counter != ncl[slice])
	cerr<<"AliLDataCompressor::RestoreData : Mismatching cluster count :"<<counter<<" "<<ncl[slice]<<endl;
      delete [] clPt;
    }

  cout<<"Writing "<<totcounter<<" clusters to rootfile "<<endl;

  sprintf(filename,"TreeC_TPC_%d",fEvent);
  carray->GetTree()->SetName(filename);
  carray->GetTree()->Write();
  delete carray;
  delete darray;
  rootfile->Close();
  
  for(Int_t i=0; i<36; i++)
    delete [] clusters[i];
  delete [] clusters;
  delete [] ncl;
#endif
}

void AliL3DataCompressor::ReadUncompressedData(TempCluster **clusters,Int_t *ncl,const Int_t maxpoints)
{

  cout<<"Reading uncompressed tracks "<<endl;
  AliL3Compress *comp = new AliL3Compress(-1,-1,fPath,fWriteClusterShape,fEvent);
  
  if(!comp->ReadFile('u'))
    return;
  
  AliL3TrackArray *tracks = comp->GetTracks();
  
  Int_t charge;
  Float_t pad,time,sigmaY2,sigmaZ2;
  for(Int_t i=0; i<tracks->GetNTracks(); i++)
    {
      AliL3ModelTrack *track = (AliL3ModelTrack*)tracks->GetCheckedTrack(i);
      if(!track) continue;
      for(Int_t padrow=0; padrow < AliL3Transform::GetNRows(-1); padrow++)
	{
	  if(!track->IsPresent(padrow)) continue;
	  track->GetPad(padrow,pad);
	  track->GetTime(padrow,time);
	  track->GetClusterCharge(padrow,charge);
	  track->GetXYWidth(padrow,sigmaY2);
	  track->GetZWidth(padrow,sigmaZ2);
	  Int_t slice = track->GetClusterModel(padrow)->fSlice;
	  /*
	    if(pad < -1 || pad > AliL3Transform::GetNPads(padrow) || time < -1 || time > AliL3Transform::GetNTimeBins())
	    {
	    cerr<<"AliL3DataCompressor::ReadUncompressData : Wrong pad "<<pad<<" or time "<<time<<" on row "<<padrow<<" track index "<<i<<endl;
	    track->Print();
	    exit(5);
	    }
	  */
	  if(ncl[slice] >= maxpoints)
	    {
	      cerr<<"AliL3DataCompressor::ReadUncompressedData : Too many clusters"<<endl;
	      exit(5);
	    }
	  clusters[slice][ncl[slice]].pad = pad;
	  clusters[slice][ncl[slice]].time = time;
	  clusters[slice][ncl[slice]].charge = charge;
	  clusters[slice][ncl[slice]].sigmaY2 = sigmaY2;
	  clusters[slice][ncl[slice]].sigmaZ2 = sigmaZ2;
	  clusters[slice][ncl[slice]].padrow = padrow;
	  //cout<<"row "<<padrow<<" pad "<<pad<<" time "<<time<<" charge "<<charge<<" sigmas "<<sigmaY2<<" "<<sigmaZ2<<endl;
	  ncl[slice]++;
	}
    }

  delete comp;
}

void AliL3DataCompressor::ReadRemaining(TempCluster **clusters,Int_t *ncl,const Int_t maxpoints)
{
  
  Char_t filename[1024];
  cout<<"Reading remaining clusters "<<endl;
  AliL3MemHandler mem;
  
  for(Int_t slice=0; slice<=35; slice++)
    {
      for(Int_t p=0; p<1; p++)
	{
	  sprintf(filename,"%s/comp/remains_%d_%d_%d.raw",fPath,fEvent,slice,-1);
	  
	  mem.SetBinaryInput(filename);
	  AliL3RemainingRow *tempPt = (AliL3RemainingRow*)mem.Allocate();
	  
	  Int_t nrows=0;
	  FILE *infile = mem.GetFilePointer();
	  while(!feof(infile))
	    {
	      Byte_t *dPt = (Byte_t*)tempPt;
	      if(fread(tempPt,sizeof(AliL3RemainingRow),1,infile)!=1) break;
	      
	      dPt += sizeof(AliL3RemainingRow);
	      
	      Int_t size = sizeof(AliL3RemainingCluster)*tempPt->fNClusters;
	      
	      fread(dPt,size,1,infile);
	      dPt += size;
	      tempPt = (AliL3RemainingRow*)dPt;
	      nrows++;
	    }
	  
	  mem.CloseBinaryInput();
	  UInt_t dummy;
	  tempPt = (AliL3RemainingRow*)mem.GetDataPointer(dummy);
	  
	  for(Int_t i=0; i<nrows; i++)
	    {
	      AliL3RemainingCluster *points = tempPt->fClusters;
	      Int_t padrow = (Int_t)tempPt->fPadRow;
	      Int_t patch = AliL3Transform::GetPatch(padrow);
	      Int_t sector,row;
	      AliL3Transform::Slice2Sector(slice,padrow,sector,row);
	      //cout<<"Loading slice "<<slice<<" row "<<padrow<<" with "<<(Int_t)tempPt->fNClusters<<" clusters "<<endl;
	      for(Int_t j=0; j<tempPt->fNClusters; j++)
		{
		  
		  Float_t xyz[3] = {AliL3Transform::Row2X(padrow),points[j].fY,points[j].fZ};
		  
		  AliL3Transform::Local2Raw(xyz,sector,row);
		  
		  if(ncl[slice] >= maxpoints)
		    {
		      cerr<<"AliL3DataCompressor::ReadRemaining : Too many clusters"<<endl;
		      exit(5);
		    }
		  //cout<<"slice "<<slice<<" padrow "<<padrow<<" pad "<<xyz[1]<<" time "<<xyz[2]<<endl;
		  clusters[slice][ncl[slice]].pad = xyz[1];
		  clusters[slice][ncl[slice]].time = xyz[2];
		  clusters[slice][ncl[slice]].charge = points[j].fCharge;
		  clusters[slice][ncl[slice]].sigmaY2 = points[j].fSigmaY2/pow(AliL3Transform::GetPadPitchWidth(patch),2);
		  clusters[slice][ncl[slice]].sigmaZ2 = points[j].fSigmaZ2/pow(AliL3Transform::GetZWidth(),2);
		  clusters[slice][ncl[slice]].padrow = padrow;
		  ncl[slice]++;
		}
	      Byte_t *dPt = (Byte_t*)tempPt;
	      Int_t size = sizeof(AliL3RemainingRow) + tempPt->fNClusters*sizeof(AliL3RemainingCluster);
	      dPt += size;
	      tempPt = (AliL3RemainingRow*)dPt;
	    }
	  
	  mem.Free();
	}
    }
}

void AliL3DataCompressor::QSort(TempCluster **a, Int_t first, Int_t last)
{
  static TempCluster *tmp;
   static int i;           // "static" to save stack space
   int j;

   while (last - first > 1) {
      i = first;
      j = last;
      for (;;) {
	while (++i < last && Compare(a[i], a[first]) < 0)
	  ;
	while (--j > first && Compare(a[j], a[first]) > 0)
	  ;
         if (i >= j)
            break;

         tmp  = a[i];
         a[i] = a[j];
         a[j] = tmp;
      }
      if (j == first) {
         ++first;
         continue;
      }
      tmp = a[first];
      a[first] = a[j];
      a[j] = tmp;
      if (j - first < last - (j + 1)) {
         QSort(a, first, j);
         first = j + 1;   // QSort(j + 1, last);
      } else {
         QSort(a, j + 1, last);
         last = j;        // QSort(first, j);
      }
   }
}

Int_t AliL3DataCompressor::Compare(TempCluster *a,TempCluster *b)
{
  /*
  if(a->padrow < 0 || a->padrow > AliL3Transform::GetNRows(-1) ||
     b->padrow < 0 || b->padrow > AliL3Transform::GetNRows(-1))
    {
      cerr<<"AliL3Compressor::Compare : Wrong padrows "<<a->padrow<<" "<<b->padrow<<endl;
      exit(5);
    }
  else if(a->pad < 0 || a->pad > AliL3Transform::GetNPads(a->padrow) || 
	  b->pad < 0 || b->pad > AliL3Transform::GetNPads(b->padrow))
    {
      cerr<<"AliL3Compressor::Compare : Wrong pads "<<a->pad<<" "<<b->pad<<endl;
      exit(5);
    }
  else if(a->time < 0 || a->time > AliL3Transform::GetNTimeBins() || 
	  b->time < 0 || b->time > AliL3Transform::GetNTimeBins())
    {
      cerr<<"AliL3Compressor::Compare : Wrong timebins "<<a->time<<" "<<b->time<<endl;
      exit(5);
    }
  */
  if(a->padrow < b->padrow) return -1;
  if(a->padrow > b->padrow) return 1;

  if(rint(a->pad) == rint(b->pad) && rint(a->time) == rint(b->time)) return 0;
  
  if(rint(a->pad) < rint(b->pad)) return -1;
  if(rint(a->pad) == rint(b->pad) && rint(a->time) < rint(b->time)) return -1;
  
  return 1;
}

void AliL3DataCompressor::LoadOfflineData(Int_t event)
{
  //Take offline reconstructed tracks as an input.
  //In this case, no remaining clusters are written.
  
#ifndef use_aliroot
   LOG(AliL3Log::kError,"AliL3DataCompressor::LoadOfflineData","Version")
     <<"You have to compile with use_aliroot flag in order to use this function"<<ENDLOG;
#else

  char filename[1024];
  
  AliKalmanTrack::SetConvConst(1000/0.299792458/AliL3Transform::GetSolenoidField());
  sprintf(filename,"%s/offline/AliTPCclusters.root",fPath);

  TFile *in = TFile::Open(filename);
  AliTPCParam *param=(AliTPCParam*)in->Get("75x40_100x60_150x60");
  
  AliTPCtracker *tracker = new AliTPCtracker(param);
  tracker->SetEventNumber(event);
  tracker->LoadClusters();
  
  const Int_t MAX=20000;
  Int_t nentr=0,i=0; TObjArray tarray(MAX);
  sprintf(filename,"%s/offline/AliTPCtracks.root",fPath);
  TFile *tf=TFile::Open(filename);
  
  char tname[100]; sprintf(tname,"TreeT_TPC_%d",event);
  TTree *tracktree=(TTree*)tf->Get(tname);
  
  TBranch *tbranch=tracktree->GetBranch("tracks");
  nentr=(Int_t)tracktree->GetEntries();
  AliTPCtrack *iotrack=0;

  for (i=0; i<nentr; i++) {
    iotrack=new AliTPCtrack;
    tbranch->SetAddress(&iotrack);
    tracktree->GetEvent(i);
    tarray.AddLast(iotrack);
  }   
  delete tracktree; 
  tf->Close();
  
  AliL3TrackArray *comptracks = new AliL3TrackArray("AliL3ModelTrack");
  cout<<"Loaded "<<nentr<<" offline tracks"<<endl;
  Int_t slice,padrow;
  Int_t totcounter=0;
  for(i=0; i<nentr; i++)
    {
      
      AliTPCtrack *track=(AliTPCtrack*)tarray.UncheckedAt(i);
      Int_t nhits = track->GetNumberOfClusters();
      Int_t idx = track->GetClusterIndex(nhits-1);
      Int_t sec=(idx&0xff000000)>>24, row=(idx&0x00ff0000)>>16;
      
      if(sec >= 18)
	sec += 18;
      
      AliL3Transform::Sector2Slice(slice,padrow,sec,row);
      Double_t par[5],xk=AliL3Transform::Row2X(padrow);
      track->PropagateTo(xk);
      track->GetExternalParameters(xk,par);
      Double_t psi = TMath::ASin(par[2]) + track->GetAlpha();
      if (psi<-TMath::Pi()) psi+=2*TMath::Pi();
      if (psi>=TMath::Pi()) psi-=2*TMath::Pi();
      Float_t pt_1=TMath::Abs(par[4]);
      Int_t charge = 1;
      if(par[4] > 0)
	charge=-1;

      Float_t first[3];
      AliCluster *fcl = tracker->GetCluster(idx);
      first[0] = xk;
      first[1] = fcl->GetY();
      first[2] = fcl->GetZ();

      AliL3Transform::Local2Global(first,slice);
      
      AliL3ModelTrack *outtrack = (AliL3ModelTrack*)comptracks->NextTrack();
      outtrack->SetNHits(nhits);
      outtrack->SetFirstPoint(first[0],first[1],first[2]);
      outtrack->SetPt(1/pt_1);
      outtrack->SetPsi(psi);
      outtrack->SetTgl(par[3]);
      outtrack->SetCharge(charge);
      outtrack->CalculateHelix();
      outtrack->Init(0,-1);

      //for(int j=0; j<nhits; j++)
      for(int j=nhits-1; j>=0; j--)
	{
	  Int_t index = track->GetClusterIndex(j);
	  
	  Float_t xyz[3];
	  Int_t clustercharge =0;
	  
	  AliTPCcluster *cluster = (AliTPCcluster*)tracker->GetCluster(index);
	  xyz[1] = cluster->GetY();
	  xyz[2] = cluster->GetZ();
	  clustercharge = (Int_t)cluster->GetQ();
	  
	  cluster->SetQ(-clustercharge);//Set as used
	  
	  sec=(index&0xff000000)>>24; row=(index&0x00ff0000)>>16;
	  
	  //This we do because the sector numbering of course are different internally
	  //in the fucked up AliTPCtracker class. 
	  if(sec >= 18)
	    sec += 18;
 	  
	  if(xyz[2] < 0)
	    sec += 18;

	  //cout<<"sector "<<sec<<" row "<<row<<endl;
	  if(!AliL3Transform::Sector2Slice(slice,padrow,sec,row))
	    exit(5);
	  xyz[0] = AliL3Transform::Row2X(padrow);
	  
	  //cout<<"Hit in slice "<<slice<<" padrow "<<padrow<<" y "<<cluster->GetY()<<" z "<<cluster->GetZ()<<endl;
	  AliL3Transform::Local2Raw(xyz,sec,row);
	  //cout<<"slice "<<slice<<" padrow "<<padrow<<" pad "<<xyz[1]<<" time "<<xyz[2]<<endl;
	  
	  if(xyz[1] < -1 || xyz[1] > AliL3Transform::GetNPads(padrow) ||
	     xyz[2] < -1 || xyz[2] > AliL3Transform::GetNTimeBins())
	    {
	      cerr<<"AliL3DataCompressor::FillOfflineData : Wrong time "<<xyz[2]<<" in slice "
		  <<slice<<" padrow "<<padrow<<endl;
	      cout<<"sector "<<sec<<" row "<<row<<endl;
	      //cout<<"Hit in slice "<<slice<<" padrow "<<padrow<<" y "<<cluster->GetY()<<" z "<<cluster->GetZ()<<endl;
	      cout<<"Track hit "<<xyz[0]<<" "<<xyz[1]<<" "<<xyz[2]<<endl;
	      exit(5);
	    }
	  
	  Float_t angle = 0;
	  AliL3Transform::Local2GlobalAngle(&angle,slice);
	  if(!outtrack->CalculateReferencePoint(angle,AliL3Transform::Row2X(padrow)))
	    {
	      cerr<<"AliL3DataCompressor::FillOfflineData : Error in crossing point calc on slice "
		  <<slice<<" row "<<padrow<<endl;
	      exit(5);
	    }
	  Float_t xyz_cross[3] = {outtrack->GetPointX(),outtrack->GetPointY(),outtrack->GetPointZ()};
	  AliL3Transform::Global2Raw(xyz_cross,sec,row);
	  /*
	    if(fabs(xyz_cross[1] - xyz[1]) > 10 ||
	    fabs(xyz_cross[2] - xyz[2]) > 10)
	    {
	    cout<<"AliL3DataCompressor::FillOfflineData : Wrong crossing slice "<<slice<<" padrow "
	    <<padrow<<" pad "<<xyz[1]<<" padhit "<<xyz_cross[1]<<" time "<<xyz[2]<<" timehit "<<xyz_cross[2]<<endl;
	    outtrack->Print();
	    exit(5);
	    }
	  */
	  //cout<<" crossing "<<xyz_cross[0]<<" "<<xyz_cross[1]<<" "<<xyz_cross[2]<<endl;
	  outtrack->SetPadHit(padrow,xyz_cross[1]);
	  outtrack->SetTimeHit(padrow,xyz_cross[2]);
	  
	  if(fWriteClusterShape)
	    {
	      Float_t angle = outtrack->GetCrossingAngle(padrow,slice);
	      outtrack->SetCrossingAngleLUT(padrow,angle);
	      outtrack->CalculateClusterWidths(padrow,kTRUE);
	      Int_t patch = AliL3Transform::GetPatch(padrow);
	      Float_t sigmaY2 = cluster->GetSigmaY2() / pow(AliL3Transform::GetPadPitchWidth(patch),2);
	      Float_t sigmaZ2 = cluster->GetSigmaZ2() / pow(AliL3Transform::GetZWidth(),2);
	      outtrack->SetCluster(padrow,xyz[1],xyz[2],clustercharge,sigmaY2,sigmaZ2,3);
	    }
	  else
	    outtrack->SetCluster(padrow,xyz[1],xyz[2],clustercharge,0,0,3);
	  totcounter++;
	  outtrack->GetClusterModel(padrow)->fSlice = slice;
	}
      /*
      int label = track->GetLabel();
      if(label == 10019 || label==6281 || label== 5326 || label==3054 || label==1366 || label==1005)
	outtrack->Print();
      */
    }
  
  tracker->UnloadClusters();
  
  cout<<"AliL3DataCompressor::FillOfflineData : Wrote "<<totcounter<<" clusters"<<endl;
  //Write tracks to file
  AliL3Compress *comp = new AliL3Compress(-1,-1,fPath,fWriteClusterShape,fEvent);
  comp->WriteFile(comptracks);
  delete comp;
  delete comptracks;
  delete tracker;
#endif
}
