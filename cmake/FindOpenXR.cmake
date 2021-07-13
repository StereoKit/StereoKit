# Copyright 2019-2021 Collabora, Ltd.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
# SPDX-License-Identifier: BSL-1.0
#
# Original Author:
# 2019-2021 Ryan Pavlik <ryan.pavlik@collabora.com>

#.rst:
# FindOpenXR
# ----------
#
# Find various parts of OpenXR 1.0.
#
# COMPONENTS
# ^^^^^^^^^^
#
# This module respects several COMPONENTS: ``headers``, ``loader``, ``registry``, ``specscripts``, and
# ``sdkscripts``. If no components are specified, ``headers`` and ``loader`` are assumed.
#
# Targets
# ^^^^^^^
# If the corresponding files are available and components are specified, the following imported targets
# are created:
#
# ``OpenXR::openxr_loader``
#  Link against to get the loader and headers. (Deprecated alias is ``OpenXR::Loader``)
#
# ``OpenXR::headers``
#  Link against to get only the headers. (Deprecated alias is ``OpenXR::Headers``)
#
# ``OpenXR::headers_no_prototypes``
#  Link against to get only the headers, plus the define to disable the prototypes. (Deprecated alias is ``OpenXR::HeadersNoPrototypes``)
#
# Cache variables
# ^^^^^^^^^^^^^^^
#
# The following cache variables may also be set to assist/control the operation of this module:
#
# Related to the ``OpenXR-SDK`` repo <https://github.com/KhronosGroup/OpenXR-SDK> (this is the repo with pre-built headers),
# the ``OpenXR-SDK-Source`` repo <https://github.com/KhronosGroup/OpenXR-SDK-Source>, or the internal Khronos GitLab ``openxr`` repo:
#
# ``OPENXR_SDK_SRC_DIR``
#  Path to the root of the ``openxr``, ``OpenXR-SDK``, or ``OpenXR-SDK-Source`` repo source.
# ``OPENXR_SDK_BUILD_DIR``
#  Path to the root of the build directory corresponding to the ``openxr``, ``OpenXR-SDK``, or ``OpenXR-SDK-Source`` repo.
#

# Stolen from https://github.com/KhronosGroup/OpenXR-Hpp/blob/master/cmake/FindOpenXR.cmake at 6fcea9e472622c9c7f4df0b5f0bfe7ff5d8553f7 / Jun 22, 2021

# Normalize paths
foreach(PATHVAR OPENXR_SDK_SRC_DIR OPENXR_SDK_BUILD_DIR)
    if(${${PATHVAR}})
        file(TO_CMAKE_PATH ${${PATHVAR}} ${PATHVAR})
    endif()
endforeach()

# Set up cache variables
set(OPENXR_SDK_SRC_DIR
    "${OPENXR_SDK_SRC_DIR}"
    CACHE
        PATH
        "The root of your OpenXR-SDK, OpenXR-SDK-Source, or GitLab openxr source directory - see https://github.com/KhronosGroup/OpenXR-SDK"
)
set(OPENXR_SDK_BUILD_DIR
    "${OPENXR_SDK_BUILD_DIR}"
    CACHE
        PATH
        "The root of your OpenXR-SDK, OpenXR-SDK-Source, or GitLab openxr build directory."
)

# Currently only explicitly supporting 1.0
set(OPENXR_MAJOR_VER 1)
set(OPENXR_MINOR_VER 0)
set(OPENXR_OUT_DIR ${OPENXR_MAJOR_VER}.${OPENXR_MINOR_VER})
if(WIN32)
    set(OPENXR_STATIC ON)
else()
    set(OPENXR_STATIC OFF)
endif()

find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_OPENXR QUIET openxr)
endif()

###
# Assemble lists of places to look
###
set(_oxr_include_search_dirs)
set(_oxr_loader_search_dirs)
set(_oxr_specscripts_search_dirs)
set(_oxr_sdkscripts_search_dirs)
set(_oxr_registry_search_dirs)

find_package(OpenXR CONFIG)
if(OpenXR_INCLUDE_DIR)
    list(APPEND _oxr_include_search_dirs ${OpenXR_INCLUDE_DIR})
endif()
# These are macros to avoid a new variable scope.

# Macro to extend search locations given a source dir.
macro(_oxr_handle_potential_root_src_dir _dir)
    list(APPEND _oxr_include_search_dirs "${_dir}/include"
                "${_dir}/specification/out/${OPENXR_OUT_DIR}")
    list(APPEND _oxr_registry_search_dirs "${_dir}/specification/registry/")
    list(APPEND _oxr_specscripts_search_dirs "${_dir}/specification/scripts/")
    list(APPEND _oxr_sdkscripts_search_dirs "${_dir}/src/scripts/")
endmacro()

# Macro to extend search locations given a build dir.
macro(_oxr_handle_potential_root_build_dir _dir)
    list(APPEND _oxr_include_search_dirs "${_dir}/include")
    list(APPEND _oxr_loader_search_dirs "${_dir}/src/loader")
endmacro()

if(PC_OPENXR_FOUND)
    list(APPEND _oxr_include_search_dirs ${PC_OPENXR_INCLUDE_DIRS})
    list(APPEND _oxr_loader_search_dirs ${PC_OPENXR_LIBRARY_DIRS})
endif()

set(_oxr_lib_path_suffixes)
if(WIN32)
    unset(_UWP_SUFFIX)
    if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
        set(_UWP_SUFFIX _uwp)
    endif()
    if(CMAKE_GENERATOR_PLATFORM_UPPER MATCHES "ARM.*")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_PLATFORM ARM64)
        else()
            set(_PLATFORM ARM)
        endif()
    else()
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_PLATFORM x64)
        else()
            set(_PLATFORM Win32)
        endif()
    endif()
    list(APPEND _oxr_lib_path_suffixes ${_PLATFORM}${_UWP_SUFFIX}/lib)
    # old builds
    if(MSVC_VERSION GREATER 1919)
        list(APPEND _oxr_lib_path_suffixes msvs2019_static/lib)
    endif()
endif()

# User-supplied directories
if(OPENXR_SDK_SRC_DIR)
    _oxr_handle_potential_root_src_dir(${OPENXR_SDK_SRC_DIR})
endif()
if(OPENXR_SDK_BUILD_DIR)
    _oxr_handle_potential_root_build_dir(${OPENXR_SDK_BUILD_DIR})
endif()

# Guessed build dir based on src dir
if(OPENXR_SDK_SRC_DIR)
    _oxr_handle_potential_root_build_dir("${OPENXR_SDK_SRC_DIR}/build")
endif()

# Guesses of sibling directories by name - last resort.
foreach(_dir "${CMAKE_SOURCE_DIR}/../OpenXR-SDK"
        "${CMAKE_SOURCE_DIR}/../OpenXR-SDK-Source"
        "${CMAKE_SOURCE_DIR}/../openxr")
    _oxr_handle_potential_root_src_dir(${_dir})
    _oxr_handle_potential_root_build_dir(${_dir}/build)
endforeach()

###
# Search for includes
###

# This must also contain openxr/openxr_platform.h
find_path(
    OPENXR_OPENXR_INCLUDE_DIR
    NAMES openxr/openxr.h
    PATHS ${_oxr_include_search_dirs})

find_path(
    OPENXR_PLATFORM_DEFINES_INCLUDE_DIR
    NAMES openxr/openxr_platform_defines.h
    PATHS ${_oxr_include_search_dirs})

###
# Search for other parts
###
find_library(
    OPENXR_loader_LIBRARY
    NAMES libopenxr_loader openxr_loader
          openxr_loader-${OPENXR_MAJOR_VER}_${OPENXR_MINOR_VER}
    PATHS ${_oxr_loader_search_dirs}
    PATH_SUFFIXES ${_oxr_lib_path_suffixes})

find_path(
    OPENXR_SPECSCRIPTS_DIR
    NAMES reg.py
    PATHS ${_oxr_specscripts_search_dirs})

find_path(
    OPENXR_SDKSCRIPTS_DIR
    NAMES automatic_source_generator.py
    PATHS ${_oxr_sdkscripts_search_dirs})

find_file(
    OPENXR_REGISTRY
    NAMES xr.xml
    PATHS ${_oxr_registry_search_dirs})

###
# Fix up list of requested components
###
if(NOT OpenXR_FIND_COMPONENTS)
    # Default to headers and loader
    set(OpenXR_FIND_COMPONENTS headers loader)
endif()

if("${OpenXR_FIND_COMPONENTS}" MATCHES "scripts"
   AND NOT "${OpenXR_FIND_COMPONENTS}" MATCHES "registry")
    # scripts depend on registry (mostly).
    list(APPEND OpenXR_FIND_COMPONENTS registry)
endif()

if("${OpenXR_FIND_COMPONENTS}" MATCHES "loader"
   AND NOT "${OpenXR_FIND_COMPONENTS}" MATCHES "headers")
    # loader depends on headers.
    list(APPEND OpenXR_FIND_COMPONENTS headers)
endif()

if("${OpenXR_FIND_COMPONENTS}" MATCHES "sdkscripts"
   AND NOT "${OpenXR_FIND_COMPONENTS}" MATCHES "specscripts")
    # source scripts depend on spec scripts.
    list(APPEND OpenXR_FIND_COMPONENTS specscripts)
endif()

###
# Determine if the various requested components are found.
###
set(_oxr_component_required_vars)
foreach(_comp IN LISTS OpenXR_FIND_COMPONENTS)

    if(${_comp} STREQUAL "headers")
        if(TARGET OpenXR::headers)
            set(OpenXR_headers_FOUND TRUE)
            mark_as_advanced(OPENXR_OPENXR_INCLUDE_DIR
                             OPENXR_PLATFORM_DEFINES_INCLUDE_DIR)
        else()
            list(APPEND _oxr_component_required_vars OPENXR_OPENXR_INCLUDE_DIR
                        OPENXR_PLATFORM_DEFINES_INCLUDE_DIR)
            if(EXISTS "${OPENXR_OPENXR_INCLUDE_DIR}/openxr/openxr.h"
               AND EXISTS
                   "${OPENXR_OPENXR_INCLUDE_DIR}/openxr/openxr_platform.h"
               AND EXISTS
                   "${OPENXR_OPENXR_INCLUDE_DIR}/openxr/openxr_reflection.h"
               AND EXISTS
                   "${OPENXR_PLATFORM_DEFINES_INCLUDE_DIR}/openxr/openxr_platform_defines.h"
            )
                set(OpenXR_headers_FOUND TRUE)
                mark_as_advanced(OPENXR_OPENXR_INCLUDE_DIR
                                 OPENXR_PLATFORM_DEFINES_INCLUDE_DIR)
            else()
                set(OpenXR_headers_FOUND FALSE)
            endif()
        endif()

    elseif(${_comp} STREQUAL "loader")
        if(TARGET OpenXR::openxr_loader)
            set(OpenXR_loader_FOUND TRUE)
            mark_as_advanced(OPENXR_loader_LIBRARY)
            set(OPENXR_loader_LIBRARY OpenXR::openxr_loader)
        else()
            list(APPEND _oxr_component_required_vars OPENXR_loader_LIBRARY)
            if(EXISTS "${OPENXR_loader_LIBRARY}")
                set(OpenXR_loader_FOUND TRUE)
                mark_as_advanced(OPENXR_loader_LIBRARY)
            else()
                set(OpenXR_loader_FOUND FALSE)
            endif()
        endif()

    elseif(${_comp} STREQUAL "registry")
        list(APPEND _oxr_component_required_vars OPENXR_REGISTRY)
        if(EXISTS "${OPENXR_REGISTRY}")
            set(OpenXR_registry_FOUND TRUE)
            mark_as_advanced(OPENXR_REGISTRY)
        else()
            set(OpenXR_registry_FOUND FALSE)
        endif()

    elseif(${_comp} STREQUAL "specscripts")
        list(APPEND _oxr_component_required_vars OPENXR_SPECSCRIPTS_DIR)
        if(EXISTS "${OPENXR_SPECSCRIPTS_DIR}/generator.py"
           AND EXISTS "${OPENXR_SPECSCRIPTS_DIR}/reg.py"
           AND EXISTS "${OPENXR_SPECSCRIPTS_DIR}/genxr.py")
            set(OpenXR_specscripts_FOUND TRUE)
            mark_as_advanced(OPENXR_SPECSCRIPTS_DIR)
        else()
            set(OpenXR_specscripts_FOUND FALSE)
        endif()

    elseif(${_comp} STREQUAL "sdkscripts")
        list(APPEND _oxr_component_required_vars OPENXR_SDKSCRIPTS_DIR)
        if(EXISTS "${OPENXR_SDKSCRIPTS_DIR}/automatic_source_generator.py"
           AND EXISTS "${OPENXR_SDKSCRIPTS_DIR}/src_genxr.py")
            set(OpenXR_sdkscripts_FOUND TRUE)
            mark_as_advanced(OPENXR_SDKSCRIPTS_DIR)
        else()
            set(OpenXR_sdkscripts_FOUND FALSE)
        endif()

    else()
        message(WARNING "${_comp} is not a recognized OpenXR component")
        set(OpenXR_${_comp}_FOUND FALSE)
    endif()
endforeach()
unset(_comp)

###
# FPHSA call
###
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    OpenXR
    REQUIRED_VARS
    ${_oxr_component_required_vars}
    HANDLE_COMPONENTS
    FAIL_MESSAGE
    "Could NOT find the requested OpenXR components, try setting OPENXR_SDK_SRC_DIR and/or OPENXR_SDK_BUILD_DIR"
)

###
# If found, set variables and create targets
###

# Component: headers
if(OpenXR_headers_FOUND AND NOT TARGET OpenXR::headers)
    set(OPENXR_INCLUDE_DIRS ${OPENXR_OPENXR_INCLUDE_DIR}
                            ${OPENXR_PLATFORM_DEFINES_INCLUDE_DIR})
    list(REMOVE_DUPLICATES OPENXR_INCLUDE_DIRS)

    # This target just provides the headers with prototypes.
    # You may have linker errors if you try using this
    # without linking to the loader
    add_library(OpenXR::headers INTERFACE IMPORTED)
    # This is not duplication: interface system dirs just marks as system.
    set_target_properties(
        OpenXR::headers
        PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
                   "${OPENXR_INCLUDE_DIRS}" INTERFACE_INCLUDE_DIRECTORIES
                   "${OPENXR_INCLUDE_DIRS}")
endif()

# Back-compat
if(TARGET OpenXR::headers AND NOT TARGET OpenXR::Headers)
    add_library(OpenXR::Headers INTERFACE IMPORTED)
    set_target_properties(OpenXR::Headers PROPERTIES INTERFACE_LINK_LIBRARIES
                                                     "OpenXR::headers")
endif()

# This target just provides the headers, without any prototypes.
# Finding and loading the loader at runtime is your problem.
if(TARGET OpenXR::headers AND NOT TARGET OpenXR::headers_no_prototypes)
    add_library(OpenXR::headers_no_prototypes INTERFACE IMPORTED)
    set_target_properties(
        OpenXR::headers_no_prototypes
        PROPERTIES INTERFACE_COMPILE_DEFINITIONS "XR_NO_PROTOTYPES"
                   INTERFACE_LINK_LIBRARIES "OpenXR::headers")
endif()
if(TARGET OpenXR::headers AND NOT TARGET OpenXR::HeadersNoPrototypes)
    add_library(OpenXR::HeadersNoPrototypes INTERFACE IMPORTED)
    set_target_properties(
        OpenXR::HeadersNoPrototypes
        PROPERTIES INTERFACE_COMPILE_DEFINITIONS "XR_NO_PROTOTYPES"
                   INTERFACE_LINK_LIBRARIES "OpenXR::headers")
endif()

# Component: loader
if(OpenXR_loader_FOUND
   AND OpenXR_headers_FOUND
   AND NOT TARGET OpenXR::openxr_loader)
    set(_oxr_loader_interface_libs OpenXR::headers)
    # include dl library for statically-linked loader
    get_filename_component(_oxr_loader_ext ${OPENXR_loader_LIBRARY} EXT)
    if(_oxr_loader_ext STREQUAL CMAKE_STATIC_LIBRARY_SUFFIX)
        set(_oxr_loader_lib_type STATIC)
        list(APPEND _oxr_loader_interface_libs ${CMAKE_DL_LIBS})
    else()
        set(_oxr_loader_lib_type SHARED)
    endif()

    add_library(OpenXR::openxr_loader UNKNOWN IMPORTED)
    set_target_properties(
        OpenXR::openxr_loader
        PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C" IMPORTED_LOCATION
                   "${OPENXR_loader_LIBRARY}" INTERFACE_LINK_LIBRARIES
                   "${_oxr_loader_interface_libs}")

endif()

# Back-compat
if(TARGET OpenXR::openxr_loader AND NOT TARGET OpenXR::Loader)
    add_library(OpenXR::Loader INTERFACE IMPORTED)
    set_target_properties(OpenXR::Loader PROPERTIES INTERFACE_LINK_LIBRARIES
                                                    "OpenXR::openxr_loader")
endif()
