///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Time Of Flight                                                           //
//  This class contains the functions for version 2 of the Time Of Flight    //
//  detector.                                                                //
//                                                                           //
//Begin_Html
/*
<img src="gif/AliTOFv3Class.gif">
*/
//End_Html
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "AliTOFv2.h"
#include "AliRun.h"
#include "AliConst.h"
 
ClassImp(AliTOFv2)
 
//_____________________________________________________________________________
AliTOFv2::AliTOFv2()
{
  //
  // Default constructor
  //
}
 
//_____________________________________________________________________________
AliTOFv2::AliTOFv2(const char *name, const char *title)
       : AliTOF(name,title)
{
  //
  // Standard constructor
  //
}
 
//_____________________________________________________________________________
void AliTOFv2::CreateGeometry()
{
  //
  // Create geometry for Time Of Flight version 2
  //
  //Begin_Html
  /*
    <img src="gif/AliTOFv3.gif">
  */
  //End_Html
  //

  //
  // Create common geometry between version 2 and 3
  //
  AliTOF::CreateGeometry();
}
 
//_____________________________________________________________________________
void AliTOFv2::TOFpc(Float_t xm, Float_t ym, Float_t zm0,
		     Float_t zm1, Float_t zm2)
{
  //
  // Definition of the Time Of Fligh Resistive Plate Chambers
  //

  AliMC* pMC = AliMC::GetMC();
  
  Int_t inum;
  Float_t xcor, zcor, ytop, ycoor;
  Float_t zazor, dx, dy, dz, xp, yp, zp, ywidth;
  Int_t ink;
  Float_t par[10];
  Int_t inz, nxp, npx, npz;
  Float_t xsz, ysz, zsz;
  Int_t nzp0, nzp1, nzp2;
  
  Int_t *idtmed = gAlice->Idtmed();
  
  // X size of PPC plate 
  xsz = 60.;
  // Y size of PPC plate 
  ysz = .2;
  // Z size of PPC plate 
  zsz = 50.;
  // Width of DME box 
  ywidth = 4.;
  // Frame width along X,Y and Z axis of RPC chambers 
  dx = 0.;
  dy = .2;
  // + 0.1cm (Zagreev) 
  dz = 0.;
  // gap in RPC chamber 
  zazor = .03;
  // X size of RPC chamber 
  //      XP=3.06 ! + 0.06cm (Zagreev) 
  xp = 3.9;
  // Y size of RPC chamber 
  yp = zazor + dy * 2;
  //      YP=0.32 
  // Z size of RPC chamber 
  //      ZP=3.06 ! + 0.06cm (Zagreev) 
  // (Zagreev) 
  zp = 4.1;
  // No sensitive volumes with DME 
  par[0] = xm / 2.;
  par[1] = ywidth / 2.;
  par[2] = zm0 / 2.;
  ycoor = ym / 3. - ywidth / 2.;
  pMC->Gsvolu("FBT1", "BOX ", idtmed[505], par, 3);
  pMC->Gspos("FBT1", 0, "FTO1", 0., 0., 0., 0, "ONLY");
  par[2] = zm1 / 2.;
  pMC->Gsvolu("FBT2", "BOX ", idtmed[505], par, 3);
  pMC->Gspos("FBT2", 1, "FTO2", 0., 0., 0., 0, "ONLY");
  par[2] = zm2 / 2.;
  pMC->Gsvolu("FBT3", "BOX ", idtmed[505], par, 3);
  pMC->Gspos("FBT3", 2, "FTO3", 0., 0., 0., 0, "ONLY");
  // Electronic plate 
  par[1] = ysz / 2.;
  par[2] = zm0 / 2.;
  ycoor = ywidth / 2. - ysz / 2.;
  pMC->Gsvolu("FPE1", "BOX ", idtmed[504], par, 3);
  pMC->Gspos("FPE1", 0, "FBT1", 0., ycoor, 0., 0, "ONLY");
  pMC->Gspos("FPE1", 1, "FBT1", 0., -ycoor, 0., 0, "ONLY");
  par[2] = zm1 / 2.;
  pMC->Gsvolu("FPE2", "BOX ", idtmed[504], par, 3);
  pMC->Gspos("FPE2", 0, "FBT2", 0., ycoor, 0., 0, "ONLY");
  pMC->Gspos("FPE2", 1, "FBT2", 0., -ycoor, 0., 0, "ONLY");
  par[2] = zm2 / 2.;
  pMC->Gsvolu("FPE3", "BOX ", idtmed[504], par, 3);
  pMC->Gspos("FPE3", 0, "FBT3", 0., ycoor, 0., 0, "ONLY");
  pMC->Gspos("FPE3", 1, "FBT3", 0., -ycoor, 0., 0, "ONLY");
  // Electronic insensitive volumes 
  par[1] = yp / 2.;
  par[2] = zm0 / 2.;
  ytop = ywidth / 2. - (ysz * 2 + yp) / 2.;
  pMC->Gsvolu("FLT1", "BOX ", idtmed[505], par, 3);
  pMC->Gspos("FLT1", 0, "FBT1", 0., -ytop, 0., 0, "ONLY");
  par[2] = zm1 / 2.;
  pMC->Gsvolu("FLT2", "BOX ", idtmed[505], par, 3);
  pMC->Gspos("FLT2", 0, "FBT2", 0., -ytop, 0., 0, "ONLY");
  par[2] = zm2 / 2.;
  pMC->Gsvolu("FLT3", "BOX ", idtmed[505], par, 3);
  pMC->Gspos("FLT3", 0, "FBT3", 0., -ytop, 0., 0, "ONLY");
  // PPC-plate number along X axis 
  nxp = Int_t (xm / xsz);
  // PPC-plate number along Z axis 
  nzp0 = Int_t (zm0 / zsz);
  nzp1 = Int_t (zm1 / zsz);
  nzp2 = Int_t (zm2 / zsz);
  // Position of PPC-plates 
  par[0] = xm * .5 / nxp;
  par[2] = zm0 * .5 / nzp0;
  pMC->Gsvolu("FLK1", "BOX ", idtmed[505], par, 3);
  inum = 0;
  for (ink = 1; ink <= nxp; ++ink) {
    xcor = xm * .5 * ((ink * 2 - 1) / (Float_t) nxp - 
		      1.);
    for (inz = 1; inz <= nzp0; ++inz) {
      zcor = zm0 * .5 * ((inz * 2 - 1) / (Float_t) nzp0 - 1.);
      ++inum;
      pMC->Gspos("FLK1", inum, "FLT1", xcor, 0., zcor, 0, "ONLY");
    }
    for (inz = 1; inz <= nzp1; ++inz) {
      zcor = zm1 * .5 * ((inz * 2 - 1) / (Float_t) nzp1 - 1.);
      ++inum;
      pMC->Gspos("FLK1", inum, "FLT2", xcor, 0., zcor, 0, "ONLY");
    }
    for (inz = 1; inz <= nzp2; ++inz) {
      zcor = zm2 * .5 * ((inz * 2 - 1) / (Float_t) nzp2 - 1.);
      ++inum;
      pMC->Gspos("FLK1", inum, "FLT3", xcor, 0., zcor, 0, "ONLY");
    }
  }
  // RPC position on RPC-plate 
  npx = 15;
  // Zagreev 
  npz = 12;
  // Zagreev 
  par[0] = xsz * .5 / npx;
  par[2] = zsz * .5 / npz;
  pMC->Gsvolu("FLL1", "BOX ", idtmed[505], par, 3);
  inum = 0;
  for (ink = 1; ink <= npx; ++ink) {
    xcor = xsz * .5 * ((ink * 2 - 1) / (Float_t) npx - 1.);
    for (inz = 1; inz <= npz; ++inz) {
      zcor = zsz * .5 * ((inz * 2 - 1) / (Float_t) npz - 1.);
      ++inum;
      pMC->Gspos("FLL1", inum, "FLK1", xcor, 0., zcor, 0, "ONLY");
    }
  }
  // RPC geometry 
  par[0] = xp / 2.;
  par[1] = yp / 2.;
  par[2] = zp / 2.;
  pMC->Gsvolu("FPG1", "BOX ", idtmed[507], par, 3);
  pMC->Gspos("FPG1", inum, "FLL1", 0., 0., 0., 0, "ONLY");
  par[0] = xp / 2. - dx;
  par[1] = yp / 2. - dy;
  par[2] = zp / 2. - dz;
  pMC->Gsvolu("FPG2", "BOX ", idtmed[509], par, 3);
  pMC->Gspos("FPG2", 0, "FPG1", 0., 0., 0., 0, "ONLY");
}

//_____________________________________________________________________________
void AliTOFv2::DrawModule()
{
  //
  // Draw a shaded view of the Time Of Flight version 2
  //

  AliMC* pMC = AliMC::GetMC();
  
  // Set everything unseen
  pMC->Gsatt("*", "seen", -1);
  // 
  // Set ALIC mother transparent
  pMC->Gsatt("ALIC","SEEN",0);
  //
  // Set the volumes visible
  pMC->Gsatt("ALIC","SEEN",0);
  pMC->Gsatt("FBAR","SEEN",0);
  pMC->Gsatt("FTO1","SEEN",0);
  pMC->Gsatt("FTO2","SEEN",0);
  pMC->Gsatt("FTO3","SEEN",0);
  pMC->Gsatt("FBT1","SEEN",0);
  pMC->Gsatt("FBT2","SEEN",0);
  pMC->Gsatt("FBT3","SEEN",0);
  pMC->Gsatt("FLT1","SEEN",0);
  pMC->Gsatt("FLT2","SEEN",0);
  pMC->Gsatt("FLT3","SEEN",0);
  pMC->Gsatt("FLK1","SEEN",1);
  //
  pMC->Gdopt("hide", "on");
  pMC->Gdopt("shad", "on");
  pMC->Gsatt("*", "fill", 7);
  pMC->SetClipBox(".");
  pMC->SetClipBox("*", 0, 1000, -1000, 1000, -1000, 1000);
  pMC->DefaultRange();
  pMC->Gdraw("alic", 40, 30, 0, 12, 9.5, .02, .02);
  pMC->Gdhead(1111, "Time Of Flight");
  pMC->Gdman(18, 4, "MAN");
  pMC->Gdopt("hide","off");
}

//_____________________________________________________________________________
void AliTOFv2::CreateMaterials()
{
  //
  // Define materials for the Time Of Flight
  //
  AliTOF::CreateMaterials();
}
 
//_____________________________________________________________________________
void AliTOFv2::Init()
{
  //
  // Initialise the detector after the geometry has been defined
  //

  AliMC* pMC = AliMC::GetMC();
  
  AliTOF::Init();
  fIdFTO2=pMC->VolId("FTO2");
  fIdFTO3=pMC->VolId("FTO3");
  fIdFLT1=pMC->VolId("FLT1");
  fIdFLT2=pMC->VolId("FLT2");
  fIdFLT3=pMC->VolId("FLT3");
}
 
//_____________________________________________________________________________
void AliTOFv2::StepManager()
{
  //
  // Procedure called at each step in the Time Of Flight
  //
  Float_t hits[8];
  Int_t vol[3];
  Int_t copy, id;
  AliMC *pMC= AliMC::GetMC();
  Int_t *idtmed = gAlice->Idtmed();
  if(pMC->GetMedium()==idtmed[510-1] && 
     pMC->TrackEntering() && pMC->TrackCharge()
     && pMC->CurrentVol(0,copy)==fIdSens) {
    TClonesArray &lhits = *fHits;
    //
    // Record only charged tracks at entrance
    pMC->CurrentVolOff(1,0,copy);
    vol[2]=copy;
    pMC->CurrentVolOff(3,0,copy);
    vol[1]=copy;
    id=pMC->CurrentVolOff(6,0,copy);
    vol[0]=copy;
    if(id==fIdFTO3) {
      vol[0]+=22;
      id=pMC->CurrentVolOff(4,0,copy);
      if(id==fIdFLT3) vol[1]+=6;
    } else if (id==fIdFTO2) {
      vol[0]+=20;
      id=pMC->CurrentVolOff(4,0,copy);
      if(id==fIdFLT2) vol[1]+=8;
    } else {
      id=pMC->CurrentVolOff(4,0,copy);
      if(id==fIdFLT1) vol[1]+=14;
    }
    pMC->TrackPosition(hits);
    pMC->TrackMomentum(&hits[3]);
    hits[7]=pMC->TrackTime();
    new(lhits[fNhits++]) AliTOFhit(fIshunt,gAlice->CurrentTrack(),vol,hits);
  }
}
