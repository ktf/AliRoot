// $Id$

/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Authors: Matthias Richter <Matthias.Richter@ift.uib.no>                *
 *          Timm Steinbeck <timm@kip.uni-heidelberg.de>                   *
 *          Jochen Thaeder <thaeder@kip.uni-heidelberg.de>                *
 *          for The ALICE Off-line Project.                               *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// a TPC cluster finder processing component for the HLT                     //
// useable for packed data or unpacked data                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#if __GNUC__== 3
using namespace std;
#endif

#include "AliHLTTPCClusterFinderComponent.h"
#include "AliHLTTPCDigitReaderPacked.h"
#include "AliHLTTPCDigitReaderUnpacked.h"
#include "AliHLTTPCClusterFinder.h"
#include "AliHLTTPCSpacePointData.h"
#include "AliHLTTPCRawDataFormat.h"
#include "AliHLTTPCClusterDataFormat.h"
#include "AliHLTTPCTransform.h"
#include <stdlib.h>
#include <errno.h>

// this is a global object used for automatic component registration, do not use this
AliHLTTPCClusterFinderComponent gAliHLTTPCClusterFinderComponentPacked(true);
AliHLTTPCClusterFinderComponent gAliHLTTPCClusterFinderComponentUnpacked(false);

ClassImp(AliHLTTPCClusterFinderComponent)

AliHLTTPCClusterFinderComponent::AliHLTTPCClusterFinderComponent(bool packed)
  :
  // use fPackedSwitch = true for packed inputtype "gkDDLPackedRawDataType"
  // use fPackedSwitch = false for unpacked inputtype "gkUnpackedRawDataType"
  fPackedSwitch(packed),

  fClusterFinder(NULL),
  fReader(NULL),
  fClusterDeconv(true),
  fXYClusterError(-1),
  fZClusterError(-1)
{
}

AliHLTTPCClusterFinderComponent::AliHLTTPCClusterFinderComponent(const AliHLTTPCClusterFinderComponent&)
  :
  fPackedSwitch(0),
  fClusterFinder(NULL),
  fReader(NULL),
  fClusterDeconv(true),
  fXYClusterError(-1),
  fZClusterError(-1)
{
  HLTFatal("copy constructor untested");
}

AliHLTTPCClusterFinderComponent& AliHLTTPCClusterFinderComponent::operator=(const AliHLTTPCClusterFinderComponent&)
{ 
  HLTFatal("assignment operator untested");
  return *this;
}

AliHLTTPCClusterFinderComponent::~AliHLTTPCClusterFinderComponent()
    {
    }

// Public functions to implement AliHLTComponent's interface.
// These functions are required for the registration process

const char* AliHLTTPCClusterFinderComponent::GetComponentID()
    {
      if (fPackedSwitch) return "TPCClusterFinderPacked";
      else return "TPCClusterFinderUnpacked";
    }

void AliHLTTPCClusterFinderComponent::GetInputDataTypes( vector<AliHLTComponent_DataType>& list)
    {
    list.clear(); 
    if (fPackedSwitch) list.push_back( AliHLTTPCDefinitions::gkDDLPackedRawDataType );
    else list.push_back( AliHLTTPCDefinitions::gkUnpackedRawDataType );
   
    }

AliHLTComponent_DataType AliHLTTPCClusterFinderComponent::GetOutputDataType()
    {
    return AliHLTTPCDefinitions::gkClustersDataType;
    }

void AliHLTTPCClusterFinderComponent::GetOutputDataSize( unsigned long& constBase, double& inputMultiplier )
    {
    // XXX TODO: Find more realistic values.  
    constBase = 0;
    if (fPackedSwitch)  inputMultiplier = (6 * 0.4);
    else  inputMultiplier = 0.4;
    }

AliHLTComponent* AliHLTTPCClusterFinderComponent::Spawn()
    {
    return new AliHLTTPCClusterFinderComponent(fPackedSwitch);
    }
	
int AliHLTTPCClusterFinderComponent::DoInit( int argc, const char** argv )
    {
    if ( fClusterFinder )
	return EINPROGRESS;

    fClusterFinder = new AliHLTTPCClusterFinder();

    if (fPackedSwitch) {
#if defined(HAVE_ALIRAWDATA) && defined(HAVE_ALITPCRAWSTREAM_H)
      fReader = new AliHLTTPCDigitReaderPacked();
      fClusterFinder->SetReader(fReader);
#else // ! defined(HAVE_ALIRAWDATA) && defined(HAVE_ALITPCRAWSTREAM_H)
      HLTFatal("DigitReaderPacked not available - check your build");
      return -ENODEV;
#endif //  defined(HAVE_ALIRAWDATA) && defined(HAVE_ALITPCRAWSTREAM_H)
    }
    else {
      fReader = new AliHLTTPCDigitReaderUnpacked();
      fClusterFinder->SetReader(fReader);
    }
    
    // Variables to setup the Clusterfinder
    fClusterDeconv = true;
    fXYClusterError = -1;
    fZClusterError = -1;

    int i = 0;
    while ( i < argc )
	{
	  if ( !strcmp( argv[i], "pp-run" ) ){
	    fClusterDeconv = false;
	    i++;
	    continue;
	  }

	Logging(kHLTLogError, "HLT::TPCClusterFinder::DoInit", "Unknown Option", "Unknown option '%s'", argv[i] );
	return EINVAL;
	}


    fClusterFinder->SetDeconv( fClusterDeconv );
    fClusterFinder->SetXYError( fXYClusterError );
    fClusterFinder->SetZError( fZClusterError );
    if ( (fXYClusterError>0) && (fZClusterError>0) )
      fClusterFinder->SetCalcErr( false );
    
    return 0;
    }

int AliHLTTPCClusterFinderComponent::DoDeinit()
    {

    if ( fClusterFinder )
	delete fClusterFinder;
    fClusterFinder = NULL;
 
    if ( fReader )
	delete fReader;
    fReader = NULL;
    
    return 0;
    }

int AliHLTTPCClusterFinderComponent::DoEvent( const AliHLTComponent_EventData& evtData, 
					      const AliHLTComponent_BlockData* blocks, 
					      AliHLTComponent_TriggerData& trigData, AliHLTUInt8_t* outputPtr, 
					      AliHLTUInt32_t& size, 
					      vector<AliHLTComponent_BlockData>& outputBlocks )
    {
    //  == init iter (pointer to datablock)
    const AliHLTComponent_BlockData* iter = NULL;
    unsigned long ndx;

    //  == OUTdatatype pointer
    AliHLTTPCClusterData* outPtr;

    AliHLTUInt8_t* outBPtr;
    UInt_t offset, mysize, nSize, tSize = 0;

    outBPtr = outputPtr;
    outPtr = (AliHLTTPCClusterData*)outBPtr;

    Int_t slice, patch, row[2];
    unsigned long maxPoints, realPoints = 0;

    for ( ndx = 0; ndx < evtData.fBlockCnt; ndx++ )
	{
	iter = blocks+ndx;
	mysize = 0;
	offset = tSize;


	if (fPackedSwitch) {	
	  char tmp1[14], tmp2[14];
	  DataType2Text( iter->fDataType, tmp1 );
	  DataType2Text( AliHLTTPCDefinitions::gkDDLPackedRawDataType, tmp2 );
	  Logging( kHLTLogDebug, "HLT::TPCClusterFinder::DoEvent", "Event received", 
		   "Event 0x%08LX (%Lu) received datatype: %s - required datatype: %s",
		   evtData.fEventID, evtData.fEventID, tmp1, tmp2 );

	  if ( iter->fDataType != AliHLTTPCDefinitions::gkDDLPackedRawDataType ) continue;

	}
	else {
	  char tmp1[14], tmp2[14];
	  DataType2Text( iter->fDataType, tmp1 );
	  DataType2Text( AliHLTTPCDefinitions::gkUnpackedRawDataType, tmp2 );
	  Logging( kHLTLogDebug, "HLT::TPCClusterFinder::DoEvent", "Event received", 
		   "Event 0x%08LX (%Lu) received datatype: %s - required datatype: %s",
		   evtData.fEventID, evtData.fEventID, tmp1, tmp2 );

	  if ( iter->fDataType != AliHLTTPCDefinitions::gkUnpackedRawDataType ) continue;

	}
    	
	slice = AliHLTTPCDefinitions::GetMinSliceNr( *iter );
	patch = AliHLTTPCDefinitions::GetMinPatchNr( *iter );
	row[0] = AliHLTTPCTransform::GetFirstRow( patch );
	row[1] = AliHLTTPCTransform::GetLastRow( patch );
	
	Logging( kHLTLogDebug, "HLT::TPCClusterFinder::DoEvent", "Input Spacepoints", 
		 "Input: Number of spacepoints: %lu Slice/Patch/RowMin/RowMax: %d/%d/%d/%d.",
		 realPoints, slice, patch, row[0], row[1] );
	
	outPtr = (AliHLTTPCClusterData*)outBPtr;

	maxPoints = (size-tSize-sizeof(AliHLTTPCClusterData))/sizeof(AliHLTTPCSpacePointData);
	
	fClusterFinder->InitSlice( slice, patch, row[0], row[1], maxPoints );
	fClusterFinder->SetOutputArray( outPtr->fSpacePoints );
	fClusterFinder->Read(iter->fPtr, iter->fSize );
	fClusterFinder->ProcessDigits();
	realPoints = fClusterFinder->GetNumberOfClusters();
	
	Logging( kHLTLogDebug, "HLT::TPCClusterFinder::DoEvent", "Spacepoints", 
		 "Number of spacepoints found: %lu.", realPoints );
	
	outPtr->fSpacePointCnt = realPoints;
	nSize = sizeof(AliHLTTPCSpacePointData)*realPoints;
	mysize += nSize+sizeof(AliHLTTPCClusterData);
	
	Logging( kHLTLogDebug, "HLT::TPCClusterFinder::DoEvent", "Input Spacepoints", 
		 "Number of spacepoints: %lu Slice/Patch/RowMin/RowMax: %d/%d/%d/%d.",
		 realPoints, slice, patch, row[0], row[1] );
	
	
	AliHLTComponent_BlockData bd;
	FillBlockData( bd );
	bd.fOffset = offset;
	bd.fSize = mysize;
	bd.fSpecification = iter->fSpecification;
	//AliHLTSubEventDescriptor::FillBlockAttributes( bd.fAttributes );
	outputBlocks.push_back( bd );
	
	tSize += mysize;
	outBPtr += mysize;
	outPtr = (AliHLTTPCClusterData*)outBPtr;
	
	if ( tSize > size )
	    {
	    Logging( kHLTLogFatal, "HLT::TPCClusterFinder::DoEvent", "Too much data", 
		     "Data written over allowed buffer. Amount written: %lu, allowed amount: %lu.",
		     tSize, size );
	    return EMSGSIZE;
	    }
	}
    
    size = tSize;
    return 0;
    }

   
