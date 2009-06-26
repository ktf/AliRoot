#ifndef ALIVZEROLOGICALSIGNAL_H
#define ALIVZEROLOGICALSIGNAL_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights
 * reserved. 
 *
 * See cxx source for full Copyright notice                               
 */

#include <TObject.h>

class AliVZEROLogicalSignal  : public TObject {
public:
	AliVZEROLogicalSignal();
	AliVZEROLogicalSignal(Float_t start, Float_t stop);
	AliVZEROLogicalSignal(UShort_t profilClock, UInt_t delay);
	~AliVZEROLogicalSignal();
	AliVZEROLogicalSignal(const AliVZEROLogicalSignal &signal);
	AliVZEROLogicalSignal& operator= (const AliVZEROLogicalSignal &signal);
	AliVZEROLogicalSignal operator& (const AliVZEROLogicalSignal &signal) const;
	AliVZEROLogicalSignal operator| (const AliVZEROLogicalSignal &signal) const;
	// Print method
	virtual void Print(Option_t* opt="") const { AliInfo(Form("\t -> Start %f Stop %f\n ",fStart,fStop));}
	
	Float_t GetStartTime() const {return fStart;};
	Float_t GetStopTime() const {return fStop;};
	Float_t GetWidth() const {return (fStop - fStart);};
	
	void SetStartTime(Float_t time);
	void SetStopTime(Float_t time);
	
	Bool_t IsInCoincidence(Float_t time);
	
private:
	
	Float_t fStart; // Start Time of the signal with respect to the LHC Clock
	Float_t fStop;  // Stop Time of the signal with respect to the LHC Clock
	
	
	ClassDef( AliVZEROLogicalSignal, 1 )  
	
};

#endif // ALIVZEROLOGICALSIGNAL_H
