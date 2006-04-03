#ifndef ALITPCROC_H
#define ALITPCROC_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id: AliTPCROC.h,v */

//////////////////////////////////////////////////
//                                              //
//  TPC geometry class for   ROC                //
//                                              //
//////////////////////////////////////////////////

#include <TObject.h>

//_____________________________________________________________________________
class AliTPCROC : public TObject {
 public:
  static AliTPCROC* Instance();
  AliTPCROC();
  AliTPCROC(const AliTPCROC &roc);
  void Init(); 
  virtual           ~AliTPCROC();

  //
  //    numbering
  UInt_t GetNSectors() const          { return fNSectorsAll;}
  UInt_t GetNRows(UInt_t sector) const { return (sector<fNSectors[1]) ? fNRows[0]:fNRows[1];}
  UInt_t GetNChannels(UInt_t sector) const { return (sector<fNSectors[1]) ? fNChannels[0]:fNChannels[1];}
  UInt_t GetNPads(UInt_t sector,UInt_t row) const { return (sector<fNSectors[1]) ? fNPads[0][row]:fNPads[1][row];}
  const UInt_t * GetRowIndexes(UInt_t sector) const {return (sector<fNSectors[1]) ? fRowPosIndex[0]:fRowPosIndex[1];}  
  //
  //get sector parameters
  //
  Float_t  GetInnerRadiusLow() const {return fInnerRadiusLow;}
  Float_t  GetInnerRadiusUp() const {return fInnerRadiusUp;} 
  Float_t  GetOuterRadiusLow() const {return fOuterRadiusLow;} 
  Float_t  GetOuterRadiusUp() const {return fOuterRadiusUp;} 
  Float_t  GetInnerFrameSpace() const {return fInnerFrameSpace;}
  Float_t  GetOuterFrameSpace() const {return fOuterFrameSpace;}
  Float_t  GetInnerWireMount() const {return fInnerWireMount;}
  Float_t  GetOuterWireMount() const {return fOuterWireMount;}
  Float_t  GetInnerAngle() const {return fInnerAngle;}
  Float_t  GetOuterAngle() const {return fOuterAngle;}
  UInt_t    GetNInnerSector() const {return fNSectors[0];}
  UInt_t    GetNOuterSector() const {return fNSectors[1];}
  UInt_t    GetNSector() const {return fNSectorsAll;}
  Float_t  GetZLength() const {return fZLength;}
  //

 protected:
  //
  //     number of pads
  //
  void   SetGeometry();    // set geometry parameters
  UInt_t  fNSectorsAll;     // number of sectors
  UInt_t  fNSectors[2];     // number of sectors - inner outer
  UInt_t  fNRows[2];        // number of row     - inner outer
  UInt_t  fNChannels[2];    // total number of pads   - inner sector - outer sector
  UInt_t *fNPads[2];        // number of pads in row  - inner - outer      
  UInt_t *fRowPosIndex[2];  // index array            - inner - outer
  //
  //
  //---------------------------------------------------------------------
  //   ALICE TPC sector geometry
  //--------------------------------------------------------------------  
  Float_t fInnerRadiusLow;    // lower radius of inner sector-IP
  Float_t fInnerRadiusUp;     // upper radius of inner  sector-IP
  Float_t fOuterRadiusUp;     // upper radius of outer  sector-IP
  Float_t fOuterRadiusLow;    // lower radius of outer sector-IP
  Float_t fInnerFrameSpace;   //space for inner frame in the phi direction 
  Float_t fOuterFrameSpace;   //space for outer frame in the phi direction 
  Float_t fInnerWireMount;    //space for wire mount, inner sector
  Float_t fOuterWireMount;    //space for wire mount, outer sector
  Float_t fZLength;           //length of the drift region of the TPC
  Float_t fInnerAngle;        //angular coverage
  Float_t fOuterAngle;        //angular coverage
  //
  //---------------------------------------------------------------------
  //   ALICE TPC wires  geometry - for GEM we can consider that it is gating  
  //--------------------------------------------------------------------
  UInt_t   fNInnerWiresPerPad; //Number of wires per pad
  Float_t fInnerWWPitch;      //pitch between wires  in inner sector     - calculated
  UInt_t   fInnerDummyWire;    //number of wires without pad readout
  Float_t fInnerOffWire;      //oofset of first wire to the begining of the sector
  Float_t fRInnerFirstWire;   //position of the first wire                -calculated
  Float_t fRInnerLastWire;    //position of the last wire                 -calculated
  Float_t fLastWireUp1;     //position of the last wire in outer1 sector
  UInt_t   fNOuter1WiresPerPad; //Number of wires per pad
  UInt_t   fNOuter2WiresPerPad; // Number of wires per pad
  Float_t fOuterWWPitch;      //pitch between wires in outer sector      -calculated
  UInt_t   fOuterDummyWire;    //number of wires without pad readout
  Float_t fOuterOffWire;      //oofset of first wire to the begining of the sector
  Float_t fROuterFirstWire;   //position of the first wire                -calulated
  Float_t fROuterLastWire;    //position of the last wire                 -calculated 
  //---------------------------------------------------------------------
  //   ALICE TPC pad parameters
  //--------------------------------------------------------------------
  Float_t   fInnerPadPitchLength;    //Inner pad pitch length
  Float_t   fInnerPadPitchWidth;     //Inner pad pitch width
  Float_t   fInnerPadLength;         //Inner pad  length
  Float_t   fInnerPadWidth;          //Inner pad  width
  Float_t   fOuter1PadPitchLength;    //Outer pad pitch length
  Float_t   fOuter2PadPitchLength;    //Outer pad pitch length
  Float_t   fOuterPadPitchWidth;     //Outer pad pitch width
  Float_t   fOuter1PadLength;         //Outer pad  length
  Float_t   fOuter2PadLength;         //Outer pad length
  Float_t   fOuterPadWidth;          //Outer pad  width
  // 
  UInt_t     fNRowLow;           //number of pad rows per low sector        -set
  UInt_t     fNRowUp1;            //number of short pad rows per sector up  -set
  UInt_t     fNRowUp2;            //number of long pad rows per sector up   -set
  UInt_t     fNRowUp;            //number of pad rows per sector up     -calculated
  UInt_t     fNtRows;            //total number of rows in TPC          -calculated
  Float_t   fPadRowLow[600]; //Lower sector, pad row radii          -calculated
  Float_t   fPadRowUp[600];  //Upper sector, pad row radii          -calculated 
  UInt_t     fNPadsLow[600];  //Lower sector, number of pads per row -calculated
  UInt_t     fNPadsUp[600];   //Upper sector, number of pads per row -calculated
  Float_t   fYInner[600];     //Inner sector, wire-length
  Float_t   fYOuter[600];     //Outer sector, wire-length   
 protected:
  static AliTPCROC*   fgInstance; // Instance of this class (singleton implementation)
  ClassDef(AliTPCROC,1)    //  TPC ROC class
};

#endif
