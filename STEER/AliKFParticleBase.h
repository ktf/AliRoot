//---------------------------------------------------------------------------------
// The AliKFParticleBase class
// .
// @author  S.Gorbunov, I.Kisel
// @version 1.0
// @since   13.05.07
// 
// Class to reconstruct and store the decayed particle parameters.
// The method is described in CBM-SOFT note 2007-003, 
// ``Reconstruction of decayed particles based on the Kalman filter'', 
// http://www.gsi.de/documents/DOC-2007-May-14-1.pdf
//
// This class describes general mathematics which is used by AliKFParticle class
// 
//  -= Copyright &copy ALICE HLT Group =-
//_________________________________________________________________________________



#ifndef ALIKFPARTICLEBASE_H
#define ALIKFPARTICLEBASE_H

#include "TObject.h"

class AliKFParticleBase :public TObject {
  
 public:

  //*
  //* ABSTRACT METHODS HAVE TO BE DEFINED IN USER CLASS 
  //* 

  //* Virtual method to access the magnetic field

  virtual void GetFieldValue(const Double_t xyz[], Double_t B[]) const = 0;
  
  //* Virtual methods needed for particle transportation 
  //* One can use particular implementations for collider (only Bz component) 
  //* geometry and for fixed-target (CBM-like) geometry which are provided below 
  //* in TRANSPORT section
 
  //* Get dS to xyz[] space point 

  virtual Double_t GetDStoPoint( const Double_t xyz[] ) const = 0;

  //* Get dS to other particle p (dSp for particle p also returned) 

  virtual void GetDStoParticle( const AliKFParticleBase &p, 
				Double_t &DS, Double_t &DSp ) const = 0;
  
  //* Transport on dS value along trajectory, output to P,C

  virtual void Transport( Double_t dS, Double_t P[], Double_t C[] ) const = 0;



  //*
  //*  INITIALIZATION
  //*

  //* Constructor 

  AliKFParticleBase();

  //* Destructor 

  virtual ~AliKFParticleBase() { ; }

 //* Initialisation from "cartesian" coordinates ( X Y Z Px Py Pz )
 //* Parameters, covariance matrix, charge, and mass hypothesis should be provided 

  void Initialize( const Double_t Param[], const Double_t Cov[], Int_t Charge, Double_t Mass );

  //* Initialise covariance matrix and set current parameters to 0.0 

  void Initialize();

  //* Set decay vertex parameters for linearisation 

  void SetVtxGuess( Double_t x, Double_t y, Double_t z );

  //*
  //*  ACCESSORS
  //*

  //* Simple accessors 

  Double_t GetX    () const { return fP[0]; }
  Double_t GetY    () const { return fP[1]; }
  Double_t GetZ    () const { return fP[2]; }
  Double_t GetPx   () const { return fP[3]; }
  Double_t GetPy   () const { return fP[4]; }
  Double_t GetPz   () const { return fP[5]; }
  Double_t GetE    () const { return fP[6]; }
  Double_t GetS    () const { return fP[7]; }
  Int_t    GetQ    () const { return fQ;    }
  Double_t GetChi2 () const { return fChi2; }
  Int_t    GetNDF  () const { return fNDF;  }
  
  Double_t GetParameter ( Int_t i )        const { return fP[i];       }
  Double_t GetCovariance( Int_t i )        const { return fC[i];       }
  Double_t GetCovariance( Int_t i, Int_t j ) const { return fC[IJ(i,j)]; }

  //* Accessors with calculations( &value, &estimated sigma )
  //* error flag returned (0 means no error during calculations) 

  Int_t GetMomentum    ( Double_t &pmom, Double_t &sigmap ) const ;
  Int_t GetMass        ( Double_t &mass, Double_t &sigmam ) const ;
  Int_t GetDecayLength ( Double_t &dlen, Double_t &sigmal ) const ;
  Int_t GetLifeTime    ( Double_t &tauc, Double_t &sigmat ) const ;

  //*
  //*  MODIFIERS
  //*
  
  Double_t & X    () { return fP[0]; }
  Double_t & Y    () { return fP[1]; }
  Double_t & Z    () { return fP[2]; }
  Double_t & Px   () { return fP[3]; }
  Double_t & Py   () { return fP[4]; }
  Double_t & Pz   () { return fP[5]; }
  Double_t & E    () { return fP[6]; }
  Double_t & S    () { return fP[7]; }
  Int_t    & Q    () { return fQ;    }
  Double_t & Chi2 () { return fChi2; }
  Int_t    & NDF  () { return fNDF;  }

  Double_t & Parameter ( Int_t i )        { return fP[i];       }
  Double_t & Covariance( Int_t i )        { return fC[i];       }
  Double_t & Covariance( Int_t i, Int_t j ) { return fC[IJ(i,j)]; }


  //* 
  //* CONSTRUCTION OF THE PARTICLE BY ITS DAUGHTERS AND MOTHER
  //* USING THE KALMAN FILTER METHOD
  //*


  //* Simple way to add daughter ex. D0+= Pion; 

  void operator +=( const AliKFParticleBase &Daughter );  

  //* Add daughter track to the particle 

  void AddDaughter( const AliKFParticleBase &Daughter );

  //* Set production vertex 

  void SetProductionVertex( const AliKFParticleBase &Vtx );

  //* Set mass constraint 

  void SetMassConstraint( Double_t Mass, Double_t SigmaMass = 0 );
  
  //* Set no decay length for resonances

  void SetNoDecayLength();


  //* Everything in one go  

  void Construct( const AliKFParticleBase *vDaughters[], Int_t NDaughters, 
		  const AliKFParticleBase *ProdVtx=0,   Double_t Mass=-1  );


  //*
  //*                   TRANSPORT
  //* 
  //*  ( main transportation parameter is S = SignedPath/Momentum )
  //*  ( parameters of decay & production vertices are stored locally )
  //*


  //* Transport the particle to its decay vertex 

  void TransportToDecayVertex();

  //* Transport the particle to its production vertex 

  void TransportToProductionVertex();

  //* Transport the particle on dS parameter (SignedPath/Momentum) 

  void TransportToDS( Double_t dS );

  //* Particular extrapolators one can use 

  Double_t GetDStoPointBz( Double_t Bz, const Double_t xyz[] ) const;
  
  void GetDStoParticleBz( Double_t Bz, const AliKFParticleBase &p, 
			  Double_t &dS, Double_t &dS1       ) const ;
 
  // Double_t GetDStoPointCBM( const Double_t xyz[] ) const;
 
   void TransportBz( Double_t Bz, Double_t dS, Double_t P[], Double_t C[] ) const;
   void TransportCBM( Double_t dS, Double_t P[], Double_t C[] ) const;  


  //* 
  //* OTHER UTILITIES
  //*

  //* Calculate distance from another object [cm]

  Double_t GetDistanceFromVertex( const Double_t vtx[] ) const;
  Double_t GetDistanceFromVertex( const AliKFParticleBase &Vtx ) const;
  Double_t GetDistanceFromParticle( const AliKFParticleBase &p ) const;

  //* Calculate sqrt(Chi2/ndf) deviation from vertex
  //* v = [xyz], Cv=[Cxx,Cxy,Cyy,Cxz,Cyz,Czz]-covariance matrix

  Double_t GetDeviationFromVertex( const Double_t v[], 
				   const Double_t Cv[]=0 ) const;
  Double_t GetDeviationFromVertex( const AliKFParticleBase &Vtx ) const;
  Double_t GetDeviationFromParticle( const AliKFParticleBase &p ) const;  

  //* Subtract the particle from the vertex  

  void SubtractFromVertex( Double_t v[], Double_t Cv[], 
			   Double_t &vChi2, Int_t vNDF ) const ;
  
 protected:

  static Int_t IJ( Int_t i, Int_t j ){ 
    return ( j<=i ) ? i*(i+1)/2+j :j*(j+1)/2+i;
  }

  Double_t & Cij( Int_t i, Int_t j ){ return fC[IJ(i,j)]; }

  void Convert( bool ToProduction );
  void TransportLine( Double_t S, Double_t P[], Double_t C[] ) const ;
  Double_t GetDStoPointLine( const Double_t xyz[] ) const;

  static void MultQSQt( const Double_t Q[], const Double_t S[], 
			Double_t SOut[] );

  void GetMeasurement( const Double_t XYZ[], Double_t m[], Double_t V[] ) const ;

  Double_t fP[8];  //* Main particle parameters {X,Y,Z,Px,Py,Pz,E,S[=DecayLength/P]}
  Double_t fC[36]; //* Low-triangle covariance matrix of fP
  Int_t    fQ;     //* Particle charge 
  Int_t    fNDF;   //* Number of degrees of freedom 
  Double_t fChi2;  //* Chi^2

  Double_t fSFromDecay; //* Distance from decay vertex to current position

  Bool_t fAtProductionVertex; //* Flag shows that the particle error along
                              //* its trajectory is taken from production vertex    

  Double_t fVtxGuess[3];  //* Guess for the position of the decay vertex 
                          //* ( used for linearisation of equations )

  Bool_t fIsLinearized;   //* Flag shows that the guess is present

  ClassDef( AliKFParticleBase, 1 );
};

#endif 
