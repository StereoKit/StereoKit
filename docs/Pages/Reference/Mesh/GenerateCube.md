---
layout: default
title: Mesh.GenerateCube
description: Generates a flat-shaded cube mesh, pre-sized to the given dimensions. UV coordinates are projected flat on each face, 0,0 -> 1,1.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).GenerateCube

<div class='signature' markdown='1'>
static [Mesh]({{site.url}}/Pages/Reference/Mesh.html) GenerateCube([Vec3]({{site.url}}/Pages/Reference/Vec3.html) dimensions, int subdivisions)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) dimensions|How large is this cube on each axis, in meters?|
|int subdivisions|Use this to add extra slices of vertices across the cube's              faces. This can be useful for some types of vertex-based effects!|
|RETURNS: [Mesh]({{site.url}}/Pages/Reference/Mesh.html)|A flat-shaded cube mesh, pre-sized to the given dimensions.|

Generates a flat-shaded cube mesh, pre-sized to the given
dimensions. UV coordinates are projected flat on each face, 0,0 -> 1,1.




## Examples

![Procedural Geometry Demo](/img/screenshots/ProceduralGeometry.jpg)
Here's a quick example of generating a mesh! You can store it in just a
Mesh, or you can attach it to a Model for easier rendering later on.
```csharp
Mesh  cubeMesh  = Mesh.GenerateCube(Vec3.One * 0.8f);
Model cubeModel = new Model(cubeMesh, Material.Copy(DefaultIds.material));
```
Drawing both a Mesh and a Model generated this way is reasonably simple,
here's a short example! For the Mesh, you'll need to create your own material,
we just loaded up the default Material here.
```csharp
Matrix cubeTransform = Matrix.TRS(new Vec3(-1.5f, 0, 1), Quat.Identity, Vec3.One);
Renderer.Add(cubeMesh, defaultMaterial, cubeTransform);

cubeTransform = Matrix.TRS(new Vec3(-1.5f, 0, -1), Quat.Identity, Vec3.One);
Renderer.Add(cubeModel, cubeTransform);
```

