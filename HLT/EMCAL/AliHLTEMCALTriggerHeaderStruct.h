#ifndef ALIHLTEMCALTRIGGERHEADERSTRUCT_H
#define ALIHLTEMCALTRIGGERHEADERSTRUCT_H

struct AliHLTEMCALTriggerHeaderStruct {
  /** L1 thresholds from raw data */
  Int_t    fL1Threshold[4];
  /** L1 threshold components */
  Int_t    fL1V0[2];
  /** Validation flag for L1 data */
  Int_t    fL1FrameMask;
  /** Number of fastors */
  Int_t    fNfastor;
};
#endif
