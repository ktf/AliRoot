//-*- Mode: C++ -*-
// @(#) $Id$

//* This file is property of and copyright by the ALICE HLT Project         * 
//* ALICE Experiment at CERN, All rights reserved.                          *
//* See cxx source for full Copyright notice                                *

//*                                                                         *
//*  AliHLTTPCCADisplay class is a debug utility.                           *
//*  It is not used in the normal data processing.                          *
//*                                                                         *

#ifndef ALIHLTTPCCADISPLAY_H
#define ALIHLTTPCCADISPLAY_H


class AliHLTTPCCATracker;
class AliHLTTPCCAGBTracker;
class AliHLTTPCCATrack;
class AliHLTTPCCATrackParam;
class AliHLTTPCCAPerformance;
class TCanvas;
#include "TArc.h"
#include "TLine.h"
#include "TPolyLine.h"
#include "TBox.h"
#include "TCrown.h"
#include "TMarker.h"
#include "TLatex.h"

/**
 * @class AliHLTTPCCADisplay
 */
class AliHLTTPCCADisplay
{

 public:

  class AliHLTTPCCADisplayTmpHit{  

  public:
    Int_t ID() const { return fHitID; }
    Double_t S() const { return fS; }
    Double_t Z() const { return fZ; }

    void SetID( Int_t v ){ fHitID = v; }
    void SetS( Double_t v){ fS = v; }
    void SetZ( Double_t v){ fZ = v; }

    static Bool_t CompareHitDS( const AliHLTTPCCADisplayTmpHit &a, 
				 const AliHLTTPCCADisplayTmpHit  &b )
    {    
      return (a.fS < b.fS);
    }
    static Bool_t CompareHitZ( const AliHLTTPCCADisplayTmpHit &a, 
				 const AliHLTTPCCADisplayTmpHit  &b )
    {    
      return (a.fZ < b.fZ);
    }
  protected:
    Int_t fHitID; // hit ID
    Double_t fS;  // hit position on the XY track curve 
    Double_t fZ;  // hit Z position
  };

  static AliHLTTPCCADisplay &Instance();
  
  AliHLTTPCCADisplay();
  AliHLTTPCCADisplay( const AliHLTTPCCADisplay& );
  AliHLTTPCCADisplay& operator=(const AliHLTTPCCADisplay&);

  virtual ~AliHLTTPCCADisplay();

  void Init();
  void Update();
  void ClearView();
  void Ask();
  void SetSliceView();
  void SetTPCView();
  void SetCurrentSlice( AliHLTTPCCATracker *slice ); 
  void SetGB( AliHLTTPCCAGBTracker *GBTracker );   
  void Set2Slices( AliHLTTPCCATracker *slice );

  Int_t GetColor( Int_t i ) const;
  Int_t GetColorZ( Double_t z ) const ;
  Int_t GetColorY( Double_t y ) const ;
  Int_t GetColorK( Double_t k ) const ;
  void Global2View( Double_t x, Double_t y, Double_t *xv, Double_t *yv ) const ;
  void Slice2View( Double_t x, Double_t y, Double_t *xv, Double_t *yv ) const ;
  Int_t GetTrackMC( AliHLTTPCCADisplayTmpHit *vHits, Int_t NHits );

  void DrawTPC();
  void DrawSlice( AliHLTTPCCATracker *slice, Bool_t DrawRows=0 ); 
  void DrawSliceOutTrack( Int_t itr, Int_t color=-1, Double_t width = -1  );
  void DrawSliceOutTrack( AliHLTTPCCATrackParam &t, Double_t Alpha, Int_t itr, Int_t color=-1, Double_t width = -1  );
  void DrawSliceTrack( Int_t itr, Int_t color=-1);
  Bool_t DrawTrack( AliHLTTPCCATrackParam t, Double_t Alpha, AliHLTTPCCADisplayTmpHit *vHits, 
		  Int_t NHits, Int_t color=-1, Double_t width=-1, Bool_t pPoint=0 );

  void DrawGBTrack( Int_t itr, Int_t color=-1, Double_t width=-1 );
  void DrawGBTrackFast( AliHLTTPCCAGBTracker &tracker, Int_t itr, Int_t color=-1 );
  Bool_t DrawTracklet( AliHLTTPCCATrackParam &track, Int_t *hitstore, Int_t color=-1, Double_t width=-1, Bool_t pPoint=0 );
 
  void DrawGBHit( AliHLTTPCCAGBTracker &tracker, Int_t iHit, Int_t color=-1, Double_t width =-1 );
  void DrawGBHits( AliHLTTPCCAGBTracker &tracker, Int_t color=-1, Double_t width =-1 );

  void DrawSliceHit( Int_t iRow,Int_t iHit, Int_t color=-1, Double_t width=-1 );
  void DrawSliceHits(Int_t color=-1, Double_t width=-1 );

#ifdef XXXX

  void DrawMergedHit( Int_t iRow, Int_t iHit, Int_t color=-1 );

  void DrawTrack( AliHLTTPCCATrack &track, Int_t color=-1, Bool_t DrawCells=1 );
  void DrawTrackletPoint( AliHLTTPCCATrackParam &t, Int_t color=-1 );
#endif // XXXX

  void SetSliceTransform( Double_t alpha );

  void SetSliceTransform( AliHLTTPCCATracker *slice );

  TCanvas *CanvasYX(){ return fYX; }
  TCanvas *CanvasZX(){ return fZX; }

 protected:

  TCanvas *fYX, *fZX;               // two views
  Bool_t fAsk;                      // flag to ask for the pressing key
  Bool_t fSliceView;               // switch between slice/TPC zoom
  AliHLTTPCCATracker *fSlice;      // current CA tracker, includes slice geometry
  AliHLTTPCCAGBTracker *fGB;      // the global tracker
  AliHLTTPCCAPerformance *fPerf; // Performance class (mc labels etc)
  Double_t fCos, fSin, fZMin, fZMax, fYMin, fYMax;// view parameters
  Double_t fSliceCos, fSliceSin;        // current slice angle
  Double_t fRInnerMin, fRInnerMax, fROuterMin, fROuterMax,fTPCZMin, fTPCZMax; // view parameters

  TArc fArc;       // parameters of drawing objects are copied from this members
  TLine fLine;     //!
  TPolyLine fPLine;//!
  TMarker fMarker; //!
  TBox fBox;       //!
  TCrown fCrown;   //!
  TLatex fLatex;   //!

  Bool_t fDrawOnlyRef; // draw only clusters from ref. mc tracks

};

#endif
