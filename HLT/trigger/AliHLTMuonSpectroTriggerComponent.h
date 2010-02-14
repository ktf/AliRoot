//-*- Mode: C++ -*-
// $Id: $
#ifndef AliHLTMUONSPECTROTRIGGERCOMPONENT_H
#define AliHLTMUONSPECTROTRIGGERCOMPONENT_H
/* This file is property of and copyright by the ALICE HLT Project        *
 * ALICE Experiment at CERN, All rights reserved.                         *
 * See cxx source for full Copyright notice                               */

/// @file   AliHLTMuonSpectroTriggerComponent.h
/// @author Artur Szostak <artursz@iafrica.com>
/// @date   9 Nov 2009
/// @brief  Declares the trigger component for the muon spectrometer.

#include "AliHLTTrigger.h"

/**
 * \class AliHLTMuonSpectroTriggerComponent
 * \brief Muon spectrometer trigger component.
 *
 * This component takes the HLT MUON decision generated by the AliHLTMUONDecisionComponent
 * and creates a HLT trigger suitable for the HLT global trigger framework. Some
 * optional summary statistics scalars can be generated with the "-makestats" option.
 * This extra statistics information is filled in the AliHLTMuonSpectroScalars object,
 * and written to a kAliHLTDataTypeEventStatistics type data block. Thus, the scalars
 * object can be automatically added to the AliHLTEventSummary object by the
 * AliHLTEventSummaryProducerComponent.
 *
 * <h2>General properties:</h2>
 *
 * Component ID: \b MuonSpectroTrigger <br>
 * Library: \b libAliHLTTrigger.so <br>
 * Input Data Types: \li AliHLTMUONConstants::DDLRawDataType()               = "DDL_RAW :MUON" <br>
 *                   \li AliHLTMUONConstants::TriggerRecordsBlockDataType()  = "TRIGRECS:MUON" <br>
 *                   \li AliHLTMUONConstants::RecHitsBlockDataType()         = "RECHITS :MUON" <br>
 *                   \li AliHLTMUONConstants::MansoTracksBlockDataType()     = "MANTRACK:MUON" <br>
 *                   \li AliHLTMUONConstants::TracksBlockDataType()          = "TRACKS  :MUON" <br>
 *                   \li AliHLTMUONConstants::SinglesDecisionBlockDataType() = "DECIDSIN:MUON" <br>
 *                   \li AliHLTMUONConstants::PairsDecisionBlockDataType()   = "DECIDPAR:MUON" <br>
 * Output Data Types: \li kAliHLTDataTypeTriggerDecision                      = "TRIG_DEC:HLT " <br>
 *                    \li kAliHLTDataTypeReadoutList                          = "HLTRDLST:HLT " <br>
 *                    \li kAliHLTDataTypeEventStatistics|kAliHLTDataOriginHLT = "EV_STATI:HLT " <br>
 *
 * <h2>Mandatory arguments:</h2>
 * None.
 *
 * <h2>Optional arguments:</h2>
 * \li -makestats <br>
 *      If specified then the summary statistics scalars object is generated as output.
 *      The default is not to generate the statistics object. <br>
 * \li -triggerddls <br>
 *      Indicates that the component should trigger if any ddls are found in the muon
 *      spectrometer. <br>
 * \li -triggerhits <br>
 *      Indicates that the component should trigger if any hits are found in the muon
 *      spectrometer tracking chambers. This option requires that this trigger component
 *      receives the reconstructed hits data blocks. <br>
 * \li -triggertrigrecs <br>
 *      Indicates that the component should trigger if any trigger records are found.
 *      This option requires that this trigger component receives the trigger records
 *      data blocks. <br>
 * \li -triggertracks <br>
 *      Indicates that the component should trigger if any Manso tracks are found.
 *      This option requires that this trigger component receives the Manso track
 *      or singles decision data blocks. <br>
 * \li -triggerdimuons <br>
 *      Indicates that the component should trigger if any dimuon pairs are found.
 *      This option requires that this trigger component receives the pairs decision
 *      data blocks. <br>
 * \li -triggerany <br>
 *      This option indicates that the trigger component should trigger if anything was
 *      reconstructed in the muon spectrometer by the dHLT at all. <br>
 *
 * \note If none of the "triggerhits", "-triggertrigrecs", "-triggertracks" or "-triggerdimuons"
 *      options are specified then the default triggering mode is to trigger on tracks
 *      and dimuon pairs. <br>
 *
 * <h2>Configuration:</h2>
 * Currently there is no configuration required.
 *
 * <h2>Default CDB entries:</h2>
 * None.
 *
 * <h2>Performance:</h2>
 * Under a millisecond per event.
 *
 * <h2>Memory consumption:</h2>
 * Memory consumption is minimal. It should be less than 1 MBytes.
 *
 * <h2>Output size:</h2>
 * Requires up to 4 kBytes
 *
 * \ingroup alihlt_trigger_components
 */
class AliHLTMuonSpectroTriggerComponent : public AliHLTTrigger
{
public:
	AliHLTMuonSpectroTriggerComponent();
	virtual ~AliHLTMuonSpectroTriggerComponent();
	
	/**
	 * Inherited from AliHLTTrigger.
	 * @return string containing the global trigger name.
	 */
	virtual const char* GetTriggerName() const { return "MuonSpectroTrigger"; }
	
	/**
	 * Inherited from AliHLTTrigger. Returns the following input data types
	 * handled by this component:
	 * AliHLTMUONConstants::TriggerRecordsBlockDataType()  = "TRIGRECS:MUON"
	 * AliHLTMUONConstants::RecHitsBlockDataType()         = "RECHITS :MUON"
	 * AliHLTMUONConstants::MansoTracksBlockDataType()     = "MANTRACK:MUON"
	 * AliHLTMUONConstants::TracksBlockDataType()          = "TRACKS  :MUON"
	 * AliHLTMUONConstants::SinglesDecisionBlockDataType() = "DECIDSIN:MUON"
	 * AliHLTMUONConstants::PairsDecisionBlockDataType()   = "DECIDPAR:MUON"
	 * @param list <i>[out]</i>: The list of data types to be filled.
	 */
	virtual void GetInputDataTypes(AliHLTComponentDataTypeList& list) const;
	
	/**
	 * Inherited from AliHLTTrigger. Returns the following output types generated
	 * by this component:
	 * kAliHLTDataTypeTriggerDecision
	 * kAliHLTDataTypeEventStatistics|kAliHLTDataOriginHLT
	 * Including the type kAliHLTDataTypeReadoutList implicitly.
	 * @param list <i>[out]</i>: The list of data types to be filled.
	 */
	virtual void GetOutputDataTypes(AliHLTComponentDataTypeList& list) const;
	
	/**
	 * Inherited from AliHLTTrigger. Returns the estimated data volume required
	 * in bytes: constBase + input_volume * inputMultiplier
	 * @param constBase        <i>[out]</i>: additive part, independent of the
	 *                                   input data volume.
	 * @param inputMultiplier  <i>[out]</i>: multiplication ratio
	 */
	virtual void GetOutputDataSize(unsigned long& constBase, double& inputMultiplier);
	
	/**
	 * Inherited from AliHLTComponent.
	 * \returns a new instance of AliHLTMuonSpectroTriggerComponent.
	 */
	virtual AliHLTComponent* Spawn();

protected:

	/**
	 * Inherited from AliHLTComponent. Initialises the component.
	 * \param argc  The number of arguments in argv.
	 * \param argv  Array of component argument strings.
	 * \returns  Zero on success and negative number on failure.
	 */
	virtual Int_t DoInit(int argc, const char** argv);
	
	/**
	 * Inherited from AliHLTComponent. Cleans up the component.
	 * \returns  Zero on success and negative number on failure.
	 */
	virtual Int_t DoDeinit();
	
	/**
	 * Inherited from AliHLTComponent. Makes a trigger decision for the muon
	 * spectrometer.
	 * @return Zero is returned on success and a negative error code on failure.
	 */
	virtual int DoTrigger();
	
private:

	/// Not implemented. Do not allow copying of this class.
	AliHLTMuonSpectroTriggerComponent(const AliHLTMuonSpectroTriggerComponent& /*obj*/);
	/// Not implemented. Do not allow copying of this class.
	AliHLTMuonSpectroTriggerComponent& operator = (const AliHLTMuonSpectroTriggerComponent& /*obj*/);
	
	/**
	 * Method for checking the structure of a data block.
	 * \param reader  The reader for the data block.
	 * \param type  The data block type as given by the pub/sub framework.
	 * \returns true if the data block structure is OK and false otherwise.
	 */
	template <typename BlockReader>
	bool IsBlockOk(const BlockReader& reader, const AliHLTComponentDataType& type) const;
	
	unsigned long fBufferSizeConst; //! Constant size estimate for GetOutputDataSize.
	double fBufferSizeMultiplier; //! Buffer size multiplier estimate for GetOutputDataSize.
	bool fMakeStats; //! Indicates if the statistics scalars object should be generated or not.
	bool fTriggerDDLs; //! If true then the component will trigger on any dll in the tracking and trigger chambers.
	bool fTriggerHits; //! If true then the component will trigger on any hits in the tracking chambers.
	bool fTriggerTrigRecs; //! If true then the component will trigger on any trigger records.
	bool fTriggerTracks; //! If true then the component will trigger on any tracks found.
	bool fTriggerDimuons; //! If true then the component will trigger on any dimuons.
	
	ClassDef(AliHLTMuonSpectroTriggerComponent, 0);  // Trigger component for the muon spectrometer.
};

#endif // AliHLTMUONSPECTROTRIGGERCOMPONENT_H
