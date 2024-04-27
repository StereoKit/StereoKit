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
sudo apt-get install cmake ninja-build clang lld libx11-dev libxfixes-dev libegl-dev libgbm-dev libfontconfig-dev
```

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

## I want to modify code (Browser & WebXR)

StereoKit is a cross-platform library and can be compiled to WASM using emscripten to run in the browser!
At this stage there is no WebXR support and so the mode will default to simulator.

The build requirements are:
- Install [cmake](https://cmake.org/) 3.21+.
- Install [ninja](https://ninja-build.org/): `winget install Ninja-build.Ninja` on Windows. This is technically optional, but is used in the cmake presets.
- Install the Emscripten SDK.  [here](https://emscripten.org/docs/getting_started/downloads.html) is the install guide. 
- (Optional for Blazor) Install [.NET 9 Preview SDK](https://dotnet.microsoft.com/en-us/download/dotnet/9.00): `winget install Microsoft.DotNet.SDK.Preview` on Windows.
- (Optional for Blazor) Install dotnet SDK workloads: `dotnet workload restore Examples/StereoKitTest/StereoKitTest_Web/StereoKitTest_Web.csproj`

To build with cmake:
```shell
### From StereoKit's root directory ###

# activate emscripten SDK, use a version that matches the .NET Blazor workload tooling (emcc 3.1.34 for .NET 8/9)
emsdk activate 3.1.34

# Build using the presets
cmake --preset Browser_Release
cmake --build --preset Browser_Release

# To run the C# Blazor test app, you will need the .NET 9 Preview SDK installed and the Blazor workloads
# Start the app with the below and then open a browser to: https://localhost:57955/
dotnet run --configuration Release --project Examples/StereoKitTest/StereoKitTest_Web/StereoKitTest_Web.csproj

# check /bin/distribute for final binary files
```

### Considerations for Blazor

A Blazor WebAssembly application is essentially AOT compiled and then linked using emscripten to generate the wasm.
The .NET StereoKit implementation uses PInvoke to call into the StereoKitC native lib. 

As of .NET 7/8 the PInvoke wasm bindings is limited in it's ability to generate function signatures for wasm.
.NET 9 adds support for structs returned by value and allows the bulk of the StereoKit native API to be used from .NET.

An area that needs improvement is String or varargs when used in native to C#, until then the logging callback functions can not be used.
However, a workaround to explore could be to proxy the callback via Javascript.

## I want to build the whole NuGet package

The NuGet package build pipeline requires all the setup steps from above first! After that, you just need to run the [build powershell script](https://github.com/maluoi/StereoKit/blob/master/Build-Nuget.ps1). This script will build all binary variants, run tests, and track some statistics.
