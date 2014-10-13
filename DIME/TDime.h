#ifndef TDIME_H
#define TDIME_H


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TDime                                                                //
//                                                                      //
// This class implements an interface to the DIME event generator.      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TGenerator
#include "TGenerator.h"
#endif
class TObjArray;

class TDime : public TGenerator {
public:
   
   TDime();
   TDime(Double_t efrm);
   virtual            ~TDime();
   virtual void        Initialize();
   virtual void        GenerateEvent();
   virtual Int_t       ImportParticles(TClonesArray *particles, Option_t *option="");
   virtual TObjArray*  ImportParticles(Option_t *option="");
   //Parameters for the generation:
   virtual void        SetEnergyCMS(Float_t efrm) {fEfrm = efrm;}
   virtual Float_t     GetEnergyCMS() const {return fEfrm;}
   protected:
   Float_t      fEfrm;     // Energy in the centre of mass (CMS) or lab-frame (LAB)
   ClassDef(TDime,1)  //Interface to Dime Event Generator
};

#endif







