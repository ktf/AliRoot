// $Id$
// **************************************************************************
// This file is property of and copyright by the ALICE HLT Project          * 
// ALICE Experiment at CERN, All rights reserved.                           *
//                                                                          *
// Primary Authors: Sergey Gorbunov <sergey.gorbunov@kip.uni-heidelberg.de> *
//                  Ivan Kisel <kisel@kip.uni-heidelberg.de>                *
//                  for The ALICE HLT Project.                              *
//                                                                          *
// Permission to use, copy, modify and distribute this software and its     *
// documentation strictly for non-commercial purposes is hereby granted     *
// without fee, provided that the above copyright notice appears in all     *
// copies and that both the copyright notice and this permission notice     *
// appear in the supporting documentation. The authors make no claims       *
// about the suitability of this software for any purpose. It is            *
// provided "as is" without express or implied warranty.                    *
//                                                                          *
//***************************************************************************


#include "AliHLTTPCCATrackParam.h"
#include "AliHLTTPCCAMath.h"
#include "AliHLTTPCCATrackLinearisation.h"
#include <iostream>

//
// Circle in XY:
//
// kCLight = 0.000299792458; 
// Kappa = Bz*kCLight*QPt;
// R  = 1/TMath::Abs(Kappa);
// Xc = X - sin(Phi)/Kappa;
// Yc = Y + cos(Phi)/Kappa;
//

GPUd() Float_t AliHLTTPCCATrackParam::GetDist2( const AliHLTTPCCATrackParam &t ) const
{
  // get squared distance between tracks 

  Float_t dx = GetX() - t.GetX();
  Float_t dy = GetY() - t.GetY();
  Float_t dz = GetZ() - t.GetZ();
  return dx*dx + dy*dy + dz*dz;
}

GPUd() Float_t AliHLTTPCCATrackParam::GetDistXZ2( const AliHLTTPCCATrackParam &t ) const
{
  // get squared distance between tracks in X&Z 

  Float_t dx = GetX() - t.GetX();
  Float_t dz = GetZ() - t.GetZ();
  return dx*dx + dz*dz;
}


GPUd() Float_t  AliHLTTPCCATrackParam::GetS( Float_t x, Float_t y, Float_t Bz ) const
{
  //* Get XY path length to the given point

  Float_t k  = GetKappa( Bz );
  Float_t ex = GetCosPhi();
  Float_t ey = GetSinPhi();
  x-= GetX();
  y-= GetY();
  Float_t dS = x*ex + y*ey;
  if( CAMath::Abs(k)>1.e-4 ) dS = CAMath::ATan2( k*dS, 1+k*(x*ey-y*ex) )/k;
  return dS;
}

GPUd() void  AliHLTTPCCATrackParam::GetDCAPoint( Float_t x, Float_t y, Float_t z,
						 Float_t &xp, Float_t &yp, Float_t &zp, 
						 Float_t Bz ) const
{
  //* Get the track point closest to the (x,y,z)

  Float_t x0 = GetX();
  Float_t y0 = GetY();
  Float_t k  = GetKappa( Bz );
  Float_t ex = GetCosPhi();
  Float_t ey = GetSinPhi();
  Float_t dx = x - x0;
  Float_t dy = y - y0; 
  Float_t ax = dx*k+ey;
  Float_t ay = dy*k-ex;
  Float_t a = sqrt( ax*ax+ay*ay );
  xp = x0 + (dx - ey*( (dx*dx+dy*dy)*k - 2*(-dx*ey+dy*ex) )/(a+1) )/a;
  yp = y0 + (dy + ex*( (dx*dx+dy*dy)*k - 2*(-dx*ey+dy*ex) )/(a+1) )/a;
  Float_t s = GetS(x,y, Bz);
  zp = GetZ() + GetDzDs()*s;
  if( CAMath::Abs(k)>1.e-2 ){
    Float_t dZ = CAMath::Abs( GetDzDs()*CAMath::TwoPi()/k );
    if( dZ>.1 ){
      zp+= CAMath::Nint((z-zp)/dZ)*dZ;    
    }
  }
}


//*
//* Transport routines
//*


GPUd() Bool_t  AliHLTTPCCATrackParam::TransportToX( Float_t x, AliHLTTPCCATrackLinearisation &t0, Float_t Bz,  Float_t maxSinPhi, Float_t *DL )
{
  //* Transport the track parameters to X=x, using linearization at t0, and the field value Bz
  //* maxSinPhi is the max. allowed value for |t0.SinPhi()|
  //* linearisation of trajectory t0 is also transported to X=x, 
  //* returns 1 if OK
  //*

  Float_t ex = t0.CosPhi();
  Float_t ey = t0.SinPhi();
  Float_t k   = t0.QPt()*Bz;
  Float_t dx = x - X();

  Float_t ey1 = k*dx + ey;
  Float_t ex1;
  
  // check for intersection with X=x 

  if( CAMath::Abs(ey1)>maxSinPhi ) return 0;

  ex1 = CAMath::Sqrt(1 - ey1*ey1);
  if( ex<0 ) ex1 = -ex1;
  
  Float_t dx2 = dx*dx;
  Float_t ss = ey+ey1;
  Float_t cc = ex+ex1;  

  if( CAMath::Abs(cc)<1.e-4 || CAMath::Abs(ex)<1.e-4 || CAMath::Abs(ex1)<1.e-4 ) return 0;

  Float_t tg = ss/cc; // tan((phi1+phi)/2)
  
  Float_t dy = dx*tg;
  Float_t dl = dx*CAMath::Sqrt(1+tg*tg);

  if( cc<0 ) dl = -dl;
  Float_t dSin = dl*k/2;
  if( dSin > 1 ) dSin = 1;
  if( dSin <-1 ) dSin = -1;
  Float_t dS = ( CAMath::Abs(k)>1.e-4)  ? (2*CAMath::ASin(dSin)/k) :dl;  
  Float_t dz = dS*t0.DzDs();

  if( DL ) *DL = -dS*CAMath::Sqrt(1 + t0.DzDs()*t0.DzDs() );

  Float_t cci = 1./cc;
  Float_t exi = 1./ex;
  Float_t ex1i = 1./ex1;
  
  Float_t d[5] = { 0,
		   0,
		   fP[2]-t0.SinPhi(),
		   fP[3]-t0.DzDs(), 
		   fP[4]-t0.QPt() };

  //Float_t H0[5] = { 1,0, h2,  0, h4 };
  //Float_t H1[5] = { 0, 1, 0, dS,  0 };
  //Float_t H2[5] = { 0, 0, 1,  0, dxBz };
  //Float_t H3[5] = { 0, 0, 0,  1,  0 };
  //Float_t H4[5] = { 0, 0, 0,  0,  1 };
  
  Float_t h2 = dx*(1+ey*ey1 + ex*ex1)*exi*ex1i*cci;
  Float_t h4 = dx2*(cc + ss*ey1*ex1i )*cci*cci*Bz;
  Float_t dxBz = dx*Bz;

  t0.SetCosPhi( ex1 );
  t0.SetSinPhi( ey1 );  

  fX    = X() + dx;
  fP[0] = Y() + dy     + h2*d[2]           +   h4*d[4];
  fP[1] = Z() + dz               + dS*d[3];
  fP[2] = t0.SinPhi() +     d[2]           + dxBz*d[4];

  Float_t c00 = fC[0];
  Float_t c10 = fC[1];
  Float_t c11 = fC[2];
  Float_t c20 = fC[3];
  Float_t c21 = fC[4];
  Float_t c22 = fC[5];
  Float_t c30 = fC[6];
  Float_t c31 = fC[7];
  Float_t c32 = fC[8];
  Float_t c33 = fC[9];
  Float_t c40 = fC[10];
  Float_t c41 = fC[11];
  Float_t c42 = fC[12];
  Float_t c43 = fC[13];
  Float_t c44 = fC[14];

  fC[0]=( c00  + h2*h2*c22 + h4*h4*c44 
	  + 2*( h2*c20 + h4*c40 + h2*h4*c42 )  ); 

  fC[1]= c10 + h2*c21 + h4*c41 + dS*(c30 + h2*c32 + h4*c43);
  fC[2]= c11 + 2*dS*c31 + dS*dS*c33;

  fC[3]= c20 + h2*c22 + h4*c42 + dxBz*( c40 + h2*c42 + h4*c44);
  fC[4]= c21 + dS*c32 + dxBz*(c41 + dS*c43);
  fC[5]= c22 +2*dxBz*c42 + dxBz*dxBz*c44;

  fC[6]= c30 + h2*c32 + h4*c43;
  fC[7]= c31 + dS*c33;
  fC[8]= c32 + dxBz*c43;
  fC[9]= c33;

  fC[10]= c40 + h2*c42 + h4*c44;
  fC[11]= c41 + dS*c43;
  fC[12]= c42 + dxBz*c44;
  fC[13]= c43;
  fC[14]= c44;

  return 1;
}


GPUd() Bool_t  AliHLTTPCCATrackParam::TransportToX( Float_t x, Float_t sinPhi0, Float_t cosPhi0,  Float_t Bz, Float_t maxSinPhi )
{
  //* Transport the track parameters to X=x, using linearization at phi0 with 0 curvature, 
  //* and the field value Bz
  //* maxSinPhi is the max. allowed value for |t0.SinPhi()|
  //* linearisation of trajectory t0 is also transported to X=x, 
  //* returns 1 if OK
  //*

  Float_t ex = cosPhi0;
  Float_t ey = sinPhi0;
  Float_t dx = x - X();

  if( CAMath::Abs(ex)<1.e-4 ) return 0;
  Float_t exi = 1./ex;

  Float_t dxBz = dx*Bz;
  Float_t dS = dx*exi;
  Float_t h2 = dS*exi*exi;
  Float_t h4 = .5*h2*dxBz;
 
  //Float_t H0[5] = { 1,0, h2,  0, h4 };
  //Float_t H1[5] = { 0, 1, 0, dS,  0 };
  //Float_t H2[5] = { 0, 0, 1,  0, dxBz };
  //Float_t H3[5] = { 0, 0, 0,  1,  0 };
  //Float_t H4[5] = { 0, 0, 0,  0,  1 };

  Float_t sinPhi = SinPhi() + dxBz*QPt();
  if( maxSinPhi>0 && CAMath::Abs(sinPhi)>maxSinPhi ) return 0;

  fX    = X() + dx;
  fP[0]+= dS*ey + h2*( SinPhi() - ey )  +   h4*QPt();
  fP[1]+= dS*DzDs();
  fP[2] = sinPhi;

 
  Float_t c00 = fC[0];
  Float_t c10 = fC[1];
  Float_t c11 = fC[2];
  Float_t c20 = fC[3];
  Float_t c21 = fC[4];
  Float_t c22 = fC[5];
  Float_t c30 = fC[6];
  Float_t c31 = fC[7];
  Float_t c32 = fC[8];
  Float_t c33 = fC[9];
  Float_t c40 = fC[10];
  Float_t c41 = fC[11];
  Float_t c42 = fC[12];
  Float_t c43 = fC[13];
  Float_t c44 = fC[14];


  fC[0]=( c00  + h2*h2*c22 + h4*h4*c44 
	  + 2*( h2*c20 + h4*c40 + h2*h4*c42 )  ); 

  fC[1]= c10 + h2*c21 + h4*c41 + dS*(c30 + h2*c32 + h4*c43);
  fC[2]= c11 + 2*dS*c31 + dS*dS*c33;

  fC[3]= c20 + h2*c22 + h4*c42 + dxBz*( c40 + h2*c42 + h4*c44);
  fC[4]= c21 + dS*c32 + dxBz*(c41 + dS*c43);
  fC[5]= c22 +2*dxBz*c42 + dxBz*dxBz*c44;

  fC[6]= c30 + h2*c32 + h4*c43;
  fC[7]= c31 + dS*c33;
  fC[8]= c32 + dxBz*c43;
  fC[9]= c33;

  fC[10]= c40 + h2*c42 + h4*c44;
  fC[11]= c41 + dS*c43;
  fC[12]= c42 + dxBz*c44;
  fC[13]= c43;
  fC[14]= c44;

  return 1;
}






GPUd() Bool_t  AliHLTTPCCATrackParam::TransportToX( Float_t x, Float_t Bz, Float_t maxSinPhi )
{
  //* Transport the track parameters to X=x 

  AliHLTTPCCATrackLinearisation t0(*this);

  return TransportToX( x, t0, Bz, maxSinPhi );
}



GPUd() Bool_t  AliHLTTPCCATrackParam::TransportToXWithMaterial( Float_t x,  AliHLTTPCCATrackLinearisation &t0, AliHLTTPCCATrackFitParam &par, Float_t Bz, Float_t maxSinPhi )
{
  //* Transport the track parameters to X=x  taking into account material budget

  const Float_t kRho = 1.025e-3;//0.9e-3; 
  const Float_t kRadLen=29.532;//28.94;
  const Float_t kRhoOverRadLen = kRho/kRadLen;
  Float_t dl;

  if( !TransportToX( x, t0, Bz,  maxSinPhi, &dl ) ) return 0;

  CorrectForMeanMaterial(dl*kRhoOverRadLen,dl*kRho,par);  
  return 1;
}


GPUd() Bool_t  AliHLTTPCCATrackParam::TransportToXWithMaterial( Float_t x,  AliHLTTPCCATrackFitParam &par, Float_t Bz, Float_t maxSinPhi )
{
  //* Transport the track parameters to X=x  taking into account material budget

  AliHLTTPCCATrackLinearisation t0(*this);
  return TransportToXWithMaterial( x, t0, par, Bz, maxSinPhi );
}

GPUd() Bool_t AliHLTTPCCATrackParam::TransportToXWithMaterial( Float_t x, Float_t Bz, Float_t maxSinPhi )
{
  //* Transport the track parameters to X=x taking into account material budget

  AliHLTTPCCATrackFitParam par;
  CalculateFitParameters( par );
  return TransportToXWithMaterial(x, par, Bz, maxSinPhi );
}


//*
//*  Multiple scattering and energy losses
//*


Float_t AliHLTTPCCATrackParam::BetheBlochGeant(Float_t bg2,
					       Float_t kp0,
					       Float_t kp1,
					       Float_t kp2,
					       Float_t kp3,
					       Float_t kp4) 
{
  //
  // This is the parameterization of the Bethe-Bloch formula inspired by Geant.
  //
  // bg2  - (beta*gamma)^2
  // kp0 - density [g/cm^3]
  // kp1 - density effect first junction point
  // kp2 - density effect second junction point
  // kp3 - mean excitation energy [GeV]
  // kp4 - mean Z/A
  //
  // The default values for the kp* parameters are for silicon. 
  // The returned value is in [GeV/(g/cm^2)].
  // 

  const Float_t mK  = 0.307075e-3; // [GeV*cm^2/g]
  const Float_t me  = 0.511e-3;    // [GeV/c^2]
  const Float_t rho = kp0;
  const Float_t x0  = kp1*2.303;
  const Float_t x1  = kp2*2.303;
  const Float_t mI  = kp3;
  const Float_t mZA = kp4;
  const Float_t maxT= 2*me*bg2;    // neglecting the electron mass
  
  //*** Density effect
  Float_t d2=0.; 
  const Float_t x=0.5*TMath::Log(bg2);
  const Float_t lhwI=TMath::Log(28.816*1e-9*TMath::Sqrt(rho*mZA)/mI);
  if (x > x1) {
    d2 = lhwI + x - 0.5;
  } else if (x > x0) {
    const Float_t r=(x1-x)/(x1-x0);
    d2 = lhwI + x - 0.5 + (0.5 - lhwI - x0)*r*r*r;
  }

  return mK*mZA*(1+bg2)/bg2*(0.5*TMath::Log(2*me*bg2*maxT/(mI*mI)) - bg2/(1+bg2) - d2);
}

Float_t AliHLTTPCCATrackParam::BetheBlochSolid(Float_t bg) 
{
  //------------------------------------------------------------------
  // This is an approximation of the Bethe-Bloch formula, 
  // reasonable for solid materials. 
  // All the parameters are, in fact, for Si.
  // The returned value is in [GeV]
  //------------------------------------------------------------------

  return BetheBlochGeant(bg);
}

Float_t AliHLTTPCCATrackParam::BetheBlochGas(Float_t bg) 
{
  //------------------------------------------------------------------
  // This is an approximation of the Bethe-Bloch formula, 
  // reasonable for gas materials.
  // All the parameters are, in fact, for Ne.
  // The returned value is in [GeV]
  //------------------------------------------------------------------

  const Float_t rho = 0.9e-3;
  const Float_t x0  = 2.;
  const Float_t x1  = 4.;
  const Float_t mI  = 140.e-9;
  const Float_t mZA = 0.49555;

  return BetheBlochGeant(bg,rho,x0,x1,mI,mZA);
}




GPUd() Float_t AliHLTTPCCATrackParam::ApproximateBetheBloch( Float_t beta2 ) 
{
  //------------------------------------------------------------------
  // This is an approximation of the Bethe-Bloch formula with 
  // the density effect taken into account at beta*gamma > 3.5
  // (the approximation is reasonable only for solid materials) 
  //------------------------------------------------------------------
  if (beta2 >= 1) return 0;

  if (beta2/(1-beta2)>3.5*3.5)
    return 0.153e-3/beta2*( log(3.5*5940)+0.5*log(beta2/(1-beta2)) - beta2);
  return 0.153e-3/beta2*(log(5940*beta2/(1-beta2)) - beta2);
}


GPUd() void AliHLTTPCCATrackParam::CalculateFitParameters( AliHLTTPCCATrackFitParam &par, Float_t mass )
{
  //*!

  Float_t p2 = (1.+ fP[3]*fP[3]);
  Float_t k2 = fP[4]*fP[4];
  Float_t mass2 = mass*mass;
  Float_t beta2= p2 / (p2 + mass2*k2);

  Float_t pp2 = (k2>1.e-8) ?p2/k2 :10000; // impuls 2  

  //par.fBethe = BetheBlochGas( pp2/mass2);
  par.fBethe = ApproximateBetheBloch( pp2/mass2);
  par.fE = CAMath::Sqrt( pp2 + mass2);
  par.fTheta2 = 14.1*14.1/(beta2*pp2*1e6);
  par.fEP2 = par.fE/pp2;

  // Approximate energy loss fluctuation (M.Ivanov)
  
  const Float_t knst=0.07; // To be tuned.  
  par.fSigmadE2 = knst*par.fEP2*fP[4];
  par.fSigmadE2 = par.fSigmadE2 * par.fSigmadE2;
 
  par.fK22 = (1. + fP[3]*fP[3]);
  par.fK33 = par.fK22*par.fK22;
  par.fK43 = fP[3]*fP[4]*par.fK22;
  par.fK44 = fP[3]*fP[3]*fP[4]*fP[4];

}


GPUd() Bool_t AliHLTTPCCATrackParam::CorrectForMeanMaterial( Float_t xOverX0,  Float_t xTimesRho, const AliHLTTPCCATrackFitParam &par )
{
  //------------------------------------------------------------------
  // This function corrects the track parameters for the crossed material.
  // "xOverX0"   - X/X0, the thickness in units of the radiation length.
  // "xTimesRho" - is the product length*density (g/cm^2). 
  //------------------------------------------------------------------

  Float_t &fC22=fC[5];
  Float_t &fC33=fC[9];
  Float_t &fC40=fC[10];
  Float_t &fC41=fC[11];
  Float_t &fC42=fC[12];
  Float_t &fC43=fC[13];
  Float_t &fC44=fC[14]; 

  //Energy losses************************
 
  Float_t dE = par.fBethe*xTimesRho;
  if ( CAMath::Abs(dE) > 0.3*par.fE ) return 0; //30% energy loss is too much!
  Float_t corr = (1.- par.fEP2*dE);
  if( corr<0.3 || corr>1.3 ) return 0;

  fP[4]*= corr;
  fC40*= corr;
  fC41*= corr;
  fC42*= corr;
  fC43*= corr;
  fC44*= corr*corr;
  fC44+= par.fSigmadE2*CAMath::Abs(dE);

  //Multiple scattering******************
  
  Float_t theta2 = par.fTheta2*CAMath::Abs(xOverX0);
  fC22 += theta2*par.fK22*(1.- fP[2]*fP[2]);
  fC33 += theta2*par.fK33;
  fC43 += theta2*par.fK43; 
  fC44 += theta2*par.fK44;

  return 1;
}


//*
//* Rotation
//*


GPUd() Bool_t AliHLTTPCCATrackParam::Rotate( Float_t alpha, Float_t maxSinPhi )
{
  //* Rotate the coordinate system in XY on the angle alpha
  
  Float_t cA = CAMath::Cos( alpha );
  Float_t sA = CAMath::Sin( alpha );
  Float_t x = X(), y= Y(), sP= SinPhi(), cP= GetCosPhi();
  Float_t cosPhi = cP*cA + sP*sA;
  Float_t sinPhi =-cP*sA + sP*cA;
  
  if( CAMath::Abs(sinPhi)> maxSinPhi || CAMath::Abs(cosPhi)<1.e-2 || CAMath::Abs(cP)<1.e-2  ) return 0;
  
  Float_t j0 = cP/cosPhi; 
  Float_t j2 = cosPhi/cP;
  
  SetX( x*cA +  y*sA );
  SetY(-x*sA +  y*cA );
  SetSignCosPhi( cosPhi );
  SetSinPhi( sinPhi );


  //Float_t J[5][5] = { { j0, 0, 0,  0,  0 }, // Y
  //                      {  0, 1, 0,  0,  0 }, // Z
  //                      {  0, 0, j2, 0,  0 }, // SinPhi
  //	                  {  0, 0, 0,  1,  0 }, // DzDs
  //	                  {  0, 0, 0,  0,  1 } }; // Kappa
  //cout<<"alpha="<<alpha<<" "<<x<<" "<<y<<" "<<sP<<" "<<cP<<" "<<j0<<" "<<j2<<endl;
  //cout<<"      "<<fC[0]<<" "<<fC[1]<<" "<<fC[6]<<" "<<fC[10]<<" "<<fC[4]<<" "<<fC[5]<<" "<<fC[8]<<" "<<fC[12]<<endl;
  fC[0]*= j0*j0;
  fC[1]*= j0;
  fC[3]*= j0;
  fC[6]*= j0;
  fC[10]*= j0;

  fC[3]*= j2;
  fC[4]*= j2;
  fC[5]*= j2*j2; 
  fC[8]*= j2;
  fC[12]*= j2;
  //cout<<"      "<<fC[0]<<" "<<fC[1]<<" "<<fC[6]<<" "<<fC[10]<<" "<<fC[4]<<" "<<fC[5]<<" "<<fC[8]<<" "<<fC[12]<<endl;
  return 1;
}

GPUd() Bool_t AliHLTTPCCATrackParam::Rotate( Float_t alpha, AliHLTTPCCATrackLinearisation &t0, Float_t maxSinPhi )
{
  //* Rotate the coordinate system in XY on the angle alpha
  
  Float_t cA = CAMath::Cos( alpha );
  Float_t sA = CAMath::Sin( alpha );
  Float_t x0 = X(), y0= Y(), sP= t0.SinPhi(), cP= t0.CosPhi();
  Float_t cosPhi = cP*cA + sP*sA;
  Float_t sinPhi =-cP*sA + sP*cA;
  
  if( CAMath::Abs(sinPhi)> maxSinPhi || CAMath::Abs(cosPhi)<1.e-2 || CAMath::Abs(cP)<1.e-2  ) return 0;
  
  //Float_t J[5][5] = { { j0, 0, 0,  0,  0 }, // Y
  //                    {  0, 1, 0,  0,  0 }, // Z
  //                    {  0, 0, j2, 0,  0 }, // SinPhi
  //	                {  0, 0, 0,  1,  0 }, // DzDs
  //	                {  0, 0, 0,  0,  1 } }; // Kappa

  Float_t j0 = cP/cosPhi; 
  Float_t j2 = cosPhi/cP;
  Float_t d[2] = {Y() - y0, SinPhi() - sP};

  SetX( x0*cA +  y0*sA );
  SetY(-x0*sA +  y0*cA + j0*d[0] );
  t0.SetCosPhi( cosPhi );
  t0.SetSinPhi( sinPhi );

  SetSinPhi( sinPhi + j2*d[1] );

  fC[0]*= j0*j0;
  fC[1]*= j0;
  fC[3]*= j0;
  fC[6]*= j0;
  fC[10]*= j0;

  fC[3]*= j2;
  fC[4]*= j2;
  fC[5]*= j2*j2; 
  fC[8]*= j2;
  fC[12]*= j2;

  return 1;
}


GPUd() Bool_t AliHLTTPCCATrackParam::Filter( Float_t y, Float_t z, Float_t err2Y, Float_t err2Z, Float_t maxSinPhi )
{
  //* Add the y,z measurement with the Kalman filter 

  Float_t 
    c00 = fC[ 0],
    c11 = fC[ 2],
    c20 = fC[ 3],
    c31 = fC[ 7],
    c40 = fC[10];

  err2Y+=c00;
  err2Z+=c11;

  Float_t
    z0 = y-fP[0],
    z1 = z-fP[1];
  
  if( err2Y < 1.e-8 || err2Z<1.e-8 ) return 0;

  Float_t mS0 = 1./err2Y;
  Float_t mS2 = 1./err2Z;
 
  // K = CHtS
  
  Float_t k00, k11, k20, k31, k40;
    
  k00 = c00*mS0;
  k20 = c20*mS0;
  k40 = c40*mS0;

  k11 = c11*mS2;
  k31 = c31*mS2;

  Float_t sinPhi = fP[2] + k20*z0  ;
  
  if( maxSinPhi>0 && CAMath::Abs(sinPhi)>= maxSinPhi ) return 0;
  
  fNDF  += 2;
  fChi2 += mS0*z0*z0 + mS2*z1*z1 ;

  fP[ 0]+= k00*z0 ;
  fP[ 1]+= k11*z1 ;
  fP[ 2] = sinPhi ;
  fP[ 3]+= k31*z1 ;
  fP[ 4]+= k40*z0 ;
    
  fC[ 0]-= k00*c00 ;    
  fC[ 3]-= k20*c00 ;
  fC[ 5]-= k20*c20 ;
  fC[10]-= k40*c00 ;
  fC[12]-= k40*c20 ;
  fC[14]-= k40*c40 ;
 
  fC[ 2]-= k11*c11 ;
  fC[ 7]-= k31*c11 ;
  fC[ 9]-= k31*c31 ;
   
  return 1;
}




#if !defined(HLTCA_GPUCODE)
#include <iostream>
#endif

GPUd() void AliHLTTPCCATrackParam::Print() const
{
  //* print parameters
 
#if !defined(HLTCA_GPUCODE)
  std::cout<<"track: x="<<GetX()<<" c="<<GetSignCosPhi()<<", P= "<<GetY()<<" "<<GetZ()<<" "<<GetSinPhi()<<" "<<GetDzDs()<<" "<<GetQPt()<<std::endl;
  std::cout<<"errs2: "<<GetErr2Y()<<" "<<GetErr2Z()<<" "<<GetErr2SinPhi()<<" "<<GetErr2DzDs()<<" "<<GetErr2QPt()<<std::endl;
#endif
}

