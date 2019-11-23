---
layout: default
title: Mesh.GenerateSphere
description: Generates a sphere mesh, pre-sized to the given diameter, created by sphereifying a subdivided cube! UV coordinates are taken from the initial unspherified cube.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).GenerateSphere

<div class='signature' markdown='1'>
static [Mesh]({{site.url}}/Pages/Reference/Mesh.html) GenerateSphere(float diameter, int subdivisions)
</div>

|  |  |
|--|--|
|float diameter|The diameter of the sphere in meters, or 2*radius. This is the              full length from one side to the other.|
|int subdivisions|How many times should the initial cube be subdivided?|
|RETURNS: [Mesh]({{site.url}}/Pages/Reference/Mesh.html)|A sphere mesh, pre-sized to the given diameter, created by sphereifying a subdivided cube! UV coordinates are taken from the initial unspherified cube.|

Generates a sphere mesh, pre-sized to the given diameter, created
by sphereifying a subdivided cube! UV coordinates are taken from the initial unspherified
cube.




## Examples

![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
Here's a quick example of generating a mesh! You can store it in just a
Mesh, or you can attach it to a Model for easier rendering later on.
```csharp
Mesh  sphereMesh  = Mesh.GenerateSphere(0.4f);
Model sphereModel = new Model(sphereMesh, Material.Copy(DefaultIds.material));
```
Drawing both a Mesh and a Model generated this way is reasonably simple,
here's a short example! For the Mesh, you'll need to create your own material,
we just loaded up the default Material here.
```csharp
Matrix sphereTransform = Matrix.T(0.0f, 0, 1);
Renderer.Add(sphereMesh, defaultMaterial, sphereTransform);

sphereTransform = Matrix.T(0.0f, 0, -1);
Renderer.Add(sphereModel, sphereTransform);
```

