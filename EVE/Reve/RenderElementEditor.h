// $Header$

#ifndef REVE_RenderElementEditor_H
#define REVE_RenderElementEditor_H

#include <TGedFrame.h>

class TGCheckButton;
class TGNumberEntry;
class TGColorSelect;

namespace Reve {

class RenderElement;

class RenderElementEditor : public TGedFrame
{
protected:
  RenderElement* fRE; // fModel dynamic-casted to RenderElementEditor

  TGHorizontalFrame* fHFrame;
  TGCheckButton*     fRnrElement;
  TGColorSelect*     fMainColor;

public:
  RenderElementEditor(const TGWindow* p, Int_t id, Int_t width = 170, Int_t height = 30, UInt_t options = kChildFrame, Pixel_t back = GetDefaultFrameBackground());
  ~RenderElementEditor();

  virtual void SetModel(TVirtualPad* pad, TObject* obj, Int_t event);

  void DoRnrElement();
  void DoMainColor(Pixel_t color);

   ClassDef(RenderElementEditor, 1); // Editor for RenderElement
}; // endclass RenderElementEditor

}

#endif
