#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

ROOT="../.."
OUT="generated"
SK_DIST="${SK_DIST:-$ROOT/bin/distribute}"
SK_INCLUDE="${SK_INCLUDE:-$SK_DIST/include}"

find_lua_pkg() {
	for name in lua5.4 lua54 lua-5.4 lua lua5.3 lua-5.3 luajit; do
		if pkg-config --exists "$name"; then
			printf '%s\n' "$name"
			return 0
		fi
	done
	return 1
}

find_lib_dir() {
	if [[ -n "${SK_LIB_DIR:-}" ]]; then
		printf '%s\n' "$SK_LIB_DIR"
		return 0
	fi

	for dir in \
		"$SK_DIST/bin/Linux/x64/Release" \
		"$SK_DIST/bin/MacOS/x64/Release" \
		"$SK_DIST/bin/Win32/x64/Release" \
		"$SK_DIST/lib" \
		"$SK_DIST"; do
		if compgen -G "$dir/libStereoKitC.*" >/dev/null || [[ -f "$dir/StereoKitC.dll" ]]; then
			printf '%s\n' "$dir"
			return 0
		fi
	done

	return 1
}

mkdir -p "$OUT"

dotnet run --project "$ROOT/tools/StereoKitAPIGen/StereoKitAPIGen.csproj" -- \
	-f "$ROOT/StereoKitC/stereokit.h" \
	-f "$ROOT/StereoKitC/stereokit_ui.h" \
	-d "$OUT" \
	-l Lua

if [[ ! -d "$SK_INCLUDE" ]]; then
	echo "StereoKit headers were not found at: $SK_INCLUDE" >&2
	echo "Build StereoKitC first, or set SK_DIST/SK_INCLUDE to your distribution folder." >&2
	exit 1
fi

LUA_PKG="${LUA_PKG:-$(find_lua_pkg || true)}"
if [[ -z "$LUA_PKG" ]]; then
	echo "Could not find Lua through pkg-config." >&2
	echo "Install Lua development headers, or set LUA_CFLAGS and LUA_LIBS manually." >&2
	exit 1
fi

LUA_CFLAGS="${LUA_CFLAGS:-$(pkg-config --cflags "$LUA_PKG")}"
LUA_LIBS="${LUA_LIBS:-$(pkg-config --libs "$LUA_PKG")}"
SK_LIB_DIR="$(find_lib_dir || true)"
if [[ -z "$SK_LIB_DIR" ]]; then
	echo "Could not find the StereoKitC library under: $SK_DIST" >&2
	echo "Build StereoKitC first, or set SK_LIB_DIR to the directory containing StereoKitC." >&2
	exit 1
fi

CC="${CC:-cc}"
"$CC" -shared -fPIC -std=c11 \
	$LUA_CFLAGS \
	-I"$SK_INCLUDE" \
	"$OUT/stereokit_lua.c" \
	-L"$SK_LIB_DIR" -lStereoKitC \
	$LUA_LIBS \
	-o "$OUT/stereokit_lua.so"

export LUA_PATH="$PWD/$OUT/?.lua;${LUA_PATH:-}"
export LUA_CPATH="$PWD/$OUT/?.so;${LUA_CPATH:-}"
export LD_LIBRARY_PATH="$SK_LIB_DIR:${LD_LIBRARY_PATH:-}"
export DYLD_LIBRARY_PATH="$SK_LIB_DIR:${DYLD_LIBRARY_PATH:-}"

lua main.lua
