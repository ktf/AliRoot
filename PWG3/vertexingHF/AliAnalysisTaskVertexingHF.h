#ifndef ALIANALYSISTASKVERTEXINGHF_H
#define ALIANALYSISTASKVERTEXINGHF_H

/* Copyright(c) 1998-2007, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//*************************************************************************
// Class AliAnalysisTaskVertexingHF
// AliAnalysisTask for the reconstruction of heavy-flavour decay candidates
// Author: J.Faivre, julien.faivre@pd.infn.it
//*************************************************************************


#include <TH1.h>
#include <TChain.h>
#include "AliESDEvent.h"
#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"
#include "AliAnalysisDataContainer.h"
#include "AliAnalysisVertexingHF.h"


class AliAnalysisTaskVertexingHF : public AliAnalysisTask
{
 public:

  AliAnalysisTaskVertexingHF() : AliAnalysisTask(), fESD(0), fChain(0), fVHF(0), fTrees(0) {}
  AliAnalysisTaskVertexingHF(const char *name);
  virtual ~AliAnalysisTaskVertexingHF();

  virtual void   ConnectInputData(Option_t *); 
  virtual void   CreateOutputObjects();
  virtual void   Exec(Option_t *option);
  virtual void   Terminate(Option_t *);
  virtual void   LocalInit();
  
 private:

  AliAnalysisTaskVertexingHF(const AliAnalysisTaskVertexingHF &source);
  AliAnalysisTaskVertexingHF& operator=(const AliAnalysisTaskVertexingHF& source); 

  AliESDEvent            *fESD;   //!ESD
  TChain                 *fChain; //!Chain
  AliAnalysisVertexingHF *fVHF;    //Vertexer heavy flavour
  TTree                  **fTrees; //Output trees (D0 in 2-prongs, J/Psi to e+e-, 3-prongs (D+, Ds, Lc), D0 in 4-prongs)
  
  ClassDef(AliAnalysisTaskVertexingHF,2); //AliAnalysisTask for the reconstruction of heavy-flavour decay candidates
};

#endif

