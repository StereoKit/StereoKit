set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER       clang   CACHE FILEPATH "C compiler")
set(CMAKE_CXX_COMPILER     clang++ CACHE FILEPATH "C++ compiler")

# LTO objects are LLVM bitcode, so we need llvm-ar/llvm-ranlib to build
# proper archives. GNU ar can't read LLVM bitcode and will produce
# archives with broken symbol indexes.
find_program(_LLVM_AR     NAMES llvm-ar     REQUIRED)
find_program(_LLVM_RANLIB NAMES llvm-ranlib REQUIRED)
set(CMAKE_AR     ${_LLVM_AR}     CACHE FILEPATH "Archiver" FORCE)
set(CMAKE_RANLIB ${_LLVM_RANLIB} CACHE FILEPATH "Ranlib"   FORCE)

set(CMAKE_EXE_LINKER_FLAGS    "-fuse-ld=lld" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS "-fuse-ld=lld" CACHE STRING "")
