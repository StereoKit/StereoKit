# Starting with StereoKit

This document assumes that you want to utilize StereoKit as-is for your project. It will run you through the fundamentals along the provided [examples](https://github.com/maluoi/StereoKit/tree/master/Examples).

## Opening a screen

The application can run in two different modes:

* `sk_runtime_flatscreen` (CPP) or `Runtime.Flatscreen` (C#): The application will start in desktop mode, opening an [overlapped window](https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles) in 640x480 as default resolution.
* `sk_runtime_mixedreality` (CPP) or `Runtime.Flatscreen` (C#): The application will start in Mixed Reality mode, opening an [OpenXR instance](https://www.khronos.org/registry/OpenXR/specs/0.90/man/html/xrCreateInstance.html) inside the headset.

The mode is defined during the initialization where the first parameter is the application name and the second one the render mode:

``` cpp
// CPP
// Opening an OpenXR instance
if (!sk_init("Stereo Kit", sk_runtime_mixedreality))
	return 1;
```

``` csharp
// C#
// Opening an overlapped window
if (!StereoKitApp.Initialize("StereoKit C#", Runtime.Flatscreen, true))
	Environment.Exit(1);
``` 

In case the mode is set to sk_runtime_mixedreality and for some reason no xr_session could be created, there is the option to define sk_runtime_flatscreen as an automatic fallback. In that case the application will open in windowed mode instead. It will still fail if no fallback could be created.

```cpp
// CPP
if (!sk_init("Stereo Kit", sk_runtime_mixedreality, true))
	return 1;
```

``` csharp
// C#
if (!StereoKitApp.Initialize("StereoKit C#", Runtime.MixedReality, true))
	Environment.Exit(1);
``` 


## Loading 3D assets

StereoKit has support for 3D Wavefront .obj files (for example out of [Blender](https://docs.blender.org/manual/en/dev/addons/io_scene_obj.html?highlight=obj)) and [glTF](https://www.khronos.org/gltf/) files.

All models are stored in objects of type `model_t` (CPP) and `Model` (C#).

```cpp
// CPP
model_t gltf_example;
gltf_example = model_create_file("YourAssetDirectory/example_model.obj");

model_t obj_example;
obj_example = model_create_file("YourAssetDirectory/example_model.obj");
```

``` csharp
// C#
Model gltf_example   = new Model("YourAssetDirectory/example_model.gltf");

Model obj_example   = new Model("YourAssetDirectory/example_model.obj");
```


## Loading 2D textures & materials for procedurally generated meshes

Besides 3D files, StereoKit supports procedurally generated meshes, including textures and materials. The example below shows how to load a texture and define materials for a procedurally generated mesh.

``` cpp
// CPP
// Load the texture and create a material from it using the default shader.
tex2d_t example_texture = tex2d_create_file("YourAssetDirectory/example_texture.png");
tex2d_t example_normal_texture = tex2d_create_file("YourAssetDirectory/example_normal_texture.png");
example_material = material_create("app/example_material", shader_find("default/shader_pbr"));
material_set_texture(example_material, "diffuse", example_texture);
material_set_texture(example_material, "normal", example_normal_texture);

// Set additional material properties.
material_set_float  (example_material, "tex_scale", 6);
material_set_float  (example_material, "roughness", 1.0f);
material_set_float  (example_material, "metallic", 0.5f);
material_set_queue_offset(example_material, 1);

// Create a simple cube mesh and apply the created material to it.
mesh_t mesh_cube = mesh_gen_cube("app/mesh_cube", vec3_one, 0);
box = model_create_mesh("app/model_cube", mesh_cube, floor_mat);

// Release everything again.
if (example_texture != nullptr) tex2d_release(example_texture);
if (example_normal_texture != nullptr) tex2d_release(example_texture);
mesh_release(mesh_cube);
```

``` csharp
// C#
// Load the texture and create a material from it using the default shader.
Material exampleMaterial = new Material(Shader.Find("default/shader_pbr"));
exampleMaterial["diffuse"] = new Tex2D(root + "YourAssetDirectory/example_texture.png");
exampleMaterial["normal"] = new Tex2D(root + "YourAssetDirectory/example_normel_texture.png");
floorMat["tex_scale"] = 6;

// Create a simple cube mesh and apply the created material to it.
cube = new Model(Mesh.GenerateCube("app/mesh_cube", Vec3.One), exampleMaterial);
```
    

## Rendering things on screen

Everything in StereoKit is rendered within a step. The step function keeps track of timing, rendering, input, screen buffering and so on. Note that the implementation is different for OpenXR and Win32 mode.

``` cpp
// CPP
while (sk_step( []() {
	// do render things here
}));
```

``` csharp
// C#
while (StereoKitApp.Step(() => {
	// do render things here
}));
```
