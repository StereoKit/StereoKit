# Building StereoKit

If you're here because you want to use StereoKit to create something, then you might be a lot more interested in the [Getting Started Guide](https://stereokit.net/Pages/Guides/Getting-Started.html)! Most developers do _not_ need to build StereoKit from the repository.

However, here's a few guidelines for people who do need to build this repository!

## I just want to run the test project!

The test project is pretty spiffy, and it can be nice to see it in action! There's a super easy way to run this by itself from the `/master` branch. Just load up [StereoKitTestNuGet.csproj](https://github.com/maluoi/StereoKit/blob/master/Examples/StereoKitTest/StereoKitTestNuGet.csproj) instead of the entire StereoKit solution! This uses the latest pre-built NuGet package, so you won't have to worry about navigating StereoKit's dependencies.

## I want to modify code (Windows)

StereoKit is a cross-platform library, but many features are not platform specific! If you don't need to test on a platform other than Windows, then you only need some of the dependencies.

**Requisites:**
- Visual Studio 2022
    - VS Workload: Universal Windows Platform
    - VS Workload: Desktop development with C++
    - VS Workload: Game development with C++
- [CMake](https://cmake.org) (To build OpenXR & dependencies)

Open StereoKit.sln and _unload_ these projects
- StereoKitTest_NetAndroid
- StereoKitTest_Xamarin
- StereoKitCTest_Android

Set StereoKitTest as the startup project, set the platform to x64, and you should be able to build it and run! On the first build, StereoKit will also clone and build `openxr_loader` using a [powershell script](https://github.com/maluoi/StereoKit/blob/master/Tools/Update-OpenXR.ps1) and cmake, so this will take some extra time.

## I want to modify code (Android)

- Install [cmake](https://cmake.org/) 3.21+.
- Install [ninja](https://ninja-build.org/): `winget install Ninja-build.Ninja` on Windows. This is technically optional, but is used in the cmake presets.
- Install the Android NDK r25c, either via Android's SDK Manager in Android Studio, or [here](https://developer.android.com/ndk/downloads/revision_history).
- Add `NDK` with your NDK path to your environment variables.

```shell
# For Windows GUI, see: `Edit the system environment variables` in the Control
# Panel.

# Windows Command Line, '/' seems more reliable for something in cmake
set NDK C:/Users/[user]/AppData/Local/Android/Sdk/ndk/25.2.9519653

# Powershell
[Environment]::SetEnvironmentVariable('NDK', 'C:/Users/[user]/AppData/Local/Android/Sdk/ndk/25.2.9519653', 'User')

# Linux
sudo echo "NDK=~/Android/Sdk/ndk/25.2.9519653" >> ~/.profile
```

The Visual Studio projects `StereoKitTest_NetAndroid` and `StereoKitTest_Xamarin` will now automatically trigger Android cmake builds before compiling their C# code.

## I want to modify code (Linux)

StereoKitTest.csproj is designed to work on Linux and can be used reasonably well from VS Code, but you will currently need to manually build the native binaries. Or if you're just after the native binaries to begin with, here's the steps!

Here's the pre-reqs you'll need first!
```shell
# here's the commands for installing Linux build pre-reqs:
sudo apt-get update
sudo apt-get install cmake ninja-build clang llvm lld pkg-config \
                     libx11-dev libxrandr-dev libxcursor-dev libxi-dev libfontconfig-dev \
                     libwayland-dev libxkbcommon-dev libdecor-0-dev
```

The last three are what sk_app's Wayland backend needs, and they're new as of
that backend. sk_app dlopens libwayland and libxkbcommon at runtime, so these
three are header-only build requirements and one binary runs on both X11 and
Wayland. They're detected via pkg-config, and a missing one quietly builds an
X11/XWayland-only binary rather than failing, so watch the `Wayland backend:`
line in the cmake configure output if you expect native Wayland.

To build with cmake:
```shell
### From StereoKit's root directory ###

# Build using the presets
cmake --preset Linux_x64_Release
cmake --build --preset Linux_x64_Release

# Run the native test app
./bin/intermediate/cmake/Linux_x64_Release/StereoKitCTest

# To run the C# test app, you will need the .NET SDK installed
dotnet run --configuration Release --project Examples/StereoKitTest/StereoKitTest.csproj

# check /bin/distribute for final binary files
```

## I want to cross-compile for Linux ARM64

You can build Linux ARM64 binaries right from an x64 Linux machine! The `Linux_Arm64_*` presets can find ARM64 headers and libraries in two ways. On an ARM64 Linux machine you don't need any of this, the regular Linux steps above build natively.

**Ubuntu or Debian multiarch.** This is what CI uses. Once arm64 is enabled as a multiarch architecture, install the cross compiler and the `:arm64` versions of the pre-reqs.
```shell
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
                     libx11-dev:arm64 libxrandr-dev:arm64 libxcursor-dev:arm64 libxi-dev:arm64 \
                     libwayland-dev:arm64 libxkbcommon-dev:arm64 libdecor-0-dev:arm64
```

**Any distro, with a sysroot.** Point `SK_SYSROOT` at an ARM64 sysroot, and the presets switch over to clang and lld from the Linux pre-reqs above. Valve's Steam Runtime 4 SDK makes a great sysroot, since it has every header StereoKit needs, and matches what the Steam Frame runs. It's a ~740MB download that unpacks to ~2.5GB.
```shell
mkdir -p ~/sysroots/steamrt4-arm64
curl -L https://repo.steampowered.com/steamrt4/images/latest-public-beta/com.valvesoftware.SteamRuntime.Sdk-arm64-steamrt4-sysroot.tar.gz \
  | tar -xz -C ~/sysroots/steamrt4-arm64

# Only needed the first time a build folder is configured
export SK_SYSROOT=~/sysroots/steamrt4-arm64
```

Then build away!
```shell
# Just the native binaries
cmake --preset Linux_Arm64_Release
cmake --build --preset Linux_Arm64_Release

# The C# test app, self-contained so the device doesn't need .NET installed.
# This lands in bin/arm64_Release/StereoKitTest/linux-arm64/publish/
dotnet publish Examples/StereoKitTest/StereoKitTest.csproj -c Release -r linux-arm64 --self-contained -p:SKSDKBuildMode=ARM64
```

`dotnet run` won't work here, since it tries to launch the ARM64 app on your x64 machine. Copy the publish folder over to the device and run it there instead.

## I want to build the whole NuGet package

The NuGet package build pipeline requires all the setup steps from above first! After that, you just need to run the [build powershell script](https://github.com/maluoi/StereoKit/blob/master/tools/Build-Nuget.ps1). This script will build all binary variants, run tests, and track some statistics.
