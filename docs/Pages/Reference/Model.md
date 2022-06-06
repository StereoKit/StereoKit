---
layout: default
title: Model
description: A Model is a collection of meshes, materials, and transforms that make up a visual element! This is a great way to group together complex objects that have multiple parts in them, and in fact, most model formats are composed this way already!  This class contains a number of methods for creation. If you pass in a .obj, .stl, , .ply (ASCII), .gltf, or .glb, StereoKit will load that model from file, and assemble materials and transforms from the file information. But you can also assemble a model from procedurally generated meshes!  Because models include an offset transform for each mesh element, this does have the overhead of an extra matrix multiplication in order to execute a render command. So if you need speed, and only have a single mesh with a precalculated transform matrix, it can be faster to render a Mesh instead of a Model!
---
# class Model

A Model is a collection of meshes, materials, and transforms
that make up a visual element! This is a great way to group together
complex objects that have multiple parts in them, and in fact, most
model formats are composed this way already!

This class contains a number of methods for creation. If you pass in
a .obj, .stl, , .ply (ASCII), .gltf, or .glb, StereoKit will load
that model from file, and assemble materials and transforms from the
file information. But you can also assemble a model from procedurally
generated meshes!

Because models include an offset transform for each mesh element,
this does have the overhead of an extra matrix multiplication in
order to execute a render command. So if you need speed, and only
have a single mesh with a precalculated transform matrix, it can be
faster to render a Mesh instead of a Model!

## Instance Fields and Properties

|  |  |
|--|--|
|[Anim]({{site.url}}/Pages/Reference/Anim.html) [ActiveAnim]({{site.url}}/Pages/Reference/Model/ActiveAnim.html)|This is a link to the currently active animation. If no animation is active, this value will be null. To set the active animation, use `PlayAnim`.|
|float [AnimCompletion]({{site.url}}/Pages/Reference/Model/AnimCompletion.html)|This is the percentage of completion of the active animation. This will always be a value between 0-1. If no animation is active, this will be zero.|
|[AnimMode]({{site.url}}/Pages/Reference/AnimMode.html) [AnimMode]({{site.url}}/Pages/Reference/Model/AnimMode.html)|The playback mode of the active animation.|
|[ModelAnimCollection]({{site.url}}/Pages/Reference/ModelAnimCollection.html) [Anims]({{site.url}}/Pages/Reference/Model/Anims.html)|An enumerable collection of animations attached to this Model. You can do Linq stuff with it, foreach it, or just treat it like a List or array!|
|float [AnimTime]({{site.url}}/Pages/Reference/Model/AnimTime.html)|This is the current time of the active animation in seconds, from the start of the animation. If no animation is active, this will be zero. This will always be a value between zero and the active animation's `Duration`. For a percentage of completion, see `AnimCompletion` instead.|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) [Bounds]({{site.url}}/Pages/Reference/Model/Bounds.html)|This is a bounding box that encapsulates the Model and all its subsets! It's used for collision, visibility testing, UI layout, and probably other things. While it's normally calculated from the mesh bounds, you can also override this to suit your needs.|
|[ModelNodeCollection]({{site.url}}/Pages/Reference/ModelNodeCollection.html) [Nodes]({{site.url}}/Pages/Reference/Model/Nodes.html)|This is an enumerable collection of all the nodes in this Model, ordered non-hierarchically by when they were added. You can do Linq stuff with it, foreach it, or just treat it like a List or array!|
|[ModelNode]({{site.url}}/Pages/Reference/ModelNode.html) [RootNode]({{site.url}}/Pages/Reference/Model/RootNode.html)|Returns the first root node in the Model's hierarchy. There may be additional root nodes, and these will be Siblings of this ModelNode. If there are no nodes present on the Model, this will be null.|
|int [SubsetCount]({{site.url}}/Pages/Reference/Model/SubsetCount.html)|The number of mesh subsets attached to this model.|
|[ModelVisualCollection]({{site.url}}/Pages/Reference/ModelVisualCollection.html) [Visuals]({{site.url}}/Pages/Reference/Model/Visuals.html)|This is an enumerable collection of all the nodes with Mesh/Material data in this Model, ordered non-hierarchically by when they were added. You can do Linq stuff with it, foreach it, or just treat it like a List or array!|

## Instance Methods

|  |  |
|--|--|
|[Model]({{site.url}}/Pages/Reference/Model/Model.html)|Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.|
|[AddNode]({{site.url}}/Pages/Reference/Model/AddNode.html)|This adds a root node to the `Model`'s node hierarchy! If There is already an initial root node, this node will still be a root node, but will be a `Sibling` of the `Model`'s `RootNode`. If this is the first root node added, you'll be able to access it via `RootNode`.|
|[AddSubset]({{site.url}}/Pages/Reference/Model/AddSubset.html)|Adds a new subset to the Model, and recalculates the bounds. A default subset name of "subsetX" will be used, where X is the subset's index.|
|[Copy]({{site.url}}/Pages/Reference/Model/Copy.html)|Creates a shallow copy of a Model asset! Meshes and Materials referenced by this Model will be referenced, not copied.|
|[Draw]({{site.url}}/Pages/Reference/Model/Draw.html)|Adds this Model to the render queue for this frame! If the Hierarchy has a transform on it, that transform is combined with the Matrix provided here.|
|[FindAnim]({{site.url}}/Pages/Reference/Model/FindAnim.html)|Searches the list of animations for the first one matching the given name.|
|[FindNode]({{site.url}}/Pages/Reference/Model/FindNode.html)|Searches the entire list of Nodes, and will return the first on that matches this name exactly. If no ModelNode is found, then this will return null. Node Names are not guaranteed to be unique.|
|[GetMaterial]({{site.url}}/Pages/Reference/Model/GetMaterial.html)|Gets a link to the Material asset used by the model subset! Note that this is not necessarily a unique material, and could be shared in a number of other places. Consider copying and replacing it if you intend to modify it!|
|[GetMesh]({{site.url}}/Pages/Reference/Model/GetMesh.html)|Gets a link to the Mesh asset used by the model subset! Note that this is not necessarily a unique mesh, and could be shared in a number of other places. Consider copying and replacing it if you intend to modify it!|
|[GetName]({{site.url}}/Pages/Reference/Model/GetName.html)|Returns the name of the specific subset! This will be the node name of your model asset. If no node name is available, SteroKit will generate a name in the format of "subsetX", where X would be the subset index. Note that names are not guaranteed to be unique (users may assign the same name to multiple nodes). Some nodes may also produce multiple subsets with the same name, such as when a node contains a Mesh with multiple Materials, each Mesh/Material combination will receive a subset with the same name.|
|[GetTransform]({{site.url}}/Pages/Reference/Model/GetTransform.html)|Gets the transform matrix used by the model subset!|
|[Intersect]({{site.url}}/Pages/Reference/Model/Intersect.html)|Checks the intersection point of this ray and a Model's visual nodes. This will skip any node that is not flagged as Solid, as well as any Mesh without collision data. Ray must be in model space, intersection point will be in model space too. You can use the inverse of the mesh's world transform matrix to bring the ray into model space, see the example in the docs!|
|[PlayAnim]({{site.url}}/Pages/Reference/Model/PlayAnim.html)|Searches for an animation with the given name, and if it's found, sets it up as the active animation and begins playing it with the animation mode.|
|[RecalculateBounds]({{site.url}}/Pages/Reference/Model/RecalculateBounds.html)|Examines the visuals as they currently are, and rebuilds the bounds based on that! This is normally done automatically, but if you modify a Mesh that this Model is using, the Model can't see it, and you should call this manually.|
|[RemoveSubset]({{site.url}}/Pages/Reference/Model/RemoveSubset.html)|Removes and dereferences a subset from the model.|
|[SetMaterial]({{site.url}}/Pages/Reference/Model/SetMaterial.html)|Changes the Material for the subset to a new one!|
|[SetMesh]({{site.url}}/Pages/Reference/Model/SetMesh.html)|Changes the mesh for the subset to a new one!|
|[SetTransform]({{site.url}}/Pages/Reference/Model/SetTransform.html)|Changes the transform for the subset to a new one! This is in Model space, so it's relative to the origin of the model.|
|[StepAnim]({{site.url}}/Pages/Reference/Model/StepAnim.html)|Calling Draw will automatically step the Model's animation, but if you don't draw the Model, or need access to the animated nodes before drawing, then you can step the animation early manually via this method. Animation will only ever be stepped once per frame, so it's okay to call this multiple times, or in addition to Draw.|

## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Model/Find.html)|Looks for a Model asset that's already loaded, matching the given id!|
|[FromFile]({{site.url}}/Pages/Reference/Model/FromFile.html)|Loads a list of mesh and material subsets from a .obj, .stl, .ply (ASCII), .gltf, or .glb file.|
|[FromMemory]({{site.url}}/Pages/Reference/Model/FromMemory.html)|Loads a list of mesh and material subsets from a .obj, .stl, .ply (ASCII), .gltf, or .glb file stored in memory. Note that this function won't work well on files that reference other files, such as .gltf files with references in them.|
|[FromMesh]({{site.url}}/Pages/Reference/Model/FromMesh.html)|Creates a single mesh subset Model using the indicated Mesh and Material! An id will be automatically generated for this asset.|

## Examples

### Loading an animated Model
Here, we're loading a Model that we know has the animations "Idle"
and "Jump". This sample shows some options, but only a single call
to PlayAnim is necessary to start an animation.
```csharp
Model model = Model.FromFile("Cosmonaut.glb");

// You can look at the model's animations:
foreach (Anim anim in model.Anims)
	Log.Info($"Animation: {anim.Name} {anim.Duration}s");

// You can play an animation like this
model.PlayAnim("Jump", AnimMode.Once);

// Or you can find and store the animations in advance
Anim jumpAnim = model.FindAnim("Idle");
if (jumpAnim != null)
	model.PlayAnim(jumpAnim, AnimMode.Loop);
```

### Animation progress bar
A really simple progress bar visualization for the Model's active
animation.

![Model with progress bar]({{site.screen_url}}/AnimProgress.jpg)
```csharp
model.Draw(Matrix.Identity);

Hierarchy.Push(Matrix.T(0.5f, 1, -.25f));

// This is a pair of green lines that show the current progress through
// the animation.
float progress = model.AnimCompletion;
Lines.Add(V.XY0(0, 0), V.XY0(-progress, 0),  new Color(0,1,0,1.0f), 2*U.cm);
Lines.Add(V.XY0(-progress, 0), V.XY0(-1, 0), new Color(0,1,0,0.2f), 2*U.cm);

// These are some labels for the progress bar that tell us more about
// the active animation.
Text.Add($"{model.ActiveAnim.Name} : {model.AnimMode}", Matrix.TS(0, -2*U.cm, 0, 3),        TextAlign.TopLeft);
Text.Add($"{model.AnimTime:F1}s",                       Matrix.TS(-progress, 2*U.cm, 0, 3), TextAlign.BottomCenter);

Hierarchy.Pop();
```

### An Interactive Model

![A grabbable GLTF Model using UI.Handle]({{site.screen_url}}/HandleBox.jpg)

If you want to grab a Model and move it around, then you can use a
`UI.Handle` to do it! Here's an example of loading a GLTF from file,
and using its information to create a Handle and a UI 'cage' box that
indicates an interactive element.

```csharp
Model model      = Model.FromFile("DamagedHelmet.gltf");
Pose  handlePose = new Pose(0,0,0, Quat.Identity);
float scale      = .15f;

public void Step() {
	UI.HandleBegin("Model Handle", ref handlePose, model.Bounds*scale);

	model.Draw(Matrix.S(scale));
	Mesh.Cube.Draw(Material.UIBox, Matrix.TS(model.Bounds.center*scale, model.Bounds.dimensions*scale));

	UI.HandleEnd();
}
```

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

### Non-recursive depth first node traversal
If you need to walk through a Model's node hierarchy, this is a method
of doing this without recursion! You essentially do this by walking the
tree down (Child) and to the right (Sibling), and if neither is present,
then walking back up (Parent) until it can keep going right (Sibling)
and then down (Child) again.
```csharp
static void DepthFirstTraversal(Model model)
{
	ModelNode node  = model.RootNode;
	int       depth = 0;
	while (node != null)
	{
		string tabs = new string(' ', depth*2);
		Log.Info(tabs + node.Name);

		if      (node.Child   != null) { node = node.Child; depth++; }
		else if (node.Sibling != null)   node = node.Sibling;
		else {
			while (node != null)
			{
				if (node.Sibling != null) {
					node = node.Sibling;
					break;
				}
				depth--;
				node = node.Parent;
			}
		}
	}
}
```

### Recursive depth first node traversal
Recursive depth first traversal is a little simpler to implement as
long as you don't mind some recursion :)
This would be called like: `RecursiveTraversal(model.RootNode);`
```csharp
static void RecursiveTraversal(ModelNode node, int depth = 0)
{
	string tabs = new string(' ', depth*2);
	while (node != null)
	{
		Log.Info(tabs + node.Name);
		RecursiveTraversal(node.Child, depth + 1);
		node = node.Sibling;
	}
}
```

