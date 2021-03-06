// $Id$
//
// Root macro that opens a mini GUI for running aliroot with Geant4.
//      
//  To run aliroot with Geant4 using the g4menu.C:
//  aliroot
//  root [0] .x g4menu.C
//  --> First select "Geometry" to build geometry.root file
//  --> Then re-run aliroot and select "Run" button
//	    
// The Init button is kept for debugging purposes,
// it itializes MonteCarlo but it does not itialize
// completely ALICE framework. That's why to run simulation,
// you have to re-run aliroot and select Run button.
//
// The menu enables to start Geant4 interactive session:
//  --> Select "Geant4UI" button and use Geant4 interactive commands;
// To go back to Root UI, type exit.
//
// By I. Hrivnacova, IPN Orsay


#include <iostream>

void g4menu()
{

  // Load Geant4 libraries 
  if (!gInterpreter->IsLoaded("$ALICE/geant4_vmc/examples/macro/g4libs.C"))
    gROOT->LoadMacro("$ALICE/geant4_vmc/examples/macro/g4libs.C");
  gInterpreter->ProcessLine("g4libs()");

  // Load ALICE Geant4 library
  //cout << "Loading g4alice library ..." << endl;
  //gSystem->Load("libg4alice");

  // Menu
  TControlBar* menu = new TControlBar("vertical","Alice Geant4 menu");
  
  menu->AddButton("Geometry", "MakeGeometry()",  "Generate Root geometry file");
  menu->AddButton("Run G4",   "RunG4Simulation()",  "Process Alice run");
  menu->AddButton("Run G4 batch",   "RunG4Simulation(); >& g4.out",  "Process Alice run");
  menu->AddButton("Run G3",   "RunG3Simulation()",  "Process Alice run");
  menu->AddButton("Run G3 batch",   "RunG3Simulation(); >& g3.out",  "Process Alice run");
  menu->AddButton("Init",     "Init();",  "Initialize Alice for G4 simulation");
  menu->AddButton("Init batch","Init(); >& g4init.out",  "Initialize Alice for G4 simulation");
  menu->AddButton("Geant4UI", "StartGeant4UI()","Go to Geant4 Interactive session");
  menu->AddButton("AGDD",     "GenerateAGDD()","Generate XML (AGDD) file with geometry description");
  //menu->AddButton("GDML",     "GenerateGDML()","Generate XML (GDML) file with geometry description");
  menu->AddButton("Quit",     "Quit()", "Quit aliroot");
  gROOT->SaveContext();
  
  cout << endl
       << "**************************************************************" << endl
       << "  To run simulation:"                                           << endl
       << "  First select <Geometry> to build geometry.root file."         << endl
       << "  Then re-run aliroot and select <Run> button"                  << endl
       << endl
       << "  The <Init> button is kept for debugging purposes,"            << endl
       << "  it itializes MonteCarlo but it does not itialize"             << endl
       << "  completely ALICE framework. That's why to run simulation,"    << endl
       << "  you have to re-run aliroot and select Run button."            << endl
       << endl
       << "  The menu enables to start Geant4 interactive session:"        << endl
       << "  Select <Geant4UI> button and use Geant4 interactive commands" << endl
       << "  To go back to Root UI, type exit."                            << endl
       << "**************************************************************" << endl
       << endl;
  
  menu->Show();
}

void MakeGeometry()
{  
  AliCDBManager* man = AliCDBManager::Instance();
  man->SetDefaultStorage("local://$ALICE_ROOT/OCDB");
  man->SetRun(1);

  // MC application initialization
  TString configFileName = "$ALICE_ROOT/macros/g4ConfigGeometry.C";
  gROOT->LoadMacro(configFileName.Data());
  gInterpreter->ProcessLine(gAlice->GetConfigFunction());
  gAlice->GetMCApp()->Init();
  
  // Generate geometry file
  //
  gGeoManager->Export("geometry.root");
  
  cout << endl
       << "Geometry file geometry.root has been generated." << endl
       << "You have to re-run aliroot and choose Run in g4menu." << endl;
       
  exit(0);     
}    


void Init()
{  
  AliCDBManager* man = AliCDBManager::Instance();
  man->SetDefaultStorage("local://$ALICE_ROOT/OCDB");
  man->SetRun(0);
 
  // MC application initialization
  TString configFileName = "$ALICE_ROOT/macros/g4Config.C";
  gROOT->LoadMacro(configFileName.Data());
  gInterpreter->ProcessLine(gAlice->GetConfigFunction());
  gAlice->GetMCApp()->Init();

  ((TGeant4*)gMC)->ProcessGeantCommand("/mcDet/printMedia");

  cout << endl
       << "Only MonteCarlo initialization has been performed. " << endl
       << "To run simulation you have to re-run aliroot and choose Run in g4menu." << endl;
}    


void RunG4Simulation()
{  
  AliSimulation sim("$ALICE_ROOT/macros/g4Config.C");
  sim.SetMakeSDigits("");
  sim.SetMakeDigits("");
  //sim.SetMakeDigitsFromHits("ITS TPC");
  //sim.SetMakeDigitsFromHits("ITS");
  sim.SetMakeDigitsFromHits("");
  sim.SetRunHLT("");
  sim.SetNumberOfEvents(1000);
  TStopwatch timer;
  timer.Start();
  sim.Run(1);
  timer.Stop();
  timer.Print();
}    

void RunG3Simulation()
{  
  AliSimulation sim("$ALICE_ROOT/macros/g3Config.C");
  sim.SetMakeSDigits("");
  sim.SetMakeDigits("");
  //sim.SetMakeDigitsFromHits("ITS TPC");
  //sim.SetMakeDigitsFromHits("ITS");
  sim.SetMakeDigitsFromHits("");
  sim.SetRunHLT("");
  sim.SetNumberOfEvents(1000);
  TStopwatch timer;
  timer.Start();
  sim.Run(1);
  timer.Stop();
  timer.Print();
}    

void StartGeant4UI()
{
  if (gMC) {
    // release Root terminal control

    // go into non-raw term mode
    Getlinem(kCleanUp, 0);
    
    // add test if gMC is TGeant4
    TGeant4* g4 = (TGeant4*)gMC;
  
    g4->StartGeantUI();

    // new Root prompt
    Getlinem(kInit, ((TRint*)gROOT->GetApplication())->GetPrompt());  
  }
  else {  
    cout << "Monte Carlo has not been yet created." << endl;
  }       
} 

void GenerateAGDD()
{
  if (gMC) {
    // release Root terminal control

    // go into non-raw term mode
    //Getlinem(kCleanUp, 0);
    
    // add test if gMC is TGeant4
    TGeant4* g4 = (TGeant4*)gMC;
    
    g4->ProcessGeantCommand("/vgm/generateAGDD");

    // new Root prompt
    //Getlinem(kInit, ((TRint*)gROOT->GetApplication())->GetPrompt());  
  }
  else {  
    cout << "Monte Carlo has not been yet created." << endl;
  } 
}        
/*
void GenerateGDML()
{
  if (gMC) {
    // release Root terminal control

    // go into non-raw term mode
    //Getlinem(kCleanUp, 0);
    
    // add test if gMC is TGeant4
    TGeant4* g4 = (TGeant4*)gMC;
    
    g4->ProcessGeantCommand("/vgm/generateGDML");

    // new Root prompt
    //Getlinem(kInit, ((TRint*)gROOT->GetApplication())->GetPrompt());  
  }
  else {  
    cout << "Monte Carlo has not been yet created." << endl;
  } 
}        
*/
void Quit()
{
  delete AliRunLoader::Instance();
  delete gAlice;
  
  exit(0);
}  
