/**************************************************************************
 * Copyright(c) 2007-2009, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id: $ */

///////////////////////////////////////////////////////////////////
//                                                               //
// Implementation of the class for SDD DDL mapping in the OCDB   //
// Origin: F.Prino, Torino, prino@to.infn.it                     //
//                                                               //
///////////////////////////////////////////////////////////////////

#include "AliITSDDLModuleMapSDD.h"
#include "AliLog.h"

ClassImp(AliITSDDLModuleMapSDD)


//______________________________________________________________________
AliITSDDLModuleMapSDD::AliITSDDLModuleMapSDD():TObject(){
  // default constructor
  SetDefaultMap();
}
//______________________________________________________________________
AliITSDDLModuleMapSDD::AliITSDDLModuleMapSDD(Char_t *ddlmapfile):TObject(){
  // constructor used to read DDL map  from text file
  ReadDDLModuleMap(ddlmapfile);
}
//______________________________________________________________________
void AliITSDDLModuleMapSDD::SetDefaultMap(){
  // Fill DDL map according to the default connection scheme
  Int_t defaultmap[kDDLsNumber][kModulesPerDDL] = 
    {
 
      {240,241,242,246,247,248,252,253,254,258,259,260},
      {264,265,266,270,271,272,276,277,278,282,283,284},
      {288,289,290,294,295,296,300,301,302,306,307,308},
      {312,313,314,318,319,320,-1,-1,-1,-1,-1,-1},
      {243,244,245,249,250,251,255,256,257,261,262,263},
      {267,268,269,273,274,275,279,280,281,285,286,287},
      {291,292,293,297,298,299,303,304,305,309,310,311},
      {315,316,317,321,322,323,-1,-1,-1,-1,-1,-1},
      {324,325,326,327,332,333,334,335,340,341,342,343},
      {348,349,350,351,356,357,358,359,364,365,366,367},
      {372,373,374,375,380,381,382,383,388,389,390,391},
      {396,397,398,399,404,405,406,407,412,413,414,415},
      {420,421,422,423,428,429,430,431,436,437,438,439},
      {444,445,446,447,452,453,454,455,460,461,462,463},
      {468,469,470,471,476,477,478,479,484,485,486,487},
      {492,493,494,495,-1,-1,-1,-1,-1,-1,-1,-1},
      {328,329,330,331,336,337,338,339,344,345,346,347},
      {352,353,354,355,360,361,362,363,368,369,370,371},
      {376,377,378,379,384,385,386,387,392,393,394,395},
      {400,401,402,403,408,409,410,411,416,417,418,419},
      {424,425,426,427,432,433,434,435,440,441,442,443},
      {448,449,450,451,456,457,458,459,464,465,466,467},
      {472,473,474,475,480,481,482,483,488,489,490,491},
      {496,497,498,499,-1,-1,-1,-1,-1,-1,-1,-1}
    };

  for(Int_t iddl=0;iddl<kDDLsNumber;iddl++){
    for(Int_t ichan=0; ichan<kModulesPerDDL; ichan++){
      fDDLModuleMap[iddl][ichan]=defaultmap[iddl][ichan];
    }
  }
}

//______________________________________________________________________
void AliITSDDLModuleMapSDD::SetDec07part1Map(){
  // Fill DDL map according to dec07 cosmic run connections (run<14435)
  Int_t dec07part1map[kDDLsNumber][kModulesPerDDL] = 
    {
      {328,329,330,331,336,337,338,339,344,345,346,347}, //Crate TOP
      {352,353,354,355,-1,-1,-1,-1,-1,-1,-1,-1},
      {376,377,378,379,384,385,386,387,408,409,410,411},
      {400,401,402,403,392,393,394,395,416,417,418,419},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},   //Crate MED
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {368,369,370,371,360,361,362,363,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},   //Crate BOT
      {240,241,242,246,247,248,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,264,265,266,270,271,272},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {267,268,269,273,274,275,-1,-1,-1,-1,-1,-1},
      {243,244,245,249,250,251,255,256,257,261,262,263}
    };
  for(Int_t iddl=0;iddl<kDDLsNumber;iddl++){
    for(Int_t ichan=0; ichan<kModulesPerDDL; ichan++){
      fDDLModuleMap[iddl][ichan]=dec07part1map[iddl][ichan];
    }
  }
}


//______________________________________________________________________
void AliITSDDLModuleMapSDD::SetDec07part2Map(){
  // Fill DDL map according to dec07 cosmic run connection (run>14435)
  Int_t dec07part2map[kDDLsNumber][kModulesPerDDL] = 
    {
      {328,329,330,331,336,337,338,339,344,345,346,347}, //Crate TOP
      {352,353,354,355,360,361,362,363,368,369,370,371},
      {376,377,378,379,384,385,386,387,408,409,410,411},
      {400,401,402,403,392,393,394,395,416,417,418,419},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},   //Crate MED
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},   //Crate BOT
      {240,241,242,246,247,248,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,264,265,266,270,271,272},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {267,268,269,273,274,275,-1,-1,-1,-1,-1,-1},
      {243,244,245,249,250,251,255,256,257,261,262,263}
    };
  for(Int_t iddl=0;iddl<kDDLsNumber;iddl++){
    for(Int_t ichan=0; ichan<kModulesPerDDL; ichan++){
      fDDLModuleMap[iddl][ichan]=dec07part2map[iddl][ichan];
    }
  }
}

//______________________________________________________________________
void AliITSDDLModuleMapSDD::SetFeb08Map(){
  // Fill DDL map according to feb08 cosmic run connection 
  Int_t feb08map[kDDLsNumber][kModulesPerDDL] = 
    {
      {328,329,330,331,336,337,338,339,344,345,346,347}, //Crate TOP
      {352,353,354,355,360,361,362,363,368,369,370,371},
      {376,377,378,379,384,385,386,387,392,393,394,395},
      {400,401,402,403,408,409,410,411,416,417,418,419},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      
      {324,325,326,327,332,333,334,335,340,341,342,343},   //Crate MED
      {348,349,350,351,356,357,358,359,364,365,366,367},
      {372,373,374,375,380,381,382,383,388,389,390,391},
      {396,397,398,399,404,405,406,407,412,413,414,415},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},   //Crate BOT
      {240,241,242,246,247,248,-1,-1,-1,-1,-1,-1},
      {252,253,254,-1,-1,-1,264,265,266,270,271,272},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
      {273,274,275,267,268,269,-1,-1,-1,-1,-1,-1},
      {243,244,245,249,250,251,255,256,257,261,262,263}
    };
  for(Int_t iddl=0;iddl<kDDLsNumber;iddl++){
    for(Int_t ichan=0; ichan<kModulesPerDDL; ichan++){
      fDDLModuleMap[iddl][ichan]=feb08map[iddl][ichan];
    }
  }
}

//______________________________________________________________________
void AliITSDDLModuleMapSDD::SetDDLMap(AliITSDDLModuleMapSDD* ddlmap){
// Fill DDL map
  for(Int_t iddl=0;iddl<kDDLsNumber;iddl++){
    for(Int_t ichan=0; ichan<kModulesPerDDL; ichan++){
      fDDLModuleMap[iddl][ichan]=ddlmap->GetModuleNumber(iddl,ichan);
    }
  }
}
//______________________________________________________________________
void AliITSDDLModuleMapSDD::FindInDDLMap(Int_t modIndex, Int_t &iDDL, Int_t &iCarlos) const {
  // finds DDL and carlos number for a given module
  for(Int_t iddl=0;iddl<kDDLsNumber;iddl++){
    for(Int_t ichan=0; ichan<kModulesPerDDL; ichan++){
      if(fDDLModuleMap[iddl][ichan]==modIndex){
	iDDL=iddl;
	iCarlos=ichan;
	return;
      }
    }
  }
  AliWarning("Module not found in DDL map");
  iDDL=-1;
  iCarlos=-1;
  return;
}
//______________________________________________________________________
void AliITSDDLModuleMapSDD::ReadDDLModuleMap(Char_t *ddlmapfile){
  // Fill DDL map from ASCCI data file
  FILE *fil;
  fil=fopen(ddlmapfile,"r");  
  Int_t mod;
  for(Int_t iddl=0;iddl<kDDLsNumber;iddl++){
    for(Int_t ichan=0; ichan<kModulesPerDDL; ichan++){
      fscanf(fil,"%d",&mod);
      fDDLModuleMap[iddl][ichan]=mod;
    }
  }
  fclose(fil);
}

//______________________________________________________________________
void AliITSDDLModuleMapSDD::PrintDDLMap() const {
  // printout DDL map
  for(Int_t iddl=0;iddl<kDDLsNumber;iddl++){
    for(Int_t ichan=0; ichan<kModulesPerDDL; ichan++){
      printf("%d  ",fDDLModuleMap[iddl][ichan]);
    }
    printf("\n");
  }
}
