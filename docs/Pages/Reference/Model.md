---
layout: default
title: Model
description: A Model is a collection of meshes, materials, and transforms that make up a visual element! This is a great way to group together complex objects that have multiple parts in them, and in fact, most model formats are composed this way already!  This class contains a number of methods for creation. If you pass in a .obj, .stl, .gltf, or .glb, StereoKit will load that model from file, and assemble materials and transforms from the file information. But you can also assemble a model from procedurally generated meshes!  Because models include an offset transform for each mesh element, this does have the overhead of an extra matrix multiplication in order to execute a render command. So if you need speed, and only have a single mesh with a precalculated transform matrix, it can be faster to render a Mesh instead of a Model!
---
# class Model

A Model is a collection of meshes, materials, and transforms
that make up a visual element! This is a great way to group together
complex objects that have multiple parts in them, and in fact, most
model formats are composed this way already!

This class contains a number of methods for creation. If you pass in
a .obj, .stl, .gltf, or .glb, StereoKit will load that model from
file, and assemble materials and transforms from the file
information. But you can also assemble a model from procedurally
generated meshes!

Because models include an offset transform for each mesh element,
this does have the overhead of an extra matrix multiplication in
order to execute a render command. So if you need speed, and only
have a single mesh with a precalculated transform matrix, it can be
faster to render a Mesh instead of a Model!



## Instance Methods

|  |  |
|--|--|
|[Model]({{site.url}}/Pages/Reference/Model/Model.html)|Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.|
|[AddSubset]({{site.url}}/Pages/Reference/Model/AddSubset.html)|Adds a new subset to the Model, and recalculates the bounds. A default subset name of "subsetX" will be used, where X is the subset's index.|
|[Draw]({{site.url}}/Pages/Reference/Model/Draw.html)|Adds this Model to the render queue for this frame! If the Hierarchy has a transform on it, that transform is combined with the Matrix provided here.|
|[GetMaterial]({{site.url}}/Pages/Reference/Model/GetMaterial.html)|Gets a link to the Material asset used by the model subset! Note that this is not necessarily a unique material, and could be shared in a number of other places. Consider copying and replacing it if you intend to modify it!|
|[GetMesh]({{site.url}}/Pages/Reference/Model/GetMesh.html)|Gets a link to the Mesh asset used by the model subset! Note that this is not necessarily a unique mesh, and could be shared in a number of other places. Consider copying and replacing it if you intend to modify it!|
|[GetName]({{site.url}}/Pages/Reference/Model/GetName.html)|Returns the name of the specific subset! This will be the node name of your model asset. If no node name is available, SteroKit will generate a name in the format of "subsetX", where X would be the subset index. Note that names are not guaranteed to be unique (users may assign the same name to multiple nodes). Some nodes may also produce multiple subsets with the same name, such as when a node contains a Mesh with multiple Materials, each Mesh/Material combination will receive a subset with the same name.|
|[GetTransform]({{site.url}}/Pages/Reference/Model/GetTransform.html)|Gets the transform matrix used by the model subset!|
|[RecalculateBounds]({{site.url}}/Pages/Reference/Model/RecalculateBounds.html)|Examines the subsets as they currently are, and rebuilds the bounds based on that! This is normally done automatically, but if you modify a Mesh that this Model is using, the Model can't see it, and you should call this manually.|
|[RemoveSubset]({{site.url}}/Pages/Reference/Model/RemoveSubset.html)|Removes and dereferences a subset from the model.|
|[SetMaterial]({{site.url}}/Pages/Reference/Model/SetMaterial.html)|Changes the Material for the subset to a new one!|
|[SetMesh]({{site.url}}/Pages/Reference/Model/SetMesh.html)|Changes the mesh for the subset to a new one!|
|[SetTransform]({{site.url}}/Pages/Reference/Model/SetTransform.html)|Changes the transform for the subset to a new one! This is in Model space, so it's relative to the origin of the model.|


## Static Fields and Properties

|  |  |
|--|--|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) [Bounds]({{site.url}}/Pages/Reference/Model/Bounds.html)|This is a bounding box that encapsulates the Model and all its subsets! It's used for collision, visibility testing, UI layout, and probably other things. While it's normally cacluated from the mesh bounds, you can also override this to suit your needs.|
|int [SubsetCount]({{site.url}}/Pages/Reference/Model/SubsetCount.html)|The number of mesh subsets attached to this model.|


## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Model/Find.html)|Looks for a Model asset that's already loaded, matching the given id!|
|[FromFile]({{site.url}}/Pages/Reference/Model/FromFile.html)|Loads a list of mesh and material subsets from a .obj, .stl, .gltf, or .glb file.|
|[FromMemory]({{site.url}}/Pages/Reference/Model/FromMemory.html)|Loads a list of mesh and material subsets from a .obj, .stl, .gltf, or .glb file stored in memory. Note that this function won't work well on files that reference other files, such as .gltf files with references in them.|
|[FromMesh]({{site.url}}/Pages/Reference/Model/FromMesh.html)|Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.|

