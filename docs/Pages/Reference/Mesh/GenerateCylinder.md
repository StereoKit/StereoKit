---
layout: default
title: Mesh.GenerateCylinder
description: Generates a cylinder mesh, pre-sized to the given diameter and depth, UV coordinates are from a flattened top view right now. Additional development is needed for making better UVs for the edges.  NOTE. This generates a completely new Mesh asset on the GPU, and is best done during 'initialization' of your app/scene.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).GenerateCylinder

<div class='signature' markdown='1'>
```csharp
static Mesh GenerateCylinder(float diameter, float depth, Vec3 direction, int subdivisions)
```
Generates a cylinder mesh, pre-sized to the given
diameter and depth, UV coordinates are from a flattened top view
right now. Additional development is needed for making better UVs
for the edges.

NOTE: This generates a completely new Mesh asset on the GPU, and
is best done during 'initialization' of your app/scene.
</div>

|  |  |
|--|--|
|float diameter|Diameter of the circular part of the             cylinder in meters. Diameter is 2*radius.|
|float depth|How tall is this cylinder, in meters?|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) direction|What direction do the circular surfaces              face? This is the surface normal for the top, it does not need to             be normalized.|
|int subdivisions|How many vertices compose the edges of             the cylinder? More is smoother, but less performant.|
|RETURNS: [Mesh]({{site.url}}/Pages/Reference/Mesh.html)|Returns a cylinder mesh, pre-sized to the given diameter and depth, UV coordinates are from a flattened top view right now.|





## Examples

### Generating a Mesh and Model

![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)

Here's a quick example of generating a mesh! You can store it in just a
Mesh, or you can attach it to a Model for easier rendering later on.
```csharp
// Do this in your initialization
Mesh  cylinderMesh  = Mesh.GenerateCylinder(0.4f, 0.4f, Vec3.Up);
Model cylinderModel = Model.FromMesh(cylinderMesh, Default.Material);
```
Drawing both a Mesh and a Model generated this way is reasonably simple,
here's a short example! For the Mesh, you'll need to create your own material,
we just loaded up the default Material here.
```csharp
// Call this code every Step

Matrix cylinderTransform = Matrix.T(-.5f, 1, 0);
cylinderMesh.Draw(Default.Material, cylinderTransform);

cylinderTransform = Matrix.T(.5f, 1, 0);
cylinderModel.Draw(cylinderTransform);
```
### UV and Face layout
Here's a test image that illustrates how this mesh's geometry is
laid out.
![Procedural Cube Mesh]({{site.screen_url}}/ProcGeoCylinder.jpg)
```csharp
meshCylinder = Mesh.GenerateCylinder(1, 1, Vec3.Up);
```

