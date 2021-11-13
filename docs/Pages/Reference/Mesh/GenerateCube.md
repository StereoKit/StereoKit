---
layout: default
title: Mesh.GenerateCube
description: Generates a flat-shaded cube mesh, pre-sized to the given dimensions. UV coordinates are projected flat on each face, 0,0 -> 1,1.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).GenerateCube

<div class='signature' markdown='1'>
```csharp
static Mesh GenerateCube(Vec3 dimensions, int subdivisions)
```
Generates a flat-shaded cube mesh, pre-sized to the
given dimensions. UV coordinates are projected flat on each face,
0,0 -> 1,1.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) dimensions|How large is this cube on each axis, in              meters?|
|int subdivisions|Use this to add extra slices of             vertices across the cube's              faces. This can be useful for some types of vertex-based effects             !|
|RETURNS: [Mesh]({{site.url}}/Pages/Reference/Mesh.html)|A flat-shaded cube mesh, pre-sized to the given dimensions.|





## Examples

![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
Here's a quick example of generating a mesh! You can store it in just a
Mesh, or you can attach it to a Model for easier rendering later on.
```csharp
Mesh  cubeMesh  = Mesh.GenerateCube(Vec3.One * 0.4f);
Model cubeModel = Model.FromMesh(cubeMesh, Default.Material);
```
Drawing both a Mesh and a Model generated this way is reasonably simple,
here's a short example! For the Mesh, you'll need to create your own material,
we just loaded up the default Material here.
```csharp
Matrix cubeTransform = Matrix.T(-.5f, -.5f, 0);
Renderer.Add(cubeMesh, Default.Material, cubeTransform);

cubeTransform = Matrix.T(.5f, -.5f, 0);
Renderer.Add(cubeModel, cubeTransform);
```
### UV and Face layout
Here's a test image that illustrates how this mesh's geometry is
laid out.
![Procedural Cube Mesh]({{site.screen_url}}/ProcGeoCube.jpg)
```csharp
meshCube = Mesh.GenerateCube(Vec3.One);
```

