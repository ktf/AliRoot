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
 
/*
$Log$
Revision 1.1  2001/11/27 16:27:28  nilsen
Adding AliITSDigitizer class to do merging and digitization . Based on the
TTask method. AliITSDigitizer class added to the Makefile and ITSLinkDef.h
file. The following files required minor changes. AliITS, added functions
SetHitsAddressBranch, MakeBranchInTreeD and modified MakeBranchD.
AliITSsimulationSDD.cxx needed a Tree indepenent way of returning back to
the original Root Directory in function Compress1D. Now it uses gDirectory.

Revision 1.2  2002/03/01  E. Lopez
Diditization changed to start from SDigits instead of Hits.
The SDigits are reading as TClonesArray of AliITSpListItem
*/

#include <stdlib.h>
#include <iostream.h>
#include <TObjArray.h>
#include <TTree.h>
#include <TBranch.h>

#include <AliRun.h>
#include <AliRunDigitizer.h>

#include "AliITSDigitizer.h"
#include "AliITShit.h"
#include "AliITSmodule.h"
#include "AliITSsimulation.h"
#include "AliITSDetType.h"
#include "AliITSgeom.h"

ClassImp(AliITSDigitizer)

//______________________________________________________________________
AliITSDigitizer::AliITSDigitizer() : AliDigitizer(){
    // Default constructor. Assign fITS since it is never written out from
    // here. 
    // Inputs:
    //      Option_t * opt   Not used
    // Outputs:
    //      none.
    // Return:
    //      A blank AliITSDigitizer class.

    fITS    = 0;
    fActive = 0;
}
//______________________________________________________________________
AliITSDigitizer::AliITSDigitizer(AliRunDigitizer *mngr) : AliDigitizer(mngr){
    // Standard constructor. Assign fITS since it is never written out from
    // here. 
    // Inputs:
    //      Option_t * opt   Not used
    // Outputs:
    //      none.
    // Return:
    //      An AliItSDigitizer class.

    if(!gAlice) {
	fITS = 0;
	fActive = 0;
	return;
    } // end if
    fITS = (AliITS *)(gAlice->GetDetector("ITS"));
    if(!fITS){
	fActive = 0;
	return;
    } else if(fITS->GetITSgeom()){
	fActive = new Bool_t[fITS->GetITSgeom()->GetIndexMax()];
    } else{
	fActive = 0;
	return;
    } // end if
    // fActive needs to be set to a default all kTRUE value
    for(Int_t i=0;i<fITS->GetITSgeom()->GetIndexMax();i++) fActive[i] = kTRUE;
}
//______________________________________________________________________
AliITSDigitizer::~AliITSDigitizer(){
    // Default destructor. 
    // Inputs:
    //      Option_t * opt   Not used
    // Outputs:
    //      none.
    // Return:
    //      none.

    fITS = 0; // don't delete fITS. Done else where.
    if(fActive) delete[] fActive;
}
//______________________________________________________________________
Bool_t AliITSDigitizer::Init(){
    // Iniliztion 
    // Inputs:
    //      none.
    // Outputs:
    //      none.
    // Return:
    //      none.
    return kTRUE;
}
//______________________________________________________________________
void AliITSDigitizer::Exec(Option_t* opt){
    // Main digitizing function. 
    // Inputs:
    //      Option_t * opt   list of subdetector to digitize. =0 all.
    // Outputs:
    //      none.
    // Return:
    //      none.

    char name[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    char *all;
    const char *det[3] = {strstr(opt,"SPD"),strstr(opt,"SDD"),
                          strstr(opt,"SSD")};
    if( !det[0] && !det[1] && !det[2] ) all = "All";
    else all = 0;
    AliITSsimulation *sim      = 0;
    AliITSDetType    *iDetType = 0;
    static Bool_t    setDef    = kTRUE;

    if( !fITS ) fITS = (AliITS*)(gAlice->GetDetector("ITS"));
    if(!fITS){
	Error("Exec","The ITS not found. aborting.");
	return;
    } // end if
    if( !(fITS->GetITSgeom()) ) {
        Warning( "Exec", "Need ITS geometry to be properly defined first." );
        return; // need transformations to do digitization.
    } // end if !GetITSgeom()

    if( setDef ) fITS->SetDefaultSimulation();
    setDef = kFALSE;
    sprintf( name, "%s", fITS->GetName() );

    Int_t nfiles = GetManager()->GetNinputs();
    Int_t event  = GetManager()->GetOutputEventNr();
    Int_t size   = fITS->GetITSgeom()->GetIndexMax();
    TClonesArray * sdig = new TClonesArray( "AliITSpListItem",1000 );
    
    // Digitize 
    fITS->MakeBranchInTreeD( GetManager()->GetTreeD() );

    for( Int_t module=0; module<size; module++ ){
	if(fActive) if(!fActive[module]) continue;
        Int_t id = fITS->GetITSgeom()->GetModuleType( module );
        if( !all && !det[id] ) continue;
        iDetType = fITS->DetType( id );
        sim      = (AliITSsimulation*)iDetType->GetSimulationModel();
        if( !sim ) {
            Error( "Exec", "The simulation class was not instanciated!" );
            exit(1);
        } // end if !sim
        
        // Fill the module with the sum of SDigits
        sim->InitSimulationModule( module, event );
	cout << "Module=" << module;
        for( Int_t ifiles=0; ifiles<nfiles; ifiles++ ){
	    cout <<" ifiles=" << ifiles;
            TTree *treeS = GetManager()->GetInputTreeS( ifiles );
            if( !(treeS && fITS->GetSDigits()) ) continue;   
            TBranch *brchSDigits = treeS->GetBranch( name );
            if( brchSDigits ) {
                brchSDigits->SetAddress( &sdig ); 
            } else {
                Error( "Exec", "branch ITS not found in TreeS, input file %d ",
                       ifiles );
            } // end if brchSDigits
            sdig->Clear();
            Int_t mask = GetManager()->GetMask( ifiles );
            
            // add summable digits to module
            brchSDigits->GetEvent( module );
            sim->AddSDigitsToModule( sdig, mask );    
        } // end for ifiles
	cout << " end ifiles loop" << endl;
        // Digitise current module sum(SDigits)->Digits
        sim->FinishSDigitiseModule();

        // fills all branches - wasted disk space
        GetManager()->GetTreeD()->Fill();
        fITS->ResetDigits();
    } // end for module
    cout << "end modules loop"<<endl;
 
    GetManager()->GetTreeD()->GetEntries();
    GetManager()->GetTreeD()->Write( 0, TObject::kOverwrite );
    // reset tree
    GetManager()->GetTreeD()->Reset();
    
    sdig->Clear();
    delete sdig;
}
