# StereoKit Lua Bindings

This folder contains the Lua backend for `StereoKitAPIGen`. It generates:

- `stereokit_lua.c`: a Lua C module that marshals Lua values to the StereoKit C API.
- `stereokit.lua`: a Lua convenience layer with enum tables, callable struct tables, asset userdata methods, and module-style functions.
- `LuaUnsupported.md`: APIs that still need hand-written Lua ownership/lifetime policy or additional generator support.

The Lua layer is intentionally shaped around Lua conventions:

- C enum values are emitted as plain Lua tables.
- C structs are emitted as callable tables, so `sk.Vec3{ x = 1, y = 2, z = 3 }` is a valid value while `sk.Vec3.cross(a, b)` remains available.
- Opaque StereoKit asset handles are userdata, and generated module methods are attached to the userdata metatable for `mesh:release()` style calls.
- `out` and `ref` values are returned as extra Lua return values.
- C input arrays are accepted as Lua arrays; count parameters are inferred where the C signature uses an adjacent count.
- Mutable string buffers are accepted as Lua strings and returned as updated Lua strings.
- `char16_t*` APIs accept and return normal Lua UTF-8 strings.
- C `sk_default(...)` parameters can be omitted or passed as `nil` when the default can be represented in C.

Generate bindings from the repository root:

```sh
dotnet run --project tools/StereoKitAPIGen/StereoKitAPIGen.csproj -- \
  -f StereoKitC/stereokit.h \
  -f StereoKitC/stereokit_ui.h \
  -d build/lua \
  -l Lua
```

The generated C module expects Lua headers and the StereoKit C headers on the include path. For example, a syntax check on Linux can be run with:

```sh
clang -fsyntax-only -std=c11 -I/usr/include -IStereoKitC build/lua/stereokit_lua.c
```
