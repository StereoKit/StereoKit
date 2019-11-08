/// :CodeDoc: Guides Getting Started
/// # Getting Started with StereoKit
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
            if (!StereoKitApp.Initialize("StereoKit C#", Runtime.MixedReality, true))
                Environment.Exit(1);

            Model cube = new Model(Mesh.GenerateRoundedCube(Vec3.One, 0.1f), Material.Find("default/material"));

            while (StereoKitApp.Step(() =>
            {
                Renderer.Add(cube, Matrix.Identity, Color.White);
            }));

            StereoKitApp.Shutdown();
        }
    }
}
/// 
/// Awesome! That's pretty easy, but what next? Let's build some UI! (guide coming soon)
/// :End: