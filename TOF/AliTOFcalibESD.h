#ifndef ALITOFCALIBESD_H
#define ALITOFCALIBESD_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//////////////////////////////////////////////////////////////////
//  class for TOF calibration:: simulation of uncalibrated data //
//////////////////////////////////////////////////////////////////

#include "AliESDtrack.h"

class AliPID;

class AliTOFcalibESD:public AliESDtrack{
public:
  AliTOFcalibESD();
  AliTOFcalibESD(const AliTOFcalibESD& UnCalib);
  ~AliTOFcalibESD();
  Float_t GetToT() const{return fToT;}         //Time Over Threshold
  Float_t GetTOFsignal() const{return fTOFtime;}
  Float_t GetTOFsignalND() const{return fTOFsignalND;}
  Float_t GetIntegratedLength() const{return fIntLen;}
  void GetExternalCovariance(Double_t cov[15]) const;
  void GetIntegratedTimes(Double_t exp[AliPID::kSPECIES]) const;
  Int_t GetCombID()const{return fCombID;}
  Float_t GetP()const{return fMo;}
  Int_t GetTOFCalChannel() const {return fTOFCalCh;}
  void SetToT(Float_t ToT) {fToT=ToT;}
  void SetTOFtime(Float_t TOFtime) {fTOFtime=TOFtime;}
  void SetTOFsignalND(Float_t TOFtimeND) {fTOFsignalND=TOFtimeND;}
  void SetP(Double_t p) {fMo=p;}
  void SetIntegratedTime(const Double_t *tracktime);
  void SetCombID(Int_t ID){fCombID = ID;} // 0->pi, 1->K, 2->p
  void SetTOFCalChannel(Int_t index){fTOFCalChannel=index;}
  void CopyFromAliESD(const AliESDtrack* track);
  Bool_t IsSortable() const {return kTRUE;}
  Int_t Compare(const TObject *uncobj) const;
private:
  Int_t    fCombID; //PID for calibration, this track
  Int_t    fTOFCalCh; //TOF cal Channel
  Float_t  fToT; //ToT signal
  Float_t  fIntLen; //track int. length
  Float_t  fTOFtime;//Time signal
  Double_t fMo;//momentum
  Float_t  fTOFsignalND; //non-decalibrated time signal
  Double_t fTrTime[AliPID::kSPECIES]; // TOFs estimated by the tracking
  Double_t fExtCov[15];// external covariance matrix of the track

  ClassDef(AliTOFcalibESD,1);
};
#endif // AliTOFcalibESD_H
