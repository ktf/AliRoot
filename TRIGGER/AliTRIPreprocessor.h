#ifndef ALI_TRI_PREPROCESSOR_H
#define ALI_TRI_PREPROCESSOR_H

class TMap;
class AliShuttleInterface;

#include "AliPreprocessor.h"

// Preprocessor for triggering detectors
// Every Triggering detector should implement his own ProcessDETTriggerData 
// function which will be called according to the TriggerDetectorMask 
// found in the DAQ logbook_trigger_clusters table

class AliTRIPreprocessor : public AliPreprocessor
{
  public:
    enum { kNDetectorsMap = 31 }; // number of entries in detectors_map as in /date/db/detCodes.h. Adding empty strings when there's an "empty" index

    AliTRIPreprocessor(AliShuttleInterface* shuttle);
    virtual ~AliTRIPreprocessor();
    
    Short_t ProcessSPDTriggerData();
    Short_t ProcessTOFTriggerData();

  protected:
    virtual void Initialize(Int_t run, UInt_t startTime, UInt_t endTime);
    virtual UInt_t Process(TMap* /*dcsAliasMap*/);
    virtual Bool_t ProcessDCS();

  private:

    static const char* fgkDetectorsMapName[kNDetectorsMap];  // names of detectors/systems in the DETECTORS_MAP in /date/db/detCodes.h

    AliShuttleInterface *fShuttle;

    ClassDef(AliTRIPreprocessor, 0);
};

#endif
