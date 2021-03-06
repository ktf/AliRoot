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

//-------------------------------------------------------------------------
//               Implementation of the cascade vertex class
//              This is part of the Event Summary Data 
//              which contains the result of the reconstruction
//              and is the main set of classes for analaysis
//    Origin: Christian Kuhn, IReS, Strasbourg, christian.kuhn@ires.in2p3.fr
//     Modified by: Antonin Maire,IPHC, Antonin.Maire@ires.in2p3.fr
//            and  Boris Hippolyte,IPHC, hippolyt@in2p3.fr 
//-------------------------------------------------------------------------

#include <TDatabasePDG.h>
#include <TMath.h>
#include <TVector3.h>

#include "AliESDcascade.h"
#include "AliLog.h"

ClassImp(AliESDcascade)

AliESDcascade::AliESDcascade() : 
  AliESDv0(),
  fEffMassXi(TDatabasePDG::Instance()->GetParticle(kXiMinus)->Mass()),
  fChi2Xi(1024),
  fDcaXiDaughters(1024),
  fPdgCodeXi(kXiMinus),
  fBachIdx(-1)
{
  //--------------------------------------------------------------------
  // Default constructor  (Xi-)
  //--------------------------------------------------------------------
  for (Int_t j=0; j<3; j++) {
    fPosXi[j]=0.;
    fBachMom[j]=0.;
  }

  fPosCovXi[0]=1024;
  fPosCovXi[1]=fPosCovXi[2]=0.;
  fPosCovXi[3]=1024;
  fPosCovXi[4]=0.;
  fPosCovXi[5]=1024;

  fBachMomCov[0]=1024;
  fBachMomCov[1]=fBachMomCov[2]=0.;
  fBachMomCov[3]=1024;
  fBachMomCov[4]=0.;
  fBachMomCov[5]=1024;
}

AliESDcascade::AliESDcascade(const AliESDcascade& cas) :
  AliESDv0(cas),
  fEffMassXi(cas.fEffMassXi),
  fChi2Xi(cas.fChi2Xi),
  fDcaXiDaughters(cas.fDcaXiDaughters),
  fPdgCodeXi(cas.fPdgCodeXi),
  fBachIdx(cas.fBachIdx)
{
  //--------------------------------------------------------------------
  // The copy constructor
  //--------------------------------------------------------------------
  for (int i=0; i<3; i++) {
    fPosXi[i]     = cas.fPosXi[i];
    fBachMom[i] = cas.fBachMom[i];
  }
  for (int i=0; i<6; i++) {
    fPosCovXi[i]   = cas.fPosCovXi[i];
    fBachMomCov[i] = cas.fBachMomCov[i];
  }
}

AliESDcascade::AliESDcascade(const AliESDv0 &v,
			     const AliExternalTrackParam &t, Int_t i) : 
  AliESDv0(v),
  fEffMassXi(TDatabasePDG::Instance()->GetParticle(kXiMinus)->Mass()),
  fChi2Xi(1024),
  fDcaXiDaughters(1024),
  fPdgCodeXi(kXiMinus),
  fBachIdx(i)
{
  //--------------------------------------------------------------------
  // Main constructor  (Xi-)
  //--------------------------------------------------------------------

  Double_t r[3]; t.GetXYZ(r);
  Double_t x1=r[0], y1=r[1], z1=r[2]; // position of the bachelor
  Double_t p[3]; t.GetPxPyPz(p);
  Double_t px1=p[0], py1=p[1], pz1=p[2];// momentum of the bachelor track

  Double_t x2,y2,z2;          // position of the V0 
  v.GetXYZ(x2,y2,z2);    
  Double_t px2,py2,pz2;       // momentum of V0
  v.GetPxPyPz(px2,py2,pz2);

  Double_t a2=((x1-x2)*px2+(y1-y2)*py2+(z1-z2)*pz2)/(px2*px2+py2*py2+pz2*pz2);

  Double_t xm=x2+a2*px2;
  Double_t ym=y2+a2*py2;
  Double_t zm=z2+a2*pz2;

  // position of the cascade decay
  
  fPosXi[0]=0.5*(x1+xm); fPosXi[1]=0.5*(y1+ym); fPosXi[2]=0.5*(z1+zm);
    

  // invariant mass of the cascade (default is Ximinus)
  
  Double_t e1=TMath::Sqrt(0.13957*0.13957 + px1*px1 + py1*py1 + pz1*pz1);
  Double_t e2=TMath::Sqrt(1.11568*1.11568 + px2*px2 + py2*py2 + pz2*pz2);
  
  fEffMassXi=TMath::Sqrt((e1+e2)*(e1+e2)-
    (px1+px2)*(px1+px2)-(py1+py2)*(py1+py2)-(pz1+pz2)*(pz1+pz2));


  // momenta of the bachelor and the V0
  
  fBachMom[0]=px1; fBachMom[1]=py1; fBachMom[2]=pz1; 

  // Setting pdg code and fixing charge
  if (t.Charge()<0)
    fPdgCodeXi = kXiMinus;
  else
    fPdgCodeXi = kXiPlusBar;

  //PH Covariance matrices: to be calculated correctly in the future
  fPosCovXi[0]=1024;
  fPosCovXi[1]=fPosCovXi[2]=0.;
  fPosCovXi[3]=1024;
  fPosCovXi[4]=0.;
  fPosCovXi[5]=1024;

  fBachMomCov[0]=1024;
  fBachMomCov[1]=fBachMomCov[2]=0.;
  fBachMomCov[3]=1024;
  fBachMomCov[4]=0.;
  fBachMomCov[5]=1024;

  fChi2Xi=1024.; 

}

AliESDcascade& AliESDcascade::operator=(const AliESDcascade& cas)
{
  //--------------------------------------------------------------------
  // The assignment operator
  //--------------------------------------------------------------------

  if(this==&cas) return *this;
  AliESDv0::operator=(cas);

  fEffMassXi = cas.fEffMassXi;
  fChi2Xi    = cas.fChi2Xi;
  fDcaXiDaughters = cas.fDcaXiDaughters;
  fPdgCodeXi      = cas.fPdgCodeXi;
  fBachIdx        = cas.fBachIdx;
  for (int i=0; i<3; i++) {
    fPosXi[i]     = cas.fPosXi[i];
    fBachMom[i]   = cas.fBachMom[i];
  }
  for (int i=0; i<6; i++) {
    fPosCovXi[i]   = cas.fPosCovXi[i];
    fBachMomCov[i] = cas.fBachMomCov[i];
  }
  return *this;
}

void AliESDcascade::Copy(TObject &obj) const {
  
  // this overwrites the virtual TOBject::Copy()
  // to allow run time copying without casting
  // in AliESDEvent

  if(this==&obj)return;
  AliESDcascade *robj = dynamic_cast<AliESDcascade*>(&obj);
  if(!robj)return; // not an AliESDcascade
  *robj = *this;
}

AliESDcascade::~AliESDcascade() {
  //--------------------------------------------------------------------
  // Empty destructor
  //--------------------------------------------------------------------
}

// Start with AliVParticle functions
Double_t AliESDcascade::E() const {
  //--------------------------------------------------------------------
  // This gives the energy assuming the ChangeMassHypothesis was called
  //--------------------------------------------------------------------
  return E(fPdgCodeXi);
}

Double_t AliESDcascade::Y() const {
  //--------------------------------------------------------------------
  // This gives the energy assuming the ChangeMassHypothesis was called
  //--------------------------------------------------------------------
  return Y(fPdgCodeXi);
}

// Then extend AliVParticle functions
Double_t AliESDcascade::E(Int_t pdg) const {
  //--------------------------------------------------------------------
  // This gives the energy with the particle hypothesis as argument 
  //--------------------------------------------------------------------
  Double_t mass = TDatabasePDG::Instance()->GetParticle(pdg)->Mass();
  return TMath::Sqrt(mass*mass+P()*P());
}

Double_t AliESDcascade::Y(Int_t pdg) const {
  //--------------------------------------------------------------------
  // This gives the rapidity with the particle hypothesis as argument 
  //--------------------------------------------------------------------
  return 0.5*TMath::Log((E(pdg)+Pz())/(E(pdg)-Pz()+1.e-13));
}

// Now the functions for analysis consistency
Double_t AliESDcascade::RapXi() const {
  //--------------------------------------------------------------------
  // This gives the pseudorapidity assuming a (Anti) Xi particle
  //--------------------------------------------------------------------
  return Y(kXiMinus);
}

Double_t AliESDcascade::RapOmega() const {
  //--------------------------------------------------------------------
  // This gives the pseudorapidity assuming a (Anti) Omega particle
  //--------------------------------------------------------------------
  return Y(kOmegaMinus);
}

Double_t AliESDcascade::AlphaXi() const {
  //--------------------------------------------------------------------
  // This gives the Armenteros-Podolanski alpha
  //--------------------------------------------------------------------
  TVector3 momBach(fBachMom[0],fBachMom[1],fBachMom[2]);
  TVector3 momV0(fNmom[0]+fPmom[0],fNmom[1]+fPmom[1],fNmom[2]+fPmom[2]);
  TVector3 momTot(Px(),Py(),Pz());

  Double_t lQlBach = momBach.Dot(momTot)/momTot.Mag();
  Double_t lQlV0 = momV0.Dot(momTot)/momTot.Mag();

  return 1.-2./(1.+lQlBach/lQlV0);
}

Double_t AliESDcascade::PtArmXi() const {
  //--------------------------------------------------------------------
  // This gives the Armenteros-Podolanski ptarm
  //--------------------------------------------------------------------
  TVector3 momBach(fBachMom[0],fBachMom[1],fBachMom[2]);
  TVector3 momTot(Px(),Py(),Pz());

  return momBach.Perp(momTot);
}

// Then the older functions
Double_t AliESDcascade::ChangeMassHypothesis(Double_t &v0q, Int_t code) {
  //--------------------------------------------------------------------
  // This function changes the mass hypothesis for this cascade
  // and returns the "kinematical quality" of this hypothesis
  // together with the "quality" of associated V0 (argument v0q) 
  //--------------------------------------------------------------------
  Double_t nmass=0.13957, pmass=0.93827, ps0=0.101; 
  Double_t bmass=0.13957, mass =1.3213,  ps =0.139;

  if (Charge()*code<0)
    fPdgCodeXi = code;
  else {
    AliWarning("Chosen PDG code does not match the sign of the bachelor... Corrected !!");
    fPdgCodeXi = -code;
  }

  switch (fPdgCodeXi) {
  case kXiMinus:
       break;
  case kXiPlusBar:
       nmass=0.93827; pmass=0.13957; 
       break;
  case kOmegaMinus: 
       bmass=0.49368; mass=1.67245; ps=0.211;
       break;
  case kOmegaPlusBar: 
       nmass=0.93827; pmass=0.13957; 
       bmass=0.49368; mass=1.67245; ps=0.211;
       break;
  default:
       AliError("Invalide PDG code !  Assuming a Xi particle...");
       if (Charge()<0) {
	 fPdgCodeXi=kXiMinus;
       }
       else {
	 fPdgCodeXi=kXiPlusBar;
	 nmass=0.93827; pmass=0.13957; 
       }
    break;
  }

  Double_t pxn=fNmom[0], pyn=fNmom[1], pzn=fNmom[2];
  Double_t pxp=fPmom[0], pyp=fPmom[1], pzp=fPmom[2];

  Double_t px0=pxn+pxp, py0=pyn+pyp, pz0=pzn+pzp;
  Double_t p0=TMath::Sqrt(px0*px0 + py0*py0 + pz0*pz0);

  Double_t e0=TMath::Sqrt(1.11568*1.11568 + p0*p0);
  Double_t beta0=p0/e0;
  Double_t pln=(pxn*px0 + pyn*py0 + pzn*pz0)/p0;
  Double_t plp=(pxp*px0 + pyp*py0 + pzp*pz0)/p0;
  Double_t pt2=pxp*pxp + pyp*pyp + pzp*pzp - plp*plp;

  Double_t a=(plp-pln)/(plp+pln);
  a -= (pmass*pmass-nmass*nmass)/(1.11568*1.11568);
  a = 0.25*beta0*beta0*1.11568*1.11568*a*a + pt2;


  v0q=a - ps0*ps0;


  Double_t pxb=fBachMom[0], pyb=fBachMom[1], pzb=fBachMom[2]; 

  Double_t eb=TMath::Sqrt(bmass*bmass + pxb*pxb + pyb*pyb + pzb*pzb);
  Double_t pxl=px0+pxb, pyl=py0+pyb, pzl=pz0+pzb;
  Double_t pl=TMath::Sqrt(pxl*pxl + pyl*pyl + pzl*pzl);
  
  fEffMassXi=TMath::Sqrt(((e0+eb)-pl)*((e0+eb)+pl));

  Double_t beta=pl/(e0+eb);
  Double_t pl0=(px0*pxl + py0*pyl + pz0*pzl)/pl;
  Double_t plb=(pxb*pxl + pyb*pyl + pzb*pzl)/pl;
  pt2=p0*p0 - pl0*pl0;

  a=(pl0-plb)/(pl0+plb);
  a -= (1.11568*1.11568-bmass*bmass)/(mass*mass);
  a = 0.25*beta*beta*mass*mass*a*a + pt2;

  return (a - ps*ps);
}

void 
AliESDcascade::GetPxPyPz(Double_t &px, Double_t &py, Double_t &pz) const {
  //--------------------------------------------------------------------
  // This function returns the cascade momentum (global)
  //--------------------------------------------------------------------
  px=fNmom[0]+fPmom[0]+fBachMom[0];
  py=fNmom[1]+fPmom[1]+fBachMom[1]; 
  pz=fNmom[2]+fPmom[2]+fBachMom[2]; 
}

void AliESDcascade::GetXYZcascade(Double_t &x, Double_t &y, Double_t &z) const {
  //--------------------------------------------------------------------
  // This function returns cascade position (global)
  //--------------------------------------------------------------------
  x=fPosXi[0];
  y=fPosXi[1];
  z=fPosXi[2];
}

Double_t AliESDcascade::GetDcascade(Double_t x0, Double_t y0, Double_t z0) const {
  //--------------------------------------------------------------------
  // This function returns the cascade impact parameter
  //--------------------------------------------------------------------

  Double_t x=fPosXi[0],y=fPosXi[1],z=fPosXi[2];
  Double_t px=fNmom[0]+fPmom[0]+fBachMom[0];
  Double_t py=fNmom[1]+fPmom[1]+fBachMom[1];
  Double_t pz=fNmom[2]+fPmom[2]+fBachMom[2];

  Double_t dx=(y0-y)*pz - (z0-z)*py; 
  Double_t dy=(x0-x)*pz - (z0-z)*px;
  Double_t dz=(x0-x)*py - (y0-y)*px;
  Double_t d=TMath::Sqrt((dx*dx+dy*dy+dz*dz)/(px*px+py*py+pz*pz));

  return d;
}

Double_t AliESDcascade::GetCascadeCosineOfPointingAngle(Double_t refPointX, Double_t refPointY, Double_t refPointZ) const {
  // calculates the pointing angle of the cascade wrt a reference point

  Double_t momCas[3]; //momentum of the cascade
  GetPxPyPz(momCas[0],momCas[1],momCas[2]);

  Double_t deltaPos[3]; //vector between the reference point and the cascade vertex
  deltaPos[0] = fPosXi[0] - refPointX;
  deltaPos[1] = fPosXi[1] - refPointY;
  deltaPos[2] = fPosXi[2] - refPointZ;

  Double_t momCas2    = momCas[0]*momCas[0] + momCas[1]*momCas[1] + momCas[2]*momCas[2];
  Double_t deltaPos2 = deltaPos[0]*deltaPos[0] + deltaPos[1]*deltaPos[1] + deltaPos[2]*deltaPos[2];

  Double_t cosinePointingAngle = (deltaPos[0]*momCas[0] +
				  deltaPos[1]*momCas[1] +
				  deltaPos[2]*momCas[2] ) /
    TMath::Sqrt(momCas2 * deltaPos2);
  
  return cosinePointingAngle;
}

void AliESDcascade::GetPosCovXi(Double_t cov[6]) const {

  for (Int_t i=0; i<6; ++i) cov[i] = fPosCovXi[i];
}
