#ifndef ALIGENPYTHIAPLUS_H
#define ALIGENPYTHIAPLUS_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */


/* $Id$ */

//
// Generator using the TPythia interface (via AliPythia)
// to generate pp collisions.
// Using SetNuclei() also nuclear modifications to the structure functions
// can be taken into account. This makes, of course, only sense for the
// generation of the products of hard processes (heavy flavor, jets ...)
//
// andreas.morsch@cern.ch
//

#include "AliGenMC.h"
#include "AliPythia.h"

class AliPythiaBase;
class TParticle;
class AliGenPythiaEventHeader;
class AliGenEventHeader;
class AliStack;
class AliRunLoader;

class AliGenPythiaPlus : public AliGenMC
{
 public:

    typedef enum {kFlavorSelection, kParentSelection} StackFillOpt_t;
    typedef enum {kCountAll, kCountParents, kCountTrackables} CountMode_t;
    typedef enum {kCluster, kCell} JetRecMode_t;
	  
    AliGenPythiaPlus();
    AliGenPythiaPlus(AliPythiaBase* pythia);
    virtual ~AliGenPythiaPlus();
    virtual void    Generate();
    virtual void    Init();
    virtual void    SetSeed(UInt_t seed);
    
    // Range of events to be printed
    virtual void    SetEventListRange(Int_t eventFirst=-1, Int_t eventLast=-1);
    // Select process type
    virtual void    SetProcess(Process_t proc = kPyCharm) {fProcess = proc;}
    // Select structure function
    virtual void    SetStrucFunc(StrucFunc_t func =  kCTEQ5L) {fStrucFunc = func;}
    // Select pt of hard scattering 
    virtual void    SetPtHard(Float_t ptmin = 0, Float_t ptmax = 1.e10)
	{fPtHardMin = ptmin; fPtHardMax = ptmax; }
    // y of hard scattering
    virtual void    SetYHard(Float_t ymin = -1.e10, Float_t ymax = 1.e10)
	{fYHardMin = ymin; fYHardMax = ymax; }
    // Set initial and final state gluon radiation
    virtual void    SetGluonRadiation(Int_t iIn, Int_t iFin)
	{fGinit = iIn; fGfinal = iFin;}
    // Intrinsic kT
    virtual void    SetPtKick(Float_t kt = 1.)
	{fPtKick = kt;}
    // Use the Pythia 6.3 new multiple interations scenario
    virtual void    UseNewMultipleInteractionsScenario() {fNewMIS = kTRUE;}
    // Switch off heavy flavors
    virtual void    SwitchHFOff() {fHFoff = kTRUE;}
    // Set centre of mass energy
    virtual void    SetEnergyCMS(Float_t energy = 5500) {fEnergyCMS = energy;}
    // Treat protons as inside nuclei with mass numbers a1 and a2
    virtual void    SetNuclei(Int_t a1, Int_t a2);
    //
    // Trigger options
    //
    // Energy range for jet trigger
    virtual void    SetJetEtRange(Float_t etmin = 0., Float_t etmax = 1.e4)
	{fEtMinJet = etmin; fEtMaxJet = etmax;}
    // Eta range for jet trigger
    virtual void    SetJetEtaRange(Float_t etamin = -20., Float_t etamax = 20.)
	{fEtaMinJet = etamin; fEtaMaxJet = etamax;}
    // Phi range for jet trigger
    virtual void    SetJetPhiRange(Float_t phimin = 0., Float_t phimax = 360.)
	{fPhiMinJet = TMath::Pi()*phimin/180.; fPhiMaxJet = TMath::Pi()*phimax/180.;}
    // Jet reconstruction mode; default is cone algorithm
    virtual void    SetJetReconstructionMode(Int_t mode = kCell) {fJetReconstruction = mode;}
    // Eta range for gamma trigger 
    virtual void    SetGammaEtaRange(Float_t etamin = -20., Float_t etamax = 20.)
	{fEtaMinGamma = etamin; fEtaMaxGamma = etamax;}
    // Phi range for gamma trigger
    virtual void    SetGammaPhiRange(Float_t phimin = 0., Float_t phimax = 360.)
	{fPhiMinGamma = TMath::Pi()*phimin/180.; fPhiMaxGamma = TMath::Pi()*phimax/180.;}
   // Select jets with fragmentation photon or pi0 going to PHOS or EMCAL
    virtual void  SetFragPhotonInCalo(Bool_t b)  {fFragPhotonInCalo = b;}
    virtual void  SetPi0InCalo       (Bool_t b)  {fPi0InCalo    = b;}
    virtual void  SetPhotonInCalo(Bool_t b)      {fPhotonInCalo = b;}
    virtual void  SetCheckPHOS (Bool_t b)        {fCheckPHOS    = b;}
    virtual void  SetCheckEMCAL(Bool_t b)        {fCheckEMCAL   = b;}
    virtual void  SetFragPhotonInEMCAL(Bool_t b) {fCheckEMCAL   = b; fFragPhotonInCalo = b;}
    virtual void  SetFragPhotonInPHOS(Bool_t b)  {fCheckPHOS    = b; fFragPhotonInCalo = b;}
    virtual void  SetPi0InEMCAL(Bool_t b)        {fCheckEMCAL   = b; fPi0InCalo        = b;}
    virtual void  SetPi0InPHOS(Bool_t b)         {fCheckPHOS    = b; fPi0InCalo        = b;}
    virtual void  SetPhotonInEMCAL(Bool_t b)     {fCheckEMCAL   = b; fPhotonInCalo     = b;}
    virtual void  SetPhotonInPHOS(Bool_t b)      {fCheckPHOS    = b; fPhotonInCalo     = b;}
    virtual void  SetPhotonInPHOSeta(Bool_t b)   {fCheckPHOSeta = b; fPhotonInCalo     = b;}
    virtual void  SetFragPhotonOrPi0MinPt(Float_t pt)      {fFragPhotonOrPi0MinPt = pt;}
    virtual void  SetPhotonMinPt(Float_t pt)     {fPhotonMinPt = pt;}
    // Trigger and rotate event 
    void RotatePhi(Int_t iphcand, Bool_t& okdd);
    // Trigger on a single particle
    virtual void    SetTriggerParticle(Int_t particle = 0, Float_t etamax = 0.9) 
	{fTriggerParticle = particle; fTriggerEta = etamax;}
    //
    // Heavy flavor options
    //
    // Set option for feed down from higher family
    virtual void SetFeedDownHigherFamily(Bool_t opt) {
	fFeedDownOpt = opt;
    }
    // Set option for selecting particles kept in stack according to flavor
    // or to parent selection
    virtual void SetStackFillOpt(StackFillOpt_t opt) {
	fStackFillOpt = opt;
    }
    // Set fragmentation option
    virtual void SetFragmentation(Bool_t opt) {
	fFragmentation = opt;
    }
    // Set counting mode
    virtual void SetCountMode(CountMode_t mode) {
	fCountMode = mode;
    }
    //
    // Quenching
    //
    // Set quenching mode 0 = no, 1 = AM, 2 = IL
    virtual void SetQuench(Int_t flag = 0) {fQuench = flag;}
    virtual void SetHadronisation(Int_t flag = 1) {fHadronisation = flag;}
    virtual void SetReadFromFile(const Text_t *filname) {fFileName = filname;  fReadFromFile = 1;}    

    //
    // Pile-up
    //
    // Get interaction rate for pileup studies
    virtual void    SetInteractionRate(Float_t rate,Float_t timewindow = 90.e-6);
    virtual Float_t GetInteractionRate() const {return fInteractionRate;}
    // Get cross section of process
    virtual Float_t GetXsection() const {return fXsection;}
    // Get triggered jets
    void GetJets(Int_t& njets, Int_t& ntrig, Float_t jets[4][10]);
    void RecJetsUA1(Int_t& njets, Float_t jets[4][50]);
    void SetPycellParameters(Float_t etamax = 2., Int_t neta = 274, Int_t nphi = 432,
			     Float_t thresh = 0., Float_t etseed = 4.,
			     Float_t minet = 10., Float_t r = 1.);
    
    // Getters
    virtual Process_t    GetProcess() const {return fProcess;}
    virtual StrucFunc_t  GetStrucFunc() const {return fStrucFunc;}
    virtual void         GetPtHard(Float_t& ptmin, Float_t& ptmax) const
	{ptmin = fPtHardMin; ptmax = fPtHardMax;}
    virtual void         GetNuclei(Int_t&  a1, Int_t& a2) const
	{a1 = fAProjectile; a2 = fATarget;}
    virtual void         GetJetEtRange(Float_t& etamin, Float_t& etamax) const
	{etamin = fEtaMinJet; etamax = fEtaMaxJet;}
    virtual void         GetJetPhiRange(Float_t& phimin, Float_t& phimax) const
	{phimin = fPhiMinJet*180./TMath::Pi(); phimax = fPhiMaxJet*180/TMath::Pi();}
    virtual void         GetGammaEtaRange(Float_t& etamin, Float_t& etamax) const
	{etamin = fEtaMinGamma; etamax = fEtaMaxGamma;}
    virtual void         GetGammaPhiRange(Float_t& phimin, Float_t& phimax) const
	{phimin = fPhiMinGamma*180./TMath::Pi(); phimax = fPhiMaxGamma*180./TMath::Pi();}
    virtual Int_t        GetTrialsEvent() const {return fTrials;}
    virtual Int_t        GetTrialsRun() const {return fTrialsRun;}
    // Y range for heavy quark trigger
    virtual void    SetHeavyQuarkYRange(Float_t ymin=-20., Float_t ymax=20.){
      fYMinHQ = ymin;    fYMaxHQ=ymax;   fUseYCutHQ=kTRUE;
    }


    //
    Bool_t IsInEMCAL(Float_t phi, Float_t eta) const;
    Bool_t IsInPHOS(Float_t phi, Float_t eta) const;
    //
    virtual void FinishRun();
    Bool_t CheckTrigger(const TParticle* jet1, const TParticle* jet2);
    //Used in some processes to selected child properties
    Bool_t CheckKinematicsOnChild();
    void     GetSubEventTime();
    virtual void    SetTune(Int_t itune) {fItune = itune;}
    virtual void    SetInfo(Int_t info)  {fInfo  = info;}
    void   WriteXsection(const Char_t *fname="pyxsec.root");

 protected:
    // adjust the weight from kinematic cuts
    void     AdjustWeights() const;
    Int_t    GenerateMB();
    void     MakeHeader();    
    void     GeneratePileup();
    AliPythiaBase *fPythia;         //Pythia 
    Process_t   fProcess;           //Process type
    StrucFunc_t fStrucFunc;         //Structure Function
    Float_t     fKineBias;          //!Bias from kinematic selection
    Int_t       fTrials;            //!Number of trials for current event
    Int_t       fTrialsRun;         //!Number of trials for run
    Float_t     fQ;                 //Mean Q
    Float_t     fX1;                //Mean x1
    Float_t     fX2;                //Mean x2
    Float_t     fEventTime;         //Time of the subevent
    Float_t     fInteractionRate;   //Interaction rate (set by user)
    Float_t     fTimeWindow;        //Time window for pileup events (set by user)
    Int_t       fCurSubEvent;       //Index of the current sub-event
    TArrayF     *fEventsTime;       //Subevents time for pileup
    Int_t       fNev;               //Number of events 
    Int_t       fFlavorSelect;      //Heavy Flavor Selection
    Float_t     fXsection;          //Cross-section
    Float_t     fPtHardMin;         //lower pT-hard cut 
    Float_t     fPtHardMax;         //higher pT-hard cut
    Float_t     fYHardMin;          //lower  y-hard cut 
    Float_t     fYHardMax;          //higher y-hard cut
    Int_t       fGinit;             //initial state gluon radiation
    Int_t       fGfinal;            //final state gluon radiation
    Int_t       fHadronisation;     //hadronisation
    Int_t       fNpartons;          //Number of partons before hadronisation
    Int_t       fReadFromFile;      //read partons from file
    Int_t       fQuench;            //Flag for quenching
    Float_t     fPtKick;            //Transverse momentum kick
    Bool_t      fFullEvent;         //!Write Full event if true
    AliDecayer  *fDecayer;          //!Pointer to the decayer instance
    Int_t       fDebugEventFirst;   //!First event to debug
    Int_t       fDebugEventLast;    //!Last  event to debug
    Float_t     fEtMinJet;          //Minimum et of triggered Jet
    Float_t     fEtMaxJet;          //Maximum et of triggered Jet
    Float_t     fEtaMinJet;         //Minimum eta of triggered Jet
    Float_t     fEtaMaxJet;         //Maximum eta of triggered Jet
    Float_t     fPhiMinJet;         //Minimum phi of triggered Jet
    Float_t     fPhiMaxJet;         //Maximum phi of triggered Jet
    Int_t       fJetReconstruction; //Jet Reconstruction mode 
    Float_t     fEtaMinGamma;       // Minimum eta of triggered gamma
    Float_t     fEtaMaxGamma;       // Maximum eta of triggered gamma
    Float_t     fPhiMinGamma;       // Minimum phi of triggered gamma
    Float_t     fPhiMaxGamma;       // Maximum phi of triggered gamma
    Bool_t      fUseYCutHQ;         // siwtch for using y cut for heavy quarks
    Float_t     fYMinHQ;            // Minimum y of triggered heavy quarks
    Float_t     fYMaxHQ;            // Maximum y of triggered heavy quarks
    Float_t     fPycellEtaMax;      // Max. eta for Pycell 
    Int_t       fPycellNEta;        // Number of eta bins for Pycell 
    Int_t       fPycellNPhi;        // Number of phi bins for Pycell
    Float_t	fPycellThreshold;   // Pycell threshold
    Float_t 	fPycellEtSeed;      // Pycell seed
    Float_t	fPycellMinEtJet;    // Pycell min. jet et
    Float_t	fPycellMaxRadius;   // Pycell cone radius
    StackFillOpt_t fStackFillOpt;   // Stack filling with all particles with
                                    // that flavour or only with selected
                                    // parents and their decays
    Bool_t fFeedDownOpt;            // Option to set feed down from higher
                                    // quark families (e.g. b->c)
    Bool_t  fFragmentation;         // Option to activate fragmentation by Pythia
    Bool_t  fSetNuclei;             // Flag indicating that SetNuclei has been called
    Bool_t  fNewMIS;                // Flag for the new multipple interactions scenario
    Bool_t  fHFoff;                 // Flag for switching heafy flavor production off
    Int_t   fTriggerParticle;       // Trigger on this particle ...
    Float_t fTriggerEta;            // .. within |eta| < fTriggerEta
    CountMode_t fCountMode;         // Options for counting when the event will be finished.     
    // fCountMode = kCountAll         --> All particles that end up in the
    //                                    stack are counted
    // fCountMode = kCountParents     --> Only selected parents are counted
    // fCountMode = kCountTrackabless --> Only particles flagged for tracking
    //                                     are counted
    //
    //

    AliGenPythiaEventHeader* fHeader;  //! Event header
    AliRunLoader*            fRL;      //! Run Loader
    const Text_t* fFileName;           //! Name of file to read from


    Bool_t fFragPhotonInCalo; // Option to ask for Fragmentation Photon in calorimeters acceptance
    Bool_t fPi0InCalo;        // Option to ask for Pi0 in calorimeters acceptance
    Bool_t fPhotonInCalo;     // Option to ask for Decay Photon in calorimeter acceptance
    Bool_t fCheckEMCAL;       // Option to ask for FragPhoton or Pi0 in calorimeters EMCAL acceptance
    Bool_t fCheckPHOS;        // Option to ask for FragPhoton or Pi0 in calorimeters PHOS acceptance
    Bool_t fCheckPHOSeta;     // Option to ask for PHOS eta acceptance
    Float_t fFragPhotonOrPi0MinPt; // Minimum momentum of Fragmentation Photon or Pi0
    Float_t fPhotonMinPt;          // Minimum momentum of Photon 
    //Calorimeters eta-phi acceptance 
    Float_t fPHOSMinPhi;           // Minimum phi PHOS
    Float_t fPHOSMaxPhi;           // Maximum phi PHOS
    Float_t fPHOSEta;              // Minimum eta PHOS
    Float_t fEMCALMinPhi;          // Minimum phi EMCAL
    Float_t fEMCALMaxPhi;          // Maximum phi EMCAL
    Float_t fEMCALEta;             // Maximum eta EMCAL
    Int_t   fItune;                // Pythia tune 
    Int_t   fInfo;                 // extented event info   
 private:
    AliGenPythiaPlus(const AliGenPythiaPlus &Pythia);
    AliGenPythiaPlus & operator=(const AliGenPythiaPlus & rhs);

    ClassDef(AliGenPythiaPlus, 4) // AliGenerator interface to Pythia
};
#endif





