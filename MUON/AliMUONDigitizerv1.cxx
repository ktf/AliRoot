
#include <Riostream.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TObjArray.h>
#include <TPDGCode.h>
#include <TTree.h> 
#include <TMath.h>

#include "AliRun.h"
#include "AliRunDigitizer.h"
#include "AliRunLoader.h"
#include "AliLoader.h"

#include "AliMUON.h"
#include "AliMUONChamber.h"
#include "AliMUONConstants.h"
#include "AliMUONDigit.h"
#include "AliMUONDigitizerv1.h"
#include "AliMUONHit.h"
#include "AliMUONHitMapA1.h"
#include "AliMUONPadHit.h"
#include "AliMUONTransientDigit.h"

/////////////////////////////////////////////////////////////////////////////////
//
// AliMUONDigitizerv1 implements a full digitizer to digitize digits directly
// from hits. It also performs merging over several input streams. 
// The first input stream is assumed to be the signal and all other input
// streams are assumed to be background. SDigits are never generated by this
// digitizer and one should use the AliMUONSDigitizerv1 for that task. 
//
// NOTE: This digitizer will become depricated in the future in favour of
//       AliMUONSDigitizerv1 and AliMUONDigitizerv2.
//
/////////////////////////////////////////////////////////////////////////////////

ClassImp(AliMUONDigitizerv1)

//___________________________________________
AliMUONDigitizerv1::AliMUONDigitizerv1() : AliMUONDigitizer()
{
	// Default ctor - don't use it
}

//___________________________________________
AliMUONDigitizerv1::AliMUONDigitizerv1(AliRunDigitizer* manager) : AliMUONDigitizer(manager)
{
	// ctor which should be used
}

//___________________________________________
AliMUONDigitizerv1::~AliMUONDigitizerv1()
{
	// Destructor
}

//-----------------------------------------------------------------------
void AliMUONDigitizerv1::GenerateTransientDigits()
{
// Loops over all tracks and hits in the current selected event and calls 
// MakeTransientDigitsFromHit for each hit. 
// Note: Charge correlation is applied to the tracking chambers. 

	TTree* treeH = gime->TreeH();
	if (GetDebug() > 1)
		Info("GenerateTransientDigits", "Generating transient digits using treeH = 0x%X"
			, (void*)treeH);
	//
	// Loop over tracks
	Int_t ntracks = (Int_t) treeH->GetEntries();
	for (Int_t itrack = 0; itrack < ntracks; itrack++) 
	{
		if (GetDebug() > 2) Info("GenerateTransientDigits", "Processing track %d...", itrack);
		muondata->ResetHits();
		treeH->GetEvent(itrack);
		//
		//  Loop over hits
		TClonesArray* hits = muondata->Hits();
		for (Int_t ihit = 0; ihit < hits->GetEntriesFast(); ihit++) 
		{
			AliMUONHit* mHit = static_cast<AliMUONHit*>( hits->At(ihit) );
			Int_t ichamber = mHit->Chamber()-1;  // chamber number
			if (ichamber > AliMUONConstants::NCh()-1) 
			{
				Error("GenerateTransientDigits", 
					"Hit 0x%X has a invalid chamber number: %d", ichamber);
				continue;
			}
			//
			//Dumping Hit content:
			if (GetDebug() > 2) 
			{
				Info("GenerateTransientDigits", 
					"Hit %d: chamber = %d\tX = %f\tY = %f\tZ = %f\teloss = %f",
					ihit, mHit->Chamber(), mHit->X(), mHit->Y(), mHit->Z(), mHit->Eloss()
				    );
			}
			// 
			// Inititializing Correlation
			AliMUONChamber& chamber = pMUON->Chamber(ichamber);
			chamber.ChargeCorrelationInit();
			if (ichamber < AliMUONConstants::NTrackingCh()) 
			{
				// Tracking Chamber
				// Initialize hit position (cursor) in the segmentation model 
				chamber.SigGenInit(mHit->X(), mHit->Y(), mHit->Z());
			}; // else do nothing for Trigger Chambers
			
			MakeTransientDigitsFromHit(itrack, ihit, mHit);
		} // hit loop
	} // track loop      
};

//--------------------------------------------------------------------------
void AliMUONDigitizerv1::MakeTransientDigitsFromHit(Int_t track, Int_t iHit, AliMUONHit * mHit)
{  
// This method is called for every hit in an event to generate AliMUONTransientDigits 
// from the hit and add these to fTDList.
// The AliMUONChamber::DisIntegration method us used to figure out which pads are 
// fired for a given hit. We then loop over the fired pads and add an AliMUONTransientDigit
// for each pad.

	if (GetDebug() > 3)
		Info("MakeTransientDigitsFromHit", "Making transient digit for hit number %d.", iHit);
		
	//
	// Calls the charge disintegration method of the current chamber 
	if (GetDebug() > 4)
		Info("MakeTransientDigitsFromHit", "Calling AliMUONChamber::DisIngtegration...");

	Float_t newdigit[6][500];  // Pad information
	Int_t nnew=0;              // Number of touched Pads per hit
	Int_t ichamber = mHit->Chamber()-1;
	AliMUONChamber& chamber = pMUON->Chamber(ichamber);
	chamber.DisIntegration(mHit->Eloss(), mHit->Age(), mHit->X(), mHit->Y(), mHit->Z(), nnew, newdigit);

	// Creating new TransientDigits from hit
	for(Int_t iTD = 0; iTD < nnew; iTD++) 
	{
		Int_t charge;   
		Int_t digits[6];
		
		digits[0] = Int_t(newdigit[1][iTD]);  // Padx of the Digit
		digits[1] = Int_t(newdigit[2][iTD]);  // Pady of the Digit
		digits[2] = Int_t(newdigit[5][iTD]);  // Cathode plane
		digits[3] = Int_t(newdigit[3][iTD]);  // Induced charge in the Pad
		if (fSignal)
		{ 
			charge = digits[3];
			digits[4] = Int_t(newdigit[3][iTD]);  // Signal due to physics
		}
		else
		{
			charge = digits[3] + fMask;
			digits[4] = 0;    // No signal due to physics since this is now background.
		};
		digits[5] = iHit+fMask;    // Hit number in the list

		if (GetDebug() > 4)
			Info("MakeTransientDigitsFromHit", 
				"DisIntegration result %d: PadX %d\tPadY %d\tPlane %d\tCharge %d\tHit %d",
				iTD, digits[0], digits[1], digits[2], digits[3], digits[5]);

		AliMUONTransientDigit* mTD = new AliMUONTransientDigit(ichamber, digits);
		mTD->AddToTrackList(track + fMask, charge);

		OnCreateTransientDigit(mTD, mHit);
		AddOrUpdateTransientDigit(mTD);
	};
};

//------------------------------------------------------------------------
void AliMUONDigitizerv1::AddDigit(Int_t chamber, Int_t tracks[kMAXTRACKS], Int_t charges[kMAXTRACKS], Int_t digits[6])
{
// Derived to add digits to TreeD.
	muondata->AddDigit(chamber, tracks, charges, digits);   
};

//------------------------------------------------------------------------
Int_t AliMUONDigitizerv1::GetSignalFrom(AliMUONTransientDigit* td)
{
// Derived to apply the chamber response model to the digit. 
// Using AliMUONChamber::ResponseModel() for this.

	if (GetDebug() > 3)
		Info("GetSignalFrom", "Applying response of chamber to TransientDigit signal.");
	//
	//  Digit Response (noise, threshold, saturation, ...)
	Int_t q = td->Signal(); 
	AliMUONChamber& chamber = pMUON->Chamber(td->Chamber());
	AliMUONResponse* response = chamber.ResponseModel();
	q = response->DigitResponse(q, td);
	return q;
};

//------------------------------------------------------------------------
Bool_t AliMUONDigitizerv1::InitOutputData(AliMUONLoader* muonloader)
{
// Derived to initialize the output digits tree TreeD, create it if necessary
// and sets the muondata tree address to treeD.

	if (GetDebug() > 2)
		Info("InitOutputData", "Creating digits branch and setting the tree address.");

	muondata->SetLoader(muonloader);

	// New branch per chamber for MUON digit in the tree of digits
	if (muonloader->TreeD() == NULL)
	{
		muonloader->MakeDigitsContainer();
		if (muonloader->TreeD() == NULL)
		{
			Error("InitOutputData", "Could not create TreeD.");
			return kFALSE;
		};
	};

	muondata->MakeBranch("D");
	muondata->SetTreeAddress("D");
	
	return kTRUE;
};

//------------------------------------------------------------------------
void AliMUONDigitizerv1::FillOutputData()
{
// Derived to fill TreeD and resets the digit array in muondata.

	if (GetDebug() > 2) Info("FillOutputData", "Filling trees with digits.");
	muondata->Fill("D");
	muondata->ResetDigits();
};

//------------------------------------------------------------------------
void AliMUONDigitizerv1::CleanupOutputData(AliMUONLoader* muonloader)
{
// Derived to write the digits tree and then unload the digits tree once written.

	if (GetDebug() > 2) Info("CleanupOutputData", "Writing digits and releasing pointers.");
	muonloader->WriteDigits("OVERWRITE");
	muonloader->UnloadDigits();
};


//------------------------------------------------------------------------
Bool_t AliMUONDigitizerv1::InitInputData(AliMUONLoader* muonloader)
{
// Derived to initialise the input to read from TreeH the hits tree. 
// If the hits are not loaded then we load the hits using the muon loader.

	if (GetDebug() > 2)
		Info("InitInputData", "Loading hits in READ mode and setting the tree address.");

	muondata->SetLoader(muonloader);

	if (muonloader->TreeH() == NULL)
	{
		muonloader->LoadHits("READ");
		if (muonloader->TreeH() == NULL)
		{
			Error("InitInputData", "Can not load the hits tree.");
			return kFALSE;
		};
	};

	muondata->SetTreeAddress("H");
	return kTRUE;
};

//------------------------------------------------------------------------
void AliMUONDigitizerv1::CleanupInputData(AliMUONLoader* muonloader)
{
// Derived to release the loaded hits and unload them.

	if (GetDebug() > 2) Info("CleanupInputData", "Releasing loaded hits.");
	muondata->ResetHits();
	muonloader->UnloadHits();
};

