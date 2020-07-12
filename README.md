![StereoKit Logo](/Documentation/img/StereoKitWide.svg)

StereoKit is an easy-to-use open source mixed reality library for building HoloLens and VR applications with C# and OpenXR!

The getting started guide [can be found here](https://stereokit.net/Pages/Guides/Getting-Started.html)!

StereoKit is ready to use, but still early in its life! Check out [this Twitter thread](https://twitter.com/koujaku/status/1163977987705860097) for development news and gifs, or check [this blog](https://playdeck.net/project/stereokit) for more substantial updates!

StereoKit is designed to solve issues that occur when using a game engine to create a Mixed Reality application or tool. Game engines are one of the few places you can currently go to get rendering functionality out of the box, but they lack features that would really streamline MR application development!

![Screenshot](/Documentation/img/FeatureImage.jpg)

StereoKit Features:
- Platforms: HoloLens 2, Windows Mixed Reality, Oculus Desktop, SteamVR, eventually everywhere OpenXR is!
- Builds your application in seconds, not minutes
- Input: articulated hands, pointers, keyboard/mouse
- Easy and powerful UI and interactions
- Model formats: .gltf, .glb, .fbx(partial), .obj, .stl, procedural
- Texture formats: .jpg, .png, .tga, .bmp, .psd, .gif, .hdr, .pic, equirectangular cubemap, procedural
- Runtime asset loading
- Physics
- Performance-by-default render pipeline
- Flexible shader/material system
- [Documentation](https://stereokit.net/) is generated directly from the source code, including screenshots

## Getting started

Follow [this guide](https://stereokit.net/Pages/Guides/Getting-Started.html) for a detailed introduction! This repository is the raw source for those who wish to build StereoKit themselves, the Visul Studio templates and the NuGet packages referenced in the guide are how most people should build their applications! 

StereoKit focuses on getting you productive with the least amount of code possible. You can actually do most tasks with a single line of code, including UI! Here's hello world with StereoKit, this is all you need to get up and running!

![Hello World](/Documentation/img/StereoKitMin.gif)

## Roadmap

Where is StereoKit going next? That depends on you! What do you need? Is StereoKit missing something that it really should have? File an issue and let us know!

In the short term, StereoKit will focus on the Core API, improving performance, and ensuring all basic features are present and work well! Some basic tooling, things like a visual shader editor might get built here. The Core of StereoKit is an Immediate Mode system that does not provide any framework or application state management.

Long term, we'd love to add StereoKit Framework! Framework is a higher-level layer that manages application state, so more complicated functionality can be taken care of. Features such as automatic multi-user capabilities, component systems, WYSIWYG UI design tools, visual scene editors and code-free design environments will become possible.

## Dependencies

 Just like all software, StereoKit is built on the shoulders of incredible people! Here's a list of the libraries StereoKit uses to get things done.

 - [OpenXR](https://www.khronos.org/openxr/)
 - DirectX 11
 - [ReactPhysics3D](https://www.reactphysics3d.com/) - physics
 - [cgltf](https://github.com/jkuhlmann/cgltf) - gltf format support
 - [Sean Barrett's stb libraries](https://github.com/nothings/stb) - image and font format support
 - [miniaudio](https://github.com/dr-soft/miniaudio) - audio playback
 - [dr_wav](https://mackron.github.io/dr_wav) - wav format support
