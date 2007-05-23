/**************************************************************************
 * Copyright(c) 2007-2009, ALICE Experiment at CERN, All rights reserved. *
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

///////////////////////////////////////////////////////////////////
//                                                               //
// Implementation of the base class for SDD detector algorithms  //
// Origin: F.Prino, Torino, prino@to.infn.it                     //
//                                                               //
///////////////////////////////////////////////////////////////////

#include "AliITSOnlineSDD.h"

ClassImp(AliITSOnlineSDD)
//______________________________________________________________________
  AliITSOnlineSDD::AliITSOnlineSDD():TObject(),fModuleId(0),fSide(0)
{
  // default constructor
}
//______________________________________________________________________
  AliITSOnlineSDD::AliITSOnlineSDD(Int_t mod, Int_t sid):TObject(),fModuleId(0),fSide(0)
{
  // standard constructor
  SetModule(mod);
  SetDetectorSide(sid);
}
