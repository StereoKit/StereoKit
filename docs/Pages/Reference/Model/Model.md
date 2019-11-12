---
layout: default
title: Model.Model
description: Loads a list of mesh and material subsets from a .obj, .gltf, or .glb file.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Model

<div class='signature' markdown='1'>
 Model(string file)
</div>

|  |  |
|--|--|
|string file|Name of the file to load! This gets prefixed with the StereoKit asset             folder if no drive letter is specified in the path.|

Loads a list of mesh and material subsets from a .obj, .gltf, or .glb file.
<div class='signature' markdown='1'>
 Model([Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh, [Material]({{site.url}}/Pages/Reference/Material.html) material)
</div>

|  |  |
|--|--|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|Any Mesh asset.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Any Material asset.|

Creates a single mesh subset Model using the indicated Mesh and Material! An
id will be automatically generated for this asset.
<div class='signature' markdown='1'>
 Model(string id, [Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh, [Material]({{site.url}}/Pages/Reference/Material.html) material)
</div>

|  |  |
|--|--|
|string id|Uses this as the id, so you can Find it later.|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|Any Mesh asset.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Any Material asset.|

Creates a single mesh subset Model using the indicated Mesh and Material!



