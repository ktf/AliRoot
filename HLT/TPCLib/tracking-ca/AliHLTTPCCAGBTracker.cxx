// $Id$
// **************************************************************************
// This file is property of and copyright by the ALICE HLT Project          * 
// ALICE Experiment at CERN, All rights reserved.                           *
//                                                                          *
// Primary Authors: Sergey Gorbunov <sergey.gorbunov@kip.uni-heidelberg.de> *
//                  Ivan Kisel <kisel@kip.uni-heidelberg.de>                *
//                  for The ALICE HLT Project.                              *
//                                                                          *
// Permission to use, copy, modify and distribute this software and its     *
// documentation strictly for non-commercial purposes is hereby granted     *
// without fee, provided that the above copyright notice appears in all     *
// copies and that both the copyright notice and this permission notice     *
// appear in the supporting documentation. The authors make no claims       *
// about the suitability of this software for any purpose. It is            *
// provided "as is" without express or implied warranty.                    *
//                                                                          *
//***************************************************************************


#include "AliHLTTPCCAGBTracker.h"
#include "AliHLTTPCCAGBHit.h"
#include "AliHLTTPCCAOutTrack.h"
#include "AliHLTTPCCATracker.h"
#include "AliHLTTPCCAGBTrack.h"
#include "AliHLTTPCCATrackParam.h"
#include "AliHLTTPCCAMerger.h"
#include "AliHLTTPCCAMergerOutput.h"
#include "AliHLTTPCCADataCompressor.h"
#include "AliHLTTPCCAMath.h"
#include "AliHLTTPCCATrackLinearisation.h"
#include "TStopwatch.h"

//#define DRAW

#ifdef DRAW
#include "AliHLTTPCCADisplay.h"
#endif //DRAW


AliHLTTPCCAGBTracker::AliHLTTPCCAGBTracker()
  :
    fSlices(0), 
    fNSlices(0), 
    fHits(0),
    fExt2IntHitID(0),
    fNHits(0),
    fTrackHits(0), 
    fTracks(0), 
    fNTracks(0),
    fMerger(0),    
    fTime(0),
    fStatNEvents(0),
    fSliceTrackerTime(0)
{
  //* constructor
  for( Int_t i=0; i<20; i++ ) fStatTime[i] = 0;
  fMerger = new AliHLTTPCCAMerger;
}

AliHLTTPCCAGBTracker::AliHLTTPCCAGBTracker(const AliHLTTPCCAGBTracker&)
  : 
    fSlices(0), 
    fNSlices(0), 
    fHits(0),
    fExt2IntHitID(0),
    fNHits(0),
    fTrackHits(0), 
    fTracks(0), 
    fNTracks(0),
    fMerger(0),
    fTime(0),
    fStatNEvents(0),
    fSliceTrackerTime(0)
{
  //* dummy
}

const AliHLTTPCCAGBTracker &AliHLTTPCCAGBTracker::operator=(const AliHLTTPCCAGBTracker&) const
{
  //* dummy
  return *this;
}

AliHLTTPCCAGBTracker::~AliHLTTPCCAGBTracker()
{
  //* destructor
  StartEvent();
  if( fSlices ) delete[] fSlices;
  fSlices=0;
  delete fMerger;
}

void AliHLTTPCCAGBTracker::SetNSlices( Int_t N )
{
  //* set N of slices
  StartEvent();
  fNSlices = N;
  if( fSlices ) delete[] fSlices;
  fSlices=0;
  fSlices = new AliHLTTPCCATracker[N];
}

void AliHLTTPCCAGBTracker::StartEvent()
{
  //* clean up track and hit arrays
  if( fTrackHits ) delete[] fTrackHits;
  fTrackHits = 0;
  if( fTracks ) delete[] fTracks;
  fTracks = 0;
  if( fHits ) delete[] fHits;
  fHits=0;
  if( fExt2IntHitID ) delete[] fExt2IntHitID;
  fExt2IntHitID = 0;
  fNHits = 0;
  fNTracks = 0;
  for( Int_t i=0; i<fNSlices; i++) fSlices[i].StartEvent();
}


void AliHLTTPCCAGBTracker::SetNHits( Int_t nHits )
{
  //* set the number of hits
  if( fHits ) delete[] fHits;
  fHits = 0;
  if( fExt2IntHitID ) delete[] fExt2IntHitID;
  fExt2IntHitID = 0;
  fHits = new AliHLTTPCCAGBHit[ nHits ];
  fExt2IntHitID = new Int_t[ nHits ];
  fNHits = 0;
}  

void AliHLTTPCCAGBTracker::ReadHit( Float_t x, Float_t y, Float_t z, 
				    Float_t errY, Float_t errZ, Float_t amp,
				    Int_t ID, Int_t iSlice, Int_t iRow )
{
  //* read the hit to local array
  AliHLTTPCCAGBHit &hit = fHits[fNHits];
  hit.SetX( x );
  hit.SetY( y );
  hit.SetZ( z );  
  hit.SetErrX( 1.e-4 );//fSlices[iSlice].Param().ErrX();
  hit.SetErrY( errY );
  hit.SetErrZ( errZ );
  hit.SetAmp( amp );
  hit.SetID( ID );
  hit.SetISlice( iSlice );
  hit.SetIRow( iRow );
  hit.SetIsUsed( 0 );
  fNHits++;  
}

void AliHLTTPCCAGBTracker::FindTracks()
{
  //* main tracking routine
  fTime = 0;
  fStatNEvents++;  

#ifdef DRAW
  AliHLTTPCCADisplay::Instance().SetGB(this);
  AliHLTTPCCADisplay::Instance().SetTPCView();
  AliHLTTPCCADisplay::Instance().DrawTPC();
  AliHLTTPCCADisplay::Instance().Ask();
#endif //DRAW  

  
  std::sort(fHits,fHits+fNHits, AliHLTTPCCAGBHit::Compare );  

  for( Int_t i=0; i<fNHits; i++ )  fExt2IntHitID[fHits[i].ID()] = i;

  // Read hits, row by row

  Int_t nHitsTotal = fNHits;
  Float_t *hitX = new Float_t [nHitsTotal];
  Float_t *hitY = new Float_t [nHitsTotal];
  Float_t *hitZ = new Float_t [nHitsTotal];

  Int_t *sliceNHits = new Int_t[ fNSlices ];
  Int_t **rowNHits = new Int_t* [fNSlices];
  for( Int_t i=0; i<fNSlices; i++ ) rowNHits[i] = new Int_t[200];

  for( Int_t is=0; is<fNSlices; is++ ){
    sliceNHits[is] = 0;
    for( Int_t ir=0; ir<200; ir++ ) rowNHits[is][ir] = 0;    
  }

  for( Int_t ih=0; ih<nHitsTotal; ih++){
    AliHLTTPCCAGBHit &h = fHits[ih];    
    sliceNHits[h.ISlice()]++;
    rowNHits[h.ISlice()][h.IRow()]++;
  }
  
  Int_t firstSliceHit = 0;
  for( Int_t is=0; is<fNSlices; is++ ){
    fFirstSliceHit[is] = firstSliceHit;
    Int_t rowFirstHits[200];
    Int_t firstRowHit = 0;
    for( Int_t ir=0; ir<200; ir++ ){
      rowFirstHits[ir] = firstRowHit;
      for( Int_t ih=0; ih<rowNHits[is][ir]; ih++){
	AliHLTTPCCAGBHit &h = fHits[firstSliceHit + firstRowHit + ih];    
	hitX[firstRowHit + ih] = h.X();
	hitY[firstRowHit + ih] = h.Y();
	hitZ[firstRowHit + ih] = h.Z();	
      }
      firstRowHit+=rowNHits[is][ir];
    }    
    fSlices[is].ReadEvent( rowFirstHits, &*rowNHits[is], hitX, hitY, hitZ, sliceNHits[is] );
   
    //Int_t data[ rowNHits[is]]
    //AliHLTTPCCAEventHeader event;

    firstSliceHit+=sliceNHits[is];
  }

  if( sliceNHits ) delete[] sliceNHits;
  if( rowNHits ){
    for( Int_t i=0; i<fNSlices; i++ ) delete[] rowNHits[i];
    delete[] rowNHits;
  }

  delete[] hitX;
  hitX = 0;
  if( hitY ) delete[] hitY;
  hitY=0;
  if( hitZ ) delete[] hitZ;
  hitZ=0;
 
  if( fNHits<=0 ) return;

  TStopwatch timer1;
  TStopwatch timer2;
  //std::cout<<"Start CA reconstruction"<<std::endl;
  for( Int_t iSlice=0; iSlice<fNSlices; iSlice++ ){
    //std::cout<<"Reconstruct slice "<<iSlice<<std::endl;
    TStopwatch timer;
    AliHLTTPCCATracker &slice = fSlices[iSlice];
    slice.Reconstruct();
    timer.Stop();
    //fTime+= timer.CpuTime();
    //blaTime+= timer.CpuTime();
    fStatTime[0] += timer.CpuTime();
    fStatTime[1]+=slice.Timer(0);
    fStatTime[2]+=slice.Timer(1);
    fStatTime[3]+=slice.Timer(2);
    fStatTime[4]+=slice.Timer(3);
    fStatTime[5]+=slice.Timer(4);
    fStatTime[6]+=slice.Timer(5);
    fStatTime[7]+=slice.Timer(6);
    fStatTime[8]+=slice.Timer(7);
  }

  timer2.Stop();
  //std::cout<<"blaTime = "<<timer2.CpuTime()*1.e3<<std::endl;
  fSliceTrackerTime = timer2.CpuTime();
  
  //std::cout<<"Start CA merging"<<std::endl;

  TStopwatch timerMerge;
  Merge(); 
  timerMerge.Stop();
  fStatTime[9]+=timerMerge.CpuTime();  
  //fTime+=timerMerge.CpuTime();
  //std::cout<<"Merge time = "<<timerMerge.CpuTime()*1.e3<<"ms"<<std::endl;
  //std::cout<<"End CA merging"<<std::endl;
  timer1.Stop();
  fTime+= timer1.CpuTime();

#ifdef DRAW
  AliHLTTPCCADisplay::Instance().Ask();
#endif //DRAW
}

void AliHLTTPCCAGBTracker::Merge()
{
  // test 

#ifdef DRAW
  AliHLTTPCCADisplay &disp = AliHLTTPCCADisplay::Instance();
  AliHLTTPCCADisplay::Instance().SetTPCView();
  AliHLTTPCCADisplay::Instance().DrawTPC();
  AliHLTTPCCADisplay::Instance().DrawGBHits( *this );
  disp.Ask(); 
  std::cout<<"Slice tracks:"<<std::endl;
  for( Int_t iSlice=0; iSlice<fNSlices; iSlice++ ){
    AliHLTTPCCATracker &slice = fSlices[iSlice];
    disp.SetCurrentSlice(&slice);    
    for( Int_t itr=0; itr<*slice.NOutTracks(); itr++ ){
      disp.DrawSliceOutTrack( itr, kBlue, 2. );
    }
  }
  //AliHLTTPCCADisplay::Instance().DrawGBHits( *this );
  disp.Ask(); 
#endif //DRAW  


  AliHLTTPCCAMerger &merger = *fMerger;

  merger.Clear();
  merger.SetSliceParam( fSlices[0].Param() );

  for( Int_t i=0; i<fNSlices; i++ ) {
    merger.SetSliceData( i, fSlices[i].Output() );
  }

  merger.Reconstruct();

  const AliHLTTPCCAMergerOutput &out = *(merger.Output());
  
  
  if( fTrackHits ) delete[] fTrackHits;
  fTrackHits = 0;
  if(fTracks ) delete[] fTracks;
  fTracks = 0;
  fTrackHits = new Int_t [out.NTrackClusters()];
  fTracks = new AliHLTTPCCAGBTrack[out.NTracks()];  
  fNTracks = 0;

  Int_t nTrackHits = 0;
  
  for( Int_t itr=0; itr<out.NTracks(); itr++ ){
    const AliHLTTPCCAMergedTrack &track = out.Track( itr );

    AliHLTTPCCAGBTrack &trackGB = fTracks[fNTracks];
    trackGB.SetFirstHitRef( nTrackHits );
    trackGB.SetNHits( track.NClusters() );
    trackGB.SetParam( track.InnerParam() );
    trackGB.SetAlpha( track.InnerAlpha() );
    trackGB.SetDeDx( 0 );

    for( Int_t icl=0; icl<track.NClusters(); icl++ ){
      UInt_t  iDsrc = out.ClusterIDsrc( track.FirstClusterRef() + icl );
      UInt_t iSlice = AliHLTTPCCADataCompressor::IDsrc2ISlice( iDsrc );
      UInt_t iRow   = AliHLTTPCCADataCompressor::IDsrc2IRow( iDsrc );
      UInt_t iClu   = AliHLTTPCCADataCompressor::IDsrc2IClu( iDsrc );    
      fTrackHits[nTrackHits+icl] = fFirstSliceHit[iSlice] + fSlices[iSlice].Row(iRow).FirstHit() + iClu;
    }	            
    nTrackHits+= track.NClusters();
    fNTracks++;
  }

#ifdef DRAW
  std::cout<<"Global tracks: "<<std::endl;
  AliHLTTPCCADisplay::Instance().ClearView();
  AliHLTTPCCADisplay::Instance().SetTPCView();
  AliHLTTPCCADisplay::Instance().DrawTPC();
  AliHLTTPCCADisplay::Instance().DrawGBHits( *this );
  for( Int_t itr=0; itr<fNTracks; itr++ ){
    std::cout<<itr<<" nhits= "<<fTracks[itr].NHits()<<std::endl;
    AliHLTTPCCADisplay::Instance().DrawGBTrack( itr, kBlue, 2. );    
    //AliHLTTPCCADisplay::Instance().Ask();
  }
  AliHLTTPCCADisplay::Instance().Ask();
#endif

}



Bool_t AliHLTTPCCAGBTracker::FitTrack( AliHLTTPCCATrackParam &T, AliHLTTPCCATrackParam t0, 
				       Float_t &Alpha, Int_t hits[], Int_t &NTrackHits,
				       Bool_t dir )
{
  // Fit the track
  
  //return fMerger->FitTrack( T, Alpha, t0, Alpha, hits, NTrackHits, dir );

  Float_t alpha0 = Alpha;

  AliHLTTPCCATrackParam::AliHLTTPCCATrackFitParam fitPar;
  AliHLTTPCCATrackParam t = t0;
  AliHLTTPCCATrackLinearisation l(t0);

  Bool_t first = 1;
 
  t.CalculateFitParameters( fitPar );

  Int_t hitsNew[1000];
  Int_t nHitsNew = 0;

  for( Int_t ihit=0; ihit<NTrackHits; ihit++){
    
    Int_t jhit = dir ?(NTrackHits-1-ihit) :ihit;
    AliHLTTPCCAGBHit &h = fHits[hits[jhit]];

    Int_t iSlice = h.ISlice();

    Float_t sliceAlpha =  fSlices[0].Param().Alpha( iSlice );

    if( CAMath::Abs( sliceAlpha - alpha0)>1.e-4 ){
      if( ! t.Rotate(  sliceAlpha - alpha0, l, .999 ) ) continue;
      alpha0 = sliceAlpha;
    }

    //Float_t x = fSliceParam.RowX( h.IRow() );
    Float_t x = h.X();
    
    if( !t.TransportToXWithMaterial( x, l, fitPar, fSlices[0].Param().GetBz(t) ) ) continue;
    
    if( first ){
      t.SetCov( 0, 10 );
      t.SetCov( 1,  0 );
      t.SetCov( 2, 10 );
      t.SetCov( 3,  0 );
      t.SetCov( 4,  0 );
      t.SetCov( 5,  1 );
      t.SetCov( 6,  0 );
      t.SetCov( 7,  0 );
      t.SetCov( 8,  0 );
      t.SetCov( 9,  1 );
      t.SetCov(10,  0 );
      t.SetCov(11,  0 );
      t.SetCov(12,  0 );
      t.SetCov(13,  0 );
      t.SetCov(14,  10 );
      t.SetChi2( 0 );
      t.SetNDF( -5 );
      t.CalculateFitParameters( fitPar );
    }

    
    Float_t err2Y, err2Z;
    fSlices[0].Param().GetClusterErrors2( h.IRow(), h.Z(), l.SinPhi(), l.CosPhi(), l.DzDs(), err2Y, err2Z );
    if( !t.Filter( h.Y(), h.Z(), err2Y, err2Z ) ) continue;	  	    

    first = 0;

    hitsNew[nHitsNew++] = hits[jhit];
  }
  
  if( CAMath::Abs(t.QPt())<1.e-8 ) t.SetQPt( 1.e-8 );
  
  Bool_t ok=1;
  
  const Float_t *c = t.Cov();
  for( Int_t i=0; i<15; i++ ) ok = ok && finite(c[i]);
  for( Int_t i=0; i<5; i++ ) ok = ok && finite(t.Par()[i]);
  ok = ok && (t.GetX()>50);
  
  if( c[0]<=0 || c[2]<=0 || c[5]<=0 || c[9]<=0 || c[14]<=0 ) ok = 0;
  if( c[0]>5. || c[2]>5. || c[5]>2. || c[9]>2 || c[14]>2. ) ok = 0;
  
  if( CAMath::Abs(t.SinPhi())>.99 ) ok = 0;
  else if( l.CosPhi()>=0 ) t.SetSignCosPhi( 1 );
  else t.SetSignCosPhi( -1 );
  
  if( ok ){
    T = t;
    Alpha = alpha0;
    NTrackHits = nHitsNew;
    for( Int_t i=0; i<NTrackHits; i++ ){
      hits[dir ?(NTrackHits-1-i) :i] = hitsNew[i];
    }
  }
  return ok;
}



void AliHLTTPCCAGBTracker::WriteSettings( std::ostream &out ) const
{
  //* write settings to the file
  out<< NSlices()<<std::endl;  
  for( Int_t iSlice=0; iSlice<NSlices(); iSlice++ ){    
    fSlices[iSlice].Param().WriteSettings( out );
  }
}

void AliHLTTPCCAGBTracker::ReadSettings( std::istream &in )
{
  //* Read settings from the file
  Int_t nSlices=0;
  in >> nSlices;
  SetNSlices( nSlices );
  for( Int_t iSlice=0; iSlice<NSlices(); iSlice++ ){    
    AliHLTTPCCAParam param;
    param.ReadSettings ( in );
    fSlices[iSlice].Initialize( param ); 
  }
}

void AliHLTTPCCAGBTracker::WriteEvent( std::ostream &out ) const
{
  // write event to the file

  out<<NHits()<<std::endl;
  for (Int_t ih=0; ih<NHits(); ih++) {
    AliHLTTPCCAGBHit &h = fHits[ih];
    out<<h.X()<<" ";
    out<<h.Y()<<" ";
    out<<h.Z()<<" ";
    out<<h.ErrY()<<" ";
    out<<h.ErrZ()<<" ";
    out<<h.Amp()<<" ";
    out<<h.ID()<<" ";
    out<<h.ISlice()<<" ";
    out<<h.IRow()<<std::endl;
  }
}

void AliHLTTPCCAGBTracker::ReadEvent( std::istream &in ) 
{
  //* Read event from file 

  StartEvent();
  Int_t nHits;
  in >> nHits;
  SetNHits(nHits);
  for (Int_t i=0; i<nHits; i++) {
    Float_t x, y, z, errY, errZ;
    Float_t amp;
    Int_t id, iSlice, iRow;
    in>>x>>y>>z>>errY>>errZ>>amp>>id>>iSlice>>iRow;
    ReadHit( x, y, z, errY, errZ, amp, id, iSlice, iRow );
  }
}

void AliHLTTPCCAGBTracker::WriteTracks( std::ostream &out ) const 
{
  //* Write tracks to file 

  out<<fSliceTrackerTime<<std::endl;
  Int_t nTrackHits = 0;
  for( Int_t itr=0; itr<fNTracks; itr++ ){
    nTrackHits+=fTracks[itr].NHits();
  }
  out<<nTrackHits<<std::endl;
  for( Int_t ih=0; ih<nTrackHits; ih++ ){
    out<< fTrackHits[ih]<<" ";
  }
  out<<std::endl;
  
  out<<NTracks()<<std::endl;
  for( Int_t itr=0; itr<fNTracks; itr++ ){
    AliHLTTPCCAGBTrack &t = fTracks[itr];    
    const AliHLTTPCCATrackParam &p = t.Param();	
    out<< t.NHits()<<" ";
    out<< t.FirstHitRef()<<" ";
    out<< t.Alpha()<<" ";
    out<< t.DeDx()<<std::endl;
    out<< p.GetX()<<" ";
    out<< p.GetCosPhi()<<" ";
    out<< p.GetChi2()<<" ";
    out<< p.GetNDF()<<std::endl;
    for( Int_t i=0; i<5; i++ ) out<<p.GetPar()[i]<<" ";
    out<<std::endl;
    for( Int_t i=0; i<15; i++ ) out<<p.GetCov()[i]<<" ";
    out<<std::endl;
  }
}

void AliHLTTPCCAGBTracker::ReadTracks( std::istream &in )
{
  //* Read tracks  from file 

  in>>fTime;
  fSliceTrackerTime = fTime;
  fStatTime[0]+=fTime;
  fStatNEvents++;
  if( fTrackHits ) delete[] fTrackHits;
  fTrackHits = 0;  
  Int_t nTrackHits = 0;
  in >> nTrackHits;
  fTrackHits = new Int_t [nTrackHits];
  for( Int_t ih=0; ih<nTrackHits; ih++ ){
    in >> TrackHits()[ih];
  }
  if( fTracks ) delete[] fTracks;
  fTracks = 0;  
  in >> fNTracks;
  fTracks = new AliHLTTPCCAGBTrack[fNTracks];
  for( Int_t itr=0; itr<NTracks(); itr++ ){
    AliHLTTPCCAGBTrack &t = Tracks()[itr];    
    AliHLTTPCCATrackParam p;
    Int_t i;
    Float_t f;
    in>>i;
    t.SetNHits(i);
    in>>i;
    t.SetFirstHitRef( i );
    in>>f;
    t.SetAlpha(f);
    in>>f;
    t.SetDeDx( f );
    in>>f;
    p.SetX( f );
    in>>f;
    p.SetSignCosPhi( f );
    in>>f;
    p.SetChi2( f );
    in>>i;
    p.SetNDF( i );
    for( Int_t j=0; j<5; j++ ){ in>>f; p.SetPar( j, f); }
    for( Int_t j=0; j<15; j++ ){ in>>f; p.SetCov(j,f); }
    t.SetParam(p);	
  }
}
