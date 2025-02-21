# Copyright 2021, Collabora, Ltd.
# Copyright Moses Turner, 2021.
# Copyright Nick Klingensmith, 2021.
# Many thanks to Ryan Pavlik for showing the way
# SPDX-License-Identifier: MIT

# This build file has the following options
# - SK_MULTITHREAD_BUILD_BY_DEFAULT
#     MSVC only, on by default. This forces projects here to
#     build with multi-threading (/MP)
# - SK_BUILD_TESTS
#     Build the StereoKitCTest project in addition to the StereoKitC
#     library. This is off by default.
# - SK_BUILD_SHARED_LIBS
#     Should StereoKit build as a shared, or static library?
# - SK_DYNAMIC_OPENXR
#     Dynamic link with the standard OpenXR Loader. Not what you want
#     on desktop, but on Android you may need to dynamic link with other
#     loaders.

cmake_minimum_required(VERSION 3.10)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

project(StereoKit VERSION "0.3.10" LANGUAGES CXX C)

# Add this repository's cmake modules to CMAKE_MODULE_PATH
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR}/cmake)

###########################################
## Options                               ##
###########################################

set(SK_BUILD_OPENXR_LOADER          ON  CACHE BOOL "Build the openxr_loader target from StereoKit")
set(SK_MULTITHREAD_BUILD_BY_DEFAULT ON  CACHE BOOL "MSVC only, on by default. This forces projects here to build with multi-threading (/MP)")
set(SK_BUILD_TESTS                  ON  CACHE BOOL "Build the StereoKitCTest project in addition to the StereoKitC library.")
set(SK_BUILD_SHARED_LIBS            ON  CACHE BOOL "Should StereoKit build as a shared, or static library?")
set(SK_PHYSICS                      ON  CACHE BOOL "Enable physics.")
set(SK_DYNAMIC_OPENXR               OFF CACHE BOOL "Dynamic link with the standard OpenXR Loader. Not what you want on desktop, but on Android you may need to dynamic link with other loaders.")
set(SK_WINDOWS_GL                   OFF CACHE BOOL "Build Windows version using OpenGL as the renderer. This is primarily for debugging GL code while developing on Windows.")
set(FORCE_COLORED_OUTPUT            OFF CACHE BOOL "Always produce ANSI-colored output (GNU/Clang only).")

###########################################
## Configuration settings                ##
###########################################

# Default build configuration to Release 
if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif()
endif()

# Only export functions that are explicitly marked for export
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)

###########################################
## Source Link                           ##
###########################################

if (MSVC)
  if ("${CMAKE_C_COMPILER_VERSION}" VERSION_GREATER_EQUAL "19.20")
    include(SourceLink)
    file(TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/source_link.json" SOURCE_LINK_JSON)
    source_link(${PROJECT_SOURCE_DIR} ${SOURCE_LINK_JSON})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SOURCELINK:${SOURCE_LINK_JSON}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SOURCELINK:${SOURCE_LINK_JSON}")
  else()
    message(WARNING "Disabling SourceLink due to old version of MSVC. Please update to VS2019!")
  endif()
endif()

###########################################
## Pretty colors                         ##
###########################################

if(FORCE_COLORED_OUTPUT)
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_compile_options(-fdiagnostics-color=always)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		add_compile_options(-fcolor-diagnostics)
	endif()
endif()

# Hide shader compile metadata unless we're in verbose mode
set(SK_SHADER_VERBOSITY "-si")
if(CMAKE_VERBOSE_MAKEFILE)
  set(SK_SHADER_VERBOSITY "")
endif()

###########################################
## Platform Specific Settings            ##
###########################################

if (ANDROID)
  set(SK_SHADER_TARGETS "e")
  set(ANDROID_LIBS
    android
    EGL)
  if (SK_DYNAMIC_OPENXR)
    add_definitions("-DSK_DYNAMIC_OPENXR")
  endif()
  # Align to 16kb as per Android recommendations
  # https://developer.android.com/guide/practices/page-sizes
  set(SK_LINKER_FLAGS "-Wl,-z,max-page-size=16384")
elseif(EMSCRIPTEN)
  #set(EMSCRIPTEN_LIBS)
  set(SK_BUILD_OPENXR_LOADER OFF)
  set(SK_PHYSICS OFF)
  set(SK_BUILD_SHARED_LIBS OFF)
  set(SK_SHADER_TARGETS "w")
  add_definitions("-D_XM_NO_INTRINSICS_")
  set(SK_LINKER_FLAGS 
    -sFULL_ES3
    -sMIN_WEBGL_VERSION=2
    -sMAX_WEBGL_VERSION=2
    -sFORCE_FILESYSTEM=1
    -sALLOW_MEMORY_GROWTH=1
    -g
    -gsource-map )
elseif (UNIX)
  set(SK_SHADER_TARGETS "e")
  set(LINUX_LIBS
    Threads::Threads
    EGL
    gbm
    X11
    Xfixes
    fontconfig )

  add_definitions("-DSKG_LINUX_EGL")
  # Add pthreads
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)

elseif(WIN32)
  if (SK_WINDOWS_GL)
    set(SK_SHADER_TARGETS "g")
    add_definitions("-DSKG_FORCE_OPENGL")
    add_definitions("-DSK_GPU_LABELS")
  else()
    set(SK_SHADER_TARGETS "x")
  endif()
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  set(WINDOWS_LIBS
    Comdlg32
    DXGI
    D3D11)
    if (MSVC)
      list(APPEND WINDOWS_LIBS WindowsApp) # Required for ISAC spatial audio code
    endif()
endif()

if (__MINGW32__)
  add_compile_definitions(PAL_STDCPP_COMPAT="needed for mingw")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  add_definitions("-DWINDOWS_UWP")
endif()

if(NOT SK_PHYSICS)
  message("-- Building without physics!")
  add_definitions("-DSK_PHYSICS_PASSTHROUGH")
else()
  message("-- Building with physics!")
endif()

# On Android, shared lib SK has a JNI_OnLoad, which can conflict with dev
# provided JNI_OnLoad in apps that consume SK as a static library.
if (SK_BUILD_SHARED_LIBS)
  add_definitions("-DSK_BUILD_SHARED")
else()
  add_definitions("-DSK_BUILD_STATIC")
endif()

###########################################
## Dependencies                          ##
###########################################

# See: https://github.com/cpm-cmake/CPM.cmake
include("cmake/CPM.cmake")

#### ReactPhysics3D - https://www.reactphysics3d.com/ ####
if(SK_PHYSICS)
  # Recent versions of MSVC introduce a chrono bug, this patch removes chrono
  # code from ReactPhysics3D. Linux doesn't like applying this patch, so we
  # only use it with MSVC
  if(MSVC)
    set(SK_PHYS_PATCH git apply ${CMAKE_CURRENT_SOURCE_DIR}/tools/reactphysics.patch)
  endif()

  CPMAddPackage( # We're scraping the comment after the NAME for some other scripts
    NAME reactphysics3d # 0.9.1
    GITHUB_REPOSITORY DanielChappuis/reactphysics3d
    GIT_TAG 17dd22e677ed861b0d4ece0c00a7e3cb503cc2f0
    PATCH_COMMAND ${SK_PHYS_PATCH}
    EXCLUDE_FROM_ALL YES
  )
  # Something in reactphysics3d seems to require -fPIC
  set_property(TARGET reactphysics3d PROPERTY POSITION_INDEPENDENT_CODE ON)

  set(PHYSICS_LIB reactphysics3d)
endif()

#### OpenXR - https://www.khronos.org/openxr/ ####
if (SK_BUILD_OPENXR_LOADER)
  CPMAddPackage( # We're scraping the comment after the NAME for some other scripts
    NAME openxr_loader # 1.0.34
    GITHUB_REPOSITORY KhronosGroup/OpenXR-SDK
    GIT_TAG 288d3a7ebc1ad959f62d51da75baa3d27438c499
    OPTIONS
    "BUILD_WITH_SYSTEM_JSONCPP OFF"
    "BUILD_WITH_XCB_HEADERS OFF"
    "DYNAMIC_LOADER ${SK_DYNAMIC_OPENXR}"
  )
  
  set(OPENXR_LIB openxr_loader)
endif()

#### meshoptimizer - https://github.com/zeux/meshoptimizer ####
CPMAddPackage( # We're scraping the comment after the NAME for some other scripts
  NAME meshoptimizer # 0.20
  GITHUB_REPOSITORY zeux/meshoptimizer
  GIT_TAG           c21d3be6ddf627f8ca852ba4b6db9903b0557858
)
set_target_properties(meshoptimizer PROPERTIES POSITION_INDEPENDENT_CODE ON)

#### basis_universal - https://github.com/BinomialLLC/basis_universal ####
CPMAddPackage( # We're scraping the comment after the NAME for some other scripts
  NAME basis_universal # 1.16.4
  GITHUB_REPOSITORY BinomialLLC/basis_universal
  GIT_TAG           900e40fb5d2502927360fe2f31762bdbb624455f
  DOWNLOAD_ONLY     YES # basis universal doesn't have a library in it, we include code from this library directly
)
add_compile_definitions(BASISD_SUPPORT_FXT1=0)
add_compile_definitions(BASISD_SUPPORT_PVRTC1=0)
add_compile_definitions(BASISD_SUPPORT_PVRTC2=0)
add_compile_definitions(BASISD_SUPPORT_ATC=0)
add_compile_definitions(BASISD_SUPPORT_ETC2_EAC_A8=0)
add_compile_definitions(BASISD_SUPPORT_BC7=0)
add_compile_definitions(BASISD_SUPPORT_BC7_MODE5=0)

#### sk_gpu - https://github.com/StereoKit/sk_gpu ####
CPMAddPackage( # We're scraping the comment after the NAME for some other scripts
  NAME sk_gpu # 2025.2.11
  URL https://github.com/StereoKit/sk_gpu/releases/download/v2025.2.11/sk_gpu.v2025.2.11.zip
)
# For building directly with in-progress sk_gpu changes, point this to your
# local sk_gpu clone, and use it instead of CPM.
#add_subdirectory(../sk_gpu/bin/distribute ../sk_gpu/bin/distribute/bin)

###########################################
## StereoKitC                            ##
###########################################

set(SK_SRC_CODE
  StereoKitC/stereokit.h

  StereoKitC/_stereokit.h
  StereoKitC/_stereokit_ui.h
  StereoKitC/color.cpp
  StereoKitC/backend.cpp
  StereoKitC/device.cpp
  StereoKitC/device.h
  StereoKitC/hierarchy.h
  StereoKitC/hierarchy.cpp
  StereoKitC/intersect.cpp
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

set(SK_SRC_ASSET_TYPES
  StereoKitC/asset_types/anchor.h
  StereoKitC/asset_types/anchor.cpp
  StereoKitC/asset_types/assets.h
  StereoKitC/asset_types/assets.cpp
  StereoKitC/asset_types/animation.h
  StereoKitC/asset_types/animation.cpp
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

set(SK_SRC_LIBRARIES
  StereoKitC/libraries/aileron_font_data.h
  StereoKitC/libraries/atomic_util.h
  StereoKitC/libraries/array.h
  StereoKitC/libraries/cgltf.h
  StereoKitC/libraries/cgltf.cpp
  StereoKitC/libraries/ferr_hash.h
  StereoKitC/libraries/ferr_hash.cpp
  StereoKitC/libraries/isac_spatial_sound.h
  StereoKitC/libraries/isac_spatial_sound.cpp
  StereoKitC/libraries/micro_ply.h
  StereoKitC/libraries/miniaudio.h
  StereoKitC/libraries/miniaudio.cpp
  StereoKitC/libraries/qoi.h
  StereoKitC/libraries/qoi.cpp
  StereoKitC/libraries/sk_gpu.cpp
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

set(SK_SRC_UTILS
  StereoKitC/utils/sdf.h
  StereoKitC/utils/sdf.cpp
  StereoKitC/utils/random.h
  StereoKitC/utils/random.cpp)

set(SK_SRC_SYSTEMS
  StereoKitC/systems/audio.h
  StereoKitC/systems/audio.cpp
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
  StereoKitC/systems/line_drawer.h
  StereoKitC/systems/line_drawer.cpp
  StereoKitC/systems/physics.h
  StereoKitC/systems/physics.cpp
  StereoKitC/systems/render.h
  StereoKitC/systems/render_.h
  StereoKitC/systems/render.cpp
  StereoKitC/systems/render_pipeline.h
  StereoKitC/systems/render_pipeline.cpp
  StereoKitC/systems/sprite_drawer.h
  StereoKitC/systems/sprite_drawer.cpp
  StereoKitC/systems/system.h
  StereoKitC/systems/system.cpp
  StereoKitC/systems/text.h
  StereoKitC/systems/text.cpp  
  StereoKitC/systems/world.h
  StereoKitC/systems/world.cpp )

set(SK_SRC_HANDS
  StereoKitC/hands/hand_mouse.h
  StereoKitC/hands/hand_mouse.cpp
  StereoKitC/hands/hand_override.h
  StereoKitC/hands/hand_override.cpp
  StereoKitC/hands/hand_oxr_articulated.h
  StereoKitC/hands/hand_oxr_articulated.cpp
  StereoKitC/hands/hand_oxr_controller.h
  StereoKitC/hands/hand_oxr_controller.cpp
  StereoKitC/hands/hand_poses.h
  StereoKitC/hands/input_hand.h
  StereoKitC/hands/input_hand.cpp )

set(SK_SRC_PLATFORMS
  StereoKitC/platforms/android.cpp
  StereoKitC/platforms/android.h
  StereoKitC/platforms/linux.h
  StereoKitC/platforms/linux.cpp
  StereoKitC/platforms/platform.h
  StereoKitC/platforms/_platform.h
  StereoKitC/platforms/platform.cpp
  StereoKitC/platforms/platform_common_win.cpp
  StereoKitC/platforms/platform_common_unix.cpp
  StereoKitC/platforms/web.h
  StereoKitC/platforms/web.cpp
  StereoKitC/platforms/uwp.h
  StereoKitC/platforms/uwp.cpp
  StereoKitC/platforms/win32.h
  StereoKitC/platforms/win32.cpp )

set(SK_SRC_XR_BACKENDS
  StereoKitC/xr_backends/openxr.h
  StereoKitC/xr_backends/openxr.cpp
  StereoKitC/xr_backends/openxr_extensions.h
  StereoKitC/xr_backends/openxr_input.h
  StereoKitC/xr_backends/openxr_input.cpp
  StereoKitC/xr_backends/openxr_scene_understanding.h
  StereoKitC/xr_backends/openxr_scene_understanding.cpp
  StereoKitC/xr_backends/openxr_view.h
  StereoKitC/xr_backends/openxr_view.cpp
  StereoKitC/xr_backends/xr.h
  StereoKitC/xr_backends/xr.cpp
  StereoKitC/xr_backends/simulator.h
  StereoKitC/xr_backends/simulator.cpp
  StereoKitC/xr_backends/window.h
  StereoKitC/xr_backends/window.cpp
  StereoKitC/xr_backends/offscreen.h
  StereoKitC/xr_backends/offscreen.cpp
  StereoKitC/xr_backends/anchor_openxr_msft.h
  StereoKitC/xr_backends/anchor_openxr_msft.cpp
  StereoKitC/xr_backends/anchor_stage.h
  StereoKitC/xr_backends/anchor_stage.cpp )

set(SK_SRC_TOOLS
  StereoKitC/tools/virtual_keyboard.h
  StereoKitC/tools/virtual_keyboard.cpp
  StereoKitC/tools/file_picker.h
  StereoKitC/tools/file_picker.cpp
  StereoKitC/tools/tools.h
  StereoKitC/tools/tools.cpp )

set(SK_SRC_UI
  StereoKitC/stereokit_ui.h
  StereoKitC/ui/stereokit_ui.cpp
  StereoKitC/ui/ui_layout.h
  StereoKitC/ui/ui_layout.cpp
  StereoKitC/ui/ui_core.h
  StereoKitC/ui/ui_core.cpp
  StereoKitC/ui/ui_theming.h
  StereoKitC/ui/ui_theming.cpp )

if (basis_universal_ADDED)
  set(SK_SRC_BASISU
    ${basis_universal_SOURCE_DIR}/transcoder/basisu_transcoder.cpp
    ${basis_universal_SOURCE_DIR}/zstd/zstd.c)
endif()

set(SK_SRC_SHADERS_H
  StereoKitC/shaders_builtin/shader_builtin.h )

# Switch between a static or shared library based on
# the cmake options provided.
if (SK_BUILD_SHARED_LIBS)
  set(LIBRARY_TYPE SHARED)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
else()
  set(LIBRARY_TYPE STATIC)
endif()

add_library(StereoKitC ${LIBRARY_TYPE}
  ${SK_SRC_CODE}
  ${SK_SRC_ASSET_TYPES}
  ${SK_SRC_LIBRARIES}
  ${SK_SRC_UTILS}
  ${SK_SRC_SYSTEMS}
  ${SK_SRC_HANDS}
  ${SK_SRC_PLATFORMS}
  ${SK_SRC_XR_BACKENDS}
  ${SK_SRC_TOOLS}
  ${SK_SRC_UI}
  ${SK_SRC_BASISU}
  ${SK_SRC_SHADERS_H} )

if (ANDROID)
  set_property(TARGET StereoKitC PROPERTY CXX_STANDARD 11)
  set_property(TARGET StereoKitC PROPERTY CXX_STANDARD_REQUIRED ON)
else()
  set_property(TARGET StereoKitC PROPERTY CXX_STANDARD 17)
  set_property(TARGET StereoKitC PROPERTY CXX_STANDARD_REQUIRED ON)
endif()

skshaderc_compile_headers(StereoKitC
  ${CMAKE_BINARY_DIR}/shaders/
  "-O3 -f -e -z ${SK_SHADER_VERBOSITY} -t ${SK_SHADER_TARGETS} -i ${CMAKE_CURRENT_SOURCE_DIR}/tools/include"
  StereoKitC/shaders_builtin/shader_builtin_blit.hlsl
  StereoKitC/shaders_builtin/shader_builtin_default.hlsl
  StereoKitC/shaders_builtin/shader_builtin_equirect.hlsl
  StereoKitC/shaders_builtin/shader_builtin_font.hlsl
  StereoKitC/shaders_builtin/shader_builtin_lines.hlsl
  StereoKitC/shaders_builtin/shader_builtin_pbr.hlsl
  StereoKitC/shaders_builtin/shader_builtin_pbr_clip.hlsl
  StereoKitC/shaders_builtin/shader_builtin_skybox.hlsl
  StereoKitC/shaders_builtin/shader_builtin_ui.hlsl
  StereoKitC/shaders_builtin/shader_builtin_ui_box.hlsl
  StereoKitC/shaders_builtin/shader_builtin_ui_quadrant.hlsl
  StereoKitC/shaders_builtin/shader_builtin_ui_aura.hlsl
  StereoKitC/shaders_builtin/shader_builtin_unlit.hlsl
  StereoKitC/shaders_builtin/shader_builtin_unlit_clip.hlsl
  StereoKitC/shaders_builtin/shader_builtin_lightmap.hlsl )

# Set up Visual Studio folders/filters for a more organized
# workspace there.
source_group("code"            FILES ${SK_SRC_CODE})
source_group("asset_types"     FILES ${SK_SRC_ASSET_TYPES})
source_group("libraries"       FILES ${SK_SRC_LIBRARIES})
source_group("utils"           FILES ${SK_SRC_UTILS})
source_group("systems"         FILES ${SK_SRC_SYSTEMS})
source_group("hands"           FILES ${SK_SRC_HANDS})
source_group("platform"        FILES ${SK_SRC_PLATFORMS})
source_group("xr_backends"     FILES ${SK_SRC_XR_BACKENDS})
source_group("tools"           FILES ${SK_SRC_TOOLS})
source_group("ui"              FILES ${SK_SRC_UI})
source_group("basisu"          FILES ${SK_SRC_BASISU})
source_group("shaders_builtin" FILES ${SK_SRC_SHADERS_H})

if (NOT MSVC)
  # DirectXMath on non-msvc compilers needs a few extra files
  set(LINUX_INCLUDES
    StereoKitC/lib/include_no_win/)
  set(SK_BIN_DEBUG_EXT ".so.dbg")

  if (NOT EMSCRIPTEN AND SK_BUILD_SHARED_LIBS)
    set(SK_SEPARATE_DBG ON)
    add_custom_command(TARGET StereoKitC POST_BUILD
      COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:StereoKitC> $<TARGET_FILE_DIR:StereoKitC>/$<TARGET_FILE_BASE_NAME:StereoKitC>${SK_BIN_DEBUG_EXT}
      COMMAND ${CMAKE_STRIP} --strip-debug $<TARGET_FILE:StereoKitC>
      COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=$<TARGET_FILE_DIR:StereoKitC>/$<TARGET_FILE_BASE_NAME:StereoKitC>${SK_BIN_DEBUG_EXT} $<TARGET_FILE:StereoKitC>
    )
  endif()
  if (SK_DYNAMIC_OPENXR)
    add_custom_command(TARGET StereoKitC POST_BUILD
      COMMAND ${CMAKE_STRIP} --strip-debug $<TARGET_FILE:openxr_loader>
    )
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
if (basis_universal_ADDED)
  target_include_directories(StereoKitC PRIVATE ${basis_universal_SOURCE_DIR}/transcoder)
endif()

target_link_libraries( StereoKitC
  PRIVATE
    ${OPENXR_LIB}
    ${PHYSICS_LIB}
    ${LINUX_LIBS}
    ${WINDOWS_LIBS}
    ${CMAKE_DL_LIBS}
    meshoptimizer
  PUBLIC
    sk_gpu
    ${ANDROID_LIBS}
)

# Compiler options for all the targets, now that they're all present
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  set(SK_RELEASE_COMPILE -g -O3 -fdata-sections -ffunction-sections -flto)
  set(SK_RELEASE_LINK    -Wl,--gc-sections -Wl,--build-id -flto)
  set(SK_WARNING_FLAG    -w)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # /OPT settings need to be specified explicitly because we're also generating
  # /DEBUG pdbs, which disables them by default.
  set(SK_RELEASE_COMPILE /O2 /DNDEBUG /Zi /Gy /Gw /GL)
  set(SK_RELEASE_LINK    /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:LBR /OPT:ICF=2 /LTCG ) # See: https://learn.microsoft.com/en-us/cpp/build/reference/debug-generate-debug-info?view=msvc-170
  set(SK_WARNING_FLAG    /W0)
endif()

if (SK_BUILD_OPENXR_LOADER)
  target_compile_options(openxr_loader PRIVATE ${SK_WARNING_FLAG})
  target_compile_options(openxr_loader PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_COMPILE}>")
  target_link_options   (openxr_loader PRIVATE ${SK_LINKER_FLAGS} "$<$<CONFIG:RELEASE>:${SK_RELEASE_LINK}>")
endif()
if (SK_PHYSICS)
  target_compile_options(reactphysics3d PRIVATE ${SK_WARNING_FLAG})
  target_compile_options(reactphysics3d PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_COMPILE}>")
  target_link_options   (reactphysics3d PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_LINK}>")
endif()
target_compile_options(StereoKitC PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_COMPILE}>")
target_link_options   (StereoKitC PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_LINK}>")
target_link_options   (StereoKitC PUBLIC ${SK_LINKER_FLAGS})

###########################################
## Copy binaries to the distribute folder##
###########################################

if (CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  set(SK_BIN_OS "UWP")
elseif (WIN32)
  set(SK_BIN_OS "Win32")
elseif(ANDROID)
  set(SK_BIN_OS "Android")
elseif(UNIX)
  set(SK_BIN_OS "Linux")
elseif(EMSCRIPTEN)
  set(SK_BIN_OS "WASM")
else()
  set(SK_BIN_OS "Unknown")
endif()

if(ANDROID)
  set(SK_ARCH ${ANDROID_ABI})
  set(SK_LIB_PREFIX "lib")
elseif(WIN32)
  if(CMAKE_VS_PLATFORM_NAME STREQUAL "ARM" OR CMAKE_VS_PLATFORM_NAME STREQUAL "ARM64")
    set(SK_ARCH ${CMAKE_VS_PLATFORM_NAME})
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SK_ARCH "x64")
  else()
    set(SK_ARCH "x86")
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_SYSTEM_PROCESSOR)
    set(SK_ARCH ${CMAKE_SYSTEM_PROCESSOR})
  else()
    set(SK_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
  endif()
  if (SK_ARCH STREQUAL "x86_64")
    set(SK_ARCH "x64")
  endif()
  set(SK_LIB_PREFIX "lib")
else()
  set(SK_ARCH "Unknown")
endif()

set(SK_DISTRIBUTE_FOLDER "bin/distribute")

add_custom_command(TARGET StereoKitC POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:StereoKitC>" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>/$<TARGET_FILE_NAME:StereoKitC>"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/include"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/StereoKitC/stereokit.h" "${CMAKE_CURRENT_SOURCE_DIR}/StereoKitC/stereokit_ui.h" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/include/" )
add_custom_command(TARGET StereoKitC POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${sk_gpu_SOURCE_DIR}/tools" "${CMAKE_CURRENT_SOURCE_DIR}/tools/skshaderc" )
if (SK_SEPARATE_DBG)
  add_custom_command(TARGET StereoKitC POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:StereoKitC>/$<TARGET_FILE_BASE_NAME:StereoKitC>${SK_BIN_DEBUG_EXT}" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>/${SK_LIB_PREFIX}$<TARGET_FILE_BASE_NAME:StereoKitC>${SK_BIN_DEBUG_EXT}" )
endif()
if (SK_DYNAMIC_OPENXR)
  add_custom_command(TARGET StereoKitC POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>/standard"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:openxr_loader>" "${CMAKE_CURRENT_SOURCE_DIR}/${SK_DISTRIBUTE_FOLDER}/bin/${SK_BIN_OS}/${SK_ARCH}/$<CONFIG>/standard/$<TARGET_FILE_NAME:openxr_loader>" )
endif()

###########################################
## StereoKitCTest                        ##
###########################################

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  set(SK_WIN32 WIN32)
endif()

if (SK_BUILD_TESTS)
  add_executable( StereoKitCTest ${SK_WIN32}
    Examples/StereoKitCTest/main.cpp
    Examples/StereoKitCTest/demo_envmap.h
    Examples/StereoKitCTest/demo_envmap.cpp
    Examples/StereoKitCTest/demo_draw.h
    Examples/StereoKitCTest/demo_draw.cpp
    Examples/StereoKitCTest/demo_aliasing.h
    Examples/StereoKitCTest/demo_aliasing.cpp
    Examples/StereoKitCTest/demo_bvh.h
    Examples/StereoKitCTest/demo_bvh.cpp
    Examples/StereoKitCTest/demo_desktop.h
    Examples/StereoKitCTest/demo_desktop.cpp
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
    Examples/StereoKitCTest/demo_world.h
    Examples/StereoKitCTest/demo_world.cpp
    Examples/StereoKitCTest/demo_anchors.h
    Examples/StereoKitCTest/demo_anchors.cpp
    Examples/StereoKitCTest/demo_windows.h
    Examples/StereoKitCTest/demo_windows.cpp
    Examples/StereoKitCTest/demo_lines.h
    Examples/StereoKitCTest/demo_lines.cpp
    Examples/StereoKitCTest/demo_lighting.h
    Examples/StereoKitCTest/demo_lighting.cpp
    Examples/StereoKitCTest/skt_lighting.h
    Examples/StereoKitCTest/skt_lighting.cpp
  )

  target_link_libraries( StereoKitCTest
    StereoKitC
  )

  if (MSVC AND SK_MULTITHREAD_BUILD_BY_DEFAULT)
    target_compile_options(StereoKitCTest PRIVATE "/MP8")
  endif()

  skshaderc_compile_headers( StereoKitCTest
    ${CMAKE_BINARY_DIR}/test_shaders/
    "-O3 -f ${SK_SHADER_VERBOSITY} -t ${SK_SHADER_TARGETS} -i ${CMAKE_CURRENT_SOURCE_DIR}/tools/include"
    Examples/StereoKitCTest/Shaders/blit.hlsl
    Examples/StereoKitCTest/Shaders/skt_default_lighting.hlsl
    Examples/StereoKitCTest/Shaders/skt_light_only.hlsl)

  #target_link_options   (StereoKitCTest PRIVATE ${SK_LINKER_FLAGS})

  set (source      ${CMAKE_CURRENT_SOURCE_DIR}/Examples/Assets)
  set (destination $<TARGET_FILE_DIR:StereoKitCTest>/Assets)
  add_custom_command(
    TARGET StereoKitCTest
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${source} ${destination}
    COMMENT "Copy resources from ${source} => ${destination}")
endif()

###########################################
## Multi-threaded build MSVC             ##
###########################################

# Setup Visual Studio to use multi-threaded compilation by
# default, this is mostly for when using Visual Studio as
# the IDE. CLI users can pass in command line arguments.
if (MSVC AND SK_MULTITHREAD_BUILD_BY_DEFAULT)
  add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP8>)
  if(SK_PHYSICS)
    target_compile_options(reactphysics3d PRIVATE "/MP8")
  endif()
  target_compile_options(openxr_loader PRIVATE "/MP8")
  target_compile_options(StereoKitC PRIVATE "/MP8")
endif()

###########################################
## Installation                          ##
###########################################

# For installing on Linux
# find_package(PkgConfig)

#install(
#  TARGETS StereoKitC 
#  EXPORT  StereoKitC-targets
#)
#
#install(
#  FILES       "StereoKitC/stereokit.h" "StereoKitC/stereokit_ui.h" "tools/include/stereokit.hlsli"
#  DESTINATION include)
#
#install(
#  FILES       "tools/skshaderc" 
#  DESTINATION bin)
#
#install(
#  EXPORT      StereoKitC-targets
#  FILE        StereoKitCConfig.cmake
#  NAMESPACE   StereoKitC::
#  DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/StereoKitC"
#)
