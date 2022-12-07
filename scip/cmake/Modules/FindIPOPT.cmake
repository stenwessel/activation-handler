#.rst:
# FindIPOPT
# ---------
#
# Try to locate the IPOPT library
#
# On non Windows systems, use pkg-config to try to locate the library,
# if this fails then try to locate the library in the directory pointed by
# the IPOPT_DIR enviromental variable.
#
# On Windows systems,  just try to find the library using the IPOPT_DIR
# enviromental variable.
#
# Create the following variables::
#
#  IPOPT_INCLUDE_DIRS - Directories to include to use IPOPT
#  IPOPT_LIBRARIES    - Default library to link against to use IPOPT
#  IPOPT_FOUND        - If false, don't try to use IPOPT

#=============================================================================
# Copyright (C) 2008-2010 RobotCub Consortium
# Copyright (C) 2016 iCub Facility - Istituto Italiano di Tecnologia
#   Authors: Ugo Pattacini <ugo.pattacini@iit.it>
#   Authors: Daniele E. Domenichelli <daniele.domenichelli@iit.it>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of YCM, substitute the full
#  License text for the above reference.)

if(NOT WIN32)
  # On non Windows systems we use PkgConfig to find IPOPT
  find_package(PkgConfig QUIET)

  if(NOT IPOPT_DIR)
    set(IPOPT_DIR_TEST $ENV{IPOPT_DIR})
    if(IPOPT_DIR_TEST)
      set(IPOPT_DIR $ENV{IPOPT_DIR} CACHE PATH "Path to IPOPT build directory")
    else()
      set(IPOPT_DIR /usr            CACHE PATH "Path to IPOPT build directory")
    endif()
  endif()

  # try to find dep file, if yes, ipopt <= 3.12, don't use package-config
  # if not, ipopt >= 3.13, use package-config
  find_file(IPOPT_DEP_FILE ipopt_addlibs_cpp.txt ${IPOPT_DIR}/share/doc/coin-or/Ipopt
                                                 ${IPOPT_DIR}/share/coin-or/doc/Ipopt
                                                 ${IPOPT_DIR}/share/doc/coin/Ipopt
                                                 ${IPOPT_DIR}/share/coin/doc/Ipopt
                                                 NO_DEFAULT_PATH)

  if(PKG_CONFIG_FOUND AND NOT EXISTS ${IPOPT_DEP_FILE})
    if(IPOPT_DIR)
      set(ENV{PKG_CONFIG_PATH} "${IPOPT_DIR}/lib/pkgconfig/:$ENV{PKG_CONFIG_PATH}")
    endif()

    if(IPOPT_FIND_VERSION)
      if(IPOPT_FIND_VERSION_EXACT)
        pkg_check_modules(_PC_IPOPT QUIET IMPORTED_TARGET ipopt=${IPOPT_FIND_VERSION})
      else()
        pkg_check_modules(_PC_IPOPT QUIET IMPORTED_TARGET ipopt>=${IPOPT_FIND_VERSION})
      endif()
    else()
      pkg_check_modules(_PC_IPOPT QUIET IMPORTED_TARGET ipopt)
    endif()
  endif()

  if(_PC_IPOPT_FOUND)
    set(IPOPT_INCLUDE_DIRS ${_PC_IPOPT_INCLUDE_DIRS} CACHE PATH "IPOPT include directory")
    set(IPOPT_LIBRARIES PkgConfig::_PC_IPOPT CACHE STRING "IPOPT libraries" FORCE)
  else()
  # If pkg-config fails or hasn't been tried, try to find the package using IPOPT_DIR

    set(IPOPT_INCLUDE_DIRS ${IPOPT_DIR}/include/coin-or)

    if(NOT EXISTS "${IPOPT_INCLUDE_DIRS}")
      # # version ipopt <= 3.12
        set(IPOPT_INCLUDE_DIRS ${IPOPT_DIR}/include/coin)
    endif()

    find_library(IPOPT_LIBRARIES ipopt ${IPOPT_DIR}/lib
                                     ${IPOPT_DIR}/lib/coin
                                     ${IPOPT_DIR}/lib/coin-or
                                     NO_DEFAULT_PATH)

    if(IPOPT_LIBRARIES)
      if(IPOPT_DEP_FILE)
        # add libraries from ipopt_addlibs_cpp.txt
        file(READ ${IPOPT_DEP_FILE} IPOPT_DEP)
        string(STRIP ${IPOPT_DEP} IPOPT_DEP)
        set(IPOPT_LIBRARIES ${IPOPT_LIBRARIES} ${IPOPT_DEP})
      endif()
    endif()

  endif()

# Windows platforms
else()
  include(SelectLibraryConfigurations)

  set(IPOPT_DIR $ENV{IPOPT_DIR} CACHE PATH "Path to IPOPT build directory")

  # version ipopt >= 3.13
  set(IPOPT_INCLUDE_DIRS ${IPOPT_DIR}/include/coin-or)

  if(NOT EXISTS "${IPOPT_INCLUDE_DIRS}")
      # version ipopt <= 3.12
      set(IPOPT_INCLUDE_DIRS ${IPOPT_DIR}/include/coin)
  endif()

  find_library(IPOPT_IPOPT_LIBRARY_RELEASE libipopt ${IPOPT_DIR}/lib
                                                    ${IPOPT_DIR}/lib/coin
                                                    ${IPOPT_DIR}/lib/coin-or
                                                    NO_DEFAULT_PATH)
  find_library(IPOPT_IPOPT_LIBRARY_DEBUG   libipoptD ${IPOPT_DIR}/lib
                                                     ${IPOPT_DIR}/lib/coin
                                                     ${IPOPT_DIR}/lib/coin-or
                                                     NO_DEFAULT_PATH)

  select_library_configurations(IPOPT_IPOPT)
  set(IPOPT_LIBRARIES ${IPOPT_IPOPT_LIBRARY})

  if(IPOPT_LIBRARIES)
    find_file(IPOPT_DEP_FILE ipopt_addlibs_cpp.txt ${IPOPT_DIR}/share/doc/coin-or/Ipopt
                                                   ${IPOPT_DIR}/share/coin-or/doc/Ipopt
                                                   ${IPOPT_DIR}/share/doc/coin/Ipopt
                                                   ${IPOPT_DIR}/share/coin/doc/Ipopt
                                                   NO_DEFAULT_PATH)
    mark_as_advanced(IPOPT_DEP_FILE)

    if(IPOPT_DEP_FILE)
      # parse the file and acquire the dependencies
      file(READ ${IPOPT_DEP_FILE} IPOPT_DEP)

      string(REGEX REPLACE "-[^l][^ ]* " "" IPOPT_DEP ${IPOPT_DEP})
      string(REPLACE "\n"                "" IPOPT_DEP ${IPOPT_DEP})
      string(REPLACE "\n"                "" IPOPT_DEP ${IPOPT_DEP})
      string(REPLACE "ipopt"             "" IPOPT_DEP ${IPOPT_DEP})       # remove any possible auto-dependency
      separate_arguments(IPOPT_DEP)

      # use the find_library command in order to prepare rpath correctly
      foreach(LIB ${IPOPT_DEP})

        # skip LD library flags (this can be either -libflags or -l)
        if(${LIB} MATCHES "-l*")
          continue()
        endif()

        # check whether we compile for x86 or x64
        if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
          set(MKL_ARCH_DIR "intel64")
        else()
          set(MKL_ARCH_DIR "ia32")
        endif()

        find_library(IPOPT_SEARCH_FOR_${LIB} ${LIB} $ENV{MKLROOT}/lib/${MKL_ARCH_DIR}
                                                    ${IPOPT_DIR}/lib
                                                    ${IPOPT_DIR}/lib/coin-or
                                                    ${IPOPT_DIR}/lib/coin-or/ThirdParty
                                                    ${IPOPT_DIR}/lib/coin
                                                    ${IPOPT_DIR}/lib/coin/ThirdParty
                                                    NO_DEFAULT_PATH)

        if(IPOPT_SEARCH_FOR_${LIB})
          set(IPOPT_LIBRARIES ${IPOPT_LIBRARIES} ${IPOPT_SEARCH_FOR_${LIB}})
        endif()
        mark_as_advanced(IPOPT_SEARCH_FOR_${LIB})
      endforeach()
    endif()
  endif()

endif()

# parse the version number
foreach( INCLUDE_DIR ${IPOPT_INCLUDE_DIRS} )
  if( EXISTS ${INCLUDE_DIR}/IpoptConfig.h )
  file(STRINGS ${INCLUDE_DIR}/IpoptConfig.h CONFIGFILE)
    foreach(STR ${CONFIGFILE})
      if("${STR}" MATCHES "^#define IPOPT_VERSION ")
        string(REGEX REPLACE "#define IPOPT_VERSION " "" IPOPT_VERSION ${STR})
        string(REGEX REPLACE "\"" "" IPOPT_VERSION ${IPOPT_VERSION})
      endif()
    endforeach()
  endif()
endforeach()

mark_as_advanced(IPOPT_INCLUDE_DIRS
                 IPOPT_LIBRARIES
                 IPOPT_MODERN)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IPOPT
  FOUND_VAR IPOPT_FOUND
  REQUIRED_VARS IPOPT_LIBRARIES
  VERSION_VAR IPOPT_VERSION)
