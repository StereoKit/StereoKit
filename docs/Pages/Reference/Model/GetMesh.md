---
layout: default
title: Model.GetMesh
description: Gets a link to the Mesh asset used by the model subset! Note that this is not necessarily a unique mesh, and could be shared in a number of other places. Consider copying and replacing it if you intend to modify it!
---
# [Model]({{site.url}}/Pages/Reference/Model.html).GetMesh

<div class='signature' markdown='1'>
```csharp
Mesh GetMesh(int subsetIndex)
```
Gets a link to the Mesh asset used by the model subset!
Note that this is not necessarily a unique mesh, and could be
shared in a number of other places. Consider copying and
replacing it if you intend to modify it!
</div>

|  |  |
|--|--|
|int subsetIndex|Index of the model subset to get the             Mesh for, should be less than SubsetCount.|
|RETURNS: [Mesh]({{site.url}}/Pages/Reference/Mesh.html)|A link to the Mesh asset used by the model subset at subsetIndex|




