---
layout: default
title: Model.GetName
description: Returns the name of the specific subset! This will be the node name of your model asset. If no node name is available, SteroKit will generate a name in the format of "subsetX", where X would be the subset index. Note that names are not guaranteed to be unique (users may assign the same name to multiple nodes). Some nodes may also produce multiple subsets with the same name, such as when a node contains a Mesh with multiple Materials, each Mesh/Material combination will receive a subset with the same name.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).GetName

<div class='signature' markdown='1'>
```csharp
string GetName(int subsetIndex)
```
Returns the name of the specific subset! This will be
the node name of your model asset. If no node name is available,
SteroKit will generate a name in the format of "subsetX", where
X would be the subset index. Note that names are not guaranteed
to be unique (users may assign the same name to multiple nodes).
Some nodes may also produce multiple subsets with the same name,
such as when a node contains a Mesh with multiple Materials, each
Mesh/Material combination will receive a subset with the same
name.
</div>

|  |  |
|--|--|
|int subsetIndex|Index of the model subset to get the              Material for, should be less than SubsetCount.|
|RETURNS: string|See summary for details.|




