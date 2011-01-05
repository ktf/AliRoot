#ifndef ALIFMDMCCORRECTIONS_H
#define ALIFMDMCCORRECTIONS_H
#include "AliFMDCorrections.h"
#include <TList.h>
class TProfile2D;
class TH2;

/** 
 * @defgroup pwg2_forward_mc  Monte-carlo code
 * 
 * @ingroup pwg2_forward
 */
/** 
 * This class calculates the exclusive charged particle density
 * in each for the 5 FMD rings. 
 *
 * @par Input:
 *   - 5 RingHistos objects - each with a number of vertex dependent 
 *     2D histograms of the inclusive charge particle density 
 *
 * @par Output:
 *   - 5 RingHistos objects - each with a number of vertex dependent 
 *     2D histograms of the exclusive charge particle density 
 * 
 * @par Corrections used: 
 *   - AliFMDCorrSecondaryMap;
 *   - AliFMDCorrVertexBias
 *   - AliFMDCorrMergingEfficiency
 *
 * @ingroup pwg2_forward_algo
 * @ingroup pwg2_forward_mc
 */
class AliFMDMCCorrections : public AliFMDCorrections
{
public:
  /** 
   * Constructor 
   */
  AliFMDMCCorrections()
    : AliFMDCorrections(),
      fFMD1i(0), 
      fFMD2i(0),
      fFMD2o(0),
      fFMD3i(0),
      fFMD3o(0),
      fComps(0)
  {}
  /** 
   * Constructor 
   * 
   * @param name Name of object
   */
  AliFMDMCCorrections(const char* name)
    : AliFMDCorrections(name),
      fFMD1i(0), 
      fFMD2i(0),
      fFMD2o(0),
      fFMD3i(0),
      fFMD3o(0),
      fComps(0)
  {}
  /** 
   * Copy constructor 
   * 
   * @param o Object to copy from 
   */
  AliFMDMCCorrections(const AliFMDMCCorrections& o)
    : AliFMDCorrections(o),
      fFMD1i(o.fFMD1i), 
      fFMD2i(o.fFMD2i),
      fFMD2o(o.fFMD2o),
      fFMD3i(o.fFMD3i),
      fFMD3o(o.fFMD3o),
      fComps(0)
  {}
  /** 
   * Destructor 
   */
  virtual ~AliFMDMCCorrections();
  /** 
   * Assignement operator
   * 
   * @param o Object to assign from 
   * 
   * @return Reference to this object
   */
  AliFMDMCCorrections& operator=(const AliFMDMCCorrections&);
  /** 
   * Initialize this object 
   * 
   * @param etaAxis Eta axis to use 
   */
  void Init(const TAxis& etaAxis);
  /** 
   * Do the calculations 
   * 
   * @param hists    Cache of histograms 
   * @param vtxBin   Vertex bin 
   * 
   * @return true on successs 
   */
  virtual Bool_t CorrectMC(AliForwardUtil::Histos& hists, UShort_t vtxBin);
  /** 
   * Compare the result of analysing the ESD for 
   * the inclusive charged particle density to analysing 
   * MC truth 
   * 
   * @param esd 
   * @param mc 
   * 
   * @return 
   */
  virtual Bool_t CompareResults(AliForwardUtil::Histos& esd, 
				AliForwardUtil::Histos& mc);
  /** 
   * Output diagnostic histograms to directory 
   * 
   * @param dir List to write in
   */  
  void DefineOutput(TList* dir);
protected:
  /** 
   * MAke comparison profiles
   * 
   * @param d     Detector 
   * @param r     Ring 
   * @param axis  Eta axis 
   * 
   * @return Newly allocated profile object
   */
  TProfile2D* Make(UShort_t d, Char_t r, const TAxis& axis) const;
  /** 
   * Fill comparison profiles
   * 
   * @param d    Detector 
   * @param r    Ring 
   * @param esd  ESD histogram
   * @param mc   MC histogram
   */
  void Fill(UShort_t d, Char_t r, TH2* esd, TH2* mc);

  TProfile2D* fFMD1i; // Comparison
  TProfile2D* fFMD2i; // Comparison
  TProfile2D* fFMD2o; // Comparison
  TProfile2D* fFMD3i; // Comparison
  TProfile2D* fFMD3o; // Comparison
  TList*      fComps; // List of comparisons 
  
  ClassDef(AliFMDMCCorrections,1); // Calculate Nch density 
};

#endif
// Local Variables:
//   mode: C++
// End:

