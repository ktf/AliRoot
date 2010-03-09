#ifndef ALIDIELECTRONCFDRAW_H
#define ALIDIELECTRONCFDRAW_H
/* Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//#############################################################
//#                                                           #
//#             Class AliDielectronCF                         #
//#       Dielectron Correction Framework Draw helper         #
//#                                                           #
//#  Authors:                                                 #
//#   Anton     Andronic, GSI / A.Andronic@gsi.de             #
//#   Ionut C.  Arsene,   GSI / I.C.Arsene@gsi.de             #
//#   Julian    Book,     Uni Ffm / Julian.Book@cern.ch       #
//#   Frederick Kramer,   Uni Ffm, / Frederick.Kramer@cern.ch #
//#   Magnus    Mager,    CERN / Magnus.Mager@cern.ch         #
//#   WooJin J. Park,     GSI / W.J.Park@gsi.de               #
//#   Jens      Wiechula, Uni HD / Jens.Wiechula@cern.ch      #
//#                                                           #
//#############################################################



#include <TNamed.h>
#include <AliCFContainer.h>

class TObjArray;
class TSeqCollection;
class TH1;

class AliCFContainer;

class AliDielectronCFdraw : public TNamed {
public:
  AliDielectronCFdraw();
  AliDielectronCFdraw(const char* name, const char* title);

  void SetCFContainer(AliCFContainer * const container) {fCfContainer=container;}
  void SetCFContainers(const TSeqCollection *arr);

  void SetCFContainers(const char* filename);

  AliCFContainer* GetCFContainer() const {return fCfContainer;}

  void SetRangeUser(Int_t ivar, Double_t min, Double_t max, const char* slices="");
  void SetRangeUser(const char* varname, Double_t min, Double_t max, const char* slices="");

  void UnsetRangeUser(Int_t ivar, const char* slices="");
  void UnsetRangeUser(const char* varname, const char* slices="");

//   virtual void Draw(const Option_t* /*option*/ = "") {;}
  void Draw(const Option_t* varnames, const char* opt="", const char* slices="");
  void Draw(Int_t var, const char* opt="", const char* slices="");
  void Draw(Int_t var0, Int_t var1, const char* opt="", const char* slices="");
  void Draw(Int_t var0, Int_t var1, Int_t var2, const char* opt="", const char* slices="");
  void Draw(const TObjArray *arr, const char* opt="");
  
  TObjArray* CollectHistos(Int_t dim, Int_t *vars, const char* slices);
  TH1* Project(Int_t ndim, Int_t *vars, Int_t slice);
  
private:
  AliCFContainer *fCfContainer;                     // CF container

  AliDielectronCFdraw(const AliDielectronCFdraw &c);
  AliDielectronCFdraw &operator=(const AliDielectronCFdraw &c);
  
  ClassDef(AliDielectronCFdraw,0)                   // CF draw helper class
};

//
// Inline functions
//
inline void AliDielectronCFdraw::SetRangeUser(const char* varname, Double_t min, Double_t max, const char* slices)
{
  SetRangeUser(fCfContainer->GetVar(varname),min,max,slices);
}

//________________________________________________________________
inline void AliDielectronCFdraw::UnsetRangeUser(const char* varname, const char* slices)
{
  UnsetRangeUser(fCfContainer->GetVar(varname),slices);
}
#endif

