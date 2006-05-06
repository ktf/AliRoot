#ifndef ALIHLTMUONERROR_H
#define ALIHLTMUONERROR_H
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

////////////////////////////////////////////////////////////////////////////////
//
// Author: Artur Szostak
// Email:  artur@alice.phy.uct.ac.za | artursz@iafrica.com
//
// AliHLTMUONError is the base excpetion class used by the dHLT subsystem.
// All child classes used to throw exceptions should be derived from this
// class to allow easy catching of classes of errors.
// 
////////////////////////////////////////////////////////////////////////////////

#include "AliHLTMUONBasicTypes.h"
#include <exception>
#include <ostream>


class AliHLTMUONError : public std::exception
{
	/* Define the << operator for streams to be able to do something like:

               AliHLTMUONError myerror;
               cout << myerror << endl;
	*/
	friend std::ostream& operator << (std::ostream& os, const AliHLTMUONError& error)
	{
		os << error.Message();
		return os;
	};
	
public:

	AliHLTMUONError() throw() {};
	virtual ~AliHLTMUONError() throw() {};

	/* Should return a human readable string containing a description of the
	   error.
	 */
	virtual const char* Message() const throw() = 0;
	
	/* Returns an error code describing the error. The error code should be
	   unique to the entire system
	 */
	virtual Int ErrorCode() const throw() = 0;
	
	virtual const char* what() const throw()
	{
		return Message();
	};
};


#endif // ALIHLTMUONERROR_H
