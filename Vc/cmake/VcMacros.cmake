# Macros for use with the Vc library. Vc can be found at http://code.compeng.uni-frankfurt.de/projects/vc
#
# The following macros are provided:
# vc_determine_compiler
# vc_set_preferred_compiler_flags
#
#=============================================================================
# Copyright 2009-2012   Matthias Kretz <kretz@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file CmakeCopyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include ("${_currentDir}/UserWarning.cmake")
include ("${_currentDir}/AddCompilerFlag.cmake")
include ("${_currentDir}/OptimizeForArchitecture.cmake")

macro(vc_determine_compiler)
   if(NOT DEFINED Vc_COMPILER_IS_INTEL)
      set(Vc_COMPILER_IS_INTEL false)
      set(Vc_COMPILER_IS_OPEN64 false)
      set(Vc_COMPILER_IS_CLANG false)
      set(Vc_COMPILER_IS_MSVC false)
      set(Vc_COMPILER_IS_GCC false)
      if(CMAKE_CXX_COMPILER MATCHES "/(icpc|icc)$")
         set(Vc_COMPILER_IS_INTEL true)
         exec_program(${CMAKE_C_COMPILER} ARGS -dumpversion OUTPUT_VARIABLE Vc_ICC_VERSION)
         message(STATUS "Detected Compiler: Intel ${Vc_ICC_VERSION}")
      elseif(CMAKE_CXX_COMPILER MATCHES "/(opencc|openCC)$")
         set(Vc_COMPILER_IS_OPEN64 true)
         message(STATUS "Detected Compiler: Open64")
      elseif(CMAKE_CXX_COMPILER MATCHES "/clang\\+\\+$")
         set(Vc_COMPILER_IS_CLANG true)
         message(STATUS "Detected Compiler: Clang")
      elseif(MSVC)
         set(Vc_COMPILER_IS_MSVC true)
         message(STATUS "Detected Compiler: MSVC")
      elseif(CMAKE_COMPILER_IS_GNUCXX)
         set(Vc_COMPILER_IS_GCC true)
         exec_program(${CMAKE_C_COMPILER} ARGS -dumpversion OUTPUT_VARIABLE Vc_GCC_VERSION)
         message(STATUS "Detected Compiler: GCC ${Vc_GCC_VERSION}")

         # some distributions patch their GCC to return nothing or only major and minor version on -dumpversion.
         # In that case we must extract the version number from --version.
         if(NOT Vc_GCC_VERSION OR Vc_GCC_VERSION MATCHES "^[0-9]\\.[0-9]+$")
            exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE Vc_GCC_VERSION)
            string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" Vc_GCC_VERSION "${Vc_GCC_VERSION}")
            message(STATUS "GCC Version from --version: ${Vc_GCC_VERSION}")
         endif()

         # some distributions patch their GCC to be API incompatible to what the FSF released. In
         # those cases we require a macro to identify the distribution version
         find_program(_lsb_release lsb_release)
         mark_as_advanced(_lsb_release)
         if(_lsb_release)
            execute_process(COMMAND ${_lsb_release} -is OUTPUT_VARIABLE _distributor_id OUTPUT_STRIP_TRAILING_WHITESPACE)
            execute_process(COMMAND ${_lsb_release} -rs OUTPUT_VARIABLE _distributor_release OUTPUT_STRIP_TRAILING_WHITESPACE)
            string(TOUPPER "${_distributor_id}" _distributor_id)
            if(_distributor_id STREQUAL "UBUNTU")
               execute_process(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE _gcc_version)
               string(REGEX MATCH "\\(.* ${Vc_GCC_VERSION}-([0-9]+).*\\)" _tmp "${_gcc_version}")
               if(_tmp)
                  set(_patch ${CMAKE_MATCH_1})
                  string(REGEX MATCH "^([0-9]+)\\.([0-9]+)$" _tmp "${_distributor_release}")
                  execute_process(COMMAND printf 0x%x%02x%02x ${CMAKE_MATCH_1} ${CMAKE_MATCH_2} ${_patch} OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE _tmp)
                  set(Vc_DEFINITIONS "${Vc_DEFINITIONS} -D__GNUC_UBUNTU_VERSION__=${_tmp}")
               endif()
            endif()
         endif()
      else()
         message(WARNING "Untested/-supported Compiler for use with Vc.\nPlease fill out the missing parts in the CMake scripts and submit a patch to http://code.compeng.uni-frankfurt.de/projects/vc")
      endif()
   endif()
endmacro()

macro(vc_set_gnu_buildtype_flags)
   set(CMAKE_CXX_FLAGS_DEBUG          "-g3"          CACHE STRING "Flags used by the compiler during debug builds." FORCE)
   set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG" CACHE STRING "Flags used by the compiler during release minsize builds." FORCE)
   set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG" CACHE STRING "Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files)." FORCE)
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g" CACHE STRING "Flags used by the compiler during Release with Debug Info builds." FORCE)
   set(CMAKE_C_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG}"          CACHE STRING "Flags used by the compiler during debug builds." FORCE)
   set(CMAKE_C_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}"     CACHE STRING "Flags used by the compiler during release minsize builds." FORCE)
   set(CMAKE_C_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}"        CACHE STRING "Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files)." FORCE)
   set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" CACHE STRING "Flags used by the compiler during Release with Debug Info builds." FORCE)
   if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      set(ENABLE_STRICT_ALIASING true CACHE BOOL "Enables strict aliasing rules for more aggressive optimizations")
      if(NOT ENABLE_STRICT_ALIASING)
         set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -fno-strict-aliasing ")
         set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-strict-aliasing ")
         set(CMAKE_C_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE} -fno-strict-aliasing ")
         set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-strict-aliasing ")
      endif(NOT ENABLE_STRICT_ALIASING)
   endif()
endmacro()

macro(vc_add_compiler_flag VAR _flag)
   AddCompilerFlag("${_flag}" CXX_FLAGS ${VAR})
endmacro()

macro(vc_check_assembler)
   if(APPLE)
      if(NOT Vc_COMPILER_IS_CLANG)
         message(WARNING "Apple does not provide an assembler with AVX support. AVX will not be available. Please use Clang if you want to use AVX.")
         set(Vc_DEFINITIONS "${Vc_DEFINITIONS} -DVC_NO_XGETBV")
         set(Vc_AVX_INTRINSICS_BROKEN true)
      endif()
   else(APPLE)
      if(${ARGC} EQUAL 1)
         set(_as "${ARGV1}")
      else()
         exec_program(${CMAKE_CXX_COMPILER} ARGS -print-prog-name=as OUTPUT_VARIABLE _as)
         mark_as_advanced(_as)
      endif()
      if(NOT _as)
         message(WARNING "Could not find 'as', the assembler used by GCC. Hoping everything will work out...")
      else()
         exec_program(${_as} ARGS --version OUTPUT_VARIABLE _as_version)
         string(REGEX REPLACE "\\([^\\)]*\\)" "" _as_version "${_as_version}")
         string(REGEX MATCH "[1-9]\\.[0-9]+(\\.[0-9]+)?" _as_version "${_as_version}")
         if(_as_version VERSION_LESS "2.18.93")
            message(WARNING "Your binutils is too old (${_as_version}). Some optimizations of Vc will be disabled.")
            add_definitions(-DVC_NO_XGETBV) # old assembler doesn't know the xgetbv instruction
         endif()
      endif()
   endif(APPLE)
endmacro()

macro(vc_check_fpmath)
   # if compiling for 32 bit x86 we need to use the -mfpmath=sse since the x87 is broken by design
   include (CheckCXXSourceRuns)
   check_cxx_source_runs("int main() { return sizeof(void*) != 8; }" Vc_VOID_PTR_IS_64BIT)
   if(NOT Vc_VOID_PTR_IS_64BIT)
      exec_program(${CMAKE_C_COMPILER} ARGS -dumpmachine OUTPUT_VARIABLE _gcc_machine)
      if(_gcc_machine MATCHES "[x34567]86")
         vc_add_compiler_flag(Vc_DEFINITIONS "-mfpmath=sse")
      endif(_gcc_machine MATCHES "[x34567]86")
   endif()
endmacro()

macro(vc_set_preferred_compiler_flags)
   vc_determine_compiler()

   set(_add_warning_flags false)
   set(_add_buildtype_flags false)
   foreach(_arg ${ARGN})
      if(_arg STREQUAL "WARNING_FLAGS")
         set(_add_warning_flags true)
      elseif(_arg STREQUAL "BUILDTYPE_FLAGS")
         set(_add_buildtype_flags true)
      endif()
   endforeach()

   set(Vc_SSE_INTRINSICS_BROKEN false)
   set(Vc_AVX_INTRINSICS_BROKEN false)

   if(Vc_COMPILER_IS_OPEN64)
      ##################################################################################################
      #                                             Open64                                             #
      ##################################################################################################
      if(_add_warning_flags)
         AddCompilerFlag("-W")
         AddCompilerFlag("-Wall")
         AddCompilerFlag("-Wimplicit")
         AddCompilerFlag("-Wswitch")
         AddCompilerFlag("-Wformat")
         AddCompilerFlag("-Wchar-subscripts")
         AddCompilerFlag("-Wparentheses")
         AddCompilerFlag("-Wmultichar")
         AddCompilerFlag("-Wtrigraphs")
         AddCompilerFlag("-Wpointer-arith")
         AddCompilerFlag("-Wcast-align")
         AddCompilerFlag("-Wreturn-type")
         AddCompilerFlag("-Wno-unused-function")
         AddCompilerFlag("-ansi")
         AddCompilerFlag("-pedantic")
         AddCompilerFlag("-Wno-long-long")
         AddCompilerFlag("-Wshadow")
         AddCompilerFlag("-Wold-style-cast")
         AddCompilerFlag("-Wno-variadic-macros")
      endif()
      if(_add_buildtype_flags)
         vc_set_gnu_buildtype_flags()
      endif()

      vc_check_assembler()
   elseif(Vc_COMPILER_IS_GCC)
      ##################################################################################################
      #                                              GCC                                               #
      ##################################################################################################
      if(_add_warning_flags)
         set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wswitch -Wformat -Wchar-subscripts -Wparentheses -Wmultichar -Wtrigraphs -Wpointer-arith -Wcast-align -Wreturn-type -Wno-unused-function -ansi -pedantic -Wno-long-long -Wshadow")
         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wswitch -Wformat -Wchar-subscripts -Wparentheses -Wmultichar -Wtrigraphs -Wpointer-arith -Wcast-align -Wreturn-type -Wno-unused-function -ansi -pedantic -Wno-long-long -Wshadow")
         AddCompilerFlag("-Wimplicit")
         AddCompilerFlag("-Wold-style-cast")
         AddCompilerFlag("-Wno-variadic-macros")
         if(Vc_GCC_VERSION VERSION_GREATER "4.5.2" AND Vc_GCC_VERSION VERSION_LESS "4.6.4")
            # GCC gives bogus "array subscript is above array bounds" warnings in math.cpp
            AddCompilerFlag("-Wno-array-bounds")
         endif()
      endif()
      vc_add_compiler_flag(Vc_DEFINITIONS "-Wabi")
      vc_add_compiler_flag(Vc_DEFINITIONS "-fabi-version=0") # ABI version 4 is required to make __m128 and __m256 appear as different types. 0 should give us the latest version.

      if(_add_buildtype_flags)
         vc_set_gnu_buildtype_flags()
      endif()

      # GCC 4.5.[01] fail at inlining some functions, creating functions with a single instructions,
      # thus creating a large overhead.
      if(Vc_GCC_VERSION VERSION_LESS "4.5.2" AND NOT Vc_GCC_VERSION VERSION_LESS "4.5.0")
         UserWarning("GCC 4.5.0 and 4.5.1 have problems with inlining correctly. Setting early-inlining-insns=12 as workaround.")
         AddCompilerFlag("--param early-inlining-insns=12")
      endif()

      if(Vc_GCC_VERSION VERSION_LESS "4.4.6")
         UserWarning("Your GCC is older than 4.4.6. This is known to cause problems/bugs. Please update to the latest GCC if you can.")
         set(Vc_AVX_INTRINSICS_BROKEN true)
         if(Vc_GCC_VERSION VERSION_LESS "4.3.0")
            UserWarning("Your GCC is older than 4.3.0. It is unable to handle the full set of SSE2 intrinsics. All SSE code will be disabled. Please update to the latest GCC if you can.")
            set(Vc_SSE_INTRINSICS_BROKEN true)
         endif()
      endif()

      if(Vc_GCC_VERSION VERSION_LESS 4.5.0)
         UserWarning("GCC 4.4.x shows false positives for -Wparentheses, thus we rather disable the warning.")
         string(REPLACE " -Wparentheses " " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
         string(REPLACE " -Wparentheses " " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
         set(Vc_DEFINITIONS "${Vc_DEFINITIONS} -Wno-parentheses")
      elseif(Vc_GCC_VERSION VERSION_EQUAL 4.7.0)
         UserWarning("GCC 4.7.0 miscompiles at -O3, adding -fno-predictive-commoning to the compiler flags as workaround")
         set(Vc_DEFINITIONS "${Vc_DEFINITIONS} -fno-predictive-commoning")
      endif()

      vc_check_fpmath()
      vc_check_assembler()
   elseif(Vc_COMPILER_IS_INTEL)
      ##################################################################################################
      #                                          Intel Compiler                                        #
      ##################################################################################################

      if(_add_buildtype_flags)
         set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -O3")
         set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DNDEBUG -O3")
         set(CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE} -O3")
         set(CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DNDEBUG -O3")

         set(ALIAS_FLAGS "-no-ansi-alias")
         if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
            # default ICC to -no-ansi-alias because otherwise tests/utils_sse fails. So far I suspect a miscompilation...
            set(ENABLE_STRICT_ALIASING false CACHE BOOL "Enables strict aliasing rules for more aggressive optimizations")
            if(ENABLE_STRICT_ALIASING)
               set(ALIAS_FLAGS "-ansi-alias")
            endif(ENABLE_STRICT_ALIASING)
         endif()
         set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${ALIAS_FLAGS}")
         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ALIAS_FLAGS}")
      endif()
      vc_add_compiler_flag(Vc_DEFINITIONS "-diag-disable 913")
   elseif(Vc_COMPILER_IS_MSVC)
      if(_add_warning_flags)
         AddCompilerFlag("/wd4800") # Disable warning "forcing value to bool"
         AddCompilerFlag("/wd4996") # Disable warning about strdup vs. _strdup
         AddCompilerFlag("/wd4244") # Disable warning "conversion from 'unsigned int' to 'float', possible loss of data"
         AddCompilerFlag("/wd4146") # Disable warning "unary minus operator applied to unsigned type, result still unsigned"
         AddCompilerFlag("/wd4227") # Disable warning "anachronism used : qualifiers on reference are ignored" (this is about 'restrict' usage on references, stupid MSVC)
         AddCompilerFlag("/wd4722") # Disable warning "destructor never returns, potential memory leak" (warns about ~_UnitTest_Global_Object which we don't care about)
         AddCompilerFlag("/wd4748") # Disable warning "/GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function" (I don't get it)
         add_definitions(-D_CRT_SECURE_NO_WARNINGS)
      endif()

      # MSVC does not support inline assembly on 64 bit! :(
      # searching the help for xgetbv doesn't turn up anything. So just fall back to not supporting AVX on Windows :(
      # TODO: apparently MSVC 2010 SP1 added _xgetbv
      set(Vc_DEFINITIONS "${Vc_DEFINITIONS} -DVC_NO_XGETBV")

      # get rid of the min/max macros
      set(Vc_DEFINITIONS "${Vc_DEFINITIONS} -DNOMINMAX")
   elseif(Vc_COMPILER_IS_CLANG)
      # for now I don't know of any arguments I want to pass. -march and stuff is tried by OptimizeForArchitecture...

      # disable these warnings because clang shows them for function overloads that were discarded via SFINAE
      vc_add_compiler_flag(Vc_DEFINITIONS "-Wno-local-type-template-args")
      vc_add_compiler_flag(Vc_DEFINITIONS "-Wno-unnamed-type-template-args")
   endif()

   OptimizeForArchitecture()
   set(Vc_DEFINITIONS "${Vc_ARCHITECTURE_FLAGS} ${Vc_DEFINITIONS}")

   set(VC_IMPL "auto" CACHE STRING "Force the Vc implementation globally to the selected instruction set. \"auto\" lets Vc use the best available instructions.")
   if(NOT VC_IMPL STREQUAL "auto")
      set(Vc_DEFINITIONS "${Vc_DEFINITIONS} -DVC_IMPL=${VC_IMPL}")
      if(NOT VC_IMPL STREQUAL "Scalar")
         set(_use_var "USE_${VC_IMPL}")
         if(VC_IMPL STREQUAL "SSE")
            set(_use_var "USE_SSE2")
         endif()
         if(NOT ${_use_var})
            message(WARNING "The selected value for VC_IMPL (${VC_IMPL}) will not work because the relevant instructions are not enabled via compiler flags.")
         endif()
      endif()
   endif()
endmacro()
