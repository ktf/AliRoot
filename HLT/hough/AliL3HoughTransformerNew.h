// @(#) $Id$

#ifndef ALIL3_HOUGHTRANSFORMERNEW
#define ALIL3_HOUGHTRANSFORMERNEW

#include "AliL3RootTypes.h"
#include "AliL3HoughTransformer.h"

#include <TH3.h>

class AliL3TrackArray;
class AliL3HoughTrack;
 
class AliL3HoughTransformerNew : public AliL3HoughTransformer {
  
 private:
  
  TH3 *fParamSpace;

 public:
  AliL3HoughTransformerNew(); 
  AliL3HoughTransformerNew(Int_t slice,Int_t patch,Int_t netasegments);
  virtual ~AliL3HoughTransformerNew();
  
  void Reset();
  void CreateHistograms(Int_t nxbins,Float_t xlow,Float_t xup,
			Int_t nybins,Float_t ylow,Float_t yup,
			Int_t nzbins,Float_t zlow,Float_t zup);
  void TransformLine(Int_t *rowrange,Float_t *phirange);
  void TransformLineC(Int_t *rowrange,Float_t *phirange);
  
  TH3 *GetHistogram() {return fParamSpace;}
  
  ClassDef(AliL3HoughTransformerNew,1) //Normal Hough transformation class

};

#endif




