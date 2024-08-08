<p align="center"><img src="/tools/img/StereoKitWideBackground.svg" alt="StereoKit Logo" height="160"></p>
<p align="center">
    <a href="https://github.com/StereoKit/StereoKit/branches"><img src="https://img.shields.io/github/last-commit/StereoKit/StereoKit/develop" /></a>
    <a href="https://tldrlegal.com/license/mit-license"><img src="https://img.shields.io/github/license/StereoKit/StereoKit" /></a>
    <a href="https://www.nuget.org/packages/StereoKit/"><img src="https://img.shields.io/nuget/v/StereoKit" /></a>
    <a href="https://www.nuget.org/packages/StereoKit/"><img src="https://img.shields.io/nuget/dt/StereoKit" /></a>
</p>

StereoKit is an easy-to-use open source mixed reality library for building HoloLens and VR applications with C# and OpenXR! Inspired by libraries like XNA and Processing, StereoKit is meant to be fun to use and easy to develop with, yet still quite capable of creating professional and business ready software.

The getting started guide [can be found here](https://stereokit.net/Pages/Guides/Getting-Started.html)!

Interested in news and updates about StereoKit? Maybe just looking for some extra help?

- Submit bugs on the [Issues tab](https://github.com/StereoKit/StereoKit/issues), and ask questions in the [Discussions tab](https://github.com/StereoKit/StereoKit/discussions)!
- Follow [koujaku](https://twitter.com/koujaku/) on Twitter for development news.
- Drop into the [Discord channel](https://discord.gg/jtZpfS7nyK) for discussion, help and updates.
- Check out [this blog](https://playdeck.net/project/stereokit) for occasional substantial updates!

![Screenshot](/tools/screenshots/FeatureImage.jpg)

## StereoKit Features

- Mixed Reality inputs like hands and eyes are trivial to access
- Easy and powerful UI and interactions
- Model formats: .gltf, .glb, .obj, .stl, ASCII .ply
- Texture formats: .jpg, .png, .tga, .bmp, .psd, .gif, .hdr, .pic, .qoi, ktx2, cubemaps
- Flexible shader/material system with built-in PBR
- Performance-by-default instanced render pipeline
- Skeletal/skinned animation
- Flat screen MR simulator with input emulation for easy development
- Builds your application to device in seconds, not minutes
- Runtime asset loading and cross-platform file picking
- [Documentation](https://stereokit.net/) generated directly from the source code, including screenshots

## Platform Support

StereoKit supports HoloLens 2, Oculus Quest, Windows Mixed Reality, Oculus Desktop, SteamVR, Varjo, Monado, and essentially everywhere OpenXR is!

These are the binaries that currently ship in the NuGet package. StereoKit developers working from C/C++ should be fine to build any architecture from this list of platforms.

| Platform | x64 | ARM | ARM64 |
|----------|-----|-----|-------|
| Windows (Desktop/Win32) | X |   | X |
| Windows (HoloLens/UWP)  | X | X | X |
| Linux                   | X |   | X |
| Android                 | X |   | X |

Architecture support has focused on 64 bit architectures, with exceptions for certain platforms. (If you require an additional architecture in the NuGet, please propose it as an Issue!) Here, UWP's ARM builds much faster than UWP's ARM64, and WASM only comes in 32 bit flavors.

## Getting started

Follow [this guide](https://stereokit.net/Pages/Guides/Getting-Started.html) for a detailed introduction! This repository is the raw source for those who wish to build StereoKit themselves, the Visual Studio templates and the NuGet packages referenced in the guide are how most people should build their applications!

StereoKit focuses on getting you productive with the least amount of code possible. You can actually do most tasks with a single line of code, including UI! Here's hello world with StereoKit, this is all you need to get up and running!

```CSharp
using StereoKit;

SK.Initialize();

Model helmet = Model.FromFile("Assets/DamagedHelmet.gltf");

SK.Run(() => {
    helmet.Draw(Matrix.TS(Vec3.Zero, 0.1f));
});
```

![Hello World](/tools/img/StereoKitMin.gif)

## FAQ

- **How do I get started using StereoKit?**
  
  For the majority of users, [go here](https://stereokit.net/Pages/Guides/Getting-Started.html), this is a Visual Studio centric introduction to StereoKit, and contains the most resources for getting started! For VS Code users, [go here](https://stereokit.net/Pages/Guides/Getting-Started-VS-Code.html), and for C/C++ users, [see here](https://stereokit.net/Pages/Guides/Getting-Started-Native.html)!

- **How do I use this repository?**
  
  This is where StereoKit's core SDK code resides, and the majority of users will not need to use it directly. C# developers will rely on [prebuilt packages from NuGet](https://www.nuget.org/packages/StereoKit/) and should refer to the getting started guides for instructions. If you wish to build StereoKit or the samples from scratch, see [the build guide](BUILDING.md)!

- **I would like to contribute to StereoKit! How do I start?**
  
  First off, thank you! :) Get comfortable with [building StereoKit from source](/BUILDING.md), and then check out [our contributor's guide](/CONTRIBUTING.md). Our maintainers are easily available on the StereoKit Discord, as well as GitHub Issues/Discussions! We'll be happy to help you out with whatever's on your mind!

- **Why is the MIT License appropriate for this project?**
  
  The MIT License is a copyright license that is easy to read and has a long history in open source. The MIT License is considered to be a low-friction choice for multiple downstream licensing models, including incorporation into commercial code.

## Dependencies

Just like all software, StereoKit is built on the shoulders of incredible people! Here's a list of the libraries StereoKit uses to get things done.

- [OpenXR](https://www.khronos.org/openxr/)
- [cgltf](https://github.com/jkuhlmann/cgltf) - Gltf format support.
- [meshoptimizer](https://github.com/zeux/meshoptimizer) - Gltf decompression and mesh optimization.
- [basis_universal](https://github.com/BinomialLLC/basis_universal) - Texture compression support.
- [Sean Barrett's stb libraries](https://github.com/nothings/stb) - Image and font format support.
- [QOI](https://github.com/phoboslab/qoi) - Image format support.
- [miniaudio](https://github.com/dr-soft/miniaudio) - Audio playback.
- [sokol_time](https://github.com/floooh/sokol) - Cross platform high performance time.
