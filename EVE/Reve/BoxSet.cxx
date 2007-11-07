// $Header$

#include "BoxSet.h"
#include <TRandom.h>
#include <TBuffer3D.h>
#include <TBuffer3DTypes.h>
#include <TVirtualPad.h>
#include <TVirtualViewer3D.h>

using namespace Reve;


//______________________________________________________________________________
// BoxSet
//
// A collection of 3D-boxes. The way how the boxes are defined depends
// on the fBoxType data-member.
//   BT_FreeBox         arbitrary box: specify 8*(x,y,z) box corners
//   BT_AABox           axis-aligned box: specify (x,y,z) and (w, h, d)
//   BT_AABoxFixedDim   axis-aligned box w/ fixed dimensions: specify (x,y,z)
//                      also set fDefWidth, fDefHeight and fDefDepth
//
// Each box can be assigned:
// a) Color or signal value. Thresholds and signal-to-color mapping
//    can then be set dynamically via the RGBAPalette class.
// b) External TObject* (stored as TRef).
//
// See also base-class DigitSet for more information.

ClassImp(BoxSet)

//______________________________________________________________________________
BoxSet::BoxSet(const Text_t* n, const Text_t* t) :
  DigitSet      (n, t),

  fBoxType      (BT_Undef),
  fDefWidth     (1),
  fDefHeight    (1),
  fDefDepth     (1)
{
  // Constructor.

  // Override from DigitSet.
  fDisableLigting = kFALSE;
}

/**************************************************************************/

//______________________________________________________________________________
Int_t BoxSet::SizeofAtom(BoxSet::BoxType_e bt)
{
  // Return size of data-structure describing a box of type bt.

  static const Exc_t eH("BoxSet::SizeofAtom ");

  switch (bt) {
    case BT_Undef:                return 0;
    case BT_FreeBox:              return sizeof(BFreeBox);
    case BT_AABox:                return sizeof(BAABox);
    case BT_AABoxFixedDim:        return sizeof(BAABoxFixedDim);
    default:                      throw(eH + "unexpected atom type.");
  }
  return 0;
}

/**************************************************************************/

//______________________________________________________________________________
void BoxSet::Reset(BoxSet::BoxType_e boxType, Bool_t valIsCol, Int_t chunkSize)
{
  // Reset the data containers to zero size.
  // The arguments describe the basic parameters of data storage.

  fBoxType      = boxType;
  fValueIsColor = valIsCol;
  fDefaultValue = valIsCol ? 0 : kMinInt;
  if (fOwnIds)
    ReleaseIds();
  fPlex.Reset(SizeofAtom(fBoxType), chunkSize);
}

//______________________________________________________________________________
void BoxSet::Reset()
{
  // Reset the data containers to zero size.
  // Keep the old data-storage parameters.

  if (fOwnIds)
    ReleaseIds();
  fPlex.Reset(SizeofAtom(fBoxType), TMath::Max(fPlex.N(), 64));
}

/**************************************************************************/

//______________________________________________________________________________
void BoxSet::AddBox(const Float_t* verts)
{
  // Create a new box from a set of 8 vertices.
  // To be used for box-type BT_FreeBox.

  static const Exc_t eH("BoxSet::AddBox ");

  if (fBoxType != BT_FreeBox)
    throw(eH + "expect free box-type.");

  BFreeBox* b = (BFreeBox*) NewDigit();
  memcpy(b->fVertices, verts, sizeof(b->fVertices));
}

//______________________________________________________________________________
void BoxSet::AddBox(Float_t a, Float_t b, Float_t c, Float_t w, Float_t h, Float_t d)
{
  // Create a new axis-aligned box from at a given position and with
  // specified dimensions.
  // To be used for box-type BT_AABox.

  static const Exc_t eH("BoxSet::AddBox ");

  if (fBoxType != BT_AABox)
    throw(eH + "expect axis-aligned box-type.");

  BAABox* box = (BAABox*) NewDigit();
  box->fA = a; box->fB = b; box->fC = c;
  box->fW = w; box->fH = h; box->fD = d;
}

//______________________________________________________________________________
void BoxSet::AddBox(Float_t a, Float_t b, Float_t c)
{
  // Create a new axis-aligned box from at a given position.
  // To be used for box-type BT_AABoxFixedDim.

  static const Exc_t eH("BoxSet::AddBox ");

  if (fBoxType != BT_AABoxFixedDim)
    throw(eH + "expect axis-aligned fixed-dimension box-type.");

  BAABoxFixedDim* box = (BAABoxFixedDim*) NewDigit();
  box->fA = a; box->fB = b; box->fC = c;
}

/**************************************************************************/

//______________________________________________________________________________
void BoxSet::ComputeBBox()
{
  // Fill bounding-box information of the base-class TAttBBox (virtual method).
  // If member 'FrameBox* fFrame' is set, frame's corners are used as bbox.

  static const Exc_t eH("BoxSet::ComputeBBox ");

  if (fFrame != 0)
  {
    BBoxInit();
    Int_t    n    = fFrame->GetFrameSize() / 3;
    Float_t *bbps = fFrame->GetFramePoints();
    for (int i=0; i<n; ++i, bbps+=3)
      BBoxCheckPoint(bbps);
    return;
  }

  if(fPlex.Size() == 0)
  {
    BBoxZero();
    return;
  }

  BBoxInit();

  VoidCPlex::iterator bi(fPlex);
  switch (fBoxType)
  {

    case BT_FreeBox:
    {
      while (bi.next()) {
        BFreeBox& b = * (BFreeBox*) bi();
        Float_t * p = b.fVertices;
        for(int i=0; i<8; ++i, p+=3)
          BBoxCheckPoint(p);
      }
      break;
    }

    case BT_AABox:
    {
      while (bi.next()) {
        BAABox& b = * (BAABox*) bi();
        BBoxCheckPoint(b.fA, b.fB, b.fC);
        BBoxCheckPoint(b.fA + b.fW, b.fB + b.fH , b.fC + b.fD);
      }
      break;
    }

    case BT_AABoxFixedDim:
    {
      while (bi.next()) {
        BAABoxFixedDim& b = * (BAABoxFixedDim*) bi();
        BBoxCheckPoint(b.fA, b.fB, b.fC);
        BBoxCheckPoint(b.fA + fDefWidth, b.fB + fDefHeight , b.fC + fDefDepth);
      }
      break;
    }

    default:
    {
      throw(eH + "unsupported box-type.");
    }

  } // end switch box-type

  printf("%s BBox is x(%f,%f), y(%f,%f), z(%f,%f)\n", GetName(),
         fBBox[0], fBBox[1], fBBox[2], fBBox[3], fBBox[4], fBBox[5]);
}

/**************************************************************************/

//______________________________________________________________________________
void BoxSet::Test(Int_t nboxes)
{
  // Fill the structure with a random set of boxes.

  Reset(BT_AABox, kTRUE, nboxes);
  TRandom rnd(0);
  const Float_t origin = 10, size = 2;
  Int_t color;
  for(Int_t i=0; i<nboxes; ++i)
  {
    AddBox(origin * rnd.Uniform(-1, 1),
           origin * rnd.Uniform(-1, 1),
           origin * rnd.Uniform(-1, 1),
           size   * rnd.Uniform(0.1, 1),
           size   * rnd.Uniform(0.1, 1),
           size   * rnd.Uniform(0.1, 1));

    Reve::ColorFromIdx(rnd.Integer(256), (UChar_t*)&color);
    DigitValue(color);
  }
}
