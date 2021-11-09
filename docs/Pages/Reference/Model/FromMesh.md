---
layout: default
title: Model.FromMesh
description: Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).FromMesh

<div class='signature' markdown='1'>
```csharp
static Model FromMesh(Mesh mesh, Material material)
```
Creates a single mesh subset Model using the indicated
Mesh and Material! An id will be automatically generated for this
asset.
</div>

|  |  |
|--|--|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|Any Mesh asset.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Any Material asset.|
|RETURNS: [Model]({{site.url}}/Pages/Reference/Model.html)|A Model composed of a single mesh and Material.|

<div class='signature' markdown='1'>
```csharp
static Model FromMesh(string id, Mesh mesh, Material material)
```
Creates a single mesh subset Model using the indicated
Mesh and Material!
</div>

|  |  |
|--|--|
|string id|Uses this as the id, so you can Find it later.|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|Any Mesh asset.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Any Material asset.|
|RETURNS: [Model]({{site.url}}/Pages/Reference/Model.html)|A Model composed of a single mesh and Material.|




