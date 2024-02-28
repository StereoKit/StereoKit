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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(StereoKit VERSION "0.3.9" LANGUAGES CXX C)

# Add this repository's cmake modules to CMAKE_MODULE_PATH
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR}/cmake)

###########################################
## Options                               ##
###########################################

set(SK_BUILD_OPENXR_LOADER          ON  CACHE BOOL "Build the openxr_loader target from StereoKit")
set(SK_MULTITHREAD_BUILD_BY_DEFAULT ON  CACHE BOOL "MSVC only, on by default. This forces projects here to build with multi-threading (/MP)")
set(SK_BUILD_TESTS                  ON  CACHE BOOL "Build the StereoKitCTest project in addition to the StereoKitC library.")
set(SK_BUILD_SHARED_LIBS            ON  CACHE BOOL "Should StereoKit build as a shared, or static library?")
set(SK_DYNAMIC_OPENXR               OFF CACHE BOOL "Dynamic link with the standard OpenXR Loader. Not what you want on desktop, but on Android you may need to dynamic link with other loaders.")
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

###########################################
## Platform Specific Settings            ##
###########################################

if (ANDROID)
  set(ANDROID_LIBS
    android
    EGL)
  if (SK_DYNAMIC_OPENXR)
    add_definitions("-DSK_DYNAMIC_OPENXR")
  endif()
elseif(EMSCRIPTEN)
  #set(EMSCRIPTEN_LIBS)
  set(SK_BUILD_OPENXR_LOADER OFF)
  set(SK_BUILD_SHARED_LIBS OFF)
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
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  set(WINDOWS_LIBS
    Comdlg32
    DXGI
    D3D11)
    if (MSVC)
      list(APPEND WINDOWS_LIBS WindowsApp) # Required for ISAC spatial audio code
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
  add_definitions("-DWINDOWS_UWP")
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

###########################################
## Shader Compiler                       ##
###########################################

set(SK_SRC_SHADERS_HLSL
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
  StereoKitC/shaders_builtin/shader_builtin_unlit_clip.hlsl )

set(SK_SRC_SHADERS_H
  StereoKitC/shaders_builtin/shader_builtin.h )

set(SK_SHADER_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/tools/skshaderc")
foreach(SHADER ${SK_SRC_SHADERS_HLSL})
  if (WIN32)
    set(SK_SHADER_TARGETS "x")
  elseif(ANDROID)
    set(SK_SHADER_TARGETS "e")
  elseif(EMSCRIPTEN)
    set(SK_SHADER_TARGETS "w")
  elseif(UNIX)
    set(SK_SHADER_TARGETS "e")
  endif()

  # Hide shader compile metadata unless we're in verbose mode
  set(SK_SHADER_VERBOSITY "-si")
  if(CMAKE_VERBOSE_MAKEFILE)
    set(SK_SHADER_VERBOSITY "")
  endif()

  get_filename_component(SHADER_NAME ${SHADER} NAME)
  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.h
    COMMAND ${SK_SHADER_COMPILER} -O3 -f -h -z ${SK_SHADER_VERBOSITY} -t ${SK_SHADER_TARGETS} -i ${CMAKE_CURRENT_SOURCE_DIR}/tools/include -o ${CMAKE_BINARY_DIR}/shaders/ ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} )
  list(APPEND SK_SRC_SHADERS_H ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.h)
endforeach(SHADER)

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
  StereoKitC/asset_types/texture.cpp )

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
  StereoKitC/libraries/sk_gpu.h
  StereoKitC/libraries/sk_gpu.cpp
  StereoKitC/libraries/sokol_time.h
  StereoKitC/libraries/sokol_time.cpp
  StereoKitC/libraries/stb_image.h
  StereoKitC/libraries/stb_image_write.h
  StereoKitC/libraries/stb_truetype.h
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
  StereoKitC/systems/render.h
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

# Switch between a static or shared library based on
# the cmake options provided.
if (SK_BUILD_SHARED_LIBS)
  set(LIBRARY_TYPE SHARED)
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
  ${SK_SRC_SHADERS_H} )

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
source_group("shaders_builtin" FILES ${SK_SRC_SHADERS_H})

if (NOT MSVC)
  # DirectXMath on non-msvc compilers needs a few extra files
  set(LINUX_INCLUDES
    StereoKitC/lib/include_no_win/)
  set(SK_BIN_DEBUG_EXT ".so.dbg")

  if (NOT EMSCRIPTEN)
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
  set(SK_SEPARATE_DBG ON)
endif()

target_include_directories(StereoKitC
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/StereoKitC>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/StereoKitC/
    ${LINUX_INCLUDES}
    ${CMAKE_BINARY_DIR}/shaders/
)

target_link_libraries( StereoKitC
  PRIVATE
    ${OPENXR_LIB}
    ${PHYSICS_LIB}
    ${LINUX_LIBS}
    ${WINDOWS_LIBS}
    ${CMAKE_DL_LIBS}
  PUBLIC
    ${ANDROID_LIBS}
)

# Compiler options for all the targets, now that they're all present
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  set(SK_RELEASE_COMPILE -g -O3 -fdata-sections -ffunction-sections -flto)
  set(SK_RELEASE_LINK    -Wl,--gc-sections -flto)
  set(SK_WARNING_FLAG    -w)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(SK_RELEASE_COMPILE /O2 /DNDEBUG /Zi /Gy)
  set(SK_RELEASE_LINK    /DEBUG /INCREMENTAL:NO /OPT:REF /OPT:ICF) # See: https://learn.microsoft.com/en-us/cpp/build/reference/debug-generate-debug-info?view=msvc-170
  set(SK_WARNING_FLAG    /W0)
endif()

if (SK_BUILD_OPENXR_LOADER)
  target_compile_options(openxr_loader PRIVATE ${SK_WARNING_FLAG})
  target_compile_options(openxr_loader PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_COMPILE}>")
  target_link_options   (openxr_loader PRIVATE "$<$<CONFIG:RELEASE>:${SK_RELEASE_LINK}>")
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
    Examples/StereoKitCTest/Shaders/skt_default_lighting.hlsl.h
    Examples/StereoKitCTest/Shaders/skt_light_only.hlsl.h
  )

  target_link_libraries( StereoKitCTest
    StereoKitC
  )

  if (MSVC AND SK_MULTITHREAD_BUILD_BY_DEFAULT)
    target_compile_options(StereoKitCTest PRIVATE "/MP")
  endif()

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
  add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
  target_compile_options(openxr_loader PRIVATE "/MP")
  target_compile_options(StereoKitC PRIVATE "/MP")
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
