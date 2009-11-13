//-*- Mode: C++ -*-
// $Id: $
#ifndef AliHLTMUONSPECTROSCALARS_H
#define AliHLTMUONSPECTROSCALARS_H
/* This file is property of and copyright by the ALICE HLT Project        *
 * ALICE Experiment at CERN, All rights reserved.                         *
 * See cxx source for full Copyright notice                               */

///  @file   AliHLTMuonSpectroScalars.h
///  @author Artur Szostak <artursz@iafrica.com>
///  @date   9 Nov 2009
///  @brief  Declares the scalars class for the muon spectrometer.

#include "TObject.h"
#include "TObjArray.h"
#include "TClonesArray.h"

/**
 * @class AliHLTMuonSpectroScalars
 * @brief Muon spectrometer HLT trigger summary scalars.
 *
 * This scalars class contains summary information such as the number of tracks found,
 * number of pairs found and so on, all within the muon spectrometer.
 * Objects of this class are generated by the AliHLTMuonSpectroTriggerComponent to
 * be used in the HLT global trigger and/or added to the AliHLTEventSummary object.
 *
 * \ingroup alihlt_trigger_components
 */
class AliHLTMuonSpectroScalars : public TObject
{
public:
	/**
	 * Scalar class to store all the required information for a muon spectrometer scalar.
	 */
	class AliScalar : public TObject
	{
	public:
		/// Default constructor
		AliScalar() : TObject(), fValue(0), fName(), fDescription() {}
		
		/// Constructor to set some initial values.
		AliScalar(Double_t value, const char* name, const char* description) :
			TObject(), fValue(value), fName(name), fDescription(description)
		{}
		
		/// Default destructor
		virtual ~AliScalar() {}
		
		/// Inherited from TObject. Returns the name of the scalar.
		virtual const char* GetName() const { return fName; }
		
		/// Inherited from TObject. Returns the description of the scalar.
		virtual const char* GetTitle() const {return fDescription; }
		
		/// Inherited from TObject. Compares two scalar names.
		virtual Int_t  Compare(const TObject *obj) const
		{
			return fName.CompareTo(obj->GetName());
		}
		
		/// Inherited from TObject. Returns true.
		virtual Bool_t IsSortable() const { return kTRUE; }
		
		/**
		 * Inherited from TObject.
		 * Returns true if the names of the scalars are the same.
		 */
		virtual Bool_t IsEqual(const TObject *obj) const
		{
			return fName == obj->GetName();
		}
		
		/// Resets the scalar value.
		virtual void Clear(Option_t* /*option*/ = "") { fValue = 0; }
	
		/// Inherited form TObject. Performs a deep copy.
		virtual void Copy(TObject& object) const;
		
		/// Returns the value of the scalar.
		Double_t Value() const { return fValue; }
		
		/**
		 * Sets a new value for the scalar.
		 * \param value  The new value to use. If negative then 0 is used instead.
		 */
		void Value(Double_t value) { fValue = value >= 0 ? value : 0; }
		
		/// Increments the scalar value by 'count'.
		void Increment(UInt_t count = 1) { fValue += count; }
		
		/// Returns the name of the scalar.
		const char* Name() const { return fName.Data(); }
		
		/// Returns the description string for the scalar.
		const char* Description() const { return fDescription.Data(); }
		
		/// Checks if two scalar objects are equal.
		bool operator == (const AliScalar& x) const
		{
			return fValue == x.fValue and fName == x.fName
			       and fDescription == x.fDescription;
		}
		
		/// Checks if two scalar objects are not equal.
		bool operator != (const AliScalar& x) const
		{
			return not (this->operator == (x));
		}
		
	private:
		Double_t fValue; /// The scalar value.
		TString fName;  /// The name of the scalar.
		TString fDescription;  /// A description of what the scalar represents.
		
		ClassDef(AliScalar, 1);  // A scalar value for the muon spectrometer.
	};
	
	/// Default constructor.
	AliHLTMuonSpectroScalars();
	
	/// The copy constructor performs a deep copy.
	AliHLTMuonSpectroScalars(const AliHLTMuonSpectroScalars& obj);
	
	/// Default destructor.
	virtual ~AliHLTMuonSpectroScalars();
	
	/**
	 * Adds a new scalar to the end of the scalars list.
	 * \param name  The name of the scalar value.
	 * \param description  A short description of the scalar value.
	 * \param value  The value of the new scalar.
	 */
	void Add(const char* name, const char* description, Double_t value = 0);
	
	/// Returns the number of scalar values.
	UInt_t NumberOfScalars() const { return UInt_t(fScalars.GetEntriesFast()); }
	
	/// Checks to see if the named scalar exists.
	bool Exists(const char* name) const;
	
	/**
	 * Fetches the n'th scalar object.
	 * \param n  The scalar object to fetch.
	 * \returns the n'th scalar object, otherwise NULL if <i>n</i> was out of range.
	 */
	AliScalar* GetScalarN(UInt_t n);
	const AliScalar* GetScalarN(UInt_t n) const;
	
	/**
	 * Fetches the named scalar object. Will apply a binary search for the object.
	 * This means rebuilding the internal index if necessary.
	 * \param name  The name of the scalar object.
	 * \returns the found scalar object, otherwise NULL if it was not found.
	 */
	AliScalar* GetScalar(const char* name);
	const AliScalar* GetScalar(const char* name) const;
	
	/**
	 * Fetches the n'th scalar value.
	 * \param n  The scalar number to fetch.
	 * \returns the n'th scalar, otherwise 0 if <i>n</i> was out of range.
	 */
	Double_t GetN(UInt_t n) const;
	
	/**
	 * Fetches the a scalar value by name.
	 * \param name  The name of the scalar.
	 * \returns the scalar's value, otherwise 0 if it could not be found.
	 */
	Double_t Get(const char* name) const;
	
	/**
	 * Sets the n'th scalar value.
	 * \param n  The scalar number to set.
	 * \param value  The new value for the scalar.
	 * \returns  true if the scalar could be set, otherwise false which means
	 *    that <i>n</i> was out of range.
	 */
	bool SetN(UInt_t n, Double_t value);
	
	/**
	 * Sets the named scalar's value.
	 * \param name  The name of the scalar to set.
	 * \param value  The new value for the scalar.
	 * \returns  true if the scalar could be set, otherwise false which means
	 *    that it was not found.
	 */
	bool Set(const char* name, Double_t value);
	
	/**
	 * Increments the n'th scalar by a value of 'count'.
	 * \param n  The scalar number to increment.
	 * \param count  The number to increment the scalar by. The default is 1.
	 * \returns true if the scalar could be incremented, otherwise false
	 *    which means that <i>n</i> was out of range.
	 */
	bool IncrementN(UInt_t n, UInt_t count = 1);
	
	/**
	 * Increments the n'th scalar by a value of 'count'.
	 * \param name  The name of the scalar to set.
	 * \param count  The number to increment the scalar by. The default is 1.
	 * \returns true if the scalar could be incremented, otherwise false
	 *    which means that it could not be found.
	 */
	bool Increment(const char* name, UInt_t count = 1);
	
	/**
	 * Returns the name of the n'th scalar.
	 * \param n  The scalar number for which to return the name.
	 * \returns the n'th scalar's name, otherwise NULL if <i>n</i> was out of range.
	 */
	const char* Name(UInt_t n) const;
	
	/**
	 * Returns the description string for the n'th scalar.
	 * \param n  The scalar number for which to return the description.
	 * \returns the n'th scalar's description, otherwise NULL if <i>n</i> was out of range.
	 */
	const char* Description(UInt_t n) const;
	
	/// Resets all scalar values to zero.
	void Reset();
	
	/**
	 * Removes all the scalars from the internal array.
	 * \param option  This is passed onto the internal Delete method.
	 */
	virtual void Clear(Option_t* option = "");
	
	/// Inherited form TObject. Performs a deep copy.
	virtual void Copy(TObject& object) const;
	
	/// Finds the scalar object by name. Will not apply a binary search if fIndex
	/// was invalidated.
	virtual TObject* FindObject(const char* name) const;
	
	/// Finds the scalar object with the same name as obj->GetName().
	/// Will not apply a binary search if fIndex was invalidated.
	virtual TObject* FindObject(const TObject* obj) const;
	
	/**
	 * Inherited from TObject, this prints the contents of this summary object.
	 * \param option  Can be "compact", which will just print all the values on one line.
	 */
	virtual void Print(Option_t* option = "") const;
	
	/**
	 * The assignment operator performs a deep copy.
	 */
	AliHLTMuonSpectroScalars& operator = (const AliHLTMuonSpectroScalars& obj);
	
	/// Returns the n'th scalar value or -1 if n is out of range.
	Double_t operator [] (UInt_t n) const { return GetN(n); }
	
	/// Returns the named scalar value or -1 if not found.
	Double_t operator [] (const TString& name) const { return Get(name.Data()); }
	
	/**
	 * Comparison operator to check if two scalar objects have the same values.
	 * \note The description strings are not checked so they could be different
	 *   and the order of the scalars does not matter either.
	 */
	bool operator == (const AliHLTMuonSpectroScalars& obj) const;
	
	/**
	 * Comparison operator to check if two scalar objects are different.
	 * \note The description strings are not checked, only the values
	 *   and the order of the scalars does not matter either.
	 */
	bool operator != (const AliHLTMuonSpectroScalars& obj) const
	{
		return not (this->operator == (obj));
	}
	
private:
	
	/// Creates the index required for faster searches of the fScalars array.
	void MakeIndex() const;
	
	TClonesArray fScalars;  /// The list of scalars. Contains AliScalar objects.
	mutable TObjArray fIndex;   //! A sorted index of the scalars for faster searches.
	mutable bool fIndexValid;   //! Indicates that the index is valid.
	
	ClassDef(AliHLTMuonSpectroScalars, 1);  // HLT trigger scalars for the muon spectrometer.
};

#endif // AliHLTMUONSPECTROSCALARS_H
