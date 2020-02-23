---
layout: default
title: StereoKit
description: StereoKit is an easy-to-use mixed reality library for building HoloLens and VR applications with C# and OpenXR!
---

StereoKit is an easy-to-use mixed reality library for building HoloLens and VR applications with C# and OpenXR!

The getting started guide [can be found here]({{site.url}}/Pages/Guides/Getting-Started.html)! And a full example StereoKit application [can be found here on Github](https://github.com/maluoi/StereoKit-PaintTutorial)!

StereoKit is ready to use, but still early in its life! Keep track on [Twitter](https://twitter.com/koujaku/) for development news and gifs, or check [this blog](https://playdeck.net/project/stereokit) for more substantial updates! Can't find a feature you need for your project? Request it on [the issues page](https://github.com/maluoi/StereoKit/issues), and we'll prioritize getting you up and running!

StereoKit is designed to solve issues that occur when using a game engine to create a Mixed Reality application or tool. Game engines are one of the few places you can currently go to get rendering functionality out of the box, but they lack features that would really streamline MR application development!

![Screenshot]({{site.screen_url}}/FeatureImage.jpg)

StereoKit Features:
- Model formats: .gltf, .glb, .obj, .stl, procedural
- Texture formats: .jpg, .png, .tga, .bmp, .psd, .gif, .hdr, .pic, equirectangular cubemap, procedural
- Runtime asset loading
- Platforms: HoloLens 2, Windows Mixed Reality, eventually everywhere OpenXR is!
- Builds your application in seconds, not minutes
- Input: articulated hands, pointers, keyboard/mouse
- Physics
- Easy and powerful UI and interactions
- Performance-by-default render pipeline
- Flexible shader/material system
- All of the documentation is tested and generated from code, including screenshots

## Getting started

Follow [this guide]({{site.url}}/Pages/Guides/Getting-Started.html) for a detailed introduction! This repository is the raw source for those who wish to build StereoKit themselves, the Visul Studio templates and the NuGet packages referenced in the guide are how most people should build their applications! 

StereoKit focuses on getting you productive with the least amount of code possible. You can actually do most tasks with a single line of code, including UI! Here's hello world with StereoKit, this is all you need to get up and running!

Interested in the [source for StereoKit](https://github.com/maluoi/StereoKit)? It's open and MIT licensed! You don't need to build from source to use it, but if you want to, it's there for you to modify or debug with!

## Roadmap

Where is StereoKit going next? That depends on you! What do you need? Is StereoKit missing something that it really should have? File an issue and let us know!

In the short term, StereoKit will focus on the Core API, improving performance, and ensuring all basic features are present and work well! Some basic tooling, things like a visual shader editor might get built here. The Core of StereoKit is an Immediate Mode system that does not provide any framework or application state management.

Long term, we'd love to add StereoKit Framework! Framework is a higher-level layer that manages application state, so more complicated functionality can be taken care of. Features such as automatic multi-user capabilities, component systems, WYSIWYG UI design tools, visual scene editors and code-free design environments will become possible.

## Issue reports

We'd love to hear if you've run into an issue! The best way to let us know is to file an issue [on the Github repository](https://github.com/maluoi/StereoKit/issues). For an even faster response, file your reports with an `ITest` that can reproduce the issue, so we can quickly fix and prevent regressions! [Here's an example of such a test](https://github.com/maluoi/StereoKit/blob/develop/Examples/StereoKitTest/Tests/TestTextureCrash.cs).