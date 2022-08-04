---
layout: default
title: StereoKit
description: StereoKit is an easy-to-use open source mixed reality library for building HoloLens and VR applications with C# and OpenXR!
---
<table style="max-width:34em;">
<tr><td style="width:25%;">
<img src="{{site.url}}/img/StereoKitLogoLight.svg" alt="StereoKit Logo" />
</td><td><p>StereoKit is an easy-to-use open source mixed reality library for building HoloLens and VR applications with C# and OpenXR!</p><p><b><a href="https://marketplace.visualstudio.com/items?itemName=NickKlingensmith.StereoKitTemplates" target="_blank">Install</a></b> - <a href="{{site.url}}/Pages/Guides/Getting-Started.html">Get Started Guide</a> - <a href="https://github.com/StereoKit/StereoKit/" target="_blank">View Source</a></p></td></tr></table>

<video autoplay loop muted><source src='{{site.url}}/img/SKHighlightReel.mp4' type='video/mp4'>Your browser doesn't seem to play .mp4s!</video>

## StereoKit Features:
- Platforms: HoloLens 2, Oculus Quest, Windows Mixed Reality, Oculus Desktop, SteamVR, Varjo, Monado (Linux), and eventually everywhere OpenXR is!
- Mixed Reality inputs like hands and eyes are trivial to access
- Easy and powerful UI and interactions
- Model formats: .gltf, .glb, .obj, .stl, ASCII .ply
- Texture formats: .jpg, .png, .tga, .bmp, .psd, .gif, .hdr, .pic, .qoi, cubemaps
- Flexible shader/material system with built-in PBR
- Performance-by-default instanced render pipeline
- Skeletal/skinned animation
- Flat screen MR simulator with input emulation for easy development
- Builds your application to device in seconds, not minutes
- Runtime asset loading and cross-platform file picking
- Physics
- Documentation generated directly from the source code, including screenshots

## About

StereoKit prioritizes mixed reality application development above all else! This allows us to focus on features such as a first class mixed reality input system, fast performance by default even on mobile devices, quick iteration time on-device, and a runtime asset pipeline that lets users and developers load real assets from the file-system. All of this and more are packaged in a terse API that's well documented, easy to learn, and easy to write.

StereoKit is ready to use, but still early in its life! Keep track on [Twitter](https://twitter.com/koujaku/) for development news and gifs, or check [this blog](https://playdeck.net/project/stereokit) for more substantial updates! Can't find a feature you need for your project? Request it on [the issues page](https://github.com/StereoKit/StereoKit/issues), and we'll prioritize getting you up and running!

While StereoKit is primarily intended to be consumed from C#, all core functionality is implemented in native code, and a C compatible header file is also available for C/C++ developers!

## Getting started

Follow [this guide]({{site.url}}/Pages/Guides/Getting-Started.html) for a detailed introduction! Or check out the [additional learning resources]({{site.url}}/Pages/Guides/Learning-Resources.html), sample projects that show how to build full StereoKit applications.

StereoKit focuses on getting you productive with the least amount of code possible. You can actually do most tasks with a single line of code, including UI! Here's hello world with StereoKit, this is all you need to get up and running!

```csharp
using StereoKit;

SK.Initialize(new SKSettings{ appName = "Project" });

Model helmet = Model.FromFile("Assets/DamagedHelmet.gltf");

SK.Run(() => {
    helmet.Draw(Matrix.TS(Vec3.Zero, 0.1f));
});
```
![Hello World]({{site.url}}/img/StereoKitMin.gif)

Interested in the [source for StereoKit](https://github.com/StereoKit/StereoKit)? It's open and MIT licensed! You don't need to build from source to use it, but if you want to, it's there for you to modify or debug with!

## Building or Contributing

For those that want to build the test project, or perhaps the whole library from scratch, there's a [guide to building StereoKit's repository](https://github.com/StereoKit/StereoKit/BUILDING.md)! For those that wish to contribute features or fixes to StereoKit, awesome! Check out the [contributor's guide](https://github.com/StereoKit/StereoKit/CONTRIBUTING.md). In addition, you may want to check with the maintainers on either GitHub Issues or the Discord channel to help make sure contributions meet the project's standards.

## Issue reports

We'd love to hear if you've run into an issue! The best way to let us know is to file an issue [on the Github repository](https://github.com/StereoKit/StereoKit/issues). For an even faster response, file your reports with an `ITest` that can reproduce the issue, so we can quickly fix and prevent regressions! [Here's an example of such a test](https://github.com/StereoKit/StereoKit/blob/develop/Examples/StereoKitTest/Tests/TestTextureCrash.cs).