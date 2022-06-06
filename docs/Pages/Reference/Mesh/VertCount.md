---
layout: default
title: Mesh.VertCount
description: The number of vertices stored in this Mesh! This is available to you regardless of whether or not KeepData is set.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).VertCount

<div class='signature' markdown='1'>
int VertCount{ get }
</div>

## Description
The number of vertices stored in this Mesh! This is
available to you regardless of whether or not KeepData is set.


## Examples

### Counting the Vertices and Triangles in a Model

Model.Visuals are always guaranteed to have a Mesh, so no need to
null check there, and VertCount and IndCount are available even if
Mesh.KeepData is false!
```csharp
int vertCount = 0;
int triCount  = 0;

foreach (ModelNode node in model.Visuals)
{
	Mesh mesh = node.Mesh;
	vertCount += mesh.VertCount;
	triCount  += mesh.IndCount / 3;
}
Log.Info($"Model stats: {vertCount} vertices, {triCount} triangles");
```

