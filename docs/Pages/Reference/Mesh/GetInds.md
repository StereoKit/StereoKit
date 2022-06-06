---
layout: default
title: Mesh.GetInds
description: This marshalls the Mesh's index data into an array. If KeepData is false, then the Mesh is _not_ storing indices on the CPU, and this information will _not_ be available.  Due to the way marshalling works, this is _not_ a cheap function!
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).GetInds

<div class='signature' markdown='1'>
```csharp
UInt32[] GetInds()
```
This marshalls the Mesh's index data into an array. If
KeepData is false, then the Mesh is _not_ storing indices on the
CPU, and this information will _not_ be available.

Due to the way marshalling works, this is _not_ a cheap function!
</div>

|  |  |
|--|--|
|RETURNS: UInt32[]|An array of indices representing the Mesh, or null if KeepData is false.|




