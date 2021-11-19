---
layout: default
title: Model.Copy
description: Creates a shallow copy of a Model asset! Meshes and Materials referenced by this Model will be referenced, not copied.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Copy

<div class='signature' markdown='1'>
```csharp
Model Copy()
```
Creates a shallow copy of a Model asset! Meshes and
Materials referenced by this Model will be referenced, not
copied.
</div>

|  |  |
|--|--|
|RETURNS: [Model]({{site.url}}/Pages/Reference/Model.html)|A new shallow copy of a Model.|





## Examples

### Copying assets
Modifying an asset will affect everything that uses that asset!
Often you'll want to copy an asset before modifying it, to
ensure other parts of your application look the same. In
particular, modifying default assets is not a good idea, unless
you _do_ want to modify the defaults globally.
```csharp
Model model1 = new Model(Mesh.Sphere, Material.Default);
model1.RootNode.LocalTransform = Matrix.S(0.1f);

Material mat = Material.Default.Copy();
mat[MatParamName.ColorTint] = new Color(1,0,0,1);
Model model2 = model1.Copy();
model2.RootNode.Material = mat;
```

