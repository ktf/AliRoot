# - Try to find CLHEP
# Once done this will define
#
#  CLHEP_FOUND - system has CLHEP
#  CLHEP_INCLUDE_DIR - the CLHEP include directory
#  CLHEP_LIBRARIES - The libraries needed to use CLHEP
#  CLHEP_DEFINITIONS - Compiler switches required for using CLHEP
#

if (CLHEP_INCLUDE_DIR AND CLHEP_LIBRARY_DIR)
  SET (CLHEP_INCLUDE_DIR CLHEP_INCLUDE_DIR-NOTFOUND)
  SET (CLHEP_LIB_DIR CLHEP_LIB_DIR-NOTFOUND)
  SET (CLHEP_PLISTS_LIB_DIR CLHEP_PLISTS_LIB_DIR-NOTFOUND)
endif (CLHEP_INCLUDE_DIR AND CLHEP_LIBRARY_DIR)

MESSAGE(STATUS "Looking for CLHEP...")

FIND_PATH(CLHEP_INCLUDE_DIR NAMES CLHEP PATHS
  ${SIMPATH}/cern/clhep/include
  NO_DEFAULT_PATH
)

FIND_PATH(CLHEP_LIBRARY_DIR NAMES libCLHEP.so libCLHEP.dylib PATHS
  ${SIMPATH}/cern/clhep/lib
  NO_DEFAULT_PATH
)

if (CLHEP_INCLUDE_DIR AND CLHEP_LIBRARY_DIR)
   set(CLHEP_FOUND TRUE)
endif (CLHEP_INCLUDE_DIR AND CLHEP_LIBRARY_DIR)

if (CLHEP_FOUND)
  if (NOT CLHEP_FIND_QUIETLY)
    MESSAGE(STATUS "Looking for CLHEP... - found ${CLHEP_LIBRARY_DIR}")
#    message(STATUS "Found CLHEP: ${CLHEP_LIBRARY_DIR}")
    SET(LD_LIBRARY_PATH ${LD_LIBRARY_PATH} ${CLHEP_LIBRARY_DIR})
  endif (NOT CLHEP_FIND_QUIETLY)
else (CLHEP_FOUND)
  if (CLHEP_FIND_REQUIRED)
    message(FATAL_ERROR "Looking for CLHEP... - Not found")
  endif (CLHEP_FIND_REQUIRED)
endif (CLHEP_FOUND)

