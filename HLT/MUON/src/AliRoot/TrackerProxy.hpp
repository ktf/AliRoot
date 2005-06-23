////////////////////////////////////////////////////////////////////////////////
//
// Author: Artur Szostak
// Email:  artur@alice.phy.uct.ac.za | artursz@iafrica.com
//
////////////////////////////////////////////////////////////////////////////////

#ifndef dHLT_ALIROOT_TRACKER_PROXY_HPP
#define dHLT_ALIROOT_TRACKER_PROXY_HPP

#include "Tracking/Tracker.hpp"
#include "AliRoot/TrackerCallback.hpp"
#include "AliRoot/TrackerInterface.hpp"

namespace dHLT
{
namespace AliRoot
{


class TrackerProxy : public Tracking::Tracker, public AliMUONHLT::TrackerCallback
{
public:

	TrackerProxy(AliMUONHLT::TrackerInterface* client);
	virtual ~TrackerProxy() {};

	// inherited methods from Tracking::Tracker:
	virtual void FindTrack(const TriggerRecord& trigger);
	virtual void ReturnClusters(void* tag, const ClusterPoint* clusters, UInt count);
	virtual void EndOfClusters(void* tag);
	virtual void FillTrackData(Track& track);
	virtual void Reset();

	// inherited methods from AliMUONHLT::TrackerCallback:
	virtual void RequestClusters(
			Float_t left, Float_t right, Float_t bottom, Float_t top,
			Int_t chamber, const void* tag = NULL
		);
	virtual void EndOfClusterRequests();
	virtual void FoundTrack();
	virtual void NoTrackFound();

private:

	AliMUONHLT::TrackerInterface* tracker;  // The tracker we are proxying for.
};


} // AliRoot
} // dHLT

#endif // dHLT_ALIROOT_TRACKER_PROXY_HPP
