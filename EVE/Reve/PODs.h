// $Header$

#ifndef REVE_PODs_H
#define REVE_PODs_H

#include <TObject.h>
#include <TMath.h>

#include <TParticle.h>

namespace Reve {

/**************************************************************************/
// PODs.h
/**************************************************************************/

// Basic structures for Reve. Design criteria:
//
//  * provide basic cross-referencing functionality;
//
//  * small memory/disk footprint (floats / count on compression in
//  split mode);
//
//  * simple usage from tree selections;
//
//  * placement in TClonesArray (composites are TObject derived);
//
//  * minimal member-naming (impossible to make everybody happy).

void DisablePODTObjectStreamers();

/**************************************************************************/
// Vector
/**************************************************************************/

// Minimal Float_t copy of TVector3.
// Used to represent points and momenta.

class Vector
{
public:
  Float_t x, y, z;

  Vector() : x(0), y(0), z(0) {}
  Vector(Float_t _x, Float_t _y, Float_t _z) : x(_x), y(_y), z(_z) {}

  Float_t* c_vec() { return &x; }
  void Set(Float_t*  v) { x=v[0]; y=v[1]; z=v[2]; }
  void Set(Double_t* v) { x=v[0]; y=v[1]; z=v[2]; }
  void Set(Float_t  _x, Float_t  _y, Float_t  _z) { x=_x; y=_y; z=_z; }
  void Set(Double_t _x, Double_t _y, Double_t _z) { x=_x; y=_y; z=_z; }
  void Set(const TVector3& v) { x=v.x(); y=v.y(); z=v.z(); }

  Float_t Phi()      const;
  Float_t Theta()    const;
  Float_t CosTheta() const;
  Float_t Eta()      const;

  Float_t Mag()  const { return TMath::Sqrt(x*x+y*y+z*z);}
  Float_t Mag2() const { return x*x+y*y+z*z;}

  Float_t Perp()  const { return TMath::Sqrt(x*x+y*y);}
  Float_t Perp2() const { return x*x+y*y;}
  Float_t R()     const { return Perp(); }

  // need operator +,-,Dot

  ClassDef(Vector, 1);
};

inline Float_t Vector::Phi() const
{ return x == 0.0 && y == 0.0 ? 0.0 : TMath::ATan2(y,x); }

inline Float_t Vector::Theta() const
{ return x == 0.0 && y == 0.0 && z == 0.0 ? 0.0 : TMath::ATan2(Perp(),z); }

inline Float_t Vector::CosTheta() const
{ Float_t ptot = Mag(); return ptot == 0.0 ? 1.0 : z/ptot; }

/**************************************************************************/
// PathMark
/**************************************************************************/

class PathMark
{
 public:
  enum Type_e { Reference, Daughter, Decay };

  Vector V, P;
  Type_e type;

  PathMark(Type_e t=Reference) : type(t) {}

  ClassDef(PathMark, 1);
};

/**************************************************************************/
// MCTrack
/**************************************************************************/

class MCTrack : public TParticle // ?? Copy stuff over ??
{
public:
  Int_t   label;       // Label of the track
  Int_t   eva_label;   // Label of primary particle

  Bool_t  decayed;     // True if decayed during tracking.
  // ?? Perhaps end-of-tracking point/momentum would be better.
  Float_t t_decay;     // Decay time
  Vector  V_decay;     // Decay vertex
  Vector  P_decay;     // Decay momentum

  MCTrack() { decayed = false; }

  MCTrack& operator=(const TParticle& p)
  { *((TParticle*)this) = p; return *this; }

  void ResetPdgCode() { fPdgCode = 0; }

  ClassDef(MCTrack, 1);
};


/**************************************************************************/
// MCTrackRef
/**************************************************************************/

// Not used.

class MCTrackRef : public TObject
{
public:
  Int_t   label;
  Int_t   status;
  Vector  V;
  Vector  P;
  Float_t length;
  Float_t time;

  MCTrackRef() {}

  ClassDef(MCTrackRef, 1)
};


/**************************************************************************/
// Hit
/**************************************************************************/

// Representation of a hit.

// Members det_id (and subdet_id) serve for cross-referencing into
// geometry. Hits should be stored in det_id (+some label ordering) in
// order to maximize branch compression.


class Hit : public TObject
{
public:
  UShort_t det_id;    // Custom detector id
  UShort_t subdet_id; // Custom sub-detector id
  Int_t    label;     // Label of particle that produced the hit
  Int_t    eva_label;
  Vector   V;         // Vertex

  // ?? Float_t charge. Probably specific.

  Hit() {}

  ClassDef(Hit, 1);
};


/**************************************************************************/
// Cluster
/**************************************************************************/

// Base class for reconstructed clusters

// ?? Should Hit and cluster have common base? No.

class Cluster : public TObject
{
public:
  UShort_t det_id;    // Custom detector id
  UShort_t subdet_id; // Custom sub-detector id
  Int_t    label[3];  // Labels of particles that contributed hits
  // ?? Should include reconstructed track using it? Rather not, separate.

  Vector   V;         // Vertex
  // Vector   W;         // Cluster widths
  // ?? Coord system? Special variables Wz, Wy?

  Cluster() {}

  ClassDef(Cluster, 1);
};


/**************************************************************************/
// RecTrack
/**************************************************************************/

class RecTrack : public TObject
{
public:
  Int_t   label;       // Label of the track
  Int_t   status;      // Status as exported from reconstruction
  Int_t   sign;
  Vector  V;           // Start vertex from reconstruction
  Vector  P;           // Reconstructed momentum at start vertex
  Float_t beta;

  // PID data missing

  RecTrack() {}

  Float_t Pt() { return P.Perp(); }

  ClassDef(RecTrack, 1);
};

// Another class with specified points/clusters


/**************************************************************************/
// RecKink
/**************************************************************************/

class RecKink : public RecTrack
{
public:
  Int_t   label_sec;  // Label of the secondary track
  Vector  V_end;      // End vertex: last point on the primary track
  Vector  V_kink;     // Kink vertex: reconstructed position of the kink
  Vector  P_sec;      // Momentum of secondary track

  ClassDef(RecKink, 1);
};


/**************************************************************************/
// RecV0
/**************************************************************************/

class RecV0 : public TObject
{
public:
  Int_t  status;

  Vector V_neg;       // Vertex of negative track
  Vector P_neg;       // Momentum of negative track
  Vector V_pos;       // Vertex of positive track
  Vector P_pos;       // Momentum of positive track

  Vector V_ca;        // Point of closest approach
  Vector V0_birth;    // Reconstucted birth point of neutral particle

  // ? Data from simulation.
  Int_t label;        // Neutral mother label read from kinematics
  Int_t pdg;          // PDG code of mother
  Int_t d_label[2];   // Daughter labels ?? Rec labels present anyway.

  ClassDef(RecV0, 1);
};

/**************************************************************************/
/**************************************************************************/

// Missing primary vertex.

// Missing GenInfo, RecInfo.

class GenInfo : public TObject
{
public:
  Bool_t       is_rec;   // is reconstructed
  Bool_t       has_V0;
  Bool_t       has_kink;
  Int_t        label;
  Int_t        n_hits;
  Int_t        n_clus;

  GenInfo() { is_rec = has_V0 = has_kink = false; }

  ClassDef(GenInfo, 1);
};

/**************************************************************************/
/**************************************************************************/

// This whole construction is highly embarrassing. It requires
// shameless copying of experiment data. What is good about this
// scheme:
//
// 1) Filters can be applied at copy time so that only part of the
// data is copied over.
//
// 2) Once the data is extracted it can be used without experiment
// software. Thus, external service can provide this data and local
// client can be really thin.
//
// 3) Some pretty advanced visualization schemes/selections can be
// implemented in a general framework by providing data extractors
// only. This is also good for PR or VIP displays.
//
// 4) These classes can be extended by particular implementations. The
// container classes will use TClonesArray with user-specified element
// class.

// The common behaviour could be implemented entirely without usage of
// a common base classes, by just specifying names of members that
// retrieve specific data. This is fine as long as one only uses tree
// selections but becomes painful for extraction of data into local
// structures (could a) use interpreter but this is an overkill and
// would cause serious trouble for multi-threaded environment; b) use
// member offsets and data-types from the dictionary).

}

#endif
