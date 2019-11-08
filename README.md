![StereoKit Logo](/Documentation/img/StereoKitWide.svg)

A lightweight low-dependency C# rendering library for MR / AR / VR / Desktop using OpenXR!

StereoKit is still early in development, but an initial version is coming soon! Check out [this Twitter thread](https://twitter.com/koujaku/status/1163977987705860097) for development news and gifs.

The future of development is coming, and we need better tools! StereoKit aims to be forward thinking in its design, keep its interface simple, take advantage of more modern hardware features common to Mixed Reality devices, and be much easier to use for general computing tasks! (ie. not necessarily games)

![Screenshot](/Documentation/img/SKScreenshot1.jpg)

StereoKit Features:
- Model formats: .gltf, .glb, .obj, procedural
- Texture formats: .jpg, .png, .tga, .bmp, .psd, .gif, .hdr, .pic, equirectangular cubemap, procedural
- Runtime asset loading
- Platforms: HoloLens 2, Windows Mixed Reality, eventually everywhere OpenXR is!
- Less than 10 second total build/deploy/load time for HoloLens 2
- Input: Hands, Pointers, Keyboard/Mouse
- Physics
- Easy but powerful UI and Interactions
- Performance-by-default render pipeline
- Flexible shader/material system
- [Documentation](https://maluoi.github.io/StereoKit) as unit testing

## Getting started

While StereoKit is still in development, you can totally try it out already! The core is fairly well established, and everything should be fairly easy to use! Check out the [getting started](https://maluoi.github.io/StereoKit/Pages/Guides/Getting-Started.html) guide on the docs site for in-depth documentation, or check out the [demo project](https://github.com/maluoi/StereoKit/tree/master/Examples/StereoKitTest) in the repository! But we'll do a quick guide right here too.

Setting up to use StereoKit is a simple as installing the [StereoKit Templates](https://github.com/maluoi/StereoKit/raw/master/Documentation/SKTemplates.vsix) for Visual Studio and creating a new StereoKit project! (Some Visual Studio pre-reqs may be needed too, I'll make a list of that later)

Saying hello world after that is pretty trivial, check out the code!

```csharp
using System;
using StereoKit;

class Program
{
    static void Main(string[] args)
    {
        if (!StereoKitApp.Initialize("StereoKit C#", Runtime.MixedReality, true))
            Environment.Exit(1);

        Model model = new Model("Assets/DamagedHelmet.gltf");

        while (StereoKitApp.Step(() =>
        {
            Renderer.Add(model, Matrix.Identity, Color.White);
        }));

        StereoKitApp.Shutdown();
    }
}
```

Pretty straightforward, right? That'll give you alll of this right away, lighting, hands, everything!

![Hello World](/Documentation/img/StereoKitMin.gif)