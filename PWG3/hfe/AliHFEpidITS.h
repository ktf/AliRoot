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
//
// PID development class for ITS
// does proton rejection via dE/dx
// For more information see implementation file
#ifndef ALIHFEPIDITS_H
#define ALIHFEPIDITS_H

#ifndef ALIHFEPIDBASE_H
#include "AliHFEpidBase.h"
#endif

class AliVParticle;
class TList;

class AliHFEpidITS : public AliHFEpidBase{
  public:
    AliHFEpidITS(const Char_t *name);
    AliHFEpidITS(const AliHFEpidITS &ref);
    AliHFEpidITS& operator=(const AliHFEpidITS &ref);
    virtual ~AliHFEpidITS();

    virtual Bool_t InitializePID();
    virtual Int_t IsSelected(AliHFEpidObject *track);
    virtual Bool_t HasQAhistos() const { return kTRUE; };

    Double_t GetITSSignalV1(AliVParticle *track, Int_t mcPID);
    Double_t GetITSSignalV2(AliVParticle *track, Int_t mcPID);
  protected:
    virtual void AddQAhistograms(TList *l);
    void Copy(TObject &o) const;
    void FillHistogramsSignalV1(Double_t p, Double_t signal, Int_t species);
    void FillHistogramsSignalV2(Double_t p, Double_t signal, Int_t species);
  private:
    enum{
      kITSsigV1 = 0,
      kITSsigV2 = 1,
      kHistosSigAll = 2
    };
    TList *fQAlist;      // QA histograms for ITS pid

    ClassDef(AliHFEpidITS, 0)  // PID class for ITS
};
#endif

