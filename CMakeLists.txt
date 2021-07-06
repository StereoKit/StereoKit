# Copyright 2021, Collabora, Ltd.
# Copyright Moses Turner, 2021.
# Many thanks to Ryan Pavlik for showing the way
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.10)

set(CMAKE_VERBOSE_MAKEFILE ON)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.297.tar.gz"
    SHA1 "3319fe6a3b08090df7df98dee75134d68e2ef5a3"
    LOCAL # needed to make cmake/Hunter/config.cmake magically work
)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(StereoKit VERSION "0.3.2" LANGUAGES CXX C)

set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git" AND NOT DEFINED ENV{CI})
    set(default_build_type "Debug")
endif()

# Hunter packages. Look in cmake/Hunter/config.cmake
hunter_add_package(ReactPhysics3D)
find_package(ReactPhysics3D CONFIG REQUIRED)

# Stolen from Monado
add_library(sk-pthreads INTERFACE)
if(WIN32)
	find_package(pthreads_windows REQUIRED)
	target_link_libraries(sk-pthreads INTERFACE PThreads4W::PThreads4W_CXXEXC)
else()
	set(CMAKE_THREAD_PREFER_PTHREAD ON)
	find_package(Threads)
	target_link_libraries(sk-pthreads INTERFACE Threads::Threads)
endif()

SET(LINUX_EGL OFF CACHE BOOL "Force using EGL graphics backend on Linux")

# Linux system packages
if(NOT LINUX_EGL)
	# Linux OpenGL function pointer loader thing - on Windows, SK has its own function pointer loader thing
	find_package(GLEW REQUIRED)

	# "Old" Linux window manager
	find_package(X11 REQUIRED)
endif()

# OpenXR
# find_package(OpenXR REQUIRED)

# For installing on Linux
# find_package(PkgConfig)

if(LINUX_EGL)
	add_definitions("-DSKG_LINUX_EGL")
else()
	add_definitions("-DSKG_LINUX_GLX")
endif()

add_library(StereoKitC
SHARED
./StereoKitC/sk_math.cpp
./StereoKitC/tools/file_picker.cpp
./StereoKitC/color.cpp
./StereoKitC/systems/defaults.cpp
./StereoKitC/systems/system.cpp
./StereoKitC/systems/render.cpp
./StereoKitC/systems/sprite_drawer.cpp
./StereoKitC/systems/world.cpp
./StereoKitC/systems/input_keyboard.cpp
./StereoKitC/systems/physics.cpp
./StereoKitC/systems/line_drawer.cpp
./StereoKitC/systems/platform/linux.cpp
./StereoKitC/systems/platform/platform_utils.cpp
./StereoKitC/systems/platform/openxr_input.cpp
./StereoKitC/systems/platform/openxr.cpp
./StereoKitC/systems/platform/platform.cpp
./StereoKitC/systems/platform/win32.cpp
./StereoKitC/systems/platform/flatscreen_input.cpp
./StereoKitC/systems/platform/openxr_view.cpp
./StereoKitC/systems/platform/android.cpp
./StereoKitC/systems/platform/uwp.cpp
./StereoKitC/systems/text.cpp
./StereoKitC/systems/input.cpp
./StereoKitC/systems/audio.cpp
./StereoKitC/systems/hand/hand_override.cpp
./StereoKitC/systems/hand/hand_oxr_articulated.cpp
./StereoKitC/systems/hand/hand_oxr_controller.cpp
./StereoKitC/systems/hand/hand_mouse.cpp
./StereoKitC/systems/hand/input_hand.cpp
./StereoKitC/systems/render_sort.cpp
./StereoKitC/spherical_harmonics.cpp
./StereoKitC/sk_memory.cpp
./StereoKitC/asset_types/material.cpp
./StereoKitC/asset_types/font.cpp
./StereoKitC/asset_types/mesh.cpp
./StereoKitC/asset_types/sound.cpp
./StereoKitC/asset_types/sprite.cpp
./StereoKitC/asset_types/assets.cpp
./StereoKitC/asset_types/model_gltf.cpp
./StereoKitC/asset_types/shader.cpp
./StereoKitC/asset_types/model_obj.cpp
./StereoKitC/asset_types/model.cpp
./StereoKitC/asset_types/model_fbx.cpp
./StereoKitC/asset_types/model_stl.cpp
./StereoKitC/asset_types/texture.cpp
./StereoKitC/asset_types/model_ply.cpp
./StereoKitC/stereokit.cpp
./StereoKitC/libraries/sk_gpu.cpp
./StereoKitC/libraries/sokol_time.cpp
./StereoKitC/libraries/isac_spatial_sound.cpp
./StereoKitC/libraries/tinycthread.cpp
./StereoKitC/libraries/ofbx.cpp
./StereoKitC/libraries/miniz.cpp
./StereoKitC/libraries/ferr_hash.cpp
./StereoKitC/libraries/miniaudio.cpp
./StereoKitC/libraries/stref.cpp
./StereoKitC/intersect.cpp
./StereoKitC/hierarchy.cpp
./StereoKitC/log.cpp
./StereoKitC/stereokit_ui.cpp
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

if(LINUX_EGL)
	target_link_libraries(
		StereoKitC
		PUBLIC
		EGL
	)
else()
	target_link_libraries(
		StereoKitC
		PUBLIC
		GL
		GLEW
		X11
	)
endif()

target_link_libraries(
  StereoKitC
  # PUBLIC
  # openxr_loader
  # PRIVATE
  PUBLIC
  ReactPhysics3D::ReactPhysics3D
  GLX
  fontconfig
  openxr_loader
  ${CMAKE_DL_LIBS}
  PRIVATE
  sk-pthreads
)

add_executable(StereoKitCTest
Examples/StereoKitCTest/main.cpp
Examples/StereoKitCTest/demo_picker.cpp
Examples/StereoKitCTest/demo_mic.cpp
Examples/StereoKitCTest/demo_sprites.cpp
Examples/StereoKitCTest/scene.cpp
Examples/StereoKitCTest/demo_ui.cpp
Examples/StereoKitCTest/demo_basics.cpp
Examples/StereoKitCTest/demo_lines.cpp
)

target_include_directories(
  StereoKitCTest
  PRIVATE
  Examples/StereoKitCTest/
)
target_link_libraries(
  StereoKitCTest
  StereoKitC
)


install(
  TARGETS StereoKitC 
  EXPORT StereoKitC-targets
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(FILES "StereoKitC/stereokit.h" "StereoKitC/stereokit_ui.h" DESTINATION ${CMAKE_INSTALL_PREFIX}/include) 

install(EXPORT StereoKitC-targets
    FILE
        StereoKitCConfig.cmake
    NAMESPACE
    StereoKitC::
    DESTINATION
        "${CMAKE_INSTALL_PREFIX}/lib/cmake/StereoKitC"
)
