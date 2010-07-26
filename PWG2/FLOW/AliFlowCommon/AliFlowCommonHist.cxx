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

#include "Riostream.h"              //needed as include
#include "AliFlowCommonConstants.h" //needed as include
#include "AliFlowCommonHist.h"
#include "AliFlowEventSimple.h"
#include "AliFlowTrackSimple.h"

#include "TString.h" 
#include "TProfile.h"
#include "TMath.h"   //needed as include
#include "TList.h"
#include "TH2F.h"
#include "AliFlowVector.h"
#include "TBrowser.h"

class TH1F;
class TH1D;

// AliFlowCommonHist:
//
// Description: Class to organise common histograms for Flow Analysis
//
// authors: N. van der Kolk (kolk@nikhef.nl), A. Bilandzic (anteb@nikhef.nl), RS


ClassImp(AliFlowCommonHist)

//-----------------------------------------------------------------------

AliFlowCommonHist::AliFlowCommonHist():
  TNamed(),
  fHistMultRP(NULL),
  fHistMultPOI(NULL),
  fHistPtRP(NULL),
  fHistPtPOI(NULL),
  fHistPtSub0(NULL),
  fHistPtSub1(NULL),
  fHistPhiRP(NULL),
  fHistPhiPOI(NULL),
  fHistPhiSub0(NULL),
  fHistPhiSub1(NULL),
  fHistEtaRP(NULL),
  fHistEtaPOI(NULL),
  fHistEtaSub0(NULL),
  fHistEtaSub1(NULL),
  fHistPhiEtaRP(NULL),
  fHistPhiEtaPOI(NULL),
  fHistProMeanPtperBin(NULL),
  fHistQ(NULL),
  fHistAngleQ(NULL),
  fHistAngleQSub0(NULL),
  fHistAngleQSub1(NULL), 
  fHarmonic(NULL),
  fRefMultVsNoOfRPs(NULL),
  fHistList(NULL)
{
  
  //default constructor
  
}

AliFlowCommonHist::AliFlowCommonHist(const AliFlowCommonHist& a):
  TNamed(),
  fHistMultRP(new TH1F(*a.fHistMultRP)),
  fHistMultPOI(new TH1F(*a.fHistMultPOI)),
  fHistPtRP(new TH1F(*a.fHistPtRP)),
  fHistPtPOI(new TH1F(*a.fHistPtPOI)),
  fHistPtSub0(new TH1F(*a.fHistPtSub0)),
  fHistPtSub1(new TH1F(*a.fHistPtSub1)),
  fHistPhiRP(new TH1F(*a.fHistPhiRP)),
  fHistPhiPOI(new TH1F(*a.fHistPhiPOI)),
  fHistPhiSub0(new TH1F(*a.fHistPhiSub0)),
  fHistPhiSub1(new TH1F(*a.fHistPhiSub1)),
  fHistEtaRP(new TH1F(*a.fHistEtaRP)),
  fHistEtaPOI(new TH1F(*a.fHistEtaPOI)),
  fHistEtaSub0(new TH1F(*a.fHistEtaSub0)),
  fHistEtaSub1(new TH1F(*a.fHistEtaSub1)),
  fHistPhiEtaRP(new TH2F(*a.fHistPhiEtaRP)),
  fHistPhiEtaPOI(new TH2F(*a.fHistPhiEtaPOI)),
  fHistProMeanPtperBin(new TProfile(*a.fHistProMeanPtperBin)),
  fHistQ(new TH1F(*a.fHistQ)),
  fHistAngleQ(new TH1F(*a.fHistAngleQ)),
  fHistAngleQSub0(new TH1F(*a.fHistAngleQSub0)),
  fHistAngleQSub1(new TH1F(*a.fHistAngleQSub1)), 
  fHarmonic(new TProfile(*a.fHarmonic)),
  fRefMultVsNoOfRPs(new TProfile(*a.fRefMultVsNoOfRPs)),
  fHistList(NULL)
{
  // copy constructor

  fHistList = new TList();
  fHistList-> Add(fHistMultRP);        
  fHistList-> Add(fHistMultPOI);       
  fHistList-> Add(fHistPtRP);          
  fHistList-> Add(fHistPtPOI);
  fHistList-> Add(fHistPtSub0);
  fHistList-> Add(fHistPtSub1);
  fHistList-> Add(fHistPhiRP);          
  fHistList-> Add(fHistPhiPOI);
  fHistList-> Add(fHistPhiSub0);
  fHistList-> Add(fHistPhiSub1);    
  fHistList-> Add(fHistEtaRP);          
  fHistList-> Add(fHistEtaPOI); 
  fHistList-> Add(fHistEtaSub0);
  fHistList-> Add(fHistEtaSub1);
  fHistList-> Add(fHistPhiEtaRP);
  fHistList-> Add(fHistPhiEtaPOI);
  fHistList-> Add(fHistProMeanPtperBin); 
  fHistList-> Add(fHarmonic);  
  fHistList-> Add(fRefMultVsNoOfRPs);
  fHistList-> Add(fHistQ); 
  fHistList-> Add(fHistAngleQ);
  fHistList-> Add(fHistAngleQSub0);
  fHistList-> Add(fHistAngleQSub1);
  //  TListIter next = TListIter(a.fHistList);

}


//-----------------------------------------------------------------------

  AliFlowCommonHist::AliFlowCommonHist(const char *anInput,const char *title):
    TNamed(anInput,title),
    fHistMultRP(NULL),
    fHistMultPOI(NULL),
    fHistPtRP(NULL),
    fHistPtPOI(NULL),
    fHistPtSub0(NULL),
    fHistPtSub1(NULL),
    fHistPhiRP(NULL),
    fHistPhiPOI(NULL),
    fHistPhiSub0(NULL),
    fHistPhiSub1(NULL),
    fHistEtaRP(NULL),
    fHistEtaPOI(NULL),
    fHistEtaSub0(NULL),
    fHistEtaSub1(NULL),
    fHistPhiEtaRP(NULL),
    fHistPhiEtaPOI(NULL),
    fHistProMeanPtperBin(NULL),
    fHistQ(NULL),
    fHistAngleQ(NULL),
    fHistAngleQSub0(NULL),
    fHistAngleQSub1(NULL), 
    fHarmonic(NULL),
    fRefMultVsNoOfRPs(NULL),
    fHistList(NULL)
{

  //constructor creating histograms 
  Int_t iNbinsMult = AliFlowCommonConstants::GetMaster()->GetNbinsMult();
  Int_t iNbinsPt = AliFlowCommonConstants::GetMaster()->GetNbinsPt();
  Int_t iNbinsPhi = AliFlowCommonConstants::GetMaster()->GetNbinsPhi();
  Int_t iNbinsEta = AliFlowCommonConstants::GetMaster()->GetNbinsEta();
  Int_t iNbinsQ = AliFlowCommonConstants::GetMaster()->GetNbinsQ();
  TString sName;

  Double_t  dMultMin = AliFlowCommonConstants::GetMaster()->GetMultMin();            
  Double_t  dMultMax = AliFlowCommonConstants::GetMaster()->GetMultMax();
  Double_t  dPtMin = AliFlowCommonConstants::GetMaster()->GetPtMin();	     
  Double_t  dPtMax = AliFlowCommonConstants::GetMaster()->GetPtMax();
  Double_t  dPhiMin = AliFlowCommonConstants::GetMaster()->GetPhiMin();	     
  Double_t  dPhiMax = AliFlowCommonConstants::GetMaster()->GetPhiMax();
  Double_t  dEtaMin = AliFlowCommonConstants::GetMaster()->GetEtaMin();	     
  Double_t  dEtaMax = AliFlowCommonConstants::GetMaster()->GetEtaMax();	     
  Double_t  dQMin = AliFlowCommonConstants::GetMaster()->GetQMin();	     
  Double_t  dQMax = AliFlowCommonConstants::GetMaster()->GetQMax();	
  
  cout<<"The settings for the common histograms are as follows:"<<endl;
  cout<<"Multiplicity: "<<iNbinsMult<<" bins between "<<dMultMin<<" and "<<dMultMax<<endl;
  cout<<"Pt: "<<iNbinsPt<<" bins between "<<dPtMin<<" and "<<dPtMax<<endl;
  cout<<"Phi: "<<iNbinsPhi<<" bins between "<<dPhiMin<<" and "<<dPhiMax<<endl;
  cout<<"Eta: "<<iNbinsEta<<" bins between "<<dEtaMin<<" and "<<dEtaMax<<endl;
  cout<<"Q: "<<iNbinsQ<<" bins between "<<dQMin<<" and "<<dQMax<<endl;

  //Multiplicity
  sName = "Control_Flow_MultRP_";
  sName +=anInput;
  fHistMultRP = new TH1F(sName.Data(), sName.Data(),iNbinsMult, dMultMin, dMultMax);
  fHistMultRP ->SetXTitle("Multiplicity for RP selection");
  fHistMultRP ->SetYTitle("Counts");

  sName = "Control_Flow_MultPOI_";
  sName +=anInput;
  fHistMultPOI = new TH1F(sName.Data(), sName.Data(),iNbinsMult, dMultMin, dMultMax);
  fHistMultPOI ->SetXTitle("Multiplicity for POI selection");
  fHistMultPOI ->SetYTitle("Counts");

  //Pt
  sName = "Control_Flow_PtRP_";
  sName +=anInput;
  fHistPtRP = new TH1F(sName.Data(), sName.Data(),iNbinsPt, dPtMin, dPtMax); 
  fHistPtRP ->SetXTitle("P_{t} (GeV/c) for RP selection");
  fHistPtRP ->SetYTitle("Counts");

  sName = "Control_Flow_PtPOI_";
  sName +=anInput;
  fHistPtPOI = new TH1F(sName.Data(), sName.Data(),iNbinsPt, dPtMin, dPtMax); 
  //binning has to be the same as for fHistProVPt! use to get Nprime!
  fHistPtPOI ->SetXTitle("P_{t} (GeV/c) for POI selection");
  fHistPtPOI ->SetYTitle("Counts");

  sName = "Control_Flow_PtSub0_";
  sName +=anInput;
  fHistPtSub0 = new TH1F(sName.Data(), sName.Data(),iNbinsPt, dPtMin, dPtMax); 
  fHistPtSub0 ->SetXTitle("P_{t} (GeV/c) for Subevent 0 selection");
  fHistPtSub0 ->SetYTitle("Counts");

  sName = "Control_Flow_PtSub1_";
  sName +=anInput;
  fHistPtSub1 = new TH1F(sName.Data(), sName.Data(),iNbinsPt, dPtMin, dPtMax); 
  fHistPtSub1 ->SetXTitle("P_{t} (GeV/c) for Subevent 1 selection");
  fHistPtSub1 ->SetYTitle("Counts");

  //Phi
  sName = "Control_Flow_PhiRP_";
  sName +=anInput;
  fHistPhiRP = new TH1F(sName.Data(), sName.Data(),iNbinsPhi, dPhiMin, dPhiMax);
  fHistPhiRP ->SetXTitle("#phi for RP selection");
  fHistPhiRP ->SetYTitle("Counts");

  sName = "Control_Flow_PhiPOI_";
  sName +=anInput;
  fHistPhiPOI = new TH1F(sName.Data(), sName.Data(),iNbinsPhi, dPhiMin, dPhiMax);
  fHistPhiPOI ->SetXTitle("#phi for POI selection");
  fHistPhiPOI ->SetYTitle("Counts");

  sName = "Control_Flow_PhiSub0_";
  sName +=anInput;
  fHistPhiSub0 = new TH1F(sName.Data(), sName.Data(),iNbinsPhi, dPhiMin, dPhiMax);
  fHistPhiSub0 ->SetXTitle("#phi for Subevent 0 selection");
  fHistPhiSub0 ->SetYTitle("Counts");

  sName = "Control_Flow_PhiSub1_";
  sName +=anInput;
  fHistPhiSub1 = new TH1F(sName.Data(), sName.Data(),iNbinsPhi, dPhiMin, dPhiMax);
  fHistPhiSub1 ->SetXTitle("#phi for Subevent 1 selection");
  fHistPhiSub1 ->SetYTitle("Counts");

  //Eta
  sName = "Control_Flow_EtaRP_";
  sName +=anInput;
  fHistEtaRP = new TH1F(sName.Data(), sName.Data(),iNbinsEta, dEtaMin, dEtaMax);
  fHistEtaRP ->SetXTitle("#eta for RP selection");
  fHistEtaRP ->SetYTitle("Counts");

  sName = "Control_Flow_EtaPOI_";
  sName +=anInput;
  fHistEtaPOI = new TH1F(sName.Data(), sName.Data(),iNbinsEta, dEtaMin, dEtaMax);
  fHistEtaPOI ->SetXTitle("#eta for POI selection");
  fHistEtaPOI ->SetYTitle("Counts");

  sName = "Control_Flow_EtaSub0_";
  sName +=anInput;
  fHistEtaSub0 = new TH1F(sName.Data(), sName.Data(),iNbinsEta, dEtaMin, dEtaMax);
  fHistEtaSub0 ->SetXTitle("#eta for Subevent 0 selection");
  fHistEtaSub0 ->SetYTitle("Counts");

  sName = "Control_Flow_EtaSub1_";
  sName +=anInput;
  fHistEtaSub1 = new TH1F(sName.Data(), sName.Data(),iNbinsEta, dEtaMin, dEtaMax);
  fHistEtaSub1 ->SetXTitle("#eta for Subevent 1 selection");
  fHistEtaSub1 ->SetYTitle("Counts");

  //Phi vs Eta
  sName = "Control_Flow_PhiEtaRP_";
  sName +=anInput;
  fHistPhiEtaRP = new TH2F(sName.Data(), sName.Data(),iNbinsEta, dEtaMin, dEtaMax, iNbinsPhi, dPhiMin, dPhiMax);
  fHistPhiEtaRP ->SetXTitle("#eta");
  fHistPhiEtaRP ->SetYTitle("#phi");

  sName = "Control_Flow_PhiEtaPOI_";
  sName +=anInput;
  fHistPhiEtaPOI = new TH2F(sName.Data(), sName.Data(),iNbinsEta, dEtaMin, dEtaMax, iNbinsPhi, dPhiMin, dPhiMax);
  fHistPhiEtaPOI ->SetXTitle("#eta");
  fHistPhiEtaPOI ->SetYTitle("#phi");

  //Mean Pt per pt bin 
  sName = "Control_FlowPro_MeanPtperBin_";
  sName +=anInput;
  fHistProMeanPtperBin = new TProfile(sName.Data(), sName.Data(),iNbinsPt,dPtMin,dPtMax);
  fHistProMeanPtperBin ->SetXTitle("P_{t} (GeV/c) ");
  fHistProMeanPtperBin ->SetYTitle("<P_{t}> (GeV/c) ");

  //Q vector
  sName = "Control_Flow_Q_";
  sName +=anInput;
  fHistQ = new TH1F(sName.Data(), sName.Data(),iNbinsQ, dQMin, dQMax);
  fHistQ ->SetXTitle("Q_{vector}/Mult");
  fHistQ ->SetYTitle("Counts");  
  
  //Angle of Q vector
  sName = "Control_Flow_AngleQ_";
  sName +=anInput;
  fHistAngleQ = new TH1F(sName.Data(), sName.Data(),72, 0., TMath::Pi());
  fHistAngleQ ->SetXTitle("Angle of Q_{vector}");
  fHistAngleQ ->SetYTitle("Counts"); 
 
  sName = "Control_Flow_AngleQSub0_";
  sName +=anInput;
  fHistAngleQSub0 = new TH1F(sName.Data(), sName.Data(),72, 0., TMath::Pi());
  fHistAngleQSub0 ->SetXTitle("Angle of Q_{vector} for Subevent 0");
  fHistAngleQSub0 ->SetYTitle("Counts"); 

  sName = "Control_Flow_AngleQSub1_";
  sName +=anInput;
  fHistAngleQSub1 = new TH1F(sName.Data(), sName.Data(),72, 0., TMath::Pi());
  fHistAngleQSub1 ->SetXTitle("Angle of Q_{vector} for Subevent 1");
  fHistAngleQSub1 ->SetYTitle("Counts"); 

  //harmonic
  sName = "Control_Flow_Harmonic_";
  sName +=anInput;
  fHarmonic = new TProfile(sName.Data(),sName.Data(),1,0,1);
  fHarmonic ->SetYTitle("harmonic");
  
  //<reference multiplicity> versus # of RPs
  sName = "Reference_Multiplicity_Vs_Number_Of_RPs_";
  sName +=anInput;
  fRefMultVsNoOfRPs = new TProfile(sName.Data(),sName.Data(),iNbinsMult, dMultMin, dMultMax);
  fRefMultVsNoOfRPs->SetYTitle("<reference multiplicity>");
  fRefMultVsNoOfRPs->SetXTitle("# of RPs");

  //list of histograms if added here also add in copy constructor
  fHistList = new TList();
  fHistList-> Add(fHistMultRP);        
  fHistList-> Add(fHistMultPOI);       
  fHistList-> Add(fHistPtRP);          
  fHistList-> Add(fHistPtPOI); 
  fHistList-> Add(fHistPtSub0);
  fHistList-> Add(fHistPtSub1);
  fHistList-> Add(fHistPhiRP);          
  fHistList-> Add(fHistPhiPOI);
  fHistList-> Add(fHistPhiSub0);
  fHistList-> Add(fHistPhiSub1);
  fHistList-> Add(fHistEtaRP);          
  fHistList-> Add(fHistEtaPOI); 
  fHistList-> Add(fHistEtaSub0); 
  fHistList-> Add(fHistEtaSub1);
  fHistList-> Add(fHistPhiEtaRP);  
  fHistList-> Add(fHistPhiEtaPOI);
  fHistList-> Add(fHistProMeanPtperBin);
  fHistList-> Add(fHarmonic); 
  fHistList-> Add(fRefMultVsNoOfRPs); 
  fHistList-> Add(fHistQ);           
  fHistList-> Add(fHistAngleQ);
  fHistList-> Add(fHistAngleQSub0);
  fHistList-> Add(fHistAngleQSub1); 

}


//----------------------------------------------------------------------- 

AliFlowCommonHist::~AliFlowCommonHist()
{
  //deletes histograms
  delete fHistMultRP;       
  delete fHistMultPOI;      
  delete fHistPtRP;         
  delete fHistPtPOI;
  delete fHistPtSub0;
  delete fHistPtSub1;
  delete fHistPhiRP;        
  delete fHistPhiPOI;
  delete fHistPhiSub0;
  delete fHistPhiSub1;
  delete fHistEtaRP;        
  delete fHistEtaPOI;
  delete fHistEtaSub0;
  delete fHistEtaSub1;
  delete fHistPhiEtaRP;
  delete fHistPhiEtaPOI;
  delete fHistProMeanPtperBin;
  delete fHistQ;
  delete fHistAngleQ;
  delete fHistAngleQSub0;
  delete fHistAngleQSub1;
  delete fHarmonic;
  delete fRefMultVsNoOfRPs;
  delete fHistList;
}


//----------------------------------------------------------------------- 

Bool_t AliFlowCommonHist::FillControlHistograms(AliFlowEventSimple* anEvent)
{
  //Fills the control histograms
  if (!anEvent){
    cout<<"##### FillControlHistograms: FlowEvent pointer null"<<endl;
    return kFALSE;
  }

  Double_t dPt, dPhi, dEta, dWeight;

  //fill the histograms
  AliFlowVector vQ = anEvent->GetQ(); 
  //weight by the Multiplicity
  Double_t dQX = 0.;
  Double_t dQY = 0.;
  if (vQ.GetMult()!=0) {
    dQX = vQ.X()/vQ.GetMult();
    dQY = vQ.Y()/vQ.GetMult();
  }
  vQ.Set(dQX,dQY);
  fHistQ->Fill(vQ.Mod());
  fHistAngleQ->Fill(vQ.Phi()/2);

  AliFlowVector* vQSub = new AliFlowVector[2];
  anEvent->Get2Qsub(vQSub);
  AliFlowVector vQa = vQSub[0];
  AliFlowVector vQb = vQSub[1];
  fHistAngleQSub0->Fill(vQa.Phi()/2);
  fHistAngleQSub1->Fill(vQb.Phi()/2);

  Double_t dMultRP = 0.;
  Double_t dMultPOI = 0.;
  
  Int_t iNumberOfTracks = anEvent->NumberOfTracks();
  AliFlowTrackSimple* pTrack = NULL;     

  for (Int_t i=0;i<iNumberOfTracks;i++) {
    pTrack = anEvent->GetTrack(i);
    if (pTrack ) {
      dWeight = pTrack->Weight();
      if (pTrack->InRPSelection()){
	//pt
	dPt = pTrack->Pt();
	fHistPtRP->Fill(dPt,dWeight);
	//phi
	dPhi = pTrack->Phi();
	if (dPhi<0.) dPhi+=2*TMath::Pi();
	fHistPhiRP->Fill(dPhi,dWeight);
	//eta
	dEta = pTrack->Eta();
	fHistEtaRP->Fill(dEta,dWeight);
	//eta vs phi
	fHistPhiEtaRP->Fill(dEta,dPhi,dWeight);
	//count
	dMultRP += dWeight;
	if (pTrack->InSubevent(0)){
	  //Fill distributions for the subevent
	  fHistPtSub0 -> Fill(dPt,dWeight);
	  fHistPhiSub0 -> Fill(dPhi,dWeight);
	  fHistEtaSub0 -> Fill(dEta,dWeight);
	} 
	else if (pTrack->InSubevent(1)){
	  //Fill distributions for the subevent
	  fHistPtSub1 -> Fill(dPt,dWeight);
	  fHistPhiSub1 -> Fill(dPhi,dWeight);
	  fHistEtaSub1 -> Fill(dEta,dWeight);
	} 
      }
      if (pTrack->InPOISelection()){
	//pt
	dPt = pTrack->Pt();
	fHistPtPOI->Fill(dPt,dWeight);
	//phi
	dPhi = pTrack->Phi();
	if (dPhi<0.) dPhi+=2*TMath::Pi();
	fHistPhiPOI->Fill(dPhi,dWeight);
	//eta
	dEta = pTrack->Eta();
	fHistEtaPOI->Fill(dEta,dWeight);
	//eta vs phi
	fHistPhiEtaPOI->Fill(dEta,dPhi,dWeight);
	//mean pt
	fHistProMeanPtperBin->Fill(dPt,dPt,dWeight);
	//count
	dMultPOI += dWeight;
      }
    } //track
  } //loop over tracks
  
  fHistMultRP->Fill(dMultRP);
  fHistMultPOI->Fill(dMultPOI);
  
  //<reference multiplicity> versus # of RPs:
  fRefMultVsNoOfRPs->Fill(dMultRP+0.5,anEvent->GetReferenceMultiplicity(),1.);

  return kTRUE; 
}

//----------------------------------------------------------------------- 

Double_t AliFlowCommonHist::GetEntriesInPtBinRP(Int_t aBin)
{
  //get entries in bin aBin from fHistPtRP
  Double_t dEntries = fHistPtRP->GetBinContent(aBin);

  return dEntries;

}

//----------------------------------------------------------------------- 

Double_t AliFlowCommonHist::GetEntriesInPtBinPOI(Int_t aBin)
{
  //get entries in bin aBin from fHistPtPOI
  Double_t dEntries = fHistPtPOI->GetBinContent(aBin);

  return dEntries;

}

//----------------------------------------------------------------------- 

Double_t AliFlowCommonHist::GetEntriesInEtaBinRP(Int_t aBin)
{
  //get entries in bin aBin from fHistPtRP
  Double_t dEntries = fHistEtaRP->GetBinContent(aBin);

  return dEntries;

}

//----------------------------------------------------------------------- 

Double_t AliFlowCommonHist::GetEntriesInEtaBinPOI(Int_t aBin)
{
  //get entries in bin aBin from fHistPtPOI
  Double_t dEntries = fHistEtaPOI->GetBinContent(aBin);

  return dEntries;

}

//----------------------------------------------------------------------- 

Double_t AliFlowCommonHist::GetMeanPt(Int_t aBin)
{  
  //Get entry from bin aBin from fHistProMeanPtperBin
  Double_t dMeanPt = fHistProMeanPtperBin->GetBinContent(aBin);

  return dMeanPt;
  
}


//----------------------------------------------------------------------- 
 Double_t AliFlowCommonHist::Merge(TCollection *aList)
{
  //merge fuction
  //cout<<"entering merge function"<<endl;
  if (!aList) return 0;
  if (aList->IsEmpty()) return 0; //no merging is needed

  Int_t iCount = 0;
  TIter next(aList); // list is supposed to contain only objects of the same type as this
  AliFlowCommonHist *toMerge;
  // make a temporary list
  TList *pTemp = new TList();
  while ((toMerge=(AliFlowCommonHist*)next())) {
    pTemp->Add(toMerge->GetHistList()); 
    iCount++;
  }
  // Now call merge for fHistList providing temp list
  fHistList->Merge(pTemp);
  // Cleanup
  delete pTemp;
    
  //cout<<"Merged"<<endl;
  return (double)iCount;
    
}

void AliFlowCommonHist::Print(Option_t *option) const
{
  //   -*-*-*-*-*Print some global quantities for this histogram collection class *-*-*-*-*-*-*-*
  //             ===============================================
  //   printf( "TH1.Print Name  = %s, Entries= %d, Total sum= %g\n",GetName(),Int_t(fEntries),GetSumOfWeights());
  printf( "Class.Print Name = %s, Histogram list:\n",GetName());

  if (fHistList) {  
    fHistList->Print(option);
  }
  else
    {
      printf( "Empty histogram list \n");
    }
}

//----------------------------------------------------------------------- 
 void AliFlowCommonHist::Browse(TBrowser *b)
{

  if (!b) return;
  if (fHistList) b->Add(fHistList,"AliFlowCommonHistList");
}




