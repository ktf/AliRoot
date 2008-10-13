#include "TFile.h"
#include "TObjArray.h"
#include "TDirectory.h"
#include "TTreeStream.h"

#include "AliLog.h"
#include "AliAnalysisTask.h"

#include "AliTRDrecoTask.h"

ClassImp(AliTRDrecoTask)

//_______________________________________________________
AliTRDrecoTask::AliTRDrecoTask(const char *name, const char *title)
  : AliAnalysisTask(name, title)
  ,fNRefFigures(0)
  ,fDebugLevel(0)
  ,fContainer(0x0)
  ,fTracks(0x0)
  ,fDebugStream(0x0)
{
  DefineInput(0, TObjArray::Class());
  DefineOutput(0, TObjArray::Class());
}

//_______________________________________________________
AliTRDrecoTask::~AliTRDrecoTask() 
{
  if(fDebugStream){ 
    delete fDebugStream;
    fDebugStream = 0x0;
  }

  if(fContainer){
    //fContainer->Delete();
    delete fContainer;
    fContainer = 0x0;
  }
}

//_______________________________________________________
void AliTRDrecoTask::ConnectInputData(Option_t *)
{
  //
  // Connect input data
  //

  fTracks = dynamic_cast<TObjArray *>(GetInputData(0));
}


//_______________________________________________________
void AliTRDrecoTask::GetRefFigure(Int_t /*ifig*/)
{
  AliWarning("Retrieving reference figures not implemented.");
}

//_______________________________________________________
Bool_t AliTRDrecoTask::Load(const Char_t *filename)
{
  if(!TFile::Open(filename)) return kFALSE;
  TObjArray *o = (TObjArray*)gFile->Get(GetName());
  fContainer = (TObjArray*)o->Clone(GetName());
  gFile->Close();
  return kFALSE;
}

//_______________________________________________________
Bool_t AliTRDrecoTask::PostProcess()
{
  AliWarning("Post processing of reference histograms not implemented.");
  return kFALSE;
}

//_______________________________________________________
void AliTRDrecoTask::SetDebugLevel(Int_t level)
{
  fDebugLevel = level;
  if(fDebugLevel>=2){
    TDirectory *savedir = gDirectory;
    fDebugStream = new TTreeSRedirector(Form("TRD.Debug%s.root", GetName()));
    savedir->cd();
  }
}
