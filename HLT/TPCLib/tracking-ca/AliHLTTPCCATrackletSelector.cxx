// @(#) $Id: AliHLTTPCCATrackletSelector.cxx 27042 2008-07-02 12:06:02Z richterm $
//***************************************************************************
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
//***************************************************************************

#include "AliHLTTPCCATrackletSelector.h"
#include "AliHLTTPCCATrack.h"
#include "AliHLTTPCCATracker.h"
#include "AliHLTTPCCATrackParam.h"
#include "AliHLTTPCCAMath.h"

GPUd() void AliHLTTPCCATrackletSelector::Thread
( Int_t nBlocks, Int_t nThreads, Int_t iBlock, Int_t iThread, Int_t iSync,
  AliHLTTPCCASharedMemory &s, AliHLTTPCCATracker &tracker )
{
  // select best tracklets and kill clones

  if( iSync==0 )
    {
      if( iThread==0 ){
	if(iBlock==0){
	  CAMath::atomicExch(tracker.NTracks(),0);
	  CAMath::atomicExch(tracker.NTrackHits(),0);
	}
	s.fNTracklets = *tracker.NTracklets();
	s.fNThreadsTotal = nThreads*nBlocks;
	s.fItr0 = nThreads*iBlock;	
      }
    }
  else if( iSync==1 )
    {
      AliHLTTPCCATrack tout;
      Int_t trackHits[160];
	
      for( Int_t itr= s.fItr0 + iThread; itr<s.fNTracklets; itr+=s.fNThreadsTotal ){    		
	Int_t *t = ((Int_t*)tracker.Tracklets()) + itr*(5+ sizeof(AliHLTTPCCATrackParam)/4 + 160 );	
	Int_t tNHits = *t;
	if( tNHits<=0 ) continue;

	const Int_t kMaxRowGap = 4;
	Int_t firstRow = t[3];
	Int_t lastRow = t[4];

	tout.SetNHits( 0 );
	Int_t *hitstore = t + 5+ sizeof(AliHLTTPCCATrackParam)/4 ;    
	Int_t w = (tNHits<<16)+itr;	
	//Int_t nRows = tracker.Param().NRows();
	Int_t gap = 0;

	//std::cout<<" store tracklet: "<<firstRow<<" "<<lastRow<<std::endl;
 	for( Int_t irow=firstRow; irow<=lastRow; irow++ ){
	  gap++;
	  Int_t ih = hitstore[irow];
	  if( ih>=0 ){
	    Int_t ihTot = tracker.Row(irow).FirstHit()+ih;
	    if( tracker.HitWeights()[ihTot] <= w ){
	      gap = 0;
	      Int_t th = AliHLTTPCCATracker::IRowIHit2ID(irow,ih);
	      trackHits[tout.NHits()] = th;
	      tout.SetNHits( tout.NHits() + 1 );
	    }
	  }
	  if( gap>kMaxRowGap || irow==lastRow ){ // store 
	    if( tout.NHits()>=10 ){ //SG!!!
	      Int_t itrout = CAMath::atomicAdd(tracker.NTracks(),1);
	      tout.SetFirstHitID( CAMath::atomicAdd( tracker.NTrackHits(), tout.NHits() ));
	      tout.SetParam( *( (AliHLTTPCCATrackParam*)( t+5) ));
	      tout.SetAlive( 1 );
	      tracker.Tracks()[itrout] = tout;
	      for( Int_t jh=0; jh<tout.NHits(); jh++ ){
		tracker.TrackHits()[tout.FirstHitID() + jh] = trackHits[jh];
	      }
	    }
	    tout.SetNHits( 0 ); 
	    gap = 0;
	  }
	}
      }
    }
}
