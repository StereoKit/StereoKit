---
layout: default
title: Model.FromMemory
description: Loads a list of mesh and material subsets from a .obj, .stl, .ply (ASCII), .gltf, or .glb file stored in memory. Note that this function won't work well on files that reference other files, such as .gltf files with references in them.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).FromMemory

<div class='signature' markdown='1'>
```csharp
static Model FromMemory(string filename, Byte[]& data, Shader shader)
```
Loads a list of mesh and material subsets from a .obj,
.stl, .ply (ASCII), .gltf, or .glb file stored in memory. Note
that this function won't work well on files that reference other
files, such as .gltf files with references in them.
</div>

|  |  |
|--|--|
|string filename|StereoKit still uses the filename of the             data for format discovery, but not asset Id creation. If you              don't have a real filename for the data, just pass in an             extension with a leading '.' character here, like ".glb".|
|Byte[]& data|The binary data of a model file, this is NOT              a raw array of vertex and index data!|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) shader|The shader to use for the model's materials!             If null, this will automatically determine the best shader              available to use.|
|RETURNS: [Model]({{site.url}}/Pages/Reference/Model.html)|A Model created from the file, or null if the file failed to load!|




