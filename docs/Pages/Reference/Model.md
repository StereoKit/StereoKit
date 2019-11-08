---
layout: default
title: Model
description: A Model is a collection of meshes, materials, and transforms that make up a visual element! This is a great way to group together complex objects that have multiple parts in them, and in fact, most model formats are composed this way already!  This class contains a number of methods for creation. If you pass in a .obj, .gltf, or .glb, StereoKit will load that model from file, and assemble materials and transforms from the file information. But you can also assemble a model from procedurally generated meshes!  Because models include an offset transform for each mesh element, this does have the overhead of an extra matrix multiplication in order to execute a render command. So if you need speed, and only have a single mesh with a precalculated transform matrix, it can be faster to render a Mesh instead of a Model!
---
# Model

## Description
A Model is a collection of meshes, materials, and transforms that
make up a visual element! This is a great way to group together complex
objects that have multiple parts in them, and in fact, most model formats
are composed this way already!

This class contains a number of methods for creation. If you pass in a .obj,
.gltf, or .glb, StereoKit will load that model from file, and assemble materials
and transforms from the file information. But you can also assemble a model
from procedurally generated meshes!

Because models include an offset transform for each mesh element, this does have the
overhead of an extra matrix multiplication in order to execute a render command.
So if you need speed, and only have a single mesh with a precalculated transform
matrix, it can be faster to render a Mesh instead of a Model!



## Methods

|  |  |
|--|--|
|[#ctor]({{site.url}}/Pages/Reference/Model/#ctor.html)|Loads a list of mesh and material subsets from a .obj, .gltf, or .glb file.|
|[#ctor]({{site.url}}/Pages/Reference/Model/#ctor.html)|Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.|
|[#ctor]({{site.url}}/Pages/Reference/Model/#ctor.html)|Creates a single mesh subset Model using the indicated Mesh and Material!|
|[GetMaterial]({{site.url}}/Pages/Reference/Model/GetMaterial.html)|Gets a link to the Material asset used by the mesh subset!|


