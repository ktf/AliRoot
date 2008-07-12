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


#include "AliITSBadChannelsSSD.h"

//////////////////////////////////////////////////////
// Author: Enrico Fragiacomo
// Date: 23/08/2007
// Modified: 08/07/2008
//                                                  //
//////////////////////////////////////////////////////

//const Int_t AliITSBadChannelsSSD::fgkDefaultNModulesSSD = 1698;
//const Int_t AliITSBadChannelsSSD::fgkDefaultNStripsSSD = 768;

ClassImp(AliITSBadChannelsSSD)
  
//______________________________________________________________________
  AliITSBadChannelsSSD::AliITSBadChannelsSSD()
    // Default Constructor 
{ 
    for(Int_t i=0; i<2*fgkDefaultNModulesSSD*fgkDefaultNStripsSSD; i++) 
      fBadChannels[i]=0;    
  }

//______________________________________________________________________
AliITSBadChannelsSSD::AliITSBadChannelsSSD(const AliITSBadChannelsSSD &source): 
  TObject(source)  
{
    // copy Constructor
  memcpy(fBadChannels,source.fBadChannels,
	 2*fgkDefaultNModulesSSD*fgkDefaultNStripsSSD*sizeof(Char_t));
}

//______________________________________________________________________
AliITSBadChannelsSSD::~AliITSBadChannelsSSD(){
    // destructor
}

//______________________________________________________________________
AliITSBadChannelsSSD& AliITSBadChannelsSSD::operator=(const AliITSBadChannelsSSD &source) {
 // ass. op.
    if (this == &source)return *this;
    memcpy(fBadChannels,source.fBadChannels,
	 2*fgkDefaultNModulesSSD*fgkDefaultNStripsSSD*sizeof(Char_t));
    
    return *this;
}
