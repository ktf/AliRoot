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

//-----------------------------------------------------------------------------
// Class AliMUONTriggerElectronics
//--------------------------------
// Manager class for muon trigger electronics
// Client of trigger board classes
// Debugged by Ph. Crochet & Ch. Finck
// Interfaced with new mapping Ch. Finck
//
// Author: Rachid Guernane (LPCCFd)
//-----------------------------------------------------------------------------

#include "AliLoader.h"
#include "AliLog.h"
#include "AliMUONCalibrationData.h"
#include "AliMUONVDigit.h"
#include "AliMUONVDigitStore.h"
#include "AliMUONGlobalTrigger.h"
#include "AliMUONGlobalTriggerBoard.h"
#include "AliMUONLocalTrigger.h"
#include "AliMUONLocalTriggerBoard.h"
#include "AliMUONRegionalTrigger.h"
#include "AliMUONRegionalTriggerBoard.h"
#include "AliMUONTriggerCrate.h"
#include "AliMUONTriggerCrateStore.h"
#include "AliMUONTriggerElectronics.h"
#include "AliMUONTriggerCrateConfig.h"
#include "AliMUONRegionalTriggerConfig.h"
#include "AliMUONGlobalCrateConfig.h"
#include "AliMUONVTriggerStore.h"
#include "AliMUONVCalibParam.h"
#include "AliMpCathodType.h"
#include "AliMpCDB.h"
#include "AliMpDEManager.h"
#include "AliMpSegmentation.h"
#include "AliMpVSegmentation.h"
#include "AliMpCathodType.h"
#include "AliMpTriggerCrate.h"
#include "AliMpLocalBoard.h"
#include "AliMpDDLStore.h"
#include "AliMpExMap.h"
#include "AliMpIntPair.h"

#include "AliLog.h"
#include "AliLoader.h"
#include "AliRun.h"
#include <TBits.h>
#include <TSystem.h>

#include "AliCodeTimer.h"


/// \cond CLASSIMP
ClassImp(AliMUONTriggerElectronics)
/// \endcond

//___________________________________________
AliMUONTriggerElectronics::AliMUONTriggerElectronics(AliMUONCalibrationData* calibData) 
: TObject(),
  fCrates(new AliMUONTriggerCrateStore),
  fGlobalTriggerBoard(new AliMUONGlobalTriggerBoard)
{
 /// CONSTRUCTOR
///


  // force loading of mapping if not already done
  if ( !AliMpDDLStore::Instance(kFALSE) )
  {
    AliMpCDB::LoadDDLStore();
  }
  
  
  Factory(calibData);
  LoadMasks(calibData); 
}

//___________________________________________
AliMUONTriggerElectronics::~AliMUONTriggerElectronics()
{
/// DESTRUCTOR
///
  delete fGlobalTriggerBoard;
  delete fCrates;
    
}

//___________________________________________
void AliMUONTriggerElectronics::Factory(AliMUONCalibrationData* calibData)
{  
 /// BUILD ALL ELECTRONICS
 ///

    fCrates->ReadFromFile(calibData);
}

//___________________________________________
void AliMUONTriggerElectronics::Feed(const AliMUONVDigitStore& digitStore)
{
  /// FILL INPUTS
  ///

  AliCodeTimerAuto("",0);
  
  TIter next(digitStore.CreateTriggerIterator());
  AliMUONVDigit* mdig;
  
  while ( ( mdig = static_cast<AliMUONVDigit*>(next()) ) )
  {      
    //       CHECKME ! The TrackCharge is not ok with new digitizerV3 !
    //			for (Int_t ichg=0; ichg<10; ichg++) schg += mdig->TrackCharge(ichg);
    Int_t ichamber = AliMpDEManager::GetChamberId(mdig->DetElemId());
    Int_t schg = (Int_t)(mdig->Charge() + 0.5);
    
    //       APPLY CONDITION ON SOFT BACKGROUND	
    Int_t tchg = schg - (Int_t(schg/10))*10;	
    
    if (schg<=10 || tchg>0) 
    {
      Int_t detElemId  = mdig->DetElemId();
      Int_t cathode    = mdig->Cathode();
    
      const AliMpVSegmentation* seg = 
	  AliMpSegmentation::Instance()
	  ->GetMpSegmentation(detElemId,AliMp::GetCathodType(cathode));
  
      Int_t ix = mdig->PadX(), iy = mdig->PadY();
      
      AliDebug(3,Form("cathode %d ix %d iy %d ",cathode,ix,iy));

      AliMpPad pad = seg->PadByIndices(ix,iy,kTRUE);
      
      for (Int_t i=0; i<pad.GetNofLocations(); i++) 
      {
        Int_t nboard = pad.GetLocalBoardId(i);
        
        Int_t ibitxy = pad.GetLocalBoardChannel(i);
        
        AliMUONLocalTriggerBoard *b = fCrates->LocalBoard(nboard);
        
        if (b) 
        {
          if (cathode && b->GetSwitch(AliMpLocalBoard::kZeroAllYLSB)) ibitxy += 8;
          
          b->SetbitM(ibitxy,cathode,ichamber-10);
          
          if ( cathode == 0 ) {
            // Particular case of the columns with 22 local boards (2R(L) 3R(L))
            // Fill copy boards
            AliMpLocalBoard* mpLocalBoard = AliMpDDLStore::Instance()->GetLocalBoard(nboard);
            Int_t nboardCopy = mpLocalBoard->GetInputXto();
            if ( nboardCopy > 0 ) {
              AliMUONLocalTriggerBoard* copyBoard = fCrates->LocalBoard(nboardCopy);
              copyBoard->SetbitM(ibitxy,cathode,ichamber-10);
            }
          }
        }
        else
        {
          AliError(Form("Could not get local board number %d",nboard));
        }
      }
    }		
  }

  FeedCopyNeighbours();
}


//___________________________________________
void AliMUONTriggerElectronics::FeedCopyNeighbours()
{
  //
  /// Feed the local copies
  /// and complete the feed with the information of neighbours
  //
  
  // FILL UP/DOWN OF CURRENT BOARD (DONE VIA J3 BUS IN REAL LIFE)
  AliMUONTriggerCrate* cr;
  TIter next2(fCrates->CreateCrateIterator());
  
  while ( ( cr = static_cast<AliMUONTriggerCrate*>(next2()) ) )
  {            
    TObjArray *boards = cr->Boards();
    
    for (Int_t j = 1; j < boards->GetEntries()-1; j++)
    {
      TObject *o = boards->At(j);
			
      if (!o) break;
			
      AliMUONLocalTriggerBoard *currboard = (AliMUONLocalTriggerBoard*)o;
			
      AliMUONLocalTriggerBoard *neighbour = (AliMUONLocalTriggerBoard*)boards->At(j+1);
			
      UShort_t cXY[2][4];
			
      if (j==1) {neighbour->GetXY(cXY); currboard->SetXYU(cXY);}
			
      //       LAST BOARD IN THE CRATE HAS NO UP EXCEPT FOR CRATES 2 & 3
      if (j < boards->GetEntries()-2)  
      {
	      AliMUONLocalTriggerBoard *nextboard = (AliMUONLocalTriggerBoard*)boards->At(j+2);
				
	      currboard->GetXY(cXY); neighbour->SetXYD(cXY);
	      nextboard->GetXY(cXY); neighbour->SetXYU(cXY);
				
	      if (j==boards->GetEntries()-3) {neighbour->GetXY(cXY); nextboard->SetXYD(cXY);}
      }
    }
  }
 
}


//___________________________________________
void AliMUONTriggerElectronics::Feed(UShort_t pattern[2][4])
{
  /// FILL INPUTS
  ///
  AliMUONTriggerCrate* cr;
  TIter next(fCrates->CreateCrateIterator());
   
   while ( ( cr = static_cast<AliMUONTriggerCrate*>(next()) ) )
   {                 
     TObjArray *boards = cr->Boards();
     
     for (Int_t j = 1; j < boards->GetEntries(); j++)
     {
       TObject *o = boards->At(j);
       
       if (!o) break;
       
       AliMUONLocalTriggerBoard *board = (AliMUONLocalTriggerBoard*)o;
       
       board->SetXY(pattern);
     }
   }
}

//___________________________________________
void AliMUONTriggerElectronics::DumpOS()
{
/// DUMP IN THE OLD WAY
///
   for (Int_t i= 0; i < 234;i++)
   {
      AliMUONLocalTriggerBoard *board = fCrates->LocalBoard(i);

      if (board) board->Scan("ALL");
   }
}

//___________________________________________
void AliMUONTriggerElectronics::Scan(const Option_t *option)
{
  /// SCAN
  ///

  AliMUONTriggerCrate* cr;
  TIter next(fCrates->CreateCrateIterator());  
  
  while ( ( cr = static_cast<AliMUONTriggerCrate*>(next()) ) )
  {                
    TObjArray *boards = cr->Boards();
    
    for (Int_t j = 0; j < boards->GetEntries(); j++)
    {
      TObject *o = boards->At(j);
      
      TString op = option;
      
      Bool_t cdtion = kFALSE;
      
      if (op.Contains("LOCAL"))    cdtion = o->IsA() == AliMUONLocalTriggerBoard::Class();
      if (op.Contains("REGIONAL")) cdtion = o->IsA() == AliMUONRegionalTriggerBoard::Class();
      if (op.Contains("GLOBAL"))   cdtion = o->IsA() == AliMUONGlobalTriggerBoard::Class();
      
      if (!o || !cdtion) continue;
      
      AliMUONLocalTriggerBoard *board = (AliMUONLocalTriggerBoard*)o;
      
      board->Scan();
    }
  }
}

//___________________________________________
void AliMUONTriggerElectronics::Reset()
{
  /// RESET
  ///
  
   AliMUONTriggerCrate* cr;
  TIter next(fCrates->CreateCrateIterator());
   while ( ( cr = static_cast<AliMUONTriggerCrate*>(next()) ) )
   {            
      TObjArray *boards = cr->Boards();
            
      for (Int_t j=0; j<boards->GetEntries(); j++)
      {     
         AliMUONTriggerBoard *b = (AliMUONTriggerBoard*)boards->At(j);

         if (b) b->Reset();
      }
   }
}


//_______________________________________________________________________
void AliMUONTriggerElectronics::LoadMasks(AliMUONCalibrationData* calibData)
{
  /// Load mask from config in CDB 
  
  // Set mask
  
  AliMUONRegionalTriggerConfig* regionalConfig = calibData->RegionalTriggerConfig();
  if (!regionalConfig) {
     AliError("No valid regional trigger configuration in CDB");
     return;
  }   
  
  AliMUONTriggerCrate* cr;
  TIter next(fCrates->CreateCrateIterator());
  
  Int_t irb(0);
  
  while ( ( cr = static_cast<AliMUONTriggerCrate*>(next()) ) )
  {            
    TObjArray *boards = cr->Boards();
    
    AliMUONRegionalTriggerBoard *regb = (AliMUONRegionalTriggerBoard*)boards->At(0);

    AliMUONTriggerCrateConfig* crateConfig = regionalConfig->FindTriggerCrate(cr->GetName());
    
    if (!crateConfig)
    {
      AliError(Form("Crate %s not present in configuration !!!", cr->GetName()));
      return;
    }
    
    UShort_t rmask= crateConfig->GetMask();

    regb->Mask(rmask);
    
    for (Int_t j = 1; j < boards->GetEntries(); j++)
    {
      AliMUONLocalTriggerBoard *b = (AliMUONLocalTriggerBoard*)boards->At(j);
      
      Int_t cardNumber = b->GetNumber();
      
      if (cardNumber) // skip empty slots
      {
        AliMUONVCalibParam* localBoardMasks = calibData->LocalTriggerBoardMasks(cardNumber);
        for ( Int_t i = 0; i < localBoardMasks->Size(); ++i )
        {
          UShort_t lmask = static_cast<UShort_t>(localBoardMasks->ValueAsInt(i) & 0xFFFF);
          b->Mask(i,lmask);
        }
      }
    }
    ++irb;
  }
  
   AliMUONGlobalCrateConfig * globalConfig = calibData->GlobalTriggerCrateConfig();
   if (!globalConfig) {
     AliError("No valid trigger crate configuration in CDB");
     return;
   }   

    UInt_t gmask = 0;
    for (Int_t i = 0; i < 4; i++) {
      gmask = globalConfig->GetGlobalMask(i);
      fGlobalTriggerBoard->Mask(i,gmask);
    }
}

//___________________________________________
void AliMUONTriggerElectronics::LocalResponse()
{
/// Compute the response for local cards

  AliCodeTimerAuto("",0);
	
  AliMUONTriggerCrate* cr;
  TIter next(fCrates->CreateCrateIterator());

  UShort_t thisl[16];
  
  while ( ( cr = static_cast<AliMUONTriggerCrate*>(next()) ) )
  {            
    
    TObjArray *boards = cr->Boards();
    
    AliMUONRegionalTriggerBoard *regb = (AliMUONRegionalTriggerBoard*)boards->At(0);
    
    for (Int_t j=0; j<16; ++j) thisl[j] = 0;
  
    for (Int_t j = 1; j < boards->GetEntries(); j++)
    {     
	TObject *o = boards->At(j);
      
	if (!o) break;
      
	AliMUONLocalTriggerBoard *board = (AliMUONLocalTriggerBoard*)o;

	board->Response();
				
	UShort_t response = board->GetResponse();            
        
	// CRATE CONTAINING INTERFACE BOARD
  if (!board->IsNotified()) // copy boards
	{
	  if ( response != 0 ) 
	    AliWarning(Form("Interface board %s in slot %d of crate %s has a non zero response",
			    board->GetName(),j,cr->GetName()));
	  AliDebug(1, Form("local slot %d, number %d in crate %s\n", j, board->GetNumber(), cr->GetName()));
	  
	}
        
	thisl[j-1] = response;
    }
    
    regb->SetLocalResponse(thisl);
  }
}

//___________________________________________
void AliMUONTriggerElectronics::RegionalResponse()
{
  /// Compute the response for all regional cards.

  AliCodeTimerAuto("",0);

  AliMUONTriggerCrate* cr;
  TIter next(fCrates->CreateCrateIterator());
  
  while ( ( cr = static_cast<AliMUONTriggerCrate*>(next()) ) )
  {            
      TObjArray *boards = cr->Boards();

      AliMUONRegionalTriggerBoard *regb = (AliMUONRegionalTriggerBoard*)boards->At(0);

      regb->Response();

   }
}

//___________________________________________
void AliMUONTriggerElectronics::GlobalResponse()
{
  /// Compute the global response

  AliCodeTimerAuto("",0);

  UShort_t regional[16];
  
  AliMUONTriggerCrate* cr;
  Int_t irb(0);
  
  if ( fCrates->NumberOfCrates() > 16 ) 
  {
    AliFatal(Form("Something is wrong : too many crates %d",
                  fCrates->NumberOfCrates()));
  }

  // send regional responses to the global trigger in right order
  // do not used iterator order
  
  for (Int_t iSide = 0; iSide < 2; iSide++) // right & left side
  {            
    for (Int_t iReg = 0; iReg < 8; iReg++) // 8 crates/regional boards for each side.
    {
      cr = fCrates->Crate(iSide, iReg);     

      AliMUONTriggerBoard* rb = 
	static_cast<AliMUONTriggerBoard*>(cr->Boards()->At(0));
      regional[irb] = rb->GetResponse();
      ++irb;
    }
  }

  fGlobalTriggerBoard->SetRegionalResponse(regional);
  fGlobalTriggerBoard->Response();
}

//_______________________________________________________________________
void AliMUONTriggerElectronics::Digits2Trigger(const AliMUONVDigitStore& digitStore,
                                               AliMUONVTriggerStore& triggerStore)
{
  AliCodeTimerAuto("",0);

  /// Main method to go from digits to trigger decision
  AliMUONRegionalTrigger pRegTrig;
  
  triggerStore.Clear();

  // NOW RESET ELECTRONICS
  Reset();
  
  // RUN THE FULL BEE CHAIN
  Feed(digitStore);
  LocalResponse();
  RegionalResponse();      
  GlobalResponse();
  //    DumpOS();
	
  AliMUONTriggerCrate* cr;
  AliMUONLocalTrigger localTrigger;
  
  // stored in right order
  // do not used iterator order
  
  for (Int_t iSide = 0; iSide < 2; iSide++) // right & left side
  {            
    for (Int_t iReg = 0; iReg < 8; iReg++) // 8 crates/regional boards for each side.
    {
      cr = fCrates->Crate(iSide, iReg);     
      TObjArray *boards = cr->Boards();
      
      UInt_t regInpLpt = 0;
      UInt_t regInpHpt = 0;
      
      AliMUONRegionalTriggerBoard *regBoard = (AliMUONRegionalTriggerBoard*)boards->At(0);
      
      for (Int_t j = 1; j < boards->GetEntries(); j++)
      {     
        TObject *o = boards->At(j);
        
        if (!o) break;
        
        AliMUONLocalTriggerBoard *board = (AliMUONLocalTriggerBoard*)o;
        
        if (board) 
        {
          //          L0 TRIGGER
          // pcrochet 181206: MOOD needs ALL boards
          //	  if (board->Triggered())
          //	  {
          
          Int_t icirc = board->GetNumber();
            localTrigger.SetLoCircuit(icirc);
            localTrigger.SetLoStripX(board->GetStripX11());
            localTrigger.SetLoDev(board->GetDev());
            localTrigger.SetLoSdev(board->GetSdev());
            localTrigger.SetLoTrigY(board->GetTrigY());
            localTrigger.SetLoStripY(board->GetStripY11());
            
            //             SAVE LUT OUTPUT 
            UShort_t response = board->GetResponse();
            localTrigger.SetLoHpt((response & 12) >> 2);
            localTrigger.SetLoLpt(response &  3);
            
            // calculates regional inputs from local for the moment
            UInt_t hPt = (response >> 2) & 0x3;
            UInt_t lPt =  response       & 0x3;
            
            regInpHpt |= hPt << (30 - (j-1)*2);
            regInpLpt |= lPt << (30 - (j-1)*2);
            
            TBits rrr;
            rrr.Set(6,&response);	  
            
            //             SAVE BIT PATTERN
            localTrigger.SetX1Pattern(board->GetXY(0,0));
            localTrigger.SetX2Pattern(board->GetXY(0,1));
            localTrigger.SetX3Pattern(board->GetXY(0,2));
            localTrigger.SetX4Pattern(board->GetXY(0,3));
            
            localTrigger.SetY1Pattern(board->GetXY(1,0));
            localTrigger.SetY2Pattern(board->GetXY(1,1));
            localTrigger.SetY3Pattern(board->GetXY(1,2));
            localTrigger.SetY4Pattern(board->GetXY(1,3));
            
            //             ADD A NEW LOCAL TRIGGER          
            triggerStore.Add(localTrigger);  
            
          }
        }
      pRegTrig.SetId(iReg + 8*iSide);
      pRegTrig.SetLocalOutput(regInpLpt, 0);
      pRegTrig.SetLocalOutput(regInpHpt, 1);
      pRegTrig.SetOutput(regBoard->GetResponse());
      
      triggerStore.Add(pRegTrig);  
      }
    }
  
  // GLOBAL TRIGGER INFORMATION
  UShort_t global = fGlobalTriggerBoard->GetResponse();
  UInt_t *globalInput = fGlobalTriggerBoard->GetGlobalInput();  

  AliMUONGlobalTrigger globalTrigger;
  
  globalTrigger.SetFromGlobalResponse(global);
  globalTrigger.SetFromGlobalInput(globalInput);
  // ADD A LOCAL TRIGGER IN THE LIST 
  triggerStore.SetGlobal(globalTrigger);

}

//___________________________________________
void AliMUONTriggerElectronics::Feed(const AliMUONVTriggerStore& triggerStore)
{
  //
  /// Fill inputs from reconstructed local trigger store
  //
  AliMUONLocalTrigger* locTrg;
  TIter next(triggerStore.CreateLocalIterator());
  TArrayS xyPattern[2];
  UShort_t xy[2][4];
  Int_t loCircuit;
  while ( ( locTrg = static_cast<AliMUONLocalTrigger*>( next() )) != NULL ){
    locTrg->GetXPattern(xyPattern[0]);
    locTrg->GetYPattern(xyPattern[1]);
    loCircuit = locTrg->LoCircuit();
    AliMUONLocalTriggerBoard* localBoard = fCrates->LocalBoard(loCircuit);
    for (Int_t icath = 0; icath<2; ++icath){
      for (Int_t ich = 0; ich < 4; ++ich){
	xy[icath][ich] = xyPattern[icath][ich];
      }
    }
    localBoard->SetXY(xy);
  }

  FeedCopyNeighbours();
}

//_______________________________________________________________________
Bool_t AliMUONTriggerElectronics::ModifiedLocalResponse(Int_t loCircuit,
							Bool_t& bendingPlaneResp,
							Bool_t& nonBendingPlaneResp,
							Bool_t isCoinc44,
							Int_t removeChamber)
{
  //
  /// Re-compute the local trigger response
  /// with some modifications (i.e. setting coinc44 or after removing one chamber)
  //

  bendingPlaneResp = kFALSE;
  nonBendingPlaneResp = kFALSE;

  Bool_t isTriggered = kFALSE;

  AliMUONLocalTriggerBoard* currBoard = fCrates->LocalBoard(loCircuit);

  if ( ! currBoard ) return isTriggered;

  AliMUONLocalTriggerBoard localBoard (*currBoard);

  if (removeChamber>=0 && removeChamber<=3){

    // Set the bit pattern of selected chamber to 0
    UShort_t xy[2][4];
    UShort_t xyu[2][4];
    UShort_t xyd[2][4];

    localBoard.GetXY(xy);
    localBoard.GetXYU(xyu);
    localBoard.GetXYD(xyd);

    for(Int_t icath=0; icath<2; icath++){
      xy[icath][removeChamber] = 0;
      xyu[icath][removeChamber] = 0;
      xyd[icath][removeChamber] = 0;
    }

    localBoard.SetXY(xy);
    localBoard.SetXYU(xyu);
    localBoard.SetXYD(xyd);
  }

  localBoard.ResetResponse();

  localBoard.SetCoinc44((Int_t)isCoinc44);
  localBoard.Response();

  bendingPlaneResp = localBoard.IsTrigX();
  nonBendingPlaneResp = localBoard.IsTrigY();
  isTriggered = localBoard.Triggered();

  return isTriggered;
}


//_______________________________________________________________________
void AliMUONTriggerElectronics::ResponseRemovingChambers(AliMUONVTriggerStore& triggerStore)
{
  /// Update local board information with the trigger response after removing each chamber

  AliCodeTimerAuto("", 0);

  Reset();
  Feed(triggerStore);

  AliMUONLocalTrigger* locTrg;
  TIter next(triggerStore.CreateLocalIterator());
  Int_t loCircuit;
  Bool_t planeResp[2], isTrig44;
  Bool_t bendPlaneRespNoCh, nonBendPlaneRespNoCh, isTrigWithoutCh;
  while ( ( locTrg = static_cast<AliMUONLocalTrigger*>( next() )) != NULL ){
    if ( ! ( locTrg->IsTrigX() && locTrg->IsTrigY() ) ) continue;
    loCircuit = locTrg->LoCircuit();
    isTrig44 = ModifiedLocalResponse(loCircuit, planeResp[0], planeResp[1], kTRUE);
    for (Int_t ich=0; ich<4; ++ich){
      if ( ! isTrig44 ){
	isTrigWithoutCh = ModifiedLocalResponse(loCircuit, bendPlaneRespNoCh, nonBendPlaneRespNoCh, kFALSE, ich);
	if ( ! isTrigWithoutCh ) continue;
	for (Int_t icath=0; icath<2; icath++){
	  if ( ! planeResp[icath] )
	    locTrg->SetNoHitInPlane(icath, ich);
	} // loop on cathodes
      }
      locTrg->SetTriggerWithoutChamber(ich);
    } // loop on chambers
    AliDebug(1, Form("Is44 %i  triggers %i  pattern %i", isTrig44, locTrg->GetTriggerWithoutChamber(), locTrg->GetHitPatternFromResponse()));
  }
}
