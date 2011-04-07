#include <Riostream.h>

#include <TChain.h>
#include <TH1.h>
#include <TH2.h>
#include <TDatabasePDG.h>
#include <TParticlePDG.h>
#include <TParticle.h>
#include <TLorentzVector.h>

#include "AliESDpid.h"
#include "AliESDtrack.h"
#include "AliESDEvent.h"
#include "AliESDVertex.h"
#include "AliESDtrackCuts.h"

#include "AliPID.h"
#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"
#include "AliInputEventHandler.h"

#include "AliMCEvent.h"
#include "AliStack.h"
#include "AliMCParticle.h"
#include "AliGenEventHeader.h"

#include "AliAnalysisTaskResonanceQA.h"

// analysis task creating basic QA plots for resonance particles
// Author: Ayben Karasu Uysal


ClassImp(AliAnalysisTaskResonanceQA)

//________________________________________________________________________
AliAnalysisTaskResonanceQA::AliAnalysisTaskResonanceQA(const char *name) :
   AliAnalysisTaskSE(name), 
   fT0(AliESDpid::kTOF_T0),
   fPrimaryThr(1E-6),
   fVz(10.0),
   fOutputList(0),
   fSelectedEvts(0),
   fdEdxTPC(0),
   fdEdxITS(0),
   fTOFpid(0),
   fDCAXYvsPtBeforeCuts(0), 
   fDCAZvsPtBeforeCuts(0),
   fNClusterPtBeforeCuts(0), 
   fNFindableClusterPtBeforeCuts(0), 
   fNCrossedRowsTPCPtBeforeCuts(0),
   fProducedParticles(0), 
   fESD(0), 
   fESDpid(0),      
   fTrackCuts(0)
{
//   
// Constructor
//

   // Define input and output slots here
   // Input slot #0 works with a TChain
   DefineInput(0, TChain::Class());
   
   // Output slot #0 id reserved by the base class for AOD
   // Output slot #1 writes into a TH1 container
   DefineOutput(1, TList::Class());
   
   // setup to NULL all remaining histos
   Int_t i;
   for (i = 0; i < kResonances; i++) fRsnYPt[0][i] = fRsnYPt[1][i] = 0;
}

//________________________________________________________________________
const char* AliAnalysisTaskResonanceQA::RsnName(ERsn type)
{
//
// Return a short string with name of resonance
//

   switch(type) {
      case kPhi       : return "Phi";
      case kKStar0    : return "KStar0";
      case kRho       : return "Rho";
      case kLambdaStar: return "LambdaStar";
      case kXiStar0   : return "XiStar0";
      case kXiStarM   : return "XiStarM";
      case kSigmaStarP: return "SigmaStarP";
      case kSigmaStar0: return "SigmaStar0";
      case kSigmaStarM: return "SigmaStarM";
      case kDeltaPP   : return "DeltaPP";
      default         : return "Unknown";
   }
}

//________________________________________________________________________
const char* AliAnalysisTaskResonanceQA::RsnSymbol(ERsn type)
{
//
// Return a short string with name of resonance
//

   switch(type) {
      case kPhi       : return "#phi";
      case kKStar0    : return "K^{*0}";
      case kRho       : return "#rho";
      case kLambdaStar: return "#Lambda^{*}";
      case kXiStar0   : return "#Xi^{*0}";
      case kXiStarM   : return "#Xi^{*-}";
      case kSigmaStarP: return "#Sigma^{*+}";
      case kSigmaStar0: return "#Sigma^{*0}";
      case kSigmaStarM: return "#Sigma^{*-}";
      case kDeltaPP   : return "#Delta^{++}";
      default         : return "Unknown";
   }
}

//________________________________________________________________________
Int_t AliAnalysisTaskResonanceQA::RsnPDG(ERsn type)
{
//
// Return PDG code of resonance
//

   switch(type) {
      case kPhi       : return  333;
      case kKStar0    : return  313;
      case kRho       : return  113;
      case kLambdaStar: return 3124;
      case kXiStar0   : return 3324;
      case kXiStarM   : return 3314;
      case kSigmaStarP: return 3224;
      case kSigmaStar0: return 3214;
      case kSigmaStarM: return 3114;
      case kDeltaPP   : return 2224;
      default         : return    0;
   }
}

//________________________________________________________________________
void AliAnalysisTaskResonanceQA::UserCreateOutputObjects()
{
//
// Create histograms (called once)
//

   Int_t i;
   Int_t ESDdEdxBin = 1000;  Double_t ESDdEdxMin = 0;  Double_t ESDdEdxMax = 1000;
   Int_t ESDQPBin = 900;     Double_t ESDQPMin = -4.5; Double_t ESDQPMax = 4.5;
   Int_t PtBin = 500;        Double_t PtMin = 0.;      Double_t PtMax = 10.;
   Int_t YBin = 600;         Double_t YMin = -15.;     Double_t YMax = 15.;

   // standard objects
   fESDpid = new AliESDpid();
   fTrackCuts = AliESDtrackCuts::GetStandardITSTPCTrackCuts2010(kTRUE);
   fTrackCuts->SetPtRange(0.1, 1E20);
   fTrackCuts->SetEtaRange(-0.9, 0.9);

   // create output list
   fOutputList = new TList();
   fOutputList->SetOwner();

   // output histograms for PID signal
   fSelectedEvts = new TH1I("fSelectedEvts", "fSelectedEvts;fSelectedEvts", 3, 0, 3);
   fdEdxTPC = new TH2F("fdEdxTPC", "dEdxTPC, After all cuts;chargexmomentum p (GeV/c); TPC signal (a.u.)", ESDQPBin, ESDQPMin, ESDQPMax, ESDdEdxBin, ESDdEdxMin, ESDdEdxMax);
   fdEdxITS = new TH2F("fdEdxITS", "dEdxITS, After all cuts;chargexmomentum p (GeV/c); TPC signal (a.u.)", ESDQPBin, ESDQPMin, ESDQPMax, ESDdEdxBin, ESDdEdxMin, ESDdEdxMax);
   fTOFpid = new TH2F("fTOFpid", "TOF PID;p (GeV/c);#beta;", ESDQPBin, ESDQPMin, ESDQPMax, 300, 0.1, 1.2);
   
   // output histograms for track quality
   fDCAXYvsPtBeforeCuts = new TH2F("DCAXYvsPtBeforeCuts", "DCAXYvsPtBeforeCuts;p_{t} (GeV/c);DCAXY", PtBin, PtMin, PtMax, 500, -10.0, 10.0);
   fDCAZvsPtBeforeCuts = new TH2F("DCAZvsPtBeforeCuts", "DCAZvsPtBeforeCuts;p_{t} (GeV/c);DCAZ", PtBin, PtMin, PtMax, 500, -10.0, 10.0);
   fNClusterPtBeforeCuts = new TH2F("NClusterPtBeforeCuts", "NClusterPtBeforeCuts;p_{t} (GeV/c);NClusters", PtBin, PtMin, PtMax, 180, 0., 180.);
   fNFindableClusterPtBeforeCuts = new TH2F("NFindableClusterPtBeforeCuts", "NFindableClusterPtBeforeCuts;p_{t} (GeV/c);NClusters", PtBin, PtMin, PtMax, 180, 0., 180.);
   fNCrossedRowsTPCPtBeforeCuts = new TH2F("NCrossedRowsTPCPtBeforeCuts", "NCrossedRowsTPCPtBeforeCuts;p_{t} (GeV/c);NCrossedRowsTPC", PtBin, PtMin, PtMax, 180, 0., 180.);
   
   // output histograms for resonances
   fProducedParticles = new TH1I("ProducedParticles", "ProducedParticles;Produced Particles", kResonances, 0, kResonances);
   for (i = 0; i < kResonances; i++) {
      fRsnYPt[0][i] = new TH2F(Form("%s_all", RsnName(i)), Form("%s -- ALL;p_{t} (GeV/c);Rapidity", RsnName(i)), PtBin, PtMin, PtMax, YBin, YMin, YMax);
      fRsnYPt[1][i] = new TH2F(Form("%s_prim", RsnName(i)), Form("%s -- PRIMARY;p_{t} (GeV/c);Rapidity", RsnName(i)), PtBin, PtMin, PtMax, YBin, YMin, YMax);
      fProducedParticles->GetXaxis()->SetBinLabel(i + 1, RsnSymbol(i));
   }
   
   // add histogram to list
   fOutputList->Add(fSelectedEvts);
   fOutputList->Add(fdEdxTPC);
   fOutputList->Add(fdEdxITS);
   fOutputList->Add(fTOFpid);
   fOutputList->Add(fDCAXYvsPtBeforeCuts);
   fOutputList->Add(fDCAZvsPtBeforeCuts);
   fOutputList->Add(fNClusterPtBeforeCuts);
   fOutputList->Add(fNFindableClusterPtBeforeCuts);
   fOutputList->Add(fNCrossedRowsTPCPtBeforeCuts);
   fOutputList->Add(fProducedParticles);
   for (i = 0; i < kResonances; i++) {
      fOutputList->Add(fRsnYPt[0][i]);
      fOutputList->Add(fRsnYPt[1][i]);
   }
   
   PostData(1, fOutputList);
}

//________________________________________________________________________
void AliAnalysisTaskResonanceQA::UserExec(Option_t *)
{
//
// Execute computations
//

   // first bin of this histogram counts processed events
   // second bin counts events passing physics selection
   // all events not passing physics selections are skipped
   fSelectedEvts->Fill(0.1);
   Bool_t isSelected = (((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected() & AliVEvent::kMB);
   if (!isSelected) return;
   
   // count selected events
   fSelectedEvts->Fill(1.1);
   
   // loop on MC, if MC event and stack are available
   AliMCEvent *mcEvent = MCEvent();
   if (mcEvent) {
      // MC primary vertex
      TArrayF mcVertex(3);
      AliGenEventHeader *genEH = mcEvent->GenEventHeader();
      genEH->PrimaryVertex(mcVertex);
      // loop on particles
      AliStack *stack = mcEvent->Stack();
      if (stack) {
         Int_t iTrack, irsn, pdg, mother, motherPDG, nTracks = stack->GetNtrack();
         Double_t dx, dy, dz, dist;
         TLorentzVector vprod;
         for (iTrack = 0; iTrack < nTracks; iTrack++) {
            
            // get particle
            TParticle *part = stack->Particle(iTrack);
            if (!part) {
               AliError(Form("Could not receive track %d", iTrack));
               continue;
            }
            
            // get PDG code of particle and check physical primary
            pdg = part->GetPdgCode();
            mother = part->GetFirstMother();
            motherPDG = 0;
                        
            // loop over possible resonances
            for (irsn = 0; irsn < kResonances; irsn++) {
               if (TMath::Abs(pdg) == RsnPDG(irsn)) {
                  // debug check
                  if (mother >= 0) {
                     TParticle *p = stack->Particle(mother);
                     motherPDG = p->GetPdgCode();
                  }
                  part->ProductionVertex(vprod);
                  dx = mcVertex[0] - vprod.X();
                  dy = mcVertex[1] - vprod.Y();
                  dz = mcVertex[2] - vprod.Z();
                  dist = TMath::Sqrt(dx*dx + dy*dy + dz*dz);
                  AliDebugClass(1, Form("PDG = %d -- mother ID = %d, pdg = %d -- dist. from MC vertex = %f -- prod. time = %f", pdg, mother, motherPDG, dist, vprod.T())); 
                  // global counter is always filled
                  fRsnYPt[0][irsn]->Fill(part->Pt(), part->Y());
                  // counter for primaries is filled only if mother is less than 0 (primary)
                  if (dist < fPrimaryThr) fRsnYPt[1][irsn]->Fill(part->Pt(), part->Y());
                  // fill the global histogram
                  fProducedParticles->Fill(irsn + 1);
                  // if one PDG match was found, no need to continue loop
                  break;
               }
            }
         }
      }
   }
   
   // try to cast input event to ESD and loop on tracks
   fESD = dynamic_cast<AliESDEvent*>(InputEvent());
   if (fESD) {
      // check primary vertex:
      // we accept only tracks/SPD with at least 1 contributor
      const AliESDVertex *v0 = fESD->GetPrimaryVertexTracks();
      if (!v0) return;
      if (v0->GetNContributors() < 1) {
         v0 = fESD->GetPrimaryVertexSPD();
         if (!v0) return;
         if (v0->GetNContributors() < 1) return;
      }
      if (!v0) return;
      if (TMath::Abs(v0->GetZv()) > fVz) return;
   
      // settings for TOF time zero
      if (fESD->GetTOFHeader()) 
         fESDpid->SetTOFResponse(fESD, AliESDpid::kTOF_T0);
      else {
         Float_t t0spread[10];
         Float_t intrinsicTOFres = 100;
         for (Int_t i = 0; i < 10; i++) t0spread[i] = (TMath::Sqrt(fESD->GetSigma2DiamondZ())) / 0.03; //0.03 to convert from cm to ps
         fESDpid->GetTOFResponse().SetT0resolution(t0spread);
         fESDpid->GetTOFResponse().SetTimeResolution(intrinsicTOFres);
         fESDpid->SetTOFResponse(fESD, fT0);
      }
      fESDpid->MakePID(fESD, kFALSE, 0.);

      // use some variables to store useful info
      /*MISC   */ Int_t    iTrack, nTracks = fESD->GetNumberOfTracks();
      /*MISC   */ Double_t pt;
      /*QUALITY*/ Float_t  impactXY, impactZ, nCrossedRowsTPC;
      /*ITS    */ Double_t itsSignal = 0;
      /*TPC    */ Double_t ptotSE = 0, tpcSignal, signSE;
      /*TOF    */ Double_t momentum, length, time, beta; //, times[AliPID::kSPECIES], tzeroTrack = 0;

      // loop on tracks
      for (iTrack = 0; iTrack < nTracks; iTrack++) {
         
         // get track
         AliESDtrack* track = fESD->GetTrack(iTrack);
         if (!track) continue;
         
         // get transverse momentum (used for quality histograms)
         pt = track->Pt();
         
         // get charge and reject neutrals
         signSE = track->GetSign();
         if (signSE == 0) continue;

         // get DCA and TPC-related quality params
         track->GetImpactParameters(impactXY, impactZ);
         nCrossedRowsTPC = track->GetTPCClusterInfo(2, 1);

         // fill quality histograms when status flags are OK for TPC in+refit and ITS refit
         if (track->IsOn(AliESDtrack::kTPCin) && track->IsOn(AliESDtrack::kTPCrefit) && track->IsOn(AliESDtrack::kITSrefit)) {
            fNClusterPtBeforeCuts        ->Fill(pt, track->GetTPCNcls());
            fNFindableClusterPtBeforeCuts->Fill(pt, track->GetTPCNclsF());
            fNCrossedRowsTPCPtBeforeCuts ->Fill(pt, nCrossedRowsTPC);
            fDCAXYvsPtBeforeCuts         ->Fill(pt, impactXY);
            fDCAZvsPtBeforeCuts          ->Fill(pt, impactZ);
         }
         
         // from now on, work only with tracks passing standard cuts for 2010
         if (!fTrackCuts->AcceptTrack(track)) continue;

         // get momentum (used for PID histograms)
         momentum = track->P();

         // ITS
         itsSignal = track->GetITSsignal();
         fdEdxITS->Fill(signSE * momentum, itsSignal);
         
         // TPC
         if (track->GetInnerParam()) {
            AliExternalTrackParam trackIn1(*track->GetInnerParam());
            ptotSE = trackIn1.GetP();
            tpcSignal = track->GetTPCsignal();
            fdEdxTPC->Fill(signSE * ptotSE, tpcSignal);
         } // end TPC
         
         // TOF (requires checking some more status flags
         if (track->IsOn(AliESDtrack::kTOFpid) && track->IsOn(AliESDtrack::kTOFout) && track->IsOn(AliESDtrack::kTIME) && !track->IsOn(AliESDtrack::kTOFmismatch)) {
            length = track->GetIntegratedLength();
            
            // reject suspiciously small lengths/times
            if (track->GetIntegratedLength() < 350.) continue;
            if (track->GetTOFsignal() < 1E-6) continue;

            // thhese are maybe not needed
            // track->GetIntegratedTimes(times);
            // tzeroTrack = fESDpid->GetTOFResponse().GetStartTime(track->P());
            
            // get TOF measured time and compute beta
            time = track->GetTOFsignal() - fESDpid->GetTOFResponse().GetStartTime(momentum);
            beta = length / (2.99792457999999984e-02 * time);
            fTOFpid->Fill(signSE * momentum, beta);
         } // end TOF
      } // end track loop
   } // end ESD check

   // if we arrive here, the event was processed successfully
   // then we fill last bin in first histogram
   fSelectedEvts->Fill(2.1);
   
   PostData(1, fOutputList);
}

//________________________________________________________________________
void AliAnalysisTaskResonanceQA::Terminate(Option_t *)
{
//
// Termination
// Quickly check that the output list is there
//
  
   fOutputList = dynamic_cast<TList*>(GetOutputData(1));
   if (!fOutputList) {
      AliError("Output list not found!!!");
      return;
   }
}
