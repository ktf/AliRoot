#ifndef ALIOBJMATRIX_H
#define ALIOBJMATRIX_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

// $Id$

#include "TObject.h"
#include "TObjArray.h"
#include "TRefArray.h"

class AliObjMatrix : public TObject
{
 public:
  AliObjMatrix();                                             // Default constructor
  virtual ~AliObjMatrix();                                    // Default destructor
  virtual void Reset();                                       // Reset the whole matrix structure
  virtual void SetOwner(Int_t own=1);                         // Set the owner flag for the stored objects
  virtual Int_t GetOwner();                                   // Provide the owner flag for the stored objects
  virtual void SetSwapMode(Int_t swap=1);                     // Set the swap mode flag for this matrix
  virtual Int_t GetSwapMode();                                // Provide the swap mode flag for this matrix
  virtual void EnterObject(Int_t row,Int_t col,TObject* obj); // Enter an object into the matrix
  virtual void RemoveObject(Int_t row,Int_t col);             // Remove an object from the matrix
  virtual TObject* GetObject(Int_t row,Int_t col);            // Provide an object from the matrix
  virtual Int_t GetMaxRow();                                  // Provide the maximum row number index
  virtual Int_t GetMaxColumn();                               // Provide the maximum column number index
  virtual Int_t GetNobjects();                                // Provide the number of stored objects
  virtual TObject* GetObject(Int_t j);                        // Provide pointer to the j-th object
  virtual TObjArray* GetObjects();                            // Provide pointers of all stored onjects
 
 protected:
  TObjArray* fRows;    // Pointers to the various arrays representing the matrix rows
  Int_t fOwn;          // Flag to indicate whether the objects are owned by the matrix structure
  Int_t fSwap;         // Flag to indicate swapped mode for internal matrix storage
  Int_t fMaxrow;       // The maximum row number index
  Int_t fMaxcol;       // The maximum column number index
  TObjArray* fObjects; // Linear reference array for fast looping over the stored objects
 
 ClassDef(AliObjMatrix,2) // Handling of a matrix structure of objects.
};
#endif
