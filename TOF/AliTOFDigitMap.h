#ifndef ALITOFDIGITMAP_H
#define ALITOFDIGITMAP_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */


//////////////////////////////////////////////////////////////
//                                                          //
//   AliTOFDigitMap class                                   //
//                                                          //
//  digitmap enables fast check if the pad was already hit  //
//                                                          //
//   Author: F. Pierella | pierella@bo.infn.it              //
//                                                          //
// Modified by A. De Caro | decaro@sa.infn.it               //
//                                                          //
//////////////////////////////////////////////////////////////

#include "TObject.h"

#include "AliHitMap.h"

class AliTOFGeometry;

class AliTOFDigitMap : public TObject
{
 public:
    AliTOFDigitMap();
    AliTOFDigitMap(const AliTOFDigitMap & digitMap);
    
    virtual ~AliTOFDigitMap();

    // Clear the digit map
    virtual void  Clear(const Option_t* opt = "");
    // Add a single digit
    void  AddDigit(Int_t *vol, Int_t idigit);

    // Get index of digit in the cell labelled by vol
    Int_t  GetDigitIndex(Int_t *vol, Int_t index) const;
    // Get indices of digits in the cell labelled by vol
    void   GetDigitIndex(Int_t *vol, Int_t *index) const;

    // Test digit status
    virtual FlagType TestDigit(Int_t *vol) const;

    // Assignment operator
    AliTOFDigitMap& operator = (const AliTOFDigitMap& rhs);
    
    Int_t GetFilledCellNumber() const;

 private:
    // Check index
    Int_t CheckedIndex(Int_t *vol) const;
 private:
    enum {
      kMaxDigitsPerPad = 3
    };

    Int_t fNSector;                       // Number of sectors
    Int_t fNplate;                        // Number of plates
    Int_t fNstrip;                        // Maximum number of strips
    Int_t fNpx;                           // Number of pads in x
    Int_t fNpz;                           // Number of pads in z

    Int_t fMaxIndex;                      // maximum index in hit map
    Int_t **fDigitMap;                    // ! [fMaxIndex][kMaxDigitsPerPad]

    AliTOFGeometry *fTOFGeometry;         // Pointer to the TOF geometry

    ClassDef(AliTOFDigitMap,1) // Implements DigitMap as a 1-dim array
};
#endif	
