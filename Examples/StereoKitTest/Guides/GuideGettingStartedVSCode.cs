/// :CodeDoc: Guides 1 Getting Started VS Code
/// # Getting Started with StereoKit - Visual Studio Code
/// 
/// StereoKit's onboarding has generally been optimized for Visual Studio
/// 'Prime', but VS Code definitely has some benefits, like running on
/// platforms other than windows! This path will be expanded on in the future,
/// but here's a quick way to get started on it right now.
/// 
/// ![Creating a new StereoKit project in VS Code](/img/StereoKitDotnetWoah.gif)
/// 
/// This guide also serves as a way to get started with C# projects in a
/// command line environment! VS Code may have additional extensions that can
/// make this experience simpler.
/// 
/// This requires having [the .NET **SDK**](https://dotnet.microsoft.com/en-us/download)
/// installed on your machine. Some development setups may already have this
/// installed, you can try running `dotnet --version` to double check!
/// 
/// To create the project:
/// ```shell
/// mkdir ProjectName
/// cd ProjectName
/// 
/// dotnet new console
/// dotnet add package StereoKit
/// ```
/// 
/// Add some code to get started:
/// ```csharp
/// using StereoKit;
/// 
/// SK.Initialize();
/// SK.Run(()=>{
///     Mesh.Sphere.Draw(Material.Default, Matrix.S(0.1f));
/// });
/// ```
/// 
/// To run the project:
/// ```shell
/// # For .NET's hot-reload functionality
/// dotnet watch
/// 
/// # Or just a normal run
/// dotnet run
/// ```
/// 
/// :End: