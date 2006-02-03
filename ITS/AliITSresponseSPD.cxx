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
//////////////////////////////////////////////////////
//  Base Response class forITS                      //
//  It is used to set static data members           //
//  connected to parameters equal for all           //
//  the SPD modules                                 //
//////////////////////////////////////////////////////
#include "AliITSresponseSPD.h"

const Float_t AliITSresponseSPD::fgkDiffCoeffDefault = 0.;
const Float_t AliITSresponseSPD::fgkCouplingColDefault = 0;
const Float_t AliITSresponseSPD::fgkCouplingRowDefault = 0;

ClassImp(AliITSresponseSPD)	
//______________________________________________________________________
AliITSresponseSPD::AliITSresponseSPD():
  AliITSresponse(){


  // constructor
  SetCouplingParam(fgkCouplingColDefault,fgkCouplingRowDefault);
  SetDiffCoeff(fgkDiffCoeffDefault,0.);

}
