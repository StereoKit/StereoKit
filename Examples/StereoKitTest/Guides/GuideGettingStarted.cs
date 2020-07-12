/// :CodeDoc: Guides Getting Started
/// # Getting Started with StereoKit
/// 
/// ## Prerequisites
/// 
/// Before we get started, you'll need some software! You may have some of this already, but here's the list!
/// 
/// - [Visual Studio 2019](https://visualstudio.microsoft.com/vs/) - not 2017, the Community version is great! Use these workloads:
///   - .NET Desktop development
///   - Universal Windows Platform development
///   - .NET Core cross-platform development
/// - [Windows OpenXR runtime](https://www.microsoft.com/store/productId/9n5cvvl23qbt) - Install *and enable* this on your desktop and device!
/// - Or Oculus's preview runtime
///   - Oculus's OpenXR runtime has a trickier setup, since it's still in preview. See [here](https://github.com/maluoi/StereoKit/issues/41#issuecomment-574438421) for details!
/// - Or SteamVR's preview runtime
///   - Setup is [here](https://store.steampowered.com/newshub/app/250820/view/2396425843528787269). SteamVR's runtime is young still, I've run into issues when using it with WMR/Oculus headsets.
/// - [StereoKit's Visual Studio Template](https://marketplace.visualstudio.com/items?itemName=NickKlingensmith.StereoKitTemplates) - The fastest way to set up a StereoKit project!
///   - You can also directly install the [NuGet package](https://www.nuget.org/packages/StereoKit) into your project of choice.
/// - Enable Developer Mode (for UWP/HoloLens builds)
///   - Windows Settings->Update and Security->For Developers->Developer Mode
/// 
/// The source for StereoKit is [available on Github](https://github.com/maluoi/StereoKit) if you're interested in peeking
/// at the code or modifying it yourself, but it's a bit more complex to dive into! If you're just getting started 
/// with StereoKit, you won't need anything from the repository to get up and running :)
/// 
/// ## The Template
/// 
/// ![Create New Project]({{site.url}}/img/screenshots/VSNewProject.png)
/// 
/// There are two templates to pick from! One is for UWP, and one is for .NET Core. Choose the UWP one if you're deploying
/// to HoloLens 2, and choose the .NET Core one if you're doing development on a Windows Mixed Reality headset! When using
/// UWP, iterating is fastest with Debug/x64, using desktop VR or the flatscreen mode.
/// 
/// The starting code is pretty simple! Initialize StereoKit in Mixed Reality mode, create a 3D model that's a rounded
/// cube with a default material, and then draw that model every step of the application!
/// 
/// With that, you automatically get hands, environment, and lighting right away!
/// 
using System;
using StereoKit;

class Program
{
    static void Main(string[] args)
    {
        if (!StereoKitApp.Initialize("Project", Runtime.MixedReality))
            Environment.Exit(1);

        Model cube = Model.FromMesh(
            Mesh.GenerateRoundedCube(Vec3.One, 0.2f),
            Default.Material);

        while (StereoKitApp.Step(() =>
        {
            cube.Draw(Matrix.TS(Vec3.Zero, 0.1f));
        }));

        StereoKitApp.Shutdown();
    }
}
/// 
/// Awesome! That's pretty easy, but what next? [Why don't we build some UI]({{site.url}}/Pages/Guides/User-Interface.html)?
/// Alternatively, you can check out the [sample painting application](https://github.com/maluoi/StereoKit-PaintTutorial)
/// repository, which contains a finger-painting application written in about 220 lines of code! It's well commented, and is
/// a good example to pick through.
/// :End: