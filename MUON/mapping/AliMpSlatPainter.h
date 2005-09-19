/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

// $Id$
// $MpId: AliMpSlatPainter.h,v 1.6 2005/09/19 18:56:46 ivana Exp $

/// \ingroup graphics
/// \class AliMpSlatPainter
/// \brief Class for drawing a slat into canvas
///
/// Authors: Laurent Aphecetche

#ifndef ALIMPSLATPAINTER_H
#define ALIMPSLATPAINTER_H

#ifndef ALI_MP_V_PAINTER_H
#  include "AliMpVPainter.h"
#endif

class AliMpSlat;

class AliMpSlatPainter : public AliMpVPainter
{
 public:
  AliMpSlatPainter();
  AliMpSlatPainter(const AliMpSlat* slat);
  virtual ~AliMpSlatPainter();

  TVector2 GetDimensions() const;

  TVector2 GetPosition() const;

  void Draw(Option_t* option);

  void Paint(Option_t* option);
  
 private:
  const AliMpSlat* fkSlat;

  ClassDef(AliMpSlatPainter,1) // A painter for a slat of stations 3,4,5
};

#endif
