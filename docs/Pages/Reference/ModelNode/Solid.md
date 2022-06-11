---
layout: default
title: ModelNode.Solid
description: A flag that indicates the Mesh for this node will be used in ray intersection tests. This flag is ignored if no Mesh is attached.
---
# [ModelNode]({{site.url}}/Pages/Reference/ModelNode.html).Solid

<div class='signature' markdown='1'>
bool Solid{ get set }
</div>

## Description
A flag that indicates the Mesh for this node will be used
in ray intersection tests. This flag is ignored if no Mesh is
attached.


## Examples

### Collision Tagged Nodes
One particularly practical example of tagging your ModelNode names
would be to set up collision information for your Model. If, for
example, you have a low resolution mesh designed specifically for
fast collision detection, you can tag your non-solid nodes as
"[Intangible]", and your collider nodes as "[Invisible]":
```csharp
foreach (ModelNode node in model.Nodes)
{
	node.Solid   = node.Name.Contains("[Intangible]") == false;
	node.Visible = node.Name.Contains("[Invisible]")  == false;
}
```

