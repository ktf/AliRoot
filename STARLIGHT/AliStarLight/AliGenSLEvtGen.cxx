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
***************************************************************************/
///////////////////////////////////////////////////////////////////////////
//  Class to generate decays of particles generated by a                 //
//  previous generator. It works as a generator, but pratically it       //
//  performs only decays. It works with this scheme: first loops over    // 
//  particles on the stack, selects those to be decayed, decays them     //
//  and then pushes the decay products on the stack.                     //
//                                                                       //
//  Origin: Michal.Broz@cern.ch        					 //
///////////////////////////////////////////////////////////////////////////

#include "AliStack.h"
#include "AliGenSLEvtGen.h"
#include "AliRun.h"
#include "AliLog.h"
#include <TParticle.h>

ClassImp(AliGenSLEvtGen)
//____________________________________________________________________________
AliGenSLEvtGen::AliGenSLEvtGen(): 
  fStack(0x0),    
  fDecayer(0x0),
  fUserDecay(kFALSE),
  fUserDecayTablePath(0x0),
  fEvtGenNumber(0), 
  fDecayPolarized(kFALSE),
  fPolarization(0),
  fEtaChildMin( 1),
  fEtaChildMax(-1) 
  {
  //
  // Default Construction
  //
  }
//____________________________________________________________________________
AliGenSLEvtGen::~AliGenSLEvtGen() 
  {
  //
  // Standard Destructor
  //
  if(fStack) {delete fStack;}
  fStack = 0; 
  if(fDecayer) {delete fDecayer;}
  fDecayer = 0;
  if(fUserDecayTablePath) {delete fUserDecayTablePath;}
  fUserDecayTablePath = 0;
  }
//____________________________________________________________________________

void AliGenSLEvtGen::Init() 
  {
  //
  // Standard AliGenerator Initializer - no input 
  // 1) initialize SLEvtGen with default decay and particle table
  // 2) set the decay mode to force particle 
  // 3) set a user decay table if defined
  //
  if(fDecayer)
  {
  AliWarning("AliGenSLEvtGen already initialized!!!");
  return;
  }
  fDecayer = new AliDecayerSLEvtGen();
  fDecayer->Init(); //read the default decay table DECAY.DEC and particle table

  //if is defined a user decay table
  if(fUserDecay) 
    {
    fDecayer->SetDecayTablePath(fUserDecayTablePath);
    fDecayer->ReadDecayTable();
    }
  }

//____________________________________________________________________________
void AliGenSLEvtGen::Generate() 
  {
  //
  //Generate method - Input none - Output none
  //For each event:
  //1)return the stack of the previous generator and select particles to be decayed by SLEvtGen
  //2)decay particles selected and put the decay products on the stack
  //
  //
  Float_t Polarization[3]= {0,0,0};  // Polarisation of daughter particles 
  Float_t VertexPos[3];         // Origin of the parent particle 
  Float_t Momentum[3]; // Momentum and origin of the children particles from SLEvtGen
  Int_t nt(0);
  Float_t Tof;
  Int_t nPartStarlight;
  TLorentzVector momGen;
  TLorentzVector momTot;
  momTot.SetXYZM(0.,0.,0.,0.);
  
  static TClonesArray *decayProducts;
  if(!decayProducts) decayProducts = new TClonesArray("TParticle",1000);
  fStack = AliRunLoader::Instance()->Stack();
  if(!fStack) {Info("Generate","Error: No stack found!"); return;}
  
  nPartStarlight = fStack->GetNprimary();  
  //AliInfo(Form("nPartStarlight = %d \n",nPartStarlight));
  
  //Loop over Starlight decay products
  for (Int_t ipartStarlight = 0; ipartStarlight < nPartStarlight; ++ipartStarlight) {
	
  	TParticle *partStarlight = fStack->Particle(ipartStarlight);
	VertexPos[0]=partStarlight->Vx(); 
  	VertexPos[1]=partStarlight->Vy(); 
  	VertexPos[2]=partStarlight->Vz(); 
  	momGen.SetXYZM(partStarlight->Px(),partStarlight->Py(),partStarlight->Pz(),partStarlight->GetMass());
  	momTot += momGen;
  	}
  
  fStack->Clean(); //We are re-decaying, Remove previous particles 
  
  Int_t nProducts;
  Bool_t  genOK = kFALSE;  
  // generate events until all constraints are fulfilled
  for (Int_t trials=0; !genOK && trials < 100*1000; ++trials) {
  	if(fDecayPolarized)fDecayer->DecayPolarized(fEvtGenNumber,&momTot,fPolarization);
  	else fDecayer->Decay(fEvtGenNumber,&momTot);
  	nProducts = fDecayer->ImportParticles(decayProducts);
	genOK = kTRUE;
  	for (int i = 0; i < nProducts; i++) {
         	TParticle* partEvtGen = (TParticle *) decayProducts->At(i);

	 	//AliInfo(Form("PDG = %d, Status = %d \n",partEvtGen->GetPdgCode(),partEvtGen->GetStatusCode()));
	 	if(partEvtGen->GetStatusCode() !=1) continue;//don't fill mother particles for time being
		
		if (fEtaChildMin <= fEtaChildMax) genOK = genOK && (partEvtGen->Eta() >= fEtaChildMin && partEvtGen->Eta() <  fEtaChildMax);
      		if (!genOK) break;
		}// Particle loop - acceptance
	if (!genOK) continue;
	
	//AliInfo(Form("nProducts = %d, Weight = %d \n",nProducts,trials+1));
  	// Put decay products on the stack
	for (int i = 0; i < nProducts; i++) {
         	TParticle* partEvtGen = (TParticle *) decayProducts->At(i);

	 	//AliInfo(Form("PDG = %d, Status = %d \n",partEvtGen->GetPdgCode(),partEvtGen->GetStatusCode()));
	 	if(partEvtGen->GetStatusCode() !=1) continue;//don't fill mother particles for time being
		
         	PushTrack(1, -1, partEvtGen->GetPdgCode(),
	 	   partEvtGen->Px(),partEvtGen->Py(),partEvtGen->Pz(),partEvtGen->Energy(),
		   VertexPos[0],VertexPos[1],VertexPos[2],partEvtGen->T(),
		   Polarization[0],Polarization[1],Polarization[2],
	  	   kPPrimary, nt, trials+1, partEvtGen->GetStatusCode());
		    
         	KeepTrack(nt);
         	}// Particle loop - stack
         decayProducts->Clear();
	 }
	  
  }

//____________________________________________________________________________
Bool_t AliGenSLEvtGen::SetUserDecayTable(Char_t *path)
  {
  //
  //Set the path of user decay table if it is defined
  //
  //put a comment to control if path exists 
  if(gSystem->AccessPathName(path))
    {
    AliWarning("Attention: This path not exist!\n");
    return kFALSE;
    }
  fUserDecayTablePath = path;
  fUserDecay = kTRUE;
  return kTRUE;
  }
