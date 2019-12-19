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
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) [Bounds]({{site.url}}/Pages/Reference/Model/Bounds.html)|This is a bounding box that encapsulates the Model and all its subsets! It's used for collision, visibility testing, UI layout, and probably other things. While it's normally cacluated from the mesh bounds, you can also override this to suit your needs.|
|int [SubsetCount]({{site.url}}/Pages/Reference/Model/SubsetCount.html)|The number of mesh subsets attached to this model.|



## Methods

|  |  |
|--|--|
|[Model]({{site.url}}/Pages/Reference/Model/Model.html)|Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.|
|[Draw]({{site.url}}/Pages/Reference/Model/Draw.html)|Adds this Model to the render queue for this frame! If the Hierarchy has a transform on it, that transform is combined with the Matrix provided here.|
|[Find]({{site.url}}/Pages/Reference/Model/Find.html)|Looks for a Model asset that's already loaded, matching the given id!|
|[FromFile]({{site.url}}/Pages/Reference/Model/FromFile.html)|Loads a list of mesh and material subsets from a .obj, .gltf, or .glb file.|
|[FromMesh]({{site.url}}/Pages/Reference/Model/FromMesh.html)|Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.|
|[GetMaterial]({{site.url}}/Pages/Reference/Model/GetMaterial.html)|Gets a link to the Material asset used by the mesh subset!|


