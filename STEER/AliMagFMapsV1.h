#ifndef ALIMAGFMAPSV1_H
#define ALIMAGFMAPSV1_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

//
// Magnetic field composed by 4 maps: the L3 magnet (inside and outside measured region), extended region, and
// dipole magnet
// Author: Andreas Morsch <andreas.morsch@cern.ch>
//

#include "AliMagFMaps.h"
class AliMagFCheb;

class AliMagFMapsV1 : public AliMagFMaps
{
public:
    AliMagFMapsV1();
    AliMagFMapsV1(const char *name, const char *title, Int_t integ,
		Float_t factor, Float_t fmax, Int_t map = k2kG,
		Int_t l3 = 1);
    virtual ~AliMagFMapsV1();
    virtual void    Field(Float_t *x, Float_t *b) const;
    virtual Float_t SolenoidField() const;
 private:
    AliMagFMapsV1(const AliMagFMapsV1& maps);             
    AliMagFMapsV1& operator=(const AliMagFMapsV1& maps);
 protected:
    AliMagFCheb* fMeasuredMap;    //! Measured part of the field map
    ClassDef(AliMagFMapsV1, 0)    // Class for all Alice MagField using three Maps with Constant Mesh + measured L3 region
};

#endif
