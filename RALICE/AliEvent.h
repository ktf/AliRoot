#ifndef ALIEVENT_H
#define ALIEVENT_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

// $Id: AliEvent.h,v 1.19 2004/10/20 10:49:44 nick Exp $

#include <math.h>
 
#include "TObject.h"
#include "TObjArray.h"
#include "TDatime.h"
#include "TTimeStamp.h"
 
#include "AliVertex.h"
#include "AliDevice.h"
 
class AliEvent : public AliVertex
{
 public:
  AliEvent();                             // Default constructor
  AliEvent(Int_t n);                      // Create an event to hold initially n tracks
  virtual ~AliEvent();                    // Default destructor
  AliEvent(const AliEvent& evt);          // Copy constructor
  virtual TObject* Clone(const char* name="") const; // Make a deep copy and provide its pointer
  virtual void SetOwner(Bool_t own=kTRUE);// Set ownership of all added objects
  void SetDayTime(TTimeStamp& stamp);     // Set the date and time stamp exactly as specified (1 ns accuracy)
  void SetDayTime(TDatime& stamp);        // Set date and time stamp interpreted as local time (1 s accuracy)
  void SetRunNumber(Int_t run);           // Set the run number
  void SetEventNumber(Int_t evt);         // Set the event number
  void SetProjectile(Int_t a,Int_t z,Double_t pnuc,Int_t id=0); // Set projectile A, Z, p per nucleon and id
  Int_t GetProjectileA() const;           // Provide A value of the projectile
  Int_t GetProjectileZ() const;           // Provide Z value of the projectile
  Double_t GetProjectilePnuc() const;     // Provide the projectile momentum value per nucleon
  Int_t GetProjectileId() const;          // Provide the user defined particle ID of the projectile
  void SetTarget(Int_t a,Int_t z,Double_t pnuc,Int_t id=0); // Set target A, Z, p per nucleon and id
  Int_t GetTargetA() const;               // Provide A value of the target
  Int_t GetTargetZ() const;               // Provide Z value of the target
  Double_t GetTargetPnuc() const;         // Provide the target momentum value per nucleon
  Int_t GetTargetId() const;              // Provide the user defined particle ID of the target
  void Reset();                           // Reset all values
  TTimeStamp GetDayTime() const;          // Provide the date and time stamp
  Int_t GetRunNumber() const;             // Provide the run number
  Int_t GetEventNumber() const;           // Provide the event number
  virtual void HeaderData() const;        // Print the event header information
  virtual void Data(TString f="car");     // Print the event info within coordinate frame f
  void SetDevCopy(Int_t j);               // (De)activate creation of private copies of the devices
  Int_t GetDevCopy() const;               // Provide DevCopy flag value      
  void AddDevice(TObject& d);             // Add a device to the event
  void AddDevice(TObject* d) { AddDevice(*d); }
  Int_t GetNdevices() const;              // Provide the number of devices
  void ShowDevices() const;               // Provide on overview of the available devices
  TObject* GetDevice(Int_t i) const;      // Provide i-th device of the event
  TObject* GetDevice(TString name) const; // Provide the device with name "name"
  TObject* GetIdDevice(Int_t id) const;   // Provide the device with unique identifier "id"
  Int_t GetNhits(const char* classname);  // Provide number of hits for the specified device class
  TObjArray* GetHits(const char* classname); // Provide refs to all hits of the specified device class 
  AliSignal* GetIdHit(Int_t id,const char* classname); // Provide hit with unique "id" for the specified device class
  TObjArray* SortHits(const char* classname,TString name,Int_t mode=-1); // Sort hits by named signal value
  TObjArray* SortHits(const char* classname,Int_t idx=1,Int_t mode=-1);  // Sort hits by indexed signal value
  void GetExtremes(const char* classname,Float_t& vmin,Float_t& vmax,Int_t idx=1); // Get min. and max. signal value
  void GetExtremes(const char* classname,Float_t& vmin,Float_t& vmax,TString name);// Get min. and max. signal value
  void DisplayHits(const char* classname,TString name,Float_t scale=-1,Int_t dp=0,Int_t mstyle=8,Int_t mcol=4);
  void DisplayHits(const char* classname,Int_t idx=1,Float_t scale=-1,Int_t dp=0,Int_t mstyle=8,Int_t mcol=4);
  TObjArray* SortDevices(const char* classname,TString name,Int_t mode=-1); // Sort devices by signal value
  TObjArray* SortDevices(const char* classname,Int_t idx=1,Int_t mode=-1);  // Sort devices by signal value
  TObjArray* SortDevices(TObjArray* hits,TString name,Int_t mode=-1);       // Sort devices by signal value
  TObjArray* SortDevices(TObjArray* hits,Int_t idx=1,Int_t mode=-1);        // Sort devices by signal value

 protected:
  TTimeStamp fDaytime;                  // The date and time stamp
  Int_t fRun;                           // The run number
  Int_t fEvent;                         // The event number
  Int_t fAproj;                         // The projectile A value
  Int_t fZproj;                         // The projectile Z value
  Double_t fPnucProj;                   // The projectile momentum per nucleon
  Int_t fIdProj;                        // User defined projectile particle ID
  Int_t fAtarg;                         // The target A value
  Int_t fZtarg;                         // The target Z value
  Double_t fPnucTarg;                   // The target momentum per nucleon
  Int_t fIdTarg;                        // User defined target particle ID
  TObjArray* fDevices;                  // Array to hold the pointers to the various devices
  Int_t fDevCopy;                       // Flag to denote creation of private copies of the devices
  void LoadHits(const char* classname); // Load references to the hits registered to the specified device class
  TObjArray* fHits;                     //! Temp. array to hold references to the registered AliDevice hits
  TObjArray* fOrdered;                  //! Temp. array to hold references to various ordered objects
  TObject* fDisplay;                    //! Temp. pointer to hold objects which serve event displays

 ClassDef(AliEvent,17) // Creation and investigation of an Alice physics event.
};
#endif
