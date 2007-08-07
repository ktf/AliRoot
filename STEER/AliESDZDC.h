// -*- mode: C++ -*- 
#ifndef ALIESDZDC_H
#define ALIESDZDC_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//-------------------------------------------------------------------------
//                      Implementation of   Class AliESDZDC
//   This is a class that summarizes the ZDC data
//   for the ESD   
//   Origin: Christian Klein-Boesing, CERN, Christian.Klein-Boesing@cern.ch 
//-------------------------------------------------------------------------

#include <TObject.h>


class AliESDZDC: public TObject {
public:
  AliESDZDC();
  AliESDZDC(const AliESDZDC& zdc);
  AliESDZDC& operator=(const AliESDZDC& zdc);

  Double_t GetZDCN1Energy() const {return fZDCN1Energy;}
  Double_t GetZDCP1Energy() const {return fZDCP1Energy;}
  Double_t GetZDCN2Energy() const {return fZDCN2Energy;}
  Double_t GetZDCP2Energy() const {return fZDCP2Energy;}
  Double_t GetZDCEMEnergy() const {return fZDCEMEnergy;}
  Int_t    GetZDCParticipants() const {return fZDCParticipants;}
  void     SetZDC(Double_t n1Energy, Double_t p1Energy, Double_t emEnergy,
		  Double_t n2Energy, Double_t p2Energy, Int_t participants) 
   {fZDCN1Energy=n1Energy; fZDCP1Energy=p1Energy; fZDCEMEnergy=emEnergy;
    fZDCN2Energy=n2Energy; fZDCP2Energy=p2Energy; fZDCParticipants=participants;}

  void    Reset();
  void    Print(const Option_t *opt=0) const;

private:

  Double32_t   fZDCN1Energy;      // reconstructed energy in the neutron ZDC
  Double32_t   fZDCP1Energy;      // reconstructed energy in the proton ZDC
  Double32_t   fZDCN2Energy;      // reconstructed energy in the neutron ZDC
  Double32_t   fZDCP2Energy;      // reconstructed energy in the proton ZDC
  Double32_t   fZDCEMEnergy;      // reconstructed energy in the electromagnetic ZDC
  Int_t        fZDCParticipants;  // number of participants estimated by the ZDC

  ClassDef(AliESDZDC,2)
};

#endif
