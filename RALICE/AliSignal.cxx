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

// $Id$

///////////////////////////////////////////////////////////////////////////
// Class AliSignal
// Generic handling of (extrapolated) detector signals.
//
// Note :
// ------
// Signal positions (r) and reference frames (f) are specified via
// SetPosition(r,f) under the following conventions :
//
// f="car" ==> r is Cartesian   (x,y,z)
// f="sph" ==> r is Spherical   (r,theta,phi)
// f="cyl" ==> r is Cylindrical (rho,phi,z)
//
// The same holds for SetPositionErrors().
//
// All angles are in radians.
//
// Example :
// ---------
//
// AliSignal s;
// s.SetName("Start counter");
// Float_t pos[3]={-1,25,7};
// Float_t err[3]={0.03,0.7,0.18};
// Float_t signal=120.8;
// Float_t error=1.73;
// s.SetPosition(pos,"car");
// s.SetPositionErrors(err,"car");
// s.SetSignal(signal);
// s.SetSignalError(error);
// Float_t loc[3],dr[3],sigma;
// s.GetPosition(loc,"sph");
// s.GetPositionErrors(dr,"sph");
// Float_t adc=s.GetSignal();
// Float_t sigma=s.GetSignalError();
//
// AliSignal q(3); // q can store initially 3 signal values with their errors
//                 // In the example below a signal contains the
//                 // following data : timing, ADC and dE/dx
// q.SetName("TOF hit");
// q.SetPosition(pos,"car");
// q.SetPositionErrors(err,"car");
// signal=82.5; // e.g. signal time in ns
// error=2.01;
// q.SetSignal(signal,1);
// q.SetSignalError(error,1);
// signal=268.1; // e.g. ADC value of signal
// error=3.75;
// q.SetSignal(signal,2);
// q.SetSignalError(error,2);
// signal=23.7; // e.g. corresponding dE/dx value
// error=0.48;
// q.SetSignal(signal,3);
// q.SetSignalError(error,3);
//
//--- Author: Nick van Eijndhoven 23-jan-1999 UU-SAP Utrecht
//- Modified: NvE $Date$ UU-SAP Utrecht
///////////////////////////////////////////////////////////////////////////

#include "AliSignal.h"
 
ClassImp(AliSignal) // Class implementation to enable ROOT I/O
 
AliSignal::AliSignal(Int_t n)
{
// Creation of an AliSignal object and initialisation of parameters.
// Initially a total of n (default n=1) values (with errors) can be stored.
// If needed, the number of values (and errors) will be increased automatically
// when entering values.
 fSignal=0;
 fDsignal=0;
 fName="Unspecified";
}
///////////////////////////////////////////////////////////////////////////
AliSignal::~AliSignal()
{
// Destructor to delete dynamically allocated memory
 if (fSignal)
 {
  delete fSignal;
  fSignal=0;
 }
 if (fDsignal)
 {
  delete fDsignal;
  fDsignal=0;
 }
}
///////////////////////////////////////////////////////////////////////////
AliSignal::AliSignal(AliSignal& s)
{
// Copy constructor
 fSignal=0;
 fDsignal=0;
 fName=s.GetName();
 
 SetPosition((Ali3Vector&)s);

 Int_t nvalues=s.GetNvalues();
 Double_t sig,err;
 for (Int_t i=1; i<=nvalues; i++)
 {
  sig=s.GetSignal(i);
  err=s.GetSignalError(i);
  SetSignal(sig,i);
  SetSignalError(err,i);
 } 
}
///////////////////////////////////////////////////////////////////////////
void AliSignal::Reset()
{
// Reset all signal and position values and errors to 0.

 if (fSignal && fDsignal)
 {
  Double_t r[3]={0,0,0};
  SetPosition(r,"sph");
  SetErrors(r,"car");
  for (Int_t i=0; i<fSignal->GetSize(); i++)
  {
   fSignal->AddAt(0,i);
   fDsignal->AddAt(0,i);
  }
 }
}
///////////////////////////////////////////////////////////////////////////
void AliSignal::ResetSignals()
{
// Reset all signal values and their errors to 0.

 if (fSignal && fDsignal)
 {
  for (Int_t i=0; i<fSignal->GetSize(); i++)
  {
   fSignal->AddAt(0,i);
   fDsignal->AddAt(0,i);
  }
 }
}
///////////////////////////////////////////////////////////////////////////
void AliSignal::ResetPosition()
{
// Reset the position and corresponding errors to 0.
 Double_t r[3]={0,0,0};
 SetPosition(r,"sph");
 SetErrors(r,"car");
}
///////////////////////////////////////////////////////////////////////////
void AliSignal::SetSignal(Double_t sig,Int_t j)
{
// Store j-th (default j=1) signal value.
// Note : The first signal value is at j=1.
// In case the value of the index j exceeds the maximum number of reserved
// slots for signal values, the number of reserved slots for both the
// signal values and errors is increased automatically.

 if (!fSignal && !fDsignal)
 {
  fSignal=new TArrayF(j);
  fDsignal=new TArrayF(j);
  ResetSignals();
 }

 Int_t size=fSignal->GetSize();

 if (j>size)
 {
  fSignal->Set(j);
  fDsignal->Set(j);
 }

 fSignal->AddAt(float(sig),j-1);
}
///////////////////////////////////////////////////////////////////////////
void AliSignal::AddSignal(Double_t sig,Int_t j)
{
// Add value to j-th (default j=1) signal value.
// Note : The first signal value is at j=1.
// In case the value of the index j exceeds the maximum number of reserved
// slots for signal values, the number of reserved slots for both the
// signal values and errors is increased automatically.

 if (!fSignal && !fDsignal)
 {
  fSignal=new TArrayF(j);
  fDsignal=new TArrayF(j);
  ResetSignals();
 }

 Int_t size=fSignal->GetSize();

 if (j>size)
 {
  fSignal->Set(j);
  fDsignal->Set(j);
 }

 Float_t sum=(fSignal->At(j-1))+sig;
 fSignal->AddAt(sum,j-1);
}
///////////////////////////////////////////////////////////////////////////
Float_t AliSignal::GetSignal(Int_t j)
{
// Provide j-th (default j=1) signal value.
// Note : The first signal value is at j=1.
 if (fSignal)
 {
  return fSignal->At(j-1);
 }
 else
 {
  return 0;
 }
}
///////////////////////////////////////////////////////////////////////////
void AliSignal::SetSignalError(Double_t dsig,Int_t j)
{
// Store error on j-th (default j=1) signal value.
// Note : The error on the first signal value is at j=1.
// In case the value of the index j exceeds the maximum number of reserved
// slots for signal error values, the number of reserved slots for both the
// signal values and errors is increased automatically.

 if (!fSignal && !fDsignal)
 {
  fSignal=new TArrayF(j);
  fDsignal=new TArrayF(j);
  ResetSignals();
 }

 Int_t size=fDsignal->GetSize();

 if (j>size)
 {
  fSignal->Set(j);
  fDsignal->Set(j);
 }

 fDsignal->AddAt(float(dsig),j-1);
}
///////////////////////////////////////////////////////////////////////////
Float_t AliSignal::GetSignalError(Int_t j)
{
// Provide error on the j-th (default j=1) signal value.
// Note : The error on the first signal value is at j=1.
 if (fDsignal)
 {
  return fDsignal->At(j-1);
 }
 else
 {
  return 0;
 }
}
///////////////////////////////////////////////////////////////////////////
void AliSignal::Data(TString f)
{
// Provide signal information within the coordinate frame f
 cout << " *AliSignal::Data* For signal of kind : " << fName << endl;
 cout << " Position";
 Ali3Vector::Data(f); 
 
 if (fSignal && fDsignal)
 {
  for (Int_t i=0; i<fSignal->GetSize(); i++)
  {
   cout << "   Signal value : " << fSignal->At(i)
        << " error : " << fDsignal->At(i) << endl;
  }
 }
} 
///////////////////////////////////////////////////////////////////////////
void AliSignal::SetName(TString name)
{
// Set the name tag to indicate the kind of signal.
 fName=name;
}
///////////////////////////////////////////////////////////////////////////
TString AliSignal::GetName()
{
// Provide the name tag indicating the kind of signal.
 return fName;
}
///////////////////////////////////////////////////////////////////////////
Int_t AliSignal::GetNvalues()
{
// Provide the number of values for this signal.
 Int_t n=0;
 if (fSignal && fDsignal) n=fSignal->GetSize();
 return n;
}
///////////////////////////////////////////////////////////////////////////
