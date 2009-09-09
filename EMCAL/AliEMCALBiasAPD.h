#ifndef ALIEMCALBIASAPD_H
#define ALIEMCALBIASAPD_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id: $ */

#include <TObject.h>
#include "AliEMCALGeoParams.h"
class TString;
class TTree;

/*
  Objects of this class contain info on APD bias settings/voltages
*/

// ******* internal class definition *************
// 1 SuperModule's worth of info
class AliEMCALSuperModuleBiasAPD : public TObject {
 public:
  AliEMCALSuperModuleBiasAPD() : TObject(), // just init values
    fSuperModuleNum(0)
    {
      for (int icol=0; icol<AliEMCALGeoParams::fgkEMCALCols; icol++) {
	for (int irow=0; irow<AliEMCALGeoParams::fgkEMCALRows; irow++) {
	  fElecId[icol][irow] = 0;
	  fDAC[icol][irow] = 0;
	  fVoltage[icol][irow] = 0;
	}
      }
    }

 public:
  Int_t fSuperModuleNum; // SM index
  Int_t fElecId[AliEMCALGeoParams::fgkEMCALCols][AliEMCALGeoParams::fgkEMCALRows]; // ElectronicsIndex/Address - we keep this to help ensure that the column/row info matches with electronics indices
  Int_t fDAC[AliEMCALGeoParams::fgkEMCALCols][AliEMCALGeoParams::fgkEMCALRows]; // 0-0x3ff register
  Float_t fVoltage[AliEMCALGeoParams::fgkEMCALCols][AliEMCALGeoParams::fgkEMCALRows]; // 210 to ca 417 V. (function of DAC setting)

  ClassDef(AliEMCALSuperModuleBiasAPD, 1) // help class
};
// ******* end of internal class definition *************

class AliEMCALBiasAPD : public TObject {
public:
  AliEMCALBiasAPD();

  // Read and Write txt I/O methods are normally not used, but are useful for 
  // filling the object before it is saved in OCDB 
  void ReadTextBiasAPDInfo(Int_t nSM, const TString &txtFileName, Bool_t swapSides=kFALSE); // info file is for nSm=1 to 12 SuperModules
  void WriteTextBiasAPDInfo(const TString &txtFileName, Bool_t swapSides=kFALSE); // info file is for nSm=1 to 12 SuperModules
  void ReadRootBiasAPDInfo(const TString &rootFileName, Bool_t swapSides=kFALSE); // info file is for nSm=1 to 12 SuperModules
  void ReadTreeBiasAPDInfo(TTree *tree, Bool_t swapSides=kFALSE); // info file is for nSm=1 to 12 SuperModules
  void WriteRootBiasAPDInfo(const TString &rootFileName, Bool_t swapSides=kFALSE); // info file is for nSm=1 to 12 SuperModules

  virtual ~AliEMCALBiasAPD();

  // pointer to stored info.
  Int_t GetNSuperModule() const { return fNSuperModule; }; 
  AliEMCALSuperModuleBiasAPD * GetSuperModuleData() const { return fSuperModuleData; };

  // - via the index in the stored array:
  virtual AliEMCALSuperModuleBiasAPD GetSuperModuleBiasAPDId(Int_t smIndex) const;
  // - or via the actual SM number
  virtual AliEMCALSuperModuleBiasAPD GetSuperModuleBiasAPDNum(Int_t smNum) const;

protected:

  Int_t 	  fNSuperModule; // Number of supermodules.
  AliEMCALSuperModuleBiasAPD *fSuperModuleData; // SuperModule data

private:

  AliEMCALBiasAPD(const AliEMCALBiasAPD &);
  AliEMCALBiasAPD &operator = (const AliEMCALBiasAPD &);

  ClassDef(AliEMCALBiasAPD, 2) //BiasAPD data reader
};

#endif
