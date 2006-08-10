// @(#) $Id$

#ifndef ALIHLTDATASINK_H
#define ALIHLTDATASINK_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/** @file   AliHLTDataSink.h
    @author Matthias Richter
    @date   
    @brief  Base class declaration for HLT data sink components.
    @note   The class is used in Offline (AliRoot) context
*/

#include "AliHLTComponent.h"

/**
 * @class AliHLTDataSink
 * Base class of HLT data sink components.
 * The class provides a common interface for the implementation of HLT data
 * sink components. The child class must implement the functions:
 * - @ref DoInit (optional)
 * - @ref DoDeinit (optional)
 * - @ref DumpEvent
 * - @ref GetComponentID
 * - @ref GetInputDataTypes
 * - @ref GetOutputDataType
 * - @ref GetOutputDataSize
 * - @ref Spawn
 *
 * @ingroup alihlt_component
 */
class AliHLTDataSink : public AliHLTComponent {
 public:
  /** standard constructor */
  AliHLTDataSink();
  /** standard destructor */
  virtual ~AliHLTDataSink();

  /**
   * Event processing function.
   * The method is called by the framework to process one event. After 
   * preparation of data structures. The call is redirected to DumpEvent.
   * @return neg. error code if failed
   */
  int ProcessEvent( const AliHLTComponent_EventData& evtData,
		    const AliHLTComponent_BlockData* blocks, 
		    AliHLTComponent_TriggerData& trigData,
		    AliHLTUInt8_t* outputPtr, 
		    AliHLTUInt32_t& size,
		    AliHLTUInt32_t& outputBlockCnt, 
		    AliHLTComponent_BlockData*& outputBlocks,
		    AliHLTComponent_EventDoneData*& edd );

  // Information member functions for registration.

  /**
   * Return @ref AliHLTComponent::kSink type as component type.
   * @return component type id
   */
  TComponentType GetComponentType() { return AliHLTComponent::kSink;}

 private:
  /**
   * Data processing method for the component.
   * @param evtData       event data structure
   * @param blocks        input data block descriptors
   * @param trigData	  trigger data structure
   */
  virtual int DumpEvent( const AliHLTComponent_EventData& evtData,
			 const AliHLTComponent_BlockData* blocks, 
			 AliHLTComponent_TriggerData& trigData ) = 0;

  ClassDef(AliHLTDataSink, 0)
};
#endif
