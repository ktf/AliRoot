// $Id$

//**************************************************************************
//* This file is property of and copyright by the ALICE HLT Project        * 
//* ALICE Experiment at CERN, All rights reserved.                         *
//*                                                                        *
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *
//*                  for The ALICE HLT Project.                            *
//*                                                                        *
//* Permission to use, copy, modify and distribute this software and its   *
//* documentation strictly for non-commercial purposes is hereby granted   *
//* without fee, provided that the above copyright notice appears in all   *
//* copies and that both the copyright notice and this permission notice   *
//* appear in the supporting documentation. The authors make no claims     *
//* about the suitability of this software for any purpose. It is          *
//* provided "as is" without express or implied warranty.                  *
//**************************************************************************

/** @file   AliHLTReconstructorBase.cxx
    @author Matthias Richter
    @date   
    @brief  Base class for HLT reconstruction classes.
*/

#include "AliHLTReconstructorBase.h"
#include "AliHLTSystem.h"

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTReconstructorBase)

AliHLTReconstructorBase::AliHLTReconstructorBase()
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt
  fNofInstances++;
}

AliHLTReconstructorBase::~AliHLTReconstructorBase()
{
  // see header file for class documentation
  if (--fNofInstances<=0) delete fpSystem;
  fpSystem=NULL;
}

void AliHLTReconstructorBase::InitInstance()
{
  // see header file for class documentation
  if (!fpSystem) fpSystem=new AliHLTSystem;
}

AliHLTSystem* AliHLTReconstructorBase::GetInstance()
{
  // see header file for class documentation
  if (!fpSystem) InitInstance();
  return fpSystem;
}


AliHLTSystem* AliHLTReconstructorBase::fpSystem=NULL;

int AliHLTReconstructorBase::fNofInstances=0;
