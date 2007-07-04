#include "AliT0RawReader.h"
#include "AliT0Parameters.h"
#include "AliBitPacking.h"
#include "TBits.h"

#include <Riostream.h>
#include "TMath.h"
#include "TH1F.h"
#include "TArrayI.h"
#include "AliLog.h"
 
ClassImp(AliT0RawReader)
  
  AliT0RawReader::AliT0RawReader (AliRawReader *rawReader, Bool_t isOnline)
    :  TTask("T0RawReader","read raw T0 data"),
       fRawReader(rawReader),
       fData(NULL),
       fPosition(0),
       fParam(NULL),
       fIsOnline(isOnline)
{
  //
// create an object to read T0raw digits
  AliDebug(1,"Start ");
 
  fRawReader->Reset();
  fRawReader->Select("T0");
  fParam = AliT0Parameters::Instance();  
  if (fIsOnline)
    fParam->InitIfOnline();
  else 
    fParam->Init();
 
}
 AliT0RawReader::~AliT0RawReader ()
{
  // 
}
/*
AliT0RawReader::AliT0RawReader(const AliT0RawReader& o): TTask(o),
     fRawReader(rawReader),
       fData(NULL),
       fPosition(0)
{
  //
}
*/


Bool_t  AliT0RawReader::Next()
{
// read the next raw digit
// returns kFALSE if there is no digit left
//"LookUpTable":
// Amplitude LED TRM=0; chain=0; TDC 0 -5        channel 0,2,4,6
// Time CFD      TRM=0; chain=0; TDC 6 - 11      channel 0,2,4,6
// mean time     TRM=0; chain=0; TDC 12          channel 0
// T0A           TRM=0; chain=0; TDC 12          channel 2
// T0C           TRM=0; chain=0; TDC 12          channel 4
// vertex        TRM=0; chain=0; TDC 12          channel 6
// mult QTC0        TRM=0; chain=0; TDC 13          channel 0
// mult QTC1        TRM=0; chain=0; TDC 13          channel 2

// Charge QTC0   TRM=1; chain=0; TDC 0 -5        channel 0,2,4,6
// Charge QTC1   TRM=1; chain=0; TDC 6 - 11      channel 0,2,4,6
// T0A trigger          TRM=1; chain=0; TDC 12          channel 0
// T0C trigger          TRM=1; chain=0; TDC 12          channel 2
// vertex trigger       TRM=1; chain=0; TDC 12          channel 4
// trigger central      TRM=1; chain=0; TDC 13          channel 0
// tigger semicenral    TRM=1; chain=0; TDC 13          channel 2
//
// allData array collect data from all channels in one :
// allData[0] - allData[23] 24 CFD channels
// allData[24] -   allData[47] 24 LED channels
//  allData[48]  mean (T0) signal  
// allData[49]   time difference (vertex)

 
  UInt_t word;
  Int_t time=0,  itdc=0, ichannel=0, uu; 
  Int_t numberOfWordsInTRM=0, iTRM=0;
  Int_t tdcTime, koef,hit=0;
  Int_t koefhits[110];
    Int_t  TRM_CHAIN_0_HEADER =  0x00000000;
  Int_t  TRM_CHAIN_0_TRAILER =  0x10000000;
  
  Int_t  FILLER =  0x70000000;
  Bool_t correct=kTRUE;
  Int_t header;

   Int_t fNTRM = fParam->GetNumberOfTRMs();
   for ( Int_t k=0; k<110; k++) {
    koefhits[k]=0;
    for ( Int_t jj=0; jj<5; jj++) {
      fAllData[k][jj]=0;
     }
   }
    do {
      if (!fRawReader->ReadNextData(fData)) return kFALSE;
    } while (fRawReader->GetDataSize() == 0);
    
    fPosition = 0;
    //  cout.setf( ios_base::hex, ios_base::basefield );
    
    //DRM header
    for (Int_t i=0; i<6; i++) {
      word = GetNextWord();
      header = AliBitPacking::UnpackWord(word,28,31);
      if( header !=4 )
	{
	  AliWarning(Form(" !!!! wrong  DRM header  %x!!!!", word));
	  fRawReader->AddFatalErrorLog(kWrongDRMHeader,Form("w=%x",word));
	  break;
	}
    }
    
    for (Int_t ntrm=0; ntrm< fNTRM; ntrm++)
      {
	//TRMheader  
	word = GetNextWord();
	header = AliBitPacking::UnpackWord(word,28,31);
	if ( header != 4 )
	  {
	    AliWarning(Form(" !!!! wrong TRM header  %x!!!!", word));
	    fRawReader->AddMajorErrorLog(kWrongTRMHeader,Form("w=%x",word));
	    break;
	  }
	numberOfWordsInTRM=AliBitPacking::UnpackWord(word,4,16);
	iTRM=AliBitPacking::UnpackWord(word,0,3);
	for( Int_t ichain=0; ichain<2; ichain++)
	  {
	    //chain header
	    word = GetNextWord();
	    uu = word & TRM_CHAIN_0_HEADER;
	    if(uu != TRM_CHAIN_0_HEADER) 
	      {
		AliWarning(Form(" !!!! wrong CHAIN  0  header %x!!!!", word));
		fRawReader->AddMajorErrorLog(kWrongChain0Header,Form("w=%x",word));
		break;
	      }
	    word = GetNextWord();
	    tdcTime =  AliBitPacking::UnpackWord(word,31,31);   
	    for (; tdcTime==1; tdcTime) 
	      {
		itdc=AliBitPacking::UnpackWord(word,24,27);
		ichannel=AliBitPacking::UnpackWord(word,21,23);
		time=AliBitPacking::UnpackWord(word,0,20);
		
		koef = fParam->GetChannel(iTRM,itdc,ichain,ichannel);
		if (koef ==-1 ){
		  AliWarning(Form("Incorrect lookup table ! "));
		  fRawReader->AddMajorErrorLog(kIncorrectLUT);
		  correct=kFALSE;
		}
		if(correct){
		  hit=koefhits[koef];
		  fAllData[koef][hit]=time; 
		  koefhits[koef]++;
		}
		  word = GetNextWord();
		  tdcTime =  AliBitPacking::UnpackWord(word,31,31);   
		
	    }
	    

	    uu = word&TRM_CHAIN_0_TRAILER;
	    if(uu != TRM_CHAIN_0_TRAILER )
	      {
		AliWarning(Form(" !!!! wrong CHAIN 0 trailer %x !!!!", word));
		  fRawReader->AddMajorErrorLog(kWrongChain0Trailer,Form("w=%x",word));
		break;
	      }
	  }
            
	word = GetNextWord(); //TRM trailer
	header = AliBitPacking::UnpackWord(word,28,31);
	if( header !=5 )
	  {
	    AliWarning(Form(" !!!! wrong TRM GLOBAL trailer  %x!!!!", word));
	    fRawReader->AddMajorErrorLog(kWrongTRMTrailer,Form("w=%x",word));
	    break;
	  }
      } //TRM loop
    word = GetNextWord(); //
    header = AliBitPacking::UnpackWord(word,28,31);
    uu = word&FILLER;
    if (word == FILLER )  word = GetNextWord(); 
    if( header !=5 )
      {
	AliWarning(Form(" !!!! wrong DRM GLOBAL trailer  %x!!!!", word));
	fRawReader->AddFatalErrorLog(kWrongDRMTrailer,Form("w=%x",word));
      }
    cout.setf( ios_base::dec, ios_base::basefield );
    
    return kTRUE;
}
//_____________________________________________________________________________
Int_t AliT0RawReader::GetPosition()
{
  // Sets the position in the
  // input stream
  if (((fRawReader->GetDataSize() * 8) % 32) != 0)
    AliFatal(Form("Incorrect raw data size ! %d words are found !",fRawReader->GetDataSize()));
  return (fRawReader->GetDataSize() * 8) / 32;
}
//_____________________________________________________________________________
UInt_t AliT0RawReader::GetNextWord()
{
  // Read the next 32 bit word in backward direction
  // The input stream access is given by fData and fPosition


  //   fPosition--;
  Int_t iBit = fPosition * 32;
  Int_t iByte = iBit / 8;

  UInt_t word = 0;
  word  = fData[iByte+3]<<24;
  word |= fData[iByte+2]<<16;
  word |= fData[iByte+1]<<8;
  word |= fData[iByte];
   fPosition++;

  return word;

}

