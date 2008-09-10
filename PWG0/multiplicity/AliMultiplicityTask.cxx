/* $Id$ */

#include "AliMultiplicityTask.h"

#include <TStyle.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <TVector3.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TParticle.h>
#include <TRandom.h>
#include <TNtuple.h>
#include <TObjString.h>
#include <TF1.h>

#include <AliLog.h>
#include <AliESDVertex.h>
#include <AliESDEvent.h>
#include <AliStack.h>
#include <AliHeader.h>
#include <AliGenEventHeader.h>
#include <AliMultiplicity.h>
#include <AliAnalysisManager.h>
#include <AliMCEventHandler.h>
#include <AliMCEvent.h>
#include <AliESDInputHandler.h>

#include "AliESDtrackCuts.h"
#include "AliPWG0Helper.h"
#include "multiplicity/AliMultiplicityCorrection.h"
#include "AliCorrection.h"
#include "AliCorrectionMatrix3D.h"

ClassImp(AliMultiplicityTask)

AliMultiplicityTask::AliMultiplicityTask(const char* opt) :
  AliAnalysisTask("AliMultiplicityTask", ""),
  fESD(0),
  fOption(opt),
  fAnalysisMode(AliPWG0Helper::kSPD),
  fTrigger(AliPWG0Helper::kMB1),
  fReadMC(kFALSE),
  fUseMCVertex(kFALSE),
  fMultiplicity(0),
  fEsdTrackCuts(0),
  fSystSkipParticles(kFALSE),
  fSelectProcessType(0),
  fParticleSpecies(0),
  fPtSpectrum(0),
  fOutput(0)
{
  //
  // Constructor. Initialization of pointers
  //

  for (Int_t i = 0; i<4; i++)
    fParticleCorrection[i] = 0;

  // Define input and output slots here
  DefineInput(0, TChain::Class());
  DefineOutput(0, TList::Class());
}

AliMultiplicityTask::~AliMultiplicityTask()
{
  //
  // Destructor
  //

  // histograms are in the output list and deleted when the output
  // list is deleted by the TSelector dtor

  if (fOutput) {
    delete fOutput;
    fOutput = 0;
  }
}

//________________________________________________________________________
void AliMultiplicityTask::ConnectInputData(Option_t *)
{
  // Connect ESD
  // Called once

  Printf("AliMultiplicityTask::ConnectInputData called");

  TTree* tree = dynamic_cast<TTree*> (GetInputData(0));
  if (!tree) {
    Printf("ERROR: Could not read tree from input slot 0");
  } else {
    // Disable all branches and enable only the needed ones
    tree->SetBranchStatus("*", 0);

    tree->SetBranchStatus("AliESDHeader*", 1);
    tree->SetBranchStatus("*Vertex*", 1);

    if (fAnalysisMode == AliPWG0Helper::kSPD) {
      tree->SetBranchStatus("AliMultiplicity*", 1);
    }

    if (fAnalysisMode == AliPWG0Helper::kTPC || fAnalysisMode == AliPWG0Helper::kTPCITS) {
      //AliESDtrackCuts::EnableNeededBranches(tree);
      tree->SetBranchStatus("Tracks*", 1);
    }

    AliESDInputHandler *esdH = dynamic_cast<AliESDInputHandler*> (AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler());

    if (!esdH) {
      Printf("ERROR: Could not get ESDInputHandler");
    } else
      fESD = esdH->GetEvent();
  }

  // disable info messages of AliMCEvent (per event)
  AliLog::SetClassDebugLevel("AliMCEvent", AliLog::kWarning - AliLog::kDebug + 1);
}

void AliMultiplicityTask::CreateOutputObjects()
{
  // create result objects and add to output list

  fOutput = new TList;
  fOutput->SetOwner();

  fMultiplicity = new AliMultiplicityCorrection("Multiplicity", "Multiplicity");
  fOutput->Add(fMultiplicity);

  if (fOption.Contains("skip-particles"))
  {
    fSystSkipParticles = kTRUE;
    AliInfo("WARNING: Systematic study enabled. Particles will be skipped.");
  }

  if (fOption.Contains("particle-efficiency"))
    for (Int_t i = 0; i<4; i++)
    {
      fParticleCorrection[i] = new AliCorrection(Form("correction_%d", i), Form("correction_%d", i));
      fOutput->Add(fParticleCorrection[i]);
    }

  if (fOption.Contains("only-process-type-nd"))
    fSelectProcessType = 1;

  if (fOption.Contains("only-process-type-sd"))
    fSelectProcessType = 2;

  if (fOption.Contains("only-process-type-dd"))
    fSelectProcessType = 3;

  if (fSelectProcessType != 0)
    AliInfo(Form("WARNING: Systematic study enabled. Only considering process type %d", fSelectProcessType));

  if (fOption.Contains("pt-spectrum-hist"))
  {
    TFile* file = TFile::Open("ptspectrum_fit.root");
    if (file)
    {
      TString subStr(fOption(fOption.Index("pt-spectrum")+17, 3));
      TString histName(Form("ptspectrum_%s", subStr.Data()));
      AliInfo(Form("Pt-Spectrum modification. Using %s.", histName.Data()));
      fPtSpectrum = (TH1*) file->Get(histName);
      if (!fPtSpectrum)
	AliError("Histogram not found");
    }
    else
      AliError("Could not open ptspectrum_fit.root. Pt Spectrum study could not be enabled.");

    if (fPtSpectrum)
      AliInfo("WARNING: Systematic study enabled. Pt spectrum will be modified");
  }

  if (fOption.Contains("pt-spectrum-func"))
  {
    if (fPtSpectrum)
    {
      Printf("Using function from input list for systematic p_t study");
    }
    else
    {
      fPtSpectrum = new TH1F("fPtSpectrum", "fPtSpectrum", 1, 0, 100);
      fPtSpectrum->SetBinContent(1, 1);
    }

    if (fPtSpectrum)
      AliInfo("WARNING: Systematic study enabled. Pt spectrum will be modified");
  }

  if (fOption.Contains("particle-species")) {
    fParticleSpecies = new TNtuple("fParticleSpecies", "fParticleSpecies", "vtx:Pi_True:K_True:p_True:o_True:Pi_Rec:K_Rec:p_Rec:o_Rec:nolabel:doublePrim:doubleCount");
    fOutput->Add(fParticleSpecies);
  }

  // TODO set seed for random generator
}

void AliMultiplicityTask::Exec(Option_t*)
{
  // process the event

  // Check prerequisites
  if (!fESD)
  {
    AliDebug(AliLog::kError, "ESD not available");
    return;
  }

  Bool_t eventTriggered = AliPWG0Helper::IsEventTriggered(fESD->GetTriggerMask(), fTrigger);

  const AliESDVertex* vtxESD = AliPWG0Helper::GetVertex(fESD, fAnalysisMode);
  Bool_t eventVertex = (vtxESD != 0);

  Double_t vtx[3];
  if (vtxESD)
    vtxESD->GetXYZ(vtx);

  // post the data already here
  PostData(0, fOutput);

  //const Float_t kPtCut = 0.3;

  // create list of (label, eta) tuples
  Int_t inputCount = 0;
  Int_t* labelArr = 0;
  Float_t* etaArr = 0;
  if (fAnalysisMode == AliPWG0Helper::kSPD)
  {
    // get tracklets
    const AliMultiplicity* mult = fESD->GetMultiplicity();
    if (!mult)
    {
      AliDebug(AliLog::kError, "AliMultiplicity not available");
      return;
    }

    labelArr = new Int_t[mult->GetNumberOfTracklets()];
    etaArr = new Float_t[mult->GetNumberOfTracklets()];

    // get multiplicity from ITS tracklets
    for (Int_t i=0; i<mult->GetNumberOfTracklets(); ++i)
    {
      //printf("%d %f %f %f\n", i, mult->GetTheta(i), mult->GetPhi(i), mult->GetDeltaPhi(i));

      // This removes non-tracklets in PDC06 data. Very bad solution. New solution is implemented for newer data. Keeping this for compatibility.
      if (mult->GetDeltaPhi(i) < -1000)
        continue;

      etaArr[inputCount] = mult->GetEta(i);
      // TODO add second label array
      labelArr[inputCount] = mult->GetLabel(i, 0);
      ++inputCount;
    }
  }
  else if (fAnalysisMode == AliPWG0Helper::kTPC || fAnalysisMode == AliPWG0Helper::kTPCITS)
  {
    if (!fEsdTrackCuts)
    {
      AliDebug(AliLog::kError, "fESDTrackCuts not available");
      return;
    }

    // get multiplicity from ESD tracks
    TObjArray* list = fEsdTrackCuts->GetAcceptedTracks(fESD, (fAnalysisMode == AliPWG0Helper::kTPC));
    Int_t nGoodTracks = list->GetEntries();

    labelArr = new Int_t[nGoodTracks];
    etaArr = new Float_t[nGoodTracks];

    // loop over esd tracks
    for (Int_t i=0; i<nGoodTracks; i++)
    {
      AliESDtrack* esdTrack = dynamic_cast<AliESDtrack*> (list->At(i));
      if (!esdTrack)
      {
        AliDebug(AliLog::kError, Form("ERROR: Could not retrieve track %d.", i));
        continue;
      }

      etaArr[inputCount] = esdTrack->Eta();
      labelArr[inputCount] = TMath::Abs(esdTrack->GetLabel());
      ++inputCount;
    }

    delete list;
  }

  // eta range for nMCTracksSpecies, nESDTracksSpecies
  Float_t etaRange = -1;
  switch (fAnalysisMode) {
    case AliPWG0Helper::kInvalid: break;
    case AliPWG0Helper::kTPC:
    case AliPWG0Helper::kTPCITS:
    	etaRange = 0.9; break;
    case AliPWG0Helper::kSPD: etaRange = 2.0; break;
  }

  if (!fReadMC) // Processing of ESD information
  {
    if (eventTriggered && eventVertex)
    {
      Int_t nESDTracks05 = 0;
      Int_t nESDTracks09 = 0;
      Int_t nESDTracks15 = 0;
      Int_t nESDTracks20 = 0;

      for (Int_t i=0; i<inputCount; ++i)
      {
        Float_t eta = etaArr[i];

        if (TMath::Abs(eta) < 0.5)
          nESDTracks05++;

        if (TMath::Abs(eta) < 0.9)
          nESDTracks09++;

        if (TMath::Abs(eta) < 1.5)
          nESDTracks15++;

        if (TMath::Abs(eta) < 2.0)
          nESDTracks20++;
      }

      fMultiplicity->FillMeasured(vtx[2], nESDTracks05, nESDTracks09, nESDTracks15, nESDTracks20);
    }
  }
  else if (fReadMC)   // Processing of MC information
  {
    AliMCEventHandler* eventHandler = dynamic_cast<AliMCEventHandler*> (AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler());
    if (!eventHandler) {
      Printf("ERROR: Could not retrieve MC event handler");
      return;
    }

    AliMCEvent* mcEvent = eventHandler->MCEvent();
    if (!mcEvent) {
      Printf("ERROR: Could not retrieve MC event");
      return;
    }

    AliStack* stack = mcEvent->Stack();
    if (!stack)
    {
      AliDebug(AliLog::kError, "Stack not available");
      return;
    }

    AliHeader* header = mcEvent->Header();
    if (!header)
    {
      AliDebug(AliLog::kError, "Header not available");
      return;
    }

    if (fUseMCVertex)
    {
      Printf("WARNING: Replacing vertex by MC vertex. This is for systematical checks only.");
      // get the MC vertex
      AliGenEventHeader* genHeader = header->GenEventHeader();
      TArrayF vtxMC(3);
      genHeader->PrimaryVertex(vtxMC);

      vtx[2] = vtxMC[2];
    }

    Bool_t processEvent = kTRUE;
    if (fSelectProcessType > 0)
    {
      // getting process information; NB: this only works for Pythia
      Int_t processtype = AliPWG0Helper::GetEventProcessType(header);

      processEvent = kFALSE;

      // non diffractive
      if (fSelectProcessType == 1 && processtype == AliPWG0Helper::kND )
        processEvent = kTRUE;

      // single diffractive
      if (fSelectProcessType == 2 && processtype == AliPWG0Helper::kSD )
        processEvent = kTRUE;

      // double diffractive
      if (fSelectProcessType == 3 && processtype == AliPWG0Helper::kDD )
        processEvent = kTRUE;

      if (!processEvent)
        AliDebug(AliLog::kDebug, Form("Skipping this event, because it is not of the requested process type (%d)", processtype));
    }

    // systematic study: 10% lower efficiency
    if (fSystSkipParticles && (gRandom->Uniform() < 0.1))
      processEvent = kFALSE;

    if (processEvent)
    {
      // get the MC vertex
      AliGenEventHeader* genHeader = header->GenEventHeader();
      TArrayF vtxMC(3);
      genHeader->PrimaryVertex(vtxMC);

      // get number of tracks from MC
      // loop over mc particles
      Int_t nPrim  = stack->GetNprimary();
      Int_t nMCPart = stack->GetNtrack();

      // tracks in different eta ranges
      Int_t nMCTracks05 = 0;
      Int_t nMCTracks09 = 0;
      Int_t nMCTracks15 = 0;
      Int_t nMCTracks20 = 0;
      Int_t nMCTracksAll = 0;

      // tracks per particle species, in |eta| < 2 (systematic study)
      Int_t nMCTracksSpecies[4]; // (pi, K, p, other)
      for (Int_t i = 0; i<4; ++i)
        nMCTracksSpecies[i] = 0;

      for (Int_t iMc = 0; iMc < nPrim; ++iMc)
      {
        AliDebug(AliLog::kDebug+1, Form("MC Loop: Processing particle %d.", iMc));

        TParticle* particle = stack->Particle(iMc);

        if (!particle)
        {
          AliDebug(AliLog::kError, Form("UNEXPECTED: particle with label %d not found in stack (mc loop).", iMc));
          continue;
        }

        Bool_t debug = kFALSE;
        if (AliPWG0Helper::IsPrimaryCharged(particle, nPrim, debug) == kFALSE)
        {
          //printf("%d) DROPPED ", iMC);
          //particle->Print();
          continue;
        }

        //printf("%d) OK      ", iMC);
        //particle->Print();

        //if (particle->Pt() < kPtCut)
        //  continue;

        Int_t particleWeight = 1;

        Float_t pt = particle->Pt();

        // in case of systematic study, weight according to the change of the pt spectrum
        // it cannot be just multiplied because we cannot count "half of a particle"
        // instead a random generator decides if the particle is counted twice (if value > 1)
        // or not (if value < 0)
        if (fPtSpectrum)
        {
          Int_t bin = fPtSpectrum->FindBin(pt);
          if (bin > 0 && bin <= fPtSpectrum->GetNbinsX())
          {
            Float_t factor = fPtSpectrum->GetBinContent(bin);
            if (factor > 0)
            {
              Float_t random = gRandom->Uniform();
              if (factor > 1 && random < factor - 1)
              {
                particleWeight = 2;
              }
              else if (factor < 1 && random < 1 - factor)
                particleWeight = 0;
            }
          }
        }

        //Printf("MC weight is: %d", particleWeight);

        if (TMath::Abs(particle->Eta()) < 0.5)
          nMCTracks05 += particleWeight;

        if (TMath::Abs(particle->Eta()) < 0.9)
          nMCTracks09 += particleWeight;

        if (TMath::Abs(particle->Eta()) < 1.5)
          nMCTracks15 += particleWeight;

        if (TMath::Abs(particle->Eta()) < 2.0)
          nMCTracks20 += particleWeight;

        nMCTracksAll += particleWeight;

        if (fParticleCorrection[0] || fParticleSpecies)
        {
          Int_t id = -1;
          switch (TMath::Abs(particle->GetPdgCode()))
          {
            case 211: id = 0; break;
            case 321: id = 1; break;
            case 2212: id = 2; break;
            default: id = 3; break;
          }

          if (TMath::Abs(particle->Eta()) < etaRange)
            nMCTracksSpecies[id]++;
            
          if (fParticleCorrection[id])
            fParticleCorrection[id]->GetTrackCorrection()->FillGene(vtxMC[2], particle->Eta(), particle->Pt());
        }
      } // end of mc particle

      fMultiplicity->FillGenerated(vtxMC[2], eventTriggered, eventVertex, (Int_t) nMCTracks05, (Int_t) nMCTracks09, (Int_t) nMCTracks15, (Int_t) nMCTracks20, (Int_t) nMCTracksAll);

      if (eventTriggered && eventVertex)
      {
        Int_t nESDTracks05 = 0;
        Int_t nESDTracks09 = 0;
        Int_t nESDTracks15 = 0;
        Int_t nESDTracks20 = 0;

        // tracks per particle species, in |eta| < 2 (systematic study)
        Int_t nESDTracksSpecies[7]; // (pi, K, p, other, nolabel, doublecount_prim, doublecount_all)
        for (Int_t i = 0; i<7; ++i)
          nESDTracksSpecies[i] = 0;

        Bool_t* foundPrimaries = new Bool_t[nPrim];   // to prevent double counting
        for (Int_t i=0; i<nPrim; i++)
          foundPrimaries[i] = kFALSE;

        Bool_t* foundTracks = new Bool_t[nMCPart];    // to prevent double counting
        for (Int_t i=0; i<nMCPart; i++)
          foundTracks[i] = kFALSE;

        for (Int_t i=0; i<inputCount; ++i)
        {
          Float_t eta = etaArr[i];
          Int_t label = labelArr[i];

          Int_t particleWeight = 1;

          // in case of systematic study, weight according to the change of the pt spectrum
          if (fPtSpectrum)
          {
            TParticle* mother = 0;

            // preserve label for later
            Int_t labelCopy = label;
            if (labelCopy >= 0)
              labelCopy = AliPWG0Helper::FindPrimaryMotherLabel(stack, labelCopy);
            if (labelCopy >= 0)
              mother = stack->Particle(labelCopy);

            // in case of pt study we do not count particles w/o label, because they cannot be scaled
            if (!mother)
              continue;

            // it cannot be just multiplied because we cannot count "half of a particle"
            // instead a random generator decides if the particle is counted twice (if value > 1) 
            // or not (if value < 0)
            Int_t bin = fPtSpectrum->FindBin(mother->Pt());
            if (bin > 0 && bin <= fPtSpectrum->GetNbinsX())
            {
              Float_t factor = fPtSpectrum->GetBinContent(bin);
              if (factor > 0)
              {
                Float_t random = gRandom->Uniform();
                if (factor > 1 && random < factor - 1)
                {
                  particleWeight = 2;
                }
                else if (factor < 1 && random < 1 - factor)
                  particleWeight = 0;
              }
            }
          }

          //Printf("ESD weight is: %d", particleWeight);

          if (TMath::Abs(eta) < 0.5)
            nESDTracks05 += particleWeight;

          if (TMath::Abs(eta) < 0.9)
            nESDTracks09 += particleWeight;

          if (TMath::Abs(eta) < 1.5)
            nESDTracks15 += particleWeight;

          if (TMath::Abs(eta) < 2.0)
            nESDTracks20 += particleWeight;


          if (fParticleCorrection[0] || fParticleSpecies)
          {
            Int_t motherLabel = -1;
            TParticle* mother = 0;

            // find mother
            if (label >= 0)
              motherLabel = AliPWG0Helper::FindPrimaryMotherLabel(stack, label);
            if (motherLabel >= 0)
              mother = stack->Particle(motherLabel);

            if (!mother)
            {
              // count tracks that did not have a label
              if (TMath::Abs(eta) < etaRange)
                nESDTracksSpecies[4]++;
              continue;
            }

            // get particle type (pion, proton, kaon, other)
            Int_t id = -1;
            switch (TMath::Abs(mother->GetPdgCode()))
            {
              case 211: id = 0; break;
              case 321: id = 1; break;
              case 2212: id = 2; break;
              default: id = 3; break;
            }

            // double counting is ok for particle ratio study
            if (TMath::Abs(eta) < etaRange)
              nESDTracksSpecies[id]++;

            // double counting is not ok for efficiency study

            // check if we already counted this particle, this way distinguishes double counted particles (bug/artefact in tracking) or double counted primaries due to secondaries (physics)
            if (foundTracks[label])
            {
              if (TMath::Abs(eta) < etaRange)
                nESDTracksSpecies[6]++;
              continue;
            }
            foundTracks[label] = kTRUE;

            // particle (primary) already counted?
            if (foundPrimaries[motherLabel])
            {
              if (TMath::Abs(eta) < etaRange)
                nESDTracksSpecies[5]++;
              continue;
            }
            foundPrimaries[motherLabel] = kTRUE;

            if (fParticleCorrection[id])
              fParticleCorrection[id]->GetTrackCorrection()->FillMeas(vtxMC[2], mother->Eta(), mother->Pt());
          }
        }

        delete[] foundTracks;
        delete[] foundPrimaries;

        if ((Int_t) nMCTracks15 > 15 && nESDTracks15 <= 3)
        {
            TTree* tree = dynamic_cast<TTree*> (GetInputData(0));
            printf("WARNING: Event %lld %s (vtx-z = %f) has %d generated and %d reconstructed...\n", tree->GetReadEntry(), tree->GetCurrentFile()->GetName(), vtxMC[2], nMCTracks15, nESDTracks15);
        }

        // fill response matrix using vtxMC (best guess)
        fMultiplicity->FillCorrection(vtxMC[2],  nMCTracks05,  nMCTracks09,  nMCTracks15,  nMCTracks20,  nMCTracksAll,  nESDTracks05,  nESDTracks09,  nESDTracks15,  nESDTracks20);

        fMultiplicity->FillMeasured(vtx[2], nESDTracks05, nESDTracks09, nESDTracks15, nESDTracks20);

        if (fParticleSpecies)
          fParticleSpecies->Fill(vtxMC[2], nMCTracksSpecies[0], nMCTracksSpecies[1], nMCTracksSpecies[2], nMCTracksSpecies[3], nESDTracksSpecies[0], nESDTracksSpecies[1], nESDTracksSpecies[2], nESDTracksSpecies[3], nESDTracksSpecies[4], nESDTracksSpecies[5], nESDTracksSpecies[6]);
      }
    }
  }

  delete etaArr;
  delete labelArr;
}

void AliMultiplicityTask::Terminate(Option_t *)
{
  // The Terminate() function is the last function to be called during
  // a query. It always runs on the client, it can be used to present
  // the results graphically or save the results to file.

  fOutput = dynamic_cast<TList*> (GetOutputData(0));
  if (!fOutput) {
    Printf("ERROR: fOutput not available");
    return;
  }

  fMultiplicity = dynamic_cast<AliMultiplicityCorrection*> (fOutput->FindObject("Multiplicity"));
  for (Int_t i = 0; i < 4; ++i)
    fParticleCorrection[i] = dynamic_cast<AliCorrection*> (fOutput->FindObject(Form("correction_%d", i)));
  fParticleSpecies = dynamic_cast<TNtuple*> (fOutput->FindObject("fParticleSpecies"));

  if (!fMultiplicity)
  {
    AliDebug(AliLog::kError, Form("ERROR: Histograms not available %p", (void*) fMultiplicity));
    return;
  }

  TFile* file = TFile::Open("multiplicity.root", "RECREATE");

  fMultiplicity->SaveHistograms();
  for (Int_t i = 0; i < 4; ++i)
    if (fParticleCorrection[i])
      fParticleCorrection[i]->SaveHistograms();
  if (fParticleSpecies)
    fParticleSpecies->Write();

  TObjString option(fOption);
  option.Write();

  file->Close();

  Printf("Writting result to multiplicity.root");
}
