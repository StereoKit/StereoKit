# Starting with StereoKit

This document assumes that you want to utilize StereoKit as-is for your project. It will run you through the fundamentals and 


## Opening a screen

The application can run in two different modes:

- `sk_runtime_flatscreen` (CPP) or `Runtime.Flatscreen` (C#): The application will start in desktop mode, opening a win32 [overlapped window](https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles) in 640x480 as default resolution.
- `sk_runtime_mixedreality` (CPP) or `Runtime.Flatscreen` (C#): The application will start in Mixed Reality mode, opening an [OpenXR instance](https://www.khronos.org/registry/OpenXR/specs/0.90/man/html/xrCreateInstance.html) inside the headset.

If you are using CPP, this is done during the initialization where the first parameter is the application name and the second one the render mode:

```cpp
// CPP
if (!sk_init("Stereo Kit", sk_runtime_mixedreality))
	return 1;
```

For C#, this is done as the StereoKitApp object is created.

```csharp
// C#
StereoKitApp kit = new StereoKitApp("CSharp OpenXR", Runtime.MixedReality);
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

Everything in StereoKit is rendered within a step. The step function keeps track of timing, rendering, input, screen buffering and so on and also handles the routing to OpenXR or Win32.

```cpp
// CPP
int main() {
	..

	while (sk_step(app_update));

	..
	return 0;
}

void app_update() {
	// do render things here
}
```


```csharp
// C#
while (kit.Step(() => {
	// do render things here
}));
```