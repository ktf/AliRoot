#include <Riostream.h>
#include "TVirtualMCApplication.h"

#include "TFluka.h"

#include "Fdimpar.h"  //(DIMPAR) fluka include
#include "Ftrackr.h"  //(TRACKR) fluka common
#ifndef WIN32
# define usdraw usdraw_
#else
# define usdraw USDRAW
#endif
extern "C" {
void usdraw(Int_t& icode, Int_t& mreg, 
            Double_t& xsco, Double_t& ysco, Double_t& zsco)
{
  TFluka *fluka = (TFluka*)gMC;
  Int_t verbosityLevel = fluka->GetVerbosityLevel();
  Bool_t debug = (verbosityLevel>=3)?kTRUE:kFALSE;
  fluka->SetCaller(6);
  fluka->SetIcode(icode);
  fluka->SetMreg(mreg);
  fluka->SetXsco(xsco);
  fluka->SetYsco(ysco);
  fluka->SetZsco(zsco);
  if (debug) printf("USDRAW: Number of track segments:%d %d\n", TRACKR.ntrack, TRACKR.mtrack);

  (TVirtualMCApplication::Instance())->Stepping();
  fluka->SetTrackIsNew(kFALSE);
  
} // end of usdraw
} // end of extern "C"

