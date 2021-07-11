---
layout: default
title: Guides Getting Started
description: Getting Started
---

# Getting Started with StereoKit

## Prerequisites

Before we get started, you'll need some software! You may have some of
this already, but here's the list:

- [Visual Studio 2019](https://visualstudio.microsoft.com/vs/) - the Community version is great, use these workloads:
  - .NET Desktop development
  - Universal Windows Platform development (for HoloLens)
  - Mobile development with .Net (for Quest)
- [StereoKit's Visual Studio Template](https://marketplace.visualstudio.com/items?itemName=NickKlingensmith.StereoKitTemplates) - The fastest way to set up a StereoKit project!
  - You can also directly install the [NuGet package](https://www.nuget.org/packages/StereoKit) into your project of choice.
- Any OpenXR runtime: Windows MR, Oculus, SteamVR, Varjo, Monado, a HoloLens, or an Oculus Quest
- Enable Developer Mode (for UWP/HoloLens)
  - Windows Settings->Update and Security->For Developers->Developer Mode

The source for StereoKit is [available on Github](https://github.com/maluoi/StereoKit)
if you're interested in peeking at the code or modifying it yourself, but
it's a bit more complex to dive into! If you're just getting started with
StereoKit, you won't need anything from the repository to get up and
running :)

## The Template

![Create New Project]({{site.url}}/img/screenshots/VSNewProject.png)

There are two templates to pick from! One is for UWP, and one is for .NET
Core. Choose the UWP one if you're deploying to HoloLens 2, and choose
the .NET Core one if you're doing development for a desktop Windows or
Linux environment. Iteration is slower in UWP due to the way it builds
applications, so only choose UWP if HoloLens is important for you.

The starting code is pretty simple. Initialize StereoKit in Mixed Reality
mode, create a 3D model that's a rounded cube with a default material,
and then draw that model every step of the application!

With that, you automatically get hands, environment, and lighting right
away!

```csharp
using System;
using StereoKit;

class Program
{
	static void Main(string[] args)
	{
		if (!SK.Initialize(new SKSettings{ appName = "Project" }))
			Environment.Exit(1);

		Model cube = Model.FromMesh(
			Mesh.GenerateRoundedCube(Vec3.One, 0.2f),
			Default.Material);

		while (SK.Step(() =>
		{
			cube.Draw(Matrix.TS(Vec3.Zero, 0.1f));
		}));

		SK.Shutdown();
	}
}
```

Awesome! That's pretty easy, but what next? [Why don't we build some UI]({{site.url}}/Pages/Guides/User-Interface.html)?
Alternatively, you can check out the [sample painting application](https://github.com/maluoi/StereoKit-PaintTutorial)
repository, which contains a finger-painting application written in about
220 lines of code! It's well commented, and is a good example to pick
through.

For additional learning resources, you can check out the [Learning Resources]({{site.url}}/Pages/Guides/Learning-Resources.html)
page for a couple of repositories and links that may help you out. In
particular, the Github repository does contain a [number of small demo scenes](https://github.com/maluoi/StereoKit/tree/master/Examples/StereoKitTest/Demos)
that are excellent reference for a number of different StereoKit features!

And don't forget to peek in the docs here! Most pages contain sample code
that illustrates how a particular function or property is used
in-context. The ultimate goal is to have a sample for 100% of the docs,
so if you're looking for one and it isn't there, use the 'Create an Issue'
link at the bottom of the web page to get it prioritized!

