#ifndef ALIGENHBTOSL_H
#define ALIGENHBTOSL_H
//__________________________________________________________
/////////////////////////////////////////////////////////////
//                                                         //
//  class AliGenHBTosl                                     //
//                                                         //
//  Genarator simulating particle correlations             //
//                                                         //
//  The main idea of the generator is to produce particles //
//  according to some distribution of two particle         //
//  property. In HBT they are qout,qsie and qlong.         //
//  In order to be able to generate signal that produces   //
//  given two particle correlation background must be      //
//  known before in order to produce the shape of signal   //
//  to randomize given distribution from.                  //
//                                                         //
//  The generator works as follows:                        //
//  1. Coarse Background (fQCoarseBackground) is generated //
//     ade  from the particles                             //
//     given by the external generator (variable           //
//     fGenerator) by the mixing technique.                //
//  2. Coarse signal is prduced by multiplying Coarse      //
//     background by a required function                   //
//     See method FillCoarseSignal                         //
//  3. Signal is randomized out of the coarse signal       //
//     histogram (two particle property). First particle   //
//     is taken from the external generator, and the       //
//     second one is CALCULATED on the basis of the first  //
//     one and the two particle property (qout,qside,qlong)//
//     Background is made by the mixing out of the         //
//     genereted signal events.                            //
//     This step is cotinued up to the moment signal       //
//     histogram has enough statistics (data member        //
//     fMinFill)                                           //
//     See method StartSignalPass1()                       //
//  4. chi is calculated for each bin (chiarray variqable) // 
//     (not the chi2 because sign is important)            //
//     Two particle prioperty                              //
//     (qout,qside,qlong) is chosen at the points that     //
//     chi is the smallest. First particle is taken from   //
//     the the external generator (fGenerator) and second's /
//     momenta are caclulated out of their momenta and     //
//     (qout,qside,qlong). Background is updated           //
//     continuesely for all the events. This step is       //
//     continued until stability conditions are fullfiled  //
//     or maximum number of iteration is reached.          //
//  5. The same as step 4 but events are stored.           //
//                                                         //
////////////////////////////////////////////////////////////

#include "AliGenerator.h"

class TH3D;
class AliStack;
class TParticle;
class TVector;

class AliGenHBTosl: public AliGenerator
{
 public:
   AliGenHBTosl();
   AliGenHBTosl(Int_t n,Int_t pid = 211);
   virtual ~AliGenHBTosl();

   void      Init();
   void      Generate();
   
   void      SetGenerator(AliGenerator* gen){fGenerator = gen;}
   void      SetDebug(Int_t debug){fDebug = debug;}
   Int_t    GetDebug() const {return fDebug;}

   void      Rotate(TVector3& relvector, TVector3& vector);
   Double_t  Rotate(Double_t x,Double_t y,Double_t z);
   void      SetSamplePhiRange(Float_t min,Float_t max){fSamplePhiMin = min; fSamplePhiMax = max;}

   Int_t GetThreeD(TParticle* first,TParticle* second, Double_t qout, Double_t qside, Double_t qlong);
      
   
 protected:

   void GetOneD(TParticle* first, TParticle* second,Double_t qinv);
   
   void FillCoarse();
   void FillCoarseSignal();
   void StartSignal();
   void StartSignalPass1();
   void Mix(TList* eventbuffer,TH3D* denominator,TH3D* denominator2);
   void Mix(AliStack* stack, TH3D* numerator, TH3D* numerator2);
   Double_t GetQInv(TParticle* f, TParticle* s);
   void     GetQOutQSideQLong(TParticle* f, TParticle* s,Double_t& out, Double_t& side, Double_t& lon);
   Double_t GetQInvCorrTheorValue(Double_t qinv) const;
   Double_t GetQOutQSideQLongCorrTheorValue(Double_t& out, Double_t& side, Double_t& lon) const;
   
   Double_t Scale(TH3D* num,TH3D* den);
   void SetTrack(TParticle* p, Int_t& ntr);
   void SetTrack(TParticle* p, Int_t& ntr, AliStack* stack);
   
   AliStack* RotateStack();
   void SwapGeneratingHistograms();
   void     TestCoarseSignal();
   
   Bool_t CheckParticle(TParticle* p, TParticle* aupair,AliStack* stack);
 private:
  TH3D*    fQCoarseBackground;//Initial Background
  TH3D*    fQCoarseSignal;//
  TH3D*    fQSignal;
  TH3D*    fQBackground;

  TH3D*    fQSecondSignal;
  TH3D*    fQSecondBackground;
  
  Float_t  fQRange;
  Int_t    fQNBins;
  AliGenerator* fGenerator;
  
  TList*   fStackBuffer;
  Int_t    fBufferSize;
  Int_t    fNBinsToScale;
  Int_t    fDebug;
  Bool_t   fSignalShapeCreated;
  
  Int_t    fMaxIterations;  //maximal nuber of iterations on startup
  Float_t  fMaxChiSquereChange;//value of ChiSqr change in %, when sturtup process in stable
  Float_t  fMaxChiSquerePerNDF;//value of the chi2 where generating histograms are considered as good
  
  
  Double_t fQRadius;
  
  Int_t    fPID;
  //we limit mixing to some finit phi range to make it faster
  Float_t  fSamplePhiMin;//
  Float_t  fSamplePhiMax;//
  
  Float_t  fSignalRegion;
  
  Int_t    fMinFill;
  
  Bool_t   fSwapped;
  
  ClassDef(AliGenHBTosl,1)
};
#endif
