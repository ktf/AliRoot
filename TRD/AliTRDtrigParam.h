#ifndef ALITRDTRIGPARAM_H
#define ALITRDTRIGPARAM_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  TRD trigger parameters class                                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <TNamed.h>

class AliTRDtrigParam : public TNamed {

 public:

  AliTRDtrigParam();
  AliTRDtrigParam(const Text_t* name, const Text_t* title);
  AliTRDtrigParam(const AliTRDtrigParam &p);   
  virtual ~AliTRDtrigParam();
  AliTRDtrigParam &operator=(const AliTRDtrigParam &p); 

  virtual void     Copy(TObject &p) const;
  virtual void     Init();

  void    SetTimeRange(const Int_t time1, const Int_t time2) { fTime1 = time1; fTime2 = time2; };
  Int_t   GetTime1()                                   const { return fTime1; };
  Int_t   GetTime2()                                   const { return fTime2; };
  void    SetClusThr(const Float_t clth)                     { fClusThr = clth; };
  void    SetPadThr(const Float_t path)                      { fPadThr = path;  };
  Float_t GetClusThr()                                 const { return fClusThr; };
  Float_t GetPadThr()                                  const { return fPadThr;  };
  void    SetSum10(const Int_t sum)                          { fSum10 = sum; };
  void    SetSum12(const Int_t sum)                          { fSum12 = sum; };
  Int_t   GetSum10()                                   const { return fSum10; };
  Int_t   GetSum12()                                   const { return fSum12; };

  void    SetTailCancelation(Int_t tcOn = 0)               { fTCOn = tcOn; };
  void    SetNexponential(Int_t nexp = 1)                  { fTCnexp = nexp; };
  void    SetFilterType(Int_t ftype = 0)                   { fFilterType = ftype; };
  void    SetFilterParam(Float_t r1, Float_t r2, Float_t c1, Float_t c2, Float_t ped) 
    { fR1 = r1; fR2 = r2; fC1 = c1; fC2 = c2; fPedestal = ped; };

  Int_t   GetTailCancelation()                       const { return fTCOn; };
  Int_t   GetNexponential()                          const { return fTCnexp; };
  Int_t   GetFilterType()                            const { return fFilterType; };
  void    GetFilterParam(Float_t &r1, Float_t &r2, Float_t &c1, Float_t &c2, Float_t &ped) const { 
    r1 = fR1; r2 = fR2; c1 = fC1; c2 = fC2; ped = fPedestal; 
  };

  void    SetADCnoise(const Float_t adcn)                  { fADCnoise = adcn; };
  Float_t GetADCnoise()                                    { return fADCnoise; };

  void    SetDebugLevel(const Int_t deb) { fDebug = deb;  };
  Int_t   GetDebugLevel()                { return fDebug; };

  void    SetDeltaY(Float_t dy) { fDeltaY = dy; };
  Float_t GetDeltaY()           { return fDeltaY; };
  void    SetDeltaS(Float_t ds) { fDeltaS = ds; };
  Float_t GetDeltaS()           { return fDeltaS; };

  Float_t GetXprojPlane() { return fXprojPlane; };

  void    SetField(Float_t b) { fField = b; };
  Float_t GetField() { return fField; };

  void    SetLtuPtCut(Float_t ptcut) { fLtuPtCut = ptcut; };
  Float_t GetLtuPtCut() { return fLtuPtCut; };

  void    SetGtuPtCut(Float_t ptcut) { fGtuPtCut = ptcut; };
  Float_t GetGtuPtCut() { return fGtuPtCut; };

 protected:

  Int_t    fDebug;                         // debugging flag

  Int_t    fTime1;                         // first time bin for tracking (incl.)
  Int_t    fTime2;                         // last  time bin for tracking (incl.)
  Float_t  fClusThr;                       // cluster threshold
  Float_t  fPadThr;                        // pad threshold
  Int_t    fSum10;                         // MCM CreateSeeds: Min_Thr_Left_Neighbour
  Int_t    fSum12;                         // MCM CreateSeeds: Min_Sum_From_Two_Neighbours
  Int_t    fTCOn;                          // tail cancelation flag
  Int_t    fTCnexp;                        // number of exp in filter
  Int_t    fFilterType;                    // filter type (0=A - analog, 1=D - digital)
  Float_t  fR1;                            // filter parameters (1 = long, 2 = short component)
  Float_t  fR2;                            //
  Float_t  fC1;                            //
  Float_t  fC2;                            //
  Float_t  fPedestal;                      //
  Float_t  fADCnoise;                      // ADC noise (not contained in the digitizer)

  Float_t  fDeltaY;                        // Y (offset) matching window in the GTU
  Float_t  fDeltaS;                        // Slope matching window in the GTU

  Float_t  fXprojPlane;                    // Projection plane (X) for GTU matching

  Float_t  fLtuPtCut;                      // Local pt cut
  Float_t  fGtuPtCut;                      // Global pt cut

  Float_t  fField;                         // Magnetic field

  ClassDef(AliTRDtrigParam,1)                  // TRD trigger parameter class

};

#endif
