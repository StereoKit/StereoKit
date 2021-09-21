---
layout: default
title: Model.Model
description: Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Model

<div class='signature' markdown='1'>
 Model([Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh, [Material]({{site.url}}/Pages/Reference/Material.html) material)
</div>

|  |  |
|--|--|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|Any Mesh asset.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Any Material asset.|

Creates a single mesh subset Model using the indicated
Mesh and Material! An id will be automatically generated for this
asset.
<div class='signature' markdown='1'>
 Model()
</div>

Creates an empty Model object with an automatically
generated id. Use the AddSubset methods to fill this model out.
<div class='signature' markdown='1'>
 Model(string id, [Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh, [Material]({{site.url}}/Pages/Reference/Material.html) material)
</div>

|  |  |
|--|--|
|string id|Uses this as the id, so you can Find it later.|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|Any Mesh asset.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Any Material asset.|

Creates a single mesh subset Model using the indicated
Mesh and Material!




## Examples

```csharp
Model model = new Model();
model.AddNode("Cube",
	Matrix .Identity,
	Mesh   .GenerateCube(Vec3.One),
	Default.Material);
```

