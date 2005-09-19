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

// $Id$
// $MpId: AliMpVPadRowSegment.cxx,v 1.5 2005/08/26 15:43:36 ivana Exp $
// Category: sector
//
// Class AliMpVPadRowSegment
// --------------------
// The abstract base class for a pad row segment composed of the 
// the identic pads.
// Included in AliRoot: 2003/05/02
// Authors: David Guez, Ivana Hrivnacova; IPN Orsay

#include <TError.h>

#include "AliMpVPadRowSegment.h"
#include "AliMpPadRow.h"
#include "AliMpMotif.h"
#include "AliMpMotifType.h"

ClassImp(AliMpVPadRowSegment)

//_____________________________________________________________________________
AliMpVPadRowSegment::AliMpVPadRowSegment(AliMpPadRow* padRow, AliMpMotif* motif, 
                                       Int_t motifPositionId, Int_t nofPads)
  : TObject(),
    fNofPads(nofPads),
    fOffsetX(0.),
    fPadRow(padRow),
    fMotif(motif),
    fMotifPositionId(motifPositionId)
{
/// Standard contructor 
}

//_____________________________________________________________________________
AliMpVPadRowSegment::AliMpVPadRowSegment() 
  : TObject(),
    fNofPads(0),
    fOffsetX(0.),
    fPadRow(0),
    fMotif(0),
    fMotifPositionId(0)
{
/// Default contructor 
}

//_____________________________________________________________________________
AliMpVPadRowSegment::AliMpVPadRowSegment(const AliMpVPadRowSegment& right) 
  : TObject(right) 
{
/// Protected copy constructor (not provided) 

  Fatal("AliMpVPadRowSegment", "Copy constructor not provided.");
}

//_____________________________________________________________________________
AliMpVPadRowSegment::~AliMpVPadRowSegment() 
{
/// Destructor   
}

//
// operators
//

//_____________________________________________________________________________
AliMpVPadRowSegment& 
AliMpVPadRowSegment::operator=(const AliMpVPadRowSegment& right)
{
/// Protected assignment operator (not provided)

  // check assignment to self
  if (this == &right) return *this;

  Fatal("operator =", "Assignment operator not provided.");
    
  return *this;  
}    

//
// public methods  
//

//_____________________________________________________________________________
Double_t  AliMpVPadRowSegment::HalfSizeY() const
{
/// Return the size in y of this row segment.

  return fMotif->GetPadDimensions().Y();
}

//_____________________________________________________________________________
AliMpPadRow*  AliMpVPadRowSegment::GetPadRow() const
{
/// Return the pad row.which this pad row segment belongs to.

  return fPadRow;
}  

//_____________________________________________________________________________
AliMpMotif*  AliMpVPadRowSegment::GetMotif() const
{
/// Return the motif of this pad row segment. 

  return fMotif;
}  

//_____________________________________________________________________________
Int_t  AliMpVPadRowSegment::GetMotifPositionId() const
{
/// Return the motif of this pad row segment. 

  return fMotifPositionId;
}  

//_____________________________________________________________________________
void  AliMpVPadRowSegment::SetOffsetX(Double_t offsetX)
{
/// Set the x offset.

  fOffsetX = offsetX;
}    

