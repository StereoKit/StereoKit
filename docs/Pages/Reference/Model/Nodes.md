---
layout: default
title: Model.Nodes
description: This is an enumerable collection of all the nodes in this Model, ordered non-heirarchically by when they were added. You can do Linq stuff with it, foreach it, or just treat it like a List or array!
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Nodes

<div class='signature' markdown='1'>
[ModelNodeCollection]({{site.url}}/Pages/Reference/ModelNodeCollection.html) Nodes{ get }
</div>

## Description
This is an enumerable collection of all the nodes in this
Model, ordered non-heirarchically by when they were added. You can
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
You can search through Visuals and Nodes
```csharp
var nodes = model.Visuals
	.Where(n => n.Name.StartsWith("[Wire]"));
foreach (var node in nodes)
{
	node.Material = node.Material.Copy();
	node.Material.Wireframe = true;
}
```

