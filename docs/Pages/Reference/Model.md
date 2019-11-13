---
layout: default
title: Model
description: A Model is a collection of meshes, materials, and transforms that make up a visual element! This is a great way to group together complex objects that have multiple parts in them, and in fact, most model formats are composed this way already!  This class contains a number of methods for creation. If you pass in a .obj, .gltf, or .glb, StereoKit will load that model from file, and assemble materials and transforms from the file information. But you can also assemble a model from procedurally generated meshes!  Because models include an offset transform for each mesh element, this does have the overhead of an extra matrix multiplication in order to execute a render command. So if you need speed, and only have a single mesh with a precalculated transform matrix, it can be faster to render a Mesh instead of a Model!
---
# Model

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


## Fields and Properties

|  |  |
|--|--|
|int [SubsetCount]({{site.url}}/Pages/Reference/Model/SubsetCount.html)|The number of mesh subsets attached to this model.|



## Methods

|  |  |
|--|--|
|[Model]({{site.url}}/Pages/Reference/Model/Model.html)|Loads a list of mesh and material subsets from a .obj, .gltf, or .glb file.|
|[Find]({{site.url}}/Pages/Reference/Model/Find.html)|Looks for a Model asset that's already loaded, matching the given id!|
|[GetMaterial]({{site.url}}/Pages/Reference/Model/GetMaterial.html)|Gets a link to the Material asset used by the mesh subset!|


