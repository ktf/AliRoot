/**************************************************************************
 * This file is property of and copyright by the ALICE HLT Project        * 
 * ALICE Experiment at CERN, All rights reserved.                         *
 *                                                                        *
 * Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *
 *                  Timm Steinbeck <timm@kip.uni-heidelberg.de>           *
 *                  for The ALICE HLT Project.                            *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/** @file   AliHLTEMCALClusterizerComponent.cxx
    @author Mateusz Ploskon
    @date   
    @brief  EMCAL clusterizer component for HLT. */

#if __GNUC__== 3
using namespace std;
#endif

#include "AliHLTEMCALClusterizerComponent.h"
#include "AliHLTEMCALDefinitions.h"

#include "TString.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "AliCDBEntry.h"
#include "AliCDBManager.h"

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTEMCALClusterizerComponent)

AliHLTEMCALClusterizerComponent::AliHLTEMCALClusterizerComponent()
  : AliHLTProcessor()
  , fOutputPercentage(100)
  , fStorageDBpath("local://$ALICE_ROOT")
  , fClusterizer(NULL)
  , fCDB(NULL)
  , fMemReader(NULL)
  , fGeometryFileName("")
  , fGeometryFile(NULL)
  , fGeoManager(NULL)
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt
}

AliHLTEMCALClusterizerComponent::AliHLTEMCALClusterizerComponent(const AliHLTEMCALClusterizerComponent &/*c*/)
  : AliHLTProcessor()
  , fOutputPercentage(100)
  , fStorageDBpath("local://$ALICE_ROOT")
  , fClusterizer(NULL)
  , fCDB(NULL)
  , fMemReader(NULL)
  , fGeometryFileName("")
  , fGeometryFile(NULL)
  , fGeoManager(NULL)
{
  // may not use the copy contructor
  HLTError("May not use.");
}

AliHLTEMCALClusterizerComponent& AliHLTEMCALClusterizerComponent::operator=(const AliHLTEMCALClusterizerComponent&)
{
  // may not use the copy contructor
  HLTError("May not use.");
  return *this;
}

AliHLTEMCALClusterizerComponent::~AliHLTEMCALClusterizerComponent()
{
  // see header file for class documentation
}

AliHLTComponentDataType AliHLTEMCALClusterizerComponent::GetOutputDataType()
{
  return AliHLTEMCALDefinitions::fgkClusterDataType;
}

int AliHLTEMCALClusterizerComponent::DoInit( int argc, const char** argv )
{
  // see header file for class documentation
  int iResult=0;
  HLTInfo("parsing %d arguments", argc);

  TString argument="";
  TString configuration=""; 
  int bMissingParam=0;
  bool bHaveMandatory1=false;
  bool bHaveMandatory2=false;

  char *cpErr = 0;

  for (int i=0; i<argc && iResult>=0; i++) {
    argument=argv[i];
    if (argument.IsNull()) continue;

    // -mandatory1
    if (argument.CompareTo("-cdb")==0) {
      bHaveMandatory1|=1;
      if ((bMissingParam=(++i>=argc))) break;
      HLTInfo("got \'-cdb\' argument: %s", argv[i]);
      fStorageDBpath = argv[i];
      HLTInfo("CDB path is: %s", fStorageDBpath.c_str());
      // -mandatory2
    } else if (argument.CompareTo("-geometry")==0) {
      bHaveMandatory2|=1;
      HLTInfo("got \'-geometry\' argument");
      fGeometryFileName = argv[i];
      HLTInfo("Geometry file is: %s", fGeometryFileName.c_str());
      // -optional1
    } else if (argument.CompareTo("-output_percentage")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      HLTInfo("got \'-output_percentage\' argument: %s", argv[i]);
      fOutputPercentage = strtoul(argv[i], &cpErr, 0);
      if ( *cpErr )
	{
	  HLTError("Unable to convert ouput_percentage to a number %s", argv[i]);
	  return -EINVAL;
	}

//       // -optional2
//     } else if (argument.CompareTo("-optional2")==0) {
//       HLTInfo("got \'-optional2\' argument");

    } else {
      // the remaining arguments are treated as configuration
      if (!configuration.IsNull()) configuration+=" ";
      configuration+=argument;
    }
  }
  if (bMissingParam) {
    HLTError("missing parameter for argument %s", argument.Data());
    iResult=-EINVAL;
  }

  if (iResult>=0 && !bHaveMandatory1) {
    HLTError("mandatory argument \'-cdb\' missing");
    iResult=-EPROTO;
  }
  if (iResult>=0 && !bHaveMandatory2) {
    HLTError("mandatory argument \'-geometry\' missing");
    iResult=-EPROTO;
  }
  if (iResult>=0 && !configuration.IsNull()) {
    iResult=Configure(configuration.Data());
  } else {
    iResult=Reconfigure(NULL, NULL);
  }

  return iResult;
}

int AliHLTEMCALClusterizerComponent::DoDeinit()
{
  // see header file for class documentation
  HLTInfo("processing cleanup");
  return 0;
}

int AliHLTEMCALClusterizerComponent::DoEvent( const AliHLTComponentEventData& evtData, const AliHLTComponentBlockData* blocks, 
				      AliHLTComponentTriggerData& trigData, AliHLTUInt8_t* outputPtr, 
				      AliHLTUInt32_t& size, vector<AliHLTComponentBlockData>& outputBlocks ) {
  // see header file for class documentation
  HLTInfo("processing data");
  if (evtData.fStructSize==0 && blocks==NULL && trigData.fStructSize==0 &&
      outputPtr==0 && size==0)
  {
    outputBlocks.clear();
    // this is just to get rid of the warning "unused parameter"
  }
  return 0;
}

int AliHLTEMCALClusterizerComponent::Configure(const char* arguments)
{
  // see header file for class documentation
  int iResult=0;
  if (!arguments) return iResult;
  HLTInfo("parsing configuration string \'%s\'", arguments);

  TString allArgs=arguments;
  TString argument;
  int bMissingParam=0;

  TObjArray* pTokens=allArgs.Tokenize(" ");
  if (pTokens) {
    for (int i=0; i<pTokens->GetEntries() && iResult>=0; i++) {
      argument=((TObjString*)pTokens->At(i))->GetString();
      if (argument.IsNull()) continue;

      // -config1
      if (argument.CompareTo("-config1")==0) {
	if ((bMissingParam=(++i>=pTokens->GetEntries()))) break;
	HLTInfo("got \'-config1\': %s", ((TObjString*)pTokens->At(i))->GetString().Data());

	// -config2
      } else if (argument.CompareTo("-config2")==0) {
	HLTInfo("got \'-config2\'");
      } else {
	HLTError("unknown argument %s", argument.Data());
	iResult=-EINVAL;
	break;
      }
    }
    delete pTokens;
  }
  if (bMissingParam) {
    HLTError("missing parameter for argument %s", argument.Data());
    iResult=-EINVAL;
  }
  return iResult;
}

int AliHLTEMCALClusterizerComponent::Reconfigure(const char* cdbEntry, const char* chainId)
{
  // see header file for class documentation
  int iResult=0;
  const char* path="HLT/ConfigSample/EMCALClusterizerComponent";
  const char* defaultNotify="";
  if (cdbEntry) {
    path=cdbEntry;
    defaultNotify=" (default)";
  }
  if (path) {
    HLTInfo("reconfigure from entry %s%s, chain id %s", path, defaultNotify,(chainId!=NULL && chainId[0]!=0)?chainId:"<none>");
    AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(path/*,GetRunNo()*/);
    if (pEntry) {
      TObjString* pString=dynamic_cast<TObjString*>(pEntry->GetObject());
      if (pString) {
	HLTInfo("received configuration object string: \'%s\'", pString->GetString().Data());
	iResult=Configure(pString->GetString().Data());
      } else {
	HLTError("configuration object \"%s\" has wrong type, required TObjString", path);
      }
    } else {
      HLTError("can not fetch object \"%s\" from CDB", path);
    }
  }
  return iResult;
}

int AliHLTEMCALClusterizerComponent::ReadPreprocessorValues(const char* modules)
{
  // see header file for class documentation
  int iResult=0;
  TString detectors(modules!=NULL?modules:"");
  HLTInfo("read preprocessor values for detector(s): %s", detectors.IsNull()?"none":detectors.Data());
  return iResult;
}
