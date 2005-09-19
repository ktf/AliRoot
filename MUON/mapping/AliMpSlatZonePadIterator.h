/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

// $Id$
// $MpId: AliMpSlatZonePadIterator.h,v 1.3 2005/08/26 15:42:32 ivana Exp $

/// \ingroup slat
/// \class AliMpSlatZonePadIterator
/// \brief To be added ...
/// 
/// Author: Laurent Aphecetche

#ifndef ALI_MP_SLAT_ZONE_PAD_ITERATOR_H
#define ALI_MP_SLAT_ZONE_PAD_ITERATOR_H

#include "AliMpVPadIterator.h"
#include "AliMpArea.h"
#include "TVector2.h"

class AliMpSlat;
class AliMpSlatSegmentation;

class AliMpSlatZonePadIterator : public AliMpVPadIterator
{
 public:
  AliMpSlatZonePadIterator(const AliMpSlat* slat, const AliMpArea& area);
  virtual ~AliMpSlatZonePadIterator();

  void First();
  void Next();
  Bool_t IsDone() const;
  AliMpPad CurrentItem() const;
  void Invalidate();
 
 private:
  Bool_t CropArea();
  Bool_t GetNextPosition(Double_t& x, Double_t& y);
  void SetPad(AliMpPad& pad, const TVector2& pos);

 private:
  const AliMpSlat*       fkSlat;
  AliMpSlatSegmentation* fSlatSegmentation;
  AliMpArea  fArea;
  TVector2   fOffset;
  TVector2   fStep;
  AliMpPad   fCurrentPad;
  Bool_t     fIsDone;

  ClassDef(AliMpSlatZonePadIterator,1) // Pad iterator for a zone of constant density, for St345.
};

#endif
