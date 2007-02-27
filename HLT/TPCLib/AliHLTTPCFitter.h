// @(#) $Id$
// Original: AliHLTFitter.h,v 1.7 2004/07/05 09:02:18 loizides 

#ifndef ALIHLTTPCFITTER_H
#define ALIHLTTPCFITTER_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/** @file   AliHLTTPCFitter.h
    @author Anders Vestbo, maintained by Matthias Richter
    @date   
    @brief  Fit class HLT for helix
*/

class AliHLTTPCTrack;
class AliHLTTPCVertex;
class AliHLTTPCSpacePointData;

/** 
 * @class AliHLTTPCFitter
 * Fit class HLT for helix
 */
class AliHLTTPCFitter {

  public:
  AliHLTTPCFitter();
  /** not a valid copy constructor, defined according to effective C++ style */
  AliHLTTPCFitter(const AliHLTTPCFitter& src);
  /** not a valid assignment op, but defined according to effective C++ style */
  AliHLTTPCFitter& operator=(const AliHLTTPCFitter& src);
  AliHLTTPCFitter(AliHLTTPCVertex *vertex,Bool_t vertexconstraint=kTRUE);
  virtual ~AliHLTTPCFitter();
  
  void LoadClusters(Char_t *path,Int_t event=0,Bool_t sp=kFALSE);
  void SortTrackClusters(AliHLTTPCTrack *track) const;
  Int_t FitHelix(AliHLTTPCTrack *track);
  Int_t FitCircle();
  Int_t FitLine();
  void NoVertex() {fVertexConstraint=kFALSE;}
 
 private:
  AliHLTTPCTrack *fTrack; //!                    actual track
  AliHLTTPCVertex *fVertex; //!                  vertex info
  Bool_t fVertexConstraint; //               include vertex constraint
  AliHLTTPCSpacePointData *fClusters[36][6]; //! clusters
  UInt_t fNcl[36][6]; //                     cluster numbers
 
  ClassDef(AliHLTTPCFitter,1) //HLT fit class
};

#endif
