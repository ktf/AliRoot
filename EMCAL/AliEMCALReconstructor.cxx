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

//_________________________________________________________________________
//*--
//*-- Yves Schutz (SUBATECH) 
// Reconstruction class. Redesigned from the old AliReconstructionner class and 
// derived from STEER/AliReconstructor. 
// 
// --- ROOT system ---

// --- Standard library ---

// --- AliRoot header files ---
#include "AliESD.h"
#include "AliEMCALReconstructor.h"
#include "AliEMCALClusterizerv1.h"
#include "AliEMCALPIDv1.h"
#include "AliEMCALGetter.h"

ClassImp(AliEMCALReconstructor)

//____________________________________________________________________________
  AliEMCALReconstructor::AliEMCALReconstructor() : fDebug(kFALSE) 
{
  // ctor

} 


//____________________________________________________________________________
AliEMCALReconstructor::~AliEMCALReconstructor()
{
  // dtor
} 

//____________________________________________________________________________
void AliEMCALReconstructor::Reconstruct(AliRunLoader* runLoader) const 
{
  // method called by AliReconstruction; 
  // Only the clusterization is performed,; the rest of the reconstruction is done in FillESD because the track
  // segment maker needs access to the AliESD object to retrieve the tracks reconstructed by 
  // the global tracking.
 
  TString headerFile(runLoader->GetFileName()) ; 
  TString branchName("Default") ;  
  
  AliEMCALClusterizerv1 clu(headerFile, branchName);
  clu.SetEventRange(0, -1) ; // do all the events
  if ( Debug() ) 
    clu.ExecuteTask("deb all") ; 
  else 
    clu.ExecuteTask("") ;  

  AliEMCALGetter::Instance()->EmcalLoader()->CleanReconstructioner();
}

//____________________________________________________________________________
void AliEMCALReconstructor::FillESD(AliRunLoader* runLoader, AliESD* esd) const
{
  // Called by AliReconstruct after Reconstruct() and global tracking and vertxing 
  //Creates the tracksegments and Recparticles
  
  TString headerFile(runLoader->GetFileName()) ; 
  TString branchName("Default") ;  

  AliEMCALPIDv1 pid(headerFile, branchName);

  Int_t eventNumber = runLoader->GetEventNumber() ;
  // do current event; the loop over events is done by AliReconstruction::Run()
  Info("FillESD 1", "%d", eventNumber) ;
  pid.SetEventRange(eventNumber, eventNumber) ; 
  if ( Debug() ) 
   pid.ExecuteTask("deb all") ;
  else 
    pid.ExecuteTask("") ;
  
  // Creates AliESDtrack from AliEMCALRecParticles 
  AliEMCALGetter::Instance()->Event(eventNumber, "P") ; 
  TClonesArray *recParticles = AliEMCALGetter::Instance()->RecParticles();
  Int_t nOfRecParticles = recParticles->GetEntries();
  for (Int_t recpart = 0 ; recpart < nOfRecParticles ; recpart++) {
    AliEMCALRecParticle * rp = dynamic_cast<AliEMCALRecParticle*>(recParticles->At(recpart));
    if (Debug()) 
      rp->Print();
    AliESDtrack * et = new AliESDtrack() ; 
    // fills the ESDtrack
    Double_t xyz[3];
    for (Int_t ixyz=0; ixyz<3; ixyz++) 
      xyz[ixyz] = rp->GetPos()[ixyz];
    //et->SetEMCALposition(xyz) ; 
    //et->SetEMCALsignal  (rp->Energy()) ; 
    //et->SetEMCALpid     (rp->GetPID()) ;
    // add the track to the esd object
    esd->AddTrack(et);
    delete et;
  }
}
