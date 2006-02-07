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
//
//    Implementation of the ESD V0MI vertex class
//            This class is part of the Event Data Summary
//            set of classes and contains information about
//            V0 kind vertexes generated by a neutral particle
//    Numerical part - AliHelix functionality used             
//    
//    Likelihoods for Point angle, DCA and Causality defined => can be used as cut parameters
//    HIGHLY recomended
//                                 
//    Quality information can be used as additional cut variables
//
//    Origin: Marian Ivanov marian.ivanov@cern.ch
//-------------------------------------------------------------------------

#include <Riostream.h>
#include <TMath.h>

#include "AliESDV0MI.h"
#include "AliHelix.h"


ClassImp(AliESDV0MI)

AliESDV0MIParams  AliESDV0MI::fgkParams;


AliESDV0MI::AliESDV0MI() :
  AliESDv0(),
  fParamP(),
  fParamM(),
  fID(0),
  fDist1(-1),
  fDist2(-1),
  fRr(-1),
  fStatus(0),
  fRow0(-1),
  fDistNorm(0),
  fDistSigma(0),
  fChi2Before(0),
  fNBefore(0),
  fChi2After(0),
  fNAfter(0),
  fPointAngleFi(0),
  fPointAngleTh(0),
  fPointAngle(0)
{
  //
  //Dafault constructor
  //
  for (Int_t i=0;i<5;i++){
    fRP[i]=fRM[i]=0;
  }
  fLab[0]=fLab[1]=-1;
  fIndex[0]=fIndex[1]=-1;
  for (Int_t i=0;i<6;i++){fClusters[0][i]=0; fClusters[1][i]=0;}
  fNormDCAPrim[0]=fNormDCAPrim[1]=0;
  for (Int_t i=0;i<3;i++){fPP[i]=fPM[i]=fXr[i]=fAngle[i]=0;}
  for (Int_t i=0;i<3;i++){fOrder[i]=0;}
  for (Int_t i=0;i<4;i++){fCausality[i]=0;}
}

Double_t AliESDV0MI::GetSigmaY(){
  //
  // return sigmay in y  at vertex position  using covariance matrix 
  //
  const Double_t * cp  = fParamP.GetCovariance();
  const Double_t * cm  = fParamM.GetCovariance();
  Double_t sigmay = cp[0]+cm[0]+ cp[5]*(fParamP.GetX()-fRr)*(fParamP.GetX()-fRr)+ cm[5]*(fParamM.GetX()-fRr)*(fParamM.GetX()-fRr);
  return (sigmay>0) ? TMath::Sqrt(sigmay):100;
}

Double_t AliESDV0MI::GetSigmaZ(){
  //
  // return sigmay in y  at vertex position  using covariance matrix 
  //
  const Double_t * cp  = fParamP.GetCovariance();
  const Double_t * cm  = fParamM.GetCovariance();
  Double_t sigmaz = cp[2]+cm[2]+ cp[9]*(fParamP.GetX()-fRr)*(fParamP.GetX()-fRr)+ cm[9]*(fParamM.GetX()-fRr)*(fParamM.GetX()-fRr);
  return (sigmaz>0) ? TMath::Sqrt(sigmaz):100;
}

Double_t AliESDV0MI::GetSigmaD0(){
  //
  // Sigma parameterization using covariance matrix
  //
  // sigma of distance between two tracks in vertex position 
  // sigma of DCA is proportianal to sigmaD0
  // factor 2 difference is explained by the fact that the DCA is calculated at the position 
  // where the tracks as closest together ( not exact position of the vertex)
  //
  const Double_t * cp      = fParamP.GetCovariance();
  const Double_t * cm      = fParamM.GetCovariance();
  Double_t sigmaD0   = cp[0]+cm[0]+cp[2]+cm[2]+fgkParams.fPSigmaOffsetD0*fgkParams.fPSigmaOffsetD0;
  sigmaD0           += ((fParamP.GetX()-fRr)*(fParamP.GetX()-fRr))*(cp[5]+cp[9]);
  sigmaD0           += ((fParamM.GetX()-fRr)*(fParamM.GetX()-fRr))*(cm[5]+cm[9]);
  return (sigmaD0>0)? TMath::Sqrt(sigmaD0):100;
}


Double_t AliESDV0MI::GetSigmaAP0(){
  //
  //Sigma parameterization using covariance matrices
  //
  Double_t prec  = TMath::Sqrt((fPM[0]+fPP[0])*(fPM[0]+fPP[0])
			       +(fPM[1]+fPP[1])*(fPM[1]+fPP[1])
			       +(fPM[2]+fPP[2])*(fPM[2]+fPP[2]));
  Double_t normp = TMath::Sqrt(fPP[0]*fPP[0]+fPP[1]*fPP[1]+fPP[2]*fPP[2])/prec;  // fraction of the momenta
  Double_t normm = TMath::Sqrt(fPM[0]*fPM[0]+fPM[1]*fPM[1]+fPM[2]*fPM[2])/prec;  
  const Double_t * cp      = fParamP.GetCovariance();
  const Double_t * cm      = fParamM.GetCovariance();
  Double_t sigmaAP0 = fgkParams.fPSigmaOffsetAP0*fgkParams.fPSigmaOffsetAP0;                           // minimal part
  sigmaAP0 +=  (cp[5]+cp[9])*(normp*normp)+(cm[5]+cm[9])*(normm*normm);          // angular resolution part
  Double_t sigmaAP1 = GetSigmaD0()/(TMath::Abs(fRr)+0.01);                       // vertex position part
  sigmaAP0 +=  0.5*sigmaAP1*sigmaAP1;                              
  return (sigmaAP0>0)? TMath::Sqrt(sigmaAP0):100;
}

Double_t AliESDV0MI::GetEffectiveSigmaD0(){
  //
  // minimax - effective Sigma parameterization 
  // p12 effective curvature and v0 radius postion used as parameters  
  //  
  Double_t p12 = TMath::Sqrt(fParamP.GetParameter()[4]*fParamP.GetParameter()[4]+
			     fParamM.GetParameter()[4]*fParamM.GetParameter()[4]);
  Double_t sigmaED0= TMath::Max(TMath::Sqrt(fRr)-fgkParams.fPSigmaRminDE,0.0)*fgkParams.fPSigmaCoefDE*p12*p12;
  sigmaED0*= sigmaED0;
  sigmaED0*= sigmaED0;
  sigmaED0 = TMath::Sqrt(sigmaED0+fgkParams.fPSigmaOffsetDE*fgkParams.fPSigmaOffsetDE);
  return (sigmaED0<fgkParams.fPSigmaMaxDE) ? sigmaED0: fgkParams.fPSigmaMaxDE;
}


Double_t AliESDV0MI::GetEffectiveSigmaAP0(){
  //
  // effective Sigma parameterization of point angle resolution 
  //
  Double_t p12 = TMath::Sqrt(fParamP.GetParameter()[4]*fParamP.GetParameter()[4]+
			     fParamM.GetParameter()[4]*fParamM.GetParameter()[4]);
  Double_t sigmaAPE= fgkParams.fPSigmaBase0APE;
  sigmaAPE+= fgkParams.fPSigmaR0APE/(fgkParams.fPSigmaR1APE+fRr);
  sigmaAPE*= (fgkParams.fPSigmaP0APE+fgkParams.fPSigmaP1APE*p12);
  sigmaAPE = TMath::Min(sigmaAPE,fgkParams.fPSigmaMaxAPE);
  return sigmaAPE;
}


Double_t  AliESDV0MI::GetMinimaxSigmaAP0(){
  //
  // calculate mini-max effective sigma of point angle resolution
  //
  //compv0->fTree->SetAlias("SigmaAP2","max(min((SigmaAP0+SigmaAPE0)*0.5,1.5*SigmaAPE0),0.5*SigmaAPE0+0.003)");
  Double_t    effectiveSigma = GetEffectiveSigmaAP0();
  Double_t    sigmaMMAP = 0.5*(GetSigmaAP0()+effectiveSigma);
  sigmaMMAP  = TMath::Min(sigmaMMAP, fgkParams.fPMaxFractionAP0*effectiveSigma);
  sigmaMMAP  = TMath::Max(sigmaMMAP, fgkParams.fPMinFractionAP0*effectiveSigma+fgkParams.fPMinAP0);
  return sigmaMMAP;
}
Double_t  AliESDV0MI::GetMinimaxSigmaD0(){
  //
  // calculate mini-max sigma of dca resolution
  // 
  //compv0->fTree->SetAlias("SigmaD2","max(min((SigmaD0+SigmaDE0)*0.5,1.5*SigmaDE0),0.5*SigmaDE0)");
  Double_t    effectiveSigma = GetEffectiveSigmaD0();
  Double_t    sigmaMMD0 = 0.5*(GetSigmaD0()+effectiveSigma);
  sigmaMMD0  = TMath::Min(sigmaMMD0, fgkParams.fPMaxFractionD0*effectiveSigma);
  sigmaMMD0  = TMath::Max(sigmaMMD0, fgkParams.fPMinFractionD0*effectiveSigma+fgkParams.fPMinD0);
  return sigmaMMD0;
}


Double_t AliESDV0MI::GetLikelihoodAP(Int_t mode0, Int_t mode1){
  //
  // get likelihood for point angle
  //
  Double_t sigmaAP = 0.007;            //default sigma
  switch (mode0){
  case 0:
    sigmaAP = GetSigmaAP0();           // mode 0  - covariance matrix estimates used 
    break;
  case 1:
    sigmaAP = GetEffectiveSigmaAP0();  // mode 1 - effective sigma used
    break;
  case 2:
    sigmaAP = GetMinimaxSigmaAP0();    // mode 2 - minimax sigma
    break;
  }
  Double_t apNorm = TMath::Min(TMath::ACos(fPointAngle)/sigmaAP,50.);  
  //normalized point angle, restricted - because of overflow problems in Exp
  Double_t likelihood = 0;
  switch(mode1){
  case 0:
    likelihood = TMath::Exp(-0.5*apNorm*apNorm);   
    // one component
    break;
  case 1:
    likelihood = (TMath::Exp(-0.5*apNorm*apNorm)+0.5* TMath::Exp(-0.25*apNorm*apNorm))/1.5;
    // two components
    break;
  case 2:
    likelihood = (TMath::Exp(-0.5*apNorm*apNorm)+0.5* TMath::Exp(-0.25*apNorm*apNorm)+0.25*TMath::Exp(-0.125*apNorm*apNorm))/1.75;
    // three components
    break;
  }
  return likelihood;
}

Double_t AliESDV0MI::GetLikelihoodD(Int_t mode0, Int_t mode1){
  //
  // get likelihood for DCA
  //
  Double_t sigmaD = 0.03;            //default sigma
  switch (mode0){
  case 0:
    sigmaD = GetSigmaD0();           // mode 0  - covariance matrix estimates used 
    break;
  case 1:
    sigmaD = GetEffectiveSigmaD0();  // mode 1 - effective sigma used
    break;
  case 2:
    sigmaD = GetMinimaxSigmaD0();    // mode 2 - minimax sigma
    break;
  }
  Double_t dNorm = TMath::Min(fDist2/sigmaD,50.);  
  //normalized point angle, restricted - because of overflow problems in Exp
  Double_t likelihood = 0;
  switch(mode1){
  case 0:
    likelihood = TMath::Exp(-2.*dNorm);   
    // one component
    break;
  case 1:
    likelihood = (TMath::Exp(-2.*dNorm)+0.5* TMath::Exp(-dNorm))/1.5;
    // two components
    break;
  case 2:
    likelihood = (TMath::Exp(-2.*dNorm)+0.5* TMath::Exp(-dNorm)+0.25*TMath::Exp(-0.5*dNorm))/1.75;
    // three components
    break;
  }
  return likelihood;

}

Double_t AliESDV0MI::GetLikelihoodC(Int_t mode0, Int_t /*mode1*/){
  //
  // get likelihood for Causality
  // !!!  Causality variables defined in AliITStrackerMI !!! 
  //      when more information was available
  //  
  Double_t likelihood = 0.5;
  Double_t minCausal  = TMath::Min(fCausality[0],fCausality[1]);
  Double_t maxCausal  = TMath::Max(fCausality[0],fCausality[1]);
  //  minCausal           = TMath::Max(minCausal,0.5*maxCausal);
  //compv0->fTree->SetAlias("LCausal","(1.05-(2*(0.8-exp(-max(RC.fV0rec.fCausality[0],RC.fV0rec.fCausality[1])))+2*(0.8-exp(-min(RC.fV0rec.fCausality[0],RC.fV0rec.fCausality[1]))))/2)**4");
  
  switch(mode0){
  case 0:
    //normalization 
    likelihood = TMath::Power((1.05-2*(0.8-TMath::Exp(-maxCausal))),4.);
    break;
  case 1:
    likelihood = TMath::Power(1.05-(2*(0.8-TMath::Exp(-maxCausal))+(2*(0.8-TMath::Exp(-minCausal))))*0.5,4.);
    break;
  }
  return likelihood;
  
}

void AliESDV0MI::SetCausality(Float_t pb0, Float_t pb1, Float_t pa0, Float_t pa1)
{
  //
  // set probabilities
  //
  fCausality[0] = pb0;     // probability - track 0 exist before vertex
  fCausality[1] = pb1;     // probability - track 1 exist before vertex
  fCausality[2] = pa0;     // probability - track 0 exist close after vertex
  fCausality[3] = pa1;     // probability - track 1 exist close after vertex
}
void  AliESDV0MI::SetClusters(Int_t *clp, Int_t *clm)
{
  //
  // Set its clusters indexes
  //
  for (Int_t i=0;i<6;i++) fClusters[0][i] = clp[i]; 
  for (Int_t i=0;i<6;i++) fClusters[1][i] = clm[i]; 
}


void AliESDV0MI::SetP(const AliExternalTrackParam & paramp)  {
  //
  // set track +
  //
  fParamP   = paramp;
}

void AliESDV0MI::SetM(const AliExternalTrackParam & paramm){
  //
  //set track -
  //
  fParamM = paramm;
}
  
void AliESDV0MI::SetRp(const Double_t *rp){
  //
  // set pid +
  //
  for (Int_t i=0;i<5;i++) fRP[i]=rp[i];
}

void AliESDV0MI::SetRm(const Double_t *rm){
  //
  // set pid -
  //
  for (Int_t i=0;i<5;i++) fRM[i]=rm[i];
}


void  AliESDV0MI::UpdatePID(Double_t pidp[5], Double_t pidm[5])
{
  //
  // set PID hypothesy
  //
  // norm PID to 1
  Float_t sump =0;
  Float_t summ =0;
  for (Int_t i=0;i<5;i++){
    fRP[i]=pidp[i];
    sump+=fRP[i];
    fRM[i]=pidm[i];
    summ+=fRM[i];
  }
  for (Int_t i=0;i<5;i++){
    fRP[i]/=sump;
    fRM[i]/=summ;
  }
}

Float_t AliESDV0MI::GetProb(UInt_t p1, UInt_t p2){
  //
  //
  //
  //
  return TMath::Max(fRP[p1]+fRM[p2], fRP[p2]+fRM[p1]);
}

Float_t AliESDV0MI::GetEffMass(UInt_t p1, UInt_t p2){
  //
  // calculate effective mass
  //
  const Float_t kpmass[5] = {5.10000000000000037e-04,1.05660000000000004e-01,1.39570000000000000e-01,
		      4.93599999999999983e-01, 9.38270000000000048e-01};
  if (p1>4) return -1;
  if (p2>4) return -1;
  Float_t mass1 = kpmass[p1]; 
  Float_t mass2 = kpmass[p2];   
  Double_t *m1 = fPP;
  Double_t *m2 = fPM;
  //
  //if (fRP[p1]+fRM[p2]<fRP[p2]+fRM[p1]){
  //  m1 = fPM;
  //  m2 = fPP;
  //}
  //
  Float_t e1    = TMath::Sqrt(mass1*mass1+
                              m1[0]*m1[0]+
                              m1[1]*m1[1]+
                              m1[2]*m1[2]);
  Float_t e2    = TMath::Sqrt(mass2*mass2+
                              m2[0]*m2[0]+
                              m2[1]*m2[1]+
                              m2[2]*m2[2]);  
  Float_t mass =  
    (m2[0]+m1[0])*(m2[0]+m1[0])+
    (m2[1]+m1[1])*(m2[1]+m1[1])+
    (m2[2]+m1[2])*(m2[2]+m1[2]);
  
  mass = TMath::Sqrt((e1+e2)*(e1+e2)-mass);
  return mass;
}

void  AliESDV0MI::Update(Float_t vertex[3])
{
  //
  // updates Kink Info
  //
  //  Float_t distance1,distance2;
  Float_t distance2;
  //
  AliHelix phelix(fParamP);
  AliHelix mhelix(fParamM);    
  //
  //find intersection linear
  //
  Double_t phase[2][2],radius[2];
  Int_t  points = phelix.GetRPHIintersections(mhelix, phase, radius,200);
  Double_t delta1=10000,delta2=10000;  
  /*
  if (points<=0) return;
  if (points>0){
    phelix.LinearDCA(mhelix,phase[0][0],phase[0][1],radius[0],delta1);
    phelix.LinearDCA(mhelix,phase[0][0],phase[0][1],radius[0],delta1);
    phelix.LinearDCA(mhelix,phase[0][0],phase[0][1],radius[0],delta1);
  }
  if (points==2){    
    phelix.LinearDCA(mhelix,phase[1][0],phase[1][1],radius[1],delta2);
    phelix.LinearDCA(mhelix,phase[1][0],phase[1][1],radius[1],delta2);
    phelix.LinearDCA(mhelix,phase[1][0],phase[1][1],radius[1],delta2);
  }
  distance1 = TMath::Min(delta1,delta2);
  */
  //
  //find intersection parabolic
  //
  points = phelix.GetRPHIintersections(mhelix, phase, radius);
  delta1=10000,delta2=10000;  
  Double_t d1=1000.,d2=10000.;
  Double_t err[3],angles[3];
  if (points<=0) return;
  if (points>0){
    phelix.ParabolicDCA(mhelix,phase[0][0],phase[0][1],radius[0],delta1);
    phelix.ParabolicDCA(mhelix,phase[0][0],phase[0][1],radius[0],delta1);
    if (TMath::Abs(fParamP.GetX()-TMath::Sqrt(radius[0])<3) && TMath::Abs(fParamM.GetX()-TMath::Sqrt(radius[0])<3)){
      // if we are close to vertex use error parama
      //
      err[1] = fParamP.GetCovariance()[0]+fParamM.GetCovariance()[0]+0.05*0.05
	+0.3*(fParamP.GetCovariance()[2]+fParamM.GetCovariance()[2]);
      err[2] = fParamP.GetCovariance()[2]+fParamM.GetCovariance()[2]+0.05*0.05
	+0.3*(fParamP.GetCovariance()[0]+fParamM.GetCovariance()[0]);
      
      phelix.GetAngle(phase[0][0],mhelix,phase[0][1],angles);
      Double_t tfi  = TMath::Abs(TMath::Tan(angles[0]));
      Double_t tlam = TMath::Abs(TMath::Tan(angles[1]));
      err[0] = err[1]/((0.2+tfi)*(0.2+tfi))+err[2]/((0.2+tlam)*(0.2+tlam));
      err[0] = ((err[1]*err[2]/((0.2+tfi)*(0.2+tfi)*(0.2+tlam)*(0.2+tlam))))/err[0];
      phelix.ParabolicDCA2(mhelix,phase[0][0],phase[0][1],radius[0],delta1,err);
    }
    Double_t xd[3],xm[3];
    phelix.Evaluate(phase[0][0],xd);
    mhelix.Evaluate(phase[0][1],xm);
    d1 = (xd[0]-xm[0])*(xd[0]-xm[0])+(xd[1]-xm[1])*(xd[1]-xm[1])+(xd[2]-xm[2])*(xd[2]-xm[2]);
  }
  if (points==2){    
    phelix.ParabolicDCA(mhelix,phase[1][0],phase[1][1],radius[1],delta2);
    phelix.ParabolicDCA(mhelix,phase[1][0],phase[1][1],radius[1],delta2);
    if (TMath::Abs(fParamP.GetX()-TMath::Sqrt(radius[1])<3) && TMath::Abs(fParamM.GetX()-TMath::Sqrt(radius[1])<3)){
      // if we are close to vertex use error paramatrization
      //
      err[1] = fParamP.GetCovariance()[0]+fParamM.GetCovariance()[0]+0.05*0.05
	+0.3*(fParamP.GetCovariance()[2]+fParamM.GetCovariance()[2]);
      err[2] = fParamP.GetCovariance()[2]+fParamM.GetCovariance()[2]+0.05*0.05
	+0.3*(fParamP.GetCovariance()[0]+fParamM.GetCovariance()[0]);
      
      phelix.GetAngle(phase[1][0],mhelix,phase[1][1],angles);
      Double_t tfi  = TMath::Abs(TMath::Tan(angles[0]));
      Double_t tlam = TMath::Abs(TMath::Tan(angles[1]));
      err[0] = err[1]/((0.2+tfi)*(0.2+tfi))+err[2]/((0.2+tlam)*(0.2+tlam));     
      err[0] = ((err[1]*err[2]/((0.2+tfi)*(0.2+tfi)*(0.2+tlam)*(0.2+tlam))))/err[0];
      phelix.ParabolicDCA2(mhelix,phase[1][0],phase[1][1],radius[1],delta2,err);
    }
    Double_t xd[3],xm[3];
    phelix.Evaluate(phase[1][0],xd);
    mhelix.Evaluate(phase[1][1],xm);
    d2 = (xd[0]-xm[0])*(xd[0]-xm[0])+(xd[1]-xm[1])*(xd[1]-xm[1])+(xd[2]-xm[2])*(xd[2]-xm[2]);
  }
  //
  distance2 = TMath::Min(delta1,delta2);
  if (delta1<delta2){
    //get V0 info
    Double_t xd[3],xm[3];
    phelix.Evaluate(phase[0][0],xd);
    mhelix.Evaluate(phase[0][1], xm);
    fXr[0] = 0.5*(xd[0]+xm[0]);
    fXr[1] = 0.5*(xd[1]+xm[1]);
    fXr[2] = 0.5*(xd[2]+xm[2]);

    Float_t wy = fParamP.GetCovariance()[0]/(fParamP.GetCovariance()[0]+fParamM.GetCovariance()[0]);
    Float_t wz = fParamP.GetCovariance()[2]/(fParamP.GetCovariance()[2]+fParamM.GetCovariance()[2]);
    fXr[0] = 0.5*( (1.-wy)*xd[0]+ wy*xm[0] + (1.-wz)*xd[0]+ wz*xm[0] );
    fXr[1] = (1.-wy)*xd[1]+ wy*xm[1];
    fXr[2] = (1.-wz)*xd[2]+ wz*xm[2];
    //
    phelix.GetMomentum(phase[0][0],fPP);
    mhelix.GetMomentum(phase[0][1],fPM);
    phelix.GetAngle(phase[0][0],mhelix,phase[0][1],fAngle);
    fRr = TMath::Sqrt(fXr[0]*fXr[0]+fXr[1]*fXr[1]);
  }
  else{
    Double_t xd[3],xm[3];
    phelix.Evaluate(phase[1][0],xd);
    mhelix.Evaluate(phase[1][1], xm);
    fXr[0] = 0.5*(xd[0]+xm[0]);
    fXr[1] = 0.5*(xd[1]+xm[1]);
    fXr[2] = 0.5*(xd[2]+xm[2]);
    Float_t wy = fParamP.GetCovariance()[0]/(fParamP.GetCovariance()[0]+fParamM.GetCovariance()[0]);
    Float_t wz = fParamP.GetCovariance()[2]/(fParamP.GetCovariance()[2]+fParamM.GetCovariance()[2]);
    fXr[0] = 0.5*( (1.-wy)*xd[0]+ wy*xm[0] + (1.-wz)*xd[0]+ wz*xm[0] );
    fXr[1] = (1.-wy)*xd[1]+ wy*xm[1];
    fXr[2] = (1.-wz)*xd[2]+ wz*xm[2];
    //
    phelix.GetMomentum(phase[1][0], fPP);
    mhelix.GetMomentum(phase[1][1], fPM);
    phelix.GetAngle(phase[1][0],mhelix,phase[1][1],fAngle);
    fRr = TMath::Sqrt(fXr[0]*fXr[0]+fXr[1]*fXr[1]);
  }
  fDist1 = TMath::Sqrt(TMath::Min(d1,d2));
  fDist2 = TMath::Sqrt(distance2);      
  //            
  //
  Double_t v[3] = {fXr[0]-vertex[0],fXr[1]-vertex[1],fXr[2]-vertex[2]};
  Double_t p[3] = {fPP[0]+fPM[0], fPP[1]+fPM[1],fPP[2]+fPM[2]};
  Double_t vnorm2 = v[0]*v[0]+v[1]*v[1];
  if (TMath::Abs(v[2])>100000) return;
  Double_t vnorm3 = TMath::Sqrt(TMath::Abs(v[2]*v[2]+vnorm2));
  vnorm2 = TMath::Sqrt(vnorm2);
  Double_t pnorm2 = p[0]*p[0]+p[1]*p[1];
  Double_t pnorm3 = TMath::Sqrt(p[2]*p[2]+pnorm2);
  pnorm2 = TMath::Sqrt(pnorm2);  
  fPointAngleFi = (v[0]*p[0]+v[1]*p[1])/(vnorm2*pnorm2);
  fPointAngleTh = (v[2]*p[2]+vnorm2*pnorm2)/(vnorm3*pnorm3);  
  fPointAngle   = (v[0]*p[0]+v[1]*p[1]+v[2]*p[2])/(vnorm3*pnorm3);
  //
}

