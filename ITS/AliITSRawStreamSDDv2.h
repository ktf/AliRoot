#ifndef ALIITSRAWSTREAMSDDV2_H
#define ALIITSRAWSTREAMSDDV2_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

///////////////////////////////////////////////////////////////////////////////
///
/// This class provides access to ITS SDD digits in test beam raw data.
///
///////////////////////////////////////////////////////////////////////////////

#include "AliITSRawStreamSDD.h"

class AliRawReader;


class AliITSRawStreamSDDv2: public AliITSRawStreamSDD {
  public :
    AliITSRawStreamSDDv2(AliRawReader* rawReader);
    virtual ~AliITSRawStreamSDDv2() {};

    virtual Bool_t   Next();

  private :
 

    ClassDef(AliITSRawStreamSDDv2, 1) // class for reading ITS SDD raw digits
};

#endif
