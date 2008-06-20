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


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  		AliZDCv3 --- new ZDC geometry		     	     //
//  	    with the both ZDC set geometry implemented 		     //
//                                                                   //  
///////////////////////////////////////////////////////////////////////

// --- Standard libraries
#include "stdio.h"

// --- ROOT system
#include <TBRIK.h>
#include <TMath.h>
#include <TNode.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TTree.h>
#include <TVirtualMC.h>
#include <TGeoManager.h>
#include <TParticle.h>

// --- AliRoot classes
#include "AliConst.h"
#include "AliMagF.h"
#include "AliRun.h"
#include "AliZDCv3.h"
#include "AliMC.h"
 
class  AliZDCHit;
class  AliPDG;
class  AliDetector;
 
 
ClassImp(AliZDCv3)

//_____________________________________________________________________________
AliZDCv3::AliZDCv3() : 
  AliZDC(),
  fMedSensF1(0),
  fMedSensF2(0),
  fMedSensZP(0),
  fMedSensZN(0),
  fMedSensZEM(0),
  fMedSensGR(0),
  fMedSensPI(0),
  fMedSensTDI(0),
  fNalfan(0),
  fNalfap(0),
  fNben(0),  
  fNbep(0),
  fZEMLength(0),
  fpLostITC(0), 
  fpLostD1C(0), 
  fpDetectedC(0),
  fnDetectedC(0),
  fnLumiC(0),
  fpLostITA(0), 
  fpLostD1A(0), 
  fpLostTDI(0), 
  fpDetectedA(0),
  fnDetectedA(0),
  fnLumiA(0),
  fnTrou(0)
{
  //
  // Default constructor for Zero Degree Calorimeter
  //
  
}
 
//_____________________________________________________________________________
AliZDCv3::AliZDCv3(const char *name, const char *title) : 
  AliZDC(name,title),
  fMedSensF1(0),
  fMedSensF2(0),
  fMedSensZP(0),
  fMedSensZN(0),
  fMedSensZEM(0),
  fMedSensGR(0),
  fMedSensPI(0),
  fMedSensTDI(0),
  fNalfan(90),
  fNalfap(90),
  fNben(18),  
  fNbep(28), 
  fZEMLength(0),
  fpLostITC(0), 
  fpLostD1C(0), 
  fpDetectedC(0),
  fnDetectedC(0),
  fnLumiC(0),
  fpLostITA(0), 
  fpLostD1A(0), 
  fpLostTDI(0), 
  fpDetectedA(0),
  fnDetectedA(0),  
  fnLumiA(0),
  fnTrou(0)
{
  //
  // Standard constructor for Zero Degree Calorimeter 
  //
  //
  // Check that DIPO, ABSO, DIPO and SHIL is there (otherwise tracking is wrong!!!)
  
  AliModule* pipe=gAlice->GetModule("PIPE");
  AliModule* abso=gAlice->GetModule("ABSO");
  AliModule* dipo=gAlice->GetModule("DIPO");
  AliModule* shil=gAlice->GetModule("SHIL");
  if((!pipe) || (!abso) || (!dipo) || (!shil)) {
    Error("Constructor","ZDC needs PIPE, ABSO, DIPO and SHIL!!!\n");
    exit(1);
  } 
  //
  Int_t ip,jp,kp;
  for(ip=0; ip<4; ip++){
     for(kp=0; kp<fNalfap; kp++){
        for(jp=0; jp<fNbep; jp++){
           fTablep[ip][kp][jp] = 0;
        } 
     }
  }
  Int_t in,jn,kn;
  for(in=0; in<4; in++){
     for(kn=0; kn<fNalfan; kn++){
        for(jn=0; jn<fNben; jn++){
           fTablen[in][kn][jn] = 0;
        } 
     }
  }
  //
  // Parameters for hadronic calorimeters geometry
  fDimZN[0] = 3.52;
  fDimZN[1] = 3.52;
  fDimZN[2] = 50.;  
  fDimZP[0] = 11.2;
  fDimZP[1] = 6.;
  fDimZP[2] = 75.;    
  fPosZNC[0] = 0.;
  fPosZNC[1] = 1.2;
  fPosZNC[2] = -11600.; 
  fPosZPC[0] = 23.9;
  fPosZPC[1] = 0.;
  fPosZPC[2] = -11600.; 
  fPosZNA[0] = 0.;
  fPosZNA[1] = 1.2;
  fPosZNA[2] = 11620.; 
  fPosZPA[0] = 24.;
  fPosZPA[1] = 0.;
  fPosZPA[2] = 11620.; 
  fFibZN[0] = 0.;
  fFibZN[1] = 0.01825;
  fFibZN[2] = 50.;
  fFibZP[0] = 0.;
  fFibZP[1] = 0.0275;
  fFibZP[2] = 75.;
  // Parameters for EM calorimeter geometry
  fPosZEM[0] = 8.5;
  fPosZEM[1] = 0.;
  fPosZEM[2] = 735.;
  Float_t kDimZEMPb  = 0.15*(TMath::Sqrt(2.));  // z-dimension of the Pb slice
  Float_t kDimZEMAir = 0.001; 			// scotch
  Float_t kFibRadZEM = 0.0315; 			// External fiber radius (including cladding)
  Int_t   kDivZEM[3] = {92, 0, 20}; 		// Divisions for EM detector
  Float_t kDimZEM0 = 2*kDivZEM[2]*(kDimZEMPb+kDimZEMAir+kFibRadZEM*(TMath::Sqrt(2.)));
  fZEMLength = kDimZEM0;
  
}
 
//_____________________________________________________________________________
void AliZDCv3::CreateGeometry()
{
  //
  // Create the geometry for the Zero Degree Calorimeter version 2
  //* Initialize COMMON block ZDC_CGEOM
  //*

  CreateBeamLine();
  CreateZDC();
}
  
//_____________________________________________________________________________
void AliZDCv3::CreateBeamLine()
{
  //
  // Create the beam line elements
  //
  
  Float_t zc, zq, zd1, zd2, zql, zd2l;
  Float_t conpar[9], tubpar[3], tubspar[5], boxpar[3];
  Int_t im1, im2;
  //
  Int_t *idtmed = fIdtmed->GetArray();
  
  ////////////////////////////////////////////////////////////////
  //								//
  //                SIDE C - RB26 (dimuon side)			//
  //								//
  ///////////////////////////////////////////////////////////////
  
  
  // -- Mother of the ZDCs (Vacuum PCON)
  zd1 = 1921.6;
  
  conpar[0] = 0.;
  conpar[1] = 360.;
  conpar[2] = 2.;
  conpar[3] = -13500.;
  conpar[4] = 0.;
  conpar[5] = 55.;
  conpar[6] = -zd1;
  conpar[7] = 0.;
  conpar[8] = 55.;
  gMC->Gsvolu("ZDCC", "PCON", idtmed[10], conpar, 9);
  gMC->Gspos("ZDCC", 1, "ALIC", 0., 0., 0., 0, "ONLY");
  

  // -- FIRST SECTION OF THE BEAM PIPE (from compensator dipole to 
  //    	the beginning of D1) 
  tubpar[0] = 6.3/2.;
  tubpar[1] = 6.7/2.;
  // From beginning of ZDC volumes to beginning of D1
  tubpar[2] = (5838.3-zd1)/2.;
  gMC->Gsvolu("QT01", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT01", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT01 TUBE pipe from z = %f to z= %f (D1 beg.)\n",-zd1,-2*tubpar[2]-zd1);
  
  //-- SECOND SECTION OF THE BEAM PIPE (from the end of D1 to the
  //    	beginning of D2) 
  
  //-- FROM MAGNETIC BEGINNING OF D1 TO MAGNETIC END OF D1 + 13.5 cm
  //-- 	Cylindrical pipe (r = 3.47) + conical flare
  
  // -> Beginning of D1
  zd1 += 2.*tubpar[2];
  
  tubpar[0] = 3.47;
  tubpar[1] = 3.47+0.2;
  tubpar[2] = 958.5/2.;
  gMC->Gsvolu("QT02", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT02", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT02 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);

  zd1 += 2.*tubpar[2];
  
  conpar[0] = 25./2.;
  conpar[1] = 10./2.;
  conpar[2] = 10.4/2.;
  conpar[3] = 6.44/2.;
  conpar[4] = 6.84/2.;
  gMC->Gsvolu("QC01", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QC01", 1, "ZDCC", 0., 0., -conpar[0]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QC01 CONE pipe from z = %f to z= %f\n",-zd1,-2*conpar[0]-zd1);

  zd1 += 2.*conpar[0];
  
  tubpar[0] = 10./2.;
  tubpar[1] = 10.4/2.;
  tubpar[2] = 50./2.;
  gMC->Gsvolu("QT03", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT03", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT03 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2]*2.;
  
  tubpar[0] = 10./2.;
  tubpar[1] = 10.4/2.;
  tubpar[2] = 10./2.;
  gMC->Gsvolu("QT04", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT04", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT04 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  tubpar[0] = 10./2.;
  tubpar[1] = 10.4/2.;
  tubpar[2] = 3.16/2.;
  gMC->Gsvolu("QT05", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT05", 1, "ZDCC", 0., 0., -tubpar[0]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT05 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  tubpar[0] = 10.0/2.;
  tubpar[1] = 10.4/2;
  tubpar[2] = 190./2.;
  gMC->Gsvolu("QT06", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT06", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT06 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  conpar[0] = 30./2.;
  conpar[1] = 20.6/2.;
  conpar[2] = 21./2.;
  conpar[3] = 10./2.;
  conpar[4] = 10.4/2.;
  gMC->Gsvolu("QC02", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QC02", 1, "ZDCC", 0., 0., -conpar[0]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QC02 CONE pipe from z = %f to z= %f\n",-zd1,-2*conpar[0]-zd1);
  
  zd1 += conpar[0] * 2.;
  
  tubpar[0] = 20.6/2.;
  tubpar[1] = 21./2.;
  tubpar[2] = 450./2.;
  gMC->Gsvolu("QT07", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT07", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT07 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  conpar[0] = 13.6/2.;
  conpar[1] = 25.4/2.;
  conpar[2] = 25.8/2.;
  conpar[3] = 20.6/2.;
  conpar[4] = 21./2.;
  gMC->Gsvolu("QC03", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QC03", 1, "ZDCC", 0., 0., -conpar[0]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QC03 CONE pipe from z = %f to z= %f\n",-zd1,-2*conpar[0]-zd1);
  
  zd1 += conpar[0] * 2.;
  
  tubpar[0] = 25.4/2.;
  tubpar[1] = 25.8/2.;
  tubpar[2] = 205.8/2.;
  gMC->Gsvolu("QT08", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT08", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT08 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  tubpar[0] = 50./2.;
  tubpar[1] = 50.4/2.;
  // QT09 is 10 cm longer to accomodate TDI
  tubpar[2] = 515.4/2.;
  gMC->Gsvolu("QT09", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT09", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT09 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  tubpar[0] = 50./2.;
  tubpar[1] = 50.4/2.;
  // QT10 is 10 cm shorter
  tubpar[2] = 690./2.;
  gMC->Gsvolu("QT10", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT10", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT10 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  tubpar[0] = 50./2.;
  tubpar[1] = 50.4/2.;
  tubpar[2] = 778.5/2.;
  gMC->Gsvolu("QT11", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT11", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT11 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  conpar[0] = 14.18/2.;
  conpar[1] = 55./2.;
  conpar[2] = 55.4/2.;
  conpar[3] = 50./2.;
  conpar[4] = 50.4/2.;
  gMC->Gsvolu("QC04", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QC04", 1, "ZDCC", 0., 0., -conpar[0]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QC04 CONE pipe from z = %f to z= %f\n",-zd1,-2*conpar[0]-zd1);
  
  zd1 += conpar[0] * 2.;
  
  tubpar[0] = 55./2.;
  tubpar[1] = 55.4/2.;
  tubpar[2] = 730./2.;
  gMC->Gsvolu("QT12", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT12", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT12 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  conpar[0] = 36.86/2.;
  conpar[1] = 68./2.;
  conpar[2] = 68.4/2.;
  conpar[3] = 55./2.;
  conpar[4] = 55.4/2.;
  gMC->Gsvolu("QC05", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QC05", 1, "ZDCC", 0., 0., -conpar[0]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QC05 CONE pipe from z = %f to z= %f\n",-zd1,-2*conpar[0]-zd1);
  
  zd1 += conpar[0] * 2.;
  
  tubpar[0] = 68./2.;
  tubpar[1] = 68.4/2.;
  tubpar[2] = 927.3/2.;
  gMC->Gsvolu("QT13", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QT13", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT13 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  tubpar[0] = 0./2.;
  tubpar[1] = 68.4/2.;
  tubpar[2] = 0.2/2.;
  gMC->Gsvolu("QT14", "TUBE", idtmed[8], tubpar, 3);
  gMC->Gspos("QT14", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  // Ch.debug
  //printf("\n	QT14 TUBE pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
  
  zd1 += tubpar[2] * 2.;
  
  tubpar[0] = 0./2.;
  tubpar[1] = 6.4/2.;
  tubpar[2] = 0.2/2.;
  gMC->Gsvolu("QT15", "TUBE", idtmed[11], tubpar, 3);
  //-- Position QT15 inside QT14
  gMC->Gspos("QT15", 1, "QT14", -7.7, 0., 0., 0, "ONLY");

  gMC->Gsvolu("QT16", "TUBE", idtmed[11], tubpar, 3);  
  //-- Position QT16 inside QT14
  gMC->Gspos("QT16", 1, "QT14", 7.7, 0., 0., 0, "ONLY");
  
  
  //-- BEAM PIPE BETWEEN END OF CONICAL PIPE AND BEGINNING OF D2 
  
  tubpar[0] = 6.4/2.;
  tubpar[1] = 6.8/2.;
  tubpar[2] = 680.8/2.;
  gMC->Gsvolu("QT17", "TUBE", idtmed[7], tubpar, 3);

  tubpar[0] = 6.4/2.;
  tubpar[1] = 6.8/2.;
  tubpar[2] = 680.8/2.;
  gMC->Gsvolu("QT18", "TUBE", idtmed[7], tubpar, 3);
  
  // -- ROTATE PIPES 
  Float_t angle = 0.143*kDegrad; // Rotation angle
  
  //AliMatrix(im1, 90.+0.143, 0., 90., 90., 0.143, 0.); // x<0
  gMC->Matrix(im1, 90.+0.143, 0., 90., 90., 0.143, 0.); // x<0  
  gMC->Gspos("QT17", 1, "ZDCC", TMath::Sin(angle) * 680.8/ 2. - 9.4, 
             0., -tubpar[2]-zd1, im1, "ONLY"); 
  //printf("\n	QT17-18 pipe from z = %f to z= %f\n",-zd1,-2*tubpar[2]-zd1);
	     
  //AliMatrix(im2, 90.-0.143, 0., 90., 90., 0.143, 180.); // x>0 (ZP)
  gMC->Matrix(im2, 90.-0.143, 0., 90., 90., 0.143, 180.); // x>0 (ZP)  
  gMC->Gspos("QT18", 1, "ZDCC", 9.7 - TMath::Sin(angle) * 680.8 / 2., 
             0., -tubpar[2]-zd1, im2, "ONLY"); 
	   
  // -- Luminometer (Cu box) in front of ZN - side C
  boxpar[0] = 8.0/2.;
  boxpar[1] = 8.0/2.;
  boxpar[2] = 15./2.;
  gMC->Gsvolu("QLUC", "BOX ", idtmed[6], boxpar, 3);
  gMC->Gspos("QLUC", 1, "ZDCC", 0., 0.,  fPosZNC[2]+66.+boxpar[2], 0, "ONLY");
  //printf("\n	QLUC LUMINOMETER from z = %f to z= %f\n",  fPosZNC[2]+66., fPosZNC[2]+66.+2*boxpar[2]);
	         
  // --  END OF BEAM PIPE VOLUME DEFINITION FOR SIDE C (RB26 SIDE) 
  // ----------------------------------------------------------------

  ////////////////////////////////////////////////////////////////
  //								//
  //                SIDE A - RB24 				//
  //								//
  ///////////////////////////////////////////////////////////////

  // Rotation Matrices definition
  Int_t irotpipe2, irotpipe1,irotpipe5, irotpipe6, irotpipe7, irotpipe8;
  //-- rotation matrices for the tilted tube before and after the TDI 
  gMC->Matrix(irotpipe2,90.+6.3025,0.,90.,90.,6.3025,0.);       
  //-- rotation matrices for the tilted cone after the TDI to recenter vacuum chamber      
  gMC->Matrix(irotpipe1,90.-2.2918,0.,90.,90.,2.2918,180.);    
  //-- rotation matrices for the legs
  gMC->Matrix(irotpipe5,90.-5.0109,0.,90.,90.,5.0109,180.);      
  gMC->Matrix(irotpipe6,90.+5.0109,0.,90.,90.,5.0109,0.);	   
  gMC->Matrix(irotpipe7,90.-1.0027,0.,90.,90.,1.0027,180.);      
  gMC->Matrix(irotpipe8,90.+1.0027,0.,90.,90.,1.0027,0.);

  // -- Mother of the ZDCs (Vacuum PCON)		
  zd2 = 1910.22;// zd2 initial value
  
  conpar[0] = 0.;
  conpar[1] = 360.;
  conpar[2] = 2.;
  conpar[3] = zd2;
  conpar[4] = 0.;
  conpar[5] = 55.;
  conpar[6] = 13500.;
  conpar[7] = 0.;
  conpar[8] = 55.;
  gMC->Gsvolu("ZDCA", "PCON", idtmed[10], conpar, 9);
  gMC->Gspos("ZDCA", 1, "ALIC", 0., 0., 0., 0, "ONLY");
  
  // BEAM PIPE from 19.10 m to inner triplet beginning (22.965 m)  
  tubpar[0] = 6.0/2.;
  tubpar[1] = 6.4/2.;
  tubpar[2] = (386.5 - 0.22)/2.;
  gMC->Gsvolu("QA01", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA01", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("\n	QA01 TUBE from z = %f to z= %f (Inner triplet beg.)\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2];  

  // -- FIRST SECTION OF THE BEAM PIPE (from beginning of inner triplet to
  //    beginning of D1)  
  tubpar[0] = 6.3/2.;
  tubpar[1] = 6.7/2.;
  tubpar[2] = 3541.8/2.;
  gMC->Gsvolu("QA02", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA02", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("\n	QA02 TUBE from z = %f to z= %f (D1 beg.)\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2]; 
  
    
  // -- SECOND SECTION OF THE BEAM PIPE (from the beginning of D1 to the beginning of D2)
  //
  //  FROM (MAGNETIC) BEGINNING OF D1 TO THE (MAGNETIC) END OF D1 + 126.5 cm
  //  CYLINDRICAL PIPE of diameter increasing from 6.75 cm up to 8.0 cm
  //  from magnetic end :
  //  1) 80.1 cm still with ID = 6.75 radial beam screen
  //  2) 2.5 cm conical section from ID = 6.75 to ID = 8.0 cm
  //  3) 43.9 cm straight section (tube) with ID = 8.0 cm
  //
  //printf("\n	Beginning of D1 at z= %f\n",zd2);

  tubpar[0] = 6.75/2.;
  tubpar[1] = 7.15/2.;
  tubpar[2] = (945.0+80.1)/2.;
  gMC->Gsvolu("QA03", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA03", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("\n	QA03 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2];

  // Transition Cone from ID=67.5 mm  to ID=80 mm
  conpar[0] = 2.5/2.;
  conpar[1] = 6.75/2.;
  conpar[2] = 7.15/2.;
  conpar[3] = 8.0/2.;
  conpar[4] = 8.4/2.;
  gMC->Gsvolu("QA04", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QA04", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  //printf("	QA04 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];
  
  tubpar[0] = 8.0/2.;
  tubpar[1] = 8.4/2.;
  tubpar[2] = 43.9/2.;
  gMC->Gsvolu("QA05", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA05", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("\n	QA05 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2];
  
  //  Bellow (ID=80 mm) - length = 0.2 m - VMAA
  tubpar[0] = 8.0/2.;
  tubpar[1] = 8.4/2.;
  tubpar[2] = 20./2.;
  gMC->Gsvolu("QA06", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA06", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA06 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2];
  
  // Beam Position Monitor (ID=80 mm) Cu - BPMSX
  tubpar[0] = 8.0/2.;
  tubpar[1] = 8.4/2.;
  tubpar[2] = 28.5/2.;
  gMC->Gsvolu("QA07", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA07", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA07 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2];

  // First section of VAEHI (tube ID=80mm)
  tubpar[0] = 8.0/2.;
  tubpar[1] = 8.4/2.;
  tubpar[2] = 28.5/2.;
  gMC->Gsvolu("QAV1", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QAV1", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QAV1 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2];

  // Second section of VAEHI (transition cone from ID=80mm to ID=98mm)
  conpar[0] = 4.0/2.;
  conpar[1] = 8.0/2.;
  conpar[2] = 8.4/2.;
  conpar[3] = 9.8/2.;
  conpar[4] = 10.2/2.;
  gMC->Gsvolu("QAV2", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QAV2", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  //printf("	QAV2 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];
  
  //Third section of VAEHI (transition cone from ID=98mm to ID=90mm)
  conpar[0] = 1.0/2.;
  conpar[1] = 9.8/2.;
  conpar[2] = 10.2/2.;
  conpar[3] = 9.0/2.;
  conpar[4] = 9.4/2.;
  gMC->Gsvolu("QAV3", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QAV3", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  //printf("	QAV3 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];
 
  // Fourth section of VAEHI (tube ID=90mm)    
  tubpar[0] = 9.0/2.;
  tubpar[1] = 9.4/2.;
  tubpar[2] = 31.0/2.;
  gMC->Gsvolu("QAV4", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QAV4", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QAV4 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2]; 

  //---------------------------- TCDD beginning ----------------------------------    
  // space for the insertion of the collimator TCDD (2 m)
  // TCDD ZONE - 1st volume
  conpar[0] = 1.3/2.;
  conpar[1] = 9.0/2.;
  conpar[2] = 13.0/2.;
  conpar[3] = 9.6/2.;
  conpar[4] = 13.0/2.;
  gMC->Gsvolu("Q01T", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("Q01T", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  //printf("	Q01T CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];  

  // TCDD ZONE - 2nd volume    
  tubpar[0] = 9.6/2.;
  tubpar[1] = 10.0/2.;
  tubpar[2] = 1.0/2.;
  gMC->Gsvolu("Q02T", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("Q02T", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	Q02T TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2]; 

  // TCDD ZONE - third volume
  conpar[0] = 9.04/2.;
  conpar[1] = 9.6/2.;
  conpar[2] = 10.0/2.;
  conpar[3] = 13.8/2.;
  conpar[4] = 14.2/2.;
  gMC->Gsvolu("Q03T", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("Q03T", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  //printf("	Q03T CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];  

  // TCDD ZONE - 4th volume    
  tubpar[0] = 13.8/2.;
  tubpar[1] = 14.2/2.;
  tubpar[2] = 38.6/2.;
  gMC->Gsvolu("Q04T", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("Q04T", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	Q04T TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2]; 

  // TCDD ZONE - 5th volume    
  tubpar[0] = 21.0/2.;
  tubpar[1] = 21.4/2.;
  tubpar[2] = 100.12/2.;
  gMC->Gsvolu("Q05T", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("Q05T", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	Q05T TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2]; 
 
  // TCDD ZONE - 6th volume    
  tubpar[0] = 13.8/2.;
  tubpar[1] = 14.2/2.;
  tubpar[2] = 38.6/2.;
  gMC->Gsvolu("Q06T", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("Q06T", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	Q06T TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2];

  // TCDD ZONE - 7th volume
  conpar[0] = 11.34/2.;
  conpar[1] = 13.8/2.;
  conpar[2] = 14.2/2.;
  conpar[3] = 18.0/2.;
  conpar[4] = 18.4/2.;
  gMC->Gsvolu("Q07T", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("Q07T", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  //printf("	Q07T CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];

  // Upper section : one single phi segment of a tube 
  //  5 parameters for tubs: inner radius = 0.,
  //	outer radius = 7.5 cm, half length = 50 cm
  //	phi1 = 0., phi2 = 180. 
  tubspar[0] = 0.0/2.;
  tubspar[1] = 15.0/2.;
  tubspar[2] = 100.0/2.;
  tubspar[3] = 0.;
  tubspar[4] = 180.;  
  gMC->Gsvolu("Q08T", "TUBS", idtmed[7], tubspar, 5);
  // Ch.debug
  //printf("\n	upper part : one single phi segment of a tube (Q08T)\n");  
  
  // rectangular beam pipe inside TCDD upper section (Vacuum)  
  boxpar[0] = 7.0/2.;
  boxpar[1] = 2.5/2.;
  boxpar[2] = 100./2.;
  gMC->Gsvolu("Q09T", "BOX ", idtmed[10], boxpar, 3);
  // positioning vacuum box in the upper section of TCDD
  gMC->Gspos("Q09T", 1, "Q08T", 0., 1.25,  0., 0, "ONLY");
  
  // lower section : one single phi segment of a tube       
  tubspar[0] = 0.0/2.;
  tubspar[1] = 15.0/2.;
  tubspar[2] = 100.0/2.;
  tubspar[3] = 180.;
  tubspar[4] = 360.;  
  gMC->Gsvolu("Q10T", "TUBS", idtmed[7], tubspar, 5);
  // rectangular beam pipe inside TCDD lower section (Vacuum)  
  boxpar[0] = 7.0/2.;
  boxpar[1] = 2.5/2.;
  boxpar[2] = 100./2.;
  gMC->Gsvolu("Q11T", "BOX ", idtmed[10], boxpar, 3);
  // positioning vacuum box in the lower section of TCDD
  gMC->Gspos("Q11T", 1, "Q10T", 0., -1.25,  0., 0, "ONLY");  
  
  // positioning  TCDD elements in ZDC2, (inside TCDD volume)
  gMC->Gspos("Q08T", 1, "ZDCA", 0., 2.5, -100+zd2, 0, "ONLY");  
  gMC->Gspos("Q10T", 1, "ZDCA", 0., -2.5, -100+zd2, 0, "ONLY");  
    
  // RF screen 
  boxpar[0] = 0.2/2.;
  boxpar[1] = 5.0/2.;
  boxpar[2] = 100./2.;
  gMC->Gsvolu("Q12T", "BOX ", idtmed[7], boxpar, 3);  
  // positioning RF screen at both sides of TCDD
  gMC->Gspos("Q12T", 1, "ZDCA", tubspar[1]+boxpar[0], 0., -100+zd2, 0, "ONLY");  
  gMC->Gspos("Q12T", 2, "ZDCA", -tubspar[1]-boxpar[0], 0., -100+zd2, 0, "ONLY");      
  //---------------------------- TCDD end ---------------------------------------    

  // Bellow (ID=80 mm) - length = 0.3 m - VMAAB
  tubpar[0] = 8.0/2.;
  tubpar[1] = 8.4/2.;
  tubpar[2] = 30.0/2.;
  gMC->Gsvolu("QA08", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA08", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA08 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  // Flange (ID=80 mm) Cu (first section of VCTCE)
  tubpar[0] = 8.0/2.;
  tubpar[1] = 8.4/2.;
  tubpar[2] = 2.0/2.;
  gMC->Gsvolu("QA09", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA09", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA09 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  // transition cone from ID=80 to ID=212 (second section of VCTCE)
  conpar[0] = 25.0/2.;
  conpar[1] = 8.0/2.;
  conpar[2] = 8.4/2.;
  conpar[3] = 21.2/2.;
  conpar[4] = 21.8/2.;
  gMC->Gsvolu("QA10", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QA10", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  //printf("	QA10 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];
  
  // tube (ID=212 mm) Cu (third section of VCTCE)
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = 403.54/2.;
  gMC->Gsvolu("QA11", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA11", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA11 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  //   bellow (ID=212 mm) (VMBGA)
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = 40.0/2.;
  gMC->Gsvolu("QA12", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA12", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA12 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  //  TDI valve assembly (ID=212 mm) 
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = 30.0/2.;
  gMC->Gsvolu("QA13", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA13", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA13 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  // bellow (ID=212 mm) (VMBGA)  
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = 40.0/2.;
  gMC->Gsvolu("QA14", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA14", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA14 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  // skewed transition piece (ID=212 mm) (before TDI)   
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = (20.0-2.41)/2.;
  gMC->Gsvolu("QA15", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA15", 1, "ZDCA", 1.10446, 0., tubpar[2]+2.41/2.+zd2, irotpipe2, "ONLY");
  // Ch.debug
  //printf("	QA15 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+2.41+zd2);

  zd2 += 2.*tubpar[2]+2.41;    
      
  // Vacuum chamber containing TDI  
  tubpar[0] = 0.;
  tubpar[1] = 54.6/2.;
  tubpar[2] = 540.0/2.;
  gMC->Gsvolu("Q13TM", "TUBE", idtmed[10], tubpar, 3);
  gMC->Gspos("Q13TM", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  tubpar[0] = 54.0/2.;
  tubpar[1] = 54.6/2.;
  tubpar[2] = 540.0/2.;
  gMC->Gsvolu("Q13T", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("Q13T", 1, "Q13TM", 0., 0., 0., 0, "ONLY");
  // Ch.debug
  //printf("	Q13T TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  //---------------- INSERT TDI INSIDE Q13T -----------------------------------    
  boxpar[0] = 11.0/2.;
  boxpar[1] = 9.0/2.;
  boxpar[2] = 540.0/2.;
  gMC->Gsvolu("QTD1", "BOX ", idtmed[7], boxpar, 3);
  gMC->Gspos("QTD1", 1, "Q13TM", -3.8, 10.5,  0., 0, "ONLY");
  boxpar[0] = 11.0/2.;
  boxpar[1] = 9.0/2.;
  boxpar[2] = 540.0/2.;
  gMC->Gsvolu("QTD2", "BOX ", idtmed[7], boxpar, 3);
  gMC->Gspos("QTD2", 1, "Q13TM", -3.8, -10.5,  0., 0, "ONLY");  
  boxpar[0] = 5.1/2.;
  boxpar[1] = 0.2/2.;
  boxpar[2] = 540.0/2.;
  gMC->Gsvolu("QTD3", "BOX ", idtmed[7], boxpar, 3);
  gMC->Gspos("QTD3", 1, "Q13TM", -3.8+5.5+boxpar[0], 6.1,  0., 0, "ONLY");  
  gMC->Gspos("QTD3", 2, "Q13TM", -3.8+5.5+boxpar[0], -6.1,  0., 0, "ONLY"); 
  gMC->Gspos("QTD3", 3, "Q13TM", -3.8-5.5-boxpar[0], 6.1,  0., 0, "ONLY");  
  gMC->Gspos("QTD3", 4, "Q13TM", -3.8-5.5-boxpar[0], -6.1,  0., 0, "ONLY");  
  //
  tubspar[0] = 12.0/2.;
  tubspar[1] = 12.4/2.;
  tubspar[2] = 540.0/2.;
  tubspar[3] = 90.;
  tubspar[4] = 270.;  
  gMC->Gsvolu("QTD4", "TUBS", idtmed[7], tubspar, 5);
  gMC->Gspos("QTD4", 1, "Q13TM", -3.8-10.6, 0.,  0., 0, "ONLY");
  tubspar[0] = 12.0/2.;
  tubspar[1] = 12.4/2.;
  tubspar[2] = 540.0/2.;
  tubspar[3] = -90.;
  tubspar[4] = 90.;  
  gMC->Gsvolu("QTD5", "TUBS", idtmed[7], tubspar, 5);
  gMC->Gspos("QTD5", 1, "Q13TM", -3.8+10.6, 0.,  0., 0, "ONLY"); 
  //---------------- END DEFINING TDI INSIDE Q13T -------------------------------
  
  // skewed transition piece (ID=212 mm) (after TDI)  
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = (20.0-2.41)/2.;
  gMC->Gsvolu("QA16", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA16", 1, "ZDCA", 1.10446+2.2, 0., tubpar[2]+2.41/2.+zd2, irotpipe2, "ONLY");
  // Ch.debug
  //printf("	QA16 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+2.41+zd2);

  zd2 += 2.*tubpar[2]+2.41;
  
  // bellow (ID=212 mm) (VMBGA)
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = 40.0/2.;
  gMC->Gsvolu("QA17", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA17", 1, "ZDCA", 4.4, 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA17 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  //  TDI valve assembly (ID=212 mm)   
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = 30.0/2.;
  gMC->Gsvolu("QA18", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA18", 1, "ZDCA", 4.4, 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA18 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];  
  
  // bellow (ID=212 mm) (VMBGA)
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = 40.0/2.;
  gMC->Gsvolu("QA19", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA19", 1, "ZDCA", 4.4, 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA19 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];  

  // vacuum chamber  (ID=212 mm) (BTVST)
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = 50.0/2.;
  gMC->Gsvolu("QA20", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA20", 1, "ZDCA", 4.4, 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA20 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];

  // bellow (ID=212 mm) (VMBGA) repeated 3 times
  tubpar[0] = 21.2/2.;
  tubpar[1] = 21.8/2.;
  tubpar[2] = 120.0/2.;
  gMC->Gsvolu("QA21", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA21", 1, "ZDCA", 4.4, 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA21 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];

  // skewed transition cone from ID=212 mm to ID=797 mm SS for the moment
  conpar[0] = (110.0-0.44-1.63)/2.;
  conpar[1] = 21.2/2.;
  conpar[2] = 21.8/2.;
  conpar[3] = 79.7/2.;
  conpar[4] = 81.3/2.;
  gMC->Gsvolu("QA22", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QA22", 1, "ZDCA", 4.4-2.201, 0., conpar[0]+0.44+zd2, irotpipe1, "ONLY");
  //printf("	QA22 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+0.44+1.63+zd2);

  zd2 += 2.*conpar[0]+0.44+1.63;
  
  // beam pipe (ID=797 mm) SS
  tubpar[0] = 79.7/2.;
  tubpar[1] = 81.3/2.;
  tubpar[2] = 2393.05/2.;
  gMC->Gsvolu("QA23", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA23", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA23 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  // Transition from ID=797 mm to ID=196 mm SS for the moment:
  //
  // in order to simulate the thin window opened in the transition cone
  // we divide the transition cone in three cones:
  // the first 8 mm thick
  // the second 3 mm thick
  // the third 8 mm thick
  //
  // First section
  conpar[0] = 9.09/2.; // 15 degree
  conpar[1] = 79.7/2.;
  conpar[2] = 81.3/2.; // thickness 8 mm  
  conpar[3] = 74.82868/2.;
  conpar[4] = 76.42868/2.; // thickness 8 mm 
  gMC->Gsvolu("Q24A", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("Q24A", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  //printf("	Q24A CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];  

  // Second section
  conpar[0] = 96.2/2.; // 15 degree
  conpar[1] = 74.82868/2.;
  conpar[2] = 75.42868/2.; // thickness 3 mm  
  conpar[3] = 23.19588/2.;
  conpar[4] = 23.79588/2.; // thickness 3 mm 
  gMC->Gsvolu("QA25", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QA25", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");  
  //printf("	QA25 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];
  
  // Third section
  conpar[0] = 6.71/2.; // 15 degree
  conpar[1] = 23.19588/2.;
  conpar[2] = 24.79588/2.;// thickness 8 mm 
  conpar[3] = 19.6/2.;
  conpar[4] = 21.2/2.;// thickness 8 mm 
  gMC->Gsvolu("QA26", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QA26", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");  
  //printf("	QA26 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];
  
  // beam pipe (ID=196 mm)  
  tubpar[0] = 19.6/2.;
  tubpar[1] = 21.2/2.;
  tubpar[2] = 9.55/2.;
  gMC->Gsvolu("QA27", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA27", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA27 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];  
  
  // Flange (ID=196 mm)
  tubpar[0] = 19.6/2.;
  tubpar[1] = 25.3/2.;
  tubpar[2] = 4.9/2.;
  gMC->Gsvolu("QF01", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QF01", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QF01  TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  // Special Warm Module (made by 5 volumes)  
  tubpar[0] = 20.2/2.;
  tubpar[1] = 20.6/2.;
  tubpar[2] = 2.15/2.;
  gMC->Gsvolu("QA28", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA28", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA28  TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  conpar[0] = 6.9/2.;
  conpar[1] = 20.2/2.;
  conpar[2] = 20.6/2.;
  conpar[3] = 23.9/2.;
  conpar[4] = 24.3/2.;
  gMC->Gsvolu("QA29", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QA29", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  // Ch.debug  
  //printf("	QA29 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];

  tubpar[0] = 23.9/2.;
  tubpar[1] = 25.5/2.;
  tubpar[2] = 17.0/2.;
  gMC->Gsvolu("QA30", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA30", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug  
  //printf("	QA30  TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  conpar[0] = 6.9/2.;
  conpar[1] = 23.9/2.;
  conpar[2] = 24.3/2.;
  conpar[3] = 20.2/2.;
  conpar[4] = 20.6/2.;
  gMC->Gsvolu("QA31", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QA31", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  // Ch.debug  
  //printf("	QA31 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];
  
  tubpar[0] = 20.2/2.;
  tubpar[1] = 20.6/2.;
  tubpar[2] = 2.15/2.;
  gMC->Gsvolu("QA32", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA32", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug  
  //printf("	QA32  TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  // Flange (ID=196 mm)
  tubpar[0] = 19.6/2.;
  tubpar[1] = 25.3/2.;
  tubpar[2] = 4.9/2.;
  gMC->Gsvolu("QF02", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QF02", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QF02 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];
  
  // simulation of the trousers (VCTYB)
  // (last design -mail 3/6/05)     
  // pipe: a tube (ID = 196. OD = 200.)
  tubpar[0] = 19.6/2.;
  tubpar[1] = 20.0/2.;
  tubpar[2] = 3.9/2.;
  gMC->Gsvolu("QA33", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA33", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug
  //printf("	QA33  TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];

  // transition cone from ID=196. to ID=216.6
  conpar[0] = 32.55/2.;
  conpar[1] = 19.6/2.;
  conpar[2] = 20.0/2.;
  conpar[3] = 21.66/2.;
  conpar[4] = 22.06/2.;
  gMC->Gsvolu("QA34", "CONE", idtmed[7], conpar, 5);
  gMC->Gspos("QA34", 1, "ZDCA", 0., 0., conpar[0]+zd2, 0, "ONLY");
  // Ch.debug  
  //printf("	QA34 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+zd2);

  zd2 += 2.*conpar[0];
 
  // Flange:  first support for the trousers
  boxpar[0] = 25.3/2.;
  boxpar[1] = 25.3/2.;
  boxpar[2] = 2.5/2.;
  gMC->Gsvolu("QF03", "BOX ", idtmed[7], boxpar, 3);
  tubpar[0] = 0.0/2.;
  tubpar[1] = 22.06/2.;
  tubpar[2] = 2.5/2.;
  gMC->Gsvolu("QFV1", "TUBE", idtmed[10], tubpar, 3);
  gMC->Gspos("QFV1", 1, "QF03", 0., 0., 0., 0, "MANY");
  gMC->Gspos("QF03", 1, "ZDCA", 0., 0., 14.3+zd2, 0, "MANY");  
  
  // tube  
  tubpar[0] = 21.66/2.;
  tubpar[1] = 22.06/2.;
  tubpar[2] = 28.6/2.;
  gMC->Gsvolu("QA35", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA35", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");
  // Ch.debug 
  //printf("\n	QA35  TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);

  zd2 += 2.*tubpar[2];

  // legs of the trousers
  conpar[0] = (90.1-0.95-0.26)/2.;
  conpar[1] = 0.0/2.;
  conpar[2] = 21.6/2.;
  conpar[3] = 0.0/2.;
  conpar[4] = 5.8/2.;
  gMC->Gsvolu("QAL1", "CONE", idtmed[7], conpar, 5);
  gMC->Gsvolu("QAL2", "CONE", idtmed[7], conpar, 5); 
  gMC->Gspos("QAL1", 1, "ZDCA", -3.45-0.52, 0., conpar[0]+0.95+zd2, irotpipe5, "MANY");
  gMC->Gspos("QAL2", 1, "ZDCA", 3.45+0.52, 0., conpar[0]+0.95+zd2, irotpipe6, "MANY");
  
  conpar[0] = (90.1-0.95-0.26)/2.;
  conpar[1] = 0.0/2.;
  conpar[2] = 21.2/2.;
  conpar[3] = 0.0/2.;
  conpar[4] = 5.4/2.;
  gMC->Gsvolu("QAL3", "CONE", idtmed[10], conpar, 5);
  gMC->Gsvolu("QAL4", "CONE", idtmed[10], conpar, 5); 
  gMC->Gspos("QAL3", 1, "ZDCA", -3.45-0.52, 0., conpar[0]+0.95+zd2, irotpipe5, "ONLY");
  //gMC->Gspos("QAL4", 1, "ZDCA", 3.45+0.52, 0., conpar[0]+0.95+zd2, irotpipe6, "ONLY");
  gMC->Gspos("QAL4", 1, "QAL2", 0., 0., 0., 0, "ONLY");      
    
  zd2 += 90.1;
  
  //  second section : 2 tubes (ID = 54. OD = 58.)  
  tubpar[0] = 5.4/2.;
  tubpar[1] = 5.8/2.;
  tubpar[2] = 40.0/2.;
  gMC->Gsvolu("QA36", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA36", 1, "ZDCA", -15.8/2., 0., tubpar[2]+zd2, 0, "ONLY");
  gMC->Gspos("QA36", 2, "ZDCA",  15.8/2., 0., tubpar[2]+zd2, 0, "ONLY");  
  // Ch.debug
  //printf("	QA36 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);
  
  zd2 += 2.*tubpar[2];
  
  // transition x2zdc to recombination chamber : skewed cone  
  conpar[0] = (10.-0.2)/2.;
  conpar[1] = 5.4/2.;
  conpar[2] = 5.8/2.;
  conpar[3] = 6.3/2.;
  conpar[4] = 7.0/2.;
  gMC->Gsvolu("QA37", "CONE", idtmed[7], conpar, 5); 
  gMC->Gspos("QA37", 1, "ZDCA", -7.9-0.175, 0., conpar[0]+0.1+zd2, irotpipe7, "ONLY");
  gMC->Gspos("QA37", 2, "ZDCA", 7.9+0.175, 0., conpar[0]+0.1+zd2, irotpipe8, "ONLY");
  //printf("	QA37 CONE from z = %f to z= %f\n",zd2,2*conpar[0]+0.2+zd2);

  zd2 += 2.*conpar[0]+0.2;
  
  // Flange: second support for the trousers
  boxpar[0] = 25.9/2.;
  boxpar[1] = 9.4/2.;
  boxpar[2] = 1./2.;
  gMC->Gsvolu("QF04", "BOX ", idtmed[7], boxpar, 3);
  boxpar[0] = 16.5/2.;
  boxpar[1] = 7./2.;
  boxpar[2] = 1./2.;
  gMC->Gsvolu("QFV2", "BOX ", idtmed[10], boxpar, 3);
  gMC->Gspos("QFV2", 1, "QF04", 0., 0., 0., 0, "MANY");
  tubspar[0] = 0.0/2.;
  tubspar[1] = 7./2.;
  tubspar[2] = 1./2.;
  tubspar[3] = 90.;
  tubspar[4] = 270.;  
  gMC->Gsvolu("QFV3", "TUBS", idtmed[10], tubspar, 5);
  gMC->Gspos("QFV3", 1, "QF04", -16.5/2., 0.,  0., 0, "MANY"); 
  tubspar[0] = 0.0/2.;
  tubspar[1] = 7./2.;
  tubspar[2] = 1./2.;
  tubspar[3] = -90.;
  tubspar[4] = 90.;  
  gMC->Gsvolu("QFV4", "TUBS", idtmed[10], tubspar, 5);
  gMC->Gspos("QFV4", 1, "QF04", 16.5/2., 0.,  0., 0, "MANY");
  gMC->Gspos("QF04", 1, "ZDCA", 0., 0.,  18.5+zd2, 0, "MANY");
  
  // 2 tubes (ID = 63 mm OD=70 mm)      
  tubpar[0] = 6.3/2.;
  tubpar[1] = 7.0/2.;
  tubpar[2] = 512.9/2.;
  gMC->Gsvolu("QA38", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("QA38", 1, "ZDCA", -16.5/2., 0., tubpar[2]+zd2, 0, "ONLY");
  gMC->Gspos("QA38", 2, "ZDCA",  16.5/2., 0., tubpar[2]+zd2, 0, "ONLY");
  //printf("	QA38 TUBE from z = %f to z= %f\n",zd2,2*tubpar[2]+zd2);  

  zd2 += 2.*tubpar[2];
  //printf("\n	END OF BEAM PIPE VOLUME DEFINITION AT z= %f\n",zd2);
	   
  // -- Luminometer (Cu box) in front of ZN - side A
  boxpar[0] = 8.0/2.;
  boxpar[1] = 8.0/2.;
  boxpar[2] = 15./2.;
  gMC->Gsvolu("QLUA", "BOX ", idtmed[7], boxpar, 3);
  gMC->Gspos("QLUA", 1, "ZDCA", 0., 0.,  fPosZNA[2]-66.-boxpar[2], 0, "ONLY");
  //printf("\n	QLUC LUMINOMETER from z = %f to z= %f\n\n",  fPosZNA[2]-66., fPosZNA[2]-66.-2*boxpar[2]);
  

  // ----------------------------------------------------------------
  // --  MAGNET DEFINITION  -> LHC OPTICS 6.5  
  // ----------------------------------------------------------------      
  // ***************************************************************  
  //		SIDE C - RB26  (dimuon side) 
  // ***************************************************************   
  // --  COMPENSATOR DIPOLE (MBXW)
  zc = 1972.5;   
  
  // --  GAP (VACUUM WITH MAGNETIC FIELD)
  tubpar[0] = 0.;
  tubpar[1] = 3.14;
  tubpar[2] = 153./2.;
  gMC->Gsvolu("MBXW", "TUBE", idtmed[11], tubpar, 3);

  // --  YOKE 
  tubpar[0] = 4.5;
  tubpar[1] = 55.;
  tubpar[2] = 153./2.;
  gMC->Gsvolu("YMBX", "TUBE", idtmed[7], tubpar, 3);

  gMC->Gspos("MBXW", 1, "ZDCC", 0., 0., -tubpar[2]-zc, 0, "ONLY");
  gMC->Gspos("YMBX", 1, "ZDCC", 0., 0., -tubpar[2]-zc, 0, "ONLY");
  
  
  // -- INNER TRIPLET 
  zq = 2296.5; 

  // -- DEFINE MQXL AND MQX QUADRUPOLE ELEMENT 
  // --  MQXL 
  // --  GAP (VACUUM WITH MAGNETIC FIELD) 
  tubpar[0] = 0.;
  tubpar[1] = 3.14;
  tubpar[2] = 637./2.;
  gMC->Gsvolu("MQXL", "TUBE", idtmed[11], tubpar, 3);
    
  // --  YOKE 
  tubpar[0] = 3.5;
  tubpar[1] = 22.;
  tubpar[2] = 637./2.;
  gMC->Gsvolu("YMQL", "TUBE", idtmed[7], tubpar, 3);
  
  gMC->Gspos("MQXL", 1, "ZDCC", 0., 0., -tubpar[2]-zq, 0, "ONLY");
  gMC->Gspos("YMQL", 1, "ZDCC", 0., 0., -tubpar[2]-zq, 0, "ONLY");
  
  gMC->Gspos("MQXL", 2, "ZDCC", 0., 0., -tubpar[2]-zq-2400., 0, "ONLY");
  gMC->Gspos("YMQL", 2, "ZDCC", 0., 0., -tubpar[2]-zq-2400., 0, "ONLY");
  
  // --  MQX 
  // --  GAP (VACUUM WITH MAGNETIC FIELD) 
  tubpar[0] = 0.;
  tubpar[1] = 3.14;
  tubpar[2] = 550./2.;
  gMC->Gsvolu("MQX ", "TUBE", idtmed[11], tubpar, 3);
  
  // --  YOKE 
  tubpar[0] = 3.5;
  tubpar[1] = 22.;
  tubpar[2] = 550./2.;
  gMC->Gsvolu("YMQ ", "TUBE", idtmed[7], tubpar, 3);
  
  gMC->Gspos("MQX ", 1, "ZDCC", 0., 0., -tubpar[2]-zq-908.5,  0, "ONLY");
  gMC->Gspos("YMQ ", 1, "ZDCC", 0., 0., -tubpar[2]-zq-908.5,  0, "ONLY");
  
  gMC->Gspos("MQX ", 2, "ZDCC", 0., 0., -tubpar[2]-zq-1558.5, 0, "ONLY");
  gMC->Gspos("YMQ ", 2, "ZDCC", 0., 0., -tubpar[2]-zq-1558.5, 0, "ONLY");
  
  // -- SEPARATOR DIPOLE D1 
  zd1 = 5838.3;
  
  // --  GAP (VACUUM WITH MAGNETIC FIELD) 
  tubpar[0] = 0.;
  tubpar[1] = 6.94/2.;
  tubpar[2] = 945./2.;
  gMC->Gsvolu("MD1 ", "TUBE", idtmed[11], tubpar, 3);
  
  // --  Insert horizontal Cu plates inside D1 
  // --   (to simulate the vacuum chamber)
  boxpar[0] = TMath::Sqrt(tubpar[1]*tubpar[1]-(2.98+0.2)*(2.98+0.2)) - 0.05;
  boxpar[1] = 0.2/2.;
  boxpar[2] = 945./2.;
  gMC->Gsvolu("MD1V", "BOX ", idtmed[6], boxpar, 3);
  gMC->Gspos("MD1V", 1, "MD1 ", 0., 2.98+boxpar[1], 0., 0, "ONLY");
  gMC->Gspos("MD1V", 2, "MD1 ", 0., -2.98-boxpar[1], 0., 0, "ONLY");
    
  // --  YOKE 
  tubpar[0] = 3.48;
  tubpar[1] = 110./2.;
  tubpar[2] = 945./2.;
  gMC->Gsvolu("YD1 ", "TUBE", idtmed[7], tubpar, 3);
  
  gMC->Gspos("YD1 ", 1, "ZDCC", 0., 0., -tubpar[2]-zd1, 0, "ONLY");
  gMC->Gspos("MD1 ", 1, "YD1 ", 0., 0., 0., 0, "ONLY");
  // Ch debug
  //printf("\t **** D1 positioned! It goes from z = %1.2f to z = %1.2f cm\n",-zd1, -zd1-2*tubpar[2]); 
  
  // -- DIPOLE D2 
  // --- LHC optics v6.4
  zd2 = 12167.8;
  
  // --  GAP (VACUUM WITH MAGNETIC FIELD) 
  tubpar[0] = 0.;
  tubpar[1] = 7.5/2.;
  tubpar[2] = 945./2.;
  gMC->Gsvolu("MD2 ", "TUBE", idtmed[11], tubpar, 3);
  
  // --  YOKE 
  tubpar[0] = 0.;
  tubpar[1] = 55.;
  tubpar[2] = 945./2.;
  gMC->Gsvolu("YD2 ", "TUBE", idtmed[7], tubpar, 3);
  
  gMC->Gspos("YD2 ", 1, "ZDCC", 0., 0., -tubpar[2]-zd2, 0, "ONLY");
  
  gMC->Gspos("MD2 ", 1, "YD2 ", -9.4, 0., 0., 0, "ONLY");
  gMC->Gspos("MD2 ", 2, "YD2 ",  9.4, 0., 0., 0, "ONLY");
  
  // ***************************************************************  
  //		SIDE A - RB24 
  // ***************************************************************
  
  // COMPENSATOR DIPOLE (MCBWA) (2nd compensator)
  // --  GAP (VACUUM WITH MAGNETIC FIELD) 
  tubpar[0] = 0.;
  //tubpar[1] = 4.5;
  tubpar[1] = 3.;  
  tubpar[2] = 153./2.;
  gMC->Gsvolu("MCBW", "TUBE", idtmed[11], tubpar, 3);  
  gMC->Gspos("MCBW", 1, "ZDCA", 0., 0., tubpar[2]+1972.5, 0, "ONLY");
    
   // --  YOKE 
  tubpar[0] = 4.5;
  tubpar[1] = 55.;
  tubpar[2] = 153./2.;
  gMC->Gsvolu("YMCB", "TUBE", idtmed[7], tubpar, 3);
  gMC->Gspos("YMCB", 1, "ZDCA", 0., 0., tubpar[2]+1972.5, 0, "ONLY");  
  
  
   // -- INNER TRIPLET 
  zql = 2296.5; 

  // -- DEFINE MQX1 AND MQX2 QUADRUPOLE ELEMENT 
  // --  MQX1 
  // --  GAP (VACUUM WITH MAGNETIC FIELD) 
  tubpar[0] = 0.;
  tubpar[1] = 3.5;
  tubpar[2] = 637./2.;
  gMC->Gsvolu("MQX1", "TUBE", idtmed[11], tubpar, 3);
    
  // --  YOKE 
  tubpar[0] = 3.5;
  tubpar[1] = 22.;
  tubpar[2] = 637./2.;
  gMC->Gsvolu("YMQ1", "TUBE", idtmed[7], tubpar, 3);

   // -- BEAM SCREEN FOR Q1
   tubpar[0] = 4.78/2.;
   tubpar[1] = 5.18/2.;
   tubpar[2] = 637./2.;
   gMC->Gsvolu("QBS1", "TUBE", idtmed[6], tubpar, 3);
   gMC->Gspos("QBS1", 1, "ZDCA", 0., 0., tubpar[2]+zql, 0, "ONLY");
   // INSERT VERTICAL PLATE INSIDE Q1
   boxpar[0] = 0.2/2.0;
   boxpar[1] = TMath::Sqrt(tubpar[0]*tubpar[0]-(1.9+0.2)*(1.9+0.2));
   boxpar[2] =637./2.;
   gMC->Gsvolu("QBS2", "BOX ", idtmed[6], boxpar, 3);
   gMC->Gspos("QBS2", 1, "ZDCA", 1.9+boxpar[0], 0., boxpar[2]+zql, 0, "ONLY");
   gMC->Gspos("QBS2", 2, "ZDCA", -1.9-boxpar[0], 0., boxpar[2]+zql, 0, "ONLY");

   // -- BEAM SCREEN FOR Q3
   tubpar[0] = 5.79/2.;
   tubpar[1] = 6.14/2.;
   tubpar[2] = 637./2.;
   gMC->Gsvolu("QBS3", "TUBE", idtmed[6], tubpar, 3);
   gMC->Gspos("QBS3", 1, "ZDCA", 0., 0., tubpar[2]+zql+2400., 0, "ONLY");
   // INSERT VERTICAL PLATE INSIDE Q3
   boxpar[0] = 0.2/2.0;
   boxpar[1] = TMath::Sqrt(tubpar[0]*tubpar[0]-(2.405+0.2)*(2.405+0.2));
   boxpar[2] =637./2.;
   gMC->Gsvolu("QBS4", "BOX ", idtmed[6], boxpar, 3);
   gMC->Gspos("QBS4", 1, "ZDCA", 2.405+boxpar[0], 0., boxpar[2]+zql+2400., 0, "ONLY");
   gMC->Gspos("QBS4", 2, "ZDCA", -2.405-boxpar[0], 0., boxpar[2]+zql+2400., 0, "ONLY");
  
  // -- Q1
  gMC->Gspos("MQX1", 1, "ZDCA", 0., 0., tubpar[2]+zql, 0, "MANY");
  gMC->Gspos("YMQ1", 1, "ZDCA", 0., 0., tubpar[2]+zql, 0, "ONLY");
  
  // -- Q3
  gMC->Gspos("MQX1", 2, "ZDCA", 0., 0., tubpar[2]+zql+2400., 0, "MANY");
  gMC->Gspos("YMQ1", 2, "ZDCA", 0., 0., tubpar[2]+zql+2400., 0, "ONLY");  
  
  
  // --  MQX2
  // --  GAP (VACUUM WITH MAGNETIC FIELD) 
  tubpar[0] = 0.;
  tubpar[1] = 3.5;
  tubpar[2] = 550./2.;
  gMC->Gsvolu("MQX2", "TUBE", idtmed[11], tubpar, 3);
  
  // --  YOKE 
  tubpar[0] = 3.5;
  tubpar[1] = 22.;
  tubpar[2] = 550./2.;
  gMC->Gsvolu("YMQ2", "TUBE", idtmed[7], tubpar, 3);
  

   // -- BEAM SCREEN FOR Q2
   tubpar[0] = 5.79/2.;
   tubpar[1] = 6.14/2.;
   tubpar[2] = 550./2.;
   gMC->Gsvolu("QBS5", "TUBE", idtmed[6], tubpar, 3);
   //    VERTICAL PLATE INSIDE Q2
   boxpar[0] = 0.2/2.0;
   boxpar[1] = TMath::Sqrt(tubpar[0]*tubpar[0]-(2.405+0.2)*(2.405+0.2));
   boxpar[2] =550./2.;
   gMC->Gsvolu("QBS6", "BOX ", idtmed[6], boxpar, 3);

  // -- Q2A
  gMC->Gspos("MQX2", 1, "ZDCA", 0., 0., tubpar[2]+zql+908.5,  0, "MANY");
  gMC->Gspos("QBS5", 1, "ZDCA", 0., 0., tubpar[2]+zql+908.5, 0, "ONLY");  
  gMC->Gspos("QBS6", 1, "ZDCA", 2.405+boxpar[0], 0., boxpar[2]+zql+908.5, 0, "ONLY");
  gMC->Gspos("QBS6", 2, "ZDCA", -2.405-boxpar[0], 0., boxpar[2]+zql+908.5, 0, "ONLY");  
  gMC->Gspos("YMQ2", 1, "ZDCA", 0., 0., tubpar[2]+zql+908.5,  0, "ONLY");

  
  // -- Q2B
  gMC->Gspos("MQX2", 2, "ZDCA", 0., 0., tubpar[2]+zql+1558.5, 0, "MANY");
  gMC->Gspos("QBS5", 2, "ZDCA", 0., 0., tubpar[2]+zql+1558.5, 0, "ONLY");  
  gMC->Gspos("QBS6", 3, "ZDCA", 2.405+boxpar[0], 0., boxpar[2]+zql+1558.5, 0, "ONLY");
  gMC->Gspos("QBS6", 4, "ZDCA", -2.405-boxpar[0], 0., boxpar[2]+zql+1558.5, 0, "ONLY");
  gMC->Gspos("YMQ2", 2, "ZDCA", 0., 0., tubpar[2]+zql+1558.5, 0, "ONLY");

    // -- SEPARATOR DIPOLE D1 
  zd2 = 5838.3;
  
  // --  GAP (VACUUM WITH MAGNETIC FIELD) 
  tubpar[0] = 0.;
  tubpar[1] = 6.75/2.;
  tubpar[2] = 945./2.;
  gMC->Gsvolu("MD1L", "TUBE", idtmed[11], tubpar, 3);
  
  // --  The beam screen tube is provided by the beam pipe in D1 (QA03 volume)
  // --  Insert the beam screen horizontal Cu plates inside D1  
  // --   (to simulate the vacuum chamber)
  boxpar[0] = TMath::Sqrt(tubpar[1]*tubpar[1]-(2.885+0.2)*(2.885+0.2));
  boxpar[1] = 0.2/2.;
  boxpar[2] =(945.+80.1)/2.;
  gMC->Gsvolu("QBS7", "BOX ", idtmed[6], boxpar, 3);
  gMC->Gspos("QBS7", 1, "ZDCA", 0., 2.885+boxpar[1],boxpar[2]+zd2, 0, "ONLY");
  gMC->Gspos("QBS7", 2, "ZDCA", 0., -2.885-boxpar[1],boxpar[2]+zd2, 0, "ONLY");
    
  // --  YOKE 
  tubpar[0] = 7.34/2.; // to be checked
  tubpar[1] = 110./2;
  tubpar[2] = 945./2.;
  gMC->Gsvolu("YD1L", "TUBE", idtmed[7], tubpar, 3);
  
  gMC->Gspos("YD1L", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "ONLY");  
  gMC->Gspos("MD1L", 1, "ZDCA", 0., 0., tubpar[2]+zd2, 0, "MANY");

  
  // -- DIPOLE D2 
  // --- LHC optics v6.5
  zd2l = 12167.8;
  
  // --  GAP (VACUUM WITH MAGNETIC FIELD) 
  tubpar[0] = 0.;
  tubpar[1] = 7.5/2.; // this has to be checked
  tubpar[2] = 945./2.;
  gMC->Gsvolu("MD2L", "TUBE", idtmed[11], tubpar, 3);
  
  // --  YOKE 
  tubpar[0] = 0.;
  tubpar[1] = 55.;
  tubpar[2] = 945./2.;
  gMC->Gsvolu("YD2L", "TUBE", idtmed[7], tubpar, 3);
  
  gMC->Gspos("YD2L", 1, "ZDCA", 0., 0., tubpar[2]+zd2l, 0, "ONLY");
  
  gMC->Gspos("MD2L", 1, "YD2L", -9.4, 0., 0., 0, "ONLY");
  gMC->Gspos("MD2L", 2, "YD2L",  9.4, 0., 0., 0, "ONLY");
  
  // -- END OF MAGNET DEFINITION     
}
  
//_____________________________________________________________________________
void AliZDCv3::CreateZDC()
{
 //
 // Create the various ZDCs (ZN + ZP)
 //
  
  Float_t dimPb[6], dimVoid[6];
  
  Int_t *idtmed = fIdtmed->GetArray();

  // Parameters for hadronic calorimeters geometry
  // NB -> parameters used ONLY in CreateZDC()
  Float_t fGrvZN[3] = {0.03, 0.03, 50.};  // Grooves for neutron detector
  Float_t fGrvZP[3] = {0.04, 0.04, 75.};  // Grooves for proton detector
  Int_t   fDivZN[3] = {11, 11, 0};  	  // Division for neutron detector
  Int_t   fDivZP[3] = {7, 15, 0};  	  // Division for proton detector
  Int_t   fTowZN[2] = {2, 2};  		  // Tower for neutron detector
  Int_t   fTowZP[2] = {4, 1};  		  // Tower for proton detector

  // Parameters for EM calorimeter geometry
  // NB -> parameters used ONLY in CreateZDC()
  Float_t kDimZEMPb  = 0.15*(TMath::Sqrt(2.));  // z-dimension of the Pb slice
  Float_t kFibRadZEM = 0.0315; 			// External fiber radius (including cladding)
  Int_t   fDivZEM[3] = {92, 0, 20}; 		// Divisions for EM detector
  Float_t fDimZEM[6] = {fZEMLength, 3.5, 3.5, 45., 0., 0.}; // Dimensions of EM detector
  Float_t fFibZEM2 = fDimZEM[2]/TMath::Sin(fDimZEM[3]*kDegrad)-kFibRadZEM;
  Float_t fFibZEM[3] = {0., 0.0275, fFibZEM2};  // Fibers for EM calorimeter

  
  //-- Create calorimeters geometry
  
  // -------------------------------------------------------------------------------
  //--> Neutron calorimeter (ZN) 
  
  gMC->Gsvolu("ZNEU", "BOX ", idtmed[1], fDimZN, 3); // Passive material  
  gMC->Gsvolu("ZNF1", "TUBE", idtmed[3], fFibZN, 3); // Active material
  gMC->Gsvolu("ZNF2", "TUBE", idtmed[4], fFibZN, 3); 
  gMC->Gsvolu("ZNF3", "TUBE", idtmed[4], fFibZN, 3); 
  gMC->Gsvolu("ZNF4", "TUBE", idtmed[3], fFibZN, 3); 
  gMC->Gsvolu("ZNG1", "BOX ", idtmed[12], fGrvZN, 3); // Empty grooves 
  gMC->Gsvolu("ZNG2", "BOX ", idtmed[12], fGrvZN, 3); 
  gMC->Gsvolu("ZNG3", "BOX ", idtmed[12], fGrvZN, 3); 
  gMC->Gsvolu("ZNG4", "BOX ", idtmed[12], fGrvZN, 3); 
  
  // Divide ZNEU in towers (for hits purposes) 
  
  gMC->Gsdvn("ZNTX", "ZNEU", fTowZN[0], 1); // x-tower 
  gMC->Gsdvn("ZN1 ", "ZNTX", fTowZN[1], 2); // y-tower
  
  //-- Divide ZN1 in minitowers 
  //  fDivZN[0]= NUMBER OF FIBERS PER TOWER ALONG X-AXIS, 
  //  fDivZN[1]= NUMBER OF FIBERS PER TOWER ALONG Y-AXIS
  //  (4 fibres per minitower) 
  
  gMC->Gsdvn("ZNSL", "ZN1 ", fDivZN[1], 2); // Slices 
  gMC->Gsdvn("ZNST", "ZNSL", fDivZN[0], 1); // Sticks
  
  // --- Position the empty grooves in the sticks (4 grooves per stick)
  Float_t dx = fDimZN[0] / fDivZN[0] / 4.;
  Float_t dy = fDimZN[1] / fDivZN[1] / 4.;
  
  gMC->Gspos("ZNG1", 1, "ZNST", 0.-dx, 0.+dy, 0., 0, "ONLY");
  gMC->Gspos("ZNG2", 1, "ZNST", 0.+dx, 0.+dy, 0., 0, "ONLY");
  gMC->Gspos("ZNG3", 1, "ZNST", 0.-dx, 0.-dy, 0., 0, "ONLY");
  gMC->Gspos("ZNG4", 1, "ZNST", 0.+dx, 0.-dy, 0., 0, "ONLY");
  
  // --- Position the fibers in the grooves 
  gMC->Gspos("ZNF1", 1, "ZNG1", 0., 0., 0., 0, "ONLY");
  gMC->Gspos("ZNF2", 1, "ZNG2", 0., 0., 0., 0, "ONLY");
  gMC->Gspos("ZNF3", 1, "ZNG3", 0., 0., 0., 0, "ONLY");
  gMC->Gspos("ZNF4", 1, "ZNG4", 0., 0., 0., 0, "ONLY");
  
  // --- Position the neutron calorimeter in ZDC 
  // -- Rotation of ZDCs
  Int_t irotzdc;
  gMC->Matrix(irotzdc, 90., 180., 90., 90., 180., 0.);
  //
  gMC->Gspos("ZNEU", 1, "ZDCC", fPosZNC[0], fPosZNC[1], fPosZNC[2]-fDimZN[2], irotzdc, "ONLY");
  //Ch debug
  //printf("\n ZN -> %f < z < %f cm\n",fPosZN[2],fPosZN[2]-2*fDimZN[2]);

  // --- Position the neutron calorimeter in ZDC2 (left line) 
  // -- No Rotation of ZDCs
  gMC->Gspos("ZNEU", 2, "ZDCA", fPosZNA[0], fPosZNA[1], fPosZNA[2]+fDimZN[2], 0, "ONLY");
  //Ch debug
  //printf("\n ZN left -> %f < z < %f cm\n",fPosZNl[2],fPosZNl[2]+2*fDimZN[2]);


  // -------------------------------------------------------------------------------
  //--> Proton calorimeter (ZP)  
  
  gMC->Gsvolu("ZPRO", "BOX ", idtmed[2], fDimZP, 3); // Passive material
  gMC->Gsvolu("ZPF1", "TUBE", idtmed[3], fFibZP, 3); // Active material
  gMC->Gsvolu("ZPF2", "TUBE", idtmed[4], fFibZP, 3); 
  gMC->Gsvolu("ZPF3", "TUBE", idtmed[4], fFibZP, 3); 
  gMC->Gsvolu("ZPF4", "TUBE", idtmed[3], fFibZP, 3); 
  gMC->Gsvolu("ZPG1", "BOX ", idtmed[12], fGrvZP, 3); // Empty grooves 
  gMC->Gsvolu("ZPG2", "BOX ", idtmed[12], fGrvZP, 3); 
  gMC->Gsvolu("ZPG3", "BOX ", idtmed[12], fGrvZP, 3); 
  gMC->Gsvolu("ZPG4", "BOX ", idtmed[12], fGrvZP, 3); 
    
  //-- Divide ZPRO in towers(for hits purposes) 
  
  gMC->Gsdvn("ZPTX", "ZPRO", fTowZP[0], 1); // x-tower 
  gMC->Gsdvn("ZP1 ", "ZPTX", fTowZP[1], 2); // y-tower
  
  
  //-- Divide ZP1 in minitowers 
  //  fDivZP[0]= NUMBER OF FIBERS ALONG X-AXIS PER MINITOWER, 
  //  fDivZP[1]= NUMBER OF FIBERS ALONG Y-AXIS PER MINITOWER
  //  (4 fiber per minitower) 
  
  gMC->Gsdvn("ZPSL", "ZP1 ", fDivZP[1], 2); // Slices 
  gMC->Gsdvn("ZPST", "ZPSL", fDivZP[0], 1); // Sticks
  
  // --- Position the empty grooves in the sticks (4 grooves per stick)
  dx = fDimZP[0] / fTowZP[0] / fDivZP[0] / 2.;
  dy = fDimZP[1] / fTowZP[1] / fDivZP[1] / 2.;
  
  gMC->Gspos("ZPG1", 1, "ZPST", 0.-dx, 0.+dy, 0., 0, "ONLY");
  gMC->Gspos("ZPG2", 1, "ZPST", 0.+dx, 0.+dy, 0., 0, "ONLY");
  gMC->Gspos("ZPG3", 1, "ZPST", 0.-dx, 0.-dy, 0., 0, "ONLY");
  gMC->Gspos("ZPG4", 1, "ZPST", 0.+dx, 0.-dy, 0., 0, "ONLY");
  
  // --- Position the fibers in the grooves 
  gMC->Gspos("ZPF1", 1, "ZPG1", 0., 0., 0., 0, "ONLY");
  gMC->Gspos("ZPF2", 1, "ZPG2", 0., 0., 0., 0, "ONLY");
  gMC->Gspos("ZPF3", 1, "ZPG3", 0., 0., 0., 0, "ONLY");
  gMC->Gspos("ZPF4", 1, "ZPG4", 0., 0., 0., 0, "ONLY");
  

  // --- Position the proton calorimeter in ZDCC
  gMC->Gspos("ZPRO", 1, "ZDCC", fPosZPC[0], fPosZPC[1], fPosZPC[2]-fDimZP[2], irotzdc, "ONLY");
  //Ch debug
  //printf("\n ZP -> %f < z < %f cm\n",fPosZP[2],fPosZP[2]-2*fDimZP[2]);
  
  // --- Position the proton calorimeter in ZDCA
  // --- No rotation 
  gMC->Gspos("ZPRO", 2, "ZDCA", fPosZPA[0], fPosZPA[1], fPosZPA[2]+fDimZP[2], 0, "ONLY");
  //Ch debug
  //printf("\n ZP left -> %f < z < %f cm\n",fPosZPl[2],fPosZPl[2]+2*fDimZP[2]);  
    
  
  // -------------------------------------------------------------------------------
  // -> EM calorimeter (ZEM)  
  
  gMC->Gsvolu("ZEM ", "PARA", idtmed[10], fDimZEM, 6);

  Int_t irot1, irot2;
  gMC->Matrix(irot1,0.,0.,90.,90.,-90.,0.); 		       // Rotation matrix 1  
  gMC->Matrix(irot2,180.,0.,90.,fDimZEM[3]+90.,90.,fDimZEM[3]);// Rotation matrix 2
  //printf("irot1 = %d, irot2 = %d \n", irot1, irot2);
  
  gMC->Gsvolu("ZEMF", "TUBE", idtmed[3], fFibZEM, 3); 	// Active material

  gMC->Gsdvn("ZETR", "ZEM ", fDivZEM[2], 1); 	     	// Tranches 
  
  dimPb[0] = kDimZEMPb;					// Lead slices 
  dimPb[1] = fDimZEM[2];
  dimPb[2] = fDimZEM[1];
  //dimPb[3] = fDimZEM[3]; //controllare
  dimPb[3] = 90.-fDimZEM[3]; //originale
  dimPb[4] = 0.;
  dimPb[5] = 0.;
  gMC->Gsvolu("ZEL0", "PARA", idtmed[5], dimPb, 6);
  gMC->Gsvolu("ZEL1", "PARA", idtmed[5], dimPb, 6);
  gMC->Gsvolu("ZEL2", "PARA", idtmed[5], dimPb, 6);
  
  // --- Position the lead slices in the tranche 
  Float_t zTran = fDimZEM[0]/fDivZEM[2]; 
  Float_t zTrPb = -zTran+kDimZEMPb;
  gMC->Gspos("ZEL0", 1, "ZETR", zTrPb, 0., 0., 0, "ONLY");
  gMC->Gspos("ZEL1", 1, "ZETR", kDimZEMPb, 0., 0., 0, "ONLY");
  
  // --- Vacuum zone (to be filled with fibres)
  dimVoid[0] = (zTran-2*kDimZEMPb)/2.;
  dimVoid[1] = fDimZEM[2];
  dimVoid[2] = fDimZEM[1];
  dimVoid[3] = 90.-fDimZEM[3];
  dimVoid[4] = 0.;
  dimVoid[5] = 0.;
  gMC->Gsvolu("ZEV0", "PARA", idtmed[10], dimVoid,6);
  gMC->Gsvolu("ZEV1", "PARA", idtmed[10], dimVoid,6);
  
  // --- Divide the vacuum slice into sticks along x axis
  gMC->Gsdvn("ZES0", "ZEV0", fDivZEM[0], 3); 
  gMC->Gsdvn("ZES1", "ZEV1", fDivZEM[0], 3); 
  
  // --- Positioning the fibers into the sticks
  gMC->Gspos("ZEMF", 1,"ZES0", 0., 0., 0., irot2, "ONLY");
  gMC->Gspos("ZEMF", 1,"ZES1", 0., 0., 0., irot2, "ONLY");
  
  // --- Positioning the vacuum slice into the tranche
  //Float_t displFib = fDimZEM[1]/fDivZEM[0];
  gMC->Gspos("ZEV0", 1,"ZETR", -dimVoid[0], 0., 0., 0, "ONLY");
  gMC->Gspos("ZEV1", 1,"ZETR", -dimVoid[0]+zTran, 0., 0., 0, "ONLY");

  // --- Positioning the ZEM into the ZDC - rotation for 90 degrees  
  // NB -> ZEM is positioned in ALIC (instead of in ZDC) volume
  gMC->Gspos("ZEM ", 1,"ALIC", -fPosZEM[0], fPosZEM[1], fPosZEM[2]+fDimZEM[0], irot1, "ONLY");
  
  // Second EM ZDC (same side w.r.t. IP, just on the other side w.r.t. beam pipe)
  gMC->Gspos("ZEM ", 2,"ALIC", fPosZEM[0], fPosZEM[1], fPosZEM[2]+fDimZEM[0], irot1, "ONLY");
  
  // --- Adding last slice at the end of the EM calorimeter 
  Float_t zLastSlice = fPosZEM[2]+kDimZEMPb+2*fDimZEM[0];
  gMC->Gspos("ZEL2", 1,"ALIC", fPosZEM[0], fPosZEM[1], zLastSlice, irot1, "ONLY");
  //Ch debug
  //printf("\n ZEM lenght = %f cm\n",2*fZEMLength);
  //printf("\n ZEM -> %f < z < %f cm\n",fPosZEM[2],fPosZEM[2]+2*fZEMLength+zLastSlice+kDimZEMPb);
  
}
 
//_____________________________________________________________________________
void AliZDCv3::DrawModule() const
{
  //
  // Draw a shaded view of the Zero Degree Calorimeter version 1
  //

  // Set everything unseen
  gMC->Gsatt("*", "seen", -1);
  // 
  // Set ALIC mother transparent
  gMC->Gsatt("ALIC","SEEN",0);
  //
  // Set the volumes visible
  gMC->Gsatt("ZDCC","SEEN",0);
  gMC->Gsatt("QT01","SEEN",1);
  gMC->Gsatt("QT02","SEEN",1);
  gMC->Gsatt("QT03","SEEN",1);
  gMC->Gsatt("QT04","SEEN",1);
  gMC->Gsatt("QT05","SEEN",1);
  gMC->Gsatt("QT06","SEEN",1);
  gMC->Gsatt("QT07","SEEN",1);
  gMC->Gsatt("QT08","SEEN",1);
  gMC->Gsatt("QT09","SEEN",1);
  gMC->Gsatt("QT10","SEEN",1);
  gMC->Gsatt("QT11","SEEN",1);
  gMC->Gsatt("QT12","SEEN",1);
  gMC->Gsatt("QT13","SEEN",1);
  gMC->Gsatt("QT14","SEEN",1);
  gMC->Gsatt("QT15","SEEN",1);
  gMC->Gsatt("QT16","SEEN",1);
  gMC->Gsatt("QT17","SEEN",1);
  gMC->Gsatt("QT18","SEEN",1);
  gMC->Gsatt("QC01","SEEN",1);
  gMC->Gsatt("QC02","SEEN",1);
  gMC->Gsatt("QC03","SEEN",1);
  gMC->Gsatt("QC04","SEEN",1);
  gMC->Gsatt("QC05","SEEN",1);
  gMC->Gsatt("QTD1","SEEN",1);
  gMC->Gsatt("QTD2","SEEN",1);
  gMC->Gsatt("QTD3","SEEN",1);
  gMC->Gsatt("MQXL","SEEN",1);
  gMC->Gsatt("YMQL","SEEN",1);
  gMC->Gsatt("MQX ","SEEN",1);
  gMC->Gsatt("YMQ ","SEEN",1);
  gMC->Gsatt("ZQYX","SEEN",1);
  gMC->Gsatt("MD1 ","SEEN",1);
  gMC->Gsatt("MD1V","SEEN",1);
  gMC->Gsatt("YD1 ","SEEN",1);
  gMC->Gsatt("MD2 ","SEEN",1);
  gMC->Gsatt("YD2 ","SEEN",1);
  gMC->Gsatt("ZNEU","SEEN",0);
  gMC->Gsatt("ZNF1","SEEN",0);
  gMC->Gsatt("ZNF2","SEEN",0);
  gMC->Gsatt("ZNF3","SEEN",0);
  gMC->Gsatt("ZNF4","SEEN",0);
  gMC->Gsatt("ZNG1","SEEN",0);
  gMC->Gsatt("ZNG2","SEEN",0);
  gMC->Gsatt("ZNG3","SEEN",0);
  gMC->Gsatt("ZNG4","SEEN",0);
  gMC->Gsatt("ZNTX","SEEN",0);
  gMC->Gsatt("ZN1 ","COLO",4); 
  gMC->Gsatt("ZN1 ","SEEN",1);
  gMC->Gsatt("ZNSL","SEEN",0);
  gMC->Gsatt("ZNST","SEEN",0);
  gMC->Gsatt("ZPRO","SEEN",0);
  gMC->Gsatt("ZPF1","SEEN",0);
  gMC->Gsatt("ZPF2","SEEN",0);
  gMC->Gsatt("ZPF3","SEEN",0);
  gMC->Gsatt("ZPF4","SEEN",0);
  gMC->Gsatt("ZPG1","SEEN",0);
  gMC->Gsatt("ZPG2","SEEN",0);
  gMC->Gsatt("ZPG3","SEEN",0);
  gMC->Gsatt("ZPG4","SEEN",0);
  gMC->Gsatt("ZPTX","SEEN",0);
  gMC->Gsatt("ZP1 ","COLO",6); 
  gMC->Gsatt("ZP1 ","SEEN",1);
  gMC->Gsatt("ZPSL","SEEN",0);
  gMC->Gsatt("ZPST","SEEN",0);
  gMC->Gsatt("ZEM ","COLO",7); 
  gMC->Gsatt("ZEM ","SEEN",1);
  gMC->Gsatt("ZEMF","SEEN",0);
  gMC->Gsatt("ZETR","SEEN",0);
  gMC->Gsatt("ZEL0","SEEN",0);
  gMC->Gsatt("ZEL1","SEEN",0);
  gMC->Gsatt("ZEL2","SEEN",0);
  gMC->Gsatt("ZEV0","SEEN",0);
  gMC->Gsatt("ZEV1","SEEN",0);
  gMC->Gsatt("ZES0","SEEN",0);
  gMC->Gsatt("ZES1","SEEN",0);
  
  //
  gMC->Gdopt("hide", "on");
  gMC->Gdopt("shad", "on");
  gMC->Gsatt("*", "fill", 7);
  gMC->SetClipBox(".");
  gMC->SetClipBox("*", 0, 100, -100, 100, 12000, 16000);
  gMC->DefaultRange();
  gMC->Gdraw("alic", 40, 30, 0, 488, 220, .07, .07);
  gMC->Gdhead(1111, "Zero Degree Calorimeter Version 3");
  gMC->Gdman(18, 4, "MAN");
}

//_____________________________________________________________________________
void AliZDCv3::CreateMaterials()
{
  //
  // Create Materials for the Zero Degree Calorimeter
  //
  
  Float_t dens, ubuf[1], wmat[2], a[2], z[2];

  // --- Store in UBUF r0 for nuclear radius calculation R=r0*A**1/3 

  // --- Tantalum -> ZN passive material
  ubuf[0] = 1.1;
  AliMaterial(1, "TANT", 180.95, 73., 16.65, .4, 11.9, ubuf, 1);
  
  // --- Brass (CuZn)  -> ZP passive material
  dens = 8.48;
  a[0] = 63.546;
  a[1] = 65.39;
  z[0] = 29.;
  z[1] = 30.;
  wmat[0] = .63;
  wmat[1] = .37;
  AliMixture(2, "BRASS", a, z, dens, 2, wmat);
  
  // --- SiO2 
  dens = 2.64;
  a[0] = 28.086;
  a[1] = 15.9994;
  z[0] = 14.;
  z[1] = 8.;
  wmat[0] = 1.;
  wmat[1] = 2.;
  AliMixture(3, "SIO2", a, z, dens, -2, wmat);  
  
  // --- Lead 
  ubuf[0] = 1.12;
  AliMaterial(5, "LEAD", 207.19, 82., 11.35, .56, 18.5, ubuf, 1);

  // --- Copper (energy loss taken into account)
  ubuf[0] = 1.10;
  AliMaterial(6, "COPP0", 63.54, 29., 8.96, 1.4, 0., ubuf, 1);
  
  // --- Iron (energy loss taken into account)
  ubuf[0] = 1.1;
  AliMaterial(7, "IRON0", 55.85, 26., 7.87, 1.76, 0., ubuf, 1);
  
  // --- Iron (no energy loss)
  ubuf[0] = 1.1;
  AliMaterial(8,  "IRON1", 55.85, 26., 7.87, 1.76, 0., ubuf, 1);
  AliMaterial(13, "IRON2", 55.85, 26., 7.87, 1.76, 0., ubuf, 1);
    
  // ---------------------------------------------------------  
  Float_t aResGas[3]={1.008,12.0107,15.9994};
  Float_t zResGas[3]={1.,6.,8.};
  Float_t wResGas[3]={0.28,0.28,0.44};
  Float_t dResGas = 3.2E-14;

  // --- Vacuum (no magnetic field) 
  AliMixture(10, "VOID", aResGas, zResGas, dResGas, 3, wResGas);
  
  // --- Vacuum (with magnetic field) 
  AliMixture(11, "VOIM", aResGas, zResGas, dResGas, 3, wResGas);
  
  // --- Air (no magnetic field)
  Float_t aAir[4]={12.0107,14.0067,15.9994,39.948};
  Float_t zAir[4]={6.,7.,8.,18.};
  Float_t wAir[4]={0.000124,0.755267,0.231781,0.012827};
  Float_t dAir = 1.20479E-3;
  //
  AliMixture(12, "Air    $", aAir, zAir, dAir, 4, wAir);
  
  // ---  Definition of tracking media: 
  
  // --- Tantalum = 1 ; 
  // --- Brass = 2 ; 
  // --- Fibers (SiO2) = 3 ; 
  // --- Fibers (SiO2) = 4 ; 
  // --- Lead = 5 ; 
  // --- Copper (with energy loss)= 6 ;
  // --- Copper (with energy loss)= 13 ; 
  // --- Iron (with energy loss) = 7 ; 
  // --- Iron (without energy loss) = 8 ; 
  // --- Vacuum (no field) = 10 
  // --- Vacuum (with field) = 11 
  // --- Air (no field) = 12 
  
  // **************************************************** 
  //     Tracking media parameters
  //
  Float_t epsil  = 0.01;   // Tracking precision, 
  Float_t stmin  = 0.01;   // Min. value 4 max. step (cm)
  Float_t stemax = 1.;     // Max. step permitted (cm) 
  Float_t tmaxfd = 0.;     // Maximum angle due to field (degrees) 
  Float_t deemax = -1.;    // Maximum fractional energy loss
  Float_t nofieldm = 0.;   // Max. field value (no field)
  Float_t fieldm = 45.;    // Max. field value (with field)
  Int_t isvol = 0;         // ISVOL =0 -> not sensitive volume
  Int_t isvolActive = 1;   // ISVOL =1 -> sensitive volume
  Int_t inofld = 0;        // IFIELD=0 -> no magnetic field
  Int_t ifield =2;         // IFIELD=2 -> magnetic field defined in AliMagFC.h
  // *****************************************************
  
  AliMedium(1, "ZTANT", 1, isvolActive, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(2, "ZBRASS",2, isvolActive, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(3, "ZSIO2", 3, isvolActive, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(4, "ZQUAR", 3, isvolActive, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(5, "ZLEAD", 5, isvolActive, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(6, "ZCOPP", 6, isvol, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(7, "ZIRON", 7, isvol, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(8, "ZIRONN",8, isvol, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(10,"ZVOID",10, isvol, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(12,"ZAIR", 12, isvol, inofld, nofieldm, tmaxfd, stemax, deemax, epsil, stmin);
  //
  AliMedium(11,"ZVOIM",11, isvol, ifield, fieldm, tmaxfd, stemax, deemax, epsil, stmin);
  AliMedium(13,"ZIRONE",13, isvol, ifield, fieldm, tmaxfd, stemax, deemax, epsil, stmin);  

} 

//_____________________________________________________________________________
void AliZDCv3::AddAlignableVolumes() const
{
 //
 // Create entries for alignable volumes associating the symbolic volume
 // name with the corresponding volume path. Needs to be syncronized with
 // eventual changes in the geometry.
 //
 TString volpath1 = "ALIC_1/ZDCC_1/ZNEU_1";
 TString volpath2 = "ALIC_1/ZDCC_1/ZPRO_1";
 TString volpath3 = "ALIC_1/ZDCA_1/ZNEU_2";
 TString volpath4 = "ALIC_1/ZDCA_1/ZPRO_2";

 TString symname1="ZDC/NeutronZDC_C";
 TString symname2="ZDC/ProtonZDC_C";
 TString symname3="ZDC/NeutronZDC_A";
 TString symname4="ZDC/ProtonZDC_A";

 if(!gGeoManager->SetAlignableEntry(symname1.Data(),volpath1.Data()))
     AliFatal(Form("Alignable entry %s not created. Volume path %s not valid",   symname1.Data(),volpath1.Data()));

 if(!gGeoManager->SetAlignableEntry(symname2.Data(),volpath2.Data()))
     AliFatal(Form("Alignable entry %s not created. Volume path %s not valid",   symname2.Data(),volpath2.Data()));

 if(!gGeoManager->SetAlignableEntry(symname3.Data(),volpath3.Data()))
     AliFatal(Form("Alignable entry %s not created. Volume path %s not valid",   symname1.Data(),volpath1.Data()));

 if(!gGeoManager->SetAlignableEntry(symname4.Data(),volpath4.Data()))
     AliFatal(Form("Alignable entry %s not created. Volume path %s not valid",   symname2.Data(),volpath2.Data()));

}


//_____________________________________________________________________________
void AliZDCv3::Init()
{
 InitTables();
  Int_t *idtmed = fIdtmed->GetArray();  
  Int_t i;
  // Thresholds for showering in the ZDCs 
  i = 1; //tantalum
  gMC->Gstpar(idtmed[i], "CUTGAM", .001);
  gMC->Gstpar(idtmed[i], "CUTELE", .001);
  gMC->Gstpar(idtmed[i], "CUTNEU", .01);
  gMC->Gstpar(idtmed[i], "CUTHAD", .01);
  i = 2; //brass
  gMC->Gstpar(idtmed[i], "CUTGAM", .001);
  gMC->Gstpar(idtmed[i], "CUTELE", .001);
  gMC->Gstpar(idtmed[i], "CUTNEU", .01);
  gMC->Gstpar(idtmed[i], "CUTHAD", .01);
  i = 5; //lead
  gMC->Gstpar(idtmed[i], "CUTGAM", .001);
  gMC->Gstpar(idtmed[i], "CUTELE", .001);
  gMC->Gstpar(idtmed[i], "CUTNEU", .01);
  gMC->Gstpar(idtmed[i], "CUTHAD", .01);
  
  // Avoid too detailed showering in TDI 
  i = 6; //copper
  gMC->Gstpar(idtmed[i], "CUTGAM", .1);
  gMC->Gstpar(idtmed[i], "CUTELE", .1);
  gMC->Gstpar(idtmed[i], "CUTNEU", 1.);
  gMC->Gstpar(idtmed[i], "CUTHAD", 1.);
  
  // Avoid too detailed showering along the beam line 
  i = 7; //iron with energy loss (ZIRON)
  gMC->Gstpar(idtmed[i], "CUTGAM", .1);
  gMC->Gstpar(idtmed[i], "CUTELE", .1);
  gMC->Gstpar(idtmed[i], "CUTNEU", 1.);
  gMC->Gstpar(idtmed[i], "CUTHAD", 1.);
  
  // Avoid too detailed showering along the beam line 
  i = 8; //iron with energy loss (ZIRONN)
  gMC->Gstpar(idtmed[i], "CUTGAM", .1);
  gMC->Gstpar(idtmed[i], "CUTELE", .1);
  gMC->Gstpar(idtmed[i], "CUTNEU", 1.);
  gMC->Gstpar(idtmed[i], "CUTHAD", 1.);
  
  // Avoid too detailed showering along the beam line 
  i = 13; //iron with energy loss (ZIRONN)
  gMC->Gstpar(idtmed[i], "CUTGAM", 1.);
  gMC->Gstpar(idtmed[i], "CUTELE", 1.);
  gMC->Gstpar(idtmed[i], "CUTNEU", 1.);
  gMC->Gstpar(idtmed[i], "CUTHAD", 1.);
  
  // Avoid interaction in fibers (only energy loss allowed) 
  i = 3; //fibers (ZSI02)
  gMC->Gstpar(idtmed[i], "DCAY", 0.);
  gMC->Gstpar(idtmed[i], "MULS", 0.);
  gMC->Gstpar(idtmed[i], "PFIS", 0.);
  gMC->Gstpar(idtmed[i], "MUNU", 0.);
  gMC->Gstpar(idtmed[i], "LOSS", 1.);
  gMC->Gstpar(idtmed[i], "PHOT", 0.);
  gMC->Gstpar(idtmed[i], "COMP", 0.);
  gMC->Gstpar(idtmed[i], "PAIR", 0.);
  gMC->Gstpar(idtmed[i], "BREM", 0.);
  gMC->Gstpar(idtmed[i], "DRAY", 0.);
  gMC->Gstpar(idtmed[i], "ANNI", 0.);
  gMC->Gstpar(idtmed[i], "HADR", 0.);
  i = 4; //fibers (ZQUAR)
  gMC->Gstpar(idtmed[i], "DCAY", 0.);
  gMC->Gstpar(idtmed[i], "MULS", 0.);
  gMC->Gstpar(idtmed[i], "PFIS", 0.);
  gMC->Gstpar(idtmed[i], "MUNU", 0.);
  gMC->Gstpar(idtmed[i], "LOSS", 1.);
  gMC->Gstpar(idtmed[i], "PHOT", 0.);
  gMC->Gstpar(idtmed[i], "COMP", 0.);
  gMC->Gstpar(idtmed[i], "PAIR", 0.);
  gMC->Gstpar(idtmed[i], "BREM", 0.);
  gMC->Gstpar(idtmed[i], "DRAY", 0.);
  gMC->Gstpar(idtmed[i], "ANNI", 0.);
  gMC->Gstpar(idtmed[i], "HADR", 0.);
  
  // Avoid interaction in void 
  i = 11; //void with field
  gMC->Gstpar(idtmed[i], "DCAY", 0.);
  gMC->Gstpar(idtmed[i], "MULS", 0.);
  gMC->Gstpar(idtmed[i], "PFIS", 0.);
  gMC->Gstpar(idtmed[i], "MUNU", 0.);
  gMC->Gstpar(idtmed[i], "LOSS", 0.);
  gMC->Gstpar(idtmed[i], "PHOT", 0.);
  gMC->Gstpar(idtmed[i], "COMP", 0.);
  gMC->Gstpar(idtmed[i], "PAIR", 0.);
  gMC->Gstpar(idtmed[i], "BREM", 0.);
  gMC->Gstpar(idtmed[i], "DRAY", 0.);
  gMC->Gstpar(idtmed[i], "ANNI", 0.);
  gMC->Gstpar(idtmed[i], "HADR", 0.);

  //
  fMedSensZN  = idtmed[1];  // Sensitive volume: ZN passive material
  fMedSensZP  = idtmed[2];  // Sensitive volume: ZP passive material
  fMedSensF1  = idtmed[3];  // Sensitive volume: fibres type 1
  fMedSensF2  = idtmed[4];  // Sensitive volume: fibres type 2
  fMedSensZEM = idtmed[5];  // Sensitive volume: ZEM passive material
  fMedSensTDI = idtmed[6];  // Sensitive volume: TDI Cu shield
  fMedSensPI  = idtmed[7];  // Sensitive volume: beam pipes
  fMedSensGR  = idtmed[12]; // Sensitive volume: air into the grooves
}

//_____________________________________________________________________________
void AliZDCv3::InitTables()
{
 //
 // Read light tables for Cerenkov light production parameterization 
 //

  Int_t k, j;

  char *lightfName1,*lightfName2,*lightfName3,*lightfName4,
       *lightfName5,*lightfName6,*lightfName7,*lightfName8;
  FILE *fp1, *fp2, *fp3, *fp4, *fp5, *fp6, *fp7, *fp8;

  //  --- Reading light tables for ZN 
  lightfName1 = gSystem->ExpandPathName("$ALICE_ROOT/ZDC/light22620362207s");
  if((fp1 = fopen(lightfName1,"r")) == NULL){
     printf("Cannot open file fp1 \n");
     return;
  }
  lightfName2 = gSystem->ExpandPathName("$ALICE_ROOT/ZDC/light22620362208s");
  if((fp2 = fopen(lightfName2,"r")) == NULL){
     printf("Cannot open file fp2 \n");
     return;
  }  
  lightfName3 = gSystem->ExpandPathName("$ALICE_ROOT/ZDC/light22620362209s");
  if((fp3 = fopen(lightfName3,"r")) == NULL){
     printf("Cannot open file fp3 \n");
     return;
  }
  lightfName4 = gSystem->ExpandPathName("$ALICE_ROOT/ZDC/light22620362210s");
  if((fp4 = fopen(lightfName4,"r")) == NULL){
     printf("Cannot open file fp4 \n");
     return;
  }
  
  for(k=0; k<fNalfan; k++){
     for(j=0; j<fNben; j++){
       fscanf(fp1,"%f",&fTablen[0][k][j]);
       fscanf(fp2,"%f",&fTablen[1][k][j]);
       fscanf(fp3,"%f",&fTablen[2][k][j]);
       fscanf(fp4,"%f",&fTablen[3][k][j]);
     } 
  }
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
  fclose(fp4);
  
  //  --- Reading light tables for ZP and ZEM
  lightfName5 = gSystem->ExpandPathName("$ALICE_ROOT/ZDC/light22620552207s");
  if((fp5 = fopen(lightfName5,"r")) == NULL){
     printf("Cannot open file fp5 \n");
     return;
  }
  lightfName6 = gSystem->ExpandPathName("$ALICE_ROOT/ZDC/light22620552208s");
  if((fp6 = fopen(lightfName6,"r")) == NULL){
     printf("Cannot open file fp6 \n");
     return;
  }
  lightfName7 = gSystem->ExpandPathName("$ALICE_ROOT/ZDC/light22620552209s");
  if((fp7 = fopen(lightfName7,"r")) == NULL){
     printf("Cannot open file fp7 \n");
     return;
  }
  lightfName8 = gSystem->ExpandPathName("$ALICE_ROOT/ZDC/light22620552210s");
  if((fp8 = fopen(lightfName8,"r")) == NULL){
     printf("Cannot open file fp8 \n");
     return;
  }
  
  for(k=0; k<fNalfap; k++){
     for(j=0; j<fNbep; j++){
       fscanf(fp5,"%f",&fTablep[0][k][j]);
       fscanf(fp6,"%f",&fTablep[1][k][j]);
       fscanf(fp7,"%f",&fTablep[2][k][j]);
       fscanf(fp8,"%f",&fTablep[3][k][j]);
     } 
  }
  fclose(fp5);
  fclose(fp6);
  fclose(fp7);
  fclose(fp8);
}
//_____________________________________________________________________________
void AliZDCv3::StepManager()
{
  //
  // Routine called at every step in the Zero Degree Calorimeters
  //
  Int_t   j, vol[2]={0,0}, ibeta=0, ialfa=0, ibe=0, nphe=0;
  Float_t hits[11], x[3], xdet[3], um[3], ud[3];
  Float_t m=0., ekin=0., destep=0., be=0., out=0.;
  // Parametrization for light guide uniformity
  // NEW!!! Light guide tilted @ 51 degrees
  Float_t guiPar[4]={0.31,-0.0006305,0.01337,0.8895};
  Double_t s[3], p[3];
  const char *knamed;
  //
  for(j=0;j<11;j++) hits[j]=-999.;
  //
  // --- This part is for no shower developement in beam pipe and TDI
  // If particle interacts with beam pipe or TDI -> return
  if((gMC->CurrentMedium() == fMedSensPI) || (gMC->CurrentMedium() == fMedSensTDI)){ 
     // If option NoShower is set -> StopTrack
    if(fNoShower==1){
      gMC->TrackPosition(s[0],s[1],s[2]);
      if(gMC->CurrentMedium() == fMedSensPI){
        knamed = gMC->CurrentVolName();
        if(!strncmp(knamed,"YMQ",3)){
	  if(s[2]<0) fpLostITC += 1;
	  else fpLostITA += 1;
        }
	else if(!strncmp(knamed,"YD1",3)){
	  if(s[2]<0) fpLostD1C += 1;
	  else fpLostD1A += 1;
	}
	else if(!strncmp(knamed,"QAL",3)) fnTrou++;
      }
      else if(gMC->CurrentMedium() == fMedSensTDI){ 
        knamed = gMC->CurrentVolName();
        if(!strncmp(knamed,"MD1",3)){
	  if(s[2]<0) fpLostD1C += 1;
	  else  fpLostD1A += 1;
        }
	else if(!strncmp(knamed,"QTD",3)) fpLostTDI += 1;
	else if(!strncmp(knamed,"QLU",3)){
	  if(s[2]<0) fnLumiC ++;
	  else fnLumiA++;
	}
      }
      //
      //gMC->TrackMomentum(p[0], p[1], p[2], p[3]);
      //printf("\t Particle: mass = %1.3f, E = %1.3f GeV, pz = %1.2f GeV -> stopped in volume %s\n", 
      //     gMC->TrackMass(), p[3], p[2], gMC->CurrentVolName());
      //
      printf("\n\t **********************************\n");
      printf("\t ********** Side C **********\n");
      printf("\t # of spectators in IT = %d\n",fpLostITC);
      printf("\t # of spectators in D1 = %d\n",fpLostD1C);
      printf("\t # of spectators in luminometer = %d\n",fnLumiC);
      printf("\t ********** Side A **********\n");
      printf("\t # of spectators in IT = %d\n",fpLostITA);
      printf("\t # of spectators in D1 = %d\n",fpLostD1A);
      printf("\t # of spectators in TDI = %d\n",fpLostTDI);
      printf("\t # of spectators in luminometer = %d\n",fnLumiA);
      printf("\t # of spectators in trousers = %d\n",fnTrou);
      printf("\t **********************************\n");
      gMC->StopTrack();
    }
    return;
  }
  

  if((gMC->CurrentMedium() == fMedSensZN) || (gMC->CurrentMedium() == fMedSensZP) ||
     (gMC->CurrentMedium() == fMedSensGR) || (gMC->CurrentMedium() == fMedSensF1) ||
     (gMC->CurrentMedium() == fMedSensF2) || (gMC->CurrentMedium() == fMedSensZEM)){

    
  //Particle coordinates 
    gMC->TrackPosition(s[0],s[1],s[2]);
    for(j=0; j<=2; j++) x[j] = s[j];
    hits[0] = x[0];
    hits[1] = x[1];
    hits[2] = x[2];

  // Determine in which ZDC the particle is
    knamed = gMC->CurrentVolName();
    if(!strncmp(knamed,"ZN",2)){
          if(x[2]<0.) vol[0]=1; // ZNC (dimuon side)
	  else if(x[2]>0.) vol[0]=4; //ZNA
    }
    else if(!strncmp(knamed,"ZP",2)){ 
          if(x[2]<0.) vol[0]=2; //ZPC (dimuon side)
	  else if(x[2]>0.) vol[0]=5; //ZPA  
    }
    else if(!strncmp(knamed,"ZE",2)) vol[0]=3; //ZEM
  
  // Determine in which quadrant the particle is
    if(vol[0]==1){	//Quadrant in ZNC
      // Calculating particle coordinates inside ZNC
      xdet[0] = x[0]-fPosZNC[0];
      xdet[1] = x[1]-fPosZNC[1];
      // Calculating quadrant in ZN
      if(xdet[0]<=0.){
        if(xdet[1]<=0.) vol[1]=1;
	else vol[1]=3;
      }
      else if(xdet[0]>0.){
        if(xdet[1]<=0.) vol[1]=2;
        else vol[1]=4;
      }
      if((vol[1]!=1) && (vol[1]!=2) && (vol[1]!=3) && (vol[1]!=4))
        printf("\n	ZDC StepManager->ERROR in ZN!!! vol[1] = %d, xdet[0] = %f,"
	"xdet[1] = %f\n",vol[1], xdet[0], xdet[1]);
    }
    
    else if(vol[0]==2){	//Quadrant in ZPC
      // Calculating particle coordinates inside ZPC
      xdet[0] = x[0]-fPosZPA[0];
      xdet[1] = x[1]-fPosZPA[1];
      if(xdet[0]>=fDimZP[0])  xdet[0]=fDimZP[0]-0.01;
      if(xdet[0]<=-fDimZP[0]) xdet[0]=-fDimZP[0]+0.01;
      // Calculating tower in ZP
      Float_t xqZP = xdet[0]/(fDimZP[0]/2.);
      for(int i=1; i<=4; i++){
         if(xqZP>=(i-3) && xqZP<(i-2)){
 	   vol[1] = i;
 	   break;
 	 }
      }
      if((vol[1]!=1) && (vol[1]!=2) && (vol[1]!=3) && (vol[1]!=4))
        printf("	ZDC StepManager->ERROR in ZP!!! vol[1] = %d, xdet[0] = %f,"
	"xdet[1] = %f\n",vol[1], xdet[0], xdet[1]);
    }
    //
    // Quadrant in ZEM: vol[1] = 1 -> particle in 1st ZEM (placed at x = 8.5 cm)
    // 		 	vol[1] = 2 -> particle in 2nd ZEM (placed at x = -8.5 cm)
    else if(vol[0] == 3){	
      if(x[0]>0.){
        vol[1] = 1;
        // Particle x-coordinate inside ZEM1
        xdet[0] = x[0]-fPosZEM[0];
      }
      else{
   	vol[1] = 2;
        // Particle x-coordinate inside ZEM2
        xdet[0] = x[0]+fPosZEM[0];
      }
      xdet[1] = x[1]-fPosZEM[1];
    }
    //
    else if(vol[0]==4){	//Quadrant in ZNA
      // Calculating particle coordinates inside ZNA
      xdet[0] = x[0]-fPosZNA[0];
      xdet[1] = x[1]-fPosZNA[1];
      // Calculating quadrant in ZNA
      if(xdet[0]>=0.){
        if(xdet[1]<=0.) vol[1]=1;
	else vol[1]=3;
      }
      else if(xdet[0]<0.){
        if(xdet[1]<=0.) vol[1]=2;
        else vol[1]=4;
      }
      if((vol[1]!=1) && (vol[1]!=2) && (vol[1]!=3) && (vol[1]!=4))
        printf("\n	ZDC StepManager->ERROR in ZNA!!! vol[1] = %d, xdet[0] = %f,"
	"xdet[1] = %f\n",vol[1], xdet[0], xdet[1]);
    }    
    //
    else if(vol[0]==5){	//Quadrant in ZPA
      // Calculating particle coordinates inside ZPA
      xdet[0] = x[0]-fPosZPC[0];
      xdet[1] = x[1]-fPosZPC[1];
      if(xdet[0]>=fDimZP[0])  xdet[0]=fDimZP[0]-0.01;
      if(xdet[0]<=-fDimZP[0]) xdet[0]=-fDimZP[0]+0.01;
      // Calculating tower in ZP
      Float_t xqZP = -xdet[0]/(fDimZP[0]/2.);
      for(int i=1; i<=4; i++){
         if(xqZP>=(i-3) && xqZP<(i-2)){
 	   vol[1] = i;
 	   break;
 	 }
      }
      if((vol[1]!=1) && (vol[1]!=2) && (vol[1]!=3) && (vol[1]!=4))
        printf("	ZDC StepManager->ERROR in ZPA!!! vol[1] = %d, xdet[0] = %f,"
	"xdet[1] = %f\n",vol[1], xdet[0], xdet[1]);
    }    
    
    
  // Store impact point and kinetic energy of the ENTERING particle
    
      if(gMC->IsTrackEntering()){
        //Particle energy
        gMC->TrackMomentum(p[0],p[1],p[2],p[3]);
        hits[3] = p[3];
        // Impact point on ZDC  
        hits[4] = xdet[0];
        hits[5] = xdet[1];
	hits[6] = 0;
        hits[7] = 0;
        hits[8] = 0;
        hits[9] = 0;
	//
	Int_t curTrackN = gAlice->GetMCApp()->GetCurrentTrackNumber();
        TParticle *part = (gAlice->GetMCApp())->Particle(curTrackN);
	hits[10] = part->GetPdgCode();
	//printf("\t PDGCode = %d\n", part->GetPdgCode());

	AddHit(curTrackN, vol, hits);
	
	if(fNoShower==1){
	  //printf("\t VolName %s -> det %d quad %d - x = %f, y = %f, z = %f\n", 
	    //knamed, vol[0], vol[1], x[0], x[1], x[2]);
	  if(vol[0]==1){
	    fnDetectedC += 1;
	    printf("\n	  # of particles in ZNC = %d\n\n",fnDetectedC);
	  }
	  else if(vol[0]==2){
	    fpDetectedC += 1;
	    printf("\n	  # of particles in ZPC = %d\n\n",fpDetectedC);
	  }
	  else if(vol[0]==4){
	    fnDetectedA += 1;
	    printf("\n	  # of particles in ZNA = %d\n\n",fnDetectedA);     
	  }
	  else if(vol[0]==5){
	    fpDetectedA += 1;
	    printf("\n	  # of particles in ZPA = %d\n\n",fpDetectedA);      
	  }
          //
	  //printf("\t Particle: mass = %1.3f, E = %1.3f GeV, pz = %1.2f GeV -> stopped in volume %s\n", 
          //   gMC->TrackMass(), p[3], p[2], gMC->CurrentVolName());
	  //
	  gMC->StopTrack();
	  return;
	}
      }
             
      // Charged particles -> Energy loss
      if((destep=gMC->Edep())){
         if(gMC->IsTrackStop()){
           gMC->TrackMomentum(p[0],p[1],p[2],p[3]);
	   m = gMC->TrackMass();
	   ekin = p[3]-m;
	   hits[9] = ekin;
	   hits[7] = 0.;
	   hits[8] = 0.;
	   AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
	   }
	 else{
	   hits[9] = destep;
	   hits[7] = 0.;
	   hits[8] = 0.;
	   AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
	   }
      }
  }
 

  // *** Light production in fibres 
  if((gMC->CurrentMedium() == fMedSensF1) || (gMC->CurrentMedium() == fMedSensF2)){

     //Select charged particles
     if((destep=gMC->Edep())){

       // Particle velocity
       Float_t beta = 0.;
       gMC->TrackMomentum(p[0],p[1],p[2],p[3]);
       Float_t ptot=TMath::Sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2]);
       if(p[3] > 0.00001) beta =  ptot/p[3];
       else return;
       if(beta<0.67)return;
       else if((beta>=0.67) && (beta<=0.75)) ibeta = 0;
       else if((beta>0.75)  && (beta<=0.85)) ibeta = 1;
       else if((beta>0.85)  && (beta<=0.95)) ibeta = 2;
       else if(beta>0.95) ibeta = 3;
 
       // Angle between particle trajectory and fibre axis
       // 1 -> Momentum directions
       um[0] = p[0]/ptot;
       um[1] = p[1]/ptot;
       um[2] = p[2]/ptot;
       gMC->Gmtod(um,ud,2);
       // 2 -> Angle < limit angle
       Double_t alfar = TMath::ACos(ud[2]);
       Double_t alfa = alfar*kRaddeg;
       if(alfa>=110.) return;
       //
       ialfa = Int_t(1.+alfa/2.);
 
       // Distance between particle trajectory and fibre axis
       gMC->TrackPosition(s[0],s[1],s[2]);
       for(j=0; j<=2; j++){
   	  x[j] = s[j];
       }
       gMC->Gmtod(x,xdet,1);
       if(TMath::Abs(ud[0])>0.00001){
         Float_t dcoeff = ud[1]/ud[0];
         be = TMath::Abs((xdet[1]-dcoeff*xdet[0])/TMath::Sqrt(dcoeff*dcoeff+1.));
       }
       else{
         be = TMath::Abs(ud[0]);
       }
 
       ibe = Int_t(be*1000.+1);
       //if((vol[0]==1))      radius = fFibZN[1];
       //else if((vol[0]==2)) radius = fFibZP[1];
 
       //Looking into the light tables 
       Float_t charge = gMC->TrackCharge();
       
       if(vol[0]==1 || vol[0]==4) {	// (1)  ZN fibres
         if(ibe>fNben) ibe=fNben;
         out =  charge*charge*fTablen[ibeta][ialfa][ibe];
	 nphe = gRandom->Poisson(out);
	 // Ch. debug
         //if(ibeta==3) printf("\t %f \t %f \t %f\n",alfa, be, out);
	 //printf("\t ibeta = %d, ialfa = %d, ibe = %d -> nphe = %d\n\n",ibeta,ialfa,ibe,nphe);
	 if(gMC->CurrentMedium() == fMedSensF1){
	   hits[7] = nphe;  	//fLightPMQ
	   hits[8] = 0;
	   hits[9] = 0;
	   AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
	 }
	 else{
	   hits[7] = 0;
	   hits[8] = nphe;	//fLightPMC
	   hits[9] = 0;
	   AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
	 }
       } 
       else if(vol[0]==2 || vol[0]==5) {// (2) ZP fibres
         if(ibe>fNbep) ibe=fNbep;
         out =  charge*charge*fTablep[ibeta][ialfa][ibe];
	 nphe = gRandom->Poisson(out);
	 if(gMC->CurrentMedium() == fMedSensF1){
	   hits[7] = nphe;  	//fLightPMQ
	   hits[8] = 0;
	   hits[9] = 0;
	   AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
	 }
	 else{
	   hits[7] = 0;
	   hits[8] = nphe;	//fLightPMC
	   hits[9] = 0;
	   AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
	 }
       } 
       else if((vol[0]==3)) {	// (3) ZEM fibres
         if(ibe>fNbep) ibe=fNbep;
         out =  charge*charge*fTablep[ibeta][ialfa][ibe];
	 gMC->TrackPosition(s[0],s[1],s[2]);
	 Float_t xalic[3];
         for(j=0; j<3; j++){
            xalic[j] = s[j];
         }
	 // z-coordinate from ZEM front face 
	 // NB-> fPosZEM[2]+fZEMLength = -1000.+2*10.3 = 979.69 cm
	 Float_t z = -xalic[2]+fPosZEM[2]+2*fZEMLength-xalic[1];
//	 z = xalic[2]-fPosZEM[2]-fZEMLength-xalic[1]*(TMath::Tan(45.*kDegrad));
//         printf("\n	fPosZEM[2]+2*fZEMLength = %f", fPosZEM[2]+2*fZEMLength);
	 Float_t guiEff = guiPar[0]*(guiPar[1]*z*z+guiPar[2]*z+guiPar[3]);
	 out = out*guiEff;
	 nphe = gRandom->Poisson(out);
//         printf("	out*guiEff = %f	nphe = %d", out, nphe);
	 if(vol[1] == 1){
	   hits[7] = 0;  	
	   hits[8] = nphe;	//fLightPMC (ZEM1)
	   hits[9] = 0;
	   AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
	 }
	 else{
	   hits[7] = nphe;  	//fLightPMQ (ZEM2)
	   hits[8] = 0;		
	   hits[9] = 0;
	   AddHit(gAlice->GetMCApp()->GetCurrentTrackNumber(), vol, hits);
	 }
       }
     }
   }
}
