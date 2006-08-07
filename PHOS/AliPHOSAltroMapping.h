#ifndef ALIPHOSALTROMAPPING_H
#define ALIPHOSALTROMAPPING_H
/* Copyright(c) 1998-2003, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//////////////////////////////////////////////////////////
// Class used to setup the mapping of hardware adresses //
// in ALTRO to PHOS cells (column and row index +       //
// low/high gain.                                       //
// The mapping is defined in an external mapping files  //
// separately. The class derives from the base altro    //
// mapping class defined in the RAW package.            //
//////////////////////////////////////////////////////////

#include "AliAltroMapping.h"

class AliPHOSAltroMapping: public AliAltroMapping {
 public:
  AliPHOSAltroMapping(const char *mappingFile);
  virtual ~AliPHOSAltroMapping();

  AliPHOSAltroMapping(const AliPHOSAltroMapping& mapping);
  AliPHOSAltroMapping& operator = (const AliPHOSAltroMapping& mapping);

  // In case of PHOS the relevant segmentation is row-column-gain
  virtual Int_t GetHWAddress(Int_t row, Int_t column, Int_t gain) const;
  // Get Row (not pad-row as in the base class)
  virtual Int_t GetPadRow(Int_t hwAddress) const;
  // Get Column (not pad as in the base class)
  virtual Int_t GetPad(Int_t hwAddress) const;
  // Get Gain (0/1) (not sector as in the base class)
  virtual Int_t GetSector(Int_t hwAddress) const;

 protected:
  virtual Bool_t ReadMapping();
  virtual void   DeleteMappingArrays();

  Int_t     fMinRow;        // Minimum row index
  Int_t     fMaxRow;        // Maximum row index
  Int_t     fMinCol;        // Minimum column index
  Int_t     fMaxCol;        // Maximum column index
  Short_t **fMapping;       // Array which connects hardware adresses to row and column indeces
  Short_t **fInvMappingLow; // Inverse of fMapping (Low gain)
  Short_t **fInvMappingHigh;// Inverse of fMapping (High gain)

  ClassDef(AliPHOSAltroMapping,1)  // Altro mapping handler class
};

#endif
