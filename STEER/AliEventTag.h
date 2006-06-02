#ifndef ALIEVENTTAG_H
#define ALIEVENTTAG_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */


/* $Id$ */

//-------------------------------------------------------------------------
//                          Class AliEventTag
//   This is the class to deal with the tags for the event level
//
//    Origin: Panos Christakoglou, UOA-CERN, Panos.Christakoglou@cern.ch
//-------------------------------------------------------------------------

#include "TObject.h"

//______________________________________________________________________________
class AliEventTag : public TObject
{
 public:
  AliEventTag();
  AliEventTag(const AliEventTag & t);
  virtual ~AliEventTag();

  AliEventTag &operator=(const AliEventTag &rhs);
  
  
  void   SetEventId(Int_t Pid) {fAliceEventId = Pid;}
  void   SetGUID(const char *Pid) {fGUID = Pid;}

  void   SetMD5(const char *Pid) {fmd5 = Pid;}
  void   SetTURL(const char *Pid) {fturl = Pid;}
  void   SetSize(Long64_t i) {fsize = i;}

  void   SetNumOfParticipants(Int_t P) {fNumberOfParticipants = P;}
  void   SetImpactParameter(Float_t Pimpact) {fImpactParameter = Pimpact;}

  void   SetVertexX(Float_t Pvx) {fPrimaryVertexX = Pvx;}
  void   SetVertexY(Float_t Pvy) {fPrimaryVertexY = Pvy;}
  void   SetVertexZ(Float_t Pvz) {fPrimaryVertexZ = Pvz;}

  void   SetVertexFlag(Int_t i) {fPrimaryVertexFlag = i;}
  void   SetVertexZError(Float_t f) { fPrimaryVertexZError = f;}

  void   SetTriggerMask(ULong64_t Ptr) {fTriggerMask = Ptr;}
  void   SetTriggerCluster(UChar_t n) {fTriggerCluster = n;}

  void   SetZDCNeutron1Energy(Float_t Pen) {fZDCNeutron1Energy = Pen;}
  void   SetZDCProton1Energy(Float_t Pen) {fZDCProton1Energy = Pen;}
  void   SetZDCNeutron2Energy(Float_t Pen) {fZDCNeutron2Energy = Pen;}
  void   SetZDCProton2Energy(Float_t Pen) {fZDCProton2Energy = Pen;}
  void   SetZDCEMEnergy(Float_t Pen) {fZDCEMEnergy = Pen;}

  void   SetT0VertexZ(Float_t Pvz) {fT0VertexZ = Pvz;}

  void   SetNumOfTracks(Int_t Ptr) {fNumberOfTracks = Ptr;}
  void   SetNumOfPosTracks(Int_t Ptr) {fNumberOfPositiveTracks = Ptr;}
  void   SetNumOfNegTracks(Int_t Ptr) {fNumberOfNegativeTracks = Ptr;}
  void   SetNumOfNeutrTracks(Int_t Ptr) {fNumberOfNeutralTracks = Ptr;}

  void   SetNumOfV0s(Int_t Ptr) {fNumberOfV0s = Ptr;}
  void   SetNumOfCascades(Int_t Ptr) {fNumberOfCascades = Ptr;}
  void   SetNumOfKinks(Int_t Ptr) {fNumberOfKinks = Ptr;}

  void   SetNumOfPMDTracks(Int_t Ptr) {fNumberOfPMDTracks = Ptr;}
  void   SetNumOfFMDTracks(Int_t Ptr) {fNumberOfFMDTracks = Ptr;}
  void   SetNumOfPHOSClusters(Int_t Ptr) {fNumberOfPHOSClusters = Ptr;}
  void   SetNumOfEMCALClusters(Int_t Ptr) {fNumberOfEMCALClusters = Ptr;}

  void   SetNumOfJetCandidates(Int_t Ptr) {fNumberOfJetCandidates = Ptr;}
  void   SetNumOfHardPhotonsCandidates(Int_t Ptr) {fNumberOfHardPhotonsCandidates = Ptr;}

  void   SetMaxJetEnergy(Float_t f) {fMaxJetEnergy = f;}
  void   SetMaxNeutralEnergy(Float_t f) {fMaxNeutralEnergy = f;}
  
  void   SetNumOfChargedAbove1GeV(Int_t i) {fNumberOfChargedAbove1GeV = i;}
  void   SetNumOfChargedAbove3GeV(Int_t i) {fNumberOfChargedAbove3GeV = i;}
  void   SetNumOfChargedAbove10GeV(Int_t i) {fNumberOfChargedAbove10GeV = i;}
  void   SetNumOfMuonsAbove1GeV(Int_t i) {fNumberOfMuonsAbove1GeV = i;}
  void   SetNumOfMuonsAbove3GeV(Int_t i) {fNumberOfMuonsAbove3GeV = i;}
  void   SetNumOfMuonsAbove10GeV(Int_t i) {fNumberOfMuonsAbove10GeV = i;}
  void   SetNumOfElectronsAbove1GeV(Int_t i) {fNumberOfElectronsAbove1GeV = i;}
  void   SetNumOfElectronsAbove3GeV(Int_t i) {fNumberOfElectronsAbove3GeV = i;}
  void   SetNumOfElectronsAbove10GeV(Int_t i) {fNumberOfElectronsAbove10GeV = i;}

  void   SetNumOfElectrons(Int_t Ptr) {fNumberOfElectrons = Ptr;}
  void   SetNumOfMuons(Int_t Ptr) {fNumberOfMuons = Ptr;}
  void   SetNumOfPions(Int_t Ptr) {fNumberOfPions = Ptr;}
  void   SetNumOfKaons(Int_t Ptr) {fNumberOfKaons = Ptr;}
  void   SetNumOfProtons(Int_t Ptr) {fNumberOfProtons = Ptr;}
  void   SetNumOfLambdas(Int_t Ptr) {fNumberOfLambdas = Ptr;}


  void   SetNumOfPhotons(Int_t Ptr) {fNumberOfPhotons = Ptr;}
  void   SetNumOfPi0s(Int_t Ptr) {fNumberOfPi0s = Ptr;}
  void   SetNumOfNeutrons(Int_t Ptr) {fNumberOfNeutrons = Ptr;}
  void   SetNumOfKaon0s(Int_t Ptr) {fNumberOfKaon0s = Ptr;}

  void   SetTotalMomentum(Float_t P) {fTotalP = P;}
  void   SetMeanPt(Float_t Pt) {fMeanPt = Pt;}
  void   SetMaxPt(Float_t Pt) {fMaxPt = Pt;}

  void   SetNeutralTotalMomentum(Float_t f) {fTotalNeutralP = f;}
  void   SetNeutralMeanPt(Float_t f) {fMeanNeutralPt = f;}
  void   SetNeutralMaxPt(Float_t f) {fMaxNeutralPt = f;}

  void   SetEventPlaneAngle(Float_t f) {fEventPlaneAngle = f;}
  void   SetHBTRadii(Float_t f) {fHBTRadii = f;}



  Int_t  GetEventId() const {return fAliceEventId;}
  const char   *GetGUID() const {return fGUID;}
 
  const char   *GetMD5() const {return fmd5;}
  const char   *GetTURL() const {return fturl;}
  Long64_t   GetSize() const {return fsize;}

  Int_t   GetNumOfParticipants() const {return fNumberOfParticipants;}
  Float_t GetImpactParameter() const {return fImpactParameter;}

  Float_t GetVertexX() const {return fPrimaryVertexX;}
  Float_t GetVertexY() const {return fPrimaryVertexY;}
  Float_t GetVertexZ() const {return fPrimaryVertexZ;}

  Int_t GetVertexFlag() const {return fPrimaryVertexFlag;}
  Float_t GetVertexZError() const {return fPrimaryVertexZError;}

  ULong64_t   GetTriggerMask() const {return fTriggerMask;}
  UChar_t   GetTriggerCluster() const {return fTriggerCluster;}

  Float_t GetZDCNeutron1Energy() const {return fZDCNeutron1Energy;}
  Float_t GetZDCProton1Energy() const {return fZDCProton1Energy;}
  Float_t GetZDCNeutron2Energy() const {return fZDCNeutron2Energy;}
  Float_t GetZDCProton2Energy() const {return fZDCProton2Energy;}
  Float_t GetZDCEMEnergy() const {return fZDCEMEnergy;}

  Float_t GetT0VertexZ() const {return fT0VertexZ;}

  Int_t   GetNumOfTracks() const {return fNumberOfTracks;}
  Int_t   GetNumOfPosTracks() const {return fNumberOfPositiveTracks;}
  Int_t   GetNumOfNegTracks() const {return fNumberOfNegativeTracks;}
  Int_t   GetNumOfNeutrTracks() const {return fNumberOfNeutralTracks;}

  Int_t   GetNumOfV0s() const {return fNumberOfV0s;}
  Int_t   GetNumOfCascades() const {return fNumberOfCascades;}
  Int_t   GetNumOfKinks() const {return fNumberOfKinks;}

  Int_t   GetNumOfPMDTracks() const {return fNumberOfPMDTracks;}
  Int_t   GetNumOfFMDTracks() const {return fNumberOfFMDTracks;}
  Int_t   GetNumOfPHOSClusters() const {return fNumberOfPHOSClusters;}
  Int_t   GetNumOfEMCALClusters() const {return fNumberOfEMCALClusters;}

  Int_t   GetNumOfJetCandidates() const {return fNumberOfJetCandidates;}
  Int_t   GetNumOfHardPhotonsCandidates() const {return fNumberOfHardPhotonsCandidates;}

  Float_t GetMaxJetEnergy() const {return fMaxJetEnergy;}
  Float_t GetMaxNeutralEnergy() const {return fMaxNeutralEnergy;}
  
  Int_t   GetNumOfChargedAbove1GeV() const {return fNumberOfChargedAbove1GeV;}
  Int_t   GetNumOfChargedAbove3GeV() const {return fNumberOfChargedAbove3GeV;}
  Int_t   GetNumOfChargedAbove10GeV() const {return fNumberOfChargedAbove10GeV;}
  Int_t   GetNumOfMuonsAbove1GeV() const {return fNumberOfMuonsAbove1GeV;}
  Int_t   GetNumOfMuonsAbove3GeV() const {return fNumberOfMuonsAbove3GeV;}
  Int_t   GetNumOfMuonsAbove10GeV() const {return fNumberOfMuonsAbove10GeV;}
  Int_t   GetNumOfElectronsAbove1GeV() const {return fNumberOfElectronsAbove1GeV;}
  Int_t   GetNumOfElectronsAbove3GeV() const {return fNumberOfElectronsAbove3GeV;}
  Int_t   GetNumOfElectronsAbove10GeV() const {return fNumberOfElectronsAbove10GeV;}
 
  Int_t   GetNumOfElectrons() const {return fNumberOfElectrons;}
  Int_t   GetNumOfMuons() const {return fNumberOfMuons;}
  Int_t   GetNumOfPions() const {return fNumberOfPions;}
  Int_t   GetNumOfKaons() const {return fNumberOfKaons;}
  Int_t   GetNumOfProtons() const {return fNumberOfProtons;}
  Int_t   GetNumOfLambdas() const {return fNumberOfLambdas;}

  Int_t   GetNumOfPhotons() const {return fNumberOfPhotons;}
  Int_t   GetNumOfPi0s() const {return fNumberOfPi0s;}
  Int_t   GetNumOfNeutrons() const {return fNumberOfNeutrons;}
  Int_t   GetNumOfKaon0s() const {return fNumberOfKaon0s;}


  Float_t GetTotalMomentum() const {return fTotalP;}
  Float_t GetMeanPt() const {return fMeanPt;}
  Float_t GetMaxPt() const {return fMaxPt;}

  Float_t GetNeutralTotalMomentum() const {return fTotalNeutralP;}
  Float_t GetNeutralMeanPt() const {return fMeanNeutralPt;}
  Float_t GetNeutralMaxPt() const {return fMaxNeutralPt;}

  Float_t GetEventPlaneAngle() const {return fEventPlaneAngle;}
  Float_t GetHBTRadii() const {return fHBTRadii;}

 private:
  Int_t    fAliceEventId;                      //The event id
  const char     *fGUID;		       //The unique identifier of the file
  
  Long64_t fsize;                             //the size of the file
  const char* fmd5;                           //another file identifier
  const char* fturl;                           //the file's url
  
  Int_t    fNumberOfParticipants;    	       //Number of participants
  Float_t  fImpactParameter;		       //The impact parameter
 
  Int_t    fPrimaryVertexFlag;		       //Primary vertex flag: 0->not found, 1->found

  Float_t  fPrimaryVertexX;		       //Primary vertex - X coordinate
  Float_t  fPrimaryVertexY;		       //Primary vertex - Y coordinate
  Float_t  fPrimaryVertexZ;		       //Primary vertex - Z coordinate

  Float_t  fPrimaryVertexZError;	       //Primary vertex - Z coordinate - error

  ULong64_t    fTriggerMask;		       //Information from trigger (trigger mask)
  UChar_t      fTriggerCluster;                // Trigger cluster (mask)
  Float_t  fZDCNeutron1Energy;		       //ZDC info - neutron
  Float_t  fZDCProton1Energy;		       //ZDC info - proton
  Float_t  fZDCNeutron2Energy;		       //ZDC info - neutron
  Float_t  fZDCProton2Energy;		       //ZDC info - proton
  Float_t  fZDCEMEnergy;		       //ZDC info - em
  Float_t  fT0VertexZ;			       //T0 info
  Int_t    fNumberOfTracks;		       //Multiplicity
  Int_t    fNumberOfPositiveTracks;	       //Multiplicity of positive tracks
  Int_t    fNumberOfNegativeTracks;	       //Multiplicity of negative tracks
  Int_t    fNumberOfNeutralTracks;	       //Multiplicity of neutral tracks
  Int_t    fNumberOfV0s;		       //Number of V0s
  Int_t    fNumberOfCascades;		       //Number of cascades
  Int_t    fNumberOfKinks;		       //Number of kinks
  Int_t    fNumberOfPMDTracks;		       //PMD tracks
  Int_t    fNumberOfFMDTracks;		       //FMD tracks
  Int_t    fNumberOfPHOSClusters;	       //PHOS clusters
  Int_t    fNumberOfEMCALClusters;	       //EMCAL clusters
  Int_t    fNumberOfJetCandidates;	       //Jet candidates

  Float_t  fMaxJetEnergy;                      //jet energy info

  Int_t    fNumberOfHardPhotonsCandidates;     //Hard photons candidates

  Float_t  fMaxNeutralEnergy;                   //neutral energy info
  

  Int_t    fNumberOfChargedAbove1GeV;      //Number of charged above 1 GeV/c
  Int_t    fNumberOfChargedAbove3GeV;      //Number of charged above 3 GeV/c
  Int_t    fNumberOfChargedAbove10GeV;     //Number of charged above 10 GeV/c
  Int_t    fNumberOfMuonsAbove1GeV;        //Number of muons above 1 GeV/c
  Int_t    fNumberOfMuonsAbove3GeV;        //Number of muons above 3 GeV/c
  Int_t    fNumberOfMuonsAbove10GeV;       //Number of muons above 10 GeV/c
  Int_t    fNumberOfElectronsAbove1GeV;    //Number of electrons above 1 GeV/c
  Int_t    fNumberOfElectronsAbove3GeV;    //Number of electrons above 3 GeV/c
  Int_t    fNumberOfElectronsAbove10GeV;   //Number of electrons above 10 GeV/c



  Int_t    fNumberOfElectrons;		       //Number of electrons
  Int_t    fNumberOfMuons;		       //Number of muons
  Int_t    fNumberOfPions;		       //Number of pions
  Int_t    fNumberOfKaons;		       //Number of kaons
  Int_t    fNumberOfProtons;		       //Number of protons
  Int_t    fNumberOfLambdas;		       //Number of lambdas

  Int_t    fNumberOfPhotons;               //Number of photons
  Int_t    fNumberOfPi0s;                  //Number of pi0
  Int_t    fNumberOfNeutrons;              //Number of neutrons
  Int_t    fNumberOfKaon0s;                //Number of Ks


 
  Float_t  fTotalP;			       //Sum of the momentum per event
  Float_t  fMeanPt;			       //Mean Pt per event
  Float_t  fMaxPt;			       //Max Pt for each event

  Float_t  fTotalNeutralP;		       //Sum of the momentum per event for neutral
  Float_t  fMeanNeutralPt;		       //Mean Pt per event for neutral
  Float_t  fMaxNeutralPt;		       //Max Pt for each event for neutral

  Float_t  fEventPlaneAngle;		       //event plane info
  Float_t  fHBTRadii;                          //HBT info

  ClassDef(AliEventTag,7)  //(ClassName, ClassVersion)
    };
//______________________________________________________________________________


#endif
