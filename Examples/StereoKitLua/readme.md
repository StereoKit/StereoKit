# Experimental Lua Support

StereoKit's Lua support is generated from the C headers by `StereoKitAPIGen`. The generated Lua layer uses plain Lua tables for enums and structs, userdata for StereoKit asset handles, and method syntax for asset APIs like `mesh:draw(...)`.

This example keeps the generated files out of source control. The build script generates `stereokit.lua` and `stereokit_lua.c`, compiles the Lua C module, and then runs [main.lua](main.lua).

## Setup

Build StereoKitC first. On Linux, from the repository root:

```sh
cmake --preset Linux_x64_Release
cmake --build --preset Linux_x64_Release
```

Check that `bin/distribute` contains the StereoKitC headers and native library. If your files are somewhere else, set these environment variables before running the example:

```sh
export SK_DIST=/path/to/distribute
export SK_INCLUDE=/path/to/stereokit/headers
export SK_LIB_DIR=/path/to/stereokit/library
```

You'll also need the .NET SDK, a C compiler, Lua, and Lua development headers. On Ubuntu-like distributions:

```sh
sudo apt-get install lua5.4 liblua5.4-dev pkg-config clang
```

## Run

From this folder:

```sh
bash build.sh
```

The script does roughly this:

```sh
dotnet run --project ../../tools/StereoKitAPIGen/StereoKitAPIGen.csproj -- \
  -f ../../StereoKitC/stereokit.h \
  -f ../../StereoKitC/stereokit_ui.h \
  -d generated \
  -l Lua

cc -shared -fPIC -std=c11 \
  $(pkg-config --cflags lua5.4) \
  -I../../bin/distribute/include \
  generated/stereokit_lua.c \
  -L../../bin/distribute/bin/Linux/x64/Release -lStereoKitC \
  $(pkg-config --libs lua5.4) \
  -o generated/stereokit_lua.so

LUA_PATH="$PWD/generated/?.lua;$LUA_PATH" \
LUA_CPATH="$PWD/generated/?.so;$LUA_CPATH" \
LD_LIBRARY_PATH="../../bin/distribute/bin/Linux/x64/Release:$LD_LIBRARY_PATH" \
lua main.lua
```

If your Lua install uses a different pkg-config name, set `LUA_PKG`, or set `LUA_CFLAGS` and `LUA_LIBS` directly.
