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

add_subdirectory(TRDbase)
add_subdirectory(TRDcalib)
add_subdirectory(TRDgui)
add_subdirectory(TRDrec)
add_subdirectory(TRDsim)
add_subdirectory(TRDqaAnalysis)

if(DA)
    add_subdirectory(DA)
endif(DA)

# Install macros
install(DIRECTORY macros data DESTINATION TRD)

# Install doc
install(DIRECTORY doc DESTINATION TRD)

message(STATUS "TRD enabled")
