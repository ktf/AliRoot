// $Id$

//**************************************************************************
//* This file is property of and copyright by the ALICE HLT Project        * 
//* ALICE Experiment at CERN, All rights reserved.                         *
//*                                                                        *
//* Primary Authors: Gaute Ovrebekk <ovrebekk@ift.uib.no>                  *
//*                  for The ALICE HLT Project.                            *
//*                                                                        *
//* Permission to use, copy, modify and distribute this software and its   *
//* documentation strictly for non-commercial purposes is hereby granted   *
//* without fee, provided that the above copyright notice appears in all   *
//* copies and that both the copyright notice and this permission notice   *
//* appear in the supporting documentation. The authors make no claims     *
//* about the suitability of this software for any purpose. It is          *
//* provided "as is" without express or implied warranty.                  *
//**************************************************************************

/** @file   AliHLTTPCTrackHistoComponent.cxx
    @author Gaute Ovrebekk, Matthias Richter
    @date   
    @brief  The TPC conformal mapping tracker component.
*/

#if __GNUC__>= 3
using namespace std;
#endif

#include "AliHLTTPCTrackHistoComponent.h"
#include "AliHLTTPCTransform.h"
#include "AliHLTTPCClusterDataFormat.h"
#include "AliHLTTPCTrackletDataFormat.h"
#include "AliHLTTPCMemHandler.h"
#include "AliHLTTPCDefinitions.h"
//#include "AliHLTTPCTrackArray.h"
//#include "AliHLTTPCTrack.h"
#include "AliHLTGlobalBarrelTrack.h"
#include "AliHLTExternalTrackParam.h"
#include "AliHLTDataTypes.h"

#include <TFile.h>
#include <TString.h>
#include "TNtuple.h"
#include "TH1F.h"
#include "TProfile.h"
#include "TObjString.h"
#include "TObjArray.h"


/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTTPCTrackHistoComponent)

AliHLTTPCTrackHistoComponent::AliHLTTPCTrackHistoComponent()
    :
    fMinSlice(35)
  , fMaxSlice(0)
  , fMinPartition(5)
  , fMaxPartition(0)
  //, fReset(0)
  , fNEvents(0)
  , fNtotTracks(0)
  , fEvtMod(20)
  , fBufferSize(5000)
  , fMeanMultiplicity(NULL)
  , fMultiplicity(NULL)
  //, fDeDxVsP(NULL)
  , fClusters(NULL)
  , fTracks(NULL)
  //, fNClusterVsXY(NULL)
  //, fChargeVsXY(NULL)
  //, fTracksArray(NULL)
  //, fClustersArray(NULL)
  //, fNSpacePoints(NULL)
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt
}

const char* AliHLTTPCTrackHistoComponent::fgkOCDBEntry="HLT/ConfigTPC/TPCTrackHisto";

AliHLTTPCTrackHistoComponent::~AliHLTTPCTrackHistoComponent(){
// see header file for class documentation
}

// Public functions to implement AliHLTComponent's interface.
// These functions are required for the registration process

const char* AliHLTTPCTrackHistoComponent::GetComponentID(){
// see header file for class documentation
  
  return "TPCTrackHisto";
}

void AliHLTTPCTrackHistoComponent::GetInputDataTypes(AliHLTComponentDataTypeList& list){
// see header file for class documentation
  
  list.clear();
  list.push_back(AliHLTTPCDefinitions::fgkClustersDataType|kAliHLTDataOriginTPC);
  list.push_back(kAliHLTDataTypeTrack|kAliHLTDataOriginTPC);
  //list.push_back(kAliHLTDataTypeTObjArray|kAliHLTDataOriginTPC);
}

AliHLTComponentDataType AliHLTTPCTrackHistoComponent::GetOutputDataType(){
// see header file for class documentation
  return kAliHLTMultipleDataType;
}

int AliHLTTPCTrackHistoComponent::GetOutputDataTypes(AliHLTComponentDataTypeList& tgtList){
// see header file for class documentation
  tgtList.clear();
  tgtList.push_back(kAliHLTDataTypeTNtuple|kAliHLTDataOriginTPC);
  tgtList.push_back(kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC);
  return tgtList.size();
}

void AliHLTTPCTrackHistoComponent::GetOutputDataSize( unsigned long& constBase, double& inputMultiplier ){
// see header file for class documentation
  
  constBase = 0;
  inputMultiplier = 1;// XXX TODO: Find more realistic value
}

AliHLTComponent* AliHLTTPCTrackHistoComponent::Spawn(){
// see header file for class documentation
  return new AliHLTTPCTrackHistoComponent;
}

int AliHLTTPCTrackHistoComponent::DoInit( int argc, const char** argv ){
// see header file for class documentation
 
  fClusters = new TNtuple("fClusters", "fClusters", "charge:qmax:residualY:residualZ"); 
  fTracks   = new TNtuple("fTracks",  "fTracks",  "pt:eta:psi:nclusters"); 
 
  fClusters->SetCircular(5000);
  fTracks->SetCircular(5000);
 
  //fTracksArray = new AliHLTTPCTrackArray();

  fMultiplicity     = new TH1F("fMultiplicity",     "Track multiplicity per event",     1000,           0, 1000);
  fMeanMultiplicity = new TH1F("fMeanMultiplicity", "Mean track multiplicity vs. #evt", 10000/fEvtMod, 0, 10000);
  //fDeDxVsP          = new TProfile("fDeDxVsP",      "E-deposition per unit length vs. p",100, 0, 100);
  
  // first configure the default
  int iResult=0;
  if (iResult>=0) iResult=ConfigureFromCDBTObjString(fgkOCDBEntry);

  // configure from the command line parameters if specified
  if (iResult>=0 && argc>0)
    iResult=ConfigureFromArgumentString(argc, argv);
  
  return iResult;

  
  
  
//   int iResult = 0;
//   TString configuration = "";
//   TString argument = "";
//   for(int i=0; i<argc && iResult>=0; i++){
//       argument = argv[i];
//       if(!configuration.IsNull()) configuration += " ";
//       configuration += argument;
//   }
//   
//   if(!configuration.IsNull()){
//      iResult = Configure(configuration.Data());
//   }  
//   return iResult; 
}
  
int AliHLTTPCTrackHistoComponent::DoDeinit(){
// see header file for class documentation
  
  delete fClusters;
  delete fTracks;
  //delete fTracksArray;

  delete fMultiplicity;
  delete fMeanMultiplicity;
  //delete fDeDxVsP;

  return 0;
}

int AliHLTTPCTrackHistoComponent::Reconfigure(const char* cdbEntry, const char* /*chainId*/){
// see header file for class documentation

  // configure from the specified antry or the default one
  const char* entry=cdbEntry;
  if (!entry || entry[0]==0) {
     entry=fgkOCDBEntry;
  }

  return ConfigureFromCDBTObjString(entry);
}


int AliHLTTPCTrackHistoComponent::DoEvent(const AliHLTComponentEventData& /*evtData*/, AliHLTComponentTriggerData& /*trigData*/){
// see header file for class documentation

  if(GetFirstInputBlock(kAliHLTDataTypeSOR) || GetFirstInputBlock(kAliHLTDataTypeEOR)) return 0;  

  fNEvents++;

  //if(!fTracksArray) fTracksArray = new AliHLTTPCTrackArray();

  const AliHLTComponentBlockData *iter = NULL;
 
 
  //----------------- loop over cluster blocks ---------------------//
  
  Int_t totalSpacePoints = 0;
  
  for(iter = GetFirstInputBlock(AliHLTTPCDefinitions::fgkClustersDataType); iter != NULL; iter = GetNextInputBlock()){
            
      if(iter->fDataType!=AliHLTTPCDefinitions::fgkClustersDataType) continue;

      AliHLTUInt8_t minSlice     = AliHLTTPCDefinitions::GetMinSliceNr(*iter);
      AliHLTUInt8_t minPartition = AliHLTTPCDefinitions::GetMinPatchNr(*iter);
      //HLTDebug("Input Data - TPC cluster - slice/partition: %d/%d.", minSlice, minPartition);

      const AliHLTTPCClusterData* clusterData = (const AliHLTTPCClusterData*)iter->fPtr;
      Int_t nSpacepoint = (Int_t)clusterData->fSpacePointCnt;    
      totalSpacePoints += nSpacepoint;
      HLTDebug("TrackHisto component found %d spacepoints in slice %d partition %d", nSpacepoint, minSlice, minPartition);
      
      AliHLTTPCSpacePointData *clusters = (AliHLTTPCSpacePointData*)clusterData->fSpacePoints;
      
      if(fClustersArray[minSlice][minPartition] != NULL){
         //delete(fClustersArray[minSlice][minPartition]);
         fClustersArray[minSlice][minPartition] = NULL;
      }      

      // fill the array with AliHLTTPCSpacePointData pointers
      // it will be used in the track loop to access information
      // for the used clusters only
      fClustersArray[minSlice][minPartition] = clusters;
      fNSpacePoints[minSlice][minPartition]  = nSpacepoint;
      
      if(nSpacepoint==0) fClustersArray[minSlice][minPartition] = NULL;

  } // end of loop over cluster data blocks
  
  HLTDebug("TrackHisto found %d spacepoints",totalSpacePoints);
  
  
  
  
  //----------------- loop over merged tracks -------------------//

  Int_t totalTracks = 0;
  
  for(iter = GetFirstInputBlock(kAliHLTDataTypeTrack|kAliHLTDataOriginTPC); iter != NULL; iter = GetNextInputBlock()){    
      if(iter->fDataType != (kAliHLTDataTypeTrack|kAliHLTDataOriginTPC)) continue; 
      ReadTracks(iter,totalTracks);
  }
  
  HLTDebug("TrackHisto found %d tracks", totalTracks);  

  fMultiplicity->Fill(totalTracks);
  
  fNtotTracks += totalTracks;
  
  if(fNEvents%fEvtMod==0){    
     fMeanMultiplicity->Fill(fNEvents, Float_t(fNtotTracks)/(fEvtMod));
     //HLTInfo("-------------- Event number: %d, total tracks accummulated %d", fNEvents, fNtotTracks);
     fNtotTracks = 0;
  }

  PushHisto();
  
  //delete fTracksArray; fTracksArray = NULL;
  return 0;
}
 
// int AliHLTTPCTrackHistoComponent::Configure(const char* arguments){
// // see header file for class documentation
//    
//    int iResult=0;
//    if (!arguments) return iResult;
//    
//    TString allArgs=arguments;
//    TString argument;
//    
//    TObjArray* pTokens=allArgs.Tokenize(" ");
//    if (pTokens) {
//      for (int i=0; i<pTokens->GetEntries() && iResult>=0; i++) {
//        argument=((TObjString*)pTokens->At(i))->GetString();
//        if (argument.IsNull()) continue;
// 
//        HLTError("unknown argument %s", argument.Data());
//        iResult=-EINVAL;
//        break;
//      }
//      delete pTokens;
//    }   
//    return iResult;
// }
 
void AliHLTTPCTrackHistoComponent::ReadTracks(const AliHLTComponentBlockData* iter,Int_t &tt){
// see header file for class documentation

  AliHLTUInt8_t slice	  = AliHLTTPCDefinitions::GetMinSliceNr(*iter);
  AliHLTUInt8_t partition = AliHLTTPCDefinitions::GetMinPatchNr(*iter);
 
  if( slice     < fMinSlice )	  fMinSlice     = slice;
  if( slice     > fMaxSlice )	  fMaxSlice     = slice;
  if( partition < fMinPartition ) fMinPartition = partition;
  if( partition > fMaxPartition ) fMaxPartition = partition;
  
  Int_t usedSpacePoints = 0;
  
  vector<AliHLTGlobalBarrelTrack> tracksVector;
  AliHLTGlobalBarrelTrack::ConvertTrackDataArray(reinterpret_cast<const AliHLTTracksData*>(iter->fPtr), iter->fSize, tracksVector);
  
  tt = tracksVector.size();
  
   for(vector<AliHLTGlobalBarrelTrack>::iterator element=tracksVector.begin();  element!=tracksVector.end(); element++){
 
//     Double_t trackLength = GetTrackLength(tpcTrack);      
      UInt_t nHits = element->GetNumberOfPoints();
      fTracks->Fill( 1./element->OneOverPt(), element->GetSnp(), element->GetTgl(), nHits );  
      
      Double_t totCharge = 0;
      const UInt_t *hitnum = element->GetPoints();
      for(UInt_t i=0; i<element->GetNumberOfPoints(); i++){
          
	  UInt_t idTrack   = hitnum[i];
          Int_t sliceTrack = (idTrack>>25) & 0x7f;
          Int_t patchTrack = (idTrack>>22) & 0x7;
          UInt_t pos       = idTrack&0x3fffff;

          if( !fClustersArray[sliceTrack][patchTrack]    ) continue;
          if(  fNSpacePoints[sliceTrack][patchTrack]<pos ) HLTError("Space point array out of boundaries!"); 
	  
	  totCharge += (fClustersArray[sliceTrack][patchTrack])[pos].fCharge; 
	  
	  Float_t xyz[3]; xyz[0] = xyz[1] = xyz[2] = 0.;
       
          xyz[0] = (fClustersArray[sliceTrack][patchTrack])[pos].fX;
          xyz[1] = (fClustersArray[sliceTrack][patchTrack])[pos].fY;
          xyz[2] = (fClustersArray[sliceTrack][patchTrack])[pos].fZ;
       
          AliHLTTPCTransform::Local2Global(xyz,slice); 
	  
	  Double_t p[2]   = { xyz[1], xyz[2]};
          Double_t cov[3] = { (fClustersArray[sliceTrack][patchTrack])[pos].fSigmaY2, 0., (fClustersArray[sliceTrack][patchTrack])[pos].fSigmaZ2};  
 	  Double_t *res = element->GetResiduals(p,cov,kFALSE); 
 	  
 	  //HLTInfo("resy: %f, resz: %f", res[0], res[1]);
 	  
 	  if(!res)  res[0] = res[1] = -1000.;
 	  else      fClusters->Fill( (fClustersArray[sliceTrack][patchTrack])[pos].fCharge, (fClustersArray[sliceTrack][patchTrack])[pos].fQMax, res[0], res[1]);
           
 	  usedSpacePoints++;      
       }        
   //if(trackLength > 0) fDeDxVsP->Fill(element->OneOverPt()*TMath::Sqrt(1.+element->GetTgl()*element->GetTgl()), totCharge/trackLength);       
   }
}

void AliHLTTPCTrackHistoComponent::PushHisto(){
// see header file for class documentation

    AliHLTUInt32_t fSpecification = AliHLTTPCDefinitions::EncodeDataSpecification(fMinSlice,fMaxSlice,fMinPartition,fMaxPartition);
    
    PushBack( (TObject*)fTracks,           kAliHLTDataTypeTNtuple  |kAliHLTDataOriginTPC, fSpecification);   
    PushBack( (TObject*)fClusters,         kAliHLTDataTypeTNtuple  |kAliHLTDataOriginTPC, fSpecification);
    PushBack( (TObject*)fMultiplicity,     kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC, fSpecification);
    PushBack( (TObject*)fMeanMultiplicity, kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC, fSpecification); 
    //PushBack( (TObject*)fDeDxVsP,          kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC, fSpecification);
}

int AliHLTTPCTrackHistoComponent::ScanConfigurationArgument(int argc, const char** argv){
// see header file for class documentation
 
  if (argc<=0) return 0;
  int i=0;
  TString argument=argv[i];

  // -event-modulo
  if (argument.CompareTo("-event-modulo")==0) {
    if (++i>=argc) return -EPROTO;
    argument=argv[i];
    fEvtMod=argument.Atof();
    return 2;
  }    

  // -buffer-size
  if (argument.CompareTo("-buffer-size")==0) {
    if (++i>=argc) return -EPROTO;
    argument=argv[i];
    fBufferSize=argument.Atof();
    return 2;
  }    
  return -EINVAL;
}

// void AliHLTTPCTrackHistoComponent::FillResidual(UInt_t pos,AliHLTUInt8_t slice,AliHLTUInt8_t partition,Float_t& resy,Float_t& resz){
// // see header file for class documentation
//  
//   AliHLTTPCSpacePointData *cl =  &fClustersArray[slice][partition][pos];
//   if(!cl) return;
// 
//   AliHLTTPCTrack *gtrack = NULL;
// 
//    for(int i=0;i<fTracksArray->GetNTracks();i++){
//        
//        AliHLTTPCTrack *tt = fTracksArray->GetCheckedTrack(i); 
//        UInt_t *hitnum =tt->GetHitNumbers();
//        Int_t nHits = tt->GetNHits();
//          
//  	for(Int_t h=0; h<nHits; h++){
//  	    UInt_t id=hitnum[h];
//              Int_t Tslice = (id>>25) & 0x7f;
//              Int_t Tpatch = (id>>22) & 0x7;
//              UInt_t Tpos = id&0x3fffff; 
//              if(Tslice==slice && Tpatch==partition && Tpos==pos){
//  	       gtrack = tt; 
//  	       break;
//              }
//          }
//    }
//    
//   if(!gtrack) return;
// 
//   Int_t tslice = gtrack->GetSector();
// 
//   /*
//   Double_t radius = gtrack->GetRadius();      // radius
//   Double_t kappa = gtrack->GetKappa();        // curvature = 1/R , signed
//   Double_t lambda = atan( gtrack->GetTgl() ); // dipAngle lambda
// 
//   // ------------------------------------
//   // ++ Get first/last point of the track
//   
//   Double_t xyzL[3];      // lastpoint of track
//   Double_t xyzF[3];      // firstpoint of track
//   
//   xyzF[0] = gtrack->GetFirstPointX();
//   xyzF[1] = gtrack->GetFirstPointY();
//   xyzF[2] = gtrack->GetFirstPointZ();
//   
//   xyzL[0] = gtrack->GetLastPointX();
//   xyzL[1] = gtrack->GetLastPointY();
//   xyzL[2] = gtrack->GetLastPointZ();
// 
//   // --------------------------
//   // ++ Calculate length of the track
//   
//   Double_t s = 0.;       // length of the track
//   if (  AliHLTTPCTransform::GetBFieldValue() == 0. || kappa == 0 ) 
//     s = sqrt ( (xyzL[0] - xyzF[0])*(xyzL[0] - xyzF[0]) + (xyzL[1] - xyzF[1])*(xyzL[1] - xyzF[1]) ); 
//   else {
//     // Calculate the length of the track. If it is to flat in in s,z plane use sxy, otherwise use sz
//     if (fabs(lambda) > 0.05){
//       // length of track calculated out of z
//       s = fabs( (xyzL[2] - xyzF[2]) / sin(lambda) ); // length of track calculated out of z
//     }
//     else {
//       Double_t d = (xyzL[0] - xyzF[0])*(xyzL[0] - xyzF[0]) + (xyzL[1] - xyzF[1])*(xyzL[1] - xyzF[1]); 
//       // length of track calculated out of xy
//       s = fabs ( acos( 0.5 * (2 - (d / (radius*radius)))) / ( kappa * cos(lambda) ) ); 		
//     }
//   }
//   */  //???
//   
//   gtrack->Rotate(tslice,kTRUE);
//   
//   //Double_t padrows = 0;                   
//     
//   Float_t xyzC[3];       // cluster tmp
//   Float_t xyzTtmp[3];    // track tmp
// 
//   xyzC[0] = cl->fX;
//   xyzC[1] = cl->fY;
//   xyzC[2] = cl->fZ;
//  
//   Int_t padrow = AliHLTTPCTransform::GetPadRow(cl->fX);
// 
//   xyzTtmp[0] = gtrack->GetFirstPointX();
// 
//   if(gtrack->GetCrossingPoint(padrow,xyzTtmp)) {
//     // ----------------------
//        // ++ Calculate Residuals
//        
//        Float_t deltaY = ( xyzC[1] - xyzTtmp[1] );
//        Float_t deltaZ = ( xyzC[2] - xyzTtmp[2] );
//        
//        resy = deltaY;
//        resz = deltaZ;
//   } else {
//     resy = -1000;
//     resz = -1000;
//   }
// }

// Double_t AliHLTTPCTrackHistoComponent::GetTrackLength(AliHLTTPCTrack *hltTrack){
// // see header file for class documentation
//   
//   AliHLTTPCTrack * gtrack = hltTrack;
// 
//   //Caculate the HLT Track Length
// 
//   Double_t radius = gtrack->GetRadius();      // radius 
//   Double_t kappa = gtrack->GetKappa();        // curvature = 1/R , signed 
//   Double_t lambda = atan( gtrack->GetTgl() ); // dipAngle lambda 
// 
//   // ------------------------------------ 
//   // ++ Get first/last point of the track 
//    
//   Double_t xyzL[3];      // lastpoint of track 
//   Double_t xyzF[3];      // firstpoint of track 
//    
//   xyzF[0] = gtrack->GetFirstPointX(); 
//   xyzF[1] = gtrack->GetFirstPointY(); 
//   xyzF[2] = gtrack->GetFirstPointZ(); 
//    
//   xyzL[0] = gtrack->GetLastPointX(); 
//   xyzL[1] = gtrack->GetLastPointY(); 
//   xyzL[2] = gtrack->GetLastPointZ(); 
// 
//   // -------------------------- 
//   // ++ Calculate length of the track 
//    
//   Double_t s = 0.;       // length of the track 
//   if (  AliHLTTPCTransform::GetBFieldValue() == 0. || kappa == 0 )  
//     s = sqrt ( (xyzL[0] - xyzF[0])*(xyzL[0] - xyzF[0]) 
// 	       + (xyzL[1] - xyzF[1])*(xyzL[1] - xyzF[1]) );  
//   else { 
//     // Calculate the length of the track. If it is to flat in in s,z plane use sxy, otherwise use sz 
//     if (fabs(lambda) > 0.05){ 
//       // length of track calculated out of z 
//       s = fabs( (xyzL[2] - xyzF[2]) / sin(lambda) ); // length of track calculated out of z 
//   } else { 
//       Double_t d = (xyzL[0] - xyzF[0])*(xyzL[0] - xyzF[0]) 
// 	+ (xyzL[1] - xyzF[1])*(xyzL[1] - xyzF[1]);  
//       // length of track calculated out of xy 
//       s = fabs ( acos( 0.5 * (2 - (d / (radius*radius)))) 
// 		 / ( kappa * cos(lambda) ) );                  
//     } 
//   }
//   return s;
// }
