// $Id$

//**************************************************************************
//* This file is property of and copyright by the ALICE HLT Project        * 
//* ALICE Experiment at CERN, All rights reserved.                         *
//*                                                                        *
//* Primary Authors: Mikolaj Krzewicki <mikolaj.krzewicki@cern.ch>         *
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

//  @file   AliHLTGlobalPromptRecoQAComponent.cxx
//  @author Mikolaj Krzewicki, mikolaj.krzewicki@cern.ch
//  @date   
//  @brief  Simple HLT reco QA/monitor
// 

#include <cassert>
#include "AliHLTGlobalPromptRecoQAComponent.h"
#include "AliHLTGlobalBarrelTrack.h"
#include "AliHLTExternalTrackParam.h"
#include "AliHLTTrackMCLabel.h"
#include "AliHLTCTPData.h"
#include "AliHLTTPCDefinitions.h"
#include "AliHLTTPCSpacePointData.h"
#include "AliHLTTPCClusterDataFormat.h"
#include "AliTPCclusterMI.h"
#include "AliTPCseed.h"
#include "AliITStrackV2.h"
#include "AliESDfriend.h"
#include "AliESDfriendTrack.h"
#include "AliHLTTPCGeometry.h"
#include "AliHLTErrorGuard.h"
#include "AliESDEvent.h"
#include "AliESDtrack.h"
#include "AliESDMuonTrack.h"
#include "AliESDMuonCluster.h"
#include "AliCDBEntry.h"
#include "AliCDBManager.h"
#include "AliPID.h"
#include "TTree.h"
#include "TList.h"
#include "TClonesArray.h"
#include "TTimeStamp.h"
#include "THnSparse.h"
#include "AliHLTESDCaloClusterMaker.h"
#include "AliHLTCaloClusterDataStruct.h"
#include "AliHLTCaloClusterReader.h"
#include "AliESDCaloCluster.h"
#include "AliESDVZERO.h"
#include "AliHLTGlobalVertexerComponent.h"
#include "AliHLTVertexFinderBase.h"
#include "AliSysInfo.h"
#include "AliHLTSAPTrackerData.h"
#include "AliFlatESDVertex.h"

#include "TH1I.h"
#include <string>
#include "AliHLTITSClusterDataFormat.h"

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTGlobalPromptRecoQAComponent)

AliHLTGlobalPromptRecoQAComponent::AliHLTGlobalPromptRecoQAComponent()
  : AliHLTProcessor()
  , fVerbosity(0)
  , fBenchmark("PromptRecoQA")
  , fSkipEvents(0)
  , fEventsSinceSkip(0)
  , fHistSPDclusters_SPDrawSize(NULL)
  , fHistSSDclusters_SSDrawSize(NULL)
  , fHistSDDclusters_SDDrawSize(NULL)
  , fHistITSSAtracks_SPDclusters(NULL)
  , fHistSPDclusters_SSDclusters(NULL)

{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
}

AliHLTGlobalPromptRecoQAComponent::~AliHLTGlobalPromptRecoQAComponent()
{
  // see header file for class documentation
  delete fHistSPDclusters_SPDrawSize;
  delete fHistSDDclusters_SDDrawSize;
  delete fHistSSDclusters_SSDrawSize;
  delete fHistITSSAtracks_SPDclusters;
  delete fHistSPDclusters_SSDclusters;
}

int AliHLTGlobalPromptRecoQAComponent::Configure(const char* arguments)
{
  // see header file for class documentation
  int iResult=0;
  if (!arguments) return iResult;

  TString allArgs=arguments;
  TString argument;
  int bMissingParam=0;

  TObjArray* pTokens=allArgs.Tokenize(" ");
  if (pTokens) {
    for (int i=0; i<pTokens->GetEntries() && iResult>=0; i++) {
      argument=((TObjString*)pTokens->At(i))->String();	
      if (argument.IsNull()) continue;
      
      if (argument.CompareTo("-skip-events")==0) {
	      argument.ReplaceAll("-skip-events=","");
        fSkipEvents = argument.Atoi();
      }	else {
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

int AliHLTGlobalPromptRecoQAComponent::Reconfigure(const char* cdbEntry, const char* chainId)
{
  // see header file for class documentation
  int iResult=0;
  const char* path=NULL;
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
	HLTInfo("received configuration object string: \'%s\'", pString->String().Data());
	iResult=Configure(pString->String().Data());
      } else {
	HLTError("configuration object \"%s\" has wrong type, required TObjString", path);
      }
    } else {
      HLTError("can not fetch object \"%s\" from CDB", path);
    }
  }
  
  return iResult;
}

void AliHLTGlobalPromptRecoQAComponent::GetInputDataTypes(AliHLTComponentDataTypeList& list)
{
  // see header file for class documentation
  list.push_back(kAliHLTDataTypeClusters | kAliHLTDataOriginITSSPD);
  list.push_back(kAliHLTDataTypeClusters | kAliHLTDataOriginITSSDD);
  list.push_back(kAliHLTDataTypeClusters | kAliHLTDataOriginITSSSD);
  list.push_back(kAliHLTDataTypeDDLRaw | kAliHLTDataOriginITSSPD);
  list.push_back(kAliHLTDataTypeDDLRaw | kAliHLTDataOriginITSSDD);
  list.push_back(kAliHLTDataTypeDDLRaw | kAliHLTDataOriginITSSSD);
  list.push_back(kAliHLTDataTypeITSSAPData | kAliHLTDataOriginITS);
  list.push_back(AliHLTTPCDefinitions::fgkHWClustersDataType | kAliHLTDataOriginTPC);
  list.push_back(kAliHLTDataTypeDDLRaw | kAliHLTDataOriginTPC);
}

AliHLTComponentDataType AliHLTGlobalPromptRecoQAComponent::GetOutputDataType()
{
  // see header file for class documentation
  return kAliHLTMultipleDataType;
}

int AliHLTGlobalPromptRecoQAComponent::GetOutputDataTypes(AliHLTComponentDataTypeList& tgtList){ 
// see header file for class documentation

  tgtList.clear();
  tgtList.push_back( kAliHLTDataTypeHistogram|kAliHLTDataOriginOut );
  tgtList.push_back( kAliHLTDataTypeTObject|kAliHLTDataOriginOut );
  return tgtList.size();
}

void AliHLTGlobalPromptRecoQAComponent::GetOutputDataSize(unsigned long& constBase, double& inputMultiplier)
{
  // see header file for class documentation
  constBase=2000000;
  inputMultiplier=0.0;
}

int AliHLTGlobalPromptRecoQAComponent::DoInit(int argc, const char** argv)
{
  // see header file for class documentation
  int iResult=0;

  std::string argString = GetComponentArgs();
  if (Configure(argString.c_str())<0) return -EINVAL;

  fHistSPDclusters_SPDrawSize = new TH2I("SPDncls_SPDsize", "SPDncls vs SPD raw size", 50, 0., 1000., 50, 0., 10000.);
  fHistSSDclusters_SSDrawSize = new TH2I("SSDncls_SSDsize", "SSDncls vs SSD raw size", 50, 0., 1000., 50, 0., 10000.);
  fHistSDDclusters_SDDrawSize = new TH2I("SDDncls_SDDsize", "SDDncls vs SDD raw size", 50, 0., 1000., 50, 0., 10000.);
  fHistITSSAtracks_SPDclusters = new TH2I("ITSSAPntrk_SPDncls", "ITSSAP tracks vs SPD ncls", 50, 0., 1000., 50, 0., 10000.);
  fHistSPDclusters_SSDclusters = new TH2I("SPDncls_SSDncls", "SPDncls vs SSDncls", 50, 0., 1000., 50, 0., 1000.);

  return iResult;
}

int AliHLTGlobalPromptRecoQAComponent::DoDeinit()
{
  // see header file for class documentation
  return 0;
}

int AliHLTGlobalPromptRecoQAComponent::DoEvent( const AliHLTComponentEventData& evtData,
    const AliHLTComponentBlockData* blocks, 
    AliHLTComponentTriggerData& /*trigData*/,
    AliHLTUInt8_t* /*outputPtr*/, 
    AliHLTUInt32_t& /*size*/,
    AliHLTComponentBlockDataList& /*outputBlocks*/ )
{
  // see header file for class documentation
  int iResult=0;

  //perhaps downscale a bit
  if (fEventsSinceSkip++<fSkipEvents)
  {
    fEventsSinceSkip=0;
    return 0;
  }

  //what are we interested in?
  AliHLTUInt32_t nClustersSPD = 0;
  AliHLTUInt32_t rawSizeSPD = 0;
  AliHLTUInt32_t nClustersSDD = 0;
  AliHLTUInt32_t rawSizeSDD = 0;
  AliHLTUInt32_t nClustersSSD = 0;
  AliHLTUInt32_t rawSizeSSD = 0;
  AliHLTUInt32_t nClustersITS = 0;
  AliHLTUInt32_t rawSizeITS = 0;
  
  AliHLTUInt32_t nClustersTPC = 0;
  AliHLTUInt32_t rawSizeTPC = 0;
  AliHLTUInt32_t hwcfSizeTPC = 0;
  AliHLTUInt32_t compressedSizeTPC = 0;

  AliHLTUInt32_t nITSSAPtracks = 0;
  AliHLTUInt32_t nSPDtracklets =0;
  AliHLTUInt32_t nTPCtracks = 0;

  //loop over input blocks and extract basic stats
  int nBlocks = evtData.fBlockCnt;  
  for (int ndx=0; ndx<nBlocks; ndx++) {
    const AliHLTComponentBlockData* iter = blocks+ndx;

    //numbers of clusters
    if (iter->fDataType == (kAliHLTDataTypeClusters | kAliHLTDataOriginITSSPD))
    {
      AliHLTITSClusterData *inPtr=reinterpret_cast<AliHLTITSClusterData*>( iter->fPtr );
      nClustersSPD += inPtr->fSpacePointCnt;
    }
    if (iter->fDataType == (kAliHLTDataTypeClusters | kAliHLTDataOriginITSSDD))
    {
      AliHLTITSClusterData *inPtr=reinterpret_cast<AliHLTITSClusterData*>( iter->fPtr );
      nClustersSDD += inPtr->fSpacePointCnt;
    }
    if (iter->fDataType == (kAliHLTDataTypeClusters | kAliHLTDataOriginITSSSD))
    {
      AliHLTITSClusterData *inPtr=reinterpret_cast<AliHLTITSClusterData*>( iter->fPtr );
      nClustersSSD += inPtr->fSpacePointCnt;
    }
    if (iter->fDataType == (kAliHLTDataTypeClusters | kAliHLTDataOriginITS))
    {
      AliHLTITSClusterData *inPtr=reinterpret_cast<AliHLTITSClusterData*>( iter->fPtr );
      nClustersITS += inPtr->fSpacePointCnt;
    }

    //RAW sizes
    if (iter->fDataType == (kAliHLTDataTypeDDLRaw | kAliHLTDataOriginITSSPD))
    {
      rawSizeSPD += iter->fSize;
    }
    if (iter->fDataType == (kAliHLTDataTypeDDLRaw | kAliHLTDataOriginITSSDD))
    {
      rawSizeSDD += iter->fSize;
    }
    if (iter->fDataType == (kAliHLTDataTypeDDLRaw | kAliHLTDataOriginITSSSD))
    {
      rawSizeSSD += iter->fSize;
    }
    if (iter->fDataType == (kAliHLTDataTypeDDLRaw | kAliHLTDataOriginITS))
    {
      rawSizeITS += iter->fSize;
    }
    if (iter->fDataType == (AliHLTTPCDefinitions::fgkHWClustersDataType | kAliHLTDataOriginTPC))
    {
      hwcfSizeTPC += iter->fSize;
    }
    if (iter->fDataType == (kAliHLTDataTypeDDLRaw | kAliHLTDataOriginTPC))
    {
      rawSizeTPC += iter->fSize;
    }

    //numbers of tracks
    if (iter->fDataType == (kAliHLTDataTypeITSSAPData | kAliHLTDataOriginITS))
    {
      AliHLTITSSAPTrackerDataContainer* inPtr = reinterpret_cast<AliHLTITSSAPTrackerDataContainer*>(iter->fPtr);
      nITSSAPtracks += inPtr->fCount;
    }
  }// end read input blocks
  
  //fill histograms
  fHistSPDclusters_SPDrawSize->Fill(nClustersSPD, rawSizeSPD);
  if (PushBack(fHistSPDclusters_SPDrawSize, kAliHLTDataTypeHistogram|kAliHLTDataOriginOut) > 0)
    fHistSPDclusters_SPDrawSize->Reset();
  fHistSDDclusters_SDDrawSize->Fill(nClustersSDD, rawSizeSDD);
  if (PushBack(fHistSDDclusters_SDDrawSize, kAliHLTDataTypeHistogram|kAliHLTDataOriginOut) > 0)
    fHistSDDclusters_SDDrawSize->Reset();
  fHistSSDclusters_SSDrawSize->Fill(nClustersSSD, rawSizeSSD);
  if (PushBack(fHistSSDclusters_SSDrawSize, kAliHLTDataTypeHistogram|kAliHLTDataOriginOut) > 0)
    fHistSSDclusters_SSDrawSize->Reset();
  fHistITSSAtracks_SPDclusters->Fill(nITSSAPtracks, nClustersSPD);
  if (PushBack(fHistITSSAtracks_SPDclusters, kAliHLTDataTypeHistogram|kAliHLTDataOriginOut) > 0)
    fHistITSSAtracks_SPDclusters->Reset();
  fHistSPDclusters_SSDclusters->Fill(nClustersSPD, nClustersSSD);
  if (PushBack(fHistSPDclusters_SSDclusters, kAliHLTDataTypeHistogram|kAliHLTDataOriginOut) > 0)
    fHistSPDclusters_SSDclusters->Reset();

  return iResult;
}
