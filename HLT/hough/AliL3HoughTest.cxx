//$Id$

// Author: Anders Vestbo <mailto:vestbo@fi.uib.no>
//*-- Copyright &copy ASV 

#include "AliL3StandardIncludes.h"
#include "AliL3HoughTest.h"
#include "AliL3ModelTrack.h"
#include "AliL3Transform.h"
#include "AliL3Histogram.h"
#include "TRandom.h"
#include "TMath.h"
#include "TH2.h"
#include "TH3.h"

#if GCCVERSION == 3
using namespace std;
#endif

//_____________________________________________________________
// AliL3HoughTest


ClassImp(AliL3HoughTest)

AliL3HoughTest::AliL3HoughTest()
{
  fData=0;
}


AliL3HoughTest::~AliL3HoughTest()
{
  if(fData)
    delete [] fData;
}

Bool_t AliL3HoughTest::GenerateTrackData(Double_t pt,Double_t psi,Double_t tgl,Int_t sign,Int_t patch,Int_t minhits)
{
  fCurrentPatch=patch;
  if(fData)
    delete fData;
  fData = new SimData[AliL3Transform::GetNRows(patch)];
  memset(fData,0,AliL3Transform::GetNRows(patch)*sizeof(SimData));
  
  AliL3ModelTrack *track = new AliL3ModelTrack();
  track->Init(0,patch);
  track->SetPt(pt);
  track->SetPsi(psi);
  track->SetTgl(tgl);
  track->SetCharge(sign);
  track->SetFirstPoint(0,0,0);
  track->CalculateHelix();

  Int_t temp[200];
  Int_t temp2[AliL3Transform::GetNTimeBins()];
  Int_t entries=100;
  Int_t clustercharge=100;
  Int_t hitcounter=0;
  for(Int_t i=AliL3Transform::GetFirstRow(patch); i<=AliL3Transform::GetLastRow(patch); i++)
    {
      Float_t xyz[3];
      
      if(!track->GetCrossingPoint(i,xyz))
	continue;
      
      Int_t rowindex = i - AliL3Transform::GetFirstRow(patch);
      Int_t sector,row;
      AliL3Transform::Slice2Sector(0,i,sector,row);
      AliL3Transform::Local2Raw(xyz,sector,row);

      if(xyz[1] < 0 || xyz[1] >= AliL3Transform::GetNPads(i) || xyz[2] < 0 || xyz[2] >= AliL3Transform::GetNTimeBins())
	continue;
      hitcounter++;
      track->SetPadHit(i,xyz[1]);
      track->SetTimeHit(i,xyz[2]);
      
      memset(temp,0,200*sizeof(Int_t));
      memset(temp2,0,AliL3Transform::GetNTimeBins()*sizeof(Int_t));
      Double_t xysigma = sqrt(track->GetParSigmaY2(i));
      Double_t zsigma = sqrt(track->GetParSigmaZ2(i));
      //cout<<i<<" "<<xyz[1]<<" "<<xyz[2]<<" "<<xysigma<<" "<<zsigma<<endl;
      Int_t minpad=200,j;
      Int_t mintime = 1000;
      for(j=0; j<entries; j++)
	{
	  Int_t pad = TMath::Nint(gRandom->Gaus(xyz[1],xysigma));
	  Int_t time = TMath::Nint(gRandom->Gaus(xyz[2],zsigma));
	  if(pad < 0 || pad >= AliL3Transform::GetNPads(i) || time < 0 || time >= AliL3Transform::GetNTimeBins())
	    continue;
	  temp[pad]++;
	  temp2[time]++;
	  if(pad < minpad)
	    minpad=pad;
	  if(time < mintime)
	    mintime=time;
	}
      Int_t npads=0;
      for(j=0; j<200; j++)
	{
	  if(temp[j]==0) continue;
	  
	  Int_t index = j - minpad;
	  
	  if(index < 0 || index >= 10)
	    {
	      cerr<<"AliL3HoughTest::GenerateTrackData : Wrong index "<<index<<endl;
	      exit(5);
	    }
	  npads++;
	  Int_t seq_charge = clustercharge*temp[j]/entries;
	  Int_t ntimes=0;
	  for(Int_t k=0; k<AliL3Transform::GetNTimeBins(); k++)
	    {
	      if(temp2[k]==0) continue;
	      Int_t tindex = k - mintime;
	      if(tindex < 0 || tindex >= 10)
		{
		  cerr<<"AliL3HoughTest::GenerateTrackData : Wrong timeindex "<<tindex<<" "<<k<<" "<<mintime<<endl;
		  exit(5);
		}
	      Int_t charge = seq_charge*temp2[k]/entries;
	      if(charge < 3 ) 
		continue;
	      if(charge > 1023)
		charge=1023;
	      //cout<<"row "<<i<<" pad "<<j<<" time "<<k<<" charge "<<charge<<endl;
	      ntimes++;
	      fData[rowindex].pads[index][tindex]=charge;
	    }
	}
      fData[rowindex].minpad=minpad;
      fData[rowindex].mintime=mintime;
      fData[rowindex].npads=npads;
    }
  delete track;
  if(hitcounter < minhits)
    return kFALSE;
  return kTRUE;
}

void AliL3HoughTest::Transform2Circle(AliL3Histogram *hist)
{
  if(!fData)
    {
      cerr<<"AliL3HoughTest::Transform : No data"<<endl;
      return;
    }
  Float_t R,phi,phi0,kappa,xyz[3];
  Int_t pad,time,charge,sector,row;
  for(Int_t i=AliL3Transform::GetFirstRow(fCurrentPatch); i<=AliL3Transform::GetLastRow(fCurrentPatch); i++)
    {
      Int_t rowindex = i - AliL3Transform::GetFirstRow(fCurrentPatch);
      AliL3Transform::Slice2Sector(0,i,sector,row);
      for(Int_t j=0; j<fData[rowindex].npads; j++)
	{
	  pad = j + fData[rowindex].minpad;
	  for(Int_t k=0; k<10; k++)
	    {
	      time = k + fData[rowindex].mintime;
	      charge = fData[rowindex].pads[j][k];
	      if(charge == 0) continue;
	      AliL3Transform::Raw2Local(xyz,sector,row,pad,time);
	      
	      R = sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1]);
	      
	      phi = AliL3Transform::GetPhi(xyz);
	      
	      for(Int_t k=hist->GetFirstYbin(); k<=hist->GetLastYbin(); k++)
		{
		  phi0 = hist->GetBinCenterY(k);
		  kappa = 2*sin(phi-phi0)/R;
		  hist->Fill(kappa,phi0,charge);
		}
	    }
	}
    }
}

void AliL3HoughTest::Transform2CircleC(AliL3Histogram *hist)
{
  if(!fData)
    {
      cerr<<"AliL3HoughTest::TransformC : No data"<<endl;
      return;
    }
  Int_t pad1,pad2,time1,time2,charge1,charge2,sector,row;
  Float_t r1,r2,phi1,phi2,phi_0,kappa,hit[3],hit2[3];
  for(Int_t i=AliL3Transform::GetFirstRow(fCurrentPatch); i<=AliL3Transform::GetLastRow(fCurrentPatch); i++)
    {
      Int_t rowindex1 = i - AliL3Transform::GetFirstRow(fCurrentPatch);
      for(Int_t d1=0; d1<fData[rowindex1].npads; d1++)
	{
	  pad1 = d1 + fData[rowindex1].minpad;
	  for(Int_t j=0; j<10; j++)
	    {
	      time1 = j + fData[rowindex1].mintime;
	      charge1 = fData[rowindex1].pads[d1][j];
	      if(charge1==0) continue;
	      AliL3Transform::Slice2Sector(0,i,sector,row);
	      AliL3Transform::Raw2Local(hit,sector,row,pad1,time1);
	      r1 = sqrt(hit[0]*hit[0]+hit[1]*hit[1]);
	      phi1 = atan2(hit[1],hit[0]);
	      
	      for(Int_t j=i+1; j<=AliL3Transform::GetLastRow(fCurrentPatch); j++)
		{
		  Int_t rowindex2 = j - AliL3Transform::GetFirstRow(fCurrentPatch);
		  for(Int_t d2=0; d2<fData[rowindex2].npads; d2++)
		    {
		      pad2 = d2 + fData[rowindex2].minpad;
		      for(Int_t k=0; k<10; k++)
			{
			  time2 = k + fData[rowindex2].mintime;
			  charge2 = fData[rowindex2].pads[d2][k];
			  if(charge2==0) continue;
			  AliL3Transform::Slice2Sector(0,j,sector,row);
			  AliL3Transform::Raw2Local(hit2,sector,row,pad2,time2);
			  r2 = sqrt(hit2[0]*hit2[0]+hit2[1]*hit2[1]);
			  phi2 = atan2(hit2[1],hit2[0]);
			  phi_0 = atan( (r2*sin(phi1) - r1*sin(phi2)) / (r2*cos(phi1) - r1*cos(phi2)) );
			  
			  kappa = 2*sin(phi1-phi_0) / r1;
			  hist->Fill(kappa,phi_0,charge1+charge2);


			}
		    }
		}
	    }
	}
    }
}

void AliL3HoughTest::Transform2Line(AliL3Histogram *hist,Int_t *rowrange)
{
  if(!fData)
    {
      cerr<<"AliL3HoughTest::Transform2Line : No data"<<endl;
      return;
    }
  
  Int_t pad,time,charge,sector,row;
  Float_t hit[3],theta,rho;
  for(Int_t i=rowrange[0]; i<=rowrange[1]; i++)
    {
      Int_t rowindex = i - AliL3Transform::GetFirstRow(fCurrentPatch);
      for(Int_t d=0; d<fData[rowindex].npads; d++)
	{
	  pad = d + fData[rowindex].minpad;
	  for(Int_t j=0; j<10; j++)
	    {
	      time = j + fData[rowindex].mintime;
	      charge = fData[rowindex].pads[d][j];
	      if(charge==0) continue;
	      AliL3Transform::Slice2Sector(0,i,sector,row);
	      AliL3Transform::Raw2Local(hit,sector,row,pad,time);
	      
	      hit[0] = hit[0] - AliL3Transform::Row2X(rowrange[0]);
	      
	      for(Int_t xbin=hist->GetFirstXbin(); xbin<hist->GetLastXbin(); xbin++)
		{
		  theta = hist->GetBinCenterX(xbin);
		  rho = hit[0]*cos(theta) + hit[1]*sin(theta);
		  hist->Fill(theta,rho,charge);
		}
	    }
	}
    }
}

void AliL3HoughTest::Transform2LineC(AliL3Histogram *hist,Int_t *rowrange)
{
  if(!fData)
    {
      cerr<<"AliL3HoughTest::Transform2Line : No data"<<endl;
      return;
    }
  
  Int_t pad1,pad2,time1,time2,charge1,charge2,sector,row;
  Float_t theta,rho,hit[3],hit2[3];
  for(Int_t i=rowrange[0]; i<=rowrange[1]; i++)
    {
      Int_t rowindex1 = i - AliL3Transform::GetFirstRow(fCurrentPatch);
      for(Int_t d1=0; d1<fData[rowindex1].npads; d1++)
	{
	  pad1 = d1 + fData[rowindex1].minpad;
	  for(Int_t j=0; j<10; j++)
	    {
	      time1 = j + fData[rowindex1].mintime;
	      charge1 = fData[rowindex1].pads[d1][j];
	      if(charge1==0) continue;
	      AliL3Transform::Slice2Sector(0,i,sector,row);
	      AliL3Transform::Raw2Local(hit,sector,row,pad1,time1);
	      
	      hit[0] = hit[0] - AliL3Transform::Row2X(rowrange[0]);
	      
	      for(Int_t j=i+1; j<=rowrange[1]; j++)
		{
		  Int_t rowindex2 = j - AliL3Transform::GetFirstRow(fCurrentPatch);
		  for(Int_t d2=0; d2<fData[rowindex2].npads; d2++)
		    {
		      pad2 = d2 + fData[rowindex2].minpad;
		      for(Int_t k=0; k<10; k++)
			{
			  time2 = k + fData[rowindex2].mintime;
			  charge2 = fData[rowindex2].pads[d2][k];
			  if(charge2==0) continue;
			  AliL3Transform::Slice2Sector(0,j,sector,row);
			  AliL3Transform::Raw2Local(hit2,sector,row,pad2,time2);
			  
			  hit2[0] = hit2[0] - AliL3Transform::Row2X(rowrange[0]);
			  
			  theta = atan2(hit2[0]-hit[0],hit[1]-hit2[1]);
			  rho = hit[0]*cos(theta)+hit[1]*sin(theta);
			  hist->Fill(theta,rho,charge1+charge2);
			}
		    }
		}
	    }
	}
    }
}

void AliL3HoughTest::FillImage(TH2 *hist,Int_t row)
{
  if(!fData)
    {
      cerr<<"AliL3HoughTest::FillImage : No data to fill"<<endl;
      return;
    }
  
  for(Int_t i=AliL3Transform::GetFirstRow(fCurrentPatch); i<=AliL3Transform::GetLastRow(fCurrentPatch); i++)
    {
      Int_t rowindex = i - AliL3Transform::GetFirstRow(fCurrentPatch);
      if(row >=0)
	if(i != row) continue;

      //cout<<"row "<<i<<" npads "<<fData[rowindex].npads<<endl;
      for(Int_t j=0; j<fData[rowindex].npads; j++)
	{
	  Int_t pad = j + fData[rowindex].minpad;
	  for(Int_t k=0; k<10; k++)
	    {
	      Int_t time = k + fData[rowindex].mintime;
	      Int_t charge = fData[rowindex].pads[j][k];
	      if(charge==0) continue;
	      //cout<<i<<" "<<pad<<" "<<time<<" "<<charge<<endl;
	      Float_t xyz[3];
	      Int_t sector,row;
	      AliL3Transform::Slice2Sector(0,i,sector,row);
	      AliL3Transform::Raw2Local(xyz,sector,row,pad,time);
	      if(row>=0)
		hist->Fill(pad,time,charge);
	      else
		hist->Fill(xyz[0],xyz[1],charge);
	    }
	}
      if(row>=0)
	break;
    }
}

void AliL3HoughTest::Transform2Line3D(TH3 *hist,Int_t *rowrange)
{
  if(!fData)
    {
      cerr<<"AliL3HoughTest::Transform2Line : No data"<<endl;
      return;
    }
  
  Int_t pad,time,charge,sector,row;
  Float_t hit[3],theta,rho,R,delta;
  for(Int_t i=rowrange[0]; i<=rowrange[1]; i++)
    {
      Int_t rowindex = i - AliL3Transform::GetFirstRow(fCurrentPatch);
      for(Int_t d=0; d<fData[rowindex].npads; d++)
	{
	  pad = d + fData[rowindex].minpad;
	  for(Int_t j=0; j<10; j++)
	    {
	      time = j + fData[rowindex].mintime;
	      charge = fData[rowindex].pads[d][j];
	      if(charge==0) continue;
	      AliL3Transform::Slice2Sector(0,i,sector,row);
	      AliL3Transform::Raw2Local(hit,sector,row,pad,time);
	      
	      hit[0] = hit[0] - AliL3Transform::Row2X(rowrange[0]);
	      
	      for(Int_t xbin=hist->GetXaxis()->GetFirst(); xbin<=hist->GetXaxis()->GetLast(); xbin++)
		{
		  theta = hist->GetXaxis()->GetBinCenter(xbin);
		  rho = hit[0]*cos(theta) + hit[1]*sin(theta);
		  Float_t x = hit[0] + AliL3Transform::Row2X(rowrange[0]);
		  R = sqrt(x*x + hit[1]*hit[1]);
		  delta = atan(hit[2]/R);
		  hist->Fill(theta,rho,delta,charge);
		}
	    }
	}
    }
}

void AliL3HoughTest::Transform2LineC3D(TH3 *hist,Int_t *rowrange)
{
  if(!fData)
    {
      cerr<<"AliL3HoughTest::Transform2Line : No data"<<endl;
      return;
    }
  
  Int_t pad1,pad2,time1,time2,charge1,charge2,sector,row;
  Float_t theta,rho,hit[3],hit2[3],R1,R2,delta,delta1,delta2;
  for(Int_t i=rowrange[0]; i<=rowrange[1]; i++)
    {
      Int_t rowindex1 = i - AliL3Transform::GetFirstRow(fCurrentPatch);
      for(Int_t d1=0; d1<fData[rowindex1].npads; d1++)
	{
	  pad1 = d1 + fData[rowindex1].minpad;
	  for(Int_t j=0; j<10; j++)
	    {
	      time1 = j + fData[rowindex1].mintime;
	      charge1 = fData[rowindex1].pads[d1][j];
	      if(charge1==0) continue;
	      AliL3Transform::Slice2Sector(0,i,sector,row);
	      AliL3Transform::Raw2Local(hit,sector,row,pad1,time1);
	      R1 = sqrt(hit[0]*hit[0]+hit[1]*hit[1]);
	      delta1 = atan(hit[2]/R1);
	      hit[0] = hit[0] - AliL3Transform::Row2X(rowrange[0]);
	      
	      for(Int_t j=i+1; j<=rowrange[1]; j++)
		{
		  Int_t rowindex2 = j - AliL3Transform::GetFirstRow(fCurrentPatch);
		  for(Int_t d2=0; d2<fData[rowindex2].npads; d2++)
		    {
		      pad2 = d2 + fData[rowindex2].minpad;
		      for(Int_t k=0; k<10; k++)
			{
			  time2 = k + fData[rowindex2].mintime;
			  charge2 = fData[rowindex2].pads[d2][k];
			  if(charge2==0) continue;
			  AliL3Transform::Slice2Sector(0,j,sector,row);
			  AliL3Transform::Raw2Local(hit2,sector,row,pad2,time2);
			  R2 = sqrt(hit2[0]*hit2[0]+hit2[1]*hit2[1]);
			  delta2 = atan(hit2[2]/R2);
			  delta = (charge1*delta1 + charge2*delta2)/(charge1+charge2);
			  hit2[0] = hit2[0] - AliL3Transform::Row2X(rowrange[0]);
			  
			  theta = atan2(hit2[0]-hit[0],hit[1]-hit2[1]);
			  rho = hit[0]*cos(theta)+hit[1]*sin(theta);
			  hist->Fill(theta,rho,delta,charge1+charge2);
			}
		    }
		}
	    }
	}
    }  
}
