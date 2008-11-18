#ifndef ALITRDARRAYDICTIONARY_H
#define ALITRDARRAYDICTIONARY_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. * 
 * See cxx source for full Copyright notice */ 

/* $Id: AliTRDarrayDictionary.h 23387 2008-01-17 17:25:16Z cblume $ */

///////////////////////////////////////////////////
//                                               //
// Container Class for Dictionary Info           //
//                                               //
///////////////////////////////////////////////////

#include <TObject.h>

class AliTRDarrayDictionary: public TObject
{

 public:

  AliTRDarrayDictionary();
  AliTRDarrayDictionary(Int_t nrow, Int_t ncol, Int_t ntime);
  AliTRDarrayDictionary(const AliTRDarrayDictionary &a);
  ~AliTRDarrayDictionary();
  AliTRDarrayDictionary &operator=(const AliTRDarrayDictionary &a);

  void  Allocate(Int_t nrow, Int_t ncol, Int_t ntime);
  void  SetNdet(Int_t ndet) {fNdet=ndet;};  
  Int_t GetNdet()  const {return fNdet;};
  void  SetData(Int_t nrow, Int_t ncol, Int_t ntime, Int_t value)
               { fDictionary[(nrow*fNcol+ncol)*fNtime+ntime]=value; };
  Int_t GetData(Int_t nrow, Int_t ncol, Int_t ntime) const
               { return fDictionary[(nrow*fNcol+ncol)*fNtime+ntime]; };
  Int_t GetDim() const {return fNDdim;};
  void  Compress();
  void  Expand();

 protected:

  Int_t   fNdet;        //ID number of the chamber
  Int_t   fNrow;        //Number of rows
  Int_t   fNcol;        //Number of columns
  Int_t   fNtime;       //Number of time bins
  Int_t   fNDdim;       //Dimension of the Dictionary array
  Int_t*  fDictionary;  //[fNDdim]  //Pointer to integers array


  ClassDef(AliTRDarrayDictionary,1) //Dictionary container class
    
};
#endif
