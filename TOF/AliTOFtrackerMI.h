#ifndef ALITOFTRACKERMI_H
#define ALITOFTRACKERMI_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

//-----------------------------------------------------------------//
//                                                                 //
//   AliTOFtrackerMI Class                                         //
//   Task: Perform association of the ESD tracks to TOF Clusters   //
//   and Update ESD track with associated TOF Cluster parameters   //
//                                                                 //
//-----------------------------------------------------------------//

#include "AliTracker.h"

//#include "AliTOFpidESD.h"

class TTreeSRedirector;
class TClonesArray;

class AliESDEvent;

class AliTOFcluster;
class AliTOFRecoParam;
class AliTOFGeometry;
class AliTOFtrack;
class AliTOFpidESD;

class AliTOFtrackerMI : public AliTracker {

enum {kMaxCluster=77777}; //maximal number of the TOF clusters

public:

 AliTOFtrackerMI(); 
 AliTOFtrackerMI(const AliTOFtrackerMI &t); //Copy Ctor 
 AliTOFtrackerMI& operator=(const AliTOFtrackerMI &source); // ass. op.

 //  virtual ~AliTOFtrackerMI() {delete fTOFpid;}
 virtual ~AliTOFtrackerMI();
 virtual Int_t Clusters2Tracks(AliESDEvent* /*event*/) {return -1;};
 virtual Int_t PropagateBack(AliESDEvent* event);
 virtual Int_t RefitInward(AliESDEvent* /*event*/) {return -1;};
 virtual Int_t LoadClusters(TTree *dTree); // Loading Clusters from Digits
 virtual void  UnloadClusters();// UnLoad Clusters
 virtual AliCluster *GetCluster(Int_t /*index*/) const {return NULL;};
 void    GetLikelihood(Float_t dy, Float_t dz, const Double_t *cov, AliTOFtrack * track, Float_t & py, Float_t &pz);
public:
 /*  class AliTOFcluster { */
/*     friend class AliTOFtrackerMI; */
/*   public: */
/*     AliTOFcluster(Double_t *h, Int_t *l, Int_t *ind, Int_t idx) { */
/*       fR=h[0]; fPhi=h[1]; fZ=h[2]; fTDC=h[3]; fADC=h[4]; */
/*       fLab[0]=l[0]; fLab[1]=l[1]; fLab[2]=l[2]; */
/*       fIdx=idx; */
/*       fdetIndex[0]=ind[0]; */
/*       fdetIndex[1]=ind[1]; */
/*       fdetIndex[2]=ind[2]; */
/*       fdetIndex[3]=ind[3]; */
/*       fdetIndex[4]=ind[4]; */
/*       fQuality    = -100; */
/*     } */
/*     void Use() {fADC=-fADC;} */

/*     Double_t GetR() const {return fR;}  // Cluster Radius */
/*     Double_t GetPhi() const {return fPhi;} // Cluster Phi */
/*     Double_t GetZ()   const {return fZ;} // Cluster Z */
/*     Double_t GetTDC() const {return fTDC;} // Cluster ToF */
/*     Double_t GetADC() const {return TMath::Abs(fADC);} // Cluster Charge */
/*     Int_t IsUsed() const {return (fADC<0) ? 1 : 0;} // Flagging */
/*     Int_t GetLabel(Int_t n) const {return fLab[n];} // Labels of tracks in Cluster */
/*     Int_t GetDetInd(Int_t n) const {return fdetIndex[n];} //Cluster Det Indeces */
/*     Int_t GetIndex() const {return fIdx;} // Cluster Index */

/*   private: */

/*     Int_t fLab[3]; //track labels */
/*     Int_t fIdx;    //index of this cluster */
/*     Int_t fdetIndex[5]; //Cluster detector Indeces (plate,strip,..) */
/*     Double_t fR;   //r-coordinate */
/*     Double_t fPhi; //phi-coordinate */
/*     Double_t fZ;   //z-coordinate */
/*     Double_t fTDC; //TDC count */
/*     Double_t fADC; //ADC count */
/*     Double_t fQuality;  // quality of the best track */
/*   }; */

private:

 Int_t InsertCluster(AliTOFcluster *c); // Fills TofClusters Array
 Int_t FindClusterIndex(Double_t z) const; // Returns cluster index 
 void  MatchTracks(Bool_t mLastStep); // Matching Algorithm 
 void  MatchTracksMI(Bool_t mLastStep); // Matching Algorithm 
 void  CollectESD(); // Select starting Set for Matching 
 //void  Init();
 Float_t GetLinearDistances(AliTOFtrack * track, AliTOFcluster *cluster, Float_t distances[5]);
 AliTOFRecoParam*  fRecoParam;           // Pointer to TOF Recontr. Params
 AliTOFGeometry*  fGeom;                 // Pointer to TOF geometry
 AliTOFpidESD*    fPid;               // Pointer to TOF PID
 AliTOFcluster *fClusters[kMaxCluster];  // pointers to the TOF clusters

 Int_t fN;              // Number of Clusters
 Int_t fNseeds;         // Number of track seeds  
 Int_t fNseedsTOF;      // TPC BP tracks
 Int_t fngoodmatch;     // Correctly matched  tracks
 Int_t fnbadmatch;      // Wrongly matched tracks
 Int_t fnunmatch;       // Unmatched tracks
 Int_t fnmatch;         // Total matched tracks
 
 Float_t fR;            // Intermediate radius in TOF, used in matching
 Float_t fTOFHeigth;    // Inner TOF radius for propagation
 Float_t fdCut;         // Cut on minimum distance track-pad in matching 
 Float_t fDx;           // Pad Size in X   
 Float_t fDy;           // Pad Size in Y (== X  TOF convention)
 Float_t fDz;           // Pad Size in Z 
 TClonesArray* fTracks; //! pointer to the TClonesArray with TOF tracks
 TClonesArray* fSeeds;  //! pointer to the TClonesArray with ESD tracks
 TTreeSRedirector *fDebugStreamer;     //!debug streamer
 ClassDef(AliTOFtrackerMI, 1) // TOF trackerMI 
};

#endif
