// @(#) $Id$

#ifndef ALIHLTTPCCLUSTERFINDERCOMPONENT_H
#define ALIHLTTPCCLUSTERFINDERCOMPONENT_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* AliHLTTPCClusterFinderComponent
 */

#include "AliHLTProcessor.h"
#include "AliHLTTPCDefinitions.h"
#include "AliHLTTPCDigitReaderPacked.h"
#include "AliHLTTPCDigitReaderUnpacked.h"

class AliHLTTPCClusterFinder;

class AliHLTTPCClusterFinderComponent : public AliHLTProcessor
    {
    public:
	AliHLTTPCClusterFinderComponent(bool packed);
	virtual ~AliHLTTPCClusterFinderComponent();

	// Public functions to implement AliHLTComponent's interface.
	// These functions are required for the registration process

	const char* GetComponentID();
	void GetInputDataTypes( vector<AliHLTComponent_DataType>& list);
	AliHLTComponent_DataType GetOutputDataType();
	virtual void GetOutputDataSize( unsigned long& constBase, double& inputMultiplier );
	AliHLTComponent* Spawn();
	
    protected:
	
	// Protected functions to implement AliHLTComponent's interface.
	// These functions provide initialization as well as the actual processing
	// capabilities of the component. 

	int DoInit( int argc, const char** argv );
	int DoDeinit();
	int DoEvent( const AliHLTComponent_EventData& evtData, const AliHLTComponent_BlockData* blocks, 
		     AliHLTComponent_TriggerData& trigData, AliHLTUInt8_t* outputPtr, 
		     AliHLTUInt32_t& size, vector<AliHLTComponent_BlockData>& outputBlocks );
	
    private:

      AliHLTTPCClusterFinder* fClusterFinder;
      AliHLTTPCDigitReaderPacked* fReaderPacked;
      AliHLTTPCDigitReaderUnpacked* fReaderUnpacked;
      bool fClusterDeconv;
      float fXYClusterError;
      float fZClusterError;
      Int_t fPackedSwitch;
      
      ClassDef(AliHLTTPCClusterFinderComponent, 0)

    };
#endif
