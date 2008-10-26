#ifndef ALITRDCHAMBERTIMEBIN_H
#define ALITRDCHAMBERTIMEBIN_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
* See cxx source for full Copyright notice                               */

/* $Id: AliTRDchamberTimeBin.h 22646 2007-11-29 18:13:40Z cblume $ */

////////////////////////////////////////////////////////////////////////////
//                                                                        //
//  A TRD layer in a single stack                                         //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ALITRDCLUSTER_H
#include "AliTRDcluster.h"
#endif

class AliTRDReconstructor;
class AliTRDchamberTimeBin : public TObject
{
public:
  enum{
    kMaxClustersLayer = 150
    ,kMaxRows = 16
  };

  AliTRDchamberTimeBin(Int_t plane=-1, Int_t stack=-1, Int_t sector=-1, Double_t z0=-1., Double_t zLength=-1.);
  //AliTRDchamberTimeBin(const AliTRDpropagationLayer &layer, Double_t z0, Double_t zLength, UChar_t stackNr);
  //AliTRDchamberTimeBin(const AliTRDpropagationLayer &layer);
  AliTRDchamberTimeBin(const AliTRDchamberTimeBin &layer);
  ~AliTRDchamberTimeBin();
  //AliTRDchamberTimeBin   &operator=(const AliTRDpropagationLayer &myLayer);
  operator Int_t() const                                        { return fN;                    }
  AliTRDchamberTimeBin   &operator=(const AliTRDchamberTimeBin &myLayer);
  AliTRDcluster      *operator[](const Int_t i) const {
    return ((i < fN) && (i >= 0)) ? fClusters[i] : 0x0;
  }

  void           BuildIndices(Int_t iter = 0);
  void           BuildCond(AliTRDcluster *cl, Double_t *cond, UChar_t Layer, Double_t theta=0., Double_t phi=0.);
  void           Clear(const Option_t *opt = 0x0);
  AliTRDcluster* GetCluster(Int_t index) const {return index < fN && index >= 0 ? fClusters[index] : 0x0;}
  Int_t          GetGlobalIndex(Int_t index) const {return ((index < fN) && (index >= 0)) ? fIndex[index] : 0; }
  void           GetClusters(Double_t *cond, Int_t *index, Int_t& ncl, Int_t BufferSize = kMaxClustersLayer);
  AliTRDcluster* GetNearestCluster(Double_t *cond);
  Double_t       GetX()                            const {
  return fX;      }
  Double_t       GetZ0()                           const { return fZ0;     }
  Double_t       GetDZ0()                          const { return fZLength;}
  Int_t          GetNClusters()                    const { return fN; }
  Int_t          GetPlane()                        const { return fPlane;  }
  Int_t          GetStack()                        const { return fStack;  }
  Int_t          GetSector()                       const { return fSector; }
  void           InsertCluster(AliTRDcluster *c, UInt_t index);

  Bool_t         IsT0() const {return TestBit(1);}
  
  void           Print(Option_t *opt=0x0) const;
  Int_t          SearchNearestCluster(Double_t y, Double_t z, Double_t Roady, Double_t Roadz) const;
  void           SetRange(Float_t z0, Float_t zLength);
  void           SetNRows(Int_t nRows){ fNRows = nRows; }
  void           SetPlane(Int_t plane){ fPlane = plane; }
  void           SetReconstructor(const AliTRDReconstructor *rec) {fReconstructor = rec;}
  void           SetStack(Int_t stack){ fStack = stack; }
  void           SetSector(Int_t sector){ fSector = sector; }
  void           SetOwner(Bool_t own = kTRUE) {fOwner = own;}
  void           SetT0(Bool_t set=kTRUE) {SetBit(1, set);}
  void           SetX(Double_t x) {fX = x;}
private:
  void           Copy(TObject &o) const;
  Int_t          Find(Float_t y) const;
  Int_t          FindYPosition(Double_t y, UChar_t z, Int_t nClusters) const;
  Int_t          FindNearestYCluster(Double_t y, UChar_t z) const;

private:
  const AliTRDReconstructor *fReconstructor; //
  Bool_t        fOwner;               //  owner of the clusters
  Char_t        fPlane;               // Plane number
  Char_t        fStack;               //  stack number in supermodule
  Char_t        fSector;              // Sector mumber
  Char_t        fNRows;               //  number of pad rows in the chamber
  UChar_t       fPositions[kMaxRows]; //  starting index of clusters in pad row 
  Int_t         fN;                   // number of clusters
  AliTRDcluster *fClusters[kMaxClustersLayer];            //Array of pointers to clusters
  UInt_t        fIndex[kMaxClustersLayer];                //Array of cluster indexes
  Double_t      fX;                   //  radial position of tb
  
  // obsolete !!
  Double_t      fZ0;                  //  starting position of the layer in Z direction
  Double_t      fZLength;             //  length of the layer in Z direction
  
  ClassDef(AliTRDchamberTimeBin, 2)   //  tracking propagation layer for one time bin in chamber

};


#endif	// ALITRDCHAMBERTIMEBIN_H

