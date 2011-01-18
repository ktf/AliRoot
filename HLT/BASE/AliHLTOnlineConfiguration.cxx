// $Id$

//**************************************************************************
//* This file is property of and copyright by the ALICE HLT Project        *
//* ALICE Experiment at CERN, All rights reserved.                         *
//*                                                                        *
//* Primary Authors:                                                       *
//*                  for The ALICE HLT Project.                            *
//*                                                                        *
//* Permission to use, copy, modify and distribute this software and its   *
//* documentation strictly for non-commercial purposes is hereby granted   *
//* without fee, provided that the above copyright notice appears in all   *
//* copies and that both the copyright notice and this permission notice   *
//* appear in the supporting documentation. The authors make no claims     *
//* about the suitability of this software for any purpose. It is          *
//* provided "as is" without express or implied warranty.                  *
//**************************************************************************

///  @file   AliHLTOnlineConfiguration.h
///  @author Matthias Richter
///  @author Lars Christian Raae
///  @date   
///  @brief  Description of the HLT online configuration

#include <cerrno>
#include <fstream>
#include <iostream>
#include <cstdio>

#include <TDOMParser.h>
#include <TXMLAttr.h>
#include <TXMLNode.h>
#include <TString.h>

#include "AliHLTOnlineConfiguration.h"
#include "AliHLTComponentConfiguration.h"

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTOnlineConfiguration)

AliHLTOnlineConfiguration::AliHLTOnlineConfiguration()
  : TObject()
  , fXMLBuffer()
  , fXMLSize(0)
  , fConfEntryList()
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt

}

AliHLTOnlineConfiguration::~AliHLTOnlineConfiguration()
{
  // destructor
}

int AliHLTOnlineConfiguration::LoadConfiguration(const char* filename)
{
  /// load configuration from file

  ifstream in;
  in.open(filename);
  if (!in.is_open())
    return -EIO;

  size_t filesize = 0;
  in.seekg(0, std::ios::end );
  filesize = in.tellg();
  in.seekg(0, std::ios::beg);

  char * content = new char[filesize];
  in.read(content, filesize);
  in.close();

  fXMLBuffer.Adopt(filesize, content);
  fXMLSize = filesize;
  SetBit(kLoaded);

  return filesize;
}

int AliHLTOnlineConfiguration::Compress()
{
  /// compress the xml buffer

  return 0;
}

int AliHLTOnlineConfiguration::Uncompress()
{
  /// compress the xml buffer

  return 0;
}

int AliHLTOnlineConfiguration::Parse()
{
  /// parse the xml buffer

  int iResult = 0;
  if (TestBit(kLoaded)) {
    iResult = -EINVAL;
    TDOMParser *domParser = new TDOMParser();
    domParser->SetValidate(false);
    Int_t parsecode = domParser->ParseBuffer(fXMLBuffer.GetArray(), fXMLSize);
    if (parsecode < 0) {
       HLTError("Configuration XML invalid or not well-formed (error code %d)", parsecode);
    }
    else {
      TXMLDocument* doc;
      if ((doc = domParser->GetXMLDocument()) && doc->GetRootNode()) {
        iResult = ParseConfiguration(doc->GetRootNode());
        if (iResult == 0)
          SetBit(kParsed);
      }
    }
  }
  else
    iResult = -EPROTO; 
  return iResult;
}

int AliHLTOnlineConfiguration::ParseConfiguration(TXMLNode* node)
{
  int iResult = 0;
  int nElems = 0;
  if (node && node->GetChildren()) {
    node = node->GetChildren();
    for (; node; node = node->GetNextNode()) {
      if (node->GetNodeType() == TXMLNode::kXMLElementNode) {
        if (strcmp(node->GetNodeName(), "Proc") == 0) {
          const char* id = 0;
          if (node->HasAttributes()) {
            TList* attrList = node->GetAttributes();
            TXMLAttr* attr = 0;
            TIter next(attrList);
            while ((attr=(TXMLAttr*)next()))
              if (strcmp(attr->GetName(), "ID") == 0) {
                id = attr->GetValue();
            }
          }
          if (id && node->GetChildren()) {
            if (ParseEntry(node->GetChildren(), id) == 0) {
              nElems++;
            }
          }
          else if (!id) {
            HLTError("Configuration component missing ID attribute");
          }
          else {
            HLTError("Empty configuration component %s", id);
          }
        }
      }
    }
    if (!nElems) {
      iResult = -EINVAL;
      HLTError("Configuration did not contain any (valid) elements");
    }
  }
  else {
    iResult = -EINVAL;
    HLTError("Configuration was empty");
  }
  return iResult;
}

int AliHLTOnlineConfiguration::ParseEntry(TXMLNode* node, const char* id)
{
  int iResult = 0;
  const char* cmd = 0;
  TString sources;
  TString nodes;
  for (; node; node = node->GetNextNode()) {
    if (node->GetNodeType() == TXMLNode::kXMLElementNode) {
      if (strcmp(node->GetNodeName(), "Cmd") == 0) {
        if (!cmd) // Use only the first (non-empty) <Cmd> element
          cmd = node->GetText();
      }
      else if (strcmp(node->GetNodeName(), "Node") == 0) {
        if (!nodes.IsNull()) nodes += " "; nodes += node->GetText();
      }
      else if (strcmp(node->GetNodeName(), "Parent") == 0) {
        if (!sources.IsNull()) sources += " "; sources += node->GetText();
      }
    }
  }

  if (!cmd) {
    iResult = -EINVAL;
    HLTError("Configuration component %s does not contain <Cmd> element", id);
  }
  else if (cmd == strstr(cmd, "AliRootWrapperSubscriber"))
    iResult = ParseStandardComponent(id, cmd, sources, nodes);
  else if (cmd == strstr(cmd, "RORCPublisher"))
    iResult = ParseRORCPublisher(id, cmd, sources, nodes);
  else if (cmd == strstr(cmd, "TCPDumpSubscriber"))
    iResult = ParseTCPDumpSubscriber(id, cmd, sources, nodes);
  else if (cmd == strstr(cmd, "Relay"))
    iResult = ParseRelay(id, cmd, sources, nodes);
  else if (cmd == strstr(cmd, "HLTOutFormatter"))
    iResult = ParseHLTOutFormatter(id, cmd, sources, nodes);
  else if (cmd == strstr(cmd, "HLTOutWriterSubscriber"))
    iResult = ParseHLTOutWriterSubscriber(id, cmd, sources, nodes);
  else {
    iResult = -EINVAL;
    HLTError("Configuration component %s contains unknown <Cmd> element", id);
  }
  
  return iResult;
}

int AliHLTOnlineConfiguration::ParseStandardComponent(const char* id, const char* cmd, TString& sources, TString& nodes)
{
  int iResult = 0;
  // Parse component command
  const char* compid="";
  const char* complib="";
  char* compargs = 0;
  bool hasArgs = false;
  char* cmdcopy = new char[strlen(cmd)+1];
  strcpy(cmdcopy, cmd);
  strtok(cmdcopy, " -"); // Skip "AliRootWrapperSubscriber"
  char* strtmp = 0;
  while ((bool)(strtmp = strtok(0, " -"))) {
    if (strcmp(strtmp, "componentid") == 0)
      compid = strtok(0, " -");
    else if (strcmp(strtmp, "componentargs") == 0)
      hasArgs = true;
    else if (strcmp(strtmp, "componentlibrary") == 0)
      complib = strtok(0, " -");
  }
  if (hasArgs) {
    // Parse component arguments
    int start = strstr(cmd, "-componentargs") + strlen("-componentargs") + 2
      - cmd;
    int arglen = strcspn(cmd+start, "\"");
    // Verify that we have the actual limits of the componentargs
    if ((size_t)(start+arglen) < strlen(cmd) && cmd[start-1] == '\"' &&
      cmd[start+arglen] == '\"')
    {
      compargs = new char[arglen+1];
      strncpy(compargs, cmd + start, arglen);
      compargs[arglen] = '\0';
    }
  }
  if (!compid) {
    iResult = -EINVAL;
    HLTError("Configuration component %s is missing component id", id);
  }
  if (!complib) {
    iResult = -EINVAL;
    HLTError("Configuration component %s is missing component library", id);
  }
  if (hasArgs && !compargs) {
    iResult = -EINVAL;
    HLTError("Configuration component %s is missing component arguments", id);
  }
  if (iResult == 0) {
    AliHLTComponentConfiguration* entry = new AliHLTComponentConfiguration(id,
      compid, sources.Data(), compargs);
    entry->SetOnlineCommand(cmd);
    entry->SetComponentLibrary(complib);
    entry->SetNodeNames(nodes.Data());
    fConfEntryList.Add(entry);
  }
  return iResult;
}

int AliHLTOnlineConfiguration::ParseRORCPublisher(const char* id,
  const char* cmd, TString& sources, TString& nodes)
{
  int iResult = 0;
  const char compid[] = "RawReaderPublisher";
  const char complib[] = "libAliHLTUtil.so";
  const char compargs[] = "";
  // Parse (and validate) component command
  int ddlid;
  int res = sscanf(cmd, "RORCPublisher -slot %*d %*d %*d %*d -rorcinterface %*d -sleep "
    "-sleeptime %*d -maxpendingevents %*d -alicehlt -ddlid %d", &ddlid);
  if (res != 1) {
    iResult = -EINVAL;
    HLTError("Configuration component %s has <Cmd> element of unknown format\n"
      "Expected format: RORCPublisher -slot %%d %%d %%d %%d -rorcinterface %%d "
      "-sleep -sleeptime %%d -maxpendingevents %%d -alicehlt -ddlid %%d", id);
  }
  else {
    AliHLTComponentConfiguration* entry = new AliHLTComponentConfiguration(id,
      compid, sources.Data(), compargs);
    entry->SetOnlineCommand(cmd);
    entry->SetComponentLibrary(complib);
    entry->SetNodeNames(nodes.Data());
    fConfEntryList.Add(entry);
  }
  return iResult;
}

int AliHLTOnlineConfiguration::ParseTCPDumpSubscriber(const char* /* id */,
  const char* /* cmd */, TString& /* sources */, TString& /* nodes */)
{
  int iResult = 0;
  
  return iResult;
}

int AliHLTOnlineConfiguration::ParseRelay(const char* id, const char* cmd,
  TString& sources, TString& nodes)
{
  int iResult = 0;
  const char compid[] = "BlockFilter";
  const char complib[] = "libAliHLTUtil.so";
  const char* compargs = "";
  if (strcmp(cmd, "Relay") == 0) {
    AliHLTComponentConfiguration* entry = new AliHLTComponentConfiguration(id,
      compid, sources.Data(), compargs);
    entry->SetOnlineCommand(cmd);
    entry->SetComponentLibrary(complib);
    entry->SetNodeNames(nodes.Data());
    fConfEntryList.Add(entry);
  }
  else {
    iResult = -EINVAL;
    HLTError("Configuration component %s has <Cmd> element of unknown type "
      "\"%s\"", id, cmd);
  }
  return iResult;
}

int AliHLTOnlineConfiguration::ParseHLTOutFormatter(const char* /* id */,
  const char* /* cmd */, TString& /* sources */, TString& /* nodes */)
{
  int iResult = 0;
  
  return iResult;
}

int AliHLTOnlineConfiguration::ParseHLTOutWriterSubscriber(const char* /* id */,
  const char* /* cmd */, TString& /* sources */, TString& /* nodes */)
{
  int iResult = 0;
  
  return iResult;
}

void AliHLTOnlineConfiguration::Print(const char* options) const
{
  /// overloaded from TObject, print info
  const UInt_t defaultSampleSize = 200;

  TObject::Print(options);
  printf("Configuration loaded: %s\n", (TestBit(kLoaded) ? "YES" : "NO"));
  TString opt = options;
  opt.ToLower();
  Bool_t full = opt.Contains("full");

  if (TestBit(kLoaded)) {
    if (full) {
      char configuration[fXMLSize + 1];
      strncpy(configuration, fXMLBuffer.GetArray(), fXMLSize);
      printf("%s\n\n", configuration);
    } else {
      Int_t sampleSize = (defaultSampleSize <= fXMLSize) ?
	      defaultSampleSize : fXMLSize;
      char sample[sampleSize];
      for (int i = 0; i < sampleSize - 1; i++)
	      sample[i] = fXMLBuffer.At(i);
      sample[sampleSize - 1] = '\0';
      printf("%s...\n\n", sample);
    }
  }

  printf("XML size (uncompressed): %d\n", fXMLSize);
  printf("Configuration compressed: %s\n", (TestBit(kCompressed) ? "YES" : "NO"));
  printf("Configuration parsed: %s\n", (TestBit(kParsed) ? "YES" : "NO"));
  printf("Parsed configuration entries: %d\n", fConfEntryList.GetSize());
}

void AliHLTOnlineConfiguration::Dump() const
{
  /// overloaded from TObject, more crude data dump

  TObject::Dump();
}

void AliHLTOnlineConfiguration::Clear(Option_t * option)
{
  /// overloaded from TObject, clear object

  TObject::Clear(option);
  fConfEntryList.Clear();
  fXMLBuffer.Reset();
  fXMLSize = 0;
  ResetBit(kLoaded);
  ResetBit(kCompressed);
  ResetBit(kParsed);
}

TObject * AliHLTOnlineConfiguration::Clone(const char *newname) const
{
  /// overloaded from TObject, clone object

  return TObject::Clone(newname);
}

void AliHLTOnlineConfiguration::Copy(TObject &object) const
{
  /// overloaded from TObject, copy object

  TObject::Copy(object);
}

void AliHLTOnlineConfiguration::Draw(Option_t */*option*/)
{
  /// overloaded from TObject, draw graph of the configuration
}
