# Points the shell at the emscripten that ships inside the .NET wasm workload,
# rather than a standalone emsdk. Source it, don't run it:
#
#   source tools/web/dotnet_emsdk_env.sh
#   cmake --preset Web_Debug_Fast
#
# The C# web build links our archives with .NET's emcc, and emscripten refuses
# to mix objects across versions, so every web preset builds with this copy.
# It's the only emscripten the repo needs.

sk_dotnet_root="${DOTNET_ROOT:-$HOME/.dotnet}"

case "$(uname -s)-$(uname -m)" in
	Linux-x86_64)  sk_rid=linux-x64   ;;
	Linux-aarch64) sk_rid=linux-arm64 ;;
	Darwin-x86_64) sk_rid=osx-x64     ;;
	Darwin-arm64)  sk_rid=osx-arm64   ;;
	*) echo "No .NET emscripten pack RID for $(uname -s)-$(uname -m)."; unset sk_dotnet_root; return 1 ;;
esac

# Newest wins. Pack directories are versioned twice, once for the emscripten
# release and once for the .NET build, so sort both.
sk_find_pack() {
	ls -d "$sk_dotnet_root"/packs/Microsoft.NET.Runtime.Emscripten.*."$1"."$sk_rid"/*/tools 2>/dev/null | sort -V | tail -1
}

sk_emsdk="$(sk_find_pack Sdk)"
sk_emcache="$(sk_find_pack Cache)"
sk_emnode="$(sk_find_pack Node)"

if [ -z "$sk_emsdk" ] || [ -z "$sk_emcache" ] || [ -z "$sk_emnode" ]; then
	echo "No .NET emscripten packs under $sk_dotnet_root/packs."
	echo "Install them with: dotnet workload install wasm-tools"
	unset sk_dotnet_root sk_emsdk sk_emcache sk_emnode
	return 1
fi

# Work from a copy. emcc wipes the cache when the llvm path moves, and the pack
# was sealed on a build agent so it always has; freezing it stops that but then
# the emdawnwebgpu port can't unpack. The C# project points WasmCachePath here.
sk_repo="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export SK_EM_CACHE="$sk_repo/bin/intermediate/em_cache"
if [ ! -d "$SK_EM_CACHE/sysroot" ]; then
	echo "Copying emscripten cache to $SK_EM_CACHE (once, ~460MB)"
	mkdir -p "$SK_EM_CACHE"
	cp -r "$sk_emcache/emscripten/cache/." "$SK_EM_CACHE/"
fi

# Beside the cache it names, so checkouts never share a config
export EM_CONFIG="$sk_repo/bin/intermediate/sk_dotnet_emscripten.config"
export EM_FROZEN_CACHE=0
cat > "$EM_CONFIG" <<EOF
LLVM_ROOT = '$sk_emsdk/bin'
BINARYEN_ROOT = '$sk_emsdk'
NODE_JS = '$sk_emnode/bin/node'
CACHE = '$SK_EM_CACHE'
EOF

export DOTNET_EMSDK="$sk_emsdk"
export EMSDK_NODE="$sk_emnode/bin/node"
export PATH="$sk_emsdk/emscripten:$PATH"

echo "emscripten: $("$sk_emsdk/emscripten/emcc" --version 2>/dev/null | head -1)"
echo "DOTNET_EMSDK=$DOTNET_EMSDK"
echo "SK_EM_CACHE=$SK_EM_CACHE"

unset sk_dotnet_root sk_emsdk sk_emcache sk_emnode sk_repo sk_rid
unset -f sk_find_pack
