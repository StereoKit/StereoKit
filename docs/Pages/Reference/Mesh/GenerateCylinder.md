---
layout: default
title: Mesh.GenerateCylinder
description: Generates a cylinder mesh, pre-sized to the given diameter and depth, UV coordinates are from a flattened top view right now. Additional development is needed for making better UVs for the edges.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).GenerateCylinder

<div class='signature' markdown='1'>
static [Mesh]({{site.url}}/Pages/Reference/Mesh.html) GenerateCylinder(float diameter, float depth, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) direction, int subdivisions)
</div>

|  |  |
|--|--|
|float diameter|Diameter of the circular part of the cylinder in meters. Diameter is              2*radius.|
|float depth|How tall is this cylinder, in meters?|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) direction|What direction do the circular surfaces face? This is the surface normal             for the top, it does not need to be normalized.|
|int subdivisions|How many vertices compose the edges of the cylinder? More is smoother,             but less performant.|
|RETURNS: [Mesh]({{site.url}}/Pages/Reference/Mesh.html)|Returns a cylinder mesh, pre-sized to the given diameter and depth, UV coordinates are from a flattened top view right now.|

Generates a cylinder mesh, pre-sized to the given diameter and depth,
UV coordinates are from a flattened top view right now. Additional development is needed for
making better UVs for the edges.




## Examples

![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
Here's a quick example of generating a mesh! You can store it in just a
Mesh, or you can attach it to a Model for easier rendering later on.
```csharp
Mesh  cylinderMesh  = Mesh.GenerateCylinder(0.8f, 0.8f, Vec3.Up);
Model cylinderModel = new Model(cylinderMesh, Material.Copy(DefaultIds.material));
```
Drawing both a Mesh and a Model generated this way is reasonably simple,
here's a short example! For the Mesh, you'll need to create your own material,
we just loaded up the default Material here.
```csharp
Matrix cylinderTransform = Matrix.TRS(new Vec3(1.5f, 0, 1), Quat.Identity, Vec3.One);
Renderer.Add(cylinderMesh, defaultMaterial, cylinderTransform);

cylinderTransform = Matrix.TRS(new Vec3(1.5f, 0, -1), Quat.Identity, Vec3.One);
Renderer.Add(cylinderModel, cylinderTransform);
```

