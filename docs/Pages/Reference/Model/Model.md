---
layout: default
title: Model.Model
description: Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Model

<div class='signature' markdown='1'>
```csharp
void Model(Mesh mesh, Material material)
```
Creates a single mesh subset Model using the indicated
Mesh and Material! An id will be automatically generated for this
asset.
</div>

|  |  |
|--|--|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|Any Mesh asset.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Any Material asset.|

<div class='signature' markdown='1'>
```csharp
void Model()
```
Creates an empty Model object with an automatically
generated id. Use the AddSubset methods to fill this model out.
</div>

<div class='signature' markdown='1'>
```csharp
void Model(string id, Mesh mesh, Material material)
```
Creates a single mesh subset Model using the indicated
Mesh and Material!
</div>

|  |  |
|--|--|
|string id|Uses this as the id, so you can Find it later.|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|Any Mesh asset.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Any Material asset.|





## Examples

```csharp
Model model = new Model();
model.AddNode("Cube",
	Matrix .Identity,
	Mesh   .GenerateCube(Vec3.One),
	Default.Material);
```

