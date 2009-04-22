//-*- Mode: C++ -*-
// ************************************************************************
// This file is property of and copyright by the ALICE HLT Project        *
// ALICE Experiment at CERN, All rights reserved.                         *
// See cxx source for full Copyright notice                               *
//                                                                        *
//*************************************************************************


#ifndef ALIHLTTPCCAMERGEROUTPUT_H
#define ALIHLTTPCCAMERGEROUTPUT_H

#include "AliHLTTPCCADef.h"
#include "AliHLTTPCCAMergedTrack.h"

/**
 * @class AliHLTTPCCAMergerOutput
 *
 * AliHLTTPCCAMergerOutput class is used to store the output of AliHLTTPCCATracker{Component}
 * and transport the output to AliHLTTPCCAMerger{Component}
 *
 * The class contains all the necessary information about TPC tracks, reconstructed in one slice.
 * This includes the reconstructed track parameters and some compressed information
 * about the assigned clusters: clusterId, position and amplitude.
 *
 */
class AliHLTTPCCAMergerOutput
{
  public:

    AliHLTTPCCAMergerOutput()
        : fNTracks( 0 ), fNTrackClusters( 0 ), fTracks( 0 ), fClusterIDsrc( 0 ), fClusterPackedAmp( 0 ) {}

    AliHLTTPCCAMergerOutput( const AliHLTTPCCAMergerOutput & )
        : fNTracks( 0 ), fNTrackClusters( 0 ), fTracks( 0 ), fClusterIDsrc( 0 ), fClusterPackedAmp( 0 ) {}

    const AliHLTTPCCAMergerOutput& operator=( const AliHLTTPCCAMergerOutput &/*v*/ ) const {
      return *this;
    }

    ~AliHLTTPCCAMergerOutput() {}


    GPUhd() int NTracks()                    const { return fNTracks;              }
    GPUhd() int NTrackClusters()             const { return fNTrackClusters;       }

    GPUhd() const AliHLTTPCCAMergedTrack &Track( int i ) const { return fTracks[i]; }
    GPUhd() unsigned int   ClusterIDsrc     ( int i )  const { return fClusterIDsrc[i]; }
    GPUhd()  int   ClusterHltID     ( int i )  const { return fClusterHltID[i]; }
    GPUhd() UChar_t  ClusterPackedAmp( int i )  const { return fClusterPackedAmp[i]; }

    GPUhd() static int EstimateSize( int nOfTracks, int nOfTrackClusters );
    GPUhd() void SetPointers();

    GPUhd() void SetNTracks       ( int v )  { fNTracks = v;        }
    GPUhd() void SetNTrackClusters( int v )  { fNTrackClusters = v; }

    GPUhd() void SetTrack( int i, const AliHLTTPCCAMergedTrack &v ) {  fTracks[i] = v; }
    GPUhd() void SetClusterIDsrc( int i, unsigned int v ) {  fClusterIDsrc[i] = v; }
    GPUhd() void SetClusterHltID( int i,  int v ) {  fClusterHltID[i] = v; }
    GPUhd() void SetClusterPackedAmp( int i, UChar_t v ) {  fClusterPackedAmp[i] = v; }

  private:

    int fNTracks;                 // number of reconstructed tracks
    int fNTrackClusters;          // total number of track clusters
    AliHLTTPCCAMergedTrack *fTracks; // pointer to reconstructed tracks
    unsigned int   *fClusterIDsrc;         // pointer to cluster IDs ( packed IRow and ICluster)
    int   *fClusterHltID;         // pointer to cluster IDs ( packed IRow and ICluster)
    UChar_t  *fClusterPackedAmp;    // pointer to packed cluster amplitudes

};



GPUhd() inline int AliHLTTPCCAMergerOutput::EstimateSize( int nOfTracks, int nOfTrackClusters )
{
  // calculate the amount of memory [bytes] needed for the event

  const int kClusterDataSize = sizeof( unsigned int ) + sizeof( int ) + sizeof( UChar_t );

  return sizeof( AliHLTTPCCAMergerOutput ) + sizeof( AliHLTTPCCAMergedTrack )*nOfTracks + kClusterDataSize*nOfTrackClusters;
}


GPUhd() inline void AliHLTTPCCAMergerOutput::SetPointers()
{
  // set all pointers

  fTracks            = ( AliHLTTPCCAMergedTrack* )( ( &fClusterPackedAmp ) + 1 );
  fClusterIDsrc      = ( unsigned int* )  ( fTracks            + fNTracks );
  fClusterHltID      = ( int* )  ( fClusterIDsrc + fNTrackClusters );
  fClusterPackedAmp  = ( UChar_t* ) ( fClusterHltID + fNTrackClusters );
}

#endif
