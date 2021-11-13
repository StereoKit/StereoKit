---
layout: default
title: StereoKit
description: StereoKit is an easy-to-use open source mixed reality library for building HoloLens and VR applications with C# and OpenXR!
---
<table style="max-width:34em;">
<tr><td style="width:25%;">
<img src="{{site.url}}/img/StereoKitLogoLight.svg" alt="StereoKit Logo" />
</td><td><p>StereoKit is an easy-to-use open source mixed reality library for building HoloLens and VR applications with C# and OpenXR!</p><p><b><a href="https://marketplace.visualstudio.com/items?itemName=NickKlingensmith.StereoKitTemplates" target="_blank">Install</a></b> - <a href="{{site.url}}/Pages/Guides/Getting-Started.html">Get Started Guide</a> - <a href="https://github.com/maluoi/StereoKit/" target="_blank">View Source</a></p></td></tr></table>

<video autoplay loop muted><source src='{{site.url}}/img/SKHighlightReel.mp4' type='video/mp4'>Your browser doesn't seem to play .mp4s!</video>

## StereoKit Features:
- Platforms: HoloLens 2, Oculus Quest, Windows Mixed Reality, Oculus Desktop, SteamVR, Varjo, Monado Linux, and eventually everywhere OpenXR is!
- Flat screen mode with input emulation for easy development
- Builds your application to device in seconds, not minutes
- Mixed Reality inputs like hands and eyes are trivial to access
- Easy and powerful UI and interactions
- Model formats: .gltf, .glb, .obj, .stl, ASCII .ply, procedural
- Texture formats: .jpg, .png, .tga, .bmp, .psd, .gif, .hdr, .pic, equirectangular cubemap, procedural
- Runtime asset loading
- Physics
- Performance-by-default instanced render pipeline
- Flexible shader/material system with built-in PBR
- Skeletal/skinned animation
- Documentation generated directly from the source code, including screenshots

## About

StereoKit prioritizes mixed reality application development above all else! This allows us to focus on features such as a first class mixed reality input system, fast performance by default even on mobile devices, quick iteration time on-device, and a runtime asset pipeline that lets users and developers load real assets from the file-system. All of this and more are packaged in a terse API that's well documented, easy to learn, and easy to write.

StereoKit is ready to use, but still early in its life! Keep track on [Twitter](https://twitter.com/koujaku/) for development news and gifs, or check [this blog](https://playdeck.net/project/stereokit) for more substantial updates! Can't find a feature you need for your project? Request it on [the issues page](https://github.com/maluoi/StereoKit/issues), and we'll prioritize getting you up and running!

## Getting started

Follow [this guide]({{site.url}}/Pages/Guides/Getting-Started.html) for a detailed introduction! Or check out the [additional learning resources]({{site.url}}/Pages/Guides/Learning-Resources.html), sample projects that show how to build full StereoKit applications.

StereoKit focuses on getting you productive with the least amount of code possible. You can actually do most tasks with a single line of code, including UI! Here's hello world with StereoKit, this is all you need to get up and running!

```csharp
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
![Hello World]({{site.url}}/img/StereoKitMin.gif)

Interested in the [source for StereoKit](https://github.com/maluoi/StereoKit)? It's open and MIT licensed! You don't need to build from source to use it, but if you want to, it's there for you to modify or debug with!

## Roadmap

Where is StereoKit going next? That depends on you! What do you need? Is StereoKit missing something that it really should have? File an issue and let us know!

In the short term, StereoKit will focus on the Core API, improving performance, and ensuring all basic features are present and work well! Some basic tooling, things like a visual shader editor might get built here. The Core of StereoKit is an Immediate Mode system that does not provide any framework or application state management.

Long term, we'd love to add StereoKit Framework! Framework is a higher-level layer that manages application state, so more complicated functionality can be taken care of. Features such as automatic multi-user capabilities, component systems, WYSIWYG UI design tools, visual scene editors and code-free design environments will become possible.

## Issue reports

We'd love to hear if you've run into an issue! The best way to let us know is to file an issue [on the Github repository](https://github.com/maluoi/StereoKit/issues). For an even faster response, file your reports with an `ITest` that can reproduce the issue, so we can quickly fix and prevent regressions! [Here's an example of such a test](https://github.com/maluoi/StereoKit/blob/develop/Examples/StereoKitTest/Tests/TestTextureCrash.cs).