<p align="center"><img src="/Tools/img/StereoKitWide.svg" alt="StereoKit Logo" height="160"></p>
<p align="center">
    <a href="https://github.com/maluoi/stereokit/branches"><img src="https://img.shields.io/github/last-commit/maluoi/stereokit/develop" /></a>
    <a href="https://tldrlegal.com/license/mit-license"><img src="https://img.shields.io/github/license/maluoi/stereokit" /></a>
    <a href="https://www.nuget.org/packages/StereoKit/"><img src="https://img.shields.io/nuget/v/StereoKit" /></a>
    <a href="https://marketplace.visualstudio.com/items?itemName=NickKlingensmith.StereoKitTemplates"><img src="https://img.shields.io/visual-studio-marketplace/i/NickKlingensmith.StereoKitTemplates" /></a>
</p>

StereoKit is an easy-to-use open source mixed reality library for building HoloLens and VR applications with C# and OpenXR! Inspired by libraries like XNA and Processing, StereoKit is meant to be fun to use and easy to develop with, yet still quite capable of creating professional and business ready software.

The getting started guide [can be found here](https://stereokit.net/Pages/Guides/Getting-Started.html)!

Interested in news and updates about StereoKit? Maybe just looking for some extra help?
- Submit bugs on the [Issues tab](https://github.com/maluoi/StereoKit/issues), and ask questions in the [Discussions tab](https://github.com/maluoi/StereoKit/discussions)!
- Follow [koujaku](https://twitter.com/koujaku/) on Twitter for development news.
- Drop into the [Discord channel](https://discord.gg/jtZpfS7nyK) for discussion, help and updates.
- Check out [this blog](https://playdeck.net/project/stereokit) for occasional substantial updates!

![Screenshot](/Tools/img/FeatureImage.jpg)

## StereoKit Features:
- Platforms: HoloLens 2, Oculus Quest, Windows Mixed Reality, Oculus Desktop, SteamVR, Monado (Linux), and eventually everywhere OpenXR is!
- Flat screen MR simulator with input emulation for easy development
- Builds your application to device in seconds, not minutes
- Mixed Reality inputs like hands and eyes are trivial to access
- Easy and powerful UI and interactions
- Model formats: .gltf, .glb, .obj, .stl, ASCII .ply, procedural
- Texture formats: .jpg, .png, .tga, .bmp, .psd, .gif, .hdr, .pic, .qoi, equirectangular cubemap, procedural
- Runtime asset loading
- Physics
- Performance-by-default instanced render pipeline
- Flexible shader/material system with built-in PBR
- Skeletal/skinned animation
- [Documentation](https://stereokit.net/) generated directly from the source code, including screenshots

## Getting started

Follow [this guide](https://stereokit.net/Pages/Guides/Getting-Started.html) for a detailed introduction! This repository is the raw source for those who wish to build StereoKit themselves, the Visual Studio templates and the NuGet packages referenced in the guide are how most people should build their applications!

StereoKit focuses on getting you productive with the least amount of code possible. You can actually do most tasks with a single line of code, including UI! Here's hello world with StereoKit, this is all you need to get up and running!

```CSharp
using StereoKit;

class Program
{
	static void Main(string[] args)
	{
		SK.Initialize(new SKSettings{ appName = "Project" });

		Model helmet = Model.FromFile("Assets/DamagedHelmet.gltf");

		while (SK.Step(() => {
			helmet.Draw(Matrix.TS(Vec3.Zero, 0.1f));
		}));

		SK.Shutdown();
	}
}
```
![Hello World](/Tools/img/StereoKitMin.gif)

## Building or Contributing

For those that wish to build the test project, or perhaps the whole library, there's a [guide to building StereoKit's repository](/BUILDING.md)! For those that wish to contribute features or fixes to StereoKit, awesome! There will soon be a contributor guide to help you navigate style and architecture requirements. In the meantime, check with the maintainer on either GitHub Issues or the Discord channel to help make sure contributions meet the project's standards.

## Dependencies

Just like all software, StereoKit is built on the shoulders of incredible people! Here's a list of the libraries StereoKit uses to get things done.

- [OpenXR](https://www.khronos.org/openxr/)
- [ReactPhysics3D](https://www.reactphysics3d.com/) - physics
- [cgltf](https://github.com/jkuhlmann/cgltf) - gltf format support
- [Sean Barrett's stb libraries](https://github.com/nothings/stb) - image and font format support
- [QOI](https://github.com/phoboslab/qoi) - image format support
- [miniaudio](https://github.com/dr-soft/miniaudio) - audio playback
- [sokol_time](https://github.com/floooh/sokol) - cross platform high performance time
- [TinyCThread](https://github.com/tinycthread/tinycthread) - for portable threading

And some of my own libraries that I maintain separately from this repository.

- [sk_gpu.h](https://github.com/maluoi/sk_gpu) - cross platform graphics API
- [ferr_hash.h](https://github.com/maluoi/header-libs) - string and data hashing
- [array.h](https://github.com/maluoi/header-libs) - dynamic arrays