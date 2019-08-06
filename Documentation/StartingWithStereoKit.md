# Starting with StereoKit

This document assumes that you want to utilize StereoKit as-is for your project. It will run you through the fundamentals along the provided [examples](https://github.com/maluoi/StereoKit/tree/master/Examples).


## Opening a screen

The application can run in two different modes:

- `sk_runtime_flatscreen` (CPP) or `Runtime.Flatscreen` (C#): The application will start in desktop mode, opening a win32 [overlapped window](https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles) in 640x480 as default resolution.
- `sk_runtime_mixedreality` (CPP) or `Runtime.Flatscreen` (C#): The application will start in Mixed Reality mode, opening an [OpenXR instance](https://www.khronos.org/registry/OpenXR/specs/0.90/man/html/xrCreateInstance.html) inside the headset.

The mode is defined during the initialization where the first parameter is the application name and the second one the render mode:

```cpp
// CPP
if (!sk_init("Stereo Kit", sk_runtime_mixedreality))
	return 1;
```

```csharp
// C#
if (!StereoKitApp.Initialize("StereoKit C#", Runtime.Flatscreen, true))
	Environment.Exit(1);
``` 

In case the mode is set to sk_runtime_mixedreality and for some reason no xr_session could be created, there is the option to define sk_runtime_flatscreen as an automatic fallback. In that case the application will open in windowed mode instead.

```cpp
// CPP
if (!sk_init("Stereo Kit", sk_runtime_mixedreality, true))
	return 1;
```

```csharp
// C#
if (!StereoKitApp.Initialize("StereoKit C#", Runtime.MixedReality, true))
	Environment.Exit(1);
``` 


## Loading assets

StereoKit has support for 3D Wavefront .obj files (for example out of [Blender](https://docs.blender.org/manual/en/dev/addons/io_scene_obj.html?highlight=obj)) and [glTF](https://www.khronos.org/gltf/) files.

All models are stored in objects of type `model_t` (CPP) and `Model` (C#).

```cpp
// CPP
model_t gltf_example;
gltf_example = model_create_file("Assets/example_model.obj");

model_t obj_example;
obj_example = model_create_file("Assets/example_model.obj");
```

```csharp
// C#
Model gltf_example   = new Model("Assets/example_model.gltf");

Model obj_example   = new Model("Assets/example_model.obj");
```


## Rendering things on screen

Everything in StereoKit is rendered within a step. The step function keeps track of timing, rendering, input, screen buffering and so on. Note that the implementation is different for OpenXR and Win32 mode.

```cpp
// CPP
while (sk_step( []() {
	// do render things here
}));
```

```csharp
// C#
while (StereoKitApp.Step(() => {
	// do render things here
}));
```


## Known issues / FAQ

| Issue | Solution |
| :--- | :--- |
| Error during compilation: _The command "xcopy "C:\Projects\StereoKit\\x64\Debug\*.dll" "C:\Projects\StereoKit\StereoKitTest\bin\Debug\" /d /k /y" exited with code 4. | Select x64 as target. |
| Runtime error: _Can't load Assets/..! File not found, or invalid format. | The asset folder is missing from your folder - make sure to copy it when you compiled the example. |
| Runtime warning: _Error [SPEC \| xrBeginFrame \| VUID-xrBeginFrame-session-parameter] : session is not a valid XrSession_  | There seems to be no XR device active on your computer. Make sure you connected a headset / run the application on a headset or change the render target to `StereoKitApp kit = new StereoKitApp("CSharp OpenXR", Runtime.Flatscreen);` |

