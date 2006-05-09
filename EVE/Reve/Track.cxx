// $Header$

#include "Track.h"
#include "MCHelixLine.hi"

#include <TPolyLine3D.h>
#include <TPolyMarker3D.h>
#include <TColor.h>

// Updates
#include <Reve/RGTopFrame.h>
#include <TCanvas.h>

#include <vector>

using namespace Reve;

//______________________________________________________________________
// Track
//

ClassImp(Reve::Track)

Track::Track()
{
  fRnrStyle = 0;
}

Track::Track(Reve::MCTrack* t, TrackRnrStyle* rs)
{
  fRnrStyle = rs;

  fName = t->GetName();
  fLineColor = fRnrStyle->GetColor();
  fMainColorPtr = &fLineColor;

  fV.Set(t->Vx(), t->Vy(), t->Vz());
  fP.Set(t->Px(), t->Py(), t->Pz());
  fBeta   = t->P()/t->Energy();

  TParticlePDG* pdgp = t->GetPDG();
  if(pdgp == 0) {
    t->ResetPdgCode(); pdgp = t->GetPDG();
  }

  fCharge = (Int_t) pdgp->Charge();
  fLabel  = t->label;
}

Track::Track(Reve::RecTrack* t, TrackRnrStyle* rs)
{
  fRnrStyle = rs;
  fName = t->GetName();
  fLineColor = fRnrStyle->GetColor();
  fMainColorPtr = &fLineColor;

  fV = t->V;
  fP = t->P;
  fBeta   = t->beta;
  fCharge = t->sign;
  fLabel  = t->label; 
}

Track::~Track()
{}

void Track::Reset(Int_t n_points)
{
  delete [] TPolyLine3D::fP; TPolyLine3D::fP = 0;
  fN = n_points;
  if(fN) TPolyLine3D::fP = new Float_t [3*fN];
  memset(TPolyLine3D::fP, 0, 3*fN*sizeof(Float_t));
  fLastPoint = -1;
}

/**************************************************************************/

void Track::MakeTrack()
{
  
  TrackRnrStyle& RS((fRnrStyle != 0) ? *fRnrStyle : TrackRnrStyle::fgDefStyle);

  Float_t px = fP.x, py = fP.y, pz = fP.z;  

  MCVertex  mc_v0;
  mc_v0.x = fV.x;
  mc_v0.y = fV.y; 
  mc_v0.z = fV.z; 
  mc_v0.t = 0;

  std::vector<MCVertex> track_points;
  Bool_t decay = false;

  if ((TMath::Abs(fV.z) > RS.fMaxZ) || (fV.x*fV.x + fV.y*fV.y > RS.fMaxR*RS.fMaxR)) 
    goto make_polyline;
  
  if (fCharge) { // Charged particle

    Float_t a = 0.2998*RS.fMagField*fCharge/300; // m->cm
   
    MCHelix helix(fRnrStyle, &mc_v0, TMath::C()*fBeta, &track_points, a); //m->cm
    helix.Init(TMath::Sqrt(px*px+py*py), pz);
   
    if(!fPathMarks.empty()){
      for(std::vector<Reve::PathMark*>::iterator i=fPathMarks.begin(); i!=fPathMarks.end(); ++i) {
	Reve::PathMark* pm = *i;
        
	if(RS.fFitDaughters &&  pm->type == Reve::PathMark::Daughter){
	  if(TMath::Abs(pm->V.z) > RS.fMaxZ 
	     || TMath::Sqrt(pm->V.x*pm->V.x + pm->V.y*pm->V.y) > RS.fMaxR )
	    goto helix_bounds;

          //printf("%s fit daughter  \n", fName.Data()); 
	  helix.LoopToVertex(fP.x, fP.y, fP.z, pm->V.x, pm->V.y, pm->V.z);
	  fP.x -=  pm->P.x;
	  fP.y -=  pm->P.y;
	  fP.z -=  pm->P.z;
	}
	if(RS.fFitDecay &&  pm->type == Reve::PathMark::Decay){
	  
	  if(TMath::Abs(pm->V.z) > RS.fMaxZ 
	     || TMath::Sqrt(pm->V.x*pm->V.x + pm->V.y*pm->V.y) > RS.fMaxR )
	    goto helix_bounds;
	  helix.LoopToVertex(fP.x, fP.y, fP.z, pm->V.x, pm->V.y, pm->V.z);
          decay = true;
          break;
	}
      }
    }
  helix_bounds:
    //go to bounds
    if(!decay || RS.fFitDecay == false){
      helix.LoopToBounds(px,py,pz);
      // printf("%s loop to bounds  \n",fName.Data() );
    }

  } else { // Neutral particle

    MCLine line(fRnrStyle, &mc_v0, TMath::C()*fBeta, &track_points);
   
    if(!fPathMarks.empty()){
      for(std::vector<Reve::PathMark*>::iterator i=fPathMarks.begin(); i!=fPathMarks.end(); ++i) {
	Reve::PathMark* pm = *i;

	if(RS.fFitDaughters &&  pm->type == Reve::PathMark::Daughter){
          if(TMath::Abs(pm->V.z) > RS.fMaxZ 
	     || TMath::Sqrt(pm->V.x*pm->V.x + pm->V.y*pm->V.y) > RS.fMaxR )
	    goto line_bounds;
	  line.GotoVertex(pm->V.x, pm->V.y, pm->V.z);
	  fP.x -=  pm->P.x;
	  fP.y -=  pm->P.y;
	  fP.z -=  pm->P.z;
	}

	if(RS.fFitDecay &&  pm->type == Reve::PathMark::Decay){
	  if(TMath::Abs(pm->V.z) > RS.fMaxZ 
	     || TMath::Sqrt(pm->V.x*pm->V.x + pm->V.y*pm->V.y) > RS.fMaxR )
	    goto line_bounds;
	  line.GotoVertex(pm->V.x, pm->V.y, pm->V.z);
          decay = true;
	  break;
	}
      }
    }

  line_bounds:
    if(!decay || RS.fFitDecay == false)
      line.GotoBounds(px,py,pz);

  }
make_polyline:
  Reset(track_points.size());
  for(std::vector<MCVertex>::iterator i=track_points.begin(); i!=track_points.end(); ++i)
    SetNextPoint(i->x, i->y, i->z);
}

/**************************************************************************/

//______________________________________________________________________
// TrackRnrStyle
//

ClassImp(Reve::TrackRnrStyle)

Float_t       TrackRnrStyle::fgDefMagField = 0.5;
TrackRnrStyle TrackRnrStyle::fgDefStyle;

void TrackRnrStyle::Init()
{
  fMagField = fgDefMagField;

  fMaxR  = 450;
  fMaxZ  = 550;

  fMaxOrbs = 2;
  fMinAng  = 45;

  fFitDaughters = true;
  fFitDecay     = true;

  fDelta  = 0.1; //calculate step size depending on helix radius
}

/**************************************************************************/
/**************************************************************************/

//______________________________________________________________________
// TrackList
//

ClassImp(Reve::TrackList)

void TrackList::Init()
{
  fMarkerStyle = 6;
  fMarkerColor = 5;
  // fMarker->SetMarkerSize(0.05);

  fRnrMarkers = true;
  fRnrTracks  = true;

  mRnrStyle = new TrackRnrStyle;
  SetMainColorPtr(&mRnrStyle->fColor);
}

TrackList::TrackList(Int_t n_tracks) :
  TPolyMarker3D(n_tracks),
  RenderElementListBase()
{
  Init();
}

TrackList::TrackList(const Text_t* name, Int_t n_tracks) :
  TPolyMarker3D(n_tracks),
  RenderElementListBase()
{
  Init();
  SetName(name);
}

void TrackList::Reset(Int_t n_tracks)
{
  delete [] fP; fP = 0;
  fN = n_tracks;
  if(fN) fP = new Float_t [3*fN];
  memset(fP, 0, 3*fN*sizeof(Float_t));
  fLastPoint = -1;
}

/**************************************************************************/

void TrackList::Paint(Option_t* option)
{
  if(fRnrElement) {
    if(fRnrMarkers) {
      TPolyMarker3D::Paint(option);
    }
    if(fRnrTracks) {
      for(lpRE_i i=fList.begin(); i!=fList.end(); ++i) {
	if((*i)->GetRnrElement())
	  (*i)->GetObject()->Paint(option);
      }
    }
  }
}

/**************************************************************************/

void TrackList::AddElement(RenderElement* el)
{
  static const Exc_t eH("TrackList::AddElement ");
  if (dynamic_cast<Track*>(el)  == 0)
    throw(eH + "new element not a Track.");
  RenderElementListBase::AddElement(el);
}

/**************************************************************************/

void TrackList::SetRnrMarkers(Bool_t rnr)
{
  fRnrMarkers = rnr;
  gReve->Redraw3D();
}

void TrackList::SetRnrTracks(Bool_t rnr)
{

  fRnrTracks = rnr;
  gReve->Redraw3D();
}

/**************************************************************************/

void TrackList::MakeTracks()
{
  for(lpRE_i i=fList.begin(); i!=fList.end(); ++i) {
    ((Track*)(*i))->MakeTrack();
  }
  gReve->Redraw3D();
}


void TrackList::MakeMarkers()
{
  Reset(fList.size());
  for(lpRE_i i=fList.begin(); i!=fList.end(); ++i) {
    Track& t = *((Track*)(*i));
    if(t.GetN() > 0)
      SetNextPoint(t.fV.x, t.fV.y, t.fV.z);
  }
  gReve->Redraw3D();
}

/**************************************************************************/
/*************************************************************************/

void TrackList::SetMaxR(Float_t x)
{
  mRnrStyle->fMaxR = x;
  MakeTracks();
  MakeMarkers();
}

void TrackList::SetMaxZ(Float_t x)
{
  mRnrStyle->fMaxZ = x;
  MakeTracks();
  MakeMarkers();
}

void TrackList::SetMaxOrbs(Float_t x)
{
  mRnrStyle->fMaxOrbs = x;
  MakeTracks();
}

void TrackList::SetMinAng(Float_t x)
{
  mRnrStyle->fMinAng = x;
  MakeTracks();
}

void TrackList::SetDelta(Float_t x)
{
  mRnrStyle->fDelta = x;
  MakeTracks();
}

void TrackList::SetFitDaughters(Bool_t x)
{
  mRnrStyle->fFitDaughters = x;
  MakeTracks();
}

void TrackList::SetFitDecay(Bool_t x)
{
  mRnrStyle->fFitDecay = x;
  MakeTracks();
}

/**************************************************************************/
/**************************************************************************/

void TrackList::SelectByPt(Float_t min_pt, Float_t max_pt)
{
  Float_t minptsq = min_pt*min_pt;
  Float_t maxptsq = max_pt*max_pt;
  Float_t ptsq;

  for(lpRE_i i=fList.begin(); i!=fList.end(); ++i) {
    ptsq = ((Track*)(*i))->fP.Perp2();
    (*i)->SetRnrElement(ptsq >= minptsq && ptsq <= maxptsq);
  }
}

/**************************************************************************/

#include <TGFrame.h>
#include <TGDoubleSlider.h>
#include <TGXYLayout.h>

void TrackList::MakePtScrollbar()
{
  TGMainFrame* mf = new TGMainFrame(gClient->GetRoot(), 320, 60);

  TGDoubleHSlider* hs = new TGDoubleHSlider(mf);
  hs->SetRange(0.2, 10);
  hs->SetPosition(0.2, 10);
  hs->Resize(300, 25);
  mf->AddFrame(hs, new TGLayoutHints(kLHintsCenterX, 10, 10, 10, 10));

  hs->Connect("PositionChanged()", "Reve::TrackList",
	      this, "HandlePtScrollEvent()");

  mf->SetWindowName("Pt Selector");
  mf->MapSubwindows();
  mf->Resize(mf->GetDefaultSize()); // this is used here to init layout algorithm
  mf->MapWindow();
}

void TrackList::HandlePtScrollEvent()
{
  TGDoubleHSlider* hs = (TGDoubleHSlider*)gTQSender;

  Float_t min = hs->GetMinPosition(), max = hs->GetMaxPosition();
  printf("hslidor min=%f max=%f\n", min, max);
  SelectByPt(min, max);
}
