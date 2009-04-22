// @(#) $Id$
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


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// a TPC tracker processing component for the HLT based on CA by Ivan Kisel  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#if __GNUC__>= 3
using namespace std;
#endif

#include "AliHLTTPCCATrackerComponent.h"
#include "AliHLTTPCTransform.h"
#include "AliHLTTPCCATracker.h"
#include "AliHLTTPCCAOutTrack.h"
#include "AliHLTTPCCAParam.h"
#include "AliHLTTPCCATrackConvertor.h"
#include "AliHLTArray.h"

#include "AliHLTTPCSpacePointData.h"
#include "AliHLTTPCClusterDataFormat.h"
#include "AliHLTTPCTransform.h"
#include "AliHLTTPCTrackSegmentData.h"
#include "AliHLTTPCTrackArray.h"
#include "AliHLTTPCTrackletDataFormat.h"
#include "AliHLTTPCDefinitions.h"
#include "AliExternalTrackParam.h"
#include "TStopwatch.h"
#include "TMath.h"
#include "AliCDBEntry.h"
#include "AliCDBManager.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "AliHLTTPCCASliceOutput.h"
#include "AliHLTTPCCAClusterData.h"

const AliHLTComponentDataType AliHLTTPCCADefinitions::fgkTrackletsDataType = AliHLTComponentDataTypeInitializer( "CATRACKL", kAliHLTDataOriginTPC );

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp( AliHLTTPCCATrackerComponent )

/** global object for registration
 * Matthias 2009-01-13 temporarily using the global object approach again.
 * CA cade had to be disabled because of various compilation problems, so
 * the global object approach fits better for the moment.
 */

AliHLTTPCCATrackerComponent gAliHLTTPCCATrackerComponent;

AliHLTTPCCATrackerComponent::AliHLTTPCCATrackerComponent()
    :
    fTracker( NULL ),
    fSolenoidBz( 0 ),
    fMinNTrackClusters( 0 ),
    fClusterZCut( 500. ),
    fFullTime( 0 ),
    fRecoTime( 0 ),
    fNEvents( 0 ),
    fNewOutputType( 0 )
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt
}

AliHLTTPCCATrackerComponent::AliHLTTPCCATrackerComponent( const AliHLTTPCCATrackerComponent& )
    :
    AliHLTProcessor(),
    fTracker( NULL ),
    fSolenoidBz( 0 ),
    fMinNTrackClusters( 30 ),
    fClusterZCut( 500. ),
    fFullTime( 0 ),
    fRecoTime( 0 ),
    fNEvents( 0 ),
    fNewOutputType( 0 )
{
  // see header file for class documentation
  HLTFatal( "copy constructor untested" );
}

AliHLTTPCCATrackerComponent& AliHLTTPCCATrackerComponent::operator=( const AliHLTTPCCATrackerComponent& )
{
  // see header file for class documentation
  HLTFatal( "assignment operator untested" );
  return *this;
}

AliHLTTPCCATrackerComponent::~AliHLTTPCCATrackerComponent()
{
  // see header file for class documentation
  delete fTracker;
}

//
// Public functions to implement AliHLTComponent's interface.
// These functions are required for the registration process
//

const char* AliHLTTPCCATrackerComponent::GetComponentID()
{
  // see header file for class documentation
  return "TPCCATracker";
}

void AliHLTTPCCATrackerComponent::GetInputDataTypes( vector<AliHLTComponentDataType>& list )
{
  // see header file for class documentation
  list.clear();
  list.push_back( AliHLTTPCDefinitions::fgkClustersDataType );
}

AliHLTComponentDataType AliHLTTPCCATrackerComponent::GetOutputDataType()
{
  // see header file for class documentation
  if ( fNewOutputType ) return AliHLTTPCCADefinitions::fgkTrackletsDataType;
  else return AliHLTTPCDefinitions::fgkTrackSegmentsDataType;
}

void AliHLTTPCCATrackerComponent::GetOutputDataSize( unsigned long& constBase, double& inputMultiplier )
{
  // define guess for the output data size
  constBase = 200;       // minimum size
  inputMultiplier = 0.5; // size relative to input
}

AliHLTComponent* AliHLTTPCCATrackerComponent::Spawn()
{
  // see header file for class documentation
  return new AliHLTTPCCATrackerComponent;
}

int AliHLTTPCCATrackerComponent::DoInit( int argc, const char** argv )
{
  // Configure the CA tracker component

  fSolenoidBz = 5;
  fMinNTrackClusters = 0;
  fClusterZCut = 500.;
  fFullTime = 0;
  fRecoTime = 0;
  fNEvents = 0;
  fNewOutputType = 0;

  if ( fTracker ) return EINPROGRESS;
  fTracker = new AliHLTTPCCATracker();

  int iResult = 0;

  TString arguments = "";
  for ( int i = 0; i < argc; i++ ) {
    TString argument = argv[i];
    if ( !arguments.IsNull() ) arguments += " ";
    arguments += argument;
  }
  if ( !arguments.IsNull() ) {
    iResult = Configure( arguments.Data() );
  } else {
    iResult = Reconfigure( NULL, NULL );
  }
  return iResult;
}


int AliHLTTPCCATrackerComponent::DoDeinit()
{
  // see header file for class documentation
  if ( fTracker ) delete fTracker;
  fTracker = NULL;
  return 0;
}

int AliHLTTPCCATrackerComponent::Reconfigure( const char* /*cdbEntry*/, const char* /*chainId*/ )
{
  // see header file for class documentation

  HLTWarning( "TODO: dummy Reconfigure() method" );

  return 0;

  /*

  int iResult=0;
  const char* path="HLT/ConfigTPC/CATrackerComponent";
  const char* defaultNotify="";
  if (cdbEntry) {
    path=cdbEntry;
    defaultNotify=" (default)";
  }
  if (path) {
    HLTInfo("reconfigure from entry %s%s, chain id %s", path, defaultNotify,(chainId!=NULL && chainId[0]!=0)?chainId:"<none>");
    AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(path);//,GetRunNo());
    if (pEntry) {
      TObjString* pString=dynamic_cast<TObjString*>(pEntry->GetObject());
      if (pString) {
  HLTInfo("received configuration object string: \'%s\'", pString->GetString().Data());
  iResult=Configure(pString->GetString().Data());
      } else {
  HLTError("configuration object \"%s\" has wrong type, required TObjString", path);
      }
    } else {
      HLTError("cannot fetch object \"%s\" from CDB", path);
    }
  }

  const char* pathBField=kAliHLTCDBSolenoidBz;

  if (pathBField) {
    HLTInfo("reconfigure B-Field from entry %s, chain id %s", path,(chainId!=NULL && chainId[0]!=0)?chainId:"<none>");
    AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(pathBField);//,GetRunNo());
    if (pEntry) {
      TObjString* pString=dynamic_cast<TObjString*>(pEntry->GetObject());
      if (pString) {
  HLTInfo("received configuration object string: \'%s\'", pString->GetString().Data());
  iResult=Configure(pString->GetString().Data());
      } else {
  HLTError("configuration object \"%s\" has wrong type, required TObjString", path);
      }
    } else {
      HLTError("cannot fetch object \"%s\" from CDB", path);
    }
  }
  return iResult;
  */
}


bool AliHLTTPCCATrackerComponent::CompareClusters( AliHLTTPCSpacePointData *a, AliHLTTPCSpacePointData *b )
{
  //* Comparison function for sorting clusters
  if ( a->fPadRow < b->fPadRow ) return 1;
  if ( a->fPadRow > b->fPadRow ) return 0;
  return ( a->fZ < b->fZ );
}


int AliHLTTPCCATrackerComponent::Configure( const char* arguments )
{
  //* Set parameters

  int iResult = 0;
  if ( !arguments ) return iResult;

  TString allArgs = arguments;
  TString argument;
  int bMissingParam = 0;

  TObjArray* pTokens = allArgs.Tokenize( " " );

  int nArgs =  pTokens ? pTokens->GetEntries() : 0;

  for ( int i = 0; i < nArgs; i++ ) {
    argument = ( ( TObjString* )pTokens->At( i ) )->GetString();
    if ( argument.IsNull() ) {
    } else if ( argument.CompareTo( "-solenoidBz" ) == 0 ) {
      if ( ( bMissingParam = ( ++i >= pTokens->GetEntries() ) ) ) break;
      fSolenoidBz = ( ( TObjString* )pTokens->At( i ) )->GetString().Atof();
      HLTInfo( "Magnetic Field set to: %f", fSolenoidBz );
    } else if ( argument.CompareTo( "-minNClustersOnTrack" ) == 0 ||
                argument.CompareTo( "-minNTrackClusters" ) == 0 ) {
      if ( ( bMissingParam = ( ++i >= pTokens->GetEntries() ) ) ) break;
      fMinNTrackClusters = ( ( TObjString* )pTokens->At( i ) )->GetString().Atoi();
      HLTInfo( "minNClustersOnTrack set to: %d", fMinNTrackClusters );
    } else if ( argument.CompareTo( "-clusterZCut" ) == 0 ) {
      if ( ( bMissingParam = ( ++i >= pTokens->GetEntries() ) ) ) break;
      fClusterZCut = TMath::Abs( ( ( TObjString* )pTokens->At( i ) )->GetString().Atof() );
      HLTInfo( "ClusterZCut set to: %f", fClusterZCut );
    } else if ( argument.CompareTo( "-newOutputType" ) == 0 ) {
      fNewOutputType = 1;
      HLTInfo( "NewOutputType is set" );
    } else {
      HLTError( "Unknown option %s ", argument.Data() );
      iResult = -EINVAL;
    }
  }
  delete pTokens;

  if ( bMissingParam ) {
    HLTError( "Specifier missed for %s", argument.Data() );
    iResult = -EINVAL;
  }

  return iResult;
}




int AliHLTTPCCATrackerComponent::DoEvent
(
  const AliHLTComponentEventData& evtData,
  const AliHLTComponentBlockData* blocks,
  AliHLTComponentTriggerData& /*trigData*/,
  AliHLTUInt8_t* outputPtr,
  AliHLTUInt32_t& size,
  vector<AliHLTComponentBlockData>& outputBlocks )
{
//* process event
  AliHLTUInt32_t maxBufferSize = size;
  size = 0; // output size

  if ( GetFirstInputBlock( kAliHLTDataTypeSOR ) || GetFirstInputBlock( kAliHLTDataTypeEOR ) ) {
    return 0;
  }

  TStopwatch timer;

  // Event reconstruction in one TPC slice with CA Tracker

  //Logging( kHLTLogWarning, "HLT::TPCCATracker::DoEvent", "DoEvent", "CA::DoEvent()" );
  if ( evtData.fBlockCnt <= 0 ) {
    HLTWarning( "no blocks in event" );
    return 0;
  }

  const AliHLTComponentBlockData* iter = NULL;
  unsigned long ndx;
  AliHLTTPCClusterData* inPtrSP;

  // Determine the slice number

  int slice = -1;
  {
    std::vector<int> slices;
    std::vector<int>::iterator slIter;
    std::vector<unsigned> sliceCnts;
    std::vector<unsigned>::iterator slCntIter;

    for ( ndx = 0; ndx < evtData.fBlockCnt; ndx++ ) {
      iter = blocks + ndx;
      if ( iter->fDataType != AliHLTTPCDefinitions::fgkClustersDataType ) continue;

      slice = AliHLTTPCDefinitions::GetMinSliceNr( *iter );

      bool found = 0;
      slCntIter = sliceCnts.begin();
      for ( slIter = slices.begin(); slIter != slices.end(); slIter++, slCntIter++ ) {
        if ( *slIter == slice ) {
          found = kTRUE;
          break;
        }
      }
      if ( !found ) {
        slices.push_back( slice );
        sliceCnts.push_back( 1 );
      } else *slCntIter++;
    }


    // Determine slice number to really use.
    if ( slices.size() > 1 ) {
      Logging( kHLTLogError, "HLT::TPCSliceTracker::DoEvent", "Multiple slices found in event",
               "Multiple slice numbers found in event 0x%08lX (%lu). Determining maximum occuring slice number...",
               evtData.fEventID, evtData.fEventID );
      unsigned maxCntSlice = 0;
      slCntIter = sliceCnts.begin();
      for ( slIter = slices.begin(); slIter != slices.end(); slIter++, slCntIter++ ) {
        Logging( kHLTLogError, "HLT::TPCSliceTracker::DoEvent", "Multiple slices found in event",
                 "Slice %lu found %lu times.", *slIter, *slCntIter );
        if ( maxCntSlice < *slCntIter ) {
          maxCntSlice = *slCntIter;
          slice = *slIter;
        }
      }
      Logging( kHLTLogError, "HLT::TPCSliceTracker::DoEvent", "Multiple slices found in event",
               "Using slice %lu.", slice );
    } else if ( slices.size() > 0 ) {
      slice = *( slices.begin() );
    }
  }

  if ( slice < 0 ) {
    HLTWarning( "no slices found in event" );
    return 0;
  }


  // Initialize the tracker


  {
    if ( !fTracker ) fTracker = new AliHLTTPCCATracker;
    int iSec = slice;
    float inRmin = 83.65;
    //    float inRmax = 133.3;
    //    float outRmin = 133.5;
    float outRmax = 247.7;
    float plusZmin = 0.0529937;
    float plusZmax = 249.778;
    float minusZmin = -249.645;
    float minusZmax = -0.0799937;
    float dalpha = 0.349066;
    float alpha = 0.174533 + dalpha * iSec;

    bool zPlus = ( iSec < 18 );
    float zMin =  zPlus ? plusZmin : minusZmin;
    float zMax =  zPlus ? plusZmax : minusZmax;
    //TPCZmin = -249.645, ZMax = 249.778
    //    float rMin =  inRmin;
    //    float rMax =  outRmax;
    int nRows = AliHLTTPCTransform::GetNRows();

    float padPitch = 0.4;
    float sigmaZ = 0.228808;

    float *rowX = new float [nRows];
    for ( int irow = 0; irow < nRows; irow++ ) {
      rowX[irow] = AliHLTTPCTransform::Row2X( irow );
    }

    const double kCLight = 0.000299792458;

    AliHLTTPCCAParam param;

    param.Initialize( iSec, nRows, rowX, alpha, dalpha,
                      inRmin, outRmax, zMin, zMax, padPitch, sigmaZ, fSolenoidBz*kCLight );
    param.SetHitPickUpFactor( 2 );
    param.Update();
    fTracker->Initialize( param );
    delete[] rowX;
  }


  // min and max patch numbers and row numbers

  int row[2] = {0, 0};
  int minPatch = 100, maxPatch = -1;

  // total n Hits

  int nHitsTotal = 0;

  // sort patches

  std::vector<unsigned long> patchIndices;

  for ( ndx = 0; ndx < evtData.fBlockCnt; ndx++ ) {
    iter = blocks + ndx;
    if ( iter->fDataType != AliHLTTPCDefinitions::fgkClustersDataType ) continue;
    if ( slice != AliHLTTPCDefinitions::GetMinSliceNr( *iter ) ) continue;
    inPtrSP = ( AliHLTTPCClusterData* )( iter->fPtr );
    nHitsTotal += inPtrSP->fSpacePointCnt;
    int patch = AliHLTTPCDefinitions::GetMinPatchNr( *iter );
    if ( minPatch > patch ) {
      minPatch = patch;
      row[0] = AliHLTTPCTransform::GetFirstRow( patch );
    }
    if ( maxPatch < patch ) {
      maxPatch = patch;
      row[1] = AliHLTTPCTransform::GetLastRow( patch );
    }
    std::vector<unsigned long>::iterator pIter = patchIndices.begin();
    while ( pIter != patchIndices.end() && AliHLTTPCDefinitions::GetMinPatchNr( blocks[*pIter] ) < patch ) {
      pIter++;
    }
    patchIndices.insert( pIter, ndx );
  }


  // pass event to CA Tracker

  fTracker->StartEvent();

  Logging( kHLTLogDebug, "HLT::TPCCATracker::DoEvent", "Reading hits",
           "Total %d hits to read for slice %d", nHitsTotal, slice );


  AliHLTResizableArray<AliHLTTPCSpacePointData *> vOrigClusters( nHitsTotal );

  int nClusters = 0;

  for ( std::vector<unsigned long>::iterator pIter = patchIndices.begin(); pIter != patchIndices.end(); pIter++ ) {
    ndx = *pIter;
    iter = blocks + ndx;

    int patch = AliHLTTPCDefinitions::GetMinPatchNr( *iter );
    inPtrSP = ( AliHLTTPCClusterData* )( iter->fPtr );

    Logging( kHLTLogDebug, "HLT::TPCCATracker::DoEvent", "Reading hits",
             "Reading %d hits for slice %d - patch %d", inPtrSP->fSpacePointCnt, slice, patch );

    for ( unsigned int i = 0; i < inPtrSP->fSpacePointCnt; i++ ) {
      vOrigClusters[nClusters++] = &( inPtrSP->fSpacePoints[i] );
    }
  }

  // sort clusters since they are not sorted for some reason

  // candidate for parallelization should this become an issue, can use tbb::parallel_sort
  sort( vOrigClusters.Data(), ( vOrigClusters + nClusters ).Data(), CompareClusters );

  ClusterData clusterData( vOrigClusters, nClusters, fClusterZCut );
  fTracker->ReadEvent( &clusterData );

  // reconstruct the event

  TStopwatch timerReco;

  fTracker->Reconstruct();

  timerReco.Stop();

  int ret = 0;

  Logging( kHLTLogDebug, "HLT::TPCCATracker::DoEvent", "Reconstruct",
           "%d tracks found for slice %d", fTracker->NOutTracks(), slice );


  // write reconstructed tracks

  unsigned int mySize = 0;
  int ntracks = *fTracker->NOutTracks();


  if ( !fNewOutputType ) {

    AliHLTTPCTrackletData* outPtr = ( AliHLTTPCTrackletData* )( outputPtr );

    AliHLTTPCTrackSegmentData* currOutTracklet = outPtr->fTracklets;

    mySize =   ( ( AliHLTUInt8_t * )currOutTracklet ) -  ( ( AliHLTUInt8_t * )outputPtr );

    outPtr->fTrackletCnt = 0;

    for ( int itr = 0; itr < ntracks; itr++ ) {

      AliHLTTPCCAOutTrack &t = fTracker->OutTracks()[itr];

      //Logging( kHLTLogDebug, "HLT::TPCCATracker::DoEvent", "Wrtite output","track %d with %d hits", itr, t.NHits());

      if ( t.NHits() < fMinNTrackClusters ) continue;

      // calculate output track size

      unsigned int dSize = sizeof( AliHLTTPCTrackSegmentData ) + t.NHits() * sizeof( unsigned int );

      if ( mySize + dSize > maxBufferSize ) {
        HLTWarning( "Output buffer size exceed (buffer size %d, current size %d), %d tracks are not stored", maxBufferSize, mySize, ntracks - itr + 1 );
        ret = -ENOSPC;
        break;
      }

      // convert CA track parameters to HLT Track Segment

      int iFirstRow = 1000;
      int iLastRow = -1;
      int iFirstHit = fTracker->OutTrackHits()[t.FirstHitRef()];
      int iLastHit = iFirstHit;
      for ( int ih = 0; ih < t.NHits(); ih++ ) {
        int hitID = fTracker->OutTrackHits()[t.FirstHitRef() + ih ];
        int iRow = clusterData.RowNumber( hitID );
        if ( iRow < iFirstRow ) {  iFirstRow = iRow; iFirstHit = hitID; }
        if ( iRow > iLastRow ) { iLastRow = iRow; iLastHit = hitID; }
      }

      AliHLTTPCCATrackParam par = t.StartPoint();

      par.TransportToX( clusterData.X( iFirstHit ), .99 );

      AliExternalTrackParam tp;
      AliHLTTPCCATrackConvertor::GetExtParam( par, tp, 0 );

      currOutTracklet->fX = tp.GetX();
      currOutTracklet->fY = tp.GetY();
      currOutTracklet->fZ = tp.GetZ();
      currOutTracklet->fCharge = ( int ) tp.GetSign();
      currOutTracklet->fPt = TMath::Abs( tp.GetSignedPt() );
      float snp =  tp.GetSnp() ;
      if ( snp > .999 ) snp = .999;
      if ( snp < -.999 ) snp = -.999;
      currOutTracklet->fPsi = TMath::ASin( snp );
      currOutTracklet->fTgl = tp.GetTgl();

      currOutTracklet->fY0err = tp.GetSigmaY2();
      currOutTracklet->fZ0err = tp.GetSigmaZ2();
      float h = -currOutTracklet->fPt * currOutTracklet->fPt;
      currOutTracklet->fPterr = h * h * tp.GetSigma1Pt2();
      h = 1. / TMath::Sqrt( 1 - snp * snp );
      currOutTracklet->fPsierr = h * h * tp.GetSigmaSnp2();
      currOutTracklet->fTglerr = tp.GetSigmaTgl2();

      if ( par.TransportToX( clusterData.X( iLastHit ), .99 ) ) {
        currOutTracklet->fLastX = par.GetX();
        currOutTracklet->fLastY = par.GetY();
        currOutTracklet->fLastZ = par.GetZ();
      } else {
        currOutTracklet->fLastX = clusterData.X( iLastHit );
        currOutTracklet->fLastY = clusterData.Y( iLastHit );
        currOutTracklet->fLastZ = clusterData.Z( iLastHit );
      }
      //if( currOutTracklet->fLastX<10. ) {
      //HLTError("CA last point: hitxyz=%f,%f,%f, track=%f,%f,%f, tracklet=%f,%f,%f, nhits=%d",clusterData.X( iLastHit ),clusterData.Y( iLastHit],clusterData.Z( iLastHit],
      //par.GetX(), par.GetY(),par.GetZ(),currOutTracklet->fLastX,currOutTracklet->fLastY ,currOutTracklet->fLastZ, t.NHits());
      //}
#ifdef INCLUDE_TPC_HOUGH
#ifdef ROWHOUGHPARAMS
      currOutTracklet->fTrackID = 0;
      currOutTracklet->fRowRange1 = clusterData.RowNumber( iFirstHit );
      currOutTracklet->fRowRange2 = clusterData.RowNumber( iLastHit );
      currOutTracklet->fSector = slice;
      currOutTracklet->fPID = 211;
#endif
#endif // INCLUDE_TPC_HOUGH


      currOutTracklet->fNPoints = t.NHits();

      for ( int i = 0; i < t.NHits(); i++ ) {
        currOutTracklet->fPointIDs[i] = clusterData.Id( fTracker->OutTrackHits()[t.FirstHitRef()+i] );
      }

      currOutTracklet = ( AliHLTTPCTrackSegmentData* )( ( Byte_t * )currOutTracklet + dSize );
      mySize += dSize;
      outPtr->fTrackletCnt++;
    }
  } else { // new output type

    mySize = fTracker->Output()->EstimateSize( fTracker->Output()->NTracks(),
             fTracker->Output()->NTrackClusters() );
    if ( mySize <= maxBufferSize ) {
      const AliHLTUInt8_t* outputevent = reinterpret_cast<const AliHLTUInt8_t*>( fTracker->Output() );
      for ( unsigned int i = 0; i < mySize; i++ ) outputPtr[i] = outputevent[i];
    } else {
      HLTWarning( "Output buffer size exceed (buffer size %d, current size %d), tracks are not stored", maxBufferSize, mySize );
      mySize = 0;
      ret = -ENOSPC;
    }
  }

  if ( mySize > 0 ) {
    AliHLTComponentBlockData bd;
    FillBlockData( bd );
    bd.fOffset = 0;
    bd.fSize = mySize;
    bd.fSpecification = AliHLTTPCDefinitions::EncodeDataSpecification( slice, slice, minPatch, maxPatch );
    outputBlocks.push_back( bd );
  }
  size = mySize;

  timer.Stop();

  fFullTime += timer.RealTime();
  fRecoTime += timerReco.RealTime();
  fNEvents++;

  // Set log level to "Warning" for on-line system monitoring
  int hz = ( int ) ( fFullTime > 1.e-10 ? fNEvents / fFullTime : 100000 );
  int hz1 = ( int ) ( fRecoTime > 1.e-10 ? fNEvents / fRecoTime : 100000 );
  HLTWarning( "CATracker slice %d: output %d tracks;  input %d clusters, patches %d..%d, rows %d..%d; reco time %d/%d Hz",
              slice, ntracks, nClusters, minPatch, maxPatch, row[0], row[1], hz, hz1 );

  return ret;
}


