#ifndef ALIHLTMUONCORETRACK_H
#define ALIHLTMUONCORETRACK_H
/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

////////////////////////////////////////////////////////////////////////////////
//
// Author: Artur Szostak
// Email:  artur@alice.phy.uct.ac.za | artursz@iafrica.com
//
////////////////////////////////////////////////////////////////////////////////

#include "AliHLTMUONCorePoint.h"
#include "AliHLTMUONCoreRegionOfInterest.h"
#include "AliHLTMUONCoreTriggerRecord.h"


typedef UInt AliHLTMUONCoreTrackID;


struct AliHLTMUONCoreTrack
{

	AliHLTMUONCoreTriggerRecordID fTriggerid;
	AliHLTMUONCoreParticleSign fSign;
	Float fP;   // momentum.
	Float fPt;  // transverse momentum.
	AliHLTMUONCorePoint fPoint[10];  // Computed track coordinates on the 10 tracking chambers.
	AliHLTMUONCoreROI fRegion[10];   // Regions of interest from which clusters were used to compute this track.

};


#endif // ALIHLTMUONCORETRACK_H
