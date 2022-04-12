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
- StereoKitTest_Android
- StereoKitCTest_Android

Set StereoKitTest as the startup project, set the platform to x64, and you should be able to build it and run! On the first build, StereoKit will also clone and build `openxr_loader` as well as `ReactPhysics3D` using a [powershell script](https://github.com/maluoi/StereoKit/blob/master/Tools/Update-OpenXR.ps1) and cmake, so this will take some extra time.

## I want to modify code (Android)

In addition to the Windows steps, to build for Android, StereoKit uses [xmake](https://xmake.io/). You will need to install it and configure it for building Android applications. You will probably need to tell it where the Android SDK is on your system. I've technically done this at least once, but don't remember it well enough to write a good guide on it.

I may update this guide eventually, but if you can track what steps you take while doing this and submit a PR, that would be really incredible!

## I want to modify code (Linux)

StereoKit builds Linux code using xmake running on Windows Subsystems for Linux. So set up WSL2 on your system, and install xmake on it. You may also need to install a number of various packages to get it to build correctly, but I don't have that list easily on hand.

It may help to build just the C++ side on Linux first to find which packages need to be installed. Here's how you build StereoKit using xmake:

```
# StereoKit uses xmake, check their docs (https://xmake.io/#/getting_started)
# for details, but here's the quick way to install:
bash <(curl -fsSL https://xmake.io/shget.text)

# here's a few other pre-reqs, mostly pulled from OpenXR's build list:
sudo apt-get update
sudo apt-get install build-essential cmake unzip libfontconfig1-dev libgl1-mesa-dev libvulkan-dev libx11-xcb-dev libxcb-dri2-0-dev libxcb-glx0-dev libxcb-icccm4-dev libxcb-keysyms1-dev libxcb-randr0-dev libxrandr-dev libxxf86vm-dev mesa-common-dev

### From StereoKit's root directory ###

# Configure the xmake build
xmake f -p linux -a x64 --tests=y -y
# Build
xmake

# Run the project with software graphics on WSL
LIBGL_ALWAYS_SOFTWARE=1 xmake run

# Or, real Linux often has more recent graphics drivers
xmake run
```

## I want to build the whole NuGet package

The NuGet package build pipeline requires all the setup steps from above first! After that, you just need to run the [build powershell script](https://github.com/maluoi/StereoKit/blob/master/Build-Nuget.ps1). This script will build all binary variants, run tests, and track some statistics.
