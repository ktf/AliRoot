#ifndef AliHMPIDCalib_h
#define AliHMPIDCalib_h
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

// Class of HMPID to manage digits ---> pads
//.
//.
//.

//#include "TTreePlayer.h"
//#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TH1S.h>
#include <TMath.h>
#include <TFile.h>
#include "AliHMPIDParam.h"
#include "AliHMPIDRawStream.h"

class TH1I;
class TFile;
class AliHMPIDCalib: public TObject { 


public:
  AliHMPIDCalib();
  virtual ~AliHMPIDCalib();
          void Init();
          void FillPedestal(Int_t pad,Int_t q);                             //absolute pad number and the charge of the pad
          void FillErrors(Int_t nDDL,Int_t nErrType, Int_t nErr);           //Fill the errors from RawStream
        Bool_t CalcPedestal(Int_t nDDL, Char_t* name, Int_t nEv);           //number of the DDL, name of the output file and the number of events processed
        Bool_t WriteErrors(Int_t nDDL, Char_t* name, Int_t nEv);            //number of the DDL, name of the output file and the number of events processed
         void InitHisto(Int_t q,Int_t histocnt,Char_t* name);               //Init the pad histograms
         void FillHisto(Int_t histocnt,Int_t q);                            //Fill the ADC histograms
         void InitFile(Int_t nDDL);                                         //Init the ADC histo output file (one per LDC)
         void CloseFile(Int_t nDDL);                                        //Close the file
         void SetRunParams(ULong_t runNum,Int_t timeStamp, Int_t ldcId);    //Set Run Parameters such as Run Number, TimeStamp, LDCid 
  inline void SetSigCut(Int_t nSigCut) { fSigCut=nSigCut;}                  //Set Sigma Cuts from Setter
         void SetSigCutFromFile(Char_t* name);                              //Set Sigma Cuts from File
  inline void SetWriteHistoPads(Bool_t isOn) {fWritePads=isOn;}             //Set wether ADC histos of pads are written or not
  inline Bool_t GetWritePads()            {return fWritePads;}              //Set wether ADC histos of pads are written or not
protected: 

    Bool_t  *faddl;                                                         //check is ddl is filled
    Float_t  fsq[AliHMPIDRawStream::kNDDL+1][AliHMPIDRawStream::kNRows+1][AliHMPIDRawStream::kNDILOGICAdd+1][AliHMPIDRawStream::kNPadAdd+1];                           //Sum of pad Q
    Float_t fsq2[AliHMPIDRawStream::kNDDL+1][AliHMPIDRawStream::kNRows+1][AliHMPIDRawStream::kNDILOGICAdd+1][AliHMPIDRawStream::kNPadAdd+1];                          //Sum of pad Q^2
    Int_t   fErr[AliHMPIDRawStream::kNDDL+1][AliHMPIDRawStream::kSumErr+1];                                            // Store the numner of errors for a given error type and a given DDL
    TH1I   **fPadAdc;                                                            //Charge distribution for pads    
    Bool_t  *fIsPad;                                                             //Check if the ADC histo for the pad is booked or not
    TFile   *fFile;                                                              //ADC histo output file (one per LDC)      
    UInt_t  fLdcId;                                                              //Ldc ID 
    UInt_t  fTimeStamp;                                                          //Time Stamp
    Int_t   fRunNum;                                                             //Run Number
    Int_t   fSigCut;                                                             //n. of pedestal distribution sigmas used to create zero suppresion table                          
    Bool_t  fWritePads;                                                          //Select wether to write ADC pad histograms or not
    ClassDef(AliHMPIDCalib,2)                                                    //HMPID calibration and pedestal class        
};
#endif
