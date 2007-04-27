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
//                                                                           //
//  Experimental Hall                                                        //
//  This class contains the description of the experimental hall             //
//  and the shielding blocks.                                                // 
//  Author: Andreas Morsch                                                   //
//  andreas.morsch@cern.ch                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <TVirtualMC.h>

#include "AliConst.h"
#include "AliHALLv3.h"
#include "AliMagF.h"
#include "AliRun.h"
#include "AliLog.h"
#include <TGeoVolume.h>
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TGeoCompositeShape.h>
#include <TGeoBBox.h>
#include <TGeoTube.h>
#include <TGeoPgon.h>
#include <TGeoPcon.h>
#include <TGeoCone.h>
#include <TGeoTrd1.h>
#include <TGeoArb8.h>

ClassImp(AliHALL)
 
//_____________________________________________________________________________
AliHALLv3::AliHALLv3()
{
  //
  // Default constructor for the experimental Hall
  //
}
 
//_____________________________________________________________________________
AliHALLv3::AliHALLv3(const char *name, const char *title)
       : AliHALL(name,title)
{
  //
  // Standard constructor for the experimental Hall
  //

}
 
//_____________________________________________________________________________
void AliHALLv3::CreateGeometry()
{
  //
  // Create the geometry of the exprimental hall
  //
  Float_t r2, dy;
  Float_t phid, phim, h, r;
  Float_t w1, dh, am, bm, dl,cm, hm, dr, dx, xl;
  Float_t hullen;
  Float_t phi;

  // The top volume
  //
  TGeoVolume* top = gGeoManager->GetVolume("ALIC");
  TGeoVolumeAssembly* asHall = new TGeoVolumeAssembly("HALL");
  
  // Rotations
  // rotation by 90 deg in the y-z plane
  TGeoRotation* rot000 = new TGeoRotation("rot000",  90., 0.,  0.,  0., 90., 90.);  
  TGeoRotation* rot001 = new TGeoRotation("rot001", 270., 0., 90., 90.,  0.,  0.);  

  // Materials
  TGeoMedium* kMedCC     = gGeoManager->GetMedium("HALL_CC_C2");
  TGeoMedium* kMedST     = gGeoManager->GetMedium("HALL_STST_C2");
  TGeoMedium* kMedAir    = gGeoManager->GetMedium("HALL_AIR_C2");
  kMedST->Dump();
  
  // Floor thickness 
  Float_t dyFloor  =  190.;
  // Floor width
  Float_t dxFloor  = 1400.;
  // Floor level 
  Float_t  yFloor  = -801.;
  // Pit centre
  Float_t  zPit    = 2300.;
  // Pit radius
  Float_t  rPit    = 1140.;
  // Hall end 
  Float_t  zHall24 = 1700.;
  Float_t  zHall26 = 1900.;
    // Overlap between hall and pit radius
  Float_t oPit = zHall24 - (zPit - rPit);
  // Length of the L3 floor
  Float_t dzL3 = 1700.;
  // Start of hall roof in y
  Float_t yHall = 500.;
  // Radius of the hall roof
  Float_t rHall = 1070.;
  //
  Float_t epsBig   = 100.;
  Float_t epsSmall =   1.;
  
  

  //
  // RB24/26 Tunnel Floor 
  r   = 220.;
  h   = 140.;
  phi = TMath::ACos(h / r);
  xl  = r * TMath::Sin(phi);
  dr  = 1600.;
  dh  = dr * TMath::Cos(phi);
  dl  = dr * TMath::Sin(phi);
  if (gAlice->GetModule("ZDC") == 0) {
    //     No ZDC 
    hullen = 370.;
  } else {
    
    //     ZDC is present 
    hullen = 6520.;
  }

  TGeoVolume* voHUFL = new TGeoVolume("HUFL",
				      new TGeoTrd1(xl +dl, xl, hullen, dh / 2.),
				      kMedCC);
  r2 = hullen + zHall26;
  asHall->AddNode(voHUFL, 1, new TGeoCombiTrans(70., -100. - dh / 2., -r2, rot000));

  //
  // RB24/26 wall 
  phid     = phi * kRaddeg;
  TGeoVolume* voHUWA = new TGeoVolume("HUWA",
				      new TGeoTubeSeg(r, r+dr, hullen, phid - 90., 270. - phid),
				      kMedCC);
  asHall->AddNode(voHUWA, 1, new TGeoTranslation(70., 40., -zHall26 - hullen ));
  //
  // Air inside tunnel
  TGeoTube* shHUWAT1 = new TGeoTube(0., r, hullen);
  shHUWAT1->SetName("shHUWAT1");
  //
  // Space for ZDC
  TGeoBBox*    shHUWAT2 = new TGeoBBox(70., 110., hullen + 20.);
  shHUWAT2->SetName("shHUWAT2");
  TGeoTranslation*   tHUWAT2 = new TGeoTranslation("tHUWAT2", -70., -30., 0.);
  tHUWAT2->RegisterYourself();

  TGeoBBox*    shHUWAT3 = new TGeoBBox(270., 110., hullen + 20.);
  shHUWAT3->SetName("shHUWAT3");
  TGeoTranslation*   tHUWAT3 = new TGeoTranslation("tHUWAT3", 0., -110. - 140., 0.);
  tHUWAT3->RegisterYourself();
  
  TGeoCompositeShape*  shHUWAT = new TGeoCompositeShape("HUWAT", "(shHUWAT1-shHUWAT2:tHUWAT2)-shHUWAT3:tHUWAT3");
  TGeoVolume* voHUWAT = new TGeoVolume("HUWAT", shHUWAT, kMedAir);
  asHall->AddNode(voHUWAT, 1, new TGeoTranslation(70., 40., -zHall26 - hullen));
  
  //
  //  Hall floor 
  //  RB26 side
  phid      = 16.197;
  Float_t dzFloor26 = zHall26 - dzL3/2.;
  TGeoBBox* shHHF1 = new TGeoBBox(dxFloor/2. + 470., dyFloor/2., dzFloor26/2.);
  shHHF1->SetName("shHHF1");
  TGeoVolume* voHHF1 = new TGeoVolume("HHF1", shHHF1, kMedCC);
  asHall->AddNode(voHHF1, 2, new TGeoTranslation(0., yFloor, -(dzL3/2. + dzFloor26/2.)));
  // RB24 side
  Float_t dzFloor24 = zHall24 - dzL3/2.;
  TGeoBBox* shHHF41 = new TGeoBBox(dxFloor/2. + 470., dyFloor/2., dzFloor24/2.);
  shHHF41->SetName("shHHF41");
  TGeoTube* shHHF42 = new TGeoTube(0., rPit + epsBig, dyFloor/2.);
  shHHF42->SetName("shHHF42");
  TGeoCombiTrans* trHHF42 = new TGeoCombiTrans("trHHF42", 0., 0., dzFloor24/2. + rPit - oPit, rot000);
  trHHF42->RegisterYourself();
  
  TGeoCompositeShape*  shHHF4 = new TGeoCompositeShape("HHF4", "shHHF41+shHHF42:trHHF42");
  TGeoVolume* voHHF4 = new TGeoVolume("HHF4", shHHF4, kMedCC);
  asHall->AddNode(voHHF4, 1, new TGeoTranslation(0., yFloor,  dzL3/2. + dzFloor24/2.));


  //
  //  Hall side walls 
  Float_t trH1   = (1273.78 - dyFloor)/ 2.;
  Float_t trBL1  = 207.3;
  Float_t trTL1  =  50.;
  Float_t trALP1 = TMath::ATan((trBL1 - trTL1) / 2. / trH1) * kRaddeg;
  dx = 1.5 * trBL1 - 0.5 * trTL1 + dxFloor/2. + dyFloor * TMath::Tan(phid * kDegrad);
  TGeoVolume* voHHW11 = new TGeoVolume("HHW11",
				       new TGeoTrap(dzFloor26/2., 0., 0., 
						    trH1, trBL1, trTL1, trALP1, trH1, trBL1, trTL1, trALP1),
				       kMedCC);
  TGeoVolume* voHHW12 = new TGeoVolume("HHW12",
				       new TGeoTrap(dzFloor24/2., 0., 0., 
						    trH1, trBL1, trTL1, trALP1, trH1, trBL1, trTL1, trALP1),
				       kMedCC);
 
  dy = yFloor + dyFloor/2. + trH1;
  
  asHall->AddNode(voHHW12, 1, new TGeoTranslation( dx, dy,  (dzL3/2. + dzFloor24/2.)));
  asHall->AddNode(voHHW12, 2, new TGeoCombiTrans (-dx, dy,  (dzL3/2. + dzFloor24/2.), rot001));
  asHall->AddNode(voHHW11, 3, new TGeoTranslation( dx, dy, -(dzL3/2. + dzFloor26/2.)));
  asHall->AddNode(voHHW11, 4, new TGeoCombiTrans (-dx, dy, -(dzL3/2. + dzFloor26/2.), rot001));

  Float_t boDY =  (yHall - (yFloor + dyFloor/2.) - 2. * trH1)/ 2.;
  Float_t dzHall = zHall26 + zHall24;

  TGeoVolume* voHBW1 = new TGeoVolume("HBW1", new TGeoBBox(50., boDY, dzHall / 2.), kMedCC);

  asHall->AddNode(voHBW1, 1,  new TGeoTranslation( 1120., yHall - boDY, (zHall24-zHall26)/2.));
  asHall->AddNode(voHBW1, 2,  new TGeoTranslation(-1120., yHall - boDY, (zHall24-zHall26)/2.));

  //
  // Slanted wall close to L3 magnet 
  //
  phim =  45.;
  hm   = 790.;
  am   = hm * TMath::Tan(phim / 2. * kDegrad);
  bm   = (hm + 76.) / hm * am;
  cm   = bm * 2. / TMath::Sqrt(2.);
  trH1   =  (1273.78 - cm) / 2.;
  trBL1  =  235. - cm * TMath::Tan(phid * kDegrad) / 2.;
  trTL1  =   50.;
  trALP1 = TMath::ATan((trBL1 - trTL1) / 2. / trH1) * kRaddeg;

  w1 = trBL1;
  dx = cm * TMath::Tan(phid * kDegrad) + dxFloor/2. + trBL1 * 1.5 - trTL1 * .5;

  TGeoVolume* voHHW2 = new TGeoVolume("HHW2",
				      new TGeoTrap(dzL3/2., 0., 0., 
						   trH1, trBL1, trTL1, trALP1, trH1, trBL1, trTL1, trALP1),
				      kMedCC);

  r2 = cm + yFloor - dyFloor/2. + trH1;

  asHall->AddNode(voHHW2, 1, new TGeoTranslation(dx, r2, 0.));
  asHall->AddNode(voHHW2, 2, new TGeoCombiTrans(-dx, r2, 0., rot001));

  trH1   = cm / 2.;
  trBL1  = w1 + cm / 2.;
  trTL1  = w1;
  trALP1 = TMath::ATan(.5) * kRaddeg;
  dx = 1170. - trBL1 * .5 - trTL1 * .5;

  TGeoVolume* voHHW3 = new TGeoVolume("HHW3",
				      new TGeoTrap(dzL3/2., 0., 0., 
						   trH1, trBL1, trTL1, trALP1, trH1, trBL1, trTL1, trALP1),
				      kMedCC);

  r2 = trH1 - 896.;
  asHall->AddNode(voHHW3, 1, new TGeoTranslation( dx, r2, 0.));
  asHall->AddNode(voHHW3, 2, new TGeoCombiTrans (-dx, r2, 0., rot001));
  //
  // Floor L3
  Float_t dyFloorL3  = 76.;
  Float_t dx1FloorL3 = rHall + epsBig - 2. * trBL1;
  Float_t dx2FloorL3 = dx1FloorL3 + TMath::Tan(phim * kDegrad) * dyFloorL3;
  
  
  TGeoVolume* voHHF2 = new TGeoVolume("HHF2", 
				      new TGeoTrd1(dx1FloorL3, dx2FloorL3, dzL3/2., dyFloorL3/2.), 
				      kMedCC);

  asHall->AddNode(voHHF2, 1, new TGeoCombiTrans(0., yFloor - dyFloor / 2. + dyFloorL3 / 2.,0., rot000));
  //
  // Tunnel roof and pit
  // Roof
  TGeoTubeSeg* shHHC11 = new TGeoTubeSeg(rHall, rHall + 100., dzHall / 2., 0., 180.);
  shHHC11->SetName("shHHC11");
  // Pit 
  TGeoTube*    shHHC12 = new TGeoTube(rPit, rPit + 100., 1000.);
  shHHC12->SetName("shHHC12");
  // Pit inside
  TGeoTube*    shHHC13 = new TGeoTube(   0, rPit - epsSmall, 1000.);
  shHHC13->SetName("shHHC13");
  // Roof inside
  TGeoTubeSeg* shHHC14 = new TGeoTubeSeg(0., rHall, dzHall / 2. + epsBig, 0., 180.);
  shHHC14->SetName("shHHC14");

  TGeoCombiTrans* trHHC = new TGeoCombiTrans("trHHC", 0., 1000., dzHall/2. + rPit - oPit, rot000);
  trHHC->RegisterYourself();
  TGeoCompositeShape*  shHHC1 = new TGeoCompositeShape("HHC1", "shHHC11+shHHC12:trHHC-(shHHC14+shHHC13:trHHC)");
  TGeoVolume* voHHC1 = new TGeoVolume("HHC1", shHHC1, kMedCC);

  asHall->AddNode(voHHC1, 1, new TGeoTranslation(0., yHall, -(zHall26-zHall24)/2.));


  //
  // Pit wall ground level
  TGeoTube* shHHCPW1 = new TGeoTube(rPit, rPit + 100., 1206./2.);
  shHHCPW1->SetName("shHHCPW1");
  TGeoCombiTrans* trHHCPW1 = new TGeoCombiTrans("trHHCPW1", 0., 0., 0., rot000);
  trHHCPW1->RegisterYourself();

  TGeoBBox* shHHCPW2 = new TGeoBBox(rPit + 100., 1206./ 2. + 20., rPit + 100.);
  shHHCPW2->SetName("shHHCPW2");

  
  TGeoTranslation* trHHCPW2 = new TGeoTranslation("trHHCPW2", 0., 0., -(rPit + 100.) - oPit);
  trHHCPW2->RegisterYourself();

  TGeoCompositeShape*  shHHCPW = new TGeoCompositeShape("HHCPW", "shHHCPW1:trHHCPW1-shHHCPW2:trHHCPW2");
  TGeoVolume* voHHCPW = new TGeoVolume("HHCPW", shHHCPW, kMedCC);
  dy = yFloor + 1206. / 2. + dyFloor/2.;
  asHall->AddNode(voHHCPW, 1, new TGeoTranslation(0., dy, 2300.));
  // 
  // Foundations of the Muon Spectrometer
  // Drawing ALIP2A_0110
  //
  TGeoVolumeAssembly* asFMS = new TGeoVolumeAssembly("asFMS");
  Float_t zFil = -1465.86 - 60.;
  // Muon Filter Foundation
  // Pillars
  dy = 263.54/2.;
  Float_t ys = yFloor + dyFloor / 2.;
  TGeoVolume* voFmsMfPil = new TGeoVolume("FmsMfPil", new TGeoBBox( 50., dy, 165.), kMedCC);
  ys += dy;
  asFMS->AddNode(voFmsMfPil, 1, new TGeoTranslation(-330. + 50., ys, zFil + 165. - 90.));
  asFMS->AddNode(voFmsMfPil, 2, new TGeoTranslation( 330. - 50., ys, zFil + 165  - 90.));
  //
  // Transverse bars
  ys += dy;
  dy = 126.46/2.;
  ys += dy;
  TGeoVolume* voFmsMfTb1 = new TGeoVolume("FmsMfTb1", new TGeoBBox(330., dy,  90.), kMedCC);
  asFMS->AddNode(voFmsMfTb1, 1, new TGeoTranslation(0., ys, zFil));
  ys += dy;
  dy = 41.14/2.;
  ys += dy;
  TGeoVolume* voFmsMfTb2 = new TGeoVolume("FmsMfTb2", new TGeoBBox(330., dy,  60.), kMedCC);
  asFMS->AddNode(voFmsMfTb2, 1, new TGeoTranslation(0., ys, zFil));
  //
  // Dipole foundation
  ys = yFloor + dyFloor / 2.;
  dy = 263.54/2;
  ys += dy;
  TGeoVolume* voFmsDf1 = new TGeoVolume("FmsDf1", new TGeoBBox(370., dy,  448.0 / 2.), kMedCC);
  asFMS->AddNode(voFmsDf1, 1, new TGeoTranslation(0., ys, zFil + 240. + 224.));
  TGeoVolume* voFmsDf2 = new TGeoVolume("FmsDf2", new TGeoBBox(370., (263.54 + 110.)/2.,  112.0 / 2.), kMedCC);
  asFMS->AddNode(voFmsDf2, 1, new TGeoTranslation(0., ys - 110./2., zFil + 688. + 56.));

  //
  // Shielding in front of L3 magnet in PX24 and UX25
  // Drawing ALIP2I__0016
  //

  TGeoVolumeAssembly* asShRb24 = new TGeoVolumeAssembly("ShRb24");
  //
  // Side walls 
  // start 7450 from IP
  TGeoVolume* voShRb24Sw = new TGeoVolume("ShRb24Sw", new TGeoBBox(80., 420., 520.), kMedCC);
  asShRb24->AddNode(voShRb24Sw, 1, new TGeoTranslation(+315, -420. + 140., 0.));
  asShRb24->AddNode(voShRb24Sw, 2, new TGeoTranslation(-315, -420. + 140., 0.));
  //
  // Roof
  TGeoVolume* voShRb24Ro = new TGeoVolume("ShRb24Ro", new TGeoBBox(395., 80., 520.), kMedCC);
  asShRb24->AddNode(voShRb24Ro, 1, new TGeoTranslation(0., +80. + 140., 0.));
  //
  // Plug
  TGeoBBox* shShRb24Pl1 = new TGeoBBox(235., 140., 40.);
  shShRb24Pl1->SetName("ShRb24Pl1");
  //
  // Opening for beam pipe
  TGeoBBox* shShRb24Pl2 = new TGeoBBox(15., 20., 60.);
  shShRb24Pl2->SetName("ShRb24Pl2");
  //
  // Opening for tubes
  TGeoBBox* shShRb24Pl3 = new TGeoBBox(20., 60., 60.);
  shShRb24Pl3->SetName("ShRb24Pl3");

  TGeoTranslation* trPl3 = new TGeoTranslation("trPl3", +235. -90., 80., 0.);
  trPl3->RegisterYourself();
  TGeoCompositeShape*  shRb24Pl = new TGeoCompositeShape("Rb24Pl", "ShRb24Pl1-(ShRb24Pl2+ShRb24Pl3:trPl3)");
  TGeoVolume* voRb24Pl = new TGeoVolume("Rb24Pl", shRb24Pl, kMedCC);
  asShRb24->AddNode(voRb24Pl, 1, new TGeoTranslation(0., 0., 520. - 40.));

  //
  // Concrete platform and shielding PX24
  // Drawing LHCJUX 250014
  //
  TGeoVolumeAssembly* asShPx24 = new TGeoVolumeAssembly("ShPx24");
  // Platform
  TGeoVolume* voShPx24Pl = new TGeoVolume("ShPx24Pl", new TGeoBBox(1613.5/2., 120./2., 1205./2.), kMedCC);
  asShPx24->AddNode(voShPx24Pl, 1, new TGeoTranslation(55., -140. - 60., 0.));
  // Pillars
  TGeoVolume* voShPx24Pi = new TGeoVolume("ShPx24Pi", new TGeoBBox(160./2., 440./2., 40/2.), kMedCC);
  asShPx24->AddNode(voShPx24Pi, 1, new TGeoTranslation(-180. - 80., -220. -260.,  1205./2. - 20.));
  asShPx24->AddNode(voShPx24Pi, 2, new TGeoTranslation(+290. + 80., -220. -260.,  1205./2. - 20.));
  asShPx24->AddNode(voShPx24Pi, 3, new TGeoTranslation(-180. - 80., -220. -260., -1205./2. + 20. + 120.));
  asShPx24->AddNode(voShPx24Pi, 4, new TGeoTranslation(+290. + 80., -220. -260., -1205./2. + 20. + 120.));
  asShPx24->AddNode(voShPx24Pi, 5, new TGeoTranslation(-180. - 80., -220. -260., -1205./2. - 20. + 480.));
  asShPx24->AddNode(voShPx24Pi, 6, new TGeoTranslation(+290. + 80., -220. -260., -1205./2. - 20. + 480.));
  asShPx24->AddNode(voShPx24Pi, 7, new TGeoTranslation(-180. - 80., -220. -260., -1205./2. - 20. + 800.));
  asShPx24->AddNode(voShPx24Pi, 8, new TGeoTranslation(+290. + 80., -220. -260., -1205./2. - 20. + 800.));
  // Side Walls 
  TGeoVolume* voShPx24Sw = new TGeoVolume("ShPx24Sw", new TGeoBBox(160./2., 280./2., 1205./2.), kMedCC);
  asShPx24->AddNode(voShPx24Sw, 1, new TGeoTranslation(-180, 0., 0.));
  asShPx24->AddNode(voShPx24Sw, 2, new TGeoTranslation(+290, 0., 0.));
  // Roof
  TGeoVolume* voShPx24Ro = new TGeoVolume("ShPx24Ro", new TGeoBBox(630./2., 160./2., 1205./2.), kMedCC);
  asShPx24->AddNode(voShPx24Ro, 1, new TGeoTranslation(55., 80.+ 140., 0.));
  asHall->AddNode(asShRb24, 1, new TGeoTranslation(0., 0., +745. + 520.));
  asHall->AddNode(asShPx24, 1, new TGeoTranslation(0., 0., +745. + 1040. + 1205./ 2.));
  // Stainless Steel Plug 80 cm thick
  TGeoBBox* shShPx24Pl1 = new TGeoBBox(155., 140., 40.);
  shShPx24Pl1->SetName("ShPx24Pl1");
  // Opening for beam pipe
  TGeoBBox* shShPx24Pl2 = new TGeoBBox(15., 20., 60.);
  shShPx24Pl2->SetName("ShPx24Pl2");
  TGeoCompositeShape*  shPx24Pl = new TGeoCompositeShape("Px24Pl", "ShPx24Pl1-ShPx24Pl2");
  TGeoVolume* voPx24Pl = new TGeoVolume("Px24Pl", shPx24Pl, kMedST);
  asShPx24->AddNode(voPx24Pl, 1, new TGeoTranslation(55., 0., -1205./2. + 40.));
  asHall->AddNode(asFMS, 1, new TGeoTranslation(0.,  0., 0.));
  
  //
  top->AddNode(asHall, 1, gGeoIdentity);
  
}
