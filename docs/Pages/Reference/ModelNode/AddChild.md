---
layout: default
title: ModelNode.AddChild
description: Adds a Child node below this node, at the end of the child chain!
---
# [ModelNode]({{site.url}}/Pages/Reference/ModelNode.html).AddChild

<div class='signature' markdown='1'>
```csharp
ModelNode AddChild(string name, Matrix localTransform, Mesh mesh, Material material, bool solid)
```
Adds a Child node below this node, at the end of the child
chain!
</div>

|  |  |
|--|--|
|string name|A text name to identify the node. If null is             provided, it will be auto named to "node"+index.|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) localTransform|A Matrix describing this node's             transform in local space relative to the currently selected node.|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|The Mesh to attach to this Node's visual, if             this is null, then material must also be null.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|The Material to attach to this Node's             visual, if this is null, then mesh must also be null.|
|bool solid|A flag that indicates the Mesh for this node             will be used in ray intersection tests. This flag is ignored if no             Mesh is attached.|
|RETURNS: [ModelNode]({{site.url}}/Pages/Reference/ModelNode.html)|The new child ModelNode.|





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

