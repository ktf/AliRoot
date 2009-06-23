#ifndef ALIITSTRACKERHLT_H
#define ALIITSTRACKERHLT_H
/* Copyright(c) 2007-2009, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */


class TTree;
class TTreeSRedirector;
class AliESDEvent;
class AliESDtrack;

class AliITSChannelStatus;
class AliITSDetTypeRec;
#include "AliHLTITSTrack.h" 
#include "AliHLTITSDetector.h"
#include "AliHLTITSLayer.h"

#include <TObjArray.h>

#include "AliITSRecPoint.h"
#include "AliTracker.h"
#include "AliHLTITSTrack.h"
#include <vector>

//-------------------------------------------------------------------------
class AliITStrackerHLT : public AliTracker {
public:

  
  void LoadClusters( std::vector<AliITSRecPoint> clusters );
  void Reconstruct( std::vector<AliExternalTrackParam> tracksTPC );
  std::vector< AliHLTITSTrack > &Tracks(){ return fTracks;}

  Bool_t TransportToX( AliExternalTrackParam *t, double x ) const;
  Bool_t TransportToPhiX( AliExternalTrackParam *t, double phi, double x ) const;
  
  void GetClusterErrors2( Int_t layer, const AliITSRecPoint *cluster, AliHLTITSTrack* track, double &err2Y, double &err2Z ) const ;

  AliITStrackerHLT();
  AliITStrackerHLT(const Char_t *geom);
  virtual ~AliITStrackerHLT();
  AliCluster *GetCluster(Int_t index) const;
  virtual Bool_t GetTrackPoint(Int_t index, AliTrackPoint& p) const;
  virtual Bool_t GetTrackPointTrackingError(Int_t index, 
			AliTrackPoint& p, const AliESDtrack *t);
  AliITSRecPoint *GetClusterLayer(Int_t layn, Int_t ncl) const
                  {return fgLayers[layn].GetCluster(ncl);}
  Int_t GetNumberOfClustersLayer(Int_t layn) const 
                        {return fgLayers[layn].GetNumberOfClusters();}
  Int_t LoadClusters(TTree *cf);
  void UnloadClusters();
  
  Int_t Clusters2Tracks(AliESDEvent *event);
  Int_t PropagateBack(AliESDEvent *event);
  Int_t RefitInward(AliESDEvent *event);

  void SetLayersNotToSkip(const Int_t *l);

  Double_t GetPredictedChi2MI(AliHLTITSTrack* track, const AliITSRecPoint *cluster,Int_t layer);
  Int_t UpdateMI(AliHLTITSTrack* track, const AliITSRecPoint* cl,Double_t chi2,Int_t layer) const;  
  void SetDetTypeRec(const AliITSDetTypeRec *detTypeRec) {fkDetTypeRec = detTypeRec; ReadBadFromDetTypeRec(); }

  TTreeSRedirector *GetDebugStreamer() {return fDebugStreamer;}
  static Int_t CorrectForTPCtoITSDeadZoneMaterial(AliHLTITSTrack *t);


  AliHLTITSLayer    & GetLayer(Int_t layer) const;
  AliHLTITSDetector & GetDetector(Int_t layer, Int_t n) const {return GetLayer(layer).GetDetector(n); }
 
  void FollowProlongationTree(AliHLTITSTrack * otrack);



protected:

  Bool_t ComputeRoad(AliHLTITSTrack* track,Int_t ilayer,Int_t idet,Double_t &zmin,Double_t &zmax,Double_t &ymin,Double_t &ymax) const;
  
  
  void CookLabel(AliKalmanTrack *t,Float_t wrong) const;
  void CookLabel(AliHLTITSTrack *t,Float_t wrong) const;

  void       SignDeltas(const TObjArray *clusterArray, Float_t zv);
  void BuildMaterialLUT(TString material);
  
  Int_t CorrectForPipeMaterial(AliHLTITSTrack *t, TString direction="inward");
  Int_t CorrectForShieldMaterial(AliHLTITSTrack *t, TString shield, TString direction="inward");
  Int_t CorrectForLayerMaterial(AliHLTITSTrack *t, Int_t layerindex, Double_t oldGlobXYZ[3], TString direction="inward");
  void UpdateESDtrack(AliESDtrack *tESD,AliHLTITSTrack* track, ULong_t flags) const;
  void ReadBadFromDetTypeRec();
  
  Int_t CheckDeadZone(AliHLTITSTrack *track,Int_t ilayer,Int_t idet,Double_t dz,Double_t dy,Bool_t noClusters=kFALSE) const;
  Bool_t LocalModuleCoord(Int_t ilayer,Int_t idet,const AliHLTITSTrack *track,
			  Float_t &xloc,Float_t &zloc) const;
// method to be used for Plane Efficiency evaluation

  // 

  static AliHLTITSLayer fgLayers[AliITSgeomTGeo::kNLayers];// ITS layers
  
  AliESDEvent  * fEsd;                   //! pointer to the ESD event
  Double_t fSPDdetzcentre[4];            // centres of SPD modules in z
  
  Int_t fUseTGeo;                        // use TGeo to get material budget

  Float_t fxOverX0Pipe;                  // material budget
  Float_t fxTimesRhoPipe;                // material budget
  Float_t fxOverX0Shield[2];             // material budget
  Float_t fxTimesRhoShield[2];           // material budget
  Float_t fxOverX0Layer[6];              // material budget
  Float_t fxTimesRhoLayer[6];            // material budget

  TTreeSRedirector *fDebugStreamer;      //!debug streamer
  AliITSChannelStatus *fITSChannelStatus;//! bitmaps with channel status for SPD and SDD
  const AliITSDetTypeRec *fkDetTypeRec;         //! ITS det type rec, from AliITSReconstructor
  std::vector< AliHLTITSTrack > fTracks;

private:
  AliITStrackerHLT(const AliITStrackerHLT &tracker);
  AliITStrackerHLT & operator=(const AliITStrackerHLT &tracker);  
  ClassDef(AliITStrackerHLT,0)   //HLT ITS tracker
};




/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////





inline void AliITStrackerHLT::CookLabel(AliKalmanTrack *t,Float_t wrong) const {
  //--------------------------------------------------------------------
  //This function "cooks" a track label. If label<0, this track is fake.
  //--------------------------------------------------------------------
   Int_t tpcLabel=t->GetLabel();
   if (tpcLabel<0) return;
   AliTracker::CookLabel(t,wrong);
   if (tpcLabel!=TMath::Abs(t->GetLabel())){
     t->SetFakeRatio(1.);
   }
   if (tpcLabel !=t->GetLabel()) {
     t->SetLabel(-tpcLabel);      
   }
}



#endif
