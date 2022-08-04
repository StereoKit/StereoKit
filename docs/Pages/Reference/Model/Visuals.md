---
layout: default
title: Model.Visuals
description: This is an enumerable collection of all the nodes with Mesh/Material data in this Model, ordered non-hierarchically by when they were added. You can do Linq stuff with it, foreach it, or just treat it like a List or array!
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Visuals

<div class='signature' markdown='1'>
[ModelVisualCollection]({{site.url}}/Pages/Reference/ModelVisualCollection.html) Visuals{ get }
</div>

## Description
This is an enumerable collection of all the nodes with
Mesh/Material data in this Model, ordered non-hierarchically by
when they were added. You can do Linq stuff with it, foreach it, or
just treat it like a List or array!


## Examples

### Counting the Vertices and Triangles in a Model

Model.Visuals are always guaranteed to have a Mesh, so no need to
null check there, and VertCount and IndCount are available even if
Mesh.KeepData is false!
```csharp
int vertCount = 0;
int triCount  = 0;

foreach (ModelNode node in model.Visuals)
{
	Mesh mesh = node.Mesh;
	vertCount += mesh.VertCount;
	triCount  += mesh.IndCount / 3;
}
Log.Info($"Model stats: {vertCount} vertices, {triCount} triangles");
```
### Simple iteration of visual nodes
This will iterate through every ModelNode in the Model with visual
data attached to it!
```csharp
Log.Info("Iterate visuals:");
foreach (ModelNode node in model.Visuals)
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

