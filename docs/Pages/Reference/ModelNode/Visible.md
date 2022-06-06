---
layout: default
title: ModelNode.Visible
description: Is this node flagged as visible? By default, this is true for all nodes with visual elements attached. These nodes will not be drawn or skinned if you set this flag to false. If a ModelNode has no visual elements attached to it, it will always return false, and setting this value will have no effect.
---
# [ModelNode]({{site.url}}/Pages/Reference/ModelNode.html).Visible

<div class='signature' markdown='1'>
bool Visible{ get set }
</div>

## Description
Is this node flagged as visible? By default, this is true
for all nodes with visual elements attached. These nodes will not
be drawn or skinned if you set this flag to false. If a ModelNode
has no visual elements attached to it, it will always return false,
and setting this value will have no effect.


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

