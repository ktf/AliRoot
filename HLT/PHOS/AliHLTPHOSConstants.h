//-*- Mode: C++ -*-
// $Id$

//* This file is property of and copyright by the ALICE HLT Project        * 
//* ALICE Experiment at CERN, All rights reserved.                         *
//* See cxx source for full Copyright notice                               */

/// @file   AliHLTPHOSConstants.h
/// @author Svein Lindal
/// @date   
/// @brief  Class containing constants for PHOS libraries.

#ifndef ALIHLTPHOSCONSTANTS_H
#define ALIHLTPHOSCONSTANTS_H


#include <TString.h>
#include "Rtypes.h"

#include "AliHLTCaloConstants.h"

//class AliHLTCaloConstants;


class AliHLTPHOSConstants : public AliHLTCaloConstants
{
public:
  AliHLTPHOSConstants();

  ~AliHLTPHOSConstants();
  
  virtual void InitConstants(); 

  // private:
  ClassDef(AliHLTPHOSConstants, 1)
};

#endif

