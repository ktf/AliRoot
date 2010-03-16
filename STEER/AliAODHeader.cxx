/**************************************************************************
 * Copyright(c) 1998-2007, ALICE Experiment at CERN, All rights reserved. *
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

//-------------------------------------------------------------------------
//     AOD event base class
//     Author: Markus Oldenburg, CERN
//-------------------------------------------------------------------------

#include "AliAODHeader.h"
#include <TGeoMatrix.h>
#include <TObjString.h>

ClassImp(AliAODHeader)

//______________________________________________________________________________
AliAODHeader::AliAODHeader() : 
  AliVHeader(),
  fMagneticField(-999.),
  fMuonMagFieldScale(-999.),
  fCentrality(-999.),
  fZDCN1Energy(-999.),
  fZDCP1Energy(-999.),
  fZDCN2Energy(-999.),
  fZDCP2Energy(-999.),
  fNQTheta(0),
  fQTheta(0x0),
  fTriggerMask(0),
  fFiredTriggers(),
  fRunNumber(-999),  
  fRefMult(-999),
  fRefMultPos(-999),
  fRefMultNeg(-999),
  fNMuons(0),
  fNDimuons(0),
  fEventType(0),
  fOrbitNumber(0),
  fPeriodNumber(0),
  fBunchCrossNumber(0),
  fTriggerCluster(0)
{
  // default constructor

  SetName("header");
  for(int j=0; j<2; j++) fZDCEMEnergy[j] = -999.;
  for(Int_t i=0; i<2; i++) fDiamondXY[i]=0.;
  fDiamondCovXY[0]=fDiamondCovXY[2]=3.*3.;
  fDiamondCovXY[1]=0.;
  for (Int_t m=0; m<kNPHOSMatrix; m++) fPHOSMatrix[m]=NULL;
  for (Int_t sm=0; sm<kNEMCALMatrix; sm++) fEMCALMatrix[sm]=NULL;
}

//______________________________________________________________________________
AliAODHeader::AliAODHeader(Int_t nRun, 
			   UShort_t nBunchX,
			   UInt_t nOrbit,
			   UInt_t nPeriod,
			   const Char_t *title) :
  AliVHeader(),
  fMagneticField(-999.),
  fMuonMagFieldScale(-999.),
  fCentrality(-999.),
  fZDCN1Energy(-999.),
  fZDCP1Energy(-999.),
  fZDCN2Energy(-999.),
  fZDCP2Energy(-999.),
  fNQTheta(0),
  fQTheta(0x0),
  fTriggerMask(0),
  fFiredTriggers(),
  fRunNumber(nRun),
  fRefMult(-999),
  fRefMultPos(-999),
  fRefMultNeg(-999),
  fNMuons(0),
  fNDimuons(0),
  fEventType(0),
  fOrbitNumber(nOrbit),
  fPeriodNumber(nPeriod),
  fBunchCrossNumber(nBunchX),
  fTriggerCluster(0)
{
  // constructor

  SetName("header");
  SetTitle(title);
  for(int j=0; j<2; j++) fZDCEMEnergy[j] = -999.;
  for(Int_t i=0; i<2; i++) fDiamondXY[i]=0.;
  fDiamondCovXY[0]=fDiamondCovXY[2]=3.*3.;
  fDiamondCovXY[1]=0.;
  for (Int_t m=0; m<kNPHOSMatrix; m++) fPHOSMatrix[m]=NULL;
  for (Int_t sm=0; sm<kNEMCALMatrix; sm++) fEMCALMatrix[sm]=NULL;
}

//______________________________________________________________________________
AliAODHeader::AliAODHeader(Int_t nRun, 
			   UShort_t nBunchX,
			   UInt_t nOrbit,
			   UInt_t nPeriod,
			   Int_t refMult,
			   Int_t refMultPos,
			   Int_t refMultNeg,
			   Double_t magField,
			   Double_t muonMagFieldScale,
			   Double_t cent,
			   Double_t n1Energy,
			   Double_t p1Energy,
			   Double_t n2Energy,
			   Double_t p2Energy,
			   Double_t *emEnergy,
 			   ULong64_t trigMask,
			   UChar_t trigClus,
			   UInt_t evttype,
			   const Char_t *title,
			   Int_t nMuons,
			   Int_t nDimuons) :
  AliVHeader(),
  fMagneticField(magField),
  fMuonMagFieldScale(muonMagFieldScale),
  fCentrality(cent),
  fZDCN1Energy(n1Energy),
  fZDCP1Energy(p1Energy),
  fZDCN2Energy(n2Energy),
  fZDCP2Energy(p2Energy),
  fNQTheta(0),
  fQTheta(0x0),
  fTriggerMask(trigMask),
  fFiredTriggers(),
  fRunNumber(nRun),  
  fRefMult(refMult),
  fRefMultPos(refMultPos),
  fRefMultNeg(refMultNeg),
  fNMuons(nMuons),
  fNDimuons(nDimuons),
  fEventType(evttype),
  fOrbitNumber(nOrbit),
  fPeriodNumber(nPeriod),
  fBunchCrossNumber(nBunchX),
  fTriggerCluster(trigClus)

{
  // constructor

  SetName("header");
  SetTitle(title);
  for(int j=0; j<2; j++) fZDCEMEnergy[j] = emEnergy[j];
  for(Int_t i=0; i<2; i++) fDiamondXY[i]=0.;
  fDiamondCovXY[0]=fDiamondCovXY[2]=3.*3.;
  fDiamondCovXY[1]=0.;
  for (Int_t m=0; m<kNPHOSMatrix; m++) fPHOSMatrix[m]=NULL;
  for (Int_t sm=0; sm<kNEMCALMatrix; sm++) fEMCALMatrix[sm]=NULL;
}

//______________________________________________________________________________
AliAODHeader::~AliAODHeader() 
{
  // destructor
  
  RemoveQTheta();
}

//______________________________________________________________________________
AliAODHeader::AliAODHeader(const AliAODHeader& hdr) :
  AliVHeader(hdr),
  fMagneticField(hdr.fMagneticField),
  fMuonMagFieldScale(hdr.fMuonMagFieldScale),
  fCentrality(hdr.fCentrality),
  fZDCN1Energy(hdr.fZDCN1Energy),
  fZDCP1Energy(hdr.fZDCP1Energy),
  fZDCN2Energy(hdr.fZDCN2Energy),
  fZDCP2Energy(hdr.fZDCP2Energy),
  fNQTheta(0),
  fQTheta(0x0),
  fTriggerMask(hdr.fTriggerMask),
  fFiredTriggers(hdr.fFiredTriggers),
  fRunNumber(hdr.fRunNumber),  
  fRefMult(hdr.fRefMult), 
  fRefMultPos(hdr.fRefMultPos), 
  fRefMultNeg(hdr.fRefMultNeg),
  fNMuons(hdr.fNMuons),
  fNDimuons(hdr.fNDimuons),
  fEventType(hdr.fEventType),
  fOrbitNumber(hdr.fOrbitNumber),
  fPeriodNumber(hdr.fPeriodNumber),
  fBunchCrossNumber(hdr.fBunchCrossNumber),
  fTriggerCluster(hdr.fTriggerCluster)

{
  // Copy constructor.
  
  SetName(hdr.fName);
  SetTitle(hdr.fTitle);
  SetQTheta(hdr.fQTheta, hdr.fNQTheta);
  SetZDCEMEnergy(hdr.fZDCEMEnergy[0], hdr.fZDCEMEnergy[1]);
  for(Int_t i=0; i<2; i++) fDiamondXY[i]=hdr.fDiamondXY[i];
  for(Int_t i=0; i<3; i++) fDiamondCovXY[i]=hdr.fDiamondCovXY[i];


  for(Int_t m=0; m<kNPHOSMatrix; m++){
      if(hdr.fPHOSMatrix[m])
	  fPHOSMatrix[m]=new TGeoHMatrix(*(hdr.fPHOSMatrix[m])) ;
      else
	  fPHOSMatrix[m]=0;
  }
  
  for(Int_t sm=0; sm<kNEMCALMatrix; sm++){
      if(hdr.fEMCALMatrix[sm])
	  fEMCALMatrix[sm]=new TGeoHMatrix(*(hdr.fEMCALMatrix[sm])) ;
      else
	  fEMCALMatrix[sm]=0;
  }

}

//______________________________________________________________________________
AliAODHeader& AliAODHeader::operator=(const AliAODHeader& hdr)
{
  // Assignment operator
  if(this!=&hdr) {
    
     AliVHeader::operator=(hdr);
    
    fMagneticField    = hdr.fMagneticField;
    fMuonMagFieldScale= hdr.fMuonMagFieldScale;
    fCentrality       = hdr.fCentrality;
    fZDCN1Energy      = hdr.fZDCN1Energy;
    fZDCP1Energy      = hdr.fZDCP1Energy;
    fZDCN2Energy      = hdr.fZDCN2Energy;
    fZDCP2Energy      = hdr.fZDCP2Energy;
    fTriggerMask      = hdr.fTriggerMask;
    fFiredTriggers    = hdr.fFiredTriggers;
    fRunNumber        = hdr.fRunNumber;
    fRefMult          = hdr.fRefMult;
    fRefMultPos       = hdr.fRefMultPos;
    fRefMultNeg       = hdr.fRefMultNeg;
    fEventType        = hdr.fEventType;
    fOrbitNumber      = hdr.fOrbitNumber;
    fPeriodNumber     = hdr.fPeriodNumber;
    fBunchCrossNumber = hdr.fBunchCrossNumber;
    fTriggerCluster   = hdr.fTriggerCluster;
    fNMuons           = hdr.fNMuons;
    fNDimuons         = hdr.fNDimuons;


    SetName(hdr.fName);
    SetTitle(hdr.fTitle);
    SetQTheta(hdr.fQTheta, hdr.fNQTheta);
    SetZDCEMEnergy(hdr.fZDCEMEnergy[0], hdr.fZDCEMEnergy[1]);
    for(Int_t i=0; i<2; i++) fDiamondXY[i]=hdr.fDiamondXY[i];
    for(Int_t i=0; i<3; i++) fDiamondCovXY[i]=hdr.fDiamondCovXY[i];

    for(Int_t m=0; m<kNPHOSMatrix; m++){
	if(hdr.fPHOSMatrix[m])
	    fPHOSMatrix[m]=new TGeoHMatrix(*(hdr.fPHOSMatrix[m])) ;
	else
	    fPHOSMatrix[m]=0;
    }
    
    for(Int_t sm=0; sm<kNEMCALMatrix; sm++){
	if(hdr.fEMCALMatrix[sm])
	    fEMCALMatrix[sm]=new TGeoHMatrix(*(hdr.fEMCALMatrix[sm])) ;
	else
	    fEMCALMatrix[sm]=0;
  }

  }


  return *this;
}

//______________________________________________________________________________
void AliAODHeader::SetQTheta(Double_t *QTheta, UInt_t size) 
{
  if (QTheta && size>0) {
    if (size != (UInt_t)fNQTheta) {
      RemoveQTheta();
      fNQTheta = size;
      fQTheta = new Double_t[fNQTheta];
    }
    
    for (Int_t i = 0; i < fNQTheta; i++) {
      fQTheta[i] = QTheta[i];
    }
  } else {
    RemoveQTheta();
  }

  return;
}

//______________________________________________________________________________
Double_t AliAODHeader::GetQTheta(UInt_t i) const
{
  if (fQTheta && i < (UInt_t)fNQTheta) {
    return fQTheta[i];
  } else {
    return -999.;
  }
}

//______________________________________________________________________________
void AliAODHeader::RemoveQTheta()
{
  delete[] fQTheta;
  fQTheta = 0x0;
  fNQTheta = 0;

  return;
}

//______________________________________________________________________________
void AliAODHeader::Print(Option_t* /*option*/) const 
{
  // prints event information

  printf("Run #                   : %d\n", fRunNumber);
  printf("Bunch Crossing  #       : %d\n", fBunchCrossNumber);
  printf("Orbit Number #          : %d\n", fOrbitNumber);
  printf("Period Number #         : %d\n", fPeriodNumber);
  printf("Trigger mask            : %lld\n", fTriggerMask);
  printf("Trigger cluster         : %d\n", fTriggerCluster);
  printf("Event Type              : %d\n", fEventType);
  printf("Magnetic field          : %f\n", fMagneticField);
  printf("Muon mag. field scale   : %f\n", fMuonMagFieldScale);
  
  printf("Centrality              : %f\n", fCentrality);
  printf("ZDC N1 Energy           : %f\n", fZDCN1Energy);
  printf("ZDC P1 Energy           : %f\n", fZDCP1Energy);
  printf("ZDC N2 Energy           : %f\n", fZDCN2Energy);
  printf("ZDC P2 Energy           : %f\n", fZDCP2Energy);
  printf("ZDC EM1 Energy          : %f\n", fZDCEMEnergy[0]);
  printf("ZDC EM2 Energy          : %f\n", fZDCEMEnergy[1]);
  printf("ref. Multiplicity       : %d\n", fRefMult);
  printf("ref. Multiplicity (pos) : %d\n", fRefMultPos);
  printf("ref. Multiplicity (neg) : %d\n", fRefMultNeg);
  printf("number of muons         : %d\n", fNMuons);
  printf("number of dimuons       : %d\n", fNDimuons);

  if (fQTheta) {
    for (UInt_t i = 0; i<(UInt_t)fNQTheta; i++) {
      printf("QTheta[%d]              : %13.3e\n", i, GetQTheta(i));
    }
  }

  return;
}
