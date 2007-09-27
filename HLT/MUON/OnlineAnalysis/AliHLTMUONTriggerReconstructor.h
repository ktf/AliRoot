#ifndef AliHLTMUONTRIGGERRECONSTRUCTOR_H
#define AliHLTMUONTRIGGERRECONSTRUCTOR_H
/**************************************************************************
 * This file is property of and copyright by the ALICE HLT Project        * 
 * All rights reserved.                                                   *
 *                                                                        *
 * Primary Authors:                                                       *
 *   Indranil Das <indra.das@saha.ac.in>                                  *
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

/**********************************************************************
 Created on : 16/05/2007
 Purpose    : This class is supposed to read the trigger DDL files and 
              give the output AliHLTMUONTriggerRecordStruct
 Author     : Indranil Das, HEP Division, SINP
 Email      : indra.das@saha.ac.in | indra.ehep@gmail.com
**********************************************************************/

#include "AliHLTLogging.h"
#include "AliHLTMUONTriggerRecordsBlockStruct.h"
#include "AliHLTMUONHitReconstructor.h"


class AliHLTMUONTriggerReconstructor : public AliHLTLogging
{
public:

	struct RegToLoc
	{
		int fTrigDDL;  // trigger id (0 or 1)
		int fRegId,fLoc,fLocId;  // regeonal and local id.
		int fSwitch;  // packed switch word of 10 bits.
		int fDetElemId[4];  // Four detection element correspond to four detection element for each local card.
	};

	AliHLTMUONTriggerReconstructor();
	virtual ~AliHLTMUONTriggerReconstructor();

	bool LoadLookUpTable(
			AliHLTMUONHitReconstructor::DHLTLut* lookUpTableData,
			int lookUpTableId
		);

	bool SetRegToLocCardMap(RegToLoc* regToLoc);

	bool Run(
			const AliHLTUInt32_t* rawData,
			AliHLTUInt32_t rawDataSize,
			AliHLTMUONTriggerRecordStruct* trigRecord,
			AliHLTUInt32_t& nofTrigRec,
			bool suppressPartialTrigs = false
		);

	int GetLutLine() { return fgkLutLine; }

	static int GetkDetectorId() { return AliHLTMUONTriggerReconstructor::fgkDetectorId; }
	static int GetkDDLOffSet() { return AliHLTMUONTriggerReconstructor::fgkDDLOffSet; }
	static int GetkNofDDL() { return AliHLTMUONTriggerReconstructor::fgkNofDDL; }
	static int GetkDDLHeaderSize() { return AliHLTMUONTriggerReconstructor::fgkDDLHeaderSize; } 

protected:

	AliHLTMUONTriggerReconstructor(const AliHLTMUONTriggerReconstructor& rhs); // copy constructor
	AliHLTMUONTriggerReconstructor& operator=(const AliHLTMUONTriggerReconstructor& rhs); // assignment operator

private:

	static const int fgkDetectorId;            // Detector ID
	static const int fgkDDLOffSet;             // DDL Offset
	static const int fgkNofDDL;                // Number of DDL 
	static const int fgkDDLHeaderSize;         // DDL header size  

	static const int fgkEvenLutSize;           // Size of the LookupTable with event DDLID
	static const int fgkOddLutSize;            // Size of the LookupTable with odd DDLID
	static const int fgkLutLine;               // nof Line in LookupTable    

	static const int fgkMinIdUnique[2];   // Minimum value of idUnique in LookupTable, 2 corresponds to two types of DDL (even/odd)  
	static const int fgkMaxIdUnique[2];   // Maximum value of idUnique in LookupTable  
	static const float fgkHalfPadSizeXB[3];    // pad halflength for the pcb zones, 3 corresponds to 3 types of pad in bending side  
	static const float fgkHalfPadSizeYNB[2];   // pad halflength for the pcb zones, 2 corresponds to 2 types on nonbneding pad  

	static const int fgkDetElem;               // nof Detection element per DDL    

	AliHLTMUONHitReconstructor::DHLTLut* fLookUpTableData; // pointer to the array of Lookuptable data
	AliHLTUInt32_t fMaxRecPointsCount;            // max nof reconstructed hit  

	//104 correspond to maximum nuber of x indices a nonbending side of detelem may have (i.e. 1101) 
	//and 64 corresponds to the maximum number of y indices occurs in bending side of detelem (i.e 1100)
	int fGetIdTotalData[104][64][2];  // an array of idUnique with argumrnt of centralX,centralY and  planeType
	RegToLoc fRegToLocCard[128];      // 8 regional card per ddl and 16 slots per regional crate together made 16*8 = 128.

	int fDDLId;
	int fIdOffSet;

	AliHLTInt32_t fTrigRecId;  // A running counter for the trigger record ID.
};

#endif // AliHLTMUONTRIGGERRECONSTRUCTOR_H
