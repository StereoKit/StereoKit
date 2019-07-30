# StereoKit

StereoKit is a lightweight low-dependency C# rendering library for MR / AR / VR / Desktop using OpenXR!

As [SharpDX](http://sharpdx.org/) and many other render libraries for C# are sadly deprecated and no longer in development, there are plenty of engines and interfaces out there for rendering in C / CPP, but much less for creating 3D projects with C#. StereoKit tries to fill that gap.

StereoKit offers a modern C# interface for high performance 3D rendering, specifically for Mixed Reality (AR, VR) applications. It also provides (currently basic) functionality around asset management and input handling.

StereoKit is based on OpenXR to enable targeting a wide array of hardware platforms.


## Tested platforms

The following platforms have been used during development and are regularly tested to assure compatibility:

- Windows Mixed Reality Headsets (VR)
- HoloLens 2
- Windows 10 Desktop


## Components

- **StereoKitC**: The StereoKit core, written in C / CPP.
- **StereoKitCTest**: A test application for StereoKitC.
- **StereoKit**: CSharp bindings for StereoKit.
- **StereoKitTest**: The test application as CSharp variant.


## Known issues / FAQ

| Issue | Solution |
| :--- | :--- |
| Error during compilation: _The command "xcopy "C:\Projects\StereoKit\\x64\Debug\*.dll" "C:\Projects\StereoKit\StereoKitTest\bin\Debug\" /d /k /y" exited with code 4. | Select x64 as target. |
| Runtime error: _Can't load Assets/..! File not found, or invalid format. | The asset folder is missing from your folder - make sure to copy it when you compiled the example. |
| Runtime warning: _Error [SPEC \| xrBeginFrame \| VUID-xrBeginFrame-session-parameter] : session is not a valid XrSession_  | There seems to be no XR device active on your computer. Make sure you connected a headset / run the application on a headset or change the render target to `StereoKitApp kit = new StereoKitApp("CSharp OpenXR", Runtime.Flatscreen);` |

