//*******************************************************************//
// Configuration file for charm / beauty generation with PYTHIA      //
//                                                                   //
// The parameters have been tuned in order to reproduce the inclusive//
// heavy quark pt distribution given by the NLO pQCD calculation by  //
// Mangano, Nason and Ridolfi.                                       //
//                                                                   //
// For details and for the NORMALIZATION of the yields see:          //
//   N.Carrer and A.Dainese,                                         //
//   "Charm and beauty production at the LHC",                       //
//   ALICE-INT-2003-019, [arXiv:hep-ph/0311225];                     //
//   PPR Chapter 6.6, CERN/LHCC 2005-030 (2005).                     //
//*******************************************************************//
#if !defined(__CINT__) || defined(__MAKECINT__)
#include <Riostream.h>
#include <TRandom.h>
#include <TDatime.h>
#include <TSystem.h>
#include <TVirtualMC.h>
#include <TGeant3TGeo.h>
#include "STEER/AliRunLoader.h"
#include "STEER/AliRun.h"
#include "STEER/AliConfig.h"
#include "PYTHIA6/AliDecayerPythia.h"
#include "PYTHIA6/AliGenPythia.h"
#include "STEER/AliMagF.h"
#include "STRUCT/AliBODY.h"
#include "STRUCT/AliMAG.h"
#include "STRUCT/AliABSOv3.h"
#include "STRUCT/AliDIPOv3.h"
#include "STRUCT/AliHALLv3.h"
#include "STRUCT/AliFRAMEv2.h"
#include "STRUCT/AliSHILv3.h"
#include "STRUCT/AliPIPEv3.h"
#include "ITS/AliITSv11.h"
#include "TPC/AliTPCv2.h"
#include "TOF/AliTOFv6T0.h"
#include "HMPID/AliHMPIDv3.h"
#include "ZDC/AliZDCv3.h"
#include "TRD/AliTRDv1.h"
#include "FMD/AliFMDv0.h"
#include "MUON/AliMUONv1.h"
#include "PHOS/AliPHOSv1.h"
#include "PMD/AliPMDv1.h"
#include "T0/AliT0v1.h"
#include "ACORDE/AliACORDEv1.h"
#endif

//--- Heavy Flavour Production ---
enum ProcessHvFl_t 
{
  kCharmPbPb5500,  kCharmpPb8800,  kCharmpp14000,  kCharmpp14000wmi,
  kCharmSemiElpp14000wmi,
  kD0PbPb5500,     kD0pPb8800,     kD0pp14000,
  kDPlusPbPb5500,  kDPluspPb8800,  kDPluspp14000,
  kDPlusStrangePbPb5500, kDPlusStrangepPb8800, kDPlusStrangepp14000,
  kBeautyPbPb5500, kBeautypPb8800, kBeautypp14000, kBeautypp14000wmi,
  kBeautySemiElpp14000wmi
};
//--- Decay Mode ---
enum DecayHvFl_t 
{
  kNature,  kHadr, kSemiEl, kSemiMu
};
//--- Rapidity Cut ---
enum YCut_t
{
  kFull, kBarrel, kMuonArm
};
//--- Trigger config ---
enum TrigConf_t
{
    kDefaultPPTrig, kDefaultPbPbTrig
};
const char * TrigConfName[] = {
    "p-p","Pb-Pb"
};

//--- Functions ---
class AliGenPythia;
AliGenPythia *PythiaHVQ(ProcessHvFl_t proc);
void          LoadPythia();


// This part for configuration
static ProcessHvFl_t procHvFl = kCharmpp14000wmi;
static DecayHvFl_t   decHvFl  = kNature; 
static YCut_t        ycut     = kFull;
static AliMagF::BMap_t mag    = AliMagF::k5kG; 
static TrigConf_t    trig     = kDefaultPbPbTrig; // default PbPb trigger configuration
// nEvts = -1  : you get 1 QQbar pair and all the fragmentation and 
//               decay chain
// nEvts = N>0 : you get N charm / beauty Hadrons 
Int_t nEvts = -1; 
// stars = kTRUE : all heavy resonances and their decay stored
//       = kFALSE: only final heavy hadrons and their decays stored
Bool_t stars = kTRUE;

// To be used only with kCharmppMNRwmi and kBeautyppMNRwmi
// To get a "reasonable" agreement with MNR results, events have to be 
// generated with the minimum ptHard set to 2.76 GeV.
// To get a "perfect" agreement with MNR results, events have to be 
// generated in four ptHard bins with the following relative 
// normalizations:
//  CHARM
// 2.76-3 GeV: 25%
//    3-4 GeV: 40%
//    4-8 GeV: 29%
//     >8 GeV:  6%
//  BEAUTY
// 2.76-4 GeV:  5% 
//    4-6 GeV: 31%
//    6-8 GeV: 28%
//     >8 GeV: 36%
Float_t ptHardMin =  2.76;
Float_t ptHardMax = -1.;


// Comment line
static TString comment;

void Config()
{
 
  //========================//
  // Set Random Number seed //
  //========================//
  TDatime dt;
  UInt_t curtime=dt.Get();
  UInt_t procid=gSystem->GetPid();
  UInt_t seed=curtime-procid;

  //  gRandom->SetSeed(seed);
  //  cerr<<"Seed for random number generation= "<<seed<<endl; 
  gRandom->SetSeed(12345);
  
  // Load Pythia libraries
  LoadPythia();
  // libraries required by geant321
#if defined(__CINT__)
  gSystem->Load("libgeant321");
#endif

  new TGeant3TGeo("C++ Interface to Geant3");

  if(!AliCDBManager::Instance()->IsDefaultStorageSet()){
    AliCDBManager::Instance()->SetDefaultStorage("local://$ALICE_ROOT/OCDB");
    AliCDBManager::Instance()->SetRun(0);
  }
  
  //=======================================================================
  //  Create the output file

   
  AliRunLoader* rl=0x0;

  cout<<"Config.C: Creating Run Loader ..."<<endl;
  rl = AliRunLoader::Open("galice.root",
			  AliConfig::GetDefaultEventFolderName(),
			  "recreate");
  if (rl == 0x0)
    {
      gAlice->Fatal("Config.C","Can not instatiate the Run Loader");
      return;
    }
  rl->SetCompressionLevel(2);
  rl->SetNumberOfEventsPerFile(1000);
  gAlice->SetRunLoader(rl);
  // gAlice->SetGeometryFromFile("geometry.root");
  // gAlice->SetGeometryFromCDB();

  // Set the trigger configuration
  AliSimulation::Instance()->SetTriggerConfig(TrigConfName[trig]);
  cout<<"Trigger configuration is set to  "<<TrigConfName[trig]<<endl;

  //
  //=======================================================================
  // ************* STEERING parameters FOR ALICE SIMULATION **************
  // --- Specify event type to be tracked through the ALICE setup
  // --- All positions are in cm, angles in degrees, and P and E in GeV


    gMC->SetProcess("DCAY",1);
    gMC->SetProcess("PAIR",1);
    gMC->SetProcess("COMP",1);
    gMC->SetProcess("PHOT",1);
    gMC->SetProcess("PFIS",0);
    gMC->SetProcess("DRAY",0);
    gMC->SetProcess("ANNI",1);
    gMC->SetProcess("BREM",1);
    gMC->SetProcess("MUNU",1);
    gMC->SetProcess("CKOV",1);
    gMC->SetProcess("HADR",1);
    gMC->SetProcess("LOSS",2);
    gMC->SetProcess("MULS",1);
    gMC->SetProcess("RAYL",1);

    Float_t cut = 1.e-3;        // 1MeV cut by default
    Float_t tofmax = 1.e10;

    gMC->SetCut("CUTGAM", cut);
    gMC->SetCut("CUTELE", cut);
    gMC->SetCut("CUTNEU", cut);
    gMC->SetCut("CUTHAD", cut);
    gMC->SetCut("CUTMUO", cut);
    gMC->SetCut("BCUTE",  cut); 
    gMC->SetCut("BCUTM",  cut); 
    gMC->SetCut("DCUTE",  cut); 
    gMC->SetCut("DCUTM",  cut); 
    gMC->SetCut("PPCUTM", cut);
    gMC->SetCut("TOFMAX", tofmax); 




  // Set External decayer //
  //======================//
  TVirtualMCDecayer* decayer = new AliDecayerPythia();
  // DECAYS
  //
  switch(decHvFl) {
  case kNature:
    decayer->SetForceDecay(kAll);
    break;
  case kHadr:
    decayer->SetForceDecay(kHadronicD);
    break;
  case kSemiEl:
    decayer->SetForceDecay(kSemiElectronic);
    break;
  case kSemiMu:
    decayer->SetForceDecay(kSemiMuonic);
    break;
  }
  decayer->Init();
  gMC->SetExternalDecayer(decayer);

  //=========================//
  // Generator Configuration //
  //=========================//
  AliGenPythia *pythia = PythiaHVQ(procHvFl);
  // FeedDown option
  pythia->SetFeedDownHigherFamily(kFALSE);
  // Stack filling option
  if(!stars) pythia->SetStackFillOpt(AliGenPythia::kParentSelection);
  // Set Count mode
  if(nEvts>0) pythia->SetCountMode(AliGenPythia::kCountParents);
  // DECAYS
  //
  switch(decHvFl) {
  case kNature:
    pythia->SetForceDecay(kAll);
    break;
  case kHadr:
    pythia->SetForceDecay(kHadronicD);
    break;
  case kSemiEl:
    pythia->SetForceDecay(kSemiElectronic);
    break;
  case kSemiMu:
    pythia->SetForceDecay(kSemiMuonic);
    break;
  }
  // GEOM & KINE CUTS
  //
  pythia->SetMomentumRange(0,99999999);
  pythia->SetPhiRange(0., 360.);
  pythia->SetThetaRange(0,180);
  switch(ycut) {
  case kFull:
    pythia->SetYRange(-999,999);
    break;
  case kBarrel:
    pythia->SetYRange(-2,2);
    break;
  case kMuonArm:
    pythia->SetYRange(1,6);
    break;
  }

  // PRIMARY VERTEX
  //
  pythia->SetOrigin(0, 0, 0);    // vertex position
  pythia->SetSigma(0, 0, 5.3);   // Sigma in (X,Y,Z) (cm) on IP position
  pythia->SetCutVertexZ(1.);     // Truncate at 1 sigma
  pythia->SetVertexSmear(kPerEvent); 



  pythia->SetTrackingFlag(0);
  // Specify GEANT tracking limits (Rmax, Zmax)
  //gAlice->GetMCApp()->TrackingLimits(90.,1.0e10);


  pythia->Init();

  // FIELD
  //    
  if (mag == AliMagF::k2kG) {
    comment = comment.Append(" | L3 field 0.2 T");
  } else if (mag == AliMagF::k5kG) {
    comment = comment.Append(" | L3 field 0.5 T");
  }
  printf("\n \n Comment: %s \n \n", comment.Data());
    
  AliMagF* field = new AliMagF("Maps","Maps",  1., 1.,  mag);
  field->SetL3ConstField(0); //Using const. field in the barrel
  TGeoGlobalMagField::Instance()->SetField(field);

  rl->CdGAFile();

  // By default all ALICE is switched off
  Int_t iABSO=0;
  Int_t iACORDE=0;
  Int_t iDIPO=0;
  Int_t iFMD=0;
  Int_t iFRAME=0;
  Int_t iHALL=0;
  Int_t iITS=0;
  Int_t iMAG=0;
  Int_t iMUON=0;
  Int_t iPHOS=0;
  Int_t iPIPE=0;
  Int_t iPMD=0;
  Int_t iHMPID=0;
  Int_t iSHIL=0;
  Int_t iT0=0;
  Int_t iTOF=0;
  Int_t iTPC=0;
  Int_t iTRD=0;
  Int_t iZDC=0;

  //=================== Alice BODY parameters =============================
  AliBODY *BODY = new AliBODY("BODY","Alice envelop");

  if(iMAG) {
    //=================== MAG parameters ============================
    // --- Start with Magnet since detector layouts may be depending ---
    // --- on the selected Magnet dimensions ---
    AliMAG *MAG  = new AliMAG("MAG","Magnet");
  }


  if(iABSO) {
    //=================== ABSO parameters ============================
    AliABSO *ABSO  = new AliABSOv3("ABSO","Muon Absorber");
  }

  if(iDIPO) {
    //=================== DIPO parameters ============================

    AliDIPO *DIPO  = new AliDIPOv3("DIPO","Dipole version 3");
  }

  if(iHALL) {
    //=================== HALL parameters ============================

    AliHALL *HALL  = new AliHALLv3("HALL","Alice Hall");
  }


  if(iFRAME) {
    //=================== FRAME parameters ============================

    AliFRAME *FRAME  = new AliFRAMEv2("FRAME","Space Frame");
    FRAME->SetHoles(1);
  }

  if(iSHIL) {
    //=================== SHIL parameters ============================

    AliSHIL *SHIL  = new AliSHILv3("SHIL","Shielding");
  }


  if(iPIPE) {
    //=================== PIPE parameters ============================

    AliPIPE *PIPE  = new AliPIPEv3("PIPE","Beam Pipe");
  }


  if(iITS) {
    //=================== ITS parameters ============================

    AliITS *ITS  = new AliITSv11("ITS","ITS v11");
  }
  

  if(iTPC) {
    //============================ TPC parameters ===================
    AliTPC *TPC  = new AliTPCv2("TPC","Default");
  }


  if(iTOF) {
    //=================== TOF parameters ============================
    AliTOF *TOF  = new AliTOFv6T0("TOF","normal TOF");
  }

  if(iHMPID) {
    //=================== HMPID parameters ===========================
    AliHMPID *HMPID  = new AliHMPIDv3("HMPID","normal HMPID");    

  }


  if(iZDC) {
    //=================== ZDC parameters ============================

    AliZDC *ZDC  = new AliZDCv3("ZDC","normal ZDC");
  }

  if(iACORDE) {
    //=================== ACORDE parameters ============================

    AliACORDE *ACORDE  = new AliACORDEv1("ACORDE","normal ACORDE");
  }

  if(iTRD) {
    //=================== TRD parameters ============================
  
    AliTRD *TRD  = new AliTRDv1("TRD","TRD slow simulator");
  }

  if(iFMD) {
    //=================== FMD parameters ============================

    AliFMD *FMD  = new AliFMDv0("FMD","normal FMD");
  }

  if(iMUON) {
    //=================== MUON parameters ===========================
    AliMUON *MUON  = new AliMUONv1("MUON","default");
  }
 
  //=================== PHOS parameters ===========================

  if(iPHOS) {
    AliPHOS *PHOS  = new AliPHOSv1("PHOS","GPS2");
  }


  //=================== ACORDE parameters ===========================

  if(iACORDE) {
    AliACORDE *ACORDE  = new AliACORDEv1("ACORDE","Normal ACORDEGPS2");
  }


  if(iPMD) {
    //=================== PMD parameters ============================

    AliPMD *PMD  = new AliPMDv1("PMD","normal PMD");
    PMD->SetPAR(1., 1., 0.8, 0.02);
    PMD->SetIN(6., 18., -580., 27., 27.);
    PMD->SetGEO(0.0, 0.2, 4.);
    PMD->SetPadSize(0.8, 1.0, 1.0, 1.5);

  }

  if(iT0) {
    //=================== T0 parameters ============================
    AliT0 *T0  = new AliT0v1("T0","T0 Detector");
  }

         
}
//
//           PYTHIA
//
AliGenPythia *PythiaHVQ(ProcessHvFl_t proc) {

  AliGenPythia * gener = 0x0;

  switch(proc) {
  case kCharmPbPb5500:
    comment = comment.Append(" Charm in Pb-Pb at 5.5 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyCharmPbPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(5500.);
    gener->SetNuclei(208,208);
    break;
  case kCharmpPb8800:
    comment = comment.Append(" Charm in p-Pb at 8.8 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyCharmpPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(8800.);
    gener->SetProjectile("P",1,1);
    gener->SetTarget("Pb",208,82);
    break;
  case kCharmpp14000:
    comment = comment.Append(" Charm in pp at 14 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyCharmppMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(14000.);
    break;
  case kCharmpp14000wmi:
    comment = comment.Append(" Charm in pp at 14 TeV with mult. interactions");
    gener = new AliGenPythia(-1);
    gener->SetProcess(kPyCharmppMNRwmi);
    gener->SetStrucFunc(kCTEQ5L);
    gener->SetPtHard(ptHardMin,ptHardMax);
    gener->SetEnergyCMS(14000.);
    break;
  case kCharmSemiElpp14000wmi:
    comment = comment.Append(" Charm in pp at 14 TeV with mult. interactions");
    gener = new AliGenPythia(-1);
    gener->SetProcess(kPyCharmppMNRwmi);
    gener->SetStrucFunc(kCTEQ5L);
    gener->SetPtHard(ptHardMin,ptHardMax);
    gener->SetEnergyCMS(14000.);
    gener->SetCutOnChild(1);
    gener->SetPdgCodeParticleforAcceptanceCut(11);
    gener->SetChildYRange(-1,1);
    gener->SetChildPtRange(1,10000.);
    break;
  case kD0PbPb5500:
    comment = comment.Append(" D0 in Pb-Pb at 5.5 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyD0PbPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(5500.);
    gener->SetNuclei(208,208);
    break;
  case kD0pPb8800:
    comment = comment.Append(" D0 in p-Pb at 8.8 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyD0pPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(8800.);
    gener->SetProjectile("P",1,1);
    gener->SetTarget("Pb",208,82);
    break;
  case kD0pp14000:
    comment = comment.Append(" D0 in pp at 14 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyD0ppMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(14000.);
    break;
  case kDPlusPbPb5500:
    comment = comment.Append(" DPlus in Pb-Pb at 5.5 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyDPlusPbPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(5500.);
    gener->SetNuclei(208,208);
    break;
  case kDPluspPb8800:
    comment = comment.Append(" DPlus in p-Pb at 8.8 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyDPluspPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(8800.);
    gener->SetProjectile("P",1,1);
    gener->SetTarget("Pb",208,82);
    break;
  case kDPluspp14000:
    comment = comment.Append(" DPlus in pp at 14 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyDPlusppMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(14000.);
    break;
 case kDPlusStrangePbPb5500:
    comment = comment.Append(" DPlusStrange in Pb-Pb at 5.5 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyDPlusStrangePbPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(5500.);
    gener->SetNuclei(208,208);
    break;
  case kDPlusStrangepPb8800:
    comment = comment.Append(" DPlusStrange in p-Pb at 8.8 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyDPlusStrangepPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(8800.);
    gener->SetProjectile("P",1,1);
    gener->SetTarget("Pb",208,82);
    break;
  case kDPlusStrangepp14000:
    comment = comment.Append(" DPlusStrange in pp at 14 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyDPlusStrangeppMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.1,-1.0);
    gener->SetEnergyCMS(14000.);
    break;
  case kBeautyPbPb5500:
    comment = comment.Append(" Beauty in Pb-Pb at 5.5 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyBeautyPbPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.75,-1.0);
    gener->SetEnergyCMS(5500.);
    gener->SetNuclei(208,208);
    break;
  case kBeautypPb8800:
    comment = comment.Append(" Beauty in p-Pb at 8.8 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyBeautypPbMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.75,-1.0);
    gener->SetEnergyCMS(8800.);
    gener->SetProjectile("P",1,1);
    gener->SetTarget("Pb",208,82);
    break;
  case kBeautypp14000:
    comment = comment.Append(" Beauty in pp at 14 TeV");
    gener = new AliGenPythia(nEvts);
    gener->SetProcess(kPyBeautyppMNR);
    gener->SetStrucFunc(kCTEQ4L);
    gener->SetPtHard(2.75,-1.0);
    gener->SetEnergyCMS(14000.);
    break;
  case kBeautypp14000wmi:
    comment = comment.Append(" Beauty in pp at 14 TeV with mult. interactions");
    gener = new AliGenPythia(-1);
    gener->SetProcess(kPyBeautyppMNRwmi);
    gener->SetStrucFunc(kCTEQ5L);
    gener->SetPtHard(ptHardMin,ptHardMax);
    gener->SetEnergyCMS(14000.);
    break;
  case kBeautySemiElpp14000wmi:
    comment = comment.Append(" Beauty in pp at 14 TeV with mult. interactions");
    gener = new AliGenPythia(-1);
    gener->SetProcess(kPyBeautyppMNRwmi);
    gener->SetStrucFunc(kCTEQ5L);
    gener->SetPtHard(ptHardMin,ptHardMax);
    gener->SetEnergyCMS(14000.);
    gener->SetCutOnChild(1);
    gener->SetPdgCodeParticleforAcceptanceCut(11);
    gener->SetChildYRange(-1,1);
    gener->SetChildPtRange(1,10000.);
    break;
  }

  return gener;
}



void LoadPythia()
{
    // Load Pythia related libraries
    gSystem->Load("liblhapdf");      // Parton density functions
    gSystem->Load("libEGPythia6");   // TGenerator interface
    gSystem->Load("libpythia6");     // Pythia
    gSystem->Load("libAliPythia6");  // ALICE specific implementations
}



