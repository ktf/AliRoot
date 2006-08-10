// $Id$

/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Authors: Matthias Richter <Matthias.Richter@ift.uib.no>                *
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

/** @file   AliHLTDataSink.cxx
    @author Matthias Richter
    @date   
    @brief  Base class implementation for HLT data source components. */

#if __GNUC__>= 3
using namespace std;
#endif

#include "AliHLTDataSink.h"

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTDataSink)

AliHLTDataSink::AliHLTDataSink()
{ 
}

AliHLTDataSink::~AliHLTDataSink()
{ 
}

int AliHLTDataSink::ProcessEvent( const AliHLTComponent_EventData& evtData,
				    const AliHLTComponent_BlockData* blocks, 
				    AliHLTComponent_TriggerData& trigData,
				    AliHLTUInt8_t* outputPtr, 
				    AliHLTUInt32_t& size,
				    AliHLTUInt32_t& outputBlockCnt, 
				    AliHLTComponent_BlockData*& outputBlocks,
				    AliHLTComponent_EventDoneData*& edd )
{
  int iResult=0;
  vector<AliHLTComponent_BlockData> blockData;
  iResult=DumpEvent(evtData, blocks, trigData);
  return iResult;
}
