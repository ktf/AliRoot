////////////////////////////////////////////////////////////////////////////////
//
// Author: Artur Szostak
// Email:  artur@alice.phy.uct.ac.za | artursz@iafrica.com
//
////////////////////////////////////////////////////////////////////////////////

#ifndef ALIHLTMUONCORETRACKER_H
#define ALIHLTMUONCORETRACKER_H

#include "AliHLTMUONBasicTypes.h"
#include "AliHLTMUONUtils.h"
#include "AliHLTMUONCoreTriggerRecord.h"
#include "AliHLTMUONCoreCluster.h"
#include "AliHLTMUONCoreTrack.h"
#include "AliHLTMUONCoreEventID.h"
#include "AliHLTMUONCoreRegionOfInterest.h"


class AliHLTMUONCoreTracker;


class AliHLTMUONCoreTrackerCallback
{
public:

	virtual ~AliHLTMUONCoreTrackerCallback() {};
	
	/* All clusters that fall within the specified boundary box on the specified
	   chamber should be returned to the tracker, by calling the ReturnClusters
	   method of the given tracker. The same tag parameter must be passed on the 
	   ReturnClusters method's parameter list.
	 */
	virtual void RequestClusters(
			AliHLTMUONCoreTracker* tracker,
			Float left, Float right, Float bottom, Float top,
			AliHLTMUONCoreChamberID chamber, const void* tag
		) = 0;

	/* When this method is called then one knows no more RequestClusters method
	   calls are expected.
	 */
	virtual void EndOfClusterRequests(AliHLTMUONCoreTracker* tracker) = 0;

	/* This method is called when the tracker has found a track. The FillTrackData
	   method of the given tracker should be called to receive the track data.
	   At this point all cluster blocks can be released.
	 */
	virtual void FoundTrack(AliHLTMUONCoreTracker* tracker) = 0;
	
	/* When the tracker is finished with its work but no track was found then
	   this method is called. At this point no more work should be performed by
	   the tracker and all cluster blocks can be released.
	 */
	virtual void NoTrackFound(AliHLTMUONCoreTracker* tracker) = 0;
};


class AliHLTMUONCoreTracker
{
public:

	AliHLTMUONCoreTracker()
	{
		fCallback = NULL;
	};

	virtual ~AliHLTMUONCoreTracker() {};

	/* This is the starting point for the tracking algorithm. The tracker is 
	   called at this point with the specified trigger record. It needs to figure
	   out which cluster blocks it needs and request them with calls to
	   RequestClusters.
	   Any memory allocated at this point should be released in the Reset method.
	 */
	virtual void FindTrack(const AliHLTMUONCoreTriggerRecord& trigger) = 0;

	/* When requested clusters have been found by the framework they are returned
	   to the tracker using this method.
	   This method should implement any processing of the cluster blocks. If more
	   more regions of interest are identified then appropriate request should me 
	   made using RequestClusters. The tag parameter will be the same one as was
	   passed to RequestClusters.
	 */
	virtual void ReturnClusters(void* tag, const AliHLTMUONCoreClusterPoint* clusters, UInt count) = 0;

	/* When no more clusters are to be expected for the request with the corresponding
	   tag value, then this method is called.
	   Any final processing can be placed in here and when the track is found then
	   the algorithm can call FoundTrack otherwise NoTrackFound to indicate end of 
	   processing.
	 */
	virtual void EndOfClusters(void* tag) = 0;
	
	/* Called to receive track information after receiving a FoundTrack call.
	   The tracker should fill the track data block with all relevant information.
	   Note: the track.triggerid field need not be filled in this method. It should
	   be overwritten by the caller. 
	 */
	virtual void FillTrackData(AliHLTMUONCoreTrack& track) = 0;
	
	/* Called when the tracker should be reset to a initial state. 
	   All extra internal allocated data structured should be released.
	 */
	virtual void Reset() = 0;


	/* To set the TrackerCallback callback object.
	 */
	inline void SetCallback(AliHLTMUONCoreTrackerCallback* callback)
	{
		fCallback = callback;
	};

protected:

	/* To request clusters from the boundary box specified by the 'left', 'right',
	   'top' and 'bottom' boundaries and on the given chamber use this method call.
	   Supply a tag parameter if you want the request uniquely identified. 
	   This is usefull to supply a pointer to some internal state data structure
	   to figure out where processing should continue in the ReturnClusters or
	   EndOfClusters methods.
	 */
	inline void RequestClusters(
			Float left, Float right, Float bottom, Float top,
			AliHLTMUONCoreChamberID chamber, const void* tag = NULL
		)
	{
		Assert( fCallback != NULL );
		fCallback->RequestClusters(this, left, right, bottom, top, chamber, tag);
	};

	/* When no more cluster requests will be generated by this tracker then this
	   method should be called.
	   DO NOT request more clusters after calling this method.
	 */
	inline void EndOfClusterRequests()
	{
		Assert( fCallback != NULL );
		fCallback->EndOfClusterRequests(this);
	};

	/* When the tracker has found a track it should call this method to inform
	   the rest of the system. At this point all cluster blocks received with
	   ReturnClusters are to be considered released and MUST NOT be accessed.
	 */
	inline void FoundTrack()
	{
		Assert( fCallback != NULL );
		fCallback->FoundTrack(this);
	};

	/* If the tracker is finished processing the trigger record but has not found 
	   a track it should call this method to inform the rest of the system.
	   At this point all cluster blocks received with ReturnClusters are to be
	   considered released and MUST NOT be accessed.
	 */
	inline void NoTrackFound()
	{
		Assert( fCallback != NULL );
		fCallback->NoTrackFound(this);
	};

private:

	AliHLTMUONCoreTrackerCallback* fCallback;
};


#endif // ALIHLTMUONCORETRACKER_H
