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

///////////////////////////////////////////////////////////////////////////////
//
// This is a class for reading a raw data from a date event and providing
// information about digits
//
///////////////////////////////////////////////////////////////////////////////

#include "AliRawReaderDate.h"
#ifdef ALI_DATE
#include "event.h"
#endif

ClassImp(AliRawReaderDate)


AliRawReaderDate::AliRawReaderDate(
#ifdef ALI_DATE
				   void* event
#else
				   void* /* event */
#endif
)
{
// create an object to read digits from the given date event

#ifdef ALI_DATE
  fEvent = (eventHeaderStruct*) event;
  fSubEvent = NULL;
  fPosition = fEnd = NULL;
#else
  Fatal("AliRawReaderDate", "this class was compiled without DATE");
#endif
}

AliRawReaderDate::AliRawReaderDate(const AliRawReaderDate& rawReader) :
  AliRawReader(rawReader)
{
// copy constructor

  fEvent = rawReader.fEvent;
  fSubEvent = rawReader.fSubEvent;
  fPosition = rawReader.fPosition;
  fEnd = rawReader.fEnd;
}

AliRawReaderDate& AliRawReaderDate::operator = (const AliRawReaderDate& 
						rawReader)
{
// assignment operator

  this->~AliRawReaderDate();
  new(this) AliRawReaderDate(rawReader);
  return *this;
}


UInt_t AliRawReaderDate::GetType() const
{
// get the type from the event header

#ifdef ALI_DATE
  if (!fEvent) return 0;
  return fEvent->eventType;
#else
  return 0;
#endif
}

UInt_t AliRawReaderDate::GetRunNumber() const
{
// get the run number from the event header

#ifdef ALI_DATE
  if (!fEvent) return 0;
  return fEvent->eventRunNb;
#else
  return 0;
#endif
}

const UInt_t* AliRawReaderDate::GetEventId() const
{
// get the event id from the event header

#ifdef ALI_DATE
  if (!fEvent) return NULL;
  return fEvent->eventId;
#else
  return NULL;
#endif
}

const UInt_t* AliRawReaderDate::GetTriggerPattern() const
{
// get the trigger pattern from the event header

#ifdef ALI_DATE
  if (!fEvent) return NULL;
  return fEvent->eventTriggerPattern;
#else
  return NULL;
#endif
}

const UInt_t* AliRawReaderDate::GetDetectorPattern() const
{
// get the detector pattern from the event header

#ifdef ALI_DATE
  if (!fEvent) return NULL;
  return fEvent->eventDetectorPattern;
#else
  return NULL;
#endif
}

const UInt_t* AliRawReaderDate::GetAttributes() const
{
// get the type attributes from the event header

#ifdef ALI_DATE
  if (!fEvent) return NULL;
  return fEvent->eventTypeAttribute;
#else
  return NULL;
#endif
}

UInt_t AliRawReaderDate::GetGDCId() const
{
// get the GDC Id from the event header

#ifdef ALI_DATE
  if (!fEvent) return 0;
  return fEvent->eventGdcId;
#else
  return 0;
#endif
}


Bool_t AliRawReaderDate::ReadMiniHeader()
{
// read a mini header at the current position
// returns kFALSE if the mini header could not be read

  fErrorCode = 0;

#ifdef ALI_DATE
  if (!fEvent) return kFALSE;
  // check whether there are sub events
  if (fEvent->eventSize <= fEvent->eventHeadSize) return kFALSE;

  do {
    // skip payload (if event was not selected)
    if (fCount > 0) fPosition += fCount;

    // get the first or the next sub event if at the end of a sub event
    if (!fSubEvent || (fPosition >= fEnd)) {

      // check for end of event data
      if (fPosition >= ((UChar_t*)fEvent)+fEvent->eventSize) return kFALSE;
      if (fSubEvent) {
	fSubEvent = (eventHeaderStruct*) (((UChar_t*)fSubEvent) + 
					  fSubEvent->eventSize);
      } else {
	fSubEvent = (eventHeaderStruct*) (((UChar_t*)fEvent) + 
					  fEvent->eventHeadSize);
      }

      // check the magic word of the sub event
      if (fSubEvent->eventMagic != EVENT_MAGIC_NUMBER) {
	Error("ReadMiniHeader", "wrong magic number in sub event!\n"
	      " run: %d  event: %d %d  LDC: %d  GDC: %d\n", 
	      fSubEvent->eventRunNb, 
	      fSubEvent->eventId[0], fSubEvent->eventId[1],
	      fSubEvent->eventLdcId, fSubEvent->eventGdcId);
	fErrorCode = kErrMagic;
	return kFALSE;
      }

      fCount = 0;
      fPosition = ((UChar_t*)fSubEvent) + fSubEvent->eventHeadSize + 
	sizeof(equipmentHeaderStruct);
      fEnd = ((UChar_t*)fSubEvent) + fSubEvent->eventSize;
    }

    // continue with the next sub event if no data left in the payload
    if (fPosition >= fEnd) continue;

    // check that there are enough bytes left for the mini header
    if (fPosition + sizeof(AliMiniHeader) > fEnd) {
      Error("ReadMiniHeader", "could not read mini header data!");
      Warning("ReadMiniHeader", "skipping %d bytes\n"
	      " run: %d  event: %d %d  LDC: %d  GDC: %d\n", 
	      fEnd - fPosition, fSubEvent->eventRunNb, 
	      fSubEvent->eventId[0], fSubEvent->eventId[1],
	      fSubEvent->eventLdcId, fSubEvent->eventGdcId);
      fCount = 0;
      fPosition = fEnd;
      fErrorCode = kErrNoMiniHeader;
      continue;
    }

    // "read" and check the mini header
    fMiniHeader = (AliMiniHeader*) fPosition;
    fPosition += sizeof(AliMiniHeader);
    if (!CheckMiniHeader()) {
      Error("ReadMiniHeader", "wrong magic word in mini header!");
      Warning("ReadMiniHeader", "skipping %d bytes\n"
	      " run: %d  event: %d %d  LDC: %d  GDC: %d\n", 
	      fEnd - fPosition, fSubEvent->eventRunNb, 
	      fSubEvent->eventId[0], fSubEvent->eventId[1],
	      fSubEvent->eventLdcId, fSubEvent->eventGdcId);
      fCount = 0;
      fPosition = fEnd;
      fErrorCode = kErrMiniMagic;
      continue;
    }
    fCount = fMiniHeader->fSize;

    // check consistency of data size in the mini header and in the sub event
    if (fPosition + fCount > fEnd) {
      Error("ReadMiniHeader", "size in mini header exceeds event size!");
      Warning("ReadMiniHeader", "skipping %d bytes\n"
	      " run: %d  event: %d %d  LDC: %d  GDC: %d\n", 
	      fEnd - fPosition, fSubEvent->eventRunNb, 
	      fSubEvent->eventId[0], fSubEvent->eventId[1],
	      fSubEvent->eventLdcId, fSubEvent->eventGdcId);
      fCount = 0;
      fPosition = fEnd;
      fErrorCode = kErrSize;
      continue;
    }

  } while (!IsSelected());

  return kTRUE;
#else
  return kFALSE;
#endif
}

Bool_t AliRawReaderDate::ReadNextData(UChar_t*& data)
{
// reads the next payload at the current position
// returns kFALSE if the data could not be read

  fErrorCode = 0;
  while (fCount == 0) {
    if (!ReadMiniHeader()) return kFALSE;
  }
  data = fPosition;
  fPosition += fCount;  
  fCount = 0;
  return kTRUE;
}

Bool_t AliRawReaderDate::ReadNext(UChar_t* data, Int_t size)
{
// reads the next block of data at the current position
// returns kFALSE if the data could not be read

  fErrorCode = 0;
  if (fPosition + size > fEnd) {
    Error("ReadNext", "could not read data!");
    fErrorCode = kErrOutOfBounds;
    return kFALSE;
  }
  memcpy(data, fPosition, size);
  fPosition += size;
  fCount -= size;
  return kTRUE;
}


Bool_t AliRawReaderDate::Reset()
{
// reset the current position to the beginning of the event

#ifdef ALI_DATE
  fSubEvent = NULL;
#endif
  fCount = 0;
  fPosition = fEnd = NULL;
  return kTRUE;
}


Int_t AliRawReaderDate::CheckData() const
{
// check the consistency of the data

#ifdef ALI_DATE
  if (!fEvent) return 0;
  // check whether there are sub events
  if (fEvent->eventSize <= fEvent->eventHeadSize) return 0;

  eventHeaderStruct* subEvent = NULL;
  UChar_t* position = 0;
  UChar_t* end = 0;
  Int_t result = 0;

  while (kTRUE) {
    // get the first or the next sub event if at the end of a sub event
    if (!subEvent || (position >= end)) {

      // check for end of event data
      if (position >= ((UChar_t*)fEvent)+fEvent->eventSize) return result;
      if (subEvent) {
	subEvent = (eventHeaderStruct*) (((UChar_t*)subEvent) + 
					 subEvent->eventSize);
      } else {
	subEvent = (eventHeaderStruct*) (((UChar_t*)fEvent) + 
					 fEvent->eventHeadSize);
      }

      // check the magic word of the sub event
      if (subEvent->eventMagic != EVENT_MAGIC_NUMBER) {
	result |= kErrMagic;
	return result;
      }

      position = ((UChar_t*)subEvent) + subEvent->eventHeadSize + 
	sizeof(equipmentHeaderStruct);
      end = ((UChar_t*)subEvent) + subEvent->eventSize;
    }

    // continue with the next sub event if no data left in the payload
    if (position >= end) continue;

    // check that there are enough bytes left for the mini header
    if (position + sizeof(AliMiniHeader) > end) {
      result |= kErrNoMiniHeader;
      position = end;
      continue;
    }

    // "read" and check the mini header
    AliMiniHeader* miniHeader = (AliMiniHeader*) position;
    position += sizeof(AliMiniHeader);
    if (!CheckMiniHeader(miniHeader)){
      result |= kErrMiniMagic;
      position = end;
      continue;
    }

    // check consistency of data size in the mini header and in the sub event
    if (position + miniHeader->fSize > end) result |= kErrSize;
    position += miniHeader->fSize;
  };

#endif
  return 0;
}
