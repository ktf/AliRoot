#ifndef ALIHLTPHOSFILEWRITERCOMPONENT_H
#define ALIHLTPHOSFILEWRITERCOMPONENT_H

#include <string>
#include "Rtypes.h"
#include "AliHLTFileWriter.h"
#include <vector>
#include "AliHLTPHOSConstants.h"

using namespace PhosHLTConst;

class AliRawReaderMemory;
class AliCaloRawStream;
class AliHLTPHOSRcuCellEnergyDataStruct;
class AliHLTPHOSFileWriterComponent;
class AliHLTPHOSDDLPackedFileWriter;  
class AliHLTPHOSCellEnergiesFileWriter;
//class AliHLTFileWriter;

class AliHLTPHOSFileWriterComponent:public AliHLTFileWriter
{
 public:
  AliHLTPHOSFileWriterComponent();
  virtual ~AliHLTPHOSFileWriterComponent();
  int AddDataType(string dataType);
  virtual int Deinit();
  virtual int DoDeinit();
  virtual const char* GetComponentID();
  virtual void GetInputDataTypes( std::vector <AliHLTComponentDataType>&);
  virtual AliHLTComponentDataType GetOutputDataType();
  virtual void GetOutputDataSize(unsigned long& constBase, double& inputMultiplier);
  virtual AliHLTComponent* Spawn();

 protected:

 private:
  Bool_t IsRegisteredDataType(const AliHLTComponentDataType&);
  AliHLTPHOSFileWriterComponent(const AliHLTPHOSFileWriterComponent & );  
  AliHLTPHOSFileWriterComponent & operator = (const AliHLTPHOSFileWriterComponent)
    {
      return *this;
    };
  AliHLTPHOSCellEnergiesFileWriter *fCellEnergiesFileWriterPtr;   /**<to write celle energies to files*/
  AliHLTPHOSDDLPackedFileWriter    *fDDLPackedFileWriterPtr ;     /**<to write untouched DDL raw data to files*/ 
  string  fDirectory;                                             /**<target directory for files*/
  string  fFilename;                                              /**<the basename of the output file*/
  AliHLTComponentDataType fDataTypesToFile[N_DATATYPES];          /**<Array for registering datatypes that should be written to files*/          
  int fEventCount;                                                /**<Event counter*/
  static const AliHLTComponentDataType fgkInputDataTypes[];         /**<Datatypes that can be given as input to this component*/
};
#endif
