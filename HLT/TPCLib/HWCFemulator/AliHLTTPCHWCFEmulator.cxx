//****************************************************************************
//* This file is property of and copyright by the ALICE HLT Project          * 
//* ALICE Experiment at CERN, All rights reserved.                           *
//*                                                                          *
//* Primary Authors: Sergey Gorbunov, Torsten Alt                            *
//* Developers:      Sergey Gorbunov <sergey.gorbunov@fias.uni-frankfurt.de> *
//*                  Torsten Alt <talt@cern.ch>                              *
//*                  for The ALICE HLT Project.                              *
//*                                                                          *
//* Permission to use, copy, modify and distribute this software and its     *
//* documentation strictly for non-commercial purposes is hereby granted     *
//* without fee, provided that the above copyright notice appears in all     *
//* copies and that both the copyright notice and this permission notice     *
//* appear in the supporting documentation. The authors make no claims       *
//* about the suitability of this software for any purpose. It is            *
//* provided "as is" without express or implied warranty.                    *
//****************************************************************************

//  @file   AliHLTTPCHWCFEmulator.cxx
//  @author Sergey Gorbunov <sergey.gorbunov@fias.uni-frankfurt.de>
//  @author Torsten Alt <talt@cern.ch> 
//  @brief  FPGA ClusterFinder Emulator for TPC
//  @note


#include "AliHLTTPCHWCFDataTypes.h"
#include "AliHLTTPCHWCFEmulator.h"
#include "AliHLTTPCClusterMCData.h"

#include <iostream>

#if __GNUC__ >= 3
using namespace std;
#endif


AliHLTTPCHWCFEmulator::AliHLTTPCHWCFEmulator()
  :
  fkMapping(0),
  fChannelExtractor(),
  fChannelProcessor(),
  fChannelMerger(),
  fDivisionUnit()
{
  //constructor 
}

AliHLTTPCHWCFEmulator::~AliHLTTPCHWCFEmulator()
{   
  //destructor
}

AliHLTTPCHWCFEmulator::AliHLTTPCHWCFEmulator(const AliHLTTPCHWCFEmulator&)
  :
  fkMapping(0),
  fChannelExtractor(),
  fChannelProcessor(),
  fChannelMerger(),
  fDivisionUnit()
{
  // dummy
}

AliHLTTPCHWCFEmulator& AliHLTTPCHWCFEmulator::operator=(const AliHLTTPCHWCFEmulator&)
{
  // dummy
  return *this;
}

void AliHLTTPCHWCFEmulator::Init( const AliHLTUInt32_t *mapping, AliHLTUInt32_t config )
{
  // Initialisation
  fkMapping = mapping;
  
  fChannelProcessor.SetDeconvolution( config & 0x1 );
  fChannelMerger.SetDeconvolution( (config>>1) & 0x1 );
  fDivisionUnit.SetSinglePadSuppression( (config>>3) & 0x1 );
  fChannelMerger.SetByPassMerger( (config>>4) & 0x1 );
  fDivisionUnit.SetClusterLowerLimit( (config>>8) & 0xFF );
  fChannelProcessor.SetSingleSeqLimit( (config>>16) & 0xFF );

  //fChannelMerger.SetDebugLevel(1);
}

 
int AliHLTTPCHWCFEmulator::FindClusters( const AliHLTUInt32_t *rawEvent,
					 AliHLTUInt32_t rawEventSize32,
					 AliHLTUInt32_t *output,
					 AliHLTUInt32_t &outputSize32,
					 const AliHLTTPCClusterMCLabel *mcLabels,
					 AliHLTUInt32_t nMCLabels,
					 AliHLTTPCClusterMCData *outputMC
					 )
{  
  // Loops over all rows finding the clusters 

  AliHLTUInt32_t maxOutputSize32 = outputSize32;
  outputSize32 = 0;
  if( outputMC ) outputMC->fCount = 0;   
  AliHLTUInt32_t maxNMCLabels = nMCLabels;
  if( !rawEvent ) return -1;    

  // Initialise 

  int ret = 0;

  fChannelExtractor.Init( fkMapping, mcLabels, 3*rawEventSize32 );
  fChannelProcessor.Init();
  fChannelMerger.Init();
  fDivisionUnit.Init();

  // Read the data, word by word 
  
  for( AliHLTUInt32_t  iWord=0; iWord<=rawEventSize32; iWord++ ){

    const AliHLTTPCHWCFBunch *bunch=0;
    const AliHLTTPCHWCFClusterFragment *fragment=0;
    const AliHLTTPCHWCFClusterFragment *candidate=0;
    const AliHLTTPCHWCFCluster *cluster = 0;
    
    if( iWord<rawEventSize32 ) fChannelExtractor.InputStream(ReadBigEndian(rawEvent[iWord]));
    else fChannelExtractor.InputEndOfData();

    while( (bunch = fChannelExtractor.OutputStream()) ){ 
      fChannelProcessor.InputStream(bunch);
      while( (fragment = fChannelProcessor.OutputStream() )){	
	fChannelMerger.InputStream( fragment );
	while( (candidate = fChannelMerger.OutputStream()) ){	    	  
	  fDivisionUnit.InputStream(candidate);
	  while( (cluster = fDivisionUnit.OutputStream()) ){	    
	    if( cluster->fFlag==1 ){
	      if( outputSize32+5 > maxOutputSize32 ){ // No space in the output buffer
		ret = -2;
		break;
	      }	      
	      AliHLTUInt32_t *co = &output[outputSize32];
	      co[0] = WriteBigEndian(cluster->fRowQ);
	      co[1] = cluster->fP;
	      co[2] = cluster->fT;
	      co[3] = cluster->fP2;
	      co[4] = cluster->fT2;
	      outputSize32+=5;
	      if( mcLabels && outputMC && outputMC->fCount < maxNMCLabels){
		outputMC->fLabels[outputMC->fCount++] = cluster->fMC;
	      }
	    }
	    else if( cluster->fFlag==2 ){
	      if( outputSize32+1 > maxOutputSize32 ){ // No space in the output buffer
		ret = -2;
		break;
	      }
	      output[outputSize32++] = cluster->fRowQ;
	    }
	  }
	}
      }
    }
  }
  return ret;
}

AliHLTUInt32_t AliHLTTPCHWCFEmulator::ReadBigEndian ( AliHLTUInt32_t word )
{  
  // read the word written in big endian format (lowest byte first)

  const AliHLTUInt8_t *bytes = reinterpret_cast<const AliHLTUInt8_t *>( &word );
  AliHLTUInt32_t i[4] = {bytes[0],bytes[1],bytes[2],bytes[3]};

  return (i[3]<<24) | (i[2]<<16) | (i[1]<<8) | i[0];
}

AliHLTUInt32_t AliHLTTPCHWCFEmulator::WriteBigEndian ( AliHLTUInt32_t word )
{
  // write the word in big endian format (least byte first)
  
  AliHLTUInt32_t ret = 0;
  AliHLTUInt8_t *bytes = reinterpret_cast<AliHLTUInt8_t *>( &ret );
  bytes[0] = (word      ) & 0xFF;
  bytes[1] = (word >>  8) & 0xFF;
  bytes[2] = (word >> 16) & 0xFF;
  bytes[3] = (word >> 24) & 0xFF;
  return ret;
}

AliHLTUInt32_t AliHLTTPCHWCFEmulator::CreateConfiguration
(
 bool doDeconvTime, bool doDeconvPad, bool doFlowControl, 
 bool doSinglePadSuppression, bool bypassMerger, 
 AliHLTUInt32_t clusterLowerLimit,AliHLTUInt32_t singleSeqLimit
 )
{
  // static method to create configuration word 

  AliHLTUInt32_t config = 0;

  config |= ( (AliHLTUInt32_t)doDeconvTime & 0x1 );
  config |= ( (AliHLTUInt32_t)doDeconvPad & 0x1 ) << 1;
  config |= ( (AliHLTUInt32_t)doFlowControl & 0x1 ) << 2;
  config |= ( (AliHLTUInt32_t)doSinglePadSuppression & 0x1 ) << 3;
  config |= ( (AliHLTUInt32_t)bypassMerger & 0x1 ) << 4;
  config |= ( (AliHLTUInt32_t)clusterLowerLimit & 0xFF )<<8;
  config |= ( (AliHLTUInt32_t)singleSeqLimit & 0xFF )<<16;

  return config;
}
