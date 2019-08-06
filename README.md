# StereoKit

StereoKit is a lightweight low-dependency C# rendering library for MR / AR / VR / Desktop using OpenXR!

As [SharpDX](http://sharpdx.org/) and many other render libraries for C# are sadly deprecated and no longer in development, there are plenty of engines and interfaces out there for rendering in C / CPP, but much less for creating 3D projects with C#. StereoKit tries to fill that gap.

StereoKit offers a modern C# interface for high performance 3D rendering, specifically for Mixed Reality (AR, VR) applications. It also provides (currently basic) functionality around asset management and input handling.

StereoKit is based on OpenXR to enable targeting a wide array of hardware platforms.

Have a look at the [Getting started documentation](Documentation/StartingWithStereoKit.md) and the [examples](https://github.com/maluoi/StereoKit/tree/master/Examples).


## Tested platforms

The following platforms have been used during development and are regularly tested to assure compatibility:

- Windows Mixed Reality Headsets (VR)
- Microsoft HoloLens 2
- Windows 10 Desktop


## Components

- **StereoKitC**: The StereoKit core, written in C / CPP.
- **StereoKit**: CSharp bindings for StereoKit.


## Examples

- **StereoKitCTest**: A test application for StereoKitC.
- **StereoKitTest**: The test application as CSharp variant.
