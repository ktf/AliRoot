/* Copyright(c) 1998-2003, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

///////////////////////////////////////////////////////////////////////////////////////////
// Class used for generating the files containing raw data, required for  Data Challenge //
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef AliITSDDLRAWDATA_H
#define AliITSDDLRAWDATA_H

class AliITS;
class TTree;

class AliITSDDLRawData:public TObject{
 public:
  AliITSDDLRawData();//default constructor
  virtual ~AliITSDDLRawData(){;}//destructor
  AliITSDDLRawData(const AliITSDDLRawData &source); // copy constructor
  AliITSDDLRawData& operator=(const AliITSDDLRawData &source); // ass. op.
 Int_t RawDataSPD(AliITS *ITS,TTree *TD ,Int_t LDCsNumber,Int_t eventNumber);
  // This method generates the files with the Silicon pixel detector data
  Int_t RawDataSDD(AliITS *ITS,TTree *TD ,Int_t LDCsNumber,Int_t eventNumbe); 
  // This method generates the files with the Silicon drift detector data
  Int_t RawDataSSD(AliITS *ITS,TTree *TD ,Int_t LDCsNumber,Int_t eventNumbe);
  // This method generates the files with the Silicon pixel detector data
  void  TestFormat(Int_t eventNumber);
  // A debugging method used to test the files generated for the SPD.
  void SetVerbose(Int_t Verbose){fVerbose=Verbose;}
  // To set the verbose level
 private: 
  void  GetDigitsSPD(TClonesArray *ITSdigits, Int_t mod,Int_t ddl,UInt_t *buf);
  //This method formats and stores in buf all the digits of a SPD module
  void  GetDigitsSDD(TClonesArray *ITSdigits, Int_t mod,Int_t modR,Int_t ddl,UInt_t *buf);
  //This method formats and stores in buf all the digits of a SDD module
  void  GetDigitsSSD(TClonesArray *ITSdigits, Int_t mod,Int_t modR,Int_t ddl,UInt_t *buf);
  //This method formats and stores in buf all the digits of a SSD module
  void  PackWord(UInt_t &BaseWord, UInt_t Word, Int_t StartBit, Int_t StopBit);
  //This method stores the value of the variable Word of StopBit-StartBit+1 bits 
  //in BaseWord, starting from the bit StartBit
  void  UnpackWord(UInt_t PackedWord, Int_t StartBit, Int_t StopBit, UInt_t &Word);
  //This method extracts a group of adjacent bits, specified by StartBit and StopBit, 
  //from the word PackedWord. The resulting word is saved in the Word variable
  void  WriteChipHeader(Int_t ChipAddr,Int_t EventCnt,UInt_t &BaseWord);
  void  WriteChipTrailer(UInt_t *buf,Int_t ChipHitCount,UInt_t &BaseWord);
  void  WriteHit(UInt_t *buf,Int_t RowAddr,Int_t HitAddr,UInt_t &BaseWord);
  //The three previous  methods are used to store the data according to the 
  //Silicon pixel detector data format
  void  ReadChipHeader(Int_t &ChipAddr,Int_t &EventCnt,UInt_t BaseWord);
  void  ReadChipTrailer(Int_t &ChipHitCount,UInt_t BaseWord);
  //Methods used for reading and dubugging SPD data files
  Int_t fVerbose;            //Verbose level (0:no msg, 1:msg, 2:digits in txt files)
  Int_t fIndex;             //number of 32 words to be stored into the output file
  Int_t fHalfStaveModule;     //first or second half of an Half Stave module
  ClassDef(AliITSDDLRawData,1)
};
    
#endif
