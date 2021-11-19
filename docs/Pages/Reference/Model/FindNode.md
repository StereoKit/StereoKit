---
layout: default
title: Model.FindNode
description: Searches the entire list of Nodes, and will return the first on that matches this name exactly. If no ModelNode is found, then this will return null. Node Names are not guaranteed to be unique.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).FindNode

<div class='signature' markdown='1'>
```csharp
ModelNode FindNode(string name)
```
Searches the entire list of Nodes, and will return the
first on that matches this name exactly. If no ModelNode is found,
then this will return null. Node Names are not guaranteed to be
unique.
</div>

|  |  |
|--|--|
|string name|Exact name to match against. ASCII only for now.|
|RETURNS: [ModelNode]({{site.url}}/Pages/Reference/ModelNode.html)|The first matching ModelNode, or null if none are found.|





## Examples

### Assembling a Model
While normally you'll load Models from file, you can also assemble
them yourself procedurally! This example shows assembling a simple
hierarchy of visual and empty nodes.
```csharp
Model model = new Model();
model
	.AddNode ("Root",    Matrix.S(0.2f), Mesh.Cube, Material.Default)
	.AddChild("Sub",     Matrix.TR (V.XYZ(0.5f, 0, 0), Quat.FromAngles(0, 0, 45)), Mesh.Cube, Material.Default)
	.AddChild("Surface", Matrix.TRS(V.XYZ(0.5f, 0, 0), Quat.LookDir(V.XYZ(1,0,0)), V.XYZ(1,1,1)));

ModelNode surfaceNode = model.FindNode("Surface");

surfaceNode.AddChild("UnitX", Matrix.T(Vec3.UnitX));
surfaceNode.AddChild("UnitY", Matrix.T(Vec3.UnitY));
surfaceNode.AddChild("UnitZ", Matrix.T(Vec3.UnitZ));
```

