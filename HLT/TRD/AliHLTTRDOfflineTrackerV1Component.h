//-*- Mode: C++ -*-
// $Id$
#ifndef ALIHLTTRDOFFLINETRACKERV1COMPONENT_H
#define ALIHLTTRDOFFLINETRACKERV1COMPONENT_H
//* This file is property of and copyright by the ALICE HLT Project        * 
//* ALICE Experiment at CERN, All rights reserved.                         *
//* See cxx source for full Copyright notice                               *

/** @file   AliHLTTRDOfflineTrackerV1Component
    @author 
    @date   2009-08-31
    @brief  
*/

#include "AliHLTTRDTrackerV1Component.h"

class AliHLTTRDOfflineTrackerV1Component : public AliHLTTRDTrackerV1Component
{
public:
  AliHLTTRDOfflineTrackerV1Component();
  virtual ~AliHLTTRDOfflineTrackerV1Component();

  const char* GetComponentID();
  int GetOutputDataTypes(AliHLTComponentDataTypeList& tgtList);
  virtual void GetOutputDataSize( unsigned long& constBase, double& inputMultiplier );
  AliHLTComponent* Spawn();

  int DoInit( int argc, const char** argv );
  int DoDeinit();
  int DoEvent( const AliHLTComponent_EventData& evtData, const AliHLTComponent_BlockData* blocks, 
  	       AliHLTComponent_TriggerData& trigData, AliHLTUInt8_t* outputPtr, 
  	       AliHLTUInt32_t& size, vector<AliHLTComponent_BlockData>& outputBlocks );

protected:
  void SetOfflineParams();

  ClassDef(AliHLTTRDOfflineTrackerV1Component, 1)

};
#endif
