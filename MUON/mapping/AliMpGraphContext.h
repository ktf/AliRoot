/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

// $Id$
// $MpId: AliMpGraphContext.h,v 1.8 2005/09/26 16:06:11 ivana Exp $

/// \ingroup graphics
/// \class AliMpGraphContext
/// \brief Class describing the correspondance between a given area
/// in pad, and a zone of real (cm) position
///
/// Author: David GUEZ, IPN Orsay

#ifndef ALI_MP_GRAPH_CONTEXT_H
#define ALI_MP_GRAPH_CONTEXT_H

#include "AliMpContainers.h"

#ifdef WITH_STL
#include <vector>
#endif

#ifdef WITH_ROOT
#include "AliMpExMap.h"
#endif

#include <TObject.h>
#include <TVector2.h>

class MPainter;

class AliMpGraphContext : public TObject
{
 public:
#ifdef WITH_STL
  typedef std::vector<AliMpGraphContext*> GraphContextVector;
#endif
#ifdef WITH_ROOT
  typedef TObjArray GraphContextVector;
#endif

 public:
  void Push() const;
  void Pop();
  static AliMpGraphContext *Instance();

  // set methods

  void SetPadPosition(const TVector2 &position){fPadPosition=position;}
  void SetPadDimensions(const TVector2 &dimensions){fPadDimensions=dimensions;}
  void SetRealPosition(const TVector2 &position){fRealPosition=position;}
  void SetRealDimensions(const TVector2 &dimensions){fRealDimensions=dimensions;}
  void SetColor(Int_t color){fColor=color;}

  // get methods
  TVector2 GetPadPosition() const {return fPadPosition;}
  TVector2 GetPadDimensions() const {return fPadDimensions;}
  TVector2 GetRealPosition() const{return fRealPosition;}
  TVector2 GetRealDimensions() const{return fRealDimensions;}
  Int_t GetColor() const {return fColor;}

  //methods
  TVector2 RealToPad(const TVector2 &position) const;
  void RealToPad(const TVector2 &position
		 ,const TVector2 &dimensions,
		 TVector2 &padPosition,
		 TVector2 &padDimensions) const;
  void SetPadPosForReal(const TVector2 &position,const TVector2 &dimensions);

 protected:
  AliMpGraphContext(const AliMpGraphContext& right);
  AliMpGraphContext&  operator = (const AliMpGraphContext& right);

 private:
  //private constructor (not instanciable from outside)
  AliMpGraphContext();

  // static data members
  static AliMpGraphContext *fgInstance;   // the global instance
  static GraphContextVector fgStack;  // the object stack
#ifdef WITH_ROOT
  static Int_t fgStackSize;  // the object stack size
#endif

  //data members
  Int_t    fColor;          // color to use
  TVector2 fPadPosition;    // Position of the pad area where to draw
  TVector2 fPadDimensions;   // Dimensions of the pad area where to draw

  TVector2 fRealPosition;   // Position of the real area to draw
  TVector2 fRealDimensions;  // Dimensions of the real area to draw

  ClassDef(AliMpGraphContext,1) // Correspondance pad area/real world
};

#endif //ALI_MP_GRAPH_CONTEXT_H


