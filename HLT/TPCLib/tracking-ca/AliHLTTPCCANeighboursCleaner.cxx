// @(#) $Id: AliHLTTPCCANeighboursCleaner.cxx 27042 2008-07-02 12:06:02Z richterm $
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


#include "AliHLTTPCCANeighboursCleaner.h"
#include "AliHLTTPCCAMath.h"
#include "AliHLTTPCCATracker.h"

GPUd() void AliHLTTPCCANeighboursCleaner::Thread
( int /*nBlocks*/, int nThreads, int iBlock, int iThread, int iSync,
  AliHLTTPCCASharedMemory &s, AliHLTTPCCATracker &tracker )
{
  // *
  // * kill link to the neighbour if the neighbour is not pointed to the cluster
  // *

  if ( iSync == 0 ) {
    if ( iThread == 0 ) {
      s.fNRows = tracker.Param().NRows();
      s.fIRow = iBlock + 2;
      if ( s.fIRow <= s.fNRows - 3 ) {
        s.fIRowUp = s.fIRow + 2;
        s.fIRowDn = s.fIRow - 2;
        s.fFirstHit = tracker.Row( s.fIRow ).FirstHit();
        const AliHLTTPCCARow &row = tracker.Row( s.fIRow );
        const AliHLTTPCCARow &rowUp = tracker.Row( s.fIRowUp );
        const AliHLTTPCCARow &rowDn = tracker.Row( s.fIRowDn );
        s.fHitLinkUp = ( ( short* )( tracker.RowData() + row.FullOffset() ) ) + row.FullLinkOffset();
        s.fHitLinkDn = s.fHitLinkUp + row.NHits();
        s.fDnHitLinkUp = ( ( short* )( tracker.RowData() + rowDn.FullOffset() ) ) + rowDn.FullLinkOffset();
        s.fUpHitLinkDn = ( ( short* )( tracker.RowData() + rowUp.FullOffset() ) ) + rowUp.FullLinkOffset() + rowUp.NHits();

        s.fNHits = tracker.Row( s.fIRow ).NHits();
      }
    }
  } else if ( iSync == 1 ) {
    if ( s.fIRow <= s.fNRows - 3 ) {

      for ( int ih = iThread; ih < s.fNHits; ih += nThreads ) {
        int up = s.fHitLinkUp[ih];
        if ( up >= 0 ) {
          short upDn = s.fUpHitLinkDn[up];
          if ( ( upDn != ih ) ) s.fHitLinkUp[ih] = -1;
        }
        int dn = s.fHitLinkDn[ih];
        if ( dn >= 0 ) {
          short dnUp = s.fDnHitLinkUp[dn];
          if ( dnUp != ih ) s.fHitLinkDn[ih] = -1;
        }
      }
    }
  }
}

