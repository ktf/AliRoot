#include <TH1D.h>
#include <TH2D.h>
#include <TBrowser.h>
#include <TROOT.h>
#include <TH1I.h>
#include <TProfile.h>
#include <TList.h>
#include <TAxis.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TFile.h>
#include <TTree.h>
#include <TError.h>
#include <TStyle.h>
#include <THStack.h>
#include <TLegend.h>
#include <TMath.h>
#include <TParameter.h>
#include "AliAODForwardMult.h"

/** 
 * @defgroup pwg2_forward_analysis_scripts PWG2 Forward analysis - scripts
 *
 * @ingroup pwg2_forward_analysis
 */
/** 
 * Example macro to loop over the event-by-event 2D histogram of 
 * @f[
 *   \frac{d^{2}N_{ch}}{d\eta\,d\phi}
 * @f]
 * stored in an AOD.  
 * 
 * The class needs the files &lt;<i>base</i>&gt;<tt>_hists.root</tt> 
 * containing the histograms generated by AliForwardMultiplicity and 
 * the file &lt;<i>base</i>&gt;<tt>_aods.root</tt> containing the tree 
 * with AliAODEvent objects where AliAODForwardMult objects have been 
 * added to in the branch <tt>Forward</tt>
 * 
 * @ingroup pwg2_forward_analysis_scripts
 */
class DrawResBase : public TObject 
{
public:
  /** 
   * Constructor 
   * 
   * @param special If true, add to the list of 'specials'
   */
  DrawResBase()
    : fBinVzMin(0),
      fBinVzMax(0), 
      fTree(0), 
      fAOD(0),
      fNorm(0),
      fNorm2(0),
      fVtx(0),
      fNAccepted(0),
      fNTriggered(0), 
      fOut(0),
      fEtaNorms(),
      fDirect(0)
  {
  }
  virtual ~DrawResBase() 
  {
    Clear();
  }
  /** 
   * Clear internal structures 
   * 
   */  
  virtual void Clear(Option_t* ="") 
  {
    // Clear previously created data objects 
    if (fTree && fTree->GetCurrentFile()) { 
      fTree->GetCurrentFile()->Close();
      delete fTree;
    }
    if (fOut) {
      fOut->Close();
      delete fOut;
    }

    if (fAOD)     delete fAOD;
    if (fNorm)    delete fNorm;
    if (fNorm2)   delete fNorm2;
    if (fVtx)     delete fVtx;
    fTree    = 0;
    fAOD     = 0;
    fNorm    = 0;
    fNorm2   = 0;
    fVtx     = 0;
    fOut     = 0;

    fEtaNorms.Clear();
    fNAccepted  = 0;
    fNTriggered = 0;
  }
  //__________________________________________________________________
  /** 
   * Open the files &lt;<i>base</i>&gt;<tt>_hists.root</tt> 
   * containing the histograms generated by AliForwardMultiplicity and 
   * the file &lt;<i>base</i>&gt;<tt>_aods.root</tt> containing the tree 
   * with AliAODEvent objects.
   * 
   * @param base  Base name of files 
   * @param vzMin Minimum collision vertex z position to use
   * @param vzMax Maximum collision vertex z position to use
   * @param rebin Rebinning factor 
   * 
   * @return true on success, false otherwise 
   */
  virtual Bool_t Open(const char* base, 
		      Double_t    vzMin=-10, 
		      Double_t    vzMax=10,
		      Bool_t      getOld=false)
  {
    // Set our cuts etc. 
    Double_t tVzMin = vzMin;
    Double_t tVzMax = vzMax;
    if (tVzMax < tVzMin && tVzMin < 0) tVzMax = -tVzMin;

    // Clear previously created data objects 
    Clear();

    // Open the AOD file 
    TString fn   = TString::Format("%s_aods.root", base);
    TFile*  file = TFile::Open(fn.Data(), "READ");
    if (!file) { 
      Error("Init", "Couldn't open %s", fn.Data());
      return kFALSE;
    }

    // Get the AOD tree 
    fTree = static_cast<TTree*>(file->Get("aodTree"));
    if (!fTree) {
      Error("Init", "Couldn't get the tree");
      return kFALSE;
    }

    // Set the branch pointer 
    fTree->SetBranchAddress("Forward", &fAOD);
    
    // Open the histogram file 
    fn   = TString::Format("%s_hists.root", base);
    if (!ReadAuxObjects(fn.Data(), tVzMin, tVzMax, getOld)) { 
      Error("Init", "Failed to read auxillary objects from %s", fn.Data());
      return kFALSE;
    }

    // Open the output file 
    fn   = TString::Format("%s_out.root", base);
    fOut = TFile::Open(fn.Data(), "RECREATE");
    if (!fOut) { 
      Error("Init", "Couldn't open %s", fn.Data());
      return kFALSE;
    }
    
    Info("Open", "Selected vertex bins are [%d,%d]", fBinVzMin, fBinVzMax);
    
    return kTRUE;
  }
  /** 
   * Read in auxillary objects from file, and initialise the acceptance 
   * histograms, and find the vertex bins to use. 
   * 
   * This member function expects the following structure in the file 
   *
   *    PWG2forwardDnDeta              TDirectory
   *     +- Forward                    TList   
   *         +- nEventsTrVtx           TH1I
   *         +- fmdHistCollector       TList
   *             +- vtxbin00           TList
   *             |   +- etaAcceptance  TH1F
   *             +- vtxbin01           TList
   *             |   +- etaAcceptance  TH1F
   *             |...
   *
   * where nEventsTrVtx has nVtx bins, and the subscript on the vtxbin
   * lists run from 0 to nVtx-1.  
   * 
   * - nEventsTrVtx is a 1D histogram of events per vertex bin.  It is
   *   only used to find the minimum and the maximum vertex bins to
   *   use. 
   * - The etaAcceptance histograms are 1D histograms of the covered
   *   eta bins for each vertex bin.
   *
   * A file with the correct output is normally generated when
   * executing the AliForwardMultiplicity task, but can also be
   * generated outside of that analysis task.  In that case, one has
   * to be certain, that the secondary map cuts used are the same, and
   * that the vertex bins are the same as used in the analysis. 
   * 
   * @param filename   File to read from 
   * @param vzMin      Least vertex Z coordinate to accept
   * @param vzMax      Most vertex Z coordinate to accept
   * 
   * @return true on succes, error otherwise 
   */
  virtual Bool_t ReadAuxObjects(const char* filename,
				Double_t vzMin, 
				Double_t vzMax, 
				Bool_t   getOld) 
  { 
    TFile* file = TFile::Open(filename, "READ");
    if (!file) { 
      Error("Init", "Couldn't open %s", filename);
      return kFALSE;
    }
    
    // Get the list stored in the file 
    TList* forward = 
      static_cast<TList*>(file->Get("PWG2forwardDnDeta/Forward"));
    if (!forward)  {
      Error("ReadAuxObjects", "Couldn't get forward list");
      return kFALSE;
    }

    // Get the list from the collector 
    TList* collect = 
      static_cast<TList*>(forward->FindObject("fmdHistCollector"));
    if (!collect)  {
      Error("Init", "Couldn't get collector list");
      return kFALSE;
    }

    // Get the event (by vertex bin) histogram 
    TH1* events = static_cast<TH1I*>(forward->FindObject("nEventsTrVtx"));
    if (!events) {
      Error("ReadAuxObjects", "Couldn't get the event histogram");
      return kFALSE;
    }
    // Find the min/max bins to use based on the cuts given 
    fBinVzMin = events->FindBin(vzMin);
    fBinVzMax = events->FindBin(vzMax-.0000001);

    // Make the normalisation 
    for (Int_t iVz = fBinVzMin; iVz <= fBinVzMax; iVz++) { 

      // Get the acceptance from the input file 
      TList* l = static_cast<TList*>(collect
				     ->FindObject(Form("vtxbin%02d",iVz-1)));
      if (!l) { 
	Error("ReadAuxObjets", "List vtxbin%02d not found in %s", 
	      iVz-1, collect->GetName());
	continue;
      }
      TH1F* ve = static_cast<TH1F*>(l->FindObject("etaAcceptance"));
      if (!ve){ 
	Error("ReadAuxObjects", "No eta acceptance histogram found in  "
	      "vtxbin%02d/etaAcceptance", iVz-1);
	continue;
      }
      TH1F* te = static_cast<TH1F*>(ve->Clone(Form("etaAcceptance%0d",iVz)));
      te->SetDirectory(0);
      fEtaNorms.AddAtAndExpand(te, iVz);
    }
    fEtaNorms.SetOwner(kTRUE);

    Bool_t ret = ReadMoreAuxObjects(file,forward,collect,getOld);
    file->Close();
    return ret;
  }
  //__________________________________________________________________
  virtual Bool_t ReadMoreAuxObjects(TFile* file, 
				    TList* forward, 
				    TList* collect, 
				    Bool_t getOld)
  {
    if (getOld) { 
      fDirect = static_cast<TH1D*>(collect->FindObject("dndeta"));
      if (fDirect) {
	fDirect->SetTitle(Form("%s directly", fDirect->GetTitle()));
	fDirect->SetLineColor(kBlue+1);
	fDirect->SetMarkerColor(kBlue+1);
	fDirect->SetMarkerStyle(21);
	fDirect->SetMarkerSize(0.8);
      }
      else 
	Warning("Finish", "Couldn't get the old dN/deta histogram");
    }

    return kTRUE;
  }
  //__________________________________________________________________
  /** 
   * Check if the passed vertex bin number [1,nVtxBins] is within our 
   * cut. 
   * 
   * @param bin Vertex bin [1,nVtxBins] 
   * 
   * @return true if within cut, false otherwise 
   */
  Bool_t IsInsideVtxCut(Int_t bin) const 
  {
    return bin >= fBinVzMin && bin <= fBinVzMax;
  }
  virtual Bool_t IsInit() const = 0;
  //__________________________________________________________________
  /** 
   * Utility function to set-up histograms based on the input 
   * @f$ dd^{2}N_{ch}/d\eta\,d\phi@f$ histogram.   This member function 
   * is called on the first event so that we have the proper binning 
   * 
   * @param templ Input histogram
   * 
   * @return true on succcess
   */
  virtual Bool_t FirstEvent(const TH2D& templ) 
  { 
    Info("FirstEvent", "Generating histograms");

    const TAxis* etaAxis = templ.GetXaxis();
    // - fNorm is the normalisation 
    fNorm = new TH1D("norm", "Normalisation", 
		     etaAxis->GetNbins(),
		     etaAxis->GetXmin(),
		     etaAxis->GetXmax());
    fNorm->SetFillColor(kRed);
    fNorm->SetFillStyle(3001);
    fNorm->SetXTitle("#eta");
    fNorm->SetYTitle("Normalisation");
    fNorm->SetStats(0);
    fNorm->SetDirectory(0);

    fNorm2 = new TH1D("norm2", "Normalisation", 
		      etaAxis->GetNbins(),
		      etaAxis->GetXmin(),
		      etaAxis->GetXmax());
    fNorm2->SetFillColor(kBlue);
    fNorm2->SetFillStyle(3003);
    fNorm2->SetXTitle("#eta");
    fNorm2->SetYTitle("Normalisation");
    fNorm2->SetStats(0);
    fNorm2->SetDirectory(0);

    // - fVtx is the vertex distribution 
    fVtx = new TH1D("vtx", "Events per vertex bin", 
		    fBinVzMax-fBinVzMin+1, 
		    fBinVzMin, fBinVzMax);
    fVtx->SetXTitle("v_{z} bin");
    fVtx->SetYTitle("Events");
    fVtx->SetDirectory(0);
    fVtx->SetFillColor(kRed+1);
    fVtx->SetFillStyle(3001);
    fVtx->SetStats(0);
    
    return kTRUE;
  }


  //__________________________________________________________________
  /** 
   * Process the events 
   * 
   * 
   * @return true on success, false otherwise 
   */
  virtual Bool_t Process() 
  {
    // Get the number of events in the tree 
    Int_t nEntries  = fTree->GetEntries();
    
    // Loop over the events in the tree 
    for (Int_t event = 0; event < nEntries; event++) { 
      fTree->GetEntry(event);
      if ((event+1) % 1000 == 0) 
	Info("Process", "Event # %6d of %d", event+1, nEntries);
      
      // Get our input histogram 
      const TH2D& hist = fAOD->GetHistogram();

      
      // If fTotal2D is not made yet, do so (first event)
      if (!IsInit()) { 
	if (!FirstEvent(hist)) { 
	  Error("Process", "Failed to initialize on first event");
	  return kFALSE;
	}
      }
      
      // Check the trigger 
      if (!fAOD->IsTriggerBits(AliAODForwardMult::kInel)) { 
	// Info("Process", "Not an INEL event");
	continue;
      }
      fNTriggered++;

      // Get the vertex bin - add 1 as we are using histogram bin 
      // numbers in this class 
      Int_t vtxBin = fAOD->GetVtxBin()+1;
      
      // Check if we're within vertex cut
      if (!IsInsideVtxCut(vtxBin)) continue;

      // Increment our vertex event and total accepted counters 
      fVtx->AddBinContent(vtxBin);
      fNAccepted++;

      for (Int_t iEta = 1; iEta <= hist.GetNbinsX(); iEta++) 
	if (hist.GetBinContent(iEta,0)) 
	  fNorm2->AddBinContent(iEta);

      AddContrib(hist, vtxBin);
    }
    FindNormalization();

    Info("Process", "Got %6d trigger, and accepted %6d out of %6d events", 
	 fNTriggered, fNAccepted, nEntries);

    return kTRUE;
  }
  //__________________________________________________________________
  /** 
   * Normalize the result 
   * 
   */
  void FindNormalization()
  {
    fEtaNorms.ls();
    // Make the normalisation 
    for (Int_t iVz = fBinVzMin; iVz <= fBinVzMax; iVz++) { 

      // Get the acceptance from the input file 
      TH1F* ve = static_cast<TH1F*>(fEtaNorms.At(iVz));
      if (!ve){ 
	Error("FindNormalization", 
	      "No eta acceptance histogram found for vertex bin %d", iVz);
	continue;
      }
      // Add the vertex-dependent acceptance weighted by the number 
      // of events in this vertex bin 
      fNorm->Add(ve, fVtx->GetBinContent(iVz));
    }
    // Set all errors to zero (not used)
    for (Int_t i = 1; i <= fNorm->GetNbinsX(); i++) {
      fNorm->SetBinError(i, 0);
      fNorm2->SetBinError(i, 0);
    }
  } 
  //__________________________________________________________________
  /** 
   * Add a contribtion from @a hist to result
   * 
   * @param hist   Contribution to add 
   * @param vtxBin Vertex bin. 
   */
  virtual void AddContrib(const TH2D& hist, Int_t vtxBin) = 0;

  //__________________________________________________________________
  /** 
   * Called at the end of the job. 
   *
   * It plots the result of the analysis in tree canvases.   
   * - One shows the per-vertex accumalted histograms and compares them 
   *   to the per-vertex histograms made in the analysis. 
   * - Another shows the final @f$ dN_{ch}/d\eta@f$ calculated in 
   *   different ways and compare those to the  @f$ dN_{ch}/d\eta@f$ made 
   *   during the analysis. 
   * - The last canvas shows the @f$ dd^{2}N_{ch}/d\eta\,d\phi@f$ histogram. 
   * 
   * @return true on succes, false otherwise 
   */
  Bool_t Finish(Int_t rebin) 
  {
    // Set the style 
    gStyle->SetOptTitle(0);
    gStyle->SetPadColor(0);
    gStyle->SetPadBorderSize(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetPadRightMargin(0.05);
    gStyle->SetPadTopMargin(0.05);
    gStyle->SetPalette(1);

    // Make a stack to 'auto-scale' when plotting more than 1 histogram.
    THStack* stack = new THStack("results","Results for #frac{dN_{ch}}{d#eta}");

    // Generate the projection from the direct sum of the input histograms, 
    // and scale it to the number of vertex bins and the bin width 
    TH1D* tmp = fNorm;
    fNorm = fNorm2;
    fNorm2 = tmp;
    TH1D* h = GetResult();
    h->SetMarkerColor(kRed+1);
    h->SetMarkerStyle(20);
    h->SetMarkerSize(1);
    h->Scale(Float_t(fNAccepted)/fNTriggered);
    TH1D* s = Symmetrice(h);
    Rebin(h, rebin);
    Rebin(s, rebin);
    stack->Add(h);
    stack->Add(s);
    
    // Get the result from the analysis and plit that too (after modifying 
    // the style and possible rebinning)
    Float_t y1 = 0;
    if (fDirect) {
      fDirect->Scale(Float_t(fNAccepted)/fNTriggered);
      Rebin(fDirect, rebin);
      stack->Add(fDirect, "");
      y1 = 0.3;
    }

    // The second canvas 
    TCanvas* cTotal1D =  new TCanvas("total", "Result", 800, 800);
    cTotal1D->SetFillColor(0);
    cTotal1D->SetBorderMode(0);
    cTotal1D->SetBorderSize(0);
    cTotal1D->cd();

    // Draw stack in pad
    TPad* p1 = new TPad("p1", "p1", 0, y1, 1.0, 1.0, 0, 0);
    p1->Draw();
    p1->cd();
    stack->DrawClone("nostack e1 p");
    stack->Write();

    // Make a legend 
    TLegend* l = p1->BuildLegend(0.31, 0.15, 0.5, 0.6);
    l->SetFillColor(0);
    l->SetBorderSize(0);
    l->SetTextSize(0.04); // l->GetTextSize()/3);
    cTotal1D->cd();

    if (fDirect) { 
      // Create a new stack 
      stack = new THStack("ratios", "Ratios to old method");
      
      // Calculate the ratio of our direct sum of input histograms 
      // result from the analysis  and draw it 
      TH1D* ratio = static_cast<TH1D*>(h->Clone("ratio"));
      ratio->SetDirectory(0);
      ratio->Divide(fDirect);
      TH1* z = Symmetrice(ratio);
      stack->Add(ratio, "");
      stack->Add(z,     "");
    
      // Generate our second pad and draw
      TPad* p2 = new TPad("p2", "p2", 0, 0.0, 1.0, y1, 0, 0);
      p2->Draw();
      p2->cd();
      stack->Draw("nostack e1");
      stack->DrawClone("nostack e1");
      stack->Write();
    }

    // update 
    cTotal1D->cd();

    // Generate the last canvas and show the summed input histogram 
    TCanvas* other = new TCanvas("other", "Other", 800, 600);
    other->SetFillColor(0);
    other->SetBorderMode(0);
    other->SetBorderSize(0);
    other->Divide(2,1);

    other->cd(1);
    fNorm->DrawCopy("HIST");
    fNorm->Write();
    
    other->cd(2);
    fNorm2->DrawCopy("HIST");
    fNorm2->Write();
    // fVtx->DrawCopy();
    // fVtx->Write();

    // out->Write();
    fOut->Close();
    return kTRUE;
  }
  //__________________________________________________________________
  /** 
   * Get the result 
   * 
   * @return The result 
   */
  virtual TH1D* GetResult() { return 0; }

  //__________________________________________________________________
  /** 
   * Rebin a histogram 
   * 
   * @param h     Histogram to rebin
   * @param rebin Rebinning factor 
   * 
   * @return 
   */
  virtual void Rebin(TH1D* h, Int_t rebin) const
  { 
    if (rebin <= 1) return;

    Int_t nBins = h->GetNbinsX();
    if(nBins % rebin != 0) {
      Warning("Rebin", "Rebin factor %d is not a devisor of current number "
	      "of bins %d in the histogram %s", rebin, nBins, h->GetName());
      return;
    }
    if (h->IsA()->InheritsFrom(TProfile::Class())){
      h->Rebin(rebin);
      return;
    }
      
    
    // Make a copy 
    TH1D* tmp = static_cast<TH1D*>(h->Clone("tmp"));
    tmp->Rebin(rebin);
    tmp->SetDirectory(0);

    // The new number of bins 
    Int_t nBinsNew = nBins / rebin;
    for(Int_t i = 1;i<= nBinsNew; i++) {
      Double_t content = 0;
      Double_t sumw    = 0;
      Double_t wsum    = 0;
      Int_t    nbins   = 0;
      for(Int_t j = 1; j<=rebin;j++) {
	Int_t bin = (i-1)*rebin + j;
	if(h->GetBinContent(bin) <= 0) continue;
	Double_t c =  h->GetBinContent(bin);
	Double_t w = 1 / TMath::Power(c,2);
	content    += c;
	sumw       += w;
	wsum       += w * c;
	nbins++;
      }
      
      if(content > 0 ) {
	tmp->SetBinContent(i, wsum / sumw);
	tmp->SetBinError(i,TMath::Sqrt(sumw));
      }
    }

    // Finally, rebin the histogram, and set new content
    h->Rebin(rebin);
    for(Int_t i =1;i<=nBinsNew; i++) {
      h->SetBinContent(i,tmp->GetBinContent(i));
      // h->SetBinError(i,tmp->GetBinError(i));
    }

    delete tmp;
  }
  //__________________________________________________________________
  TH1D* Symmetrice(const TH1D* h) const
  {
    Int_t nBins = h->GetNbinsX();
    TH1D* s     = new TH1D(Form("%s_mirror", h->GetName()),
			   Form("%s (mirrored)", h->GetTitle()), 
			   nBins, 
			   -h->GetXaxis()->GetXmax(), 
			   -h->GetXaxis()->GetXmin());
    s->SetDirectory(0);
    s->SetMarkerColor(h->GetMarkerColor());
    s->SetMarkerSize(h->GetMarkerSize());
    s->SetMarkerStyle(h->GetMarkerStyle()+4);

    // Find the first and last bin with data 
    Int_t first = nBins+1;
    Int_t last  = 0;
    for (Int_t i = 1; i <= nBins; i++) { 
      if (h->GetBinContent(i) <= 0) continue; 
      first = TMath::Min(first, i);
      last  = TMath::Max(last,  i);
    }
    
    Double_t xfirst = h->GetBinCenter(first-1);
    Double_t xlast  = h->GetBinCenter(last);
    Int_t    f1     = h->GetXaxis()->FindBin(-xfirst);
    Int_t    f2     = s->GetXaxis()->FindBin(-xlast);
    Int_t    l2     = s->GetXaxis()->FindBin(xfirst);
    Info("Symmetrice", "Data in [%d,%d], copying data from [%d,%d] to [%d,%d]", 
	 first, last, f1, last, f2, l2);
    for (Int_t i = f1, j=l2; i <= last; i++,j--) { 
      s->SetBinContent(j, h->GetBinContent(i));
      s->SetBinError(j, h->GetBinError(i));
    }
    return s;
  }
    

  //__________________________________________________________________
  /** 
   * Run the post-processing.  
   * 
   * This will open the files &lt;<i>base</i>&gt;<tt>_hists.root</tt>
   * containing the histograms generated by AliForwardMultiplicity and
   * the file &lt;<i>base</i>&gt;<tt>_aods.root</tt> containing the
   * tree with AliAODEvent objects. 
   *
   * Then it will loop over the events, accepting only INEL events
   * that have a primary collision point along z within @a vzMin and
   * @a vzMax.
   *
   * After the processing, the result will be shown in 3 canvases, 
   * possibly  rebinning the result by the factor @a rebin. 
   * 
   * @param base  Base name of files 
   * @param vzMin Minimum collision vertex z position to use
   * @param vzMax Maximum collision vertex z position to use
   * @param rebin Rebinning factor 
   * 
   * @return true on success, false otherwise 
   */
  Bool_t Run(const char* base, 
	     Double_t vzMin=-10,  
	     Double_t vzMax=10, 
	     Int_t rebin=1,
	     Bool_t getOld=false) 
  {
    if (!Open(base,vzMin,vzMax,getOld)) return kFALSE;
    if (!Process())                     return kFALSE;
    if (!Finish(rebin))                 return kFALSE;

    return kTRUE;
  } // *MENU*
  /** 
   * Get name of object
   * 
   * @return Object name 
   */
  const char* GetName() const { return "drawRes"; }
  /** 
   * This is a folder
   *
   * @return always true 
   */
  Bool_t IsFolder() const { return kTRUE; }
  /** 
   * Browse this object 
   * 
   * @param b Browser to use 
   */
  virtual void Browse(TBrowser* b) 
  {
    b->Add(&fEtaNorms, "#eta Acceptances");
    if (fTree)    b->Add(fTree);
    if (fNorm)    b->Add(fNorm);
    if (fNorm2)   b->Add(fNorm2);
    if (fVtx)     b->Add(fVtx);
    if (fDirect)  b->Add(fDirect);
    b->Add(new TParameter<Int_t>   ("binVzMin",    fBinVzMin));
    b->Add(new TParameter<Int_t>   ("binVzMax",    fBinVzMax));
    b->Add(new TParameter<Int_t>   ("fNAccepted",  fNAccepted));
    b->Add(new TParameter<Int_t>   ("fNTriggered", fNTriggered));
  }
protected:
  Int_t              fBinVzMin;  // Corresponding bin to min vertex
  Int_t              fBinVzMax;  // Corresponding bin to max vertex
  TTree*             fTree;      // Event tree 
  AliAODForwardMult* fAOD;       // Our event-by-event data
  TH1D*              fNorm;      // Histogram of # events with data per bin 
  TH1D*              fNorm2;      // Histogram of # events with data per bin 
  TH1D*              fVtx;       // Histogram of # events with data per bin 
  Int_t              fNAccepted; // # of accepted events with vertex 
  Int_t              fNTriggered;// # of events with trigger 
  TFile*             fOut;       // Output file 
  TObjArray          fEtaNorms;  // Array of eta acceptances per vertex 
  TH1D*              fDirect;

  ClassDef(DrawResBase,0)
};

// Local Variables: 
//  mode: C++
// End:
