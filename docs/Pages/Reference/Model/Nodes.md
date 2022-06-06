---
layout: default
title: Model.Nodes
description: This is an enumerable collection of all the nodes in this Model, ordered non-hierarchically by when they were added. You can do Linq stuff with it, foreach it, or just treat it like a List or array!
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Nodes

<div class='signature' markdown='1'>
[ModelNodeCollection]({{site.url}}/Pages/Reference/ModelNodeCollection.html) Nodes{ get }
</div>

## Description
This is an enumerable collection of all the nodes in this
Model, ordered non-hierarchically by when they were added. You can
do Linq stuff with it, foreach it, or just treat it like a List or
array!


## Examples

### Simple iteration
Walking through the Model's list of nodes is pretty
straightforward! This will touch every ModelNode in the Model,
in the order they were defined, regardless of hierarchy position
or contents.
```csharp
Log.Info("Iterate nodes:");
foreach (ModelNode node in model.Nodes)
	Log.Info("  "+ node.Name);
```
### Tagged Nodes
You can search through Visuals and Nodes for nodes with some sort
of tag in their names. Since these names are from your modeling
software, this can allow for some level of designer configuration
that can be specific to your project.
```csharp
var nodes = model.Visuals
	.Where(n => n.Name.Contains("[Wire]"));
foreach (ModelNode node in nodes)
{
	node.Material = node.Material.Copy();
	node.Material.Wireframe = true;
}
```
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

