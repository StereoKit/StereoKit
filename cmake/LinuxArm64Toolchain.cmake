# On an arm64 host there's nothing to cross, so use the native compilers.
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
  return()
endif()

set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# SK_SYSROOT (env or cache) cross-compiles with clang against an arm64 sysroot,
# like Valve's steamrt4 SDK. Without it, this uses Debian/Ubuntu multiarch gcc.
if(DEFINED ENV{SK_SYSROOT})
  set(SK_SYSROOT "$ENV{SK_SYSROOT}" CACHE PATH "arm64 sysroot for clang cross-compiles" FORCE)
endif()
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES SK_SYSROOT)

if(SK_SYSROOT)
  set(CMAKE_SYSROOT             ${SK_SYSROOT})
  set(CMAKE_C_COMPILER          clang)
  set(CMAKE_CXX_COMPILER        clang++)
  set(CMAKE_C_COMPILER_TARGET   aarch64-linux-gnu)
  set(CMAKE_CXX_COMPILER_TARGET aarch64-linux-gnu)

  # Headers, libraries and pkg-config come from the sysroot, tools from the host.
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
  set(ENV{PKG_CONFIG_SYSROOT_DIR} "${SK_SYSROOT}")
  set(ENV{PKG_CONFIG_LIBDIR}      "${SK_SYSROOT}/usr/lib/aarch64-linux-gnu/pkgconfig:${SK_SYSROOT}/usr/share/pkgconfig")

  # ThinLTO objects are LLVM bitcode, which GNU binutils can't index.
  find_program(_LLVM_AR      NAMES llvm-ar      REQUIRED)
  find_program(_LLVM_RANLIB  NAMES llvm-ranlib  REQUIRED)
  find_program(_LLVM_OBJCOPY NAMES llvm-objcopy REQUIRED)
  find_program(_LLVM_STRIP   NAMES llvm-strip   REQUIRED)
  set(CMAKE_AR      ${_LLVM_AR}      CACHE FILEPATH "Archiver" FORCE)
  set(CMAKE_RANLIB  ${_LLVM_RANLIB}  CACHE FILEPATH "Ranlib"   FORCE)
  set(CMAKE_OBJCOPY ${_LLVM_OBJCOPY} CACHE FILEPATH "Objcopy"  FORCE)
  set(CMAKE_STRIP   ${_LLVM_STRIP}   CACHE FILEPATH "Strip"    FORCE)

  set(CMAKE_EXE_LINKER_FLAGS    "-fuse-ld=lld" CACHE STRING "")
  set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld" CACHE STRING "")
  set(CMAKE_MODULE_LINKER_FLAGS "-fuse-ld=lld" CACHE STRING "")
  return()
endif()

set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(CMAKE_AR      /usr/bin/aarch64-linux-gnu-ar      CACHE FILEPATH "Archiver")
set(CMAKE_LINKER  /usr/bin/aarch64-linux-gnu-ld      CACHE FILEPATH "Linker")
set(CMAKE_OBJCOPY /usr/bin/aarch64-linux-gnu-objcopy CACHE FILEPATH "Objcopy")
set(CMAKE_RANLIB  /usr/bin/aarch64-linux-gnu-ranlib  CACHE FILEPATH "Ranlib")
set(CMAKE_SIZE    /usr/bin/aarch64-linux-gnu-size    CACHE FILEPATH "Size")
set(CMAKE_STRIP   /usr/bin/aarch64-linux-gnu-strip   CACHE FILEPATH "Strip")
