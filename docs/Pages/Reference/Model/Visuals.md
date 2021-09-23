---
layout: default
title: Model.Visuals
description: This is an enumerable collection of all the nodes with Mesh/Material data in this Model, ordered non-heirarchically by when they were added. You can do Linq stuff with it, foreach it, or just treat it like a List or array!
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Visuals

<div class='signature' markdown='1'>
[ModelVisualCollection]({{site.url}}/Pages/Reference/ModelVisualCollection.html) Visuals{ get }
</div>

## Description
This is an enumerable collection of all the nodes with
Mesh/Material data in this Model, ordered non-heirarchically by
when they were added. You can do Linq stuff with it, foreach it, or
just treat it like a List or array!


## Examples

### Simple iteration of visual nodes
This will iterate through every ModelNode in the Model with visual
data attached to it!
```csharp
Log.Info("Iterate visuals:");
foreach (ModelNode node in model.Visuals)
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

