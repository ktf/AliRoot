#ifndef ALIHLTMUONMANSOCANDIDATESBLOCKSTRUCT_H
#define ALIHLTMUONMANSOCANDIDATESBLOCKSTRUCT_H
/* Copyright(c) 1998-2007, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

/**
 * @file   AliHLTMUONMansoCandidatesBlockStruct.h
 * @author Artur Szostak <artursz@iafrica.com>
 * @date   
 * @brief  Definition of internal dimuon HLT track candidates data block
 *         structure generated by the Manso algorithm.
 * 
 * The structures are defined with C linkage since C generally gives us more
 * binary compatibility between compilers.
 */

#include "AliHLTMUONMansoTracksBlockStruct.h"

extern "C"
{

/**
 * Structure containing information about a region of interest (RoI) used in
 * the Manso algorithm to find track candidates.
 */
struct AliHLTMUONMansoRoIStruct
{
	AliHLTFloat32_t fX; // The X coordinate for the RoI disc centre.
	AliHLTFloat32_t fY; // The Y coordinate for the RoI disc centre.
	AliHLTFloat32_t fZ; // The detection plane Z coordinate in which the RoI is in.
	AliHLTFloat32_t fRadius; // The radius of the RoI disc.
};

/**
 * This structure contains information about a track candidate that was attempted
 * during track reconstruction in the Manso algorithm. The regions of interests
 * used are indicated including the partially reconstructed track.
 */
struct AliHLTMUONMansoCandidateStruct
{
	// The partially of fully reconstructed track.
	AliHLTMUONMansoTrackStruct fTrack;
	
	// The regions of interest searched on the various chambers.
	// fRoI[0] is region of interest on chamber 7, fRoI[1] is on 8 etc...
	AliHLTMUONMansoRoIStruct fRoI[4];
};

/**
 * AliHLTMUONMansoCandidatesBlockStruct defines the format of the internal
 * Manso track candidates data block.
 */
struct AliHLTMUONMansoCandidatesBlockStruct
{
	AliHLTMUONDataBlockHeader fHeader; // Common data block header.

	// Array of Manso track candidates.
	//AliHLTMUONMansoCandidateStruct fCandidate[/*fHeader.fNrecords*/];
};

} // extern "C"


/**
 * Stream operator for usage with std::ostream classes which prints the RoI
 * information in the following format:
 *  {fX = xx, fY = yy, fZ = zz, fRadius = rr}
 */
std::ostream& operator << (
		std::ostream& stream, const AliHLTMUONMansoRoIStruct& roi
	);

/**
 * Stream operator for usage with std::ostream classes which prints the track
 * candidate information in the following format:
 *  {fTrack = xx, fRoI[0] = {...}, fRoI[1] = {...}, fRoI[2] = {...}, fRoI[3] = {...}}
 */
std::ostream& operator << (
		std::ostream& stream, const AliHLTMUONMansoCandidateStruct& candidate
	);

/**
 * Stream operator for usage with std::ostream classes which prints the
 * AliHLTMUONMansoCandidatesBlockStruct in the following format:
 *   {fHeader = xx, fCandidate[] = [{..}, {..}, ...]}
 */
std::ostream& operator << (
		std::ostream& stream,
		const AliHLTMUONMansoCandidatesBlockStruct& block
	);


inline bool operator == (
		const AliHLTMUONMansoRoIStruct& a,
		const AliHLTMUONMansoRoIStruct& b
	)
{
	return a.fX == b.fX and a.fY == b.fY and a.fZ == b.fZ
		and a.fRadius == b.fRadius;
}

inline bool operator == (
		const AliHLTMUONMansoCandidateStruct& a,
		const AliHLTMUONMansoCandidateStruct& b
	)
{
	return a.fTrack == b.fTrack
		and a.fRoI[0] == b.fRoI[0] and a.fRoI[1] == b.fRoI[1]
		and a.fRoI[2] == b.fRoI[2] and a.fRoI[3] == b.fRoI[3];
}

inline bool operator != (
		const AliHLTMUONMansoCandidateStruct& a,
		const AliHLTMUONMansoCandidateStruct& b
	)
{
	return not operator == (a, b);
}


bool operator == (
		const AliHLTMUONMansoCandidatesBlockStruct& a,
		const AliHLTMUONMansoCandidatesBlockStruct& b
	);

inline bool operator != (
		const AliHLTMUONMansoCandidatesBlockStruct& a,
		const AliHLTMUONMansoCandidatesBlockStruct& b
	)
{
	return not operator == (a, b);
}

#endif // ALIHLTMUONMANSOCANDIDATESBLOCKSTRUCT_H
