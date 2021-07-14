# Copyright 2021, Collabora, Ltd.
# Copyright Moses Turner, 2021.
# Many thanks to Ryan Pavlik for showing the way
# SPDX-License-Identifier: MIT

# This build file has the following options
# - LINUX_EGL
#     This tells the Linux build to use EGL as the OpenGL loader, 
#     instead of the default GLX.
#

cmake_minimum_required(VERSION 3.10)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(StereoKit VERSION "0.3.2" LANGUAGES CXX C)

set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git" AND NOT DEFINED ENV{CI})
  set(default_build_type "Debug")
endif()

###########################################
## Options                               ##
###########################################

SET(LINUX_EGL OFF CACHE BOOL "Force using EGL graphics backend on Linux")

###########################################
## Dependencies                          ##
###########################################

# See: https://github.com/cpm-cmake/CPM.cmake
include("cmake/CPM.cmake")

CPMAddPackage(
  NAME reactphysics3d
  GITHUB_REPOSITORY DanielChappuis/reactphysics3d
  GIT_TAG bdc3153f552f6665ea9d2d13afd20ea871119e17 # v0.8.0
)
set_property(TARGET reactphysics3d PROPERTY POSITION_INDEPENDENT_CODE ON)

# Linux users/devs prefer shared system libraries rather than
# static libraries. OpenXR runtimes are responsible for providing
# the OpenXR loader there. On Windows and Android, static linking
# is necessary.
if (WIN32)
  CPMAddPackage(
    NAME OpenXR
    GITHUB_REPOSITORY KhronosGroup/OpenXR-SDK
    GIT_TAG bf21ccb1007bb531b45d9978919a56ea5059c245 # v1.0.17
  )
endif()

###########################################
## Platform Specific Library Linking     ##
###########################################

if (UNIX)
  set(LINUX_LIBS
    Threads::Threads
    GLX
    X11
    fontconfig )

  # Add pthreads
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)

  # Add OpenGL
  if(LINUX_EGL)
    add_definitions("-DSKG_LINUX_EGL")
    list(APPEND LINUX_LIBS EGL)
  else()
    add_definitions("-DSKG_LINUX_GLX")
    find_package(GLEW REQUIRED)
    find_package(X11 REQUIRED)
    list(APPEND LINUX_LIBS GL GLEW)
  endif()
elseif(WIN32)
  set(WINDOWS_LIBS
    WindowsApp) # Required for ISAC spatial audio code
endif()

###########################################
## StereoKitC                            ##
###########################################

add_library(StereoKitC
  SHARED
  StereoKitC/sk_math.cpp
  StereoKitC/tools/file_picker.cpp
  StereoKitC/color.cpp
  StereoKitC/systems/defaults.cpp
  StereoKitC/systems/system.cpp
  StereoKitC/systems/render.cpp
  StereoKitC/systems/sprite_drawer.cpp
  StereoKitC/systems/world.cpp
  StereoKitC/systems/input_keyboard.cpp
  StereoKitC/systems/physics.cpp
  StereoKitC/systems/line_drawer.cpp
  StereoKitC/systems/platform/linux.cpp
  StereoKitC/systems/platform/platform_utils.cpp
  StereoKitC/systems/platform/openxr_input.cpp
  StereoKitC/systems/platform/openxr.cpp
  StereoKitC/systems/platform/platform.cpp
  StereoKitC/systems/platform/win32.cpp
  StereoKitC/systems/platform/flatscreen_input.cpp
  StereoKitC/systems/platform/openxr_view.cpp
  StereoKitC/systems/platform/android.cpp
  StereoKitC/systems/platform/uwp.cpp
  StereoKitC/systems/text.cpp
  StereoKitC/systems/input.cpp
  StereoKitC/systems/audio.cpp
  StereoKitC/systems/hand/hand_override.cpp
  StereoKitC/systems/hand/hand_oxr_articulated.cpp
  StereoKitC/systems/hand/hand_oxr_controller.cpp
  StereoKitC/systems/hand/hand_mouse.cpp
  StereoKitC/systems/hand/input_hand.cpp
  StereoKitC/systems/render_sort.cpp
  StereoKitC/spherical_harmonics.cpp
  StereoKitC/sk_memory.cpp
  StereoKitC/asset_types/material.cpp
  StereoKitC/asset_types/font.cpp
  StereoKitC/asset_types/mesh.cpp
  StereoKitC/asset_types/sound.cpp
  StereoKitC/asset_types/sprite.cpp
  StereoKitC/asset_types/assets.cpp
  StereoKitC/asset_types/model_gltf.cpp
  StereoKitC/asset_types/shader.cpp
  StereoKitC/asset_types/model_obj.cpp
  StereoKitC/asset_types/model.cpp
  StereoKitC/asset_types/model_fbx.cpp
  StereoKitC/asset_types/model_stl.cpp
  StereoKitC/asset_types/texture.cpp
  StereoKitC/asset_types/model_ply.cpp
  StereoKitC/stereokit.cpp
  StereoKitC/libraries/sk_gpu.cpp
  StereoKitC/libraries/sokol_time.cpp
  StereoKitC/libraries/isac_spatial_sound.cpp
  StereoKitC/libraries/tinycthread.cpp
  StereoKitC/libraries/ofbx.cpp
  StereoKitC/libraries/miniz.cpp
  StereoKitC/libraries/ferr_hash.cpp
  StereoKitC/libraries/miniaudio.cpp
  StereoKitC/libraries/stref.cpp
  StereoKitC/intersect.cpp
  StereoKitC/hierarchy.cpp
  StereoKitC/log.cpp
  StereoKitC/stereokit_ui.cpp
) # Probably some of these can be PRIVATE, might make the debug symbols smaller for example.

target_include_directories(StereoKitC
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/StereoKitC>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/StereoKitC/
    # openxr_loader
    # ${OPENXR_INCLUDE_DIRS}  
    StereoKitC/lib/include_no_win/
    StereoKitC/lib/include/
)

target_link_libraries( StereoKitC
  PRIVATE
    reactphysics3d
  PUBLIC
    openxr_loader
    ${LINUX_LIBS}
    ${WINDOWS_LIBS}
    ${CMAKE_DL_LIBS}
)

###########################################
## StereoKitCTest                        ##
###########################################

add_executable( StereoKitCTest
  Examples/StereoKitCTest/main.cpp
  Examples/StereoKitCTest/demo_basics.h
  Examples/StereoKitCTest/demo_basics.cpp
  Examples/StereoKitCTest/demo_picker.h
  Examples/StereoKitCTest/demo_picker.cpp
  Examples/StereoKitCTest/demo_mic.h
  Examples/StereoKitCTest/demo_mic.cpp
  Examples/StereoKitCTest/demo_sprites.h
  Examples/StereoKitCTest/demo_sprites.cpp
  Examples/StereoKitCTest/scene.h
  Examples/StereoKitCTest/scene.cpp
  Examples/StereoKitCTest/demo_ui.h
  Examples/StereoKitCTest/demo_ui.cpp
  Examples/StereoKitCTest/demo_world.h
  Examples/StereoKitCTest/demo_world.cpp
  Examples/StereoKitCTest/demo_lines.h
  Examples/StereoKitCTest/demo_lines.cpp
)

target_link_libraries( StereoKitCTest
  StereoKitC
)

###########################################
## Installation                          ##
###########################################

# For installing on Linux
# find_package(PkgConfig)

install(
  TARGETS StereoKitC 
  EXPORT  StereoKitC-targets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(
  FILES       "StereoKitC/stereokit.h" "StereoKitC/stereokit_ui.h" 
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include) 

install(
  EXPORT      StereoKitC-targets
  FILE        StereoKitCConfig.cmake
  NAMESPACE   StereoKitC::
  DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/StereoKitC"
)
