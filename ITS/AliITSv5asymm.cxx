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

/* $Id$ */

///////////////////////////////////////////////////////////////////////////////
//
//  Inner Traking System version 5 with asymmetric services
//  This class contains the base procedures for the Inner Tracking System
//
// Authors: R. Barbera, B. S. Nilsen.
// version 5.
// Created October 7 2000.
//
///////////////////////////////////////////////////////////////////////////////
#include <Riostream.h>
#include <stdio.h>
#include <stdlib.h>

#include <TBRIK.h>
#include <TCanvas.h>
#include <TClonesArray.h>
#include <TFile.h>    // only required for Tracking function?
#include <TGeometry.h>
#include <TLorentzVector.h>
#include <TMath.h>
#include <TNode.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TSystem.h>
#include <TTUBE.h>
#include <TVirtualMC.h>

#include "AliRun.h"
#include "AliITShit.h"
#include "AliITSGeant3Geometry.h"
#include "AliITSgeom.h"
#include "AliITSgeomSDD.h"
#include "AliITSgeomSPD.h"
#include "AliITSgeomSSD.h"
#include "AliITShit.h"
#include "AliITSv5asymm.h"
#include "AliRun.h"
#include "AliMC.h"

ClassImp(AliITSv5asymm)
 
//_____________________________________________________________________________
AliITSv5asymm::AliITSv5asymm() {
////////////////////////////////////////////////////////////////////////
//    Standard default constructor for the ITS version 5.
////////////////////////////////////////////////////////////////////////
    Int_t i;

    fIdN    = 0;
    fIdName = 0;
    fIdSens = 0;
    fEuclidOut    = kFALSE; // Don't write Euclide file
    fGeomDetOut   = kFALSE; // Don't write .det file
    fGeomDetIn    = kFALSE; // Don't Read .det file
    fGeomOldDetIn = kFALSE; // Don't Read old formatted .det file
    fMajorVersion = IsVersion();
    fMinorVersion = 3;
    for(i=0;i<60;i++) fRead[i] = '\0';
    for(i=0;i<60;i++) fWrite[i] = '\0';
    for(i=0;i<60;i++) fEuclidGeomDet[i] = '\0';
}
//_____________________________________________________________________________
AliITSv5asymm::AliITSv5asymm(const char *name, const char *title) : AliITS(name, title){
/////////////////////////////////////////////////////////////////////////////
//    Standard constructor for the ITS version 5 with symmetrical services.
/////////////////////////////////////////////////////////////////////////////
    Int_t i;

    fIdN    = 6;
    fIdName    = new TString[fIdN];
    fIdName[0] = "ITS1";
    fIdName[1] = "ITS2";
    fIdName[2] = "ITS3";
    fIdName[3] = "ITS4";
    fIdName[4] = "ITS5";
    fIdName[5] = "ITS6";
    fIdSens    = new Int_t[fIdN];
    for (i=0;i<fIdN;i++) fIdSens[i] = 0;
    fEuclidOut    = kFALSE; // Don't write Euclide file
    fGeomDetOut   = kFALSE; // Don't write .det file
    fGeomDetIn    = kFALSE; // Don't Read .det file
    fGeomOldDetIn = kFALSE; // Don't Read old formatted .det file
    fMajorVersion = IsVersion();
    fMinorVersion = 3;
    for(i=0;i<60;i++) fRead[i] = '\0';
    for(i=0;i<60;i++) fWrite[i] = '\0';

    fEuclidMaterial = "$ALICE_ROOT/Euclid/ITSgeometry_5asymm.tme";
    fEuclidGeometry = "$ALICE_ROOT/Euclid/ITSgeometry_5asymm.euc";
    strncpy(fEuclidGeomDet,"$ALICE_ROOT/Euclid/ITSgeometry_5.det",60);
    strncpy(fRead,fEuclidGeomDet,60);
    strncpy(fWrite,fEuclidGeomDet,60);
}
//____________________________________________________________________________
AliITSv5asymm::AliITSv5asymm(const AliITSv5asymm &source):AliITS(source){
////////////////////////////////////////////////////////////////////////
//     Copy Constructor for ITS version 5.
////////////////////////////////////////////////////////////////////////
    if(&source == this) return;
    Warning("Copy Constructor","Not allowed to copy AliITSv5asymm");
    return;
}
//_____________________________________________________________________________
AliITSv5asymm& AliITSv5asymm::operator=(const AliITSv5asymm &source){
////////////////////////////////////////////////////////////////////////
//    Assignment operator for the ITS version 5.
////////////////////////////////////////////////////////////////////////

    if(&source == this) return *this;
    Warning("= operator","Not allowed to copy AliITSv5asymm");
    return *this;

}
//_____________________________________________________________________________
AliITSv5asymm::~AliITSv5asymm() {
////////////////////////////////////////////////////////////////////////
//    Standard destructor for the ITS version 5.
////////////////////////////////////////////////////////////////////////
}
//_____________________________________________________________________________
void AliITSv5asymm::BuildGeometry(){
////////////////////////////////////////////////////////////////////////
//    Geometry builder for the ITS version 5.
////////////////////////////////////////////////////////////////////////
  //
  // Build ITS TNODE geometry for event display using detailed geometry.
  // This function builds a simple ITS geometry used by the ROOT macro
  // ITSdisplay.C.

  TNode *top;
  TNode *nd;
  //const int kColorITSSPD=kRed;
  //const int kColorITSSDD=kGreen;
  const int kColorITSSSD=kBlue;
  //
  AliITSgeom  *gm = this->GetITSgeom();
  if(gm==0) return;
  top=gAlice->GetGeometry()->GetNode("alice");

  Int_t       lay,lad,det,i;
  Text_t      name[10];
  Float_t     xg[3];
  Float_t     rt[9];
  Double_t    rtd[9];
  TBRIK       *box;
  TRotMatrix  *rm;
  //TCanvas     *c1 = new TCanvas("c1","ITS");

  for(lay=1;lay<=2;lay++)
   for(lad=1;lad<=gm->GetNladders(lay);lad++)
    for(det=1;det<=gm->GetNdetectors(lay);det++){
          try {
              box  = new TBRIK("ActiveSPD","Active volume of SPD","SPD SI DET",
		        	    0.64,0.0075,4.19); 
          } catch (...) {
	      cout << "EXCEPTION in box = new TBRIK" << endl;
	      return;
	  }
          gm->GetTrans(lay,lad,det,xg[0],xg[1],xg[2]);
          gm->GetRotMatrix(lay,lad,det,rt);
          //sprintf(name,"ROT%1.1d2.2d2.2d",lay,lad,det);
          for(i=0;i<9;i++) rtd[i] = rt[i];
          try {
	        rm  = new TRotMatrix(name,name,rtd);
          } catch (...) {
	        cout << "EXCEPTION in   new TRotMatrix" << endl;
                return;
          }
         top->cd();
	  //sprintf(name,"ND%1.1d2.2d2.2d",lay,lad,det); 
         try {
              nd  = new TNode("SPD"," ",box,xg[0],xg[1],xg[2],rm);
         } catch (...) {
              cout << "EXCEPTION in new TNode" << endl;
              return;
         }
         nd->SetLineColor(kColorITSSSD);
         fNodes->Add(nd);
    }

  for(lay=3;lay<=3;lay++)
   for(lad=1;lad<=gm->GetNladders(lay);lad++)
    for(det=1;det<=gm->GetNdetectors(lay);det++){
          try {
              box  = new TBRIK ("ActiveSDD","Active volume of SDD","SDD SI DET",
		        	    3.5,0.014,3.763); 
          } catch (...) {
	      cout << "EXCEPTION in box = new TBRIK" << endl;
	      return;
	  }
          gm->GetTrans(lay,lad,det,xg[0],xg[1],xg[2]);
          gm->GetRotMatrix(lay,lad,det,rt);
          //sprintf(name,"ROT%1.1d2.2d2.2d",lay,lad,det);
          for(i=0;i<9;i++) rtd[i] = rt[i];
          try {
	        rm  = new TRotMatrix(name,name,rtd);
          } catch (...) {
	        cout << "EXCEPTION in   new TRotMatrix" << endl;
                return;
          }
         top->cd();
	  //sprintf(name,"ND%1.1d2.2d2.2d",lay,lad,det); 
         try {
              nd  = new TNode("SDD"," ",box,xg[0],xg[1],xg[2],rm);
         } catch (...) {
              cout << "EXCEPTION in new TNode" << endl;
              return;
         }
         nd->SetLineColor(kColorITSSSD);
         fNodes->Add(nd);
    }

  for(lay=4;lay<=4;lay++)
   for(lad=1;lad<=gm->GetNladders(lay);lad++)
    for(det=1;det<=gm->GetNdetectors(lay);det++){
          try {
              box  = new TBRIK ("ActiveSDD","Active volume of SDD","SDD SI DET",
		        	    3.5,0.014,3.763); 
          } catch (...) {
	      cout << "EXCEPTION in box = new TBRIK" << endl;
	      return;
	  }
          gm->GetTrans(lay,lad,det,xg[0],xg[1],xg[2]);
          gm->GetRotMatrix(lay,lad,det,rt);
          //sprintf(name,"ROT%1.1d2.2d2.2d",lay,lad,det);
          for(i=0;i<9;i++) rtd[i] = rt[i];
          try {
	        rm  = new TRotMatrix(name,name,rtd);
          } catch (...) {
	        cout << "EXCEPTION in   new TRotMatrix" << endl;
                return;
          }
         top->cd();
	  //sprintf(name,"ND%1.1d2.2d2.2d",lay,lad,det); 
         try {
              nd  = new TNode("SDD"," ",box,xg[0],xg[1],xg[2],rm);
         } catch (...) {
              cout << "EXCEPTION in new TNode" << endl;
              return;
         }
         nd->SetLineColor(kColorITSSSD);
         fNodes->Add(nd);
    }
 for(lay=5;lay<=5;lay++)
   for(lad=1;lad<=gm->GetNladders(lay);lad++)
    for(det=1;det<=gm->GetNdetectors(lay);det++){
          try {
              box  = new TBRIK ("ActiveSSD","Active volume of SSD","SSD SI DET",
		        	    3.65,0.015,2.0); 
          } catch (...) {
	      cout << "EXCEPTION in box = new TBRIK" << endl;
	      return;
	  }
          gm->GetTrans(lay,lad,det,xg[0],xg[1],xg[2]);
          gm->GetRotMatrix(lay,lad,det,rt);
          //sprintf(name,"ROT%1.1d2.2d2.2d",lay,lad,det);
          for(i=0;i<9;i++) rtd[i] = rt[i];
          try {
	        rm  = new TRotMatrix(name,name,rtd);
          } catch (...) {
	        cout << "EXCEPTION in   new TRotMatrix" << endl;
                return;
          }
         top->cd();
	  //sprintf(name,"ND%1.1d2.2d2.2d",lay,lad,det); 
         try {
              nd  = new TNode("SSD"," ",box,xg[0],xg[1],xg[2],rm);
         } catch (...) {
              cout << "EXCEPTION in new TNode" << endl;
              return;
         }
         nd->SetLineColor(kColorITSSSD);
         fNodes->Add(nd);
    }

 for(lay=6;lay<=6;lay++)
   for(lad=1;lad<=gm->GetNladders(lay);lad++)
    for(det=1;det<=gm->GetNdetectors(lay);det++){
          try {
              box  = new TBRIK ("ActiveSSD","Active volume of SSD","SSD SI DET",
		        	    3.65,0.015,2.0); 
          } catch (...) {
	      cout << "EXCEPTION in box = new TBRIK" << endl;
	      return;
	  }

          gm->GetTrans(lay,lad,det,xg[0],xg[1],xg[2]); 
          gm->GetRotMatrix(lay,lad,det,rt);
          //sprintf(name,"ROT%1.1d2.2d2.2d",lay,lad,det);
          for(i=0;i<9;i++) rtd[i] = rt[i];
          try {
	        rm  = new TRotMatrix(name,name,rtd);
          } catch (...) {
	        cout << "EXCEPTION in   new TRotMatrix" << endl;
                return;
          }
         top->cd();
          //sprintf(name,"ND%1.1d2.2d2.2d",lay,lad,det); 
         try {
              nd  = new TNode("SSD"," ",box,xg[0],xg[1],xg[2],rm);
         } catch (...) {
              cout << "EXCEPTION in new TNode" << endl;
              return;
         }
         nd->SetLineColor(kColorITSSSD);
         fNodes->Add(nd);
    }


} 
//_____________________________________________________________________________
void AliITSv5asymm::CreateMaterials(){
////////////////////////////////////////////////////////////////////////
//     Read a file containing the materials for the ITS version 5.
////////////////////////////////////////////////////////////////////////
    char *filtmp;

  filtmp = gSystem->ExpandPathName(fEuclidMaterial.Data());

  FILE *file = fopen(filtmp,"r");
  if(file) {
    fclose(file);
    ReadEuclidMedia(filtmp);
  } else {
    Error("CreateMaterials"," THE MEDIA FILE %s DOES NOT EXIST !",filtmp);
    exit(1);
  } // end if(file)
}
//_____________________________________________________________________________
void AliITSv5asymm::CreateGeometry(){
//////////////////////////////////////////////////////////////////////
//    This is the geometry used for the ITS Pre-TDR and comes from an 
// Euclid to Geant conversion. The only difference
// is in the details of the ITS supports. The detectors elements, 
// detector numbering, and local and global reference frames are shown in
//  the following figures.
//Begin_Html
/*
<img src="picts/ITS/its1+2_convention_front_5.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the front view of the SPDs.
</font>
<pre>
<img src="picts/ITS/its1+2_convention_side_5.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the perspective view of the SPDs.
</font>
<img src="picts/ITS/its1+2_tree.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the geometry Tree for the SPDs.
</font>
<pre>

<pre>
<img src="picts/ITS/its3+4_convention_front_5.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the front view of the SDDs.
</font>
<pre>
<img src="picts/ITS/its3+4_convention_side_5.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the perspective view of the SDDs.
</font>
<img src="picts/ITS/its3+4_tree.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the geometry Tree for the SDDs.
</font>
<pre>

<pre>
<img src="picts/ITS/its5+6_convention_front_5.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the front view of the SSDs.
</font>
<pre>
<img src="picts/ITS/its5+6_convention_side_5.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the perspective view of the SSDs.
</font>
<pre>
<img src="picts/ITS/its5+6_tree.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the geometry Tree for the SSDs.
</font>
<pre>


<img src="picts/ITS/its_layer1-6_2.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the front view of the whole ITS..
</font>
<pre>

<img src="picts/ITS/its_layer1-6_1.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the perspective view of the whole ITS..
</font>
<pre>

<img src="picts/ITS/its1-6_tree.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the geometry Tree for the whole ITS.
</font>
<pre>
*/
//End_Html
//
//
//      Here are shown the details of the ITS support cones and services.
// First is a GEANT tree showing the organization of all of the volumes
// that make up the ITS supports and services.
//Begin_Html
/*
<img src="picts/ITS/supports_tree.gif">
 */
//End_Html
//     What follows are a number of figures showing what these support
// structures look like.
//Begin_Html
/*

<img src="picts/ITS/supports_3.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the geometry of the supports for the Drift and Strip layers only.
</font>
<pre>

<img src="picts/ITS/supports_2.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the geometry of the supports for the Drift and Strip layers in front cut out.
</font>
<pre>

<img src="picts/ITS/supports_1.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the geometry of the supports for the Drift and Strip layers in a back cut out..
</font>
<pre>

<img src="picts/ITS/suppssd.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the geometry for the Strip layers supports.
</font>
<pre>

<img src="picts/ITS/suppsdd.gif">
</pre>
<br clear=left>
<font size=+2 color=red>
<p>This shows the geometry for the Drift layers supports.
</font>
<pre>
 */
//End_Html
//
//    Read a file containing the geometry for the ITS version 5.
////////////////////////////////////////////////////////////////////////
    char topvol[5];
    char *filtmp;

  filtmp = gSystem->ExpandPathName(fEuclidGeometry.Data());
  FILE *file = fopen(filtmp,"r");
  delete [] filtmp;
  if(file) {
    fclose(file);
    if(fDebug) cout << ClassName() << ": Ready to read Euclid geometry file" << endl;
    ReadEuclid(fEuclidGeometry.Data(),topvol);
    if(fDebug) cout << ClassName() << ": Read in euclid geometries" << endl;
  } else 
    Fatal("CreateGeometry"," THE GEOM FILE %s DOES NOT EXIST !",
	  fEuclidGeometry.Data());
  // end if(file)
  //
  // Place the ITS ghost volume ITSV in its mother volume (ALIC) and make it
  // invisible
  //
  gMC->Gspos("ITSV",1,"ALIC",0,0,0,0,"ONLY");
  //
  // Outputs the geometry tree in the EUCLID/CAD format if requested to do so
  
    if (fEuclidOut) {
      gMC->WriteEuclid("ITSgeometry", "ITSV", 1, 5);
    } // end if (fEuclidOut)

    if(fDebug) cout << ClassName() << ": finished with euclid geometrys" << endl;
}

//______________________________________________________________________
void AliITSv5asymm::ReadOldGeometry(const char *filename){
    // read in the file containing the transformations for the active
    // volumes for the ITS version 5. This is expected to be in a file
    // ending in .det. This geometry is kept in the AliITSgeom class.
    Int_t size;
    char *filtmp;
    FILE *file;

    filtmp = gSystem->ExpandPathName(filename);
    size = strlen(filtmp);
    if(size>4 && fGeomDetIn){
        filtmp[size-3] = 'd'; // change from .euc to .det
        filtmp[size-2] = 'e';
        filtmp[size-1] = 't';
        file = fopen(filtmp,"r");
        if(file){ // if file exists use it to fill AliITSgeom structure.
            fclose(file);
            fITSgeom = new AliITSgeom(filtmp);
            fITSgeom->DefineShapes(3); // if fShape isn't defined define it.
            // Now define the detector types/shapes.
            fITSgeom->ReSetShape(kSPD,(TObject *) new AliITSgeomSPD300());
            fITSgeom->ReSetShape(kSDD,(TObject *) new AliITSgeomSDD300());
            fITSgeom->ReSetShape(kSSD,(TObject *) new AliITSgeomSSD());
        }else{
            fITSgeom = 0;
            // fill AliITSgeom structure from geant structure just filled above
        }// end if(file)
        delete [] filtmp;
    }// end if(size>4)
}
//______________________________________________________________________
void AliITSv5asymm::InitAliITSgeom(){
//     Based on the geometry tree defined in Geant 3.21, this
// routine initilizes the Class AliITSgeom from the Geant 3.21 ITS geometry
// sturture.
//    if(gMC->IsA()!=TGeant3::Class()) {
    if(strcmp(gMC->GetName(),"TGeant3")) {
	Error("InitAliITSgeom",
		"Wrong Monte Carlo. InitAliITSgeom uses TGeant3 calls");
	return;
    } // end if
    if(fDebug) cout << ClassName() 
		    << ": Reading Geometry transformation directly from Geant 3." << endl;
    const Int_t nlayers = 6;
    const Int_t ndeep = 7;
    Int_t itsGeomTreeNames[nlayers][ndeep],lnam[20],lnum[20];
    Int_t nlad[nlayers],ndet[nlayers];
    Double_t t[3],r[10];
    Float_t  par[20],att[20];
    Int_t    npar,natt,idshape,imat,imed;
    AliITSGeant3Geometry *ig = new AliITSGeant3Geometry();
    Int_t mod,lay,lad,det,i,j,k;
    char *names[nlayers][ndeep] = {
        {"ALIC","ITSV","ITSD","IT12","I132","I186","ITS1"}, // lay=1
        {"ALIC","ITSV","ITSD","IT12","I132","I131","ITS2"}, // lay=2
        {"ALIC","ITSV","ITSD","IT34","I004","I302","ITS3"}, // lay=3
        {"ALIC","ITSV","ITSD","IT34","I005","I402","ITS4"}, // lay=4
        {"ALIC","ITSV","ITSD","IT56","I565","I562","ITS5"}, // lay=5
        {"ALIC","ITSV","ITSD","IT56","I569","I566","ITS6"}};// lay=6
    Int_t itsGeomTreeCopys[nlayers][ndeep] = {{1,1,1,1,10, 2,4}, // lay=1
                                              {1,1,1,1,10, 4,4}, // lay=2
                                              {1,1,1,1,14, 6,1}, // lay=3
                                              {1,1,1,1,22, 8,1}, // lay=4
                                              {1,1,1,1,34,23,1}, // lay=5
                                              {1,1,1,1,38,26,1}};// lay=6

    // Sorry, but this is not very pritty code. It should be replaced
    // at some point with a version that can search through the geometry
    // tree its self.
    if(fDebug) cout << ClassName() 
		    << ": Reading Geometry informaton from Geant3 common blocks" << endl;
    for(i=0;i<20;i++) lnam[i] = lnum[i] = 0;
    for(i=0;i<nlayers;i++)for(j=0;j<ndeep;j++)
        strncpy((char*) &itsGeomTreeNames[i][j],names[i][j],4); 
    //	itsGeomTreeNames[i][j] = ig->StringToInt(names[i][j]);
    mod = 0;
    for(i=0;i<nlayers;i++){
	k = 1;
	for(j=0;j<ndeep;j++) if(itsGeomTreeCopys[i][j]!=0)
	    k *= TMath::Abs(itsGeomTreeCopys[i][j]);
	mod += k;
    } // end for i

    if(fITSgeom!=0) delete fITSgeom;
    nlad[0]=20;nlad[1]=40;nlad[2]=14;nlad[3]=22;nlad[4]=34;nlad[5]=38;
    ndet[0]=4;ndet[1]=4;ndet[2]=6;ndet[3]=8;ndet[4]=23;ndet[5]=26;
    fITSgeom = new AliITSgeom(0,6,nlad,ndet,mod);
    mod = -1;
    for(lay=1;lay<=nlayers;lay++){
	for(j=0;j<ndeep;j++) lnam[j] = itsGeomTreeNames[lay-1][j];
	for(j=0;j<ndeep;j++) lnum[j] = itsGeomTreeCopys[lay-1][j];
	switch (lay){
	case 1: case 2: // layers 1 and 2 are a bit special
	    lad = 0;
	    for(j=1;j<=itsGeomTreeCopys[lay-1][4];j++){
		lnum[4] = j;
		for(k=1;k<=itsGeomTreeCopys[lay-1][5];k++){
		    lad++;
		    lnum[5] = k;
		    for(det=1;det<=itsGeomTreeCopys[lay-1][6];det++){
			lnum[6] = det;
			mod++;
			ig->GetGeometry(ndeep,lnam,lnum,t,r,idshape,npar,natt,
					par,att,imat,imed);
			fITSgeom->CreatMatrix(mod,lay,lad,det,kSPD,t,r);
			if(!(fITSgeom->IsShapeDefined((Int_t)kSPD)))
			    if(fMinorVersion==1){
                             fITSgeom->ReSetShape(kSPD,
						  new AliITSgeomSPD300());
			    } else if(fMinorVersion==2){
                             fITSgeom->ReSetShape(kSPD,
						  new AliITSgeomSPD300());
			    }else if(fMinorVersion==3){
                             fITSgeom->ReSetShape(kSPD,
						  new AliITSgeomSPD425Long());
			    }else{
                             fITSgeom->ReSetShape(kSPD,
						  new AliITSgeomSPD300());
			    } // end if
		    } // end for det
		} // end for k
            } // end for j
	    break;
	case 3: case 4: case 5: case 6: // layers 3-6
	    lnum[6] = 1;
	    for(lad=1;lad<=itsGeomTreeCopys[lay-1][4];lad++){
		lnum[4] = lad;
		for(det=1;det<=itsGeomTreeCopys[lay-1][5];det++){
		    lnum[5] = det;
		    mod++;
		    ig->GetGeometry(7,lnam,lnum,t,r,idshape,npar,natt,
				    par,att,imat,imed);
		    switch (lay){
		    case 3: case 4:
			fITSgeom->CreatMatrix(mod,lay,lad,det,kSDD,t,r);
			if(!(fITSgeom->IsShapeDefined(kSDD))) 
			    fITSgeom->ReSetShape(kSDD,new AliITSgeomSDD300());
			    break;
		    case 5: case 6:
			    fITSgeom->CreatMatrix(mod,lay,lad,det,kSSD,t,r);
			    if(!(fITSgeom->IsShapeDefined(kSSD))) 
				fITSgeom->ReSetShape(kSSD,new AliITSgeomSSD175());
			    break;
			} // end switch
		} // end for det
	    } // end for lad
	    break;
	} // end switch
    } // end for lay
    return;
}
//_____________________________________________________________________________
void AliITSv5asymm::Init(){
////////////////////////////////////////////////////////////////////////
//     Initialise the ITS after it has been created.
////////////////////////////////////////////////////////////////////////
    Int_t i;

    if(fDebug) {
      cout << endl << ClassName() << ": ";
      for(i=0;i<28;i++) cout << "*";cout << " ITSv5_Init ";
      for(i=0;i<27;i++) cout << "*";cout << endl;
    }
//
    if(fRead[0]=='\0') strncpy(fRead,fEuclidGeomDet,60);
    if(fWrite[0]=='\0') strncpy(fWrite,fEuclidGeomDet,60);
    if(fITSgeom!=0) delete fITSgeom;
    fITSgeom = new AliITSgeom();
    if(fGeomDetIn && !fGeomOldDetIn) fITSgeom->ReadNewFile(fRead);
    if(fGeomDetIn &&  fGeomOldDetIn) this->ReadOldGeometry(fRead);

    if(!fGeomDetIn) this->InitAliITSgeom();
    if(fGeomDetOut) fITSgeom->WriteNewFile(fWrite);
    AliITS::Init();
//
    if(fDebug) {
      cout << ClassName() << ": ";
      for(i=0;i<72;i++) cout << "*";
      cout << endl;
    }
} 
//_____________________________________________________________________________
void AliITSv5asymm::StepManager(){
////////////////////////////////////////////////////////////////////////
//    Called for every step in the ITS, then calles the AliITShit class
// creator with the information to be recoreded about that hit.
//     The value of the macro ALIITSPRINTGEOM if set to 1 will allow the
// printing of information to a file which can be used to create a .det
// file read in by the routine CreateGeometry(). If set to 0 or any other
// value except 1, the default behavior, then no such file is created nor
// it the extra variables and the like used in the printing allocated.
////////////////////////////////////////////////////////////////////////
  Int_t          copy, id;
  Int_t          copy1,copy2;
  Float_t        hits[8];
  Int_t          vol[4];
  TLorentzVector position, momentum;
  TClonesArray   &lhits = *fHits;
  //
  // Track status
  vol[3] = 0;
  if(gMC->IsTrackInside())      vol[3] +=  1;
  if(gMC->IsTrackEntering())    vol[3] +=  2;
  if(gMC->IsTrackExiting())     vol[3] +=  4;
  if(gMC->IsTrackOut())         vol[3] +=  8;
  if(gMC->IsTrackDisappeared()) vol[3] += 16;
  if(gMC->IsTrackStop())        vol[3] += 32;
  if(gMC->IsTrackAlive())       vol[3] += 64;
  //
  // Fill hit structure.
  if(!(gMC->TrackCharge())) return;
  //
  // Only entering charged tracks
  if((id = gMC->CurrentVolID(copy)) == fIdSens[0]) {
      vol[0] = 1;
      id = gMC->CurrentVolOffID(0,copy);
      //detector copy in the ladder = 1<->4  (ITS1)
      vol[1] = copy;
      gMC->CurrentVolOffID(1,copy1);
      //ladder copy in the module   = 1<->2  (I186)
      gMC->CurrentVolOffID(2,copy2);
      //module copy in the layer    = 1<->10 (I132)
      vol[2] = copy1+(copy2-1)*2;//# of ladders in one module  = 2
  } else if(id == fIdSens[1]){
      vol[0] = 2;
      id = gMC->CurrentVolOffID(0,copy);
      //detector copy in the ladder = 1<->4  (ITS2)
      vol[1] = copy;
      gMC->CurrentVolOffID(1,copy1);
      //ladder copy in the module   = 1<->4  (I131)
      gMC->CurrentVolOffID(2,copy2);
      //module copy in the layer    = 1<->10 (I132)
      vol[2] = copy1+(copy2-1)*4;//# of ladders in one module  = 4
  } else if(id == fIdSens[2]){
      vol[0] = 3;
      id = gMC->CurrentVolOffID(1,copy);
      //detector copy in the ladder = 1<->5  (ITS3 is inside I314)
      vol[1] = copy;
      id = gMC->CurrentVolOffID(2,copy);
      //ladder copy in the layer    = 1<->12 (I316)
      vol[2] = copy;
  } else if(id == fIdSens[3]){
      vol[0] = 4;
      id = gMC->CurrentVolOffID(1,copy);
      //detector copy in the ladder = 1<->8  (ITS4 is inside I414)
      vol[1] = copy;
      id = gMC->CurrentVolOffID(2,copy);
      //ladder copy in the layer    = 1<->22 (I417)
      vol[2] = copy;
  }else if(id == fIdSens[4]){
      vol[0] = 5;
      id = gMC->CurrentVolOffID(1,copy);
      //detector copy in the ladder = 1<->23  (ITS5 is inside I562)
      vol[1] = copy;
      id = gMC->CurrentVolOffID(2,copy);
     //ladder copy in the layer    = 1<->34 (I565)
      vol[2] = copy;
  }else if(id == fIdSens[5]){
      vol[0] = 6;
      id = gMC->CurrentVolOffID(1,copy);
      //detector copy in the ladder = 1<->26  (ITS6 is inside I566)
      vol[1] = copy;
      id = gMC->CurrentVolOffID(2,copy);
      //ladder copy in the layer = 1<->38 (I569)
      vol[2] = copy;
  } else {
      return; // not an ITS volume?
  } // end if/else if (gMC->CurentVolID(copy) == fIdSens[i])
//
  gMC->TrackPosition(position);
  gMC->TrackMomentum(momentum);
  hits[0]=position[0];
  hits[1]=position[1];
  hits[2]=position[2];
  hits[3]=momentum[0];
  hits[4]=momentum[1];
  hits[5]=momentum[2];
  hits[6]=gMC->Edep();
  hits[7]=gMC->TrackTime();
  // Fill hit structure with this new hit.
  new(lhits[fNhits++]) AliITShit(fIshunt,gAlice->GetMCApp()->GetCurrentTrackNumber(),vol,hits);
  return;
}
