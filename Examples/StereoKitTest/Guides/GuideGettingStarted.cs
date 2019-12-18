/// :CodeDoc: Guides Getting Started
/// # Getting Started with StereoKit
/// 
/// ## Prerequisites
/// 
/// Before we get started, you'll need some software! You may have some of this already, but here's the list!
/// 
/// [Visual Studio 2019](https://visualstudio.microsoft.com/vs/) - the Community version is great! But it doesn't
/// matter which one, so long as it's not VS 2017. Visual Studio Code also won't work, that's a totally
/// separate product :)
/// 
/// This also comes with some extra configuration! While installing Visual Studio, you'll need to pick a
/// few workloads. Try '.NET Desktop development', 'Universal Windows Platform development' and '.NET Core 
/// cross-platform development'. That should cover it, but I may refine this list later on!
/// 
/// [OpenXR Runtime](https://www.microsoft.com/store/productId/9n5cvvl23qbt) - StereoKit is powered by OpenXR! You'll
/// want this installed and enabled on any device running your application! If you're deploying to a HoloLens 2, this
/// may already be pre-installed there, just make sure it's up-to-date.
/// 
/// ## Setup
/// 
/// The recommended way to set up StereoKit is through the [Visual Studio Template](https://github.com/maluoi/StereoKit/raw/master/Documentation/SKTemplates.vsix)!
/// This is a really easy way to get set up, and will have you started in no time! For existing projects or 
/// more advanced users, you can directly use the [NuGet package](https://www.nuget.org/packages/StereoKit) that's used
/// by the template.
/// 
/// The source for StereoKit is [available on Github](https://github.com/maluoi/StereoKit) if you're interested in peeking
/// at the code or modifying it yourself, but it's a bit more complex to dive into! If you're just getting started 
/// with StereoKit, you won't need anything from the repository to get up and running :)
/// 
/// ## The Template
/// 
/// There are two templates to pick from! One is for UWP, and one is for .NET Core. Choose the UWP one if you're deploying
/// to HoloLens 2, and choose the .NET Core one if you're doing development on a Windows Mixed Reality headset! When using
/// UWP, .Net Native is used for Release builds and all ARM64 builds, so it can be faster to iterate in Debug/x64!
/// 
/// The starting code is pretty simple! Initialize StereoKit in Mixed Reality mode, create a 3D model that's a rounded
/// cube with a default material, and then draw that model every step of the application!
/// 
/// With that, you automatically get hands, environment, and lighting right away!
/// 
using System;
using StereoKit;

namespace SKYourTemplate
{
    class Program
    {
        static void Main(string[] args)
        {
            if (!StereoKitApp.Initialize("SKYourTemplate", Runtime.MixedReality))
                Environment.Exit(1);

            Model cube = Model.FromMesh(
                Mesh.GenerateRoundedCube(Vec3.One, 0.2f),
                Material.Find(DefaultIds.material));

            while (StereoKitApp.Step(() =>
            {
                cube.Draw(Matrix.TS(Vec3.Zero, 0.1f));
            }));

            StereoKitApp.Shutdown();
        }
    }
}
/// 
/// Awesome! That's pretty easy, but what next? [Let's build some UI]({{site.url}}/Pages/Guides/User-Interface.html)!
/// :End: