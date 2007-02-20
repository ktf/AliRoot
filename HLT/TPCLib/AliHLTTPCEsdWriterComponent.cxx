// @(#) $Id$

/** @file   AliHLTTPCEsdWriterComponent.cxx
    @author Matthias Richter
    @date   
    @brief  Writer component to store tracks of the HLT TPC conformal
            mapping tracker in the AliESD format

                                                                          */
#include "AliHLTTPCEsdWriterComponent.h"
#include "AliESD.h"
#include "TTree.h"
#include "AliHLTTPCTrack.h"
#include "AliHLTTPCTrackArray.h"
#include "AliHLTTPCTrackletDataFormat.h"
#include "AliHLTTPCDefinitions.h"

/** global instance for component registration */
AliHLTTPCEsdWriterComponent gTPCEsdWriter;

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTTPCEsdWriterComponent)

AliHLTTPCEsdWriterComponent::AliHLTTPCEsdWriterComponent()
  :
  fTree(NULL),
  fESD(NULL)
{
}

AliHLTTPCEsdWriterComponent::~AliHLTTPCEsdWriterComponent()
{
}

int AliHLTTPCEsdWriterComponent::InitWriter()
{
  int iResult=0;
  fESD = new AliESD;
  if (fESD) {
    fTree = new TTree("esdTree", "Tree with HLT ESD objects");
    if (fTree) {
      fTree->Branch("ESD", "AliESD", &fESD);
    }
    delete fESD;
    fESD=NULL;
  }
  if (fTree==NULL) {
    iResult=-ENOMEM;
  }
  return iResult;
}

int AliHLTTPCEsdWriterComponent::CloseWriter()
{
  int iResult=0;
  if (fTree) {
    WriteObject(kAliHLTVoidEventID, fTree);
    TTree* pTree=fTree;
    fTree=NULL;
    delete pTree;
  } else {
    HLTWarning("not initialized");
  }
  AliHLTRootFileWriterComponent::CloseWriter();
}

int AliHLTTPCEsdWriterComponent::DumpEvent( const AliHLTComponentEventData& evtData,
					    const AliHLTComponentBlockData* blocks, 
					    AliHLTComponentTriggerData& trigData )
{
  int iResult=0;
  TTree* pTree=fTree;
  if (pTree) {
    fESD = new AliESD;
    if (fESD) {
      AliESD* pESD=fESD;

      const AliHLTComponentBlockData* iter = NULL;
      AliHLTTPCTrackletData* inPtr=NULL;
 
      for (int ndx=0; ndx<evtData.fBlockCnt && iResult>=0; ndx++) {
	iter = blocks+ndx;
	if ( iter->fDataType == AliHLTTPCDefinitions::gkTrackSegmentsDataType ) {
	  Int_t minslice=AliHLTTPCDefinitions::GetMinSliceNr(iter->fSpecification);
	  Int_t maxslice=AliHLTTPCDefinitions::GetMaxSliceNr(iter->fSpecification);
	  //HLTDebug("dataspec %#x minslice %d", iter->fSpecification, minslice);
	  if (minslice >=0 && minslice<36) {
	    if (minslice!=maxslice) {
	      HLTWarning("data from multiple sectors in one block: "
			 "possible missmatch in treatment of local coordinate system");
	    }
	    AliHLTTPCTrackArray tracks;
	    inPtr=(AliHLTTPCTrackletData*)iter->fPtr;
	    HLTDebug("reading block %d (slice %d): %d tracklets", ndx, minslice, inPtr->fTrackletCnt);
	    tracks.FillTracks(inPtr->fTrackletCnt, inPtr->fTracklets, minslice, 0/*don't rotate*/);
	    if ((iResult=Tracks2ESD(&tracks, pESD))>=0) {
	    }
	  } else {
	    HLTError("invalid sector number");
	    iResult=-EBADF;
	  }
	}
      }
      if (iResult>=0) {
	pTree->Fill();
      }

      fESD=NULL;
      delete pESD;
    } else {
      iResult=-ENOMEM;
    }
  }
  return iResult;
}

int AliHLTTPCEsdWriterComponent::ScanArgument(int argc, const char** argv)
{
  int iResult=AliHLTRootFileWriterComponent::ScanArgument(argc, argv);
  return iResult;
}

int AliHLTTPCEsdWriterComponent::Tracks2ESD(AliHLTTPCTrackArray* pTracks, AliESD* pESD)
{
  int iResult=0;
  if (pTracks && pESD) {
    HLTDebug("converting %d tracks from track array", pTracks->GetNTracks());
    for (int i=0; i<pTracks->GetNTracks() && iResult>=0; i++) {
      AliHLTTPCTrack* pTrack=(*pTracks)[i];
      if (pTrack) {
	//HLTDebug("convert track %d", i);
	//pTrack->Print();
	int iLocal=pTrack->Convert2AliKalmanTrack();
	if (iLocal>=0) {
	AliESDtrack iotrack;
	iotrack.UpdateTrackParams(pTrack,AliESDtrack::kTPCin);
	iotrack.SetTPCPoints(pTrack->GetPoints());
	pESD->AddTrack(&iotrack);
	} else {
	  HLTError("conversion to AliKalmanTrack failed for track %d of %d", i, pTracks->GetNTracks());
	}
      } else {
	HLTError("internal missmatch in array");
	iResult=-EFAULT;
      }
    }
    
  } else {
    iResult=-EINVAL;
  }
  return iResult;
}
