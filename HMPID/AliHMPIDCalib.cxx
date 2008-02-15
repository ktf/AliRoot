//  **************************************************************************
//  * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
//  *                                                                        *
//  * Author: The ALICE Off-line Project.                                    *
//  * Contributors are mentioned in the code where appropriate.              *
//  *                                                                        *
//  * Permission to use, copy, modify and distribute this software and its   *
//  * documentation strictly for non-commercial purposes is hereby granted   *
//  * without fee, provided that the above copyright notice appears in all   *
//  * copies and that both the copyright notice and this permission notice   *
//  * appear in the supporting documentation. The authors make no claims     *
//  * about the suitability of this software for any purpose. It is          *
//  * provided "as is" without express or implied warranty.                  *
//  **************************************************************************

#include "AliHMPIDCalib.h" //class header
#include "AliHMPIDParam.h" //class header
#include "AliHMPIDRawStream.h" //class header
#include <fstream>
#include <TTree.h>



ClassImp(AliHMPIDCalib) 


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AliHMPIDCalib::AliHMPIDCalib():
faddl(0x0),
fsq(0x0),
fsq2(0x0),
fnpc(0x0),
fpedQ0(0x0),
fErr(0x0),
fPadAdc(0x0),
fIsPad(0x0),
fFile(0x0),
fLdcId(0),
fTimeStamp(0),
fRunNum(0),
fSigCut(0),
fWritePads(0),
fnDDLInStream(0x0),
fnDDLOutStream(0x0),
fLargeHisto(kFALSE),
fSelectDDL(0)  
{
  //
  //constructor
  //
  faddl = new Bool_t[AliHMPIDRawStream::kNDDL];
  Int_t nPads =  (AliHMPIDParam::kMaxCh+1)*(AliHMPIDParam::kMaxPcx+1)*(AliHMPIDParam::kMaxPcy+1);
 
  fpedQ0 = new Int_t***[AliHMPIDRawStream::kNDDL+1];
  fsq2   = new Float_t ***[AliHMPIDRawStream::kNDDL+1];
  fsq    = new Float_t ***[AliHMPIDRawStream::kNDDL+1];
  fnpc   = new Int_t ***[AliHMPIDRawStream::kNDDL+1];
    fErr = new Int_t*[AliHMPIDRawStream::kNDDL+1];
   
  fnDDLInStream  = new Int_t[AliHMPIDRawStream::kNDDL+1];
  fnDDLOutStream = new Int_t[AliHMPIDRawStream::kNDDL+1];

  
  for(Int_t iDDL=0;iDDL<AliHMPIDRawStream::kNDDL+1;iDDL++) {
    
      fErr[iDDL] = new Int_t[AliHMPIDRawStream::kSumErr+1];
    fpedQ0[iDDL] = new Int_t**[AliHMPIDRawStream::kNRows+1];
       fsq[iDDL] = new Float_t**[AliHMPIDRawStream::kNRows+1];
      fsq2[iDDL] = new Float_t**[AliHMPIDRawStream::kNRows+1];
      fnpc[iDDL] = new Int_t**[AliHMPIDRawStream::kNRows+1];
      
      for(Int_t iRow=0;iRow<AliHMPIDRawStream::kNRows+1;iRow++)  {
      
       fpedQ0[iDDL][iRow] = new Int_t*[AliHMPIDRawStream::kNDILOGICAdd+1];
          fsq[iDDL][iRow] = new Float_t*[AliHMPIDRawStream::kNDILOGICAdd+1];
         fsq2[iDDL][iRow] = new Float_t*[AliHMPIDRawStream::kNDILOGICAdd+1];
         fnpc[iDDL][iRow] = new Int_t*[AliHMPIDRawStream::kNDILOGICAdd+1];
      
        for(Int_t iDil=1;iDil<AliHMPIDRawStream::kNDILOGICAdd+1;iDil++){
      
         fpedQ0[iDDL][iRow][iDil] = new Int_t[AliHMPIDRawStream::kNPadAdd+1];
           fsq2[iDDL][iRow][iDil] = new Float_t[AliHMPIDRawStream::kNPadAdd+1];
            fsq[iDDL][iRow][iDil] = new Float_t[AliHMPIDRawStream::kNPadAdd+1];
           fnpc[iDDL][iRow][iDil] = new Int_t[AliHMPIDRawStream::kNPadAdd+1];
          }//iDil
      }//iRow
   }//iDDL
    
   for(Int_t iDDL=0;iDDL<AliHMPIDRawStream::kNDDL+1;iDDL++) {
        
     fnDDLInStream[iDDL]=-1;
     fnDDLOutStream[iDDL]=-1;
      
     for(Int_t iErr=0;iErr<AliHMPIDRawStream::kSumErr+1;iErr++)  {fErr[iDDL][iErr]=0;}
         
     for(Int_t iRow=0;iRow<AliHMPIDRawStream::kNRows+1;iRow++) {
        for(Int_t iDil=1;iDil<AliHMPIDRawStream::kNDILOGICAdd+1;iDil++) {
          for(Int_t iPad=1;iPad<AliHMPIDRawStream::kNPadAdd+1;iPad++) {
            fpedQ0[iDDL][iRow][iDil][iPad]=0;
               fsq[iDDL][iRow][iDil][iPad]=0;
              fsq2[iDDL][iRow][iDil][iPad]=0;
              fnpc[iDDL][iRow][iDil][iPad]=0;
        }//iPad
      }//iDil
     }//iRow
   }//iDDL
    
  fPadAdc=new TH1I*[nPads];  
  fIsPad=new Bool_t[nPads];  
  for(Int_t np=0;np<nPads;np++) {fPadAdc[np]=0x0;   fIsPad[np]=kFALSE;}
  fWritePads=kFALSE;


  Init();
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AliHMPIDCalib::~AliHMPIDCalib()
{
  //
  //destructor
  //
  if (faddl)     { delete [] faddl;   faddl = 0x0;  } 
  if (fPadAdc)   { delete [] fPadAdc; fPadAdc=0x0;  }  
  if (fIsPad)    { delete [] fIsPad;  fIsPad=0x0;   }  
  if (fFile)     { delete    fFile;   fFile=0x0;    }  
 
  for(Int_t iErr=0;iErr<AliHMPIDRawStream::kSumErr+1;iErr++) { delete [] fErr[iErr];}  delete [] fErr;
  
  for(Int_t iDDL=0; iDDL< AliHMPIDRawStream::kNDDL; iDDL++) 
   for(Int_t iRow=0;iRow<AliHMPIDRawStream::kNRows+1;iRow++)         
     for(Int_t iDil=1;iDil<AliHMPIDRawStream::kNDILOGICAdd+1;iDil++)
      {
         delete [] fpedQ0[iDDL][iRow][iDil]; //del iPad
         delete []    fsq[iDDL][iRow][iDil]; //del iPad
         delete []   fsq2[iDDL][iRow][iDil]; //del iPad
         delete []   fnpc[iDDL][iRow][iDil]; //del iPad
       }
   for(Int_t iDDL=0; iDDL< AliHMPIDRawStream::kNDDL; iDDL++) 
     for(Int_t iRow=0;iRow<AliHMPIDRawStream::kNRows+1;iRow++)         
      {
        delete [] fpedQ0[iDDL][iRow];  //del iRow
          delete []  fsq[iDDL][iRow];  //del iRow
          delete [] fsq2[iDDL][iRow];  //del iRow
          delete [] fnpc[iDDL][iRow];  //del iRow
        }
       
   for(Int_t iDDL=0; iDDL< AliHMPIDRawStream::kNDDL; iDDL++) 
   {   
       delete [] fpedQ0[iDDL];        //del iRow
         delete [] fsq2[iDDL];        //del iRow
         delete []  fsq[iDDL];        //del iRow
         delete [] fnpc[iDDL];        //del iRow
     }
       
   delete [] fpedQ0;
   delete [] fsq2;
   delete [] fsq;
   delete [] fnpc;
    
  fpedQ0=0;    
    fsq2=0;
     fsq=0;
    fnpc=0;
    
  fLdcId=0;
  fTimeStamp=0;
  fRunNum=0;
  fSigCut=0;
  fWritePads=0;
  fLargeHisto=kFALSE;
  fSelectDDL=0;
}//dtor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCalib::Init()
{
  //
  //Init the q calc.
  //Arguments: none
  //Return: none
  //
    fSigCut=3;  
    for(Int_t iDDL=0; iDDL< AliHMPIDRawStream::kNDDL; iDDL++) 
      {
         for(Int_t ierr=0; ierr <AliHMPIDRawStream::kSumErr ; ierr++) {
            fErr[iDDL][ierr]=0;
            }
        
        faddl[iDDL]=kFALSE;
      }//DDL
}//Init()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCalib::SetRunParams(ULong_t runNum,Int_t timeStamp, Int_t ldcId)
{
  //  
  //Set run parameters for the Pedestal and Error Files
  //Arguments: run number, time stamp and LDC Id
  //Returns: none
  //
  fRunNum=(Int_t)runNum;
  fTimeStamp=timeStamp;
  fLdcId=ldcId;
}//SetRunParams()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCalib::SetSigCutFromFile(Char_t* name)
{
  //
  //Set Sigma Cut from the file on the LDC, if the input file is not present default value is set!
  //Arguments: the name of the SigmaCut file on the LDC
  //Returns: none
  //
  Int_t nSigCut=0;
  ifstream infile(name);
  if(!infile.is_open()) {fSigCut=3; return;}
  while(!infile.eof())
    {
    infile>>nSigCut;
  }
  infile.close();
  fSigCut=nSigCut; 
}//SetSigCutFromFile()    
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
void AliHMPIDCalib::InitHisto(Int_t q,Int_t histocnt,Char_t* name)
{
  //
  //Init the pad histos. For one DDL we have 11520 pads. ONLY if ENABLED!
  //Arguments: q-charge, the absolute number of the histogram (AliHMPIDParam::kMaxCh+1)*(AliHMPIDParam::kMaxPcx+1)*(AliHMPIDParam::kMaxPcy+1) and the name of the histogram (unique) 
  //Returns: none
  //
 if(fWritePads==kFALSE) return;
 fFile->cd();
 Double_t lowbin,highbin=0;
 lowbin=q-40.5; highbin=q+40.5;  
 
 if(fIsPad[histocnt]==kTRUE) return;
 
 if(fLargeHisto==kFALSE) fPadAdc[histocnt]=new TH1I(name,name,81,lowbin,highbin);
 if(fLargeHisto==kTRUE) fPadAdc[histocnt]=new TH1I(name,name,4093,-0.5,4092.5);
 fPadAdc[histocnt]->Sumw2();
 fIsPad[histocnt]=kTRUE;
 
}//InitHisto()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCalib::FillHisto(Int_t histocnt,Int_t q)
{
  //
  //Fill the ADC histograms for each pad
  //Arguments:  q-charge, the absolute number of the histogram (AliHMPIDParam::kMaxCh+1)*(AliHMPIDParam::kMaxPcx+1)*(AliHMPIDParam::kMaxPcy+1)
  //Returns: none
  //
  if(fIsPad[histocnt]==kFALSE) return;
  fFile->cd();
  fPadAdc[histocnt]->Fill(q);
 
}//InitHisto()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCalib::InitFile(Int_t ldcId)
{
  //
  //Initialize the ADC histo output file (one per LDC)
  //Arguments: LDC Id
  //Returns: none
  //
  if(fWritePads==kFALSE) return;
  fFile=new TFile(Form("HmpidPadsOnLdc%2d.root",ldcId),"RECREATE");
}//InitFile()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCalib::CloseFile(Int_t /*ldcId*/)
{
  //
  //Close the ADC histo output file (one per LDC)
  //Arguments: LDC Id
  //Returns: none
  //
  fFile->cd();
  Int_t nPads = (AliHMPIDParam::kMaxCh+1)*(AliHMPIDParam::kMaxPcx+1)*(AliHMPIDParam::kMaxPcy+1);
  for(Int_t np=0;np<nPads;np++) {if(fIsPad[np]==kTRUE) fPadAdc[np]->Write();} 
  fFile->Close();
}//CloseFile()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCalib::FillPedestal(Int_t abspad,Int_t q)
{
  //
  //Called from the HMPIDda and fills the pedestal values
  //Arguments: absulote pad number as from AliHMPIDParam and q-charge
  //Returns: none
  //
  if(q<0) AliFatal("Negative charge is read!!!!!!");
  
  Int_t nDDL=0, row=0, dil=0, adr=0;
  //The decoding (abs. pad -> ddl,dil,...) is the same as in AliHMPIDDigit::Raw
  Int_t y2a[6]={5,3,1,0,2,4};

       nDDL=  2*AliHMPIDParam::A2C(abspad)+AliHMPIDParam::A2P(abspad)%2;              //DDL# 0..13
 // Int_t tmp=  1+AliHMPIDParam::A2P(abspad)/2*8+AliHMPIDParam::A2Y(abspad)/6;        //temp variable
  Int_t tmp=   AliHMPIDParam::A2P(abspad)/2*8+AliHMPIDParam::A2Y(abspad)/6;           //temp variable
//        row=   (AliHMPIDParam::A2P(abspad)%2)? 25-tmp:tmp;                          //row r=1..24
        row=   (AliHMPIDParam::A2P(abspad)%2)? tmp:24-tmp;                            //row r=1..24
        dil=  1+AliHMPIDParam::A2X(abspad)/8;                                         //DILOGIC 
        adr=y2a[AliHMPIDParam::A2Y(abspad)%6]+6*(AliHMPIDParam::A2X(abspad)%8);       //ADDRESS 0..47 
  //........... decoding done      

     if(q>0) { 
        fsq[nDDL][row][dil][adr]+=q;
      fsq2[nDDL][row][dil][adr]+=q*q;
      fnpc[nDDL][row][dil][adr]++;                                                     //Count how many times the pad is good (can be different from the good DDL  count)
                       faddl[nDDL]=kTRUE; 
                     }
      else
      {
        fpedQ0[nDDL][row][dil][adr]++;                                                 //Count how many times a pad charge is zero
      }
      
     Int_t histocnt=0;   histocnt=(nDDL)*11520+(row-1)*480+(dil-1)*48+adr;             //Histo counter for a single DDL  
     
     if(fWritePads==kTRUE)                                                             //works but make it nicer later....
     { 
       if( fLargeHisto==kTRUE && nDDL==fSelectDDL) {              
         InitHisto(q,histocnt,Form("hDDL_%d_Row_%d_Dil_%d_Pad_%d",nDDL,row,dil,adr));  //for large histos use hardware naming
         FillHisto(histocnt,q);
        }
        if(fLargeHisto==kFALSE)
        {
         InitHisto(q,histocnt,Form("hPad_Ch_%d_Pc_%d_Px_%d_Py_%d",AliHMPIDParam::A2C(abspad),AliHMPIDParam::A2P(abspad),AliHMPIDParam::A2X(abspad),AliHMPIDParam::A2Y(abspad))); 
         FillHisto(histocnt,q);  
        }
      }//fWritePads
            
}//FillPedestal()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCalib::FillErrors(Int_t nDDL,Int_t eType, Int_t nErr)
{
  //
  //Fill decoding errors from AliHMPIDRawStream
  //Arguments: nDDL-DDL number, eType- error type as in AliHMPIDRawStream.h and the # of occurence for eType
  //Retutns: none
  //
    if(nErr<=0) return;
    if(eType < 0 || eType> AliHMPIDRawStream::kSumErr ) return;
    fErr[nDDL][eType]=fErr[nDDL][eType]+nErr;
    
  
}//FillErrors()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDCalib::FillDDLCnt(Int_t iddl,Int_t inDDL, Int_t outDDL)
{
  //
  //Fill decoding DDL check from RawStream
  //Arguments: iddl - DDL under setting, inDDL- How many times the DDL is present in the raw stream, outDDL - How many time sthe DDL is succesfylly decoded
  //Retutns: none
  //
 
  if(inDDL==-1) return;
  if(fnDDLInStream[iddl]==-1) {fnDDLInStream[iddl]=0; fnDDLOutStream[iddl]=0;}
  fnDDLInStream[iddl]+=inDDL;
  fnDDLOutStream[iddl]+=outDDL;
 
  
}//FillDDLCnt()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Bool_t AliHMPIDCalib::WriteErrors(Int_t nDDL, Char_t* name, Int_t nEv)
{
  //
  //Write decoding errors to a txt file
  //Arguments: nDDL-DDL number, name of the error file and number of the read events
  //Retutns: kTRUE/kFALSE
  //
  
  if(faddl[nDDL]==kFALSE) return kFALSE;                                                                 //if ddl is missing no error file is created
  ofstream outerr;  outerr.open(name);                                                                   //open error file
  outerr << Form("%8s %2d\n","RunNumber",(Int_t)fRunNum);                                                //read run number
  outerr << Form("%8s %2d\n","LdcId" ,          fLdcId);                                                 //read LDC Id
  outerr << Form("%8s %2d\n","TimeStamp",       fTimeStamp);                                             //read time stamp
  outerr << Form("%8s %2d\n","TotNumEvt",       nEv);                                                    //read number of total events processed
  outerr << Form("%8s %2d\n","TotDDLEvt",       fnDDLInStream[nDDL]);                                    //read number of bad events for DDL # nDDL processed
  outerr << Form("%8s %2d\n","NumBadEvt",       fnDDLInStream[nDDL]-fnDDLOutStream[nDDL]);               //read number of bad events for DDL # nDDL processed
  outerr << Form("%8s %2.2f\n","NBadE(%)",      (fnDDLInStream[nDDL]-fnDDLOutStream[nDDL])*100.0/nEv);   //read number of bad events (in %) for DDL # nDDL processed
  
  for(Int_t  ierr=0; ierr <AliHMPIDRawStream::kSumErr; ierr++) outerr << Form("%2d\t",fErr[nDDL][ierr]); //write errors
                                                               outerr << Form("\n");                     //last break
  outerr.close();                                                                                        //write error file
  return kTRUE;
    
}//FillErrors()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Bool_t AliHMPIDCalib::CalcPedestal(Int_t nDDL, Char_t* name, Int_t nEv)    
{
  //
  //Calculate pedestal for each pad  
  //Arguments: nDDL-DDL number, name of the pedestal file and number of the read events
  //Retutns: kTRUE/kFALSE
  //
  
   
  if(faddl[nDDL]==kFALSE) return kFALSE;                   //if ddl is missing no ped file is created (and also for LDC selection). Check with Paolo what he checks for?!  
  Double_t mean=0,sigma=0;
  Double_t qs2m=0,qsm2=0;
  ofstream out;                                           //to write the pedestal text files
  Int_t inhard;
  Int_t nEvPerPad=0;
  out.open(name);
  out << Form("%8s %2d\n","RunNumber",(Int_t)fRunNum);                                                //read run number
  out << Form("%8s %2d\n","LdcId" ,         fLdcId);                                                  //read LDC Id
  out << Form("%8s %2d\n","TimeStamp",      fTimeStamp);                                              //read time stamp
  out << Form("%8s %2d\n","TotNumEvt",      nEv);                                                     //read number of total events processed
  out << Form("%8s %2d\n","TotDDLEvt",      fnDDLInStream[nDDL]);                                     //read number of bad events for DDL # nDDL processed
  out << Form("%8s %2d\n","NumBadEvt",      fnDDLInStream[nDDL]-fnDDLOutStream[nDDL]);                //read number of bad events for DDL # nDDL processed
  out << Form("%8s %2f\n","NBadE(%)",       (fnDDLInStream[nDDL]-fnDDLOutStream[nDDL])*100.0/nEv);    //read number of bad events (in %) for DDL # nDDL processed
  out << Form("%8s %2.2d\n","#SigCut",      fSigCut);                                                 //# of sigma cuts
      
  for(Int_t row = 1; row <= AliHMPIDRawStream::kNRows; row++){
    for(Int_t dil = 1; dil <= AliHMPIDRawStream::kNDILOGICAdd; dil++){
      for(Int_t pad = 0; pad < AliHMPIDRawStream::kNPadAdd; pad++){
        
        mean  = 50;sigma = 100;
        
        nEvPerPad=fnpc[nDDL][row][dil][pad];
        
        
        if(nEvPerPad < 1 ) {                    //if the pad is bad then we assign 100  for the sigma and 50 for the mean
          mean  = 4000;
          sigma = 1000;
        }
        else{        
         mean = fsq[nDDL][row][dil][pad]*1.0/nEvPerPad;
         qs2m = fsq2[nDDL][row][dil][pad]*1.0/nEvPerPad;
         qsm2 = TMath::Power(fsq[nDDL][row][dil][pad]*1.0/nEvPerPad,2); 
        sigma = TMath::Sqrt(TMath::Abs(qs2m-qsm2));
        }
                
        inhard=((Int_t(mean))<<9)+Int_t(mean+3*sigma);
        out << Form("%2i %2i %2i %5.2f %5.2f %4.4x \n",row,dil,pad,mean,sigma,inhard);
                 
       //if(sigma > 3.0) Printf("WARNING SIGMA DDL: %2d row: %2d dil: %2d pad: %2d mean: %3.2f sigma: %2.2f nEvPerPad: %02d fnDDLOutStream: %02d fpedQ0: %02d",nDDL,row,dil,pad,mean,sigma,nEvPerPad,fnDDLOutStream[nDDL],fpedQ0[nDDL][row][dil][pad]);
        
       
        }//adr
      }//dil
    }//row
    out.close();                                          //write pedestal file
  return kTRUE;
}//CaclPedestal()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

