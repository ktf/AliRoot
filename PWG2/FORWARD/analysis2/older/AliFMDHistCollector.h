#ifndef ALIROOT_PWG2_FORWARD_ANALYSIS2_ALIFMDHISTCOLLECTOR_H
#define ALIROOT_PWG2_FORWARD_ANALYSIS2_ALIFMDHISTCOLLECTOR_H
#include <TNamed.h>
#include <TList.h>
#include <TArrayI.h>
#include "AliForwardUtil.h"
class AliESDFMD;
class TH2D;
class TH1I;
class TH1F;

/** 
 * This class collects the event histograms into single histograms, 
 * one for each ring in each vertex bin.  
 *
 * @par Input:
 *   - AliESDFMD object possibly corrected for sharing
 *
 * @par Output:
 *   - 5 RingHistos objects - each with a number of vertex dependent 
 *     2D histograms of the inclusive charge particle density 
 * 
 * @par HistCollector used: 
 *   - AliFMDAnaCalibBackgroundCorrection
 *
 * @ingroup pwg2_forward_analysis 
 */
class AliFMDHistCollector : public TNamed
{
public:
  /** 
   * Constructor 
   */
  AliFMDHistCollector();
  /** 
   * Constructor 
   * 
   * @param name Name of object
   */
  AliFMDHistCollector(const char* name);
  /** 
   * Copy constructor 
   * 
   * @param o Object to copy from 
   */
  AliFMDHistCollector(const AliFMDHistCollector& o);
  /** 
   * Destructor 
   */
  virtual ~AliFMDHistCollector();
  /** 
   * Assignement operator
   * 
   * @param o Object to assign from 
   * 
   * @return Reference to this object
   */
  AliFMDHistCollector& operator=(const AliFMDHistCollector&);
  /** 
   * Intialise 
   * 
   * @param vtxAxis  Vertex axis 
   * @param etaAxis  Eta axis 
   */  
  virtual void Init(const TAxis& vtxAxis, const TAxis& etaAxis);
  /** 
   * Do the calculations 
   * 
   * @param hists    Cache of histograms 
   * @param vtxBin   Vertex bin 
   * @param out      Output histogram
   * 
   * @return true on successs 
   */
  virtual Bool_t Collect(AliForwardUtil::Histos& hists, Int_t vtxBin, 
			 TH2D& out);
  /** 
   * Scale the histograms to the total number of events 
   * 
   * @param nEvents Number of events 
   */
  void ScaleHistograms(const TH1I& nEvents);
  /** 
   * Output diagnostic histograms to directory 
   * 
   * @param dir List to write in
   */  
  void Output(TList* dir);
  /** 
   * Set the number of extra bins (beyond the secondary map border) 
   * to cut away. 
   * 
   * @param n Number of bins 
   */
  void SetNCutBins(UInt_t n=2) { fNCutBins = n; }
  /** 
   * Set the correction cut, that is, when bins in the secondary
   * correction maps have a value lower than this cut, they are
   * considered uncertain and not used
   * 
   * @param cut Cut-off 
   */
  void SetCorrectionCut(Float_t cut=0.5) { fCorrectionCut = cut; }
  /** 
   * Whether to use the eta range from the data 
   * 
   * @param use 
   */
  void UseEtaFromData(Bool_t use=kTRUE) { fUseEtaFromData = use; }
protected:
  /** 
   * Add the 5 input histograms to our internal sum of vertex
   * dependent histograms
   * 
   * @param hists   Result 
   * @param vtxBin  Vertex bin 
   */
  virtual void Store(AliForwardUtil::Histos& hists, Int_t vtxBin);
  /** 
   * Merge the 5 input histograms into a single histogram
   * 
   * @param hists   Result
   * @param vtxBin  Vertex bin
   * @param out     Output histogram 
   */
  virtual void Merge(AliForwardUtil::Histos& hists, Int_t vtxBin, TH2D& out);
  /** 
   * Get the first and last eta bin to use for a given ring and vertex 
   * 
   * @param d        Detector
   * @param r        Ring 
   * @param vtxBin   Vertex bin 
   * @param first    On return, the first eta bin to use 
   * @param last     On return, the last eta bin to use 
   */
  virtual void GetFirstAndLast(UShort_t d, Char_t r, Int_t vtxBin, 
			       Int_t& first, Int_t& last) const;
  /** 
   * Get the first and last eta bin to use for a given ring and vertex 
   * 
   * @param idx      Ring index as given by GetIdx
   * @param vtxBin   Vertex bin 
   * @param first    On return, the first eta bin to use 
   * @param last     On return, the last eta bin to use 
   */
  virtual void GetFirstAndLast(Int_t idx, Int_t vtxBin, 
			       Int_t& first, Int_t& last) const;
  /** 
   * Get the first eta bin to use for a given ring and vertex 
   * 
   * @param d Detector 
   * @param r Ring 
   * @param v vertex bin
   * 
   * @return First eta bin to use, or -1 in case of problems 
   */  
  Int_t GetFirst(UShort_t d, Char_t r, Int_t v) const; 
  /** 
   * Get the first eta bin to use for a given ring and vertex 
   * 
   * @param i Ring index as given by GetIdx
   * @param v vertex bin
   * 
   * @return First eta bin to use, or -1 in case of problems 
   */  
  Int_t GetFirst(Int_t idx, Int_t v) const; 
  /** 
   * Get the last eta bin to use for a given ring and vertex 
   * 
   * @param d Detector 
   * @param r Ring 
   * @param v vertex bin
   * 
   * @return Last eta bin to use, or -1 in case of problems 
   */  
  Int_t GetLast(UShort_t d, Char_t r, Int_t v) const;
  /** 
   * Get the last eta bin to use for a given ring and vertex 
   * 
   * @param i Ring index as given by GetIdx
   * @param v vertex bin
   * 
   * @return Last eta bin to use, or -1 in case of problems 
   */  
  Int_t GetLast(Int_t idx, Int_t v) const; 
  /** 
   * Get the detector and ring from the ring index 
   * 
   * @param idx Ring index 
   * @param d   On return, the detector or 0 in case of errors 
   * @param r   On return, the ring id or '\0' in case of errors 
   */
  void GetDetRing(Int_t idx, UShort_t& d, Char_t& r) const;
  /** 
   * Get the ring index from detector number and ring identifier 
   * 
   * @param d Detector
   * @param r Ring identifier 
   * 
   * @return ring index or -1 in case of problems 
   */
  Int_t GetIdx(UShort_t d, Char_t r) const;
  /** 
   * Get the possibly overlapping histogram of eta bin @a e in 
   * detector and ring 
   * 
   * @param d Detector
   * @param r Ring 
   * @param e Eta bin
   * @param v Vertex bin
   *
   * @return Overlapping histogram index or -1
   */
  Int_t GetOverlap(UShort_t d, Char_t r, Int_t e, Int_t v) const;
  /** 
   * Get the possibly overlapping histogram of eta bin @a e in 
   * detector and ring 
   * 
   * @param i Ring index
   * @param e Eta bin
   * @param v Vertex bin
   *
   * @return Overlapping histogram index or -1
   */
  Int_t GetOverlap(Int_t i, Int_t e, Int_t v) const;
  /** 
   * Check if there's an overlapping histogram with this eta bin of
   * the detector and ring
   * 
   * @param d Detector 
   * @param r Ring 
   * @param e eta bin
   * @param v Vertex bin
   * 
   * @return True if there's an overlapping histogram 
   */
  Bool_t HasOverlap(UShort_t d, Char_t r, Int_t e, Int_t v) const;
  /** 
   * Check if there's an overlapping histogram with this eta bin of
   * ring
   * 
   * @param i Ring index
   * @param e eta bin
   * @param v Vertex bin
   * 
   * @return True if there's an overlapping histogram 
   */
  Bool_t HasOverlap(Int_t i, Int_t e, Int_t v) const;


  TList       fList;            // List of histogram containers
  const TH1I* fNEvents;         // Reference event histogram 
  Int_t       fNCutBins;        // Number of additional bins to cut away
  Float_t     fCorrectionCut;   // Cut-off on secondary corrections 
  TArrayI     fFirstBins;       // Array of first eta bins 
  TArrayI     fLastBins;        // Array of last eta bins 
  Bool_t      fUseEtaFromData;  // Wether to use the data for the limits
  TH1F*       fEtaNorm;         // Normalisation in eta 
  TList       fOutput;

  ClassDef(AliFMDHistCollector,1); // Calculate Nch density 
};

//____________________________________________________________________
inline void
AliFMDHistCollector::GetFirstAndLast(UShort_t d, Char_t r, Int_t vtxbin, 
				     Int_t& first, Int_t& last) const
{
  GetFirstAndLast(GetIdx(d,r), vtxbin, first, last);
}
//____________________________________________________________________
inline Int_t
AliFMDHistCollector::GetFirst(UShort_t d, Char_t r, Int_t v) const 
{
  return GetFirst(GetIdx(d,r), v);
}
//____________________________________________________________________
inline Int_t
AliFMDHistCollector::GetLast(UShort_t d, Char_t r, Int_t v) const 
{
  return GetLast(GetIdx(d, r), v);
}
//____________________________________________________________________
inline Bool_t
AliFMDHistCollector::HasOverlap(UShort_t d, Char_t r, Int_t e, Int_t v) const
{
  return GetOverlap(d,r,e,v) >= 0;
}
//____________________________________________________________________
inline Bool_t
AliFMDHistCollector::HasOverlap(Int_t i, Int_t e, Int_t v) const
{
  return GetOverlap(i,e,v) >= 0;
}

#endif
// Local Variables:
//   mode: C++
// End:

