/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

// This class is extracted from the AliRun class
// and contains all the MC-related functionality
// The number of dependencies has to be reduced...
// Author: F.Carminati
//         Federico.Carminati@cern.ch

#include <string.h>

#include <RVersion.h>
#include <TArrayI.h>
#include <TClonesArray.h>
#include <TFile.h>
#include <TGeoGlobalMagField.h>
#include <TGeoManager.h>
#include <TParticle.h>
#include <TROOT.h>
#include <TStopwatch.h>
#include <TSystem.h>
#include <TVirtualMC.h>
#include <TTree.h>
 
#include "AliCDBEntry.h"
#include "AliCDBManager.h"
#include "AliCDBStorage.h"
#include "AliDetector.h"
#include "AliGenerator.h"
#include "AliGeomManager.h"
#include "AliHeader.h"
#include "AliHit.h"
#include "AliLego.h"
#include "AliLog.h"
#include "AliMC.h"
#include "AliMagF.h"
#include "AliRun.h"
#include "AliSimulation.h"
#include "AliStack.h"
#include "AliTrackReference.h"

ClassImp(AliMC)

//_______________________________________________________________________
AliMC::AliMC() :
  fGenerator(0),
  fEventEnergy(0),
  fSummEnergy(0),
  fSum2Energy(0),
  fTrRmax(1.e10),
  fTrZmax(1.e10),
  fRDecayMax(1.e10),
  fRDecayMin(-1.),
  fDecayPdg(0),
  fImedia(0),
  fTransParName("\0"),
  fHitLists(0),
  fTmpTreeTR(0),
  fTmpFileTR(0),
  fTrackReferences(),
  fTmpTrackReferences()

{
  //default constructor
  DecayLimits();
}

//_______________________________________________________________________
AliMC::AliMC(const char *name, const char *title) :
  TVirtualMCApplication(name, title),
  fGenerator(0),
  fEventEnergy(0),
  fSummEnergy(0),
  fSum2Energy(0),
  fTrRmax(1.e10),
  fTrZmax(1.e10),
  fRDecayMax(1.e10),
  fRDecayMin(-1.),
  fDecayPdg(0),
  fImedia(new TArrayI(1000)),
  fTransParName("\0"),
  fHitLists(new TList()),
  fTmpTreeTR(0),
  fTmpFileTR(0),
  fTrackReferences("AliTrackReference", 100),
  fTmpTrackReferences("AliTrackReference", 100)
{
  //constructor
  // Set transport parameters
  SetTransPar();
  DecayLimits();
  // Prepare the tracking medium lists
  for(Int_t i=0;i<1000;i++) (*fImedia)[i]=-99;
}

//_______________________________________________________________________
AliMC::~AliMC()
{
  //destructor
  delete fGenerator;
  delete fImedia;
  delete fHitLists;
  // Delete track references
}

//_______________________________________________________________________
void  AliMC::ConstructGeometry() 
{
  //
  // Either load geometry from file or create it through usual
  // loop on detectors. In the first case the method
  // AliModule::CreateMaterials() only builds fIdtmed and is postponed
  // at InitGeometry().
  //

  if(AliSimulation::Instance()->IsGeometryFromFile()){ //load geometry either from CDB or from file
    if(IsGeometryFromCDB()){
      AliInfo("Loading geometry from CDB default storage");
      AliCDBPath path("GRP","Geometry","Data");
      AliCDBEntry *entry=AliCDBManager::Instance()->Get(path.GetPath());
      if(!entry) AliFatal("Unable to load geometry from CDB!");
      entry->SetOwner(0);
      gGeoManager = (TGeoManager*) entry->GetObject();
      if (!gGeoManager) AliFatal("TGeoManager object not found in the specified CDB entry!");
    }else{
      // Load geometry
      const char *geomfilename = AliSimulation::Instance()->GetGeometryFile();
      if(gSystem->ExpandPathName(geomfilename)){
	AliInfo(Form("Loading geometry from file:\n %40s",geomfilename));
	TGeoManager::Import(geomfilename);
      }else{
	AliInfo(Form("Geometry file %40s not found!\n",geomfilename));
	return;
      }
    }
    gMC->SetRootGeometry();
  }else{
    // Create modules, materials, geometry
    if (!gGeoManager) new TGeoManager("ALICE", "ALICE geometry");
    TStopwatch stw;
    TIter next(gAlice->Modules());
    AliModule *detector;
    AliDebug(1, "Geometry creation:");
    while((detector = dynamic_cast<AliModule*>(next()))) {
      stw.Start();
      // Initialise detector materials and geometry
      detector->CreateMaterials();
      detector->CreateGeometry();
      AliInfo(Form("%10s R:%.2fs C:%.2fs",
		   detector->GetName(),stw.RealTime(),stw.CpuTime()));
    }
  }
  
}

//_______________________________________________________________________
Bool_t  AliMC::MisalignGeometry() 
{
  // Call misalignment code if AliSimulation object was defined.
  
  if(!AliSimulation::Instance()->IsGeometryFromFile()){
    //Set alignable volumes for the whole geometry
    SetAllAlignableVolumes();
  }
  // Misalign geometry via AliSimulation instance
  if (!AliSimulation::Instance()) return kFALSE;
  AliGeomManager::SetGeometry(gGeoManager);
  if(!AliGeomManager::CheckSymNamesLUT("ALL"))
    AliFatal("Current loaded geometry differs in the definition of symbolic names!");
  
  return AliSimulation::Instance()->MisalignGeometry(AliRunLoader::Instance());
}   

//_______________________________________________________________________
void  AliMC::ConstructOpGeometry() 
{
  //
  // Loop all detector modules and call DefineOpticalProperties() method 
  //

  TIter next(gAlice->Modules());
  AliModule *detector;
  AliInfo("Optical properties definition");
  while((detector = dynamic_cast<AliModule*>(next()))) {
    // Initialise detector geometry
    if(AliSimulation::Instance()->IsGeometryFromFile()) detector->CreateMaterials();
    // Initialise detector optical properties
    detector->DefineOpticalProperties();
  }  
}

//_______________________________________________________________________
void  AliMC::InitGeometry()
{ 
  //
  // Initialize detectors
  //

  AliInfo("Initialisation:");
  TStopwatch stw;
  TIter next(gAlice->Modules());
  AliModule *detector;
  while((detector = dynamic_cast<AliModule*>(next()))) {
    stw.Start();
    detector->Init();
    AliInfo(Form("%10s R:%.2fs C:%.2fs",
		 detector->GetName(),stw.RealTime(),stw.CpuTime()));
  }
}

//_______________________________________________________________________
void AliMC::SetGeometryFromCDB()
{
  // Set the loading of geometry from cdb instead of creating it
  // A default CDB storage needs to be set before this method is called
  if(AliCDBManager::Instance()->IsDefaultStorageSet() &&
     AliCDBManager::Instance()->GetRun() >= 0)
    AliSimulation::Instance()->SetGeometryFile("*OCDB*");
  else
    AliError("Loading of geometry from CDB ignored. First set a default CDB storage!");
}

//_______________________________________________________________________
Bool_t AliMC::IsGeometryFromCDB() const
{
  return (strcmp(AliSimulation::Instance()->GetGeometryFile(),"*OCDB*")==0);
}

//_______________________________________________________________________
void  AliMC::SetAllAlignableVolumes()
{ 
  //
  // Add alignable volumes (TGeoPNEntries) looping on all
  // active modules
  //

  AliInfo(Form("Setting entries for all alignable volumes of active detectors"));
  AliModule *detector;
  TIter next(gAlice->Modules());
  while((detector = dynamic_cast<AliModule*>(next()))) {
    detector->AddAlignableVolumes();
  }
}

//_______________________________________________________________________
void  AliMC::GeneratePrimaries() 
{ 
  //
  // Generate primary particles and fill them in the stack.
  //

  Generator()->Generate();
}

//_______________________________________________________________________
void AliMC::SetGenerator(AliGenerator *generator)
{
  //
  // Load the event generator
  //
  if(!fGenerator) fGenerator = generator;
}

//_______________________________________________________________________
void AliMC::ResetGenerator(AliGenerator *generator)
{
  //
  // Load the event generator
  //
  if(fGenerator) {
    if(generator) {
      AliWarning(Form("Replacing generator %s with %s",
		      fGenerator->GetName(),generator->GetName()));
    }
    else {
      AliWarning(Form("Replacing generator %s with NULL",
		      fGenerator->GetName()));
    }
  }
  fGenerator = generator;
}

//_______________________________________________________________________
void AliMC::FinishRun()
{
  // Clean generator information
  AliDebug(1, "fGenerator->FinishRun()");
  fGenerator->FinishRun();

  //Output energy summary tables
  AliDebug(1, "EnergySummary()");
  ToAliDebug(1, EnergySummary());
}

//_______________________________________________________________________
void AliMC::BeginPrimary()
{
  //
  // Called  at the beginning of each primary track
  //
  
  // Reset Hits info
  ResetHits();
  ResetTrackReferences();
}

//_______________________________________________________________________
void AliMC::PreTrack()
{
  // Actions before the track's transport

     TObjArray &dets = *gAlice->Modules();
     AliModule *module;

     for(Int_t i=0; i<=gAlice->GetNdets(); i++)
       if((module = dynamic_cast<AliModule*>(dets[i])))
	 module->PreTrack();
}

//_______________________________________________________________________
void AliMC::Stepping() 
{
  //
  // Called at every step during transport
  //
  Int_t id = DetFromMate(gMC->CurrentMedium());
  if (id < 0) return;


  if ( gMC->IsNewTrack()            && 
       gMC->TrackTime() == 0.       &&
       fRDecayMin >= 0.             &&  
       fRDecayMax > fRDecayMin      &&
       gMC->TrackPid() == fDecayPdg ) 
  {
      FixParticleDecaytime();
  } 
    

  
  //
  // --- If lego option, do it and leave 
  if (AliSimulation::Instance()->Lego())
    AliSimulation::Instance()->Lego()->StepManager();
  else {
    Int_t copy;
    //Update energy deposition tables
    AddEnergyDeposit(gMC->CurrentVolID(copy),gMC->Edep());
    //
    // write tracke reference for track which is dissapearing - MI

    if (gMC->IsTrackDisappeared() && !(gMC->IsTrackAlive())) {      
	if (gMC->Etot() > 0.05) AddTrackReference(GetCurrentTrackNumber(), 
						AliTrackReference::kDisappeared);
	

    }

    //Call the appropriate stepping routine;
    AliModule *det = dynamic_cast<AliModule*>(gAlice->Modules()->At(id));
    if(det && det->StepManagerIsEnabled()) {
      det->StepManager();
    }
  }
}

//_______________________________________________________________________
void AliMC::EnergySummary()
{
  //e
  // Print summary of deposited energy
  //

  Int_t ndep=0;
  Float_t edtot=0;
  Float_t ed, ed2;
  Int_t kn, i, left, j, id;
  const Float_t kzero=0;
  Int_t ievent=AliRunLoader::Instance()->GetHeader()->GetEvent()+1;
  //
  // Energy loss information
  if(ievent) {
    printf("***************** Energy Loss Information per event (GEV) *****************\n");
    for(kn=1;kn<fEventEnergy.GetSize();kn++) {
      ed=fSummEnergy[kn];
      if(ed>0) {
	fEventEnergy[ndep]=kn;
	if(ievent>1) {
	  ed=ed/ievent;
	  ed2=fSum2Energy[kn];
	  ed2=ed2/ievent;
	  ed2=100*TMath::Sqrt(TMath::Max(ed2-ed*ed,kzero))/ed;
	} else 
	  ed2=99;
	fSummEnergy[ndep]=ed;
	fSum2Energy[ndep]=TMath::Min(static_cast<Float_t>(99.),TMath::Max(ed2,kzero));
	edtot+=ed;
	ndep++;
      }
    }
    for(kn=0;kn<(ndep-1)/3+1;kn++) {
      left=ndep-kn*3;
      for(i=0;i<(3<left?3:left);i++) {
	j=kn*3+i;
        id=Int_t (fEventEnergy[j]+0.1);
	printf(" %s %10.3f +- %10.3f%%;",gMC->VolName(id),fSummEnergy[j],fSum2Energy[j]);
      }
      printf("\n");
    }
    //
    // Relative energy loss in different detectors
    printf("******************** Relative Energy Loss per event ********************\n");
    printf("Total energy loss per event %10.3f GeV\n",edtot);
    for(kn=0;kn<(ndep-1)/5+1;kn++) {
      left=ndep-kn*5;
      for(i=0;i<(5<left?5:left);i++) {
	j=kn*5+i;
        id=Int_t (fEventEnergy[j]+0.1);
	printf(" %s %10.3f%%;",gMC->VolName(id),100*fSummEnergy[j]/edtot);
      }
      printf("\n");
    }
    for(kn=0;kn<75;kn++) printf("*"); 
    printf("\n");
  }
  //
  // Reset the TArray's
  //  fEventEnergy.Set(0);
  //  fSummEnergy.Set(0);
  //  fSum2Energy.Set(0);
}

//_____________________________________________________________________________
void AliMC::BeginEvent()
{
  //
  // Clean-up previous event
  // Energy scores
  AliDebug(1, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  AliDebug(1, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  AliDebug(1, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  AliDebug(1, "          BEGINNING EVENT               ");
  AliDebug(1, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  AliDebug(1, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  AliDebug(1, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    
  AliRunLoader *runloader=AliRunLoader::Instance();

  /*******************************/    
  /*   Clean after eventual      */
  /*   previous event            */
  /*******************************/    

  
  //Set the next event in Run Loader -> Cleans trees (TreeK and all trees in detectors),
  gAlice->SetEventNrInRun(gAlice->GetEventNrInRun()+1);
  runloader->SetEventNumber(gAlice->GetEventNrInRun());// sets new files, cleans the previous event stuff, if necessary, etc.,  
  AliDebug(1, Form("EventNr is %d",gAlice->GetEventNrInRun()));
     
  fEventEnergy.Reset();  
    // Clean detector information
  
  if (runloader->Stack())
      runloader->Stack()->Reset();//clean stack -> tree is unloaded
  else
      runloader->MakeStack();//or make a new one
  
  
  if(AliSimulation::Instance()->Lego() == 0x0)
  { 
      AliDebug(1, "fRunLoader->MakeTree(K)");
      runloader->MakeTree("K");
  }
  
  AliDebug(1, "gMC->SetStack(fRunLoader->Stack())");
  gMC->SetStack(runloader->Stack());//Was in InitMC - but was moved here 
                                     //because we don't have guarantee that 
                                     //stack pointer is not going to change from event to event
	                 //since it bellobgs to header and is obtained via RunLoader
  //
  //  Reset all Detectors & kinematics & make/reset trees
  //
    
  runloader->GetHeader()->Reset(AliCDBManager::Instance()->GetRun(),gAlice->GetEvNumber(),
				gAlice->GetEventNrInRun());
//  fRunLoader->WriteKinematics("OVERWRITE");  is there any reason to rewrite here since MakeTree does so

  if(AliSimulation::Instance()->Lego()) 
  {
      AliSimulation::Instance()->Lego()->BeginEvent();
      return;
  }
  

  AliDebug(1, "ResetHits()");
  ResetHits();
  
  AliDebug(1, "fRunLoader->MakeTree(H)");
  runloader->MakeTree("H");
  


  MakeTmpTrackRefsTree();
  //create new branches and SetAdresses
  TIter next(gAlice->Modules());
  AliModule *detector;
  while((detector = (AliModule*)next()))
   {
       AliDebug(2, Form("%s->MakeBranch(H)",detector->GetName()));
       detector->MakeBranch("H"); 
   }
}

//_______________________________________________________________________
void AliMC::ResetHits()
{
  //
  //  Reset all Detectors hits
  //
  TIter next(gAlice->Modules());
  AliModule *detector;
  while((detector = dynamic_cast<AliModule*>(next()))) {
     detector->ResetHits();
  }
}

//_______________________________________________________________________
void AliMC::ResetDigits()
{
  //
  //  Reset all Detectors digits
  //
  TIter next(gAlice->Modules());
  AliModule *detector;
  while((detector = dynamic_cast<AliModule*>(next()))) {
     detector->ResetDigits();
  }
}

//_______________________________________________________________________
void AliMC::ResetSDigits()
{
  //
  //  Reset all Detectors digits
  //
  TIter next(gAlice->Modules());
  AliModule *detector;
  while((detector = dynamic_cast<AliModule*>(next()))) {
     detector->ResetSDigits();
  }
}

//_______________________________________________________________________
void AliMC::PostTrack()
{
  // Posts tracks for each module

  TObjArray &dets = *gAlice->Modules();
  AliModule *module;
  
  for(Int_t i=0; i<=gAlice->GetNdets(); i++)
    if((module = dynamic_cast<AliModule*>(dets[i])))
      module->PostTrack();
}

//_______________________________________________________________________
void AliMC::FinishPrimary()
{
  //
  // Called  at the end of each primary track
  //

  AliRunLoader *runloader=AliRunLoader::Instance();
  //  static Int_t count=0;
  //  const Int_t times=10;
  // This primary is finished, purify stack
#if ROOT_VERSION_CODE > 262152
  if (!(gMC->SecondariesAreOrdered())) {
      if (runloader->Stack()->ReorderKine()) RemapHits();
  }
#endif
  if (runloader->Stack()->PurifyKine()) RemapHits();
  
  TIter next(gAlice->Modules());
  AliModule *detector;
  while((detector = dynamic_cast<AliModule*>(next()))) {
    detector->FinishPrimary();
    AliLoader* loader = detector->GetLoader();
    if(loader)
     {
       TTree* treeH = loader->TreeH();
       if (treeH) treeH->Fill(); //can be Lego run and treeH can not exist
     }
  }

  // Write out track references if any
  if (fTmpTreeTR) fTmpTreeTR->Fill();
}

void AliMC::RemapHits()
{
//    
// Remaps the track labels of the hits
    AliRunLoader *runloader=AliRunLoader::Instance();
    AliStack* stack = runloader->Stack();
    TList* hitLists = GetHitLists();
    TIter next(hitLists);
    TCollection *hitList;
    
    while((hitList = dynamic_cast<TCollection*>(next()))) {
	TIter nexthit(hitList);
	AliHit *hit;
	while((hit = dynamic_cast<AliHit*>(nexthit()))) {
	    hit->SetTrack(stack->TrackLabel(hit->GetTrack()));
	}
    }
    
    // 
    // This for detectors which have a special mapping mechanism
    // for hits, such as TPC and TRD
    //

    
    TObjArray* modules = gAlice->Modules();
    TIter nextmod(modules);
    AliModule *module;
    while((module = (AliModule*) nextmod())) {
	AliDetector* det = dynamic_cast<AliDetector*> (module);
	if (det) det->RemapTrackHitIDs(stack->TrackLabelMap());
    }
    //
    RemapTrackReferencesIDs(stack->TrackLabelMap());
}

//_______________________________________________________________________
void AliMC::FinishEvent()
{
  //
  // Called at the end of the event.
  //
    
  if(AliSimulation::Instance()->Lego()) AliSimulation::Instance()->Lego()->FinishEvent();

  TIter next(gAlice->Modules());
  AliModule *detector;
  while((detector = dynamic_cast<AliModule*>(next()))) {
    detector->FinishEvent();
  }

  //Update the energy deposit tables
  Int_t i;
  for(i=0;i<fEventEnergy.GetSize();i++) 
   {
    fSummEnergy[i]+=fEventEnergy[i];
    fSum2Energy[i]+=fEventEnergy[i]*fEventEnergy[i];
   }

  AliRunLoader *runloader=AliRunLoader::Instance();

  AliHeader* header = runloader->GetHeader();
  AliStack* stack = runloader->Stack();
  if ( (header == 0x0) || (stack == 0x0) )
   {//check if we got header and stack. If not cry and exit aliroot
    AliFatal("Can not get the stack or header from LOADER");
    return;//never reached
   }  
  // Update Header information 
  header->SetNprimary(stack->GetNprimary());
  header->SetNtrack(stack->GetNtrack());  
  header->SetTimeStamp(AliSimulation::Instance()->GenerateTimeStamp());

  // Write out the kinematics
  if (!AliSimulation::Instance()->Lego()) stack->FinishEvent();

  // Synchronize the TreeTR with TreeK
  if (fTmpTreeTR) ReorderAndExpandTreeTR();
   
  // Write out the event Header information
  TTree* treeE = runloader->TreeE();
  if (treeE) 
   {
      header->SetStack(stack);
      treeE->Fill();
   }
  else
   {
    AliError("Can not get TreeE from RL");
   }
  
  if(AliSimulation::Instance()->Lego() == 0x0)
   {
     runloader->WriteKinematics("OVERWRITE");
     runloader->WriteTrackRefs("OVERWRITE");
     runloader->WriteHits("OVERWRITE");
   }
   
  AliDebug(1, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  AliDebug(1, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  AliDebug(1, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  AliDebug(1, "          FINISHING EVENT               ");
  AliDebug(1, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  AliDebug(1, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  AliDebug(1, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}

//_______________________________________________________________________
void AliMC::Init()
{
  // MC initialization

   //=================Create Materials and geometry
   gMC->Init();
  // Set alignable volumes for the whole geometry (with old root)
#if ROOT_VERSION_CODE < 331527
  SetAllAlignableVolumes();
#endif
   //Read the cuts for all materials
   ReadTransPar();
   //Build the special IMEDIA table
   MediaTable();

   //Compute cross-sections
   gMC->BuildPhysics();
   
   //Initialise geometry deposition table
   fEventEnergy.Set(gMC->NofVolumes()+1);
   fSummEnergy.Set(gMC->NofVolumes()+1);
   fSum2Energy.Set(gMC->NofVolumes()+1);

   // Register MC in configuration 
   AliConfig::Instance()->Add(gMC);

}

//_______________________________________________________________________
void AliMC::MediaTable()
{
  //
  // Built media table to get from the media number to
  // the detector id
  //

  Int_t kz, nz, idt, lz, i, k, ind;
  //  Int_t ibeg;
  TObjArray &dets = *gAlice->Detectors();
  AliModule *det;
  Int_t ndets=gAlice->GetNdets();
  //
  // For all detectors
  for (kz=0;kz<ndets;kz++) {
    // If detector is defined
    if((det=dynamic_cast<AliModule*>(dets[kz]))) {
        TArrayI &idtmed = *(det->GetIdtmed()); 
        for(nz=0;nz<100;nz++) {
	    
	// Find max and min material number
	if((idt=idtmed[nz])) {
	  det->LoMedium() = det->LoMedium() < idt ? det->LoMedium() : idt;
	  det->HiMedium() = det->HiMedium() > idt ? det->HiMedium() : idt;
	}
      }
      if(det->LoMedium() > det->HiMedium()) {
	det->LoMedium() = 0;
	det->HiMedium() = 0;
      } else {
	if(det->HiMedium() > fImedia->GetSize()) {
	  AliError(Form("Increase fImedia from %d to %d",
			fImedia->GetSize(),det->HiMedium()));
	  return;
	}
	// Tag all materials in rage as belonging to detector kz
	for(lz=det->LoMedium(); lz<= det->HiMedium(); lz++) {
	  (*fImedia)[lz]=kz;
	}
      }
    }
  }
  //
  // Print summary table
  AliInfo("Tracking media ranges:");
  ToAliInfo(
  for(i=0;i<(ndets-1)/6+1;i++) {
    for(k=0;k< (6<ndets-i*6?6:ndets-i*6);k++) {
      ind=i*6+k;
      det=dynamic_cast<AliModule*>(dets[ind]);
      if(det)
	printf(" %6s: %3d -> %3d;",det->GetName(),det->LoMedium(),
	       det->HiMedium());
      else
	printf(" %6s: %3d -> %3d;","NULL",0,0);
    }
    printf("\n");
  }
	    );
}

//_______________________________________________________________________
void AliMC::ReadTransPar()
{
  //
  // Read filename to set the transport parameters
  //


  const Int_t kncuts=10;
  const Int_t knflags=12;
  const Int_t knpars=kncuts+knflags;
  const char kpars[knpars][7] = {"CUTGAM" ,"CUTELE","CUTNEU","CUTHAD","CUTMUO",
			       "BCUTE","BCUTM","DCUTE","DCUTM","PPCUTM","ANNI",
			       "BREM","COMP","DCAY","DRAY","HADR","LOSS",
			       "MULS","PAIR","PHOT","RAYL","STRA"};
  char line[256];
  char detName[7];
  char* filtmp;
  Float_t cut[kncuts];
  Int_t flag[knflags];
  Int_t i, itmed, iret, ktmed, kz;
  FILE *lun;
  //
  // See whether the file is there
  filtmp=gSystem->ExpandPathName(fTransParName.Data());
  lun=fopen(filtmp,"r");
  delete [] filtmp;
  if(!lun) {
    AliWarning(Form("File %s does not exist!",fTransParName.Data()));
    return;
  }
  //
  while(1) {
    // Initialise cuts and flags
    for(i=0;i<kncuts;i++) cut[i]=-99;
    for(i=0;i<knflags;i++) flag[i]=-99;
    itmed=0;
    memset(line,0,256);
    // Read up to the end of line excluded
    iret=fscanf(lun,"%255[^\n]",line);
    if(iret<0) {
      //End of file
      fclose(lun);
      return;
    }
    // Read the end of line
    iret=fscanf(lun,"%*c");
    if(!iret) continue;
    if(line[0]=='*') continue;
    // Read the numbers
    iret=sscanf(line,"%6s %d %f %f %f %f %f %f %f %f %f %f %d %d %d %d %d %d %d %d %d %d %d %d",
		detName,&itmed,&cut[0],&cut[1],&cut[2],&cut[3],&cut[4],&cut[5],&cut[6],&cut[7],&cut[8],
		&cut[9],&flag[0],&flag[1],&flag[2],&flag[3],&flag[4],&flag[5],&flag[6],&flag[7],
		&flag[8],&flag[9],&flag[10],&flag[11]);
    if(!iret) continue;
    if(iret<0) {
      //reading error
      AliWarning(Form("Error reading file %s",fTransParName.Data()));
      continue;
    }
    // Check that the module exist
    AliModule *mod = gAlice->GetModule(detName);
    if(mod) {
      // Get the array of media numbers
      TArrayI &idtmed = *mod->GetIdtmed();
      // Check that the tracking medium code is valid
      if(0<=itmed && itmed < 100) {
	ktmed=idtmed[itmed];
	if(!ktmed) {
	  AliWarning(Form("Invalid tracking medium code %d for %s",itmed,mod->GetName()));
	  continue;
	}
	// Set energy thresholds
	for(kz=0;kz<kncuts;kz++) {
	  if(cut[kz]>=0) {
	    AliDebug(2, Form("%-6s set to %10.3E for tracking medium code %4d for %s",
			     kpars[kz],cut[kz],itmed,mod->GetName()));
	    gMC->Gstpar(ktmed,kpars[kz],cut[kz]);
	  }
	}
	// Set transport mechanisms
	for(kz=0;kz<knflags;kz++) {
	  if(flag[kz]>=0) {
	    AliDebug(2, Form("%-6s set to %10d for tracking medium code %4d for %s",
			     kpars[kncuts+kz],flag[kz],itmed,mod->GetName()));
	    gMC->Gstpar(ktmed,kpars[kncuts+kz],Float_t(flag[kz]));
	  }
	}
      } else {
	AliWarning(Form("Invalid medium code %d",itmed));
	continue;
      }
    } else {
      AliDebug(1, Form("%s not present",detName));
      continue;
    }
  }
}

//_______________________________________________________________________
void AliMC::SetTransPar(const char *filename)
{
  //
  // Sets the file name for transport parameters
  //
  fTransParName = filename;
}

//_______________________________________________________________________
void AliMC::AddHit(Int_t id, Int_t track, Int_t *vol, Float_t *hits) const
{
  //
  //  Add a hit to detector id
  //
  TObjArray &dets = *gAlice->Modules();
  if(dets[id]) static_cast<AliModule*>(dets[id])->AddHit(track,vol,hits);
}

//_______________________________________________________________________
void AliMC::AddDigit(Int_t id, Int_t *tracks, Int_t *digits) const
{
  //
  // Add digit to detector id
  //
  TObjArray &dets = *gAlice->Modules();
  if(dets[id]) static_cast<AliModule*>(dets[id])->AddDigit(tracks,digits);
}

//_______________________________________________________________________
Int_t AliMC::GetCurrentTrackNumber() const {
  //
  // Returns current track
  //
  return AliRunLoader::Instance()->Stack()->GetCurrentTrackNumber();
}

//_______________________________________________________________________
void AliMC::DumpPart (Int_t i) const
{
  //
  // Dumps particle i in the stack
  //
  AliRunLoader * runloader = AliRunLoader::Instance();
   if (runloader->Stack())
    runloader->Stack()->DumpPart(i);
}

//_______________________________________________________________________
void AliMC::DumpPStack () const
{
  //
  // Dumps the particle stack
  //
  AliRunLoader * runloader = AliRunLoader::Instance();
   if (runloader->Stack())
    runloader->Stack()->DumpPStack();
}

//_______________________________________________________________________
Int_t AliMC::GetNtrack() const {
  //
  // Returns number of tracks in stack
  //
  Int_t ntracks = -1;
  AliRunLoader * runloader = AliRunLoader::Instance();
   if (runloader->Stack())
     ntracks = runloader->Stack()->GetNtrack();
   return ntracks;
}

//_______________________________________________________________________
Int_t AliMC::GetPrimary(Int_t track) const
{
  //
  // return number of primary that has generated track
  //
  Int_t nprimary = -999;
  AliRunLoader * runloader = AliRunLoader::Instance();
  if (runloader->Stack())
    nprimary = runloader->Stack()->GetPrimary(track);
  return nprimary;
}
 
//_______________________________________________________________________
TParticle* AliMC::Particle(Int_t i) const
{
  // Returns the i-th particle from the stack taking into account
  // the remaping done by PurifyKine
  AliRunLoader * runloader = AliRunLoader::Instance();
  if (runloader)
   if (runloader->Stack())
    return runloader->Stack()->Particle(i);
  return 0x0;   
}

//_______________________________________________________________________
const TObjArray* AliMC::Particles() const {
  //
  // Returns pointer to Particles array
  //
  AliRunLoader * runloader = AliRunLoader::Instance();
  if (runloader)
   if (runloader->Stack())
    return runloader->Stack()->Particles();
  return 0x0;
}

//_______________________________________________________________________
void AliMC::PushTrack(Int_t done, Int_t parent, Int_t pdg, const Float_t *pmom,
                      const Float_t *vpos, const Float_t *polar, Float_t tof,
                      TMCProcess mech, Int_t &ntr, Float_t weight, Int_t is) const
{ 
// Delegate to stack
//
  AliRunLoader * runloader = AliRunLoader::Instance();
  if (runloader)
    if (runloader->Stack())
      runloader->Stack()->PushTrack(done, parent, pdg, pmom, vpos, polar, tof,
				    mech, ntr, weight, is);
}

//_______________________________________________________________________
void AliMC::PushTrack(Int_t done, Int_t parent, Int_t pdg,
  	              Double_t px, Double_t py, Double_t pz, Double_t e,
  		      Double_t vx, Double_t vy, Double_t vz, Double_t tof,
		      Double_t polx, Double_t poly, Double_t polz,
		      TMCProcess mech, Int_t &ntr, Float_t weight, Int_t is) const
{ 
  // Delegate to stack
  //
  AliRunLoader * runloader = AliRunLoader::Instance();
  if (runloader)
    if (runloader->Stack())
      runloader->Stack()->PushTrack(done, parent, pdg, px, py, pz, e, vx, vy, vz, tof,
				    polx, poly, polz, mech, ntr, weight, is);
}

//_______________________________________________________________________
void AliMC::SetHighWaterMark(Int_t nt) const
{
    //
    // Set high water mark for last track in event
  AliRunLoader * runloader = AliRunLoader::Instance();
  if (runloader)
    if (runloader->Stack())
      runloader->Stack()->SetHighWaterMark(nt);
}

//_______________________________________________________________________
void AliMC::KeepTrack(Int_t track) const
{ 
  //
  // Delegate to stack
  //
  AliRunLoader * runloader = AliRunLoader::Instance();
  if (runloader)
    if (runloader->Stack())
      runloader->Stack()->KeepTrack(track);
}
 
//_______________________________________________________________________
void AliMC::FlagTrack(Int_t track) const
{
  // Delegate to stack
  //
  AliRunLoader * runloader = AliRunLoader::Instance();
  if (runloader)
    if (runloader->Stack())
      runloader->Stack()->FlagTrack(track);
}

//_______________________________________________________________________
void AliMC::SetCurrentTrack(Int_t track) const
{ 
  //
  // Set current track number
  //
  AliRunLoader * runloader = AliRunLoader::Instance();
  if (runloader)
    if (runloader->Stack())
      runloader->Stack()->SetCurrentTrack(track); 
}

//_______________________________________________________________________
AliTrackReference*  AliMC::AddTrackReference(Int_t label, Int_t id) 
{
  //
  // add a trackrefernce to the list
  Int_t primary = GetPrimary(label);
  Particle(primary)->SetBit(kKeepBit);

  Int_t nref = fTmpTrackReferences.GetEntriesFast();
  return new(fTmpTrackReferences[nref]) AliTrackReference(label, id);
}



//_______________________________________________________________________
void AliMC::ResetTrackReferences()
{
  //
  //  Reset all  references
  //
    fTmpTrackReferences.Clear();
}

//_______________________________________________________________________
void AliMC::RemapTrackReferencesIDs(const Int_t *map)
{
  // 
  // Remapping track reference
  // Called at finish primary
  //
    
  Int_t nEntries = fTmpTrackReferences.GetEntries();
  for (Int_t i=0; i < nEntries; i++){
      AliTrackReference * ref = dynamic_cast<AliTrackReference*>(fTmpTrackReferences.UncheckedAt(i));
      if (ref) {
	  Int_t newID = map[ref->GetTrack()];
	  if (newID>=0) ref->SetTrack(newID);
	  else {
	      ref->SetBit(kNotDeleted,kFALSE);
	      fTmpTrackReferences.RemoveAt(i);  
	  }      
      } // if ref
  }
  fTmpTrackReferences.Compress();
}

//_______________________________________________________________________
void AliMC::FixParticleDecaytime()
{
    //
    // Fix the particle decay time according to rmin and rmax for decays
    //

    TLorentzVector p;
    gMC->TrackMomentum(p);
    Double_t tmin, tmax;
    Double_t b;

    // Transverse velocity 
    Double_t vt    = p.Pt() / p.E();
    
    if ((b = ((AliMagF*)TGeoGlobalMagField::Instance()->GetField())->SolenoidField()) > 0.) {     // [kG]

	// Radius of helix
	
	Double_t rho   = p.Pt() / 0.0003 / b; // [cm]
	
	// Revolution frequency
	
	Double_t omega = vt / rho;
	
	// Maximum and minimum decay time
	//
	// Check for curlers first
	const Double_t kOvRhoSqr2 = 1./(rho*TMath::Sqrt(2.));
	if (fRDecayMax * kOvRhoSqr2 > 1.) return;
	
	//
 
	tmax  = TMath::ACos((1.-fRDecayMax*kOvRhoSqr2)*(1.+fRDecayMax*kOvRhoSqr2)) / omega;   // [ct]
	tmin  = TMath::ACos((1.-fRDecayMin*kOvRhoSqr2)*(1.+fRDecayMin*kOvRhoSqr2)) / omega;   // [ct]
    } else {
	tmax =  fRDecayMax / vt;                                                      // [ct] 
	tmin =  fRDecayMin / vt;	                                              // [ct]
    }
    
    //
    // Dial t using the two limits
    Double_t t = tmin + (tmax - tmin) * gRandom->Rndm();                              // [ct]
    //
    //
    // Force decay time in transport code
    //
    gMC->ForceDecayTime(t / 2.99792458e10);
}

void AliMC::MakeTmpTrackRefsTree()
{
    // Make the temporary track reference tree
    fTmpFileTR = new TFile("TrackRefsTmp.root", "recreate");
    fTmpTreeTR = new TTree("TreeTR", "Track References");
    TClonesArray* pRef = &fTmpTrackReferences;
    fTmpTreeTR->Branch("TrackReferences", &pRef, 4000);
}

//_______________________________________________________________________
void AliMC::ReorderAndExpandTreeTR()
{
//
//  Reorder and expand the temporary track reference tree in order to match the kinematics tree
//

    AliRunLoader *rl = AliRunLoader::Instance();
//
//  TreeTR
    AliDebug(1, "fRunLoader->MakeTrackRefsContainer()");
    rl->MakeTrackRefsContainer(); 
    TTree * treeTR = rl->TreeTR();
	// make branch for central track references
	TClonesArray* pRef = &fTrackReferences;
	treeTR->Branch("TrackReferences", &pRef);

    AliStack* stack  = rl->Stack();
    Int_t np = stack->GetNprimary();
    Int_t nt = fTmpTreeTR->GetEntries();
    //
    // Loop over tracks and find the secondaries with the help of the kine tree
    Int_t ifills = 0;
    Int_t it = 0;
    for (Int_t ip = np - 1; ip > -1; ip--) {
	TParticle *part = stack->Particle(ip);
	//printf("Particle %5d %5d %5d %5d %5d \n", ip, part->GetPdgCode(), part->GetFirstMother(), part->GetFirstDaughter(), part->GetLastDaughter());
	
	// Skip primaries that have not been transported
	Int_t dau1  = part->GetFirstDaughter();
	Int_t dau2  = -1;
	if (!part->TestBit(kTransportBit)) continue;
	//
	fTmpTreeTR->GetEntry(it++);
	Int_t nh = fTmpTrackReferences.GetEntries();
	// Determine range of secondaries produced by this primary
	if (dau1 > -1) {
	    Int_t inext = ip - 1;
	    while (dau2 < 0) {
		if (inext >= 0) {
		    part = stack->Particle(inext);
		    dau2 =  part->GetFirstDaughter();
		    if (!(part->TestBit(kTransportBit)) || dau2 == -1 || dau2 < np) {
//		    if (dau2 == -1 || dau2 < np) {
			dau2 = -1;
		    } else {
			dau2--;
		    }
		} else {
		    dau2 = stack->GetNtrack() - 1;
		}
		inext--;
	    } // find upper bound
	}  // dau2 < 0
//	printf("Check (1) %5d %5d %5d %5d %5d \n", ip, np, it, dau1, dau2);
	// 
	// Loop over reference hits and find secondary label
	for (Int_t id = dau1; (id <= dau2) && (dau1 > -1); id++) {
	    for (Int_t ih = 0; ih < nh; ih++) {
		AliTrackReference* tr = (AliTrackReference*) fTmpTrackReferences.At(ih);
		Int_t label = tr->Label();
		// Skip primaries
		if (label == ip) continue;
		if (label > dau2 || label < dau1) 
		    AliWarning(Form("Track Reference Label out of range !: %5d %5d %5d \n", label, dau1, dau2));
		if (label == id) {
		    // secondary found
		    Int_t nref =  fTrackReferences.GetEntriesFast();
		    new(fTrackReferences[nref]) AliTrackReference(*tr);
		}
	    } // hits
	    treeTR->Fill();
	    fTrackReferences.Clear();
	    ifills++;
	} // daughters
    } // tracks
    //
    // Now loop again and write the primaries
    it = nt - 1;
    for (Int_t ip = 0; ip < np; ip++) {
	TParticle* part = stack->Particle(ip);
//	if ((part->GetFirstDaughter() == -1 && part->GetStatusCode() <= 1) || part->GetFirstDaughter() >= np) 
	if (part->TestBit(kTransportBit))
	{
	    // Skip particles that have not been transported
	    fTmpTreeTR->GetEntry(it--);
	    Int_t nh = fTmpTrackReferences.GetEntries();
	    // 
	    // Loop over reference hits and find primary labels
	    for (Int_t ih = 0; ih < nh; ih++) {
		AliTrackReference* tr = (AliTrackReference*)  fTmpTrackReferences.At(ih);
		Int_t label = tr->Label();
		if (label == ip) {
		    Int_t nref = fTrackReferences.GetEntriesFast();
		    new(fTrackReferences[nref]) AliTrackReference(*tr);
		}
	    } 
	}
	treeTR->Fill();
	fTrackReferences.Clear();
	ifills++;
    } // tracks
    // Check
    if (ifills != stack->GetNtrack()) 
	AliWarning(Form("Number of entries in TreeTR (%5d) unequal to TreeK (%5d) \n", ifills, stack->GetNtrack()));
//
//  Clean-up
    delete fTmpTreeTR;
    fTmpFileTR->Close();
    delete fTmpFileTR;
    fTmpTrackReferences.Clear();
    gSystem->Exec("rm -rf TrackRefsTmp.root");
}

