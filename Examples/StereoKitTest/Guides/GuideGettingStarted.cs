/// :CodeDoc: Guides 0 Getting Started
/// # Getting Started with StereoKit
/// 
/// ## Prerequisites
/// 
/// Here's a quick list of what you'll need to start developing with StereoKit:
/// 
/// - **[Visual Studio 2019](https://visualstudio.microsoft.com/vs/) - Use these workloads:**
///   - .NET Desktop development
///   - Universal Windows Platform development (for HoloLens)
///   - Mobile development with .Net (for Quest)
/// - **[StereoKit's Visual Studio Template](https://marketplace.visualstudio.com/items?itemName=NickKlingensmith.StereoKitTemplates)**
///   - Experienced users might directly use the [NuGet package](https://www.nuget.org/packages/StereoKit).
/// - **Any OpenXR runtime**
///   - A flatscreen fallback is available for development.
/// - **Enable Developer Mode (for UWP/HoloLens)**
///   - Windows Settings->Update and Security->For Developers->Developer Mode
/// 
/// ## The Templates
/// 
/// ![Create New Project]({{site.url}}/img/screenshots/VSNewProject.png)
/// 
/// - **StereoKit .Net Core**
///   - .Net Core is for desktop XR on Windows and Linux. It is simple, compiles quickly, and is the best option for most developers.
/// - **StereoKit UWP**
///   - UWP is for HoloLens 2, and can run on Windows desktop. UWP can be slower to compile, and is no longer receiving updates from the .Net team.
/// - _[Cross Platform Template (in development)](https://github.com/maluoi/StereoKit/tree/master/StereoKitTemplates/StereoKitXPlat)_
///   - This is an early version still in project format. It works with .Net Core, UWP, and Xamarin(Android/Quest) all at once via a DLL shared between multiple platform specific projects.
/// - _[Native C++ Template (in development)](https://github.com/maluoi/StereoKit/tree/master/StereoKitTemplates/SKNative)_
///   - StereoKit does provide a C API, but experienced developers should only choose this if the benefits outweigh the lack of C API documentation.
/// 
/// ## Minimum "Hello Cube" Application
/// 
/// The template does provide some code to help provide new developers a base
/// to work from, but what parts of the code are really necessary? We can boil
/// "Hello Cube" down to something far simpler if we want to! This is the
/// simplest possible StereoKit application:
/// 
using System;
using StereoKit;

class Program
{
	static void Main(string[] args)
	{
		if (!SK.Initialize(new SKSettings{ appName = "Project" }))
			Environment.Exit(1);

		while (SK.Step(() =>
		{
			Mesh.Cube.Draw(Material.Default, Matrix.S(0.1f));
		}));

		SK.Shutdown();
	}
}
/// 
/// ## Next Steps
/// 
/// Awesome! That's pretty great, but what next? [Why don't we build some UI]({{site.url}}/Pages/Guides/User-Interface.html)?
/// Alternatively, you can check out the [StereoKit Ink](https://github.com/maluoi/StereoKit-PaintTutorial)
/// repository, which contains an XR ink-painting application written in about
/// 220 lines of code! It's well commented, and is a good example to pick
/// through.
/// 
/// For additional learning resources, you can check out the [Learning Resources]({{site.url}}/Pages/Guides/Learning-Resources.html)
/// page for a couple of repositories and links that may help you out. In
/// particular, the Github repository does contain a [number of small demo scenes](https://github.com/maluoi/StereoKit/tree/master/Examples/StereoKitTest/Demos)
/// that are excellent reference for a number of different StereoKit features!
/// 
/// And don't forget to peek in the docs here! Most pages contain sample code
/// that illustrates how a particular function or property is used
/// in-context. The ultimate goal is to have a sample for 100% of the docs,
/// so if you're looking for one and it isn't there, use the 'Create an Issue'
/// link at the bottom of the web page to get it prioritized!
/// :End: