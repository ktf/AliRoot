
/////////////////////////////////////////////////////////////////////////////////////////////////////
//     Class for emulation of the ALTRO chip (SAMPA digital Chain) in C++                          //
//     Author: Roland Bramm                                                                        //
/////////////////////////////////////////////////////////////////////////////////////////////////////


/**		@file AliTPCSAMPAEmulator.h
 *      @brief This the header File for the SAMPA class
 
 	author: marian.ivanov@cern.ch
                mesut.arslandok@cern.ch  
*/



#include <AliTPCSAMPAEmulator.h>
#include <TH1F.h>
#include <TMath.h>
#include <TSystem.h>
#include <AliDAQ.h>
#include <AliRawReader.h>
#include <AliRawVEvent.h>
#include <AliRawData.h>
#include <AliRawVEquipment.h>
#include <AliRawEquipmentHeader.h>
#include <AliTPCRawStreamV3.h>
#include <TCanvas.h>
#include <AliRawDataHeader.h>
#include <AliRawDataHeaderV3.h>

/**	@brief Constroctor of SAMPA Class
 *
 *	Consturctor of SAMPA Class, some variables are set.\n
 *	The input Data is altered, so after running the complete emulation you have the
 *	SAMPA Processed Data in the Channel Pointer.\n
 *
 *	@param timebins an <tt> int </tt> sets the length of the input Data (Channel)
 *	@param Channel an <tt> short* </tt> Pointer to a 1d Short_tArray with the input Data
 */


ClassImp(AliTPCSAMPAEmulator)

AliTPCSAMPAEmulator::AliTPCSAMPAEmulator() : 
  TNamed(),
  fBC3SlopeDown(0.2),  // BC3 slope down parameter
  fBC3SlopeUp(0.1),    // BC3 slope up   parameter
  fBC3Round(-1)      // Rounding error of BC3 filter

{
  //
  // Constructor of SAMPA Class
  //


}



/**	@brief Destructor of SAMPA Class
 *
 *	Destructor of SAMPA Class\n
 */
AliTPCSAMPAEmulator::~AliTPCSAMPAEmulator() {
  //
  // Destructor of SAMPA Class
  //

 
}


void AliTPCSAMPAEmulator::SetBC3Parameters(Double_t slopeDown, Double_t slopeUp, Double_t round){
  //
  // 
  //
  fBC3SlopeDown=slopeDown;
  fBC3SlopeUp=slopeUp;
  fBC3Round=round;
}


Bool_t   AliTPCSAMPAEmulator::BC3SlopeFilterFloat(Int_t npoints, Double_t *dataArray){
  //
  // BC3 filter
  //
  return  AliTPCSAMPAEmulator::BC3SlopeFilterFloat(npoints,dataArray,  fBC3SlopeDown,fBC3SlopeUp,  fBC3Round); 
}

Bool_t  AliTPCSAMPAEmulator::BC3SlopeFilterFloat(Int_t npoints, Double_t *dataArray, Double_t slopeDown, Double_t slopeUp, Double_t round) {
  //
  //
  //
  // BC2 filter as should be implemented in SAMPA
  // from Konstantin description
  // https://alice.its.cern.ch/jira/browse/ATO-129
  //    discussed here the root/C code snippet for the slope based filter. The filter is applied as
  //
  //     data_filtered=data-slopefilterfloat(data);
  //
  //    The filter is floating point. To simulate the integer behavior of the hardware based filter, define INTFILTER and choose the slopes to be binary compatible (1.0, 0.5, 0.25 etc.).
  // #define SLOPEUP		1.0
  // #define SLOPEDOWN	2.0

  Double_t slopeBaseline=0;
  Bool_t slopestart=true;
  if (npoints<=1) return kFALSE;
  for (Int_t iTimeBin=0; iTimeBin<npoints; iTimeBin++){
    Double_t data=dataArray[iTimeBin];
    if (slopestart) {	// initialization, not necessarily needed
      slopeBaseline=data;	// start value
      slopestart=false;
    };
    if (data>slopeBaseline) {
      slopeBaseline+=slopeUp;
      if (slopeBaseline>data) slopeBaseline=data;
    } else if (data<slopeBaseline) {
      slopeBaseline-=slopeDown;
      if (slopeBaseline<data) slopeBaseline=data;
    };
    if (round>0){
      //    return round(slopeBaseline);
      slopeBaseline=TMath::Nint(slopeBaseline*round)/round;
    }
    dataArray[iTimeBin]-=slopeBaseline;
  }
  return kTRUE;
};
