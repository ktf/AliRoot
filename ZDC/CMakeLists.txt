# **************************************************************************
# * Copyright(c) 1998-2014, ALICE Experiment at CERN, All rights reserved. *
# *                                                                        *
# * Author: The ALICE Off-line Project.                                    *
# * Contributors are mentioned in the code where appropriate.              *
# *                                                                        *
# * Permission to use, copy, modify and distribute this software and its   *
# * documentation strictly for non-commercial purposes is hereby granted   *
# * without fee, provided that the above copyright notice appears in all   *
# * copies and that both the copyright notice and this permission notice   *
# * appear in the supporting documentation. The authors make no claims     *
# * about the suitability of this software for any purpose. It is          *
# * provided "as is" without express or implied warranty.                  *
# **************************************************************************

add_subdirectory(ZDCbase)
add_subdirectory(ZDCrec)
add_subdirectory(ZDCsim)

if(DA)
    add_subdirectory(DA)
endif(DA)

install(DIRECTORY macros DESTINATION ZDC)
install(FILES 
  GlauberMCDist.root
  GlauberMCHistos.root
  PMTGainsdata.txt
  SpectatorSignal.root
  SpectatorsFromData.root
  fragmSpecDataDriven.root
  ZNsignalntu.root
  ZPsignalntu.root
  light22620362207s
  light22620362208s
  light22620362209s
  light22620362210s
  light22620552207s
  light22620552208s
  light22620552209s
  light22620552210s
  DESTINATION ZDC)

message(STATUS "ZDC enabled")
