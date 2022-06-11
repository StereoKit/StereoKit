---
layout: default
title: Mesh.GetVerts
description: This marshalls the Mesh's vertex data into an array. If KeepData is false, then the Mesh is _not_ storing verts on the CPU, and this information will _not_ be available.  Due to the way marshalling works, this is _not_ a cheap function!
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).GetVerts

<div class='signature' markdown='1'>
```csharp
Vertex[] GetVerts()
```
This marshalls the Mesh's vertex data into an array. If
KeepData is false, then the Mesh is _not_ storing verts on the CPU,
and this information will _not_ be available.

Due to the way marshalling works, this is _not_ a cheap function!
</div>

|  |  |
|--|--|
|RETURNS: Vertex[]|An array of vertices representing the Mesh, or null if KeepData is false.|




