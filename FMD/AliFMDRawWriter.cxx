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

//____________________________________________________________________
//
// Class to write ADC values to a raw data file
//
#include "AliLog.h"		// ALILOG_H
#include "AliFMD.h"		// ALIFMD_H
#include "AliFMDDigit.h"	// ALIFMDDIGIT_H
#include "AliFMDRawWriter.h"	// ALIFMDRAWREADER_H 
#include "AliAltroBuffer.h"     // ALIALTROBUFFER_H
#include "AliLoader.h"		// ALILOADER_H
#include "TArrayI.h"		// ROOT_TArrayI
#include <TClonesArray.h>	// ROOT_TClonesArray

//____________________________________________________________________
ClassImp(AliFMDRawWriter);

//____________________________________________________________________
AliFMDRawWriter::AliFMDRawWriter(AliFMD* fmd) 
  : TTask("FMDRawWriter", "Writer of Raw ADC values from the FMD"),
    fFMD(fmd)
{
  SetSampleRate();
}


//____________________________________________________________________
void
AliFMDRawWriter::Exec(Option_t*) 
{
  // Turn digits into raw data. 
  // 
  // Digits are read from the Digit branch, and processed to make
  // three DDL files, one for each of the sub-detectors FMD1, FMD2,
  // and FMD3. 
  //
  // The raw data files consists of a header, followed by ALTRO
  // formatted blocks.  
  // 
  //          +-------------+
  //          | Header      |
  //          +-------------+
  //          | ALTRO Block |
  //          | ...         |
  //          +-------------+
  //          DDL file 
  // 
  // An ALTRO formatted block, in the FMD context, consists of a
  // number of counts followed by a trailer. 
  // 
  //          +------------------+
  //          | Count            |
  //          | ...              |
  //          | possible fillers |
  //          +------------------+
  //          | Trailer          |
  //          +------------------+
  //          ALTRO block 
  // 
  // The counts are listed backwards, that is, starting with the
  // latest count, and ending in the first. 
  // 
  // Each count consist of 1 or more ADC samples of the VA1_ALICE
  // pre-amp. signal.  Just how many samples are used depends on
  // whether the ALTRO over samples the pre-amp.  Each sample is a
  // 10-bit word, and the samples are grouped into 40-bit blocks 
  //
  //          +------------------------------------+
  //          |  S(n)   | S(n-1) | S(n-2) | S(n-3) |
  //          |  ...    | ...    | ...    | ...    |
  //          |  S(2)   | S(1)   | AA     | AA     |
  //          +------------------------------------+
  //          Counts + possible filler 
  //
  // The trailer of the number of words of signales, the starting
  // strip number, the sector number, and the ring ID; each 10-bit
  // words,  packed into 40-bits. 
  // 
  //          +------------------------------------+
  //          | # words | start  | sector | ring   |
  //          +------------------------------------+
  //          Trailer
  // 
  // Note, that this method assumes that the digits are ordered. 
  // 
  AliLoader* loader = fFMD->GetLoader();
  loader->LoadDigits();
  TTree* digitTree = loader->TreeD();
  if (!digitTree) {
    Error("Digits2Raw", "no digit tree");
    return;
  }
  
  TClonesArray* digits = new TClonesArray("AliFMDDigit", 1000);
  fFMD->SetTreeAddress();
  TBranch* digitBranch = digitTree->GetBranch(fFMD->GetName());
  if (!digitBranch) {
    Error("Digits2Raw", "no branch for %s", fFMD->GetName());
    return;
  }
  digitBranch->SetAddress(&digits);
  
  Int_t nEvents = Int_t(digitTree->GetEntries());
  for (Int_t event = 0; event < nEvents; event++) {
    fFMD->ResetDigits();
    digitTree->GetEvent(event);
    
    Int_t nDigits = digits->GetEntries();
    if (nDigits < 1) continue;


    UShort_t prevDetector = 0;
    Char_t   prevRing     = '\0';
    UShort_t prevSector   = 0;
    // UShort_t prevStrip    = 0;

    // The first seen strip number for a channel 
    UShort_t startStrip   = 0;
    
    // Which channel number in the ALTRO channel we're at 
    UShort_t offset       = 0;

    // How many times the ALTRO Samples one VA1_ALICE channel 
    Int_t sampleRate = 1;

    // A buffer to hold 1 ALTRO channel - Normally, one ALTRO channel
    // holds 128 VA1_ALICE channels, sampled at a rate of `sampleRate' 
    TArrayI channel(128 * sampleRate);
    
    // The Altro buffer 
    AliAltroBuffer* altro = 0;
    
    // Loop over the digits in the event.  Note, that we assume the
    // the digits are in order in the branch.   If they were not, we'd
    // have to cache all channels before we could write the data to
    // the ALTRO buffer, or we'd have to set up a map of the digits. 
    for (Int_t i = 0; i < nDigits; i++) {
      // Get the digit
      AliFMDDigit* digit = static_cast<AliFMDDigit*>(digits->At(i));

      UShort_t det    = digit->Detector();
      Char_t   ring   = digit->Ring();
      UShort_t sector = digit->Sector();
      UShort_t strip  = digit->Strip();
      if (det != prevDetector) {
	AliDebug(10, Form("FMD: New DDL, was %d, now %d",
			  AliFMD::kBaseDDL + prevDetector - 1,
			  AliFMD::kBaseDDL + det - 1));
	// If an altro exists, delete the object, flushing the data to
	// disk, and closing the file. 
	if (altro) { 
	  // When the first argument is false, we write the real
	  // header. 
	  AliDebug(10, Form("New altro: Write channel at %d Strip: %d "
			    "Sector: %d  Ring: %d", 
			    i, startStrip, prevSector, prevRing));
	  // TPC to FMD translations 
	  // 
	  //    TPC                FMD
	  //    ----------+-----------
	  //    pad       |      strip
	  //    row       |     sector
	  //    sector    |       ring
	  // 
	  altro->WriteChannel(Int_t(startStrip), 
			      Int_t(prevSector), 
			      Int_t((prevRing == 'I' ? 0 : 1)), 
			      channel.fN, channel.fArray, 0);
	  altro->Flush();
	  altro->WriteDataHeader(kFALSE, kFALSE);
	  delete altro;
	  altro = 0;
	}

	prevDetector = det;
	// Need to open a new DDL! 
	Int_t ddlId = AliFMD::kBaseDDL + det - 1;
	TString filename(Form("%s_%d.ddl", fFMD->GetName(),  ddlId));

	AliDebug(10, Form("New altro buffer with DDL file %s", 
			  filename.Data()));
	AliDebug(10, Form("New altro at %d", i));
	// Create a new altro buffer - a `1' as the second argument
	// means `write mode' 
	altro = new AliAltroBuffer(filename.Data(), 1);
	
	// Write a dummy (first argument is true) header to the DDL
	// file - later on, when we close the file, we write the real
	// header
	altro->WriteDataHeader(kTRUE, kFALSE);

	// Figure out the sample rate 
	if (fSampleRate > 0) sampleRate = fSampleRate;
	else {
	  if (digit->Count2() >= 0) sampleRate = 2;
	  if (digit->Count3() >= 0) sampleRate = 3;
	}

	channel.Set(128 * sampleRate);
	offset     = 0;
	prevRing   = ring;
	prevSector = sector;
	startStrip = strip;
      }
      else if (offset == 128                        
	       || digit->Ring() != prevRing 
	       || digit->Sector() != prevSector) {
	// Force a new Altro channel
	AliDebug(10, Form("Flushing channel to disk because %s",
			  (offset == 128 ? "channel is full" :
			   (ring != prevRing ? "new ring up" :
			    "new sector up"))));
	AliDebug(10, Form("New Channel: Write channel at %d Strip: %d "
			  "Sector: %d  Ring: %d", 
			  i, startStrip, prevSector, prevRing));
	altro->WriteChannel(Int_t(startStrip), 
			    Int_t(prevSector), 
			    Int_t((prevRing == 'I' ? 0 : 1)), 
			    channel.fN, channel.fArray, 0);
	// Reset and update channel variables 
	channel.Reset(0);
	offset     = 0; 
	startStrip = strip;
	prevRing   = ring;
	prevSector = sector;
      }

      // Store the counts of the ADC in the channel buffer 
      channel[offset * sampleRate] = digit->Count1();
      if (sampleRate > 1) 
	channel[offset * sampleRate + 1] = digit->Count2();
      if (sampleRate > 2) 
	channel[offset * sampleRate + 2] = digit->Count3();
      offset++;
    }
    // Finally, we need to close the final ALTRO buffer if it wasn't
    // already 
    if (altro) {
      altro->Flush();
      altro->WriteDataHeader(kFALSE, kFALSE);
      delete altro;
    }
  }
  loader->UnloadDigits();
}

//____________________________________________________________________
// 
// EOF
//
