![StereoKit Logo](/Documentation/img/StereoKitWide.svg)

A lightweight low-dependency C# rendering library for MR / AR / VR / Desktop using OpenXR!

The future of development is coming, and we need better tools! StereoKit aims to be forward thinking in its design, take advantage of more modern hardware features common to Mixed Reality devices, and be much easier to use for general computing tasks! (ie. not necessarily games) Here's some features, most of which are already present:
- Rendering
  - Physically based rendering, metal workflow.
  - Hardware instancing for speed, by default!
  - An extendable shader framework.
- Runtime everything, including asset pipeline!
  - GLTF file support
  - OBJ file support
  - Procedural mesh API!
- Physics
  - Rigid bodies and joints
  - Physically based UI!
  - Hand-centric user input
- OpenXR support
  - StereoKit will eventually run on every Mixed Reality device!

![Screenshot](/Documentation/img/SKScreenshot1.jpg)


## Motivation

As [SharpDX](http://sharpdx.org/) and many other render libraries for C# are sadly deprecated and no longer in development, there are plenty of engines and interfaces out there for rendering in C / CPP, but much less for creating 3D projects with C#. StereoKit tries to fill that gap.


## Getting started

While StereoKit is still in development, you can totally try it out already! The lower level APIs are easy to use, and already fairly well established.

Have a look at the [Getting started documentation](Documentation/StartingWithStereoKit.md) and the [examples](https://github.com/maluoi/StereoKit/tree/master/Examples) to get started!


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
