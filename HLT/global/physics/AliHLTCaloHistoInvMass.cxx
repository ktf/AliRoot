//-*- Mode: C++ -*-
/**************************************************************************
 * This file is property of and copyright by the ALICE HLT Project        * 
 * All rights reserved.                                                   *
 *                                                                        *
 * Primary Authors: Svein Lindal                                          *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          * 
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/** 
 * @file   AliHLTCaloHistoInvMass
 * @author Svein Lindal <slindal@fys.uio.no>
 * @date 
 * @brief  Produces plots of invariant mass of two clusters. 
 */

// see header file for class documentation
// or
// refer to README to build package
// or
// visit http://web.ift.uib.no/~kjeks/doc/alice-hlt

#include "AliHLTCaloHistoInvMass.h"
#include "AliHLTCaloClusterDataStruct.h"
#include "TObjArray.h"
#include "AliESDEvent.h"
#include "TRefArray.h"
#include "TH1F.h"
#include "TString.h"
#include "AliESDCaloCluster.h"
#include "TVector3.h"
#include "TLorentzVector.h"

AliHLTCaloHistoInvMass::AliHLTCaloHistoInvMass(TString det) :
  fHistTwoClusterInvMass(NULL)
{
  // See header file for documentation
  fHistTwoClusterInvMass = new TH1F(Form("%s fHistTwoClusterInvMass", det.Data()), Form("%s Invariant mass of two clusters PHOS", det.Data()), 200, 0, 1);
  fHistTwoClusterInvMass->GetXaxis()->SetTitle("m_{#gamma#gamma} GeV");
  fHistTwoClusterInvMass->GetYaxis()->SetTitle("Number of counts");
  fHistTwoClusterInvMass->SetMarkerStyle(21);
  fHistArray->AddLast(fHistTwoClusterInvMass);
}

AliHLTCaloHistoInvMass::~AliHLTCaloHistoInvMass()
{
  if(fHistTwoClusterInvMass)
    delete fHistTwoClusterInvMass;
  fHistTwoClusterInvMass = NULL;
}


Int_t AliHLTCaloHistoInvMass::FillHistograms(Int_t nc, vector<AliHLTCaloClusterDataStruct*> &cVec) {
  //See header file for documentation
  
  Float_t cPos[nc][3];
  Float_t cEnergy[nc];

  for(int ic = 0; ic < nc; ic++) {
    AliHLTCaloClusterDataStruct * cluster = cVec.at(ic);
    cluster->GetPosition(cPos[ic]);
    cEnergy[ic] = cluster->E();
  }

  for(Int_t ipho = 0; ipho<(nc-1); ipho++) { 
    for(Int_t jpho = ipho+1; jpho<nc; jpho++) { 
      
      // Calculate the theta angle between two photons
      Double_t theta = (2* asin(0.5*TMath::Sqrt((cPos[ipho][0]-cPos[jpho][0])*(cPos[ipho][0]-cPos[jpho][0]) +(cPos[ipho][1]-cPos[jpho][1])*(cPos[ipho][1]-cPos[jpho][1]))/460));
      
      // Calculate the mass m of the pion candidate
      Double_t m =(TMath::Sqrt(2 * cEnergy[ipho]* cEnergy[jpho]*(1-TMath::Cos(theta))));
      
      fHistTwoClusterInvMass->Fill(m);
    }
  }

  return 0;
}

Int_t AliHLTCaloHistoInvMass::FillHistograms(Int_t nc, TRefArray * clusterArray) {
  //See header file for documentation
  
  Float_t cPos[nc][3];
  Float_t cEnergy[nc];

  for(int ic = 0; ic < nc; ic++) {
    AliESDCaloCluster * cluster = static_cast<AliESDCaloCluster*>(clusterArray->At(ic));
    cluster->GetPosition(cPos[ic]);
    cEnergy[ic] = cluster->E();
  }

  for(Int_t ic = 0; ic<(nc-1); ic++) { 
    
    //Get the Lorentz vector of one photon
    TVector3 iVec(cPos[ic]);
   // iVec.Print();
    iVec = iVec.Unit();
    //iVec.Print();
    iVec = cEnergy[ic] * iVec;
    //iVec.Print();
    TLorentzVector iLorentz(iVec, -1);

    
    for(Int_t jc = ic+1; jc<nc; jc++) { 

      TVector3 jVec(cPos[ic]);
      //jVec.Print();
      jVec = jVec.Unit();
      //jVec.Print();
      jVec = cEnergy[jc] * jVec;
      //jVec.Print();
      TLorentzVector jLor(jVec, -1);

      
      
//       g.SetXYZM(gammaCandidate->GetPx(),gammaCandidate->GetPy(),gammaCandidate->GetPz(),fGammaMass);
//       TLorentzVector xyg = xy + g;


//       TVector3 vec( cPos[ic][0] - cPos[jc][0] , cPos[ic][1] - cPos[jc][2] , cPos[ic][1] - cPos[jc][2] );
      
//       // Calculate the theta angle between two photons
//       Double_t theta = (2* asin(0.5*TMath::Sqrt((cPos[ic][0]-cPos[jc][0])*(cPos[ic][0]-cPos[jc][0]) +(cPos[ic][1]-cPos[jc][1])*(cPos[ic][1]-cPos[jc][1]))/460));
      
      // Calculate the mass m of the pion candidate
      //Double_t m =(TMath::Sqrt(2 * cEnergy[ic]* cEnergy[jc]*(1-TMath::Cos(theta))));

      Double_t m = TMath::Sqrt( 2 *(cEnergy[ic]* cEnergy[jc] - iVec.Dot(jVec) ) );
      
      fHistTwoClusterInvMass->Fill(m);
    }
  }


  return 0;
}
