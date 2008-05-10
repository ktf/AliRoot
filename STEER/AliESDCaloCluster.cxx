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
/* $Log $ */

//-----------------------------------------------------------------
//           Implementation of the ESD Calorimeter cluster class
//   ESD = Event Summary Data
//   This is the class to deal with during the phisics analysis of data
//
//   J.L. Klay (LLNL)
//-----------------------------------------------------------------

#include <TLorentzVector.h>
#include "AliESDCaloCluster.h"

ClassImp(AliESDCaloCluster)

//_______________________________________________________________________
AliESDCaloCluster::AliESDCaloCluster() : 
  TObject(),
  fTracksMatched(0x0),
  fLabels(0x0),
  fNCells(0),
  fCellsAbsId(0x0),
  fCellsAmpFraction(0x0),
  fDigitAmplitude(0x0),//not in use
  fDigitTime(0x0),//not in use
  fDigitIndex(0x0),//not in use
  fEnergy(0),
  fDispersion(0),
  fChi2(0),
  fM20(0),
  fM02(0),
  fEmcCpvDistance(1024),
  fDistToBadChannel(1024),
  fID(0),
  fNExMax(0),
  fClusterType(kUndef), fTOF(0.)
{
  //
  // The default ESD constructor 
  //
  fGlobalPos[0] = fGlobalPos[1] = fGlobalPos[2] = 0.;
  for(Int_t i=0; i<AliPID::kSPECIESN; i++) fPID[i] = 0.;
}

//_______________________________________________________________________
AliESDCaloCluster::AliESDCaloCluster(const AliESDCaloCluster& clus) : 
  TObject(clus),
  fTracksMatched(clus.fTracksMatched?new TArrayI(*clus.fTracksMatched):0x0),
  fLabels(clus.fLabels?new TArrayI(*clus.fLabels):0x0),
  fNCells(clus.fNCells),
  fCellsAbsId(),
  fCellsAmpFraction(),
  fDigitAmplitude(clus.fDigitAmplitude?new TArrayS(*clus.fDigitAmplitude):0x0),//not in use
  fDigitTime(clus.fDigitTime?new TArrayS(*clus.fDigitTime):0x0),//not in use
  fDigitIndex(clus.fDigitIndex?new TArrayS(*clus.fDigitIndex):0x0),//not in use
  fEnergy(clus.fEnergy),
  fDispersion(clus.fDispersion),
  fChi2(clus.fChi2),
  fM20(clus.fM20),
  fM02(clus.fM02),
  fEmcCpvDistance(clus.fEmcCpvDistance),
  fDistToBadChannel(clus.fDistToBadChannel),
  fID(clus.fID),
  fNExMax(clus.fNExMax),
  fClusterType(clus.fClusterType),
  fTOF(clus.fTOF)
{
  //
  // The copy constructor 
  //
  fGlobalPos[0] = clus.fGlobalPos[0];
  fGlobalPos[1] = clus.fGlobalPos[1];
  fGlobalPos[2] = clus.fGlobalPos[2];

  for(Int_t i=0; i<AliPID::kSPECIESN; i++) fPID[i] = clus.fPID[i];

  if (clus.fNCells > 0) {

    if(clus.fCellsAbsId){
      fCellsAbsId = new UShort_t[clus.fNCells];
      for (Int_t i=0; i<clus.fNCells; i++)
	fCellsAbsId[i]=clus.fCellsAbsId[i];
    }
    
    if(clus.fCellsAmpFraction){
      fCellsAmpFraction = new Double32_t[clus.fNCells];
      for (Int_t i=0; i<clus.fNCells; i++)
	fCellsAmpFraction[i]=clus.fCellsAmpFraction[i];
    }
    
  }

}

//_______________________________________________________________________
AliESDCaloCluster &AliESDCaloCluster::operator=(const AliESDCaloCluster& source)
{
  // assignment operator

  if(&source == this) return *this;
  TObject::operator=(source);

  fGlobalPos[0] = source.fGlobalPos[0];
  fGlobalPos[1] = source.fGlobalPos[1];
  fGlobalPos[2] = source.fGlobalPos[2];


  fEnergy = source.fEnergy;
  fDispersion = source.fDispersion;
  fChi2 = source.fChi2;
  fM20 = source.fM20;
  fM02 = source.fM02;
  fEmcCpvDistance = source.fEmcCpvDistance;
  fDistToBadChannel = source.fDistToBadChannel ;
  for(Int_t i=0; i<AliPID::kSPECIESN; i++) fPID[i] = source.fPID[i];
  fID = source.fID;

  fNCells= source.fNCells;

  if (source.fNCells > 0) {
    if(source.fCellsAbsId){
      if(fNCells != source.fNCells){
	delete [] fCellsAbsId;
	fCellsAbsId = new UShort_t[source.fNCells];
      }
      for (Int_t i=0; i<source.fNCells; i++)
	fCellsAbsId[i]=source.fCellsAbsId[i];
    }
    
    if(source.fCellsAmpFraction){
      if(fNCells != source.fNCells){
	delete [] fCellsAmpFraction;
	fCellsAmpFraction = new Double32_t[source.fNCells];
      }
      for (Int_t i=0; i<source.fNCells; i++)
	fCellsAmpFraction[i]=source.fCellsAmpFraction[i];
    }  
  }

  fNExMax = source.fNExMax;
  fClusterType = source.fClusterType;
  fTOF = source.fTOF;

  //not in use
  if(source.fTracksMatched){
    // assign or copy construct
    if(fTracksMatched) *fTracksMatched = *source.fTracksMatched;
    else fTracksMatched = new TArrayI(*source.fTracksMatched);
  }
  else{
    delete fTracksMatched;
    fTracksMatched = 0;
  }

  if(source.fLabels){
    // assign or copy construct
    if(fLabels) *fLabels = *source.fLabels;
    else fLabels = new TArrayI(*source.fLabels);
  }
  else{
    delete fLabels;
    fLabels = 0;
  }


  if(source.fDigitAmplitude){
    // assign or copy construct
    if(fDigitAmplitude) *fDigitAmplitude = *source.fDigitAmplitude;
    else fDigitAmplitude = new TArrayS(*source.fDigitAmplitude);
  }
  else{
    delete fDigitAmplitude;
    fDigitAmplitude = 0;
  }



  if(source.fDigitTime){
    // assign or copy construct
    if(fDigitTime) *fDigitTime = *source.fDigitTime;
    else fDigitTime = new TArrayS(*source.fDigitTime);
  }
  else{
    delete fDigitTime;
    fDigitTime = 0;
  }



  if(source.fDigitIndex){
    // assign or copy construct
    if(fDigitIndex) *fDigitIndex = *source.fDigitIndex;
    else fDigitIndex = new TArrayS(*source.fDigitIndex);
  }
  else{
    delete fDigitIndex;
    fDigitIndex = 0;
  }
  
  return *this;

}

void AliESDCaloCluster::Copy(TObject &obj) const {
  
  // this overwrites the virtual TOBject::Copy()
  // to allow run time copying without casting
  // in AliESDEvent

  if(this==&obj)return;
  AliESDCaloCluster *robj = dynamic_cast<AliESDCaloCluster*>(&obj);
  if(!robj)return; // not an AliESDCluster
  *robj = *this;

}

//_______________________________________________________________________
AliESDCaloCluster::~AliESDCaloCluster(){ 
  //
  // This is destructor according Coding Conventions 
  //
  delete fTracksMatched;
  delete fLabels;
  delete fDigitAmplitude;  //not in use
  delete fDigitTime;  //not in use
  delete fDigitIndex;  //not in use
  if(fCellsAmpFraction) delete[] fCellsAmpFraction; fCellsAmpFraction=0;
  if(fCellsAbsId) delete[] fCellsAbsId;  fCellsAbsId = 0;
}

//_______________________________________________________________________
void AliESDCaloCluster::SetPid(const Float_t *p) {
  // Sets the probability of each particle type
  // Copied from AliESDtrack SetPIDValues
  // This function copies "n" PID weights from "scr" to "dest"
  // and normalizes their sum to 1 thus producing conditional
  // probabilities.
  // The negative weights are set to 0.
  // In case all the weights are non-positive they are replaced by
  // uniform probabilities

  Int_t n = AliPID::kSPECIESN;

  Float_t uniform = 1./(Float_t)n;

  Float_t sum = 0;
  for (Int_t i=0; i<n; i++)
    if (p[i]>=0) {
      sum+=p[i];
      fPID[i] = p[i];
    }
    else {
      fPID[i] = 0;
    }

  if(sum>0)
    for (Int_t i=0; i<n; i++) fPID[i] /= sum;
  else
    for (Int_t i=0; i<n; i++) fPID[i] = uniform;

}

//_______________________________________________________________________
void AliESDCaloCluster::GetMomentum(TLorentzVector& p, Double_t *vertex ) {
  // Returns TLorentzVector with momentum of the cluster. Only valid for clusters 
  // identified as photons or pi0 (overlapped gamma) produced on the vertex
  //Vertex can be recovered with esd pointer doing:  
  //" Double_t vertex[3] ; esd->GetVertex()->GetXYZ(vertex) ; "

  if(vertex){//calculate direction from vertex
    fGlobalPos[0]-=vertex[0];
    fGlobalPos[1]-=vertex[1];
    fGlobalPos[2]-=vertex[2];
  }
  
  Double_t r = TMath::Sqrt(fGlobalPos[0]*fGlobalPos[0]+
		            fGlobalPos[1]*fGlobalPos[1]+
		            fGlobalPos[2]*fGlobalPos[2]   ) ; 

  p.SetPxPyPzE( fEnergy*fGlobalPos[0]/r,  fEnergy*fGlobalPos[1]/r,  fEnergy*fGlobalPos[2]/r,  fEnergy) ; 
  
}
