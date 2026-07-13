set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

set(CMAKE_AR      /usr/bin/aarch64-linux-gnu-ar      CACHE FILEPATH "Archiver")
set(CMAKE_LINKER  /usr/bin/aarch64-linux-gnu-ld      CACHE FILEPATH "Linker")
set(CMAKE_OBJCOPY /usr/bin/aarch64-linux-gnu-objcopy CACHE FILEPATH "Objcopy")
set(CMAKE_RANLIB  /usr/bin/aarch64-linux-gnu-ranlib  CACHE FILEPATH "Ranlib")
set(CMAKE_SIZE    /usr/bin/aarch64-linux-gnu-size    CACHE FILEPATH "Size")
set(CMAKE_STRIP   /usr/bin/aarch64-linux-gnu-strip   CACHE FILEPATH "Strip")
