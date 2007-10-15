
/**************************************************************************
 * This file is property of and copyright by the ALICE HLT Project        * 
 * All rights reserved.                                                   *
 *                                                                        *
 * Primary Authors: Oystein Djuvsland                                                      *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          * 
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#ifndef ALIHLTPHOSRECPOINTDATASTRUCT_H
#define ALIHLTPHOSRECPOINTDATASTRUCT_H

//struct AliHLTPHOSDigitDataStruct;
#include "AliHLTPHOSDigitDataStruct.h"
//#include "AliHLTTypes.h"

struct AliHLTPHOSRecPointDataStruct
{

  //AliHLTUInt8_t fMultiplicity; 
  UInt_t fMultiplicity;
  Float_t fX;
  Float_t fZ;
  Float_t fAmp;
  Float_t fM2x;
  Float_t fM2z;
  Float_t fM3x;
  Float_t fM4z;
  Float_t fPhixe;
  Float_t fDistanceToBadChannel;
  AliHLTPHOSDigitDataStruct fDigitsList[64];

};

#endif
