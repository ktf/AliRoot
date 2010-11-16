#ifndef ALIROOT_PWG2_FORWARD_ALIFMDSHARINGFILTER_H
#define ALIROOT_PWG2_FORWARD_ALIFMDSHARINGFILTER_H
#include <TNamed.h>
#include <TH2.h>
#include <TList.h>
class AliESDFMD;
class TAxis;
class TList;
class TH2;

/**
 * Class to do the sharing correction.  That is, a filter that merges 
 * adjacent strip signals presumably originating from a single particle 
 * that impinges on the detector in such a way that it deposite energy 
 * into two or more strips. 
 *
 * @par Input: 
 *    - AliESDFMD object  - from reconstruction
 *
 * @par Output: 
 *    - AliESDFMD object  - copy of input, but with signals merged 
 *
 * @par Corrections used: 
 *    - AliFMDAnaCalibEnergyDistribution objects 
 *
 * @par Histograms: 
 *    - For each ring (FMD1i, FMD2i, FMD2o, FMD3i, FMD3o) the distribution of 
 *      signals before and after the filter.  
 *    - For each ring (see above), an array of distributions of number of 
 *      hit strips for each vertex bin (if enabled - see Init method)
 * 
 *
 * @ingroup pwg2_forward_analysis 
 */
class AliFMDSharingFilter : public TNamed
{
public: 
  /** 
   * Destructor
   */
  virtual ~AliFMDSharingFilter();
  /** 
   * Default Constructor - do not use 
   */
  AliFMDSharingFilter();
  /** 
   * Constructor 
   * 
   * @param title Title of object  - not significant 
   */
  AliFMDSharingFilter(const char* title);
  /** 
   * Copy constructor 
   * 
   * @param o Object to copy from 
   */
  AliFMDSharingFilter(const AliFMDSharingFilter& o);
  /** 
   * Assignment operator 
   * 
   * @param o Object to assign from 
   * 
   * @return Reference to this 
   */
  AliFMDSharingFilter& operator=(const AliFMDSharingFilter& o);

  /** 
   * Initialise the filter 
   * 
   */
  void   Init() {}
  /** 
   * Set the low cut used for sharing 
   * 
   * @param lowCut Low cut
   */
  void SetLowCut(Double_t lowCut=0.3) { fLowCut = lowCut; }

  /** 
   * Enable use of angle corrected signals in the algorithm 
   * 
   * @param use If true, use angle corrected signals, 
   * otherwise use de-corrected signals.  In the final output, the 
   * signals are always angle corrected. 
   */
  void UseAngleCorrectedSignals(Bool_t use) { fCorrectAngles = use; }
  /** 
   * Filter the input AliESDFMD object
   * 
   * @param input     Input 
   * @param lowFlux   If this is a low-flux event 
   * @param output    Output AliESDFMD object 
   * @param vz        Current vertex position 
   * 
   * @return True on success, false otherwise 
   */
  Bool_t Filter(const AliESDFMD& input, 
		Bool_t           lowFlux, 
		AliESDFMD&       output,
		Double_t         vz);
  /** 
   * Scale the histograms to the total number of events 
   * 
   * @param nEvents Number of events 
   */
  void ScaleHistograms(Int_t nEvents);
  
  void Output(TList* dir);
protected:
  /** 
   * Internal data structure to keep track of the histograms
   */
  struct RingHistos : public TObject 
  { 
    /** 
     * Default CTOR
     */
    RingHistos();
    /** 
     * Constructor
     * 
     * @param d detector
     * @param r ring 
     */
    RingHistos(UShort_t d, Char_t r);
    /** 
     * Copy constructor 
     * 
     * @param o Object to copy from 
     */
    RingHistos(const RingHistos& o);
    /** 
     * Assignment operator 
     * 
     * @param o Object to assign from 
     * 
     * @return Reference to this 
     */
    RingHistos& operator=(const RingHistos& o);
    /** 
     * Destructor 
     */
    ~RingHistos();
    /** 
     * Initialise this object 
     */
    void Init() {} 
    void Clear(const Option_t* ="") { fNHits = 0; } 
    void Incr() { fNHits++; } 
    void Finish(); 
    void Output(TList* dir);
    UShort_t  fDet;          // Detector
    Char_t    fRing;         // Ring
    TH1D*     fBefore;       // Distribution of signals before filter
    TH1D*     fAfter;        // Distribution of signals after filter
    TH1D*     fHits;         // Distribution of hit strips. 
    Int_t     fNHits;        // Number of hit strips per event
  };
  /** 
   * Get the ring histogram container 
   * 
   * @param d Detector
   * @param r Ring 
   * 
   * @return Ring histogram container 
   */
  RingHistos* GetRingHistos(UShort_t d, Char_t r) const;
  /** 
   * Get the signal in a strip 
   * 
   * @param fmd   ESD object
   * @param d     Detector
   * @param r     Ring 
   * @param s     Sector 
   * @param t     Strip
   * 
   * @return The energy signal 
   */
  Double_t SignalInStrip(const AliESDFMD& fmd, 
			 UShort_t d,
			 Char_t   r,
			 UShort_t s,
			 UShort_t t) const;
  /** 
   * The actual algorithm 
   * 
   * @param mult      The unfiltered signal in the strip
   * @param eta       Psuedo rapidity 
   * @param prevE     Previous strip signal (or 0)
   * @param nextE     Next strip signal (or 0) 
   * @param lowFlux   Whether this is a low flux event 
   * @param d         Detector
   * @param r         Ring 
   * @param s         Sector 
   * @param t         Strip
   * @param usedPrev  Whether the previous strip was used in sharing or not
   * @param usedThis  Wether this strip was used in sharing or not. 
   * 
   * @return The filtered signal in the strip
   */
  Double_t MultiplicityOfStrip(Double_t mult,
			       Double_t eta,
			       Double_t prevE,
			       Double_t nextE,
			       Bool_t   lowFlux,
			       UShort_t d,
			       Char_t   r,
			       UShort_t s,
			       UShort_t t,
			       Bool_t&  usedPrev, 
			       Bool_t&  usedThis);
  /** 
   * Angle correct the signal 
   * 
   * @param mult Angle Un-corrected Signal 
   * @param eta  Pseudo-rapidity 
   * 
   * @return Angle corrected signal 
   */
  Double_t AngleCorrect(Double_t mult, Double_t eta) const;
  /** 
   * Angle de-correct the signal 
   * 
   * @param mult Angle corrected Signal 
   * @param eta  Pseudo-rapidity 
   * 
   * @return Angle un-corrected signal 
   */
  Double_t DeAngleCorrect(Double_t mult, Double_t eta) const;
  /**
   * Get the high cut.  The high cut is defined as the 
   * most-probably-value peak found from the energy distributions, minus 
   * 2 times the width of the corresponding Landau.
   */
  virtual Double_t GetHighCut(UShort_t d, Char_t r, Double_t eta) const;

  TList    fRingHistos;    // List of histogram containers
  Double_t fLowCut;        // Low cut on sharing
  Bool_t   fCorrectAngles; // Whether to work on angle corrected signals
  TH2*     fEtaCorr;

  ClassDef(AliFMDSharingFilter,1); //
};

#endif
// Local Variables:
//  mode: C++ 
// End:
