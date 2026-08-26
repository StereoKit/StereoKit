# SPDX-License-Identifier: MIT
# Copyright 2021, Collabora, Ltd.
# Copyright Moses Turner, 2021.
# Copyright (c) 2021-2025 Nick Klingensmith
# Copyright (c) 2023-2025 Qualcomm Technologies, Inc.
# Many thanks to Ryan Pavlik for showing the way

cmake_minimum_required(VERSION 3.19)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

project(StereoKit VERSION "0.4.0" LANGUAGES CXX C)

# Add this repository's cmake modules to CMAKE_MODULE_PATH
# Ensure our cmake/ folder is in path first to override FindMetalTools (skip Metal on macOS)
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

###########################################
## Options                               ##
###########################################

option(SK_BUILD_OPENXR_LOADER          "Build the openxr_loader target from StereoKit" ON)
option(SK_MULTITHREAD_BUILD_BY_DEFAULT "MSVC only, on by default. This forces projects here to build with multi-threading (/MP)" ON)
option(SK_BUILD_TESTS                  "Build the StereoKitCTest project in addition to the StereoKitC library." ON)
option(SK_BUILD_SHARED_LIBS            "Should StereoKit build as a shared, or static library?" ON)
option(SK_DYNAMIC_OPENXR               "Dynamic link with the standard OpenXR Loader. Not what you want on desktop, but on Android you may need to dynamic link with other loaders." OFF)
option(SK_LOCAL_SK_RENDERER            "Build using a local version of sk_renderer, instead of the latest release. This is useful for testing in-progress changes to sk_renderer." OFF)
option(SK_LOCAL_SK_APP                 "Build using a local version of sk_app, instead of the latest release. This is useful for testing in-progress changes to sk_app." OFF)
option(SK_PROFILE                      "Build with Tracy profiler enabled." OFF)
option(SK_STRIP_DEBUG_SYMBOLS          "Strip debug symbols from the shared library. Disable on Android to keep symbols embedded for native debugging." ON)
option(SK_USE_SVSL                     "Compile shaders with skshaderc's SVSL backend instead of the glslang HLSL pipeline." OFF)
option(FORCE_COLORED_OUTPUT            "Always produce ANSI-colored output (GNU/Clang only)." OFF)

# Distribute copies feed C# builds and NuGet packing, only meaningful when
# working in the StereoKit repo itself. Consumers pulling StereoKit in via
# CMake keep their checkout untouched.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(SK_DISTRIBUTE_DEFAULT ON)
else()
  set(SK_DISTRIBUTE_DEFAULT OFF)
endif()
option(SK_DISTRIBUTE "Copy StereoKit bin & tool to the StereoKit source folder" ${SK_DISTRIBUTE_DEFAULT})

###########################################
## Configuration settings                ##
###########################################

# Default build configuration to Release
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Only export functions that are explicitly marked for export
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Dependency code isn't ours to fix, so compile it with warnings disabled.
set(SK_WARNING_FLAG $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w> $<$<CXX_COMPILER_ID:MSVC>:/W0>)

###########################################
## Source Link                           ##
###########################################

if (MSVC)
  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    if ("${CMAKE_C_COMPILER_VERSION}" VERSION_GREATER_EQUAL "19.20")
      include(SourceLink)
      file(TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/source_link.json" SOURCE_LINK_JSON)
      source_link(${PROJECT_SOURCE_DIR} ${SOURCE_LINK_JSON})
      add_link_options("/SOURCELINK:${SOURCE_LINK_JSON}")
    else()
      message(WARNING "Disabling SourceLink due to old version of MSVC. Please update to VS2019!")
    endif()
  else()
      message(WARNING "No SourceLink as we aren't in a source project!")
  endif()
endif()

###########################################
## Pretty colors                         ##
###########################################

if(FORCE_COLORED_OUTPUT)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-fdiagnostics-color=always)
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # Clang and AppleClang
		add_compile_options(-fcolor-diagnostics)
	endif()
endif()

# Hide shader compile metadata unless we're in verbose mode
set(SK_SHADER_VERBOSITY "-si")
if(CMAKE_VERBOSE_MAKEFILE)
  set(SK_SHADER_VERBOSITY "")
endif()

set(SK_SHADER_SVSL "")
if(SK_USE_SVSL)
  set(SK_SHADER_SVSL "-svsl")
endif()

###########################################
## Platform Specific Settings            ##
###########################################

# Desktop Unix platforms (Linux/macOS) link pthreads explicitly
if (UNIX AND NOT ANDROID)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
endif()

if (ANDROID)
  set(ANDROID_LIBS
    android)
  # Align to 16kb as per Android recommendations
  # https://developer.android.com/guide/practices/page-sizes
  set(SK_LINKER_FLAGS "-Wl,-z,max-page-size=16384")
  if(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
    add_compile_options(-mavx -mf16c)
  endif()
elseif (APPLE)
  # macOS-specific settings
  set(MACOS_LIBS
    Threads::Threads
    "-framework Cocoa"
    "-framework CoreText"
    "-framework IOKit" )

  # Apple Silicon uses arm64, Intel Macs use x86_64
  if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    add_compile_options(-mavx -mf16c)
  endif()

elseif (UNIX)
  # Linux-specific settings
  set(LINUX_LIBS
    Threads::Threads
    X11
    Xfixes
    fontconfig )

  if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    add_compile_options(-mavx -mf16c)
  endif()

elseif(WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  if (MSVC)
    set(WINDOWS_LIBS
    Comdlg32)
  else()
    # MinGW/GCC cross-compile settings
    # GCC spills 256-bit ymm with 32-byte-aligned vmovdqa, but only ever gets the
    # 16 bytes Win64 guarantees, so keep it on 128-bit vectors. See GCC #54412.
    add_compile_options(-mavx -mf16c -mprefer-vector-width=128)
    # Static runtime linking is applied per-target to StereoKitC only (see below)
    # Executables use the runtime from the DLL to avoid duplicate symbol errors
  endif()
endif()

if (NOT MSVC)
  # SAL macros (__in, __out, etc.) from include_no_win/sal.h conflict with
  # C++ standard library internal identifiers. This disables them.
  add_compile_definitions(PAL_STDCPP_COMPAT)
endif()

###########################################
## Dependencies                          ##
###########################################

# All dependencies build with PIC so they can fold into StereoKitC when it's a
# shared library. StereoKitC itself picks PIC based on SK_BUILD_SHARED_LIBS,
# down where the target is created.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# See: https://github.com/cpm-cmake/CPM.cmake
include("cmake/CPM.cmake")

#### OpenXR - https://www.khronos.org/openxr/ ####
if (SK_BUILD_OPENXR_LOADER)
  CPMAddPackage(
    NAME openxr_loader
    GITHUB_REPOSITORY KhronosGroup/OpenXR-SDK
    GIT_TAG release-1.1.60
    GIT_SHALLOW 1
    OPTIONS
    "BUILD_ALL_EXTENSIONS OFF" # Only need Vulkan, not Metal/OpenGL
    "BUILD_WITH_SYSTEM_JSONCPP OFF"
    "BUILD_WITH_XCB_HEADERS OFF"
    "DYNAMIC_LOADER ${SK_DYNAMIC_OPENXR}"
  )
  target_compile_options(openxr_loader PRIVATE ${SK_WARNING_FLAG})
  
  # OpenXR-SDK pins the loader to the dynamic CRT (/MD) when it's built as a
  # static lib, which collides (LNK2038) with our static-CRT build. Re-assert
  # our runtime on the target so both sides match.
  if (CMAKE_MSVC_RUNTIME_LIBRARY)
    set_target_properties(openxr_loader PROPERTIES
      MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}")
  endif()

  set(OPENXR_LIB openxr_loader)
endif()

#### meshoptimizer - https://github.com/zeux/meshoptimizer ####
CPMAddPackage(
  NAME meshoptimizer
  GITHUB_REPOSITORY zeux/meshoptimizer
  GIT_TAG           v0.20
  GIT_SHALLOW       1
)
target_compile_options(meshoptimizer PRIVATE ${SK_WARNING_FLAG})

#### zstd - https://github.com/facebook/zstd ####
# sk_ktx2 vendors no compression code, so Zstd supercompressed UASTC borrows this
# from us. STATIC rather than OBJECT so --exclude-libs below keeps ZSTD_* private.
add_library(zstd_decompress STATIC StereoKitC/libraries/zstddeclib.c)
target_compile_options(zstd_decompress PRIVATE ${SK_WARNING_FLAG})

#### Tracy Profiler - https://github.com/wolfpld/tracy ####

if (SK_PROFILE)
  CPMAddPackage(
    NAME tracy
    GITHUB_REPOSITORY wolfpld/tracy
    GIT_TAG a5e5e8a435daf36f9213d40e7e97efc6cbb13610
    GIT_SHALLOW 1
    OPTIONS
      "TRACY_ENABLE ON"
      "TRACY_ON_DEMAND ON"
  )
  # Directory-scoped so sk_renderer/sk_app, added below, pick it up as well
  add_compile_definitions(TRACY_ENABLE)
endif()

#### sk_renderer - https://github.com/StereoKit/sk_renderer ####
set(SKR_ENABLE_STATIC_ANALYSIS OFF CACHE BOOL "Faster build" FORCE)
set(SKR_BUILD_EXAMPLES OFF CACHE BOOL "Faster build" FORCE)
set(SKR_BUILD_KTX2 ON CACHE BOOL "KTX2 texture loading" FORCE)
# If we're only using the SVSL shader compiler, we can drop glslang from
# skshaderc and save ourselves a lot of binary size and build time!
if (SK_USE_SVSL)
  set(SKSHADERC_ENABLE_GLSLANG OFF CACHE BOOL "Not needed with SVSL")
else()
  set(SKSHADERC_ENABLE_GLSLANG ON  CACHE BOOL "Required for HLSL shaders")
endif()
if (NOT SK_LOCAL_SK_RENDERER)
  CPMAddPackage(
    NAME sk_renderer
    GITHUB_REPOSITORY StereoKit/sk_renderer
    GIT_TAG v2026.8.26
  )
else()
  # For building directly with in-progress sk_renderer changes, point this to
  # your local sk_renderer clone, and use it instead of CPM. Its build folder
  # lives inside this preset's own binary dir; a shared location gets
  # cross-contaminated when presets with different toolchains take turns.
  add_subdirectory(../sk_renderer ${CMAKE_BINARY_DIR}/local_deps/sk_renderer)
endif()
# Enable Win32 Vulkan surface support for Windows builds (needed by volk)
if(WIN32)
  target_compile_definitions(volk PUBLIC VK_USE_PLATFORM_WIN32_KHR)
endif()

#### sk_app - https://github.com/StereoKit/sk_app ####
set(SKA_BUILD_EXAMPLES OFF CACHE BOOL "Faster build" FORCE)
if (NOT SK_LOCAL_SK_APP)
  CPMAddPackage(
    NAME sk_app
    GITHUB_REPOSITORY StereoKit/sk_app
    GIT_TAG v2026.8.26
  )
else()
  # For building directly with in-progress sk_app changes, point this to your
  # local sk_app clone, and use it instead of CPM. See the sk_renderer note on
  # the build folder location.
  add_subdirectory(../sk_app ${CMAKE_BINARY_DIR}/local_deps/sk_app)
endif()


###########################################
## StereoKitC                            ##
###########################################

# Switch between a static or shared library based on
# the cmake options provided.
if (SK_BUILD_SHARED_LIBS)
  set(LIBRARY_TYPE SHARED)
else()
  set(LIBRARY_TYPE STATIC)
endif()
# Dependencies above always build PIC; StereoKitC and the executables after it
# only need it when building the shared library.
set(CMAKE_POSITION_INDEPENDENT_CODE ${SK_BUILD_SHARED_LIBS})

add_library(StereoKitC ${LIBRARY_TYPE})

# stereokit.h keys SK_API's MSVC dllexport off this define. PRIVATE: consumers
# compiling against the headers must not export the API from their own binary.
if (SK_BUILD_SHARED_LIBS)
  target_compile_definitions(StereoKitC PRIVATE SK_BUILD_SHARED)
else()
  target_compile_definitions(StereoKitC PRIVATE SK_BUILD_STATIC)
endif()
if (ANDROID AND SK_DYNAMIC_OPENXR)
  target_compile_definitions(StereoKitC PRIVATE SK_DYNAMIC_OPENXR)
endif()

# Core
target_sources(StereoKitC PRIVATE
  StereoKitC/stereokit.h

  StereoKitC/_stereokit.h
  StereoKitC/_stereokit_ui.h
  StereoKitC/color.cpp
  StereoKitC/backend.cpp
  StereoKitC/device.cpp
  StereoKitC/device.h
  StereoKitC/hierarchy.h
  StereoKitC/hierarchy.cpp
  StereoKitC/log.h
  StereoKitC/log.cpp
  StereoKitC/rect_atlas.h
  StereoKitC/rect_atlas.cpp
  StereoKitC/sk_math.h
  StereoKitC/sk_math_dx.h
  StereoKitC/sk_math.cpp
  StereoKitC/sk_memory.h
  StereoKitC/sk_memory.cpp
  StereoKitC/spherical_harmonics.h
  StereoKitC/spherical_harmonics.cpp
  StereoKitC/stereokit.cpp )

# Asset types
target_sources(StereoKitC PRIVATE
  StereoKitC/asset_types/anchor.h
  StereoKitC/asset_types/anchor.cpp
  StereoKitC/asset_types/assets.h
  StereoKitC/asset_types/assets.cpp
  StereoKitC/asset_types/animation.h
  StereoKitC/asset_types/animation.cpp
  StereoKitC/asset_types/compute.h
  StereoKitC/asset_types/compute.cpp
  StereoKitC/asset_types/compute_buffer.h
  StereoKitC/asset_types/compute_buffer.cpp
  StereoKitC/asset_types/font.h
  StereoKitC/asset_types/font.cpp
  StereoKitC/asset_types/material.h
  StereoKitC/asset_types/material.cpp
  StereoKitC/asset_types/mesh.h
  StereoKitC/asset_types/mesh_.h
  StereoKitC/asset_types/mesh.cpp
  StereoKitC/asset_types/model.h
  StereoKitC/asset_types/model.cpp
  StereoKitC/asset_types/model_gltf.cpp
  StereoKitC/asset_types/model_obj.cpp
  StereoKitC/asset_types/model_ply.cpp
  StereoKitC/asset_types/model_stl.cpp
  StereoKitC/asset_types/shader.h
  StereoKitC/asset_types/shader.cpp
  StereoKitC/asset_types/sound.h
  StereoKitC/asset_types/sound.cpp
  StereoKitC/asset_types/sprite.h
  StereoKitC/asset_types/sprite.cpp
  StereoKitC/asset_types/texture.h
  StereoKitC/asset_types/texture_.h
  StereoKitC/asset_types/texture.cpp
  StereoKitC/asset_types/texture_compression.h
  StereoKitC/asset_types/texture_compression.cpp )

# Vendored libraries
target_sources(StereoKitC PRIVATE
  StereoKitC/libraries/aileron_font_data.h
  StereoKitC/libraries/default_controller_l.h
  StereoKitC/libraries/default_controller_r.h
  StereoKitC/libraries/atomic_util.h
  StereoKitC/libraries/array.h
  StereoKitC/libraries/cgltf.h
  StereoKitC/libraries/cgltf.cpp
  StereoKitC/libraries/ferr_hash.h
  StereoKitC/libraries/ferr_hash.cpp
  StereoKitC/libraries/gen_ids.h
  StereoKitC/libraries/micro_ply.h
  StereoKitC/libraries/miniaudio.h
  StereoKitC/libraries/miniaudio.cpp
  StereoKitC/libraries/qoi.h
  StereoKitC/libraries/qoi.cpp
  StereoKitC/libraries/sk_fontfile.h
  StereoKitC/libraries/sk_fontfile.cpp
  StereoKitC/libraries/sokol_time.h
  StereoKitC/libraries/sokol_time.cpp
  StereoKitC/libraries/stb_image.h
  StereoKitC/libraries/stb_image_write.h
  StereoKitC/libraries/stb_truetype.h
  StereoKitC/libraries/stb.cpp
  StereoKitC/libraries/stref.h
  StereoKitC/libraries/stref.cpp
  StereoKitC/libraries/unicode.h
  StereoKitC/libraries/unicode.cpp
  StereoKitC/libraries/ferr_thread.h
  StereoKitC/libraries/ferr_thread.cpp )

# Utils
target_sources(StereoKitC PRIVATE
  StereoKitC/utils/sdf.h
  StereoKitC/utils/sdf.cpp
  StereoKitC/utils/random.h
  StereoKitC/utils/random.cpp)

# Systems
target_sources(StereoKitC PRIVATE
  StereoKitC/systems/_stereokit_systems.h
  StereoKitC/systems/_stereokit_systems.cpp
  StereoKitC/systems/audio.h
  StereoKitC/systems/audio.cpp
  StereoKitC/systems/audio_mix.cpp
  StereoKitC/systems/bbox.h
  StereoKitC/systems/bbox.cpp
  StereoKitC/systems/bvh.h
  StereoKitC/systems/bvh.cpp  
  StereoKitC/systems/defaults.h
  StereoKitC/systems/defaults.cpp
  StereoKitC/systems/input.h
  StereoKitC/systems/input.cpp
  StereoKitC/systems/input_keyboard.h
  StereoKitC/systems/input_keyboard.cpp
  StereoKitC/systems/sensor.h
  StereoKitC/systems/sensor.cpp
  StereoKitC/systems/sensor_depth.h
  StereoKitC/systems/sensor_depth.cpp
  StereoKitC/systems/input_render.h
  StereoKitC/systems/input_render.cpp
  StereoKitC/systems/line_drawer.h
  StereoKitC/systems/line_drawer.cpp
  StereoKitC/systems/permission.cpp
  StereoKitC/systems/permission.h
  StereoKitC/systems/render.h
  StereoKitC/systems/render_.h
  StereoKitC/systems/render.cpp
  StereoKitC/systems/render_pipeline.h
  StereoKitC/systems/render_pipeline.cpp
  StereoKitC/systems/lighting.h
  StereoKitC/systems/lighting.cpp
  StereoKitC/systems/sprite_drawer.h
  StereoKitC/systems/sprite_drawer.cpp
  StereoKitC/systems/system.h
  StereoKitC/systems/system.cpp
  StereoKitC/systems/text.h
  StereoKitC/systems/text.cpp
  StereoKitC/systems/vert_format.h
  StereoKitC/systems/vert_format.cpp
  StereoKitC/systems/world.h
  StereoKitC/systems/world.cpp )

# Hands
target_sources(StereoKitC PRIVATE
  StereoKitC/hands/hand_mouse.h
  StereoKitC/hands/hand_mouse.cpp
  StereoKitC/hands/hand_override.h
  StereoKitC/hands/hand_override.cpp
  StereoKitC/hands/hand_oxr_controller.h
  StereoKitC/hands/hand_oxr_controller.cpp
  StereoKitC/hands/hand_poses.h
  StereoKitC/hands/input_hand.h
  StereoKitC/hands/input_hand.cpp )

# Platforms
target_sources(StereoKitC PRIVATE
  StereoKitC/platforms/android.cpp
  StereoKitC/platforms/android.h
  StereoKitC/platforms/linux.h
  StereoKitC/platforms/linux.cpp
  StereoKitC/platforms/macos.h
  StereoKitC/platforms/macos.cpp
  StereoKitC/platforms/platform.h
  StereoKitC/platforms/_platform.h
  StereoKitC/platforms/platform.cpp
  StereoKitC/platforms/platform_common_win.cpp
  StereoKitC/platforms/win32.h
  StereoKitC/platforms/win32.cpp )

# XR backends
target_sources(StereoKitC PRIVATE
  StereoKitC/xr_backends/openxr.h
  StereoKitC/xr_backends/openxr.cpp
  StereoKitC/xr_backends/openxr_platform.h
  StereoKitC/xr_backends/openxr_input.h
  StereoKitC/xr_backends/openxr_input.cpp
  StereoKitC/xr_backends/openxr_view.h
  StereoKitC/xr_backends/openxr_view.cpp
  StereoKitC/xr_backends/extensions/ext_management.h
  StereoKitC/xr_backends/extensions/ext_management.cpp
  StereoKitC/xr_backends/extensions/_registration.h
  StereoKitC/xr_backends/extensions/_registration.cpp
  StereoKitC/xr_backends/xr.h
  StereoKitC/xr_backends/xr.cpp
  StereoKitC/xr_backends/simulator.h
  StereoKitC/xr_backends/simulator.cpp
  StereoKitC/xr_backends/window.h
  StereoKitC/xr_backends/window.cpp
  StereoKitC/xr_backends/offscreen.h
  StereoKitC/xr_backends/offscreen.cpp
  StereoKitC/xr_backends/anchor_stage.h
  StereoKitC/xr_backends/anchor_stage.cpp )

# OpenXR extensions
target_sources(StereoKitC PRIVATE
  StereoKitC/xr_backends/extensions/composition_depth.h
  StereoKitC/xr_backends/extensions/composition_depth.cpp
  StereoKitC/xr_backends/extensions/view_config_views_change.h
  StereoKitC/xr_backends/extensions/view_config_views_change.cpp
  StereoKitC/xr_backends/extensions/meta_detached_controllers.h
  StereoKitC/xr_backends/extensions/meta_detached_controllers.cpp
  StereoKitC/xr_backends/extensions/meta_environment_depth.h
  StereoKitC/xr_backends/extensions/meta_environment_depth.cpp
  StereoKitC/xr_backends/extensions/debug_utils.h
  StereoKitC/xr_backends/extensions/debug_utils.cpp
  StereoKitC/xr_backends/extensions/ext_interaction_render_model.h
  StereoKitC/xr_backends/extensions/ext_interaction_render_model.cpp
  StereoKitC/xr_backends/extensions/ext_render_model.h
  StereoKitC/xr_backends/extensions/ext_render_model.cpp
  StereoKitC/xr_backends/extensions/eye_interaction.h
  StereoKitC/xr_backends/extensions/eye_interaction.cpp
  StereoKitC/xr_backends/extensions/fb_colorspace.h
  StereoKitC/xr_backends/extensions/fb_colorspace.cpp
  StereoKitC/xr_backends/extensions/fb_haptic.h
  StereoKitC/xr_backends/extensions/fb_haptic.cpp
  StereoKitC/xr_backends/extensions/fb_render_model.h
  StereoKitC/xr_backends/extensions/fb_render_model.cpp
  StereoKitC/xr_backends/extensions/vulkan_enable.h
  StereoKitC/xr_backends/extensions/vulkan_enable.cpp
  StereoKitC/xr_backends/extensions/hand_mesh.h
  StereoKitC/xr_backends/extensions/hand_mesh.cpp
  StereoKitC/xr_backends/extensions/hand_tracking.h
  StereoKitC/xr_backends/extensions/hand_tracking.cpp
  StereoKitC/xr_backends/extensions/input_profiles.h
  StereoKitC/xr_backends/extensions/input_profiles.cpp
  StereoKitC/xr_backends/extensions/light_estimation.h
  StereoKitC/xr_backends/extensions/light_estimation.cpp
  StereoKitC/xr_backends/extensions/loader_init.h
  StereoKitC/xr_backends/extensions/loader_init.cpp
  StereoKitC/xr_backends/extensions/palm_pose.h
  StereoKitC/xr_backends/extensions/palm_pose.cpp
  StereoKitC/xr_backends/extensions/android_create_instance.h
  StereoKitC/xr_backends/extensions/android_create_instance.cpp
  StereoKitC/xr_backends/extensions/android_thread.h
  StereoKitC/xr_backends/extensions/android_thread.cpp
  StereoKitC/xr_backends/extensions/time.h
  StereoKitC/xr_backends/extensions/time.cpp
  StereoKitC/xr_backends/extensions/overlay.h
  StereoKitC/xr_backends/extensions/overlay.cpp
  StereoKitC/xr_backends/extensions/oculus_audio.h
  StereoKitC/xr_backends/extensions/oculus_audio.cpp
  StereoKitC/xr_backends/extensions/msft_anchors.h
  StereoKitC/xr_backends/extensions/msft_anchors.cpp
  StereoKitC/xr_backends/extensions/msft_bridge.h
  StereoKitC/xr_backends/extensions/msft_bridge.cpp
  StereoKitC/xr_backends/extensions/msft_observer.h
  StereoKitC/xr_backends/extensions/msft_observer.cpp
  StereoKitC/xr_backends/extensions/msft_scene_understanding.h
  StereoKitC/xr_backends/extensions/msft_scene_understanding.cpp )

# Tools
target_sources(StereoKitC PRIVATE
  StereoKitC/tools/virtual_keyboard.h
  StereoKitC/tools/virtual_keyboard.cpp
  StereoKitC/tools/file_picker.h
  StereoKitC/tools/file_picker.cpp
  StereoKitC/tools/tools.h
  StereoKitC/tools/tools.cpp )

# UI
target_sources(StereoKitC PRIVATE
  StereoKitC/stereokit_ui.h
  StereoKitC/ui/stereokit_ui.cpp
  StereoKitC/ui/interactors.h
  StereoKitC/ui/interactors.cpp
  StereoKitC/ui/interactor_modes.h
  StereoKitC/ui/interactor_modes.cpp
  StereoKitC/ui/ui_layout.h
  StereoKitC/ui/ui_layout.cpp
  StereoKitC/ui/ui_core.h
  StereoKitC/ui/ui_core.cpp
  StereoKitC/ui/ui_theming.h
  StereoKitC/ui/ui_theming.cpp )

# Built-in shaders
target_sources(StereoKitC PRIVATE
  StereoKitC/shaders_builtin/shader_builtin.h )

# Mirror the on-disk StereoKitC/ folder layout as Visual Studio folders/filters.
# This runs before shader compilation attaches generated files from the build
# folder, which live outside this tree.
get_target_property(SK_SOURCE_FILES StereoKitC SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/StereoKitC FILES ${SK_SOURCE_FILES})

# Realtime DSP built on inline SIMD wrappers is ~10x slower at -O0 and misses
# the device deadline, so these files compile optimized in every config - the
# trade is optimized-out values when stepping through them in a debugger.
# (MSVC skipped: /O2 conflicts with Debug's default /RTC1 checks.)
if (NOT MSVC)
  set_source_files_properties(
    StereoKitC/systems/audio_mix.cpp
    StereoKitC/libraries/miniaudio.cpp
    PROPERTIES COMPILE_OPTIONS "$<$<CONFIG:Debug>:-O2>")
endif()

# An exact pin rather than target_compile_features: features only guarantee a
# minimum, and on compilers that default past C++17 (GCC 16 uses C++20) u8
# literals become char8_t and no longer fit the C API.
set_target_properties(StereoKitC PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

skshaderc_compile_headers(StereoKitC
  ${CMAKE_BINARY_DIR}/shaders/
  "-e -O3 -f -z ${SK_SHADER_SVSL} ${SK_SHADER_VERBOSITY} -i ${CMAKE_CURRENT_SOURCE_DIR}/tools/include"
  StereoKitC/shaders_builtin/shader_builtin_blit.hlsl
  StereoKitC/shaders_builtin/shader_builtin_default.hlsl
  StereoKitC/shaders_builtin/shader_builtin_equirect.hlsl
  StereoKitC/shaders_builtin/shader_builtin_font.hlsl
  StereoKitC/shaders_builtin/shader_builtin_lines.hlsl
  StereoKitC/shaders_builtin/shader_builtin_pbr.hlsl
  StereoKitC/shaders_builtin/shader_builtin_pbr_clip.hlsl
  StereoKitC/shaders_builtin/shader_builtin_sh_compute.hlsl
  StereoKitC/shaders_builtin/shader_builtin_skybox.hlsl
  StereoKitC/shaders_builtin/shader_builtin_ui.hlsl
  StereoKitC/shaders_builtin/shader_builtin_ui_box.hlsl
  StereoKitC/shaders_builtin/shader_builtin_ui_quadrant.hlsl
  StereoKitC/shaders_builtin/shader_builtin_ui_aura.hlsl
  StereoKitC/shaders_builtin/shader_builtin_unlit.hlsl
  StereoKitC/shaders_builtin/shader_builtin_unlit_clip.hlsl
  StereoKitC/shaders_builtin/shader_builtin_lightmap.hlsl
  StereoKitC/shaders_builtin/shader_builtin_depth_prepass.hlsl
  StereoKitC/shaders_builtin/shader_builtin_cubemap_ggx.hlsl
  StereoKitC/shaders_builtin/shader_builtin_cubemap_downsample.hlsl )

if (NOT MSVC)
  # DirectXMath on non-msvc compilers needs a few extra files
  set(LINUX_INCLUDES
    StereoKitC/lib/include_no_win/)

  if (APPLE)
    # macOS uses dSYM for debug symbols, but we don't separate them currently
  else()
    set(SK_BIN_DEBUG_EXT ".so.dbg")

    if (SK_BUILD_SHARED_LIBS AND SK_STRIP_DEBUG_SYMBOLS)
      set(SK_SEPARATE_DBG ON)
      add_custom_command(TARGET StereoKitC POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:StereoKitC> $<TARGET_FILE_DIR:StereoKitC>/$<TARGET_FILE_PREFIX:StereoKitC>$<TARGET_FILE_BASE_NAME:StereoKitC>${SK_BIN_DEBUG_EXT}
        COMMAND ${CMAKE_STRIP} --strip-debug $<TARGET_FILE:StereoKitC>
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=$<TARGET_FILE_DIR:StereoKitC>/$<TARGET_FILE_PREFIX:StereoKitC>$<TARGET_FILE_BASE_NAME:StereoKitC>${SK_BIN_DEBUG_EXT} $<TARGET_FILE:StereoKitC>
      )
    endif()
  endif()
else()
  set(SK_BIN_DEBUG_EXT ".pdb")
  if (SK_BUILD_SHARED_LIBS)
    set(SK_SEPARATE_DBG ON)
  endif()
endif()

target_include_directories(StereoKitC
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/StereoKitC>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/StereoKitC/
    ${LINUX_INCLUDES}
)

target_link_libraries( StereoKitC
  PRIVATE
    ${LINUX_LIBS}
    ${MACOS_LIBS}
    ${WINDOWS_LIBS}
    ${CMAKE_DL_LIBS}
    meshoptimizer
    sk_ktx2
    zstd_decompress
  PUBLIC
    sk_app
    sk_renderer
    ${OPENXR_LIB}
    ${ANDROID_LIBS}
)

if (SK_PROFILE)
  target_link_libraries( StereoKitC PRIVATE TracyClient )
endif()

# Compiler options for all the targets, now that they're all present
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  if (MINGW)
    set(SK_RELEASE_COMPILE -g -O3 -fdata-sections -ffunction-sections )
    set(SK_RELEASE_LINK    -Wl,--gc-sections -Wl,--build-id)
  elseif(APPLE)
    set(SK_RELEASE_COMPILE -g -O3 -fdata-sections -ffunction-sections -flto)
    set(SK_RELEASE_LINK    -Wl,-dead_strip -flto)
  else()
    set(SK_RELEASE_COMPILE -g -O3 -fdata-sections -ffunction-sections -flto)
    set(SK_RELEASE_LINK    -Wl,--gc-sections -Wl,--build-id -flto)
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # /OPT settings need to be specified explicitly because we're also generating
  # /DEBUG pdbs, which disables them by default.
  set(SK_RELEASE_COMPILE /O2 /DNDEBUG /Zi /Gy /Gw /GL)
  set(SK_RELEASE_LINK    /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:LBR /OPT:ICF=2 /LTCG ) # See: https://learn.microsoft.com/en-us/cpp/build/reference/debug-generate-debug-info?view=msvc-170
endif()

if (SK_BUILD_OPENXR_LOADER)
  target_compile_options(openxr_loader PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_COMPILE}>")
  target_link_options   (openxr_loader PRIVATE ${SK_LINKER_FLAGS} "$<$<CONFIG:RELEASE>:${SK_RELEASE_LINK}>")
endif()
target_compile_options(StereoKitC PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_COMPILE}>")
target_link_options   (StereoKitC PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_LINK}>")
target_link_options   (StereoKitC PUBLIC ${SK_LINKER_FLAGS})

# Hide static dep symbols so they can't cross-bind against other copies in
# the process. MSVC exports dllexport-only already, Apple's ld lacks the flag.
# Android app libs bind to StereoKitC's ska_* exports, so those stay visible.
if (ANDROID)
  target_link_options(StereoKitC PRIVATE
    "-Wl,--exclude-libs,libsk_ktx2.a"
    "-Wl,--exclude-libs,libzstd_decompress.a"
    "-Wl,--exclude-libs,libmeshoptimizer.a")
elseif (NOT MSVC AND NOT APPLE)
  target_link_options(StereoKitC PRIVATE "-Wl,--exclude-libs,ALL")
endif()

# MinGW-specific settings
if (MINGW)
  # Remove 'lib' prefix - Windows DLLs shouldn't have it
  set_target_properties(StereoKitC PROPERTIES PREFIX "")
  # Statically link C/C++ runtime into the DLL to avoid shipping extra DLLs
  # Note: -static links all subsequent libraries statically
  target_link_options(StereoKitC PRIVATE -static)
endif()

###########################################
## Copy binaries to the distribute folder##
###########################################

if (WIN32)
  set(SK_BIN_OS "Win32")
elseif(ANDROID)
  set(SK_BIN_OS "Android")
elseif(APPLE)
  set(SK_BIN_OS "MacOS")
elseif(UNIX)
  set(SK_BIN_OS "Linux")
else()
  set(SK_BIN_OS "Unknown")
endif()
string(TOLOWER "${SK_BIN_OS}" SK_BIN_OS_LOWER)

if(ANDROID)
  set(SK_ARCH ${ANDROID_ABI})
elseif(WIN32)
  if(CMAKE_VS_PLATFORM_NAME STREQUAL "ARM" OR CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
    set(SK_ARCH ${CMAKE_VS_PLATFORM_NAME})
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SK_ARCH "x64")
  else()
    set(SK_ARCH "x86")
  endif()
elseif(APPLE)
  # CMAKE_OSX_ARCHITECTURES is set by presets, falls back to CMAKE_SYSTEM_PROCESSOR
  set(SK_ARCH_CHECK "${CMAKE_OSX_ARCHITECTURES}")
  if(NOT SK_ARCH_CHECK)
    set(SK_ARCH_CHECK "${CMAKE_SYSTEM_PROCESSOR}")
  endif()
  string(TOLOWER "${SK_ARCH_CHECK}" SK_ARCH_CHECK_LOWER)
  if(SK_ARCH_CHECK_LOWER STREQUAL "arm64")
    set(SK_ARCH "arm64")
  else()
    set(SK_ARCH "x64")
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_SYSTEM_PROCESSOR)
    set(SK_ARCH ${CMAKE_SYSTEM_PROCESSOR})
  else()
    set(SK_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
  endif()
  if (SK_ARCH STREQUAL "x86_64")
    set(SK_ARCH "x64")
  elseif (SK_ARCH STREQUAL "aarch64")
    set(SK_ARCH "arm64")
  endif()
else()
  set(SK_ARCH "Unknown")
endif()
# The tool folders are lowercase, MSVC's platform names are not
string(TOLOWER "${SK_ARCH}" SK_ARCH_LOWER)

set(SK_DISTRIBUTE_FOLDER "bin/distribute")

if(SK_DISTRIBUTE)
  add_custom_command(TARGET StereoKitC POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:StereoKitC>" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>/$<TARGET_FILE_NAME:StereoKitC>"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/include"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/StereoKitC/stereokit.h" "${CMAKE_CURRENT_SOURCE_DIR}/StereoKitC/stereokit_ui.h" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/include/" )

  # Copy the shader compiler out of the build folder. tools/ is what local C#
  # builds run, bin/tools/ is what the NuGet package packs. Targets that can't
  # run a compiler of their own (Android, web) build no skshaderc to copy.
  if(TARGET skshaderc)
    set(SK_TOOL_PLATFORM "${SK_BIN_OS_LOWER}_${SK_ARCH_LOWER}")
    add_custom_command(TARGET StereoKitC POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/tools/skshaderc/${SK_TOOL_PLATFORM}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:skshaderc>" "${CMAKE_CURRENT_SOURCE_DIR}/tools/skshaderc/${SK_TOOL_PLATFORM}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/bin/tools/${SK_TOOL_PLATFORM}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:skshaderc>" "${CMAKE_CURRENT_SOURCE_DIR}/bin/tools/${SK_TOOL_PLATFORM}")
  endif()

  if (SK_SEPARATE_DBG)
    add_custom_command(TARGET StereoKitC POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:StereoKitC>/$<TARGET_FILE_PREFIX:StereoKitC>$<TARGET_FILE_BASE_NAME:StereoKitC>${SK_BIN_DEBUG_EXT}" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>/$<TARGET_FILE_PREFIX:StereoKitC>$<TARGET_FILE_BASE_NAME:StereoKitC>${SK_BIN_DEBUG_EXT}" )
  endif()
  if (SK_DYNAMIC_OPENXR)
    set(SK_OPENXR_LOADER_DIST_SRC "$<TARGET_FILE:openxr_loader>")
    if (SK_STRIP_DEBUG_SYMBOLS AND NOT MSVC AND NOT APPLE)
      set(SK_OPENXR_LOADER_DIST_SRC "${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:openxr_loader>.stripped")
      add_custom_command(TARGET StereoKitC POST_BUILD
        COMMAND ${CMAKE_STRIP} --strip-debug -o "${SK_OPENXR_LOADER_DIST_SRC}" "$<TARGET_FILE:openxr_loader>" )
    endif()
    add_custom_command(TARGET StereoKitC POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>/standard"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SK_OPENXR_LOADER_DIST_SRC}" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>/standard/$<TARGET_FILE_NAME:openxr_loader>" )
  endif()
  # Copy sk_app.jar on Android (Java classes for file dialogs, activity results)
  if (ANDROID AND TARGET sk_app_jar)
    add_custom_command(TARGET StereoKitC POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${sk_app_BINARY_DIR}/sk_app.jar" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/sk_app.jar"
      COMMENT "Copying sk_app.jar to distribute folder" )
  endif()
  # Copy MoltenVK on macOS (required for Vulkan/Metal translation)
  if (APPLE AND TARGET MoltenVK)
    add_custom_command(TARGET StereoKitC POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:MoltenVK>" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>/$<TARGET_FILE_NAME:MoltenVK>"
      COMMENT "Copying MoltenVK to distribute folder" )
  endif()
endif()

###########################################
## StereoKitCTest                        ##
###########################################

if (SK_BUILD_TESTS)
  add_skapp ( StereoKitCTest
    PACKAGE_NAME net.stereokit.stereokitctest
    APP_NAME "StereoKit C Test"
    MANIFEST Examples/StereoKitCTest/AndroidManifest.xml )

  target_sources( StereoKitCTest PRIVATE
    Examples/StereoKitCTest/main.cpp
    Examples/StereoKitCTest/log_window.h
    Examples/StereoKitCTest/log_window.cpp
    Examples/StereoKitCTest/demo_envmap.h
    Examples/StereoKitCTest/demo_envmap.cpp
    Examples/StereoKitCTest/demo_shadows.h
    Examples/StereoKitCTest/demo_shadows.cpp
    Examples/StereoKitCTest/demo_draw.h
    Examples/StereoKitCTest/demo_draw.cpp
    Examples/StereoKitCTest/demo_aliasing.h
    Examples/StereoKitCTest/demo_aliasing.cpp
    Examples/StereoKitCTest/demo_postfx.h
    Examples/StereoKitCTest/demo_postfx.cpp
    Examples/StereoKitCTest/demo_bvh.h
    Examples/StereoKitCTest/demo_bvh.cpp
    Examples/StereoKitCTest/demo_custom_verts.h
    Examples/StereoKitCTest/demo_custom_verts.cpp
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
    Examples/StereoKitCTest/demo_ui_layout.h
    Examples/StereoKitCTest/demo_ui_layout.cpp
    Examples/StereoKitCTest/demo_mixed_reality.h
    Examples/StereoKitCTest/demo_mixed_reality.cpp
    Examples/StereoKitCTest/demo_anchors.h
    Examples/StereoKitCTest/demo_anchors.cpp
    Examples/StereoKitCTest/demo_lines.h
    Examples/StereoKitCTest/demo_lines.cpp
    Examples/StereoKitCTest/demo_lighting.h
    Examples/StereoKitCTest/demo_lighting.cpp
    Examples/StereoKitCTest/skt_lighting.h
    Examples/StereoKitCTest/skt_lighting.cpp
  )

  # Without a standard here we get the compiler's default, and GCC 16 moved that
  # to C++20, where u8 literals are char8_t and no longer fit the C API.
  set_target_properties(StereoKitCTest PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

  target_link_libraries( StereoKitCTest PRIVATE StereoKitC )
  if (SK_DYNAMIC_OPENXR)
    target_link_libraries( StereoKitCTest PRIVATE openxr_loader )
  endif()

  # MinGW: Statically link C++ runtime. Both DLL and EXE get their own copy.
  # --allow-multiple-definition: The DLL's import library has runtime symbols too,
  # but we want our own static copy, so tell the linker to ignore the duplicates.
  if (MINGW)
    target_link_options(StereoKitCTest PRIVATE -static -Wl,--allow-multiple-definition)
  endif()

  if (MSVC AND SK_MULTITHREAD_BUILD_BY_DEFAULT)
    target_compile_options(StereoKitCTest PRIVATE "/MP")
  endif()

  # Ensure we get MoltenVK binaries in the right place on MacOS
  skr_copy_moltenvk(StereoKitCTest)

  skshaderc_compile_headers( StereoKitCTest
    ${CMAKE_BINARY_DIR}/shaders/
    "-e -O3 -f ${SK_SHADER_SVSL} ${SK_SHADER_VERBOSITY} -i ${CMAKE_CURRENT_SOURCE_DIR}/tools/include"
    Examples/StereoKitCTest/Shaders/skt_default_lighting.hlsl
    Examples/StereoKitCTest/Shaders/skt_light_only.hlsl
    Examples/StereoKitCTest/Shaders/skt_shadow.hlsl
    Examples/StereoKitCTest/Shaders/skt_shadow_caster.hlsl
    Examples/StereoKitCTest/Shaders/skt_vert_custom.hlsl
    Examples/StereoKitCTest/Shaders/skt_postfx_vignette.hlsl
    Examples/StereoKitCTest/Shaders/skt_postfx_invert.hlsl
    Examples/StereoKitCTest/Shaders/skt_postfx_depth_fog.hlsl)

  # Add source assets and compiled shaders to the app
  target_skapp_assets(StereoKitCTest ${CMAKE_CURRENT_BINARY_DIR}/shaders)
  target_skapp_assets(StereoKitCTest ${CMAKE_CURRENT_SOURCE_DIR}/Examples/Assets)

  #target_link_options   (StereoKitCTest PRIVATE ${SK_LINKER_FLAGS})
endif()

###########################################
## SKTests                               ##
###########################################

# Headless native test runner (tools/tests). Each suite self-inits SK in
# offscreen mode, so it needs no window, shaders, or assets - just StereoKitC.
# Desktop only: it's a CLI test binary, not an interactive app.
if (SK_BUILD_TESTS AND NOT ANDROID)
  add_skapp ( SKTests
    APP_NAME "StereoKit Native Tests" )

  target_sources( SKTests PRIVATE
    tools/tests/main.cpp
    tools/tests/audio_tests.h
    tools/tests/audio_tests.cpp
    tools/tests/audio_render.cpp
    tools/tests/asset_tests.h
    tools/tests/asset_tests.cpp
  )

  # See the StereoKitCTest note: pin the standard so u8 literals stay char.
  set_target_properties(SKTests PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

  target_link_libraries( SKTests PRIVATE StereoKitC )
  if (SK_DYNAMIC_OPENXR)
    target_link_libraries( SKTests PRIVATE openxr_loader )
  endif()

  if (MINGW)
    target_link_options(SKTests PRIVATE -static -Wl,--allow-multiple-definition)
  endif()
  if (MSVC AND SK_MULTITHREAD_BUILD_BY_DEFAULT)
    target_compile_options(SKTests PRIVATE "/MP")
  endif()

  # MacOS needs MoltenVK beside the binary for the offscreen device to init.
  skr_copy_moltenvk(SKTests)
endif()

###########################################
## Multi-threaded build MSVC             ##
###########################################

# Setup Visual Studio to use multi-threaded compilation by
# default, this is mostly for when using Visual Studio as
# the IDE. CLI users can pass in command line arguments.
if (MSVC AND SK_MULTITHREAD_BUILD_BY_DEFAULT)
  target_compile_options(StereoKitC PRIVATE "/MP")
  if (TARGET openxr_loader)
    target_compile_options(openxr_loader PRIVATE "/MP")
  endif()
endif()
