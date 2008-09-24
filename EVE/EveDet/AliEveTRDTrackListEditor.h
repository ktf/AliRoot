// Author: Alexandru Bercuci and Benjamin Hess
// Last change: 23/09/2008
/**************************************************************************
 * Copyright(c) 1998-2008, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#ifndef AliEveTRDTrackListEditor_H
#define AliEveTRDTrackListEditor_H

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// AliEveTRDTrackListEditor                                             //
//                                                                      //
// The AliEveTRDTrackListEditor provides the graphical functionality    //
// for the AliEveTRDTrackList. It creates the tabs and canvases, when   //
// they are needed and, as well, frees allocated memory on destruction  //
// (or if new events are loaded and thus some tabs are closed).         //
// The function DrawHistos() accesses the temporary file created by the //
// AliEveTRDTrackList and draws the desired data (the file will be      //
// created within the call of ApplyMacros()). Have a look at this       //
// function to learn more about the structure of the file and how to    //
// access the data.                                                     //
//                                                                      //
// Authors :                                                            //
//    A.Bercuci <A.Bercuci@gsi.de>                                      //
//    B.Hess <Hess@Stud.Uni-Heidelberg.de>                              //
//////////////////////////////////////////////////////////////////////////

#include <TGedFrame.h>

class AliEveTRDTrack;
class AliEveTRDTrackList;
class AliTRDReconstructor;
class TCanvas;     
class TEveBrowser;           
class TEveGedEditor;
class TEveManager;
class TFile;
class TGButtonGroup;
class TGCheckButton;
class TGFileInfo;
class TGGroupFrame;
class TGHorizontal3DLine;
class TGHorizontalFrame;
class TGLabel;
class TGListBox;
class TGRadioButton;
class TGString;
class TGTab;
class TGTextButton;
class TGTextEntry;
class TGVerticalFrame;
class TH1;
class TTree;

class AliEveTRDTrackListEditor: public TGedFrame
{
public:
  AliEveTRDTrackListEditor(const TGWindow* p = 0, Int_t width = 170, Int_t height = 30,
		                       UInt_t options = kChildFrame, Pixel_t back = GetDefaultFrameBackground());
  virtual ~AliEveTRDTrackListEditor();

  virtual void SetModel(TObject* obj);

  void AddMacro(const Char_t* path, const Char_t* name);  // Adds macro to the macro list
  void ApplyMacros();                                     // Apply macros
  void BrowseMacros();                                    // Browse macros
  void CloseTabs();                                       // Closes + deletes all the tabs created by this class
  void DrawHistos();                                      // Draw histograms
  Int_t GetNSelectedHistograms();                         // Get the number of selected histograms for drawing
  void HandleMacroPathSet();                              // Handles the "macro path set"-signal
  void HandleNewEventLoaded();                            // Handles the "NewEventLoaded()"-signal
  void HandleTabChangedToIndex(Int_t);                    // Handles the "Selected(Int_t id)"-signal (tab changed)
  void RemoveMacros();                                    // Removes the selected macros from the lists
  void SetTrackColor(Int_t ind);                          // Sets the color model
  void SetTrackModel(Int_t ind);                          // Sets the track model
  void UpdateDataFromMacroListSelection();                // Updates the selection in the "data from macro"-list
  void UpdateHistoList();                                 // Updates the histogram list
  void UpdateMacroList();                                 // Updates the macro list
  void UpdateMacroListSelection(Int_t ind);               // Updates the selection of the process macro list
  void UpdateMacroSelListSelection(Int_t ind);            // Updates the selection of the selection macro list
  
protected:
  AliEveTRDTrackList* fM;                                 // Model object

  void InheritMacroList();                                // Inherits macro list from the previously loaded track list
  void InheritStyle();                                    // Inherits the style from the previously loaded track list

private:
  AliEveTRDTrackListEditor(const AliEveTRDTrackListEditor&);            // Not implemented
  AliEveTRDTrackListEditor& operator=(const AliEveTRDTrackListEditor&); // Not implemented 

  TCanvas*          fHistoCanvas;            // Canvas for the histograms
  TGString*         fHistoCanvasName;        // Name of the histogram canvas

  Bool_t            fInheritSettings;        // Flag indicating, whether the macro list and the style settings will be 
                                             // inherited from the previously loaded track list within the next call 
                                             // of SetModel

  TGHorizontalFrame* fStyleFrame;            // Frame for the style stuff
  TGVerticalFrame*   fMainFrame;             // Top frame for macro functionality.
  TGVerticalFrame*   fHistoFrame;            // Top frame for the histogram stuff
  TGVerticalFrame*   fHistoSubFrame;         // Frame for the histogram buttons themselves
  TGHorizontalFrame* fBrowseFrame;           // Frame for features corresponding to searching macros
  TGButtonGroup*     fbgStyleColor;          // Button group for the color model
  TGButtonGroup*     fbgStyleTrack;          // Button group for the track model
  
  TGRadioButton**    frbColor;               // Radio buttons for the color model
  TGRadioButton**    frbTrack;               // Radio buttons for the track model

  TGTextButton*   fbBrowse;                  // "Browse" button
  TGTextButton*   fbApplyMacros;             // "Apply macros" button
  TGTextButton*   fbRemoveMacros;            // "Remove macros" button
  TGTextButton*   fbDrawHisto;               // "Draw histogram" button
  TGTextEntry*    fteField;                  // Text field to insert macro path manually
  TGListBox*      ftlMacroList;              // To display the list of (process) macros
  TGListBox*      ftlMacroSelList;           // To display the list of (selection) macros

  TGFileInfo*     fFileInfo;                 // Holds data about opening macros
  Char_t**        fFileTypes;                // File types (for macros)

  // Some labels
  TGLabel* fLabel1;
  TGLabel* fLabel2;
  TGLabel* fLabel3;
  TGLabel* fLabel4;
     
  // Some lines
  TGHorizontal3DLine *fLine1;
  TGHorizontal3DLine *fLine2;
  TGHorizontal3DLine *fLine3;
  TGHorizontal3DLine *fLine4;
  TGHorizontal3DLine *fLine5;  

  TGCheckButton** fCheckButtons;            // Check buttons for histograms

  // Help functions
  void SetDrawingToHistoCanvasTab();        // Sets focus on the tab for histograms and makes fHistoCanvas be the
                                            // current tab
  void UpdateHistoCanvasTab();              // Updates the histogram and the corresponding tab (including titles)

  ClassDef(AliEveTRDTrackListEditor, 0);    // Editor for AliEveTRDTrackList.
};

#endif
