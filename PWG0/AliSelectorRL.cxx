/* $Id$ */

#include "AliSelectorRL.h"

#include <AliLog.h>
#include <AliRunLoader.h>
#include <AliHeader.h>

#include <TChain.h>
#include <TFile.h>

//
// This selector depends on the RunLoader, therefore to use it you have to have the whole AliRoot available
// The benefit is that you can use the RunLoader to access everything in the data structure
// If you only have the ESD library use AliSelector instead
//

ClassImp(AliSelectorRL)

AliSelectorRL::AliSelectorRL() :
  AliSelector(),
  fRunLoader(0),
  fHeaderFile(0),
  fHeaderTree(0),
  fHeader(0)
{
  //
  // Constructor. Initialization of pointers
  //
}

AliSelectorRL::~AliSelectorRL()
{
  //
  // Destructor
  //

  // histograms are in the output list and deleted when the output
  // list is deleted by the TSelector dtor
}

Bool_t AliSelectorRL::Notify()
{
  // Calls base class Notify
  // On top of that run loader is closed, because we change the input file

  if (AliSelector::Notify() == kFALSE)
    return kFALSE;

  DeleteRunLoader();
  DeleteHeaderFile();

  return kTRUE;
}

void AliSelectorRL::SlaveTerminate()
{
  // removes runloader

  AliSelector::SlaveTerminate();

  DeleteRunLoader();
  DeleteHeaderFile();
}

AliRunLoader* AliSelectorRL::GetAliRunLoader()
{
  // Returns AliRun instance corresponding to current ESD active in fTree
  // Loads galice.root, the file is identified by replacing "AliESDs" to
  // "galice" in the file path of the ESD file. This is a hack, to be changed!

  if (!fRunLoader)
  {
    if (!fTree->GetCurrentFile())
      return 0;

    TString fileName(fTree->GetCurrentFile()->GetName());
    fileName.ReplaceAll("AliESDs", "galice");

    fRunLoader = AliRunLoader::Open(fileName);
    if (!fRunLoader)
      return 0;

    fRunLoader->LoadgAlice();
  }

  return fRunLoader;
}

void AliSelectorRL::DeleteRunLoader()
{
  //
  // deletes the runloader
  //

  if (fRunLoader)
  {
    fRunLoader->Delete();
    fRunLoader = 0;
  }
}

AliHeader* AliSelectorRL::GetHeader()
{
  // Returns header corresponding to current ESD active in fTree
  // Loads the header from galice.root, the file is identified by replacing "AliESDs" to
  // "galice" in the file path of the ESD file. This is a hack, to be changed!

  if (!fHeaderFile || !fHeaderTree)
  {
    if (!fTree->GetCurrentFile())
      return 0;

    TString fileName(fTree->GetCurrentFile()->GetName());
    fileName.ReplaceAll("AliESDs", "galice");

    AliDebug(AliLog::kInfo, Form("Opening %s", fileName.Data()));

    fHeaderFile = TFile::Open(fileName);
    if (!fHeaderFile)
      return 0;

    fHeaderTree = dynamic_cast<TTree*> (fHeaderFile->Get("TE"));
    if (!fHeaderTree)
      return 0;

    fHeaderTree->SetBranchAddress("Header", &fHeader);
  }

  fHeaderTree->GetEntry(fTree->GetTree()->GetReadEntry());

  return fHeader;
}

void AliSelectorRL::DeleteHeaderFile()
{
  //
  // Closes the kinematics file and deletes the pointer.
  //

  if (fHeaderFile)
  {
    fHeaderFile->Close();
    delete fHeaderFile;
    fHeaderTree = 0;
    fHeader = 0;
  }
}
