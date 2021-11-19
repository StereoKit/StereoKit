---
layout: default
title: Material.Copy
description: Creates a new Material asset with the same shader and properties! Draw calls with the new Material will not batch together with this one.
---
# [Material]({{site.url}}/Pages/Reference/Material.html).Copy

<div class='signature' markdown='1'>
```csharp
Material Copy()
```
Creates a new Material asset with the same shader and
properties! Draw calls with the new Material will not batch
together with this one.
</div>

|  |  |
|--|--|
|RETURNS: [Material]({{site.url}}/Pages/Reference/Material.html)|A new Material asset with the same shader and properties.|

<div class='signature' markdown='1'>
```csharp
static Material Copy(string materialId)
```
Creates a new Material asset with the same shader and
properties! Draw calls with the new Material will not batch
together with the source Material.
</div>

|  |  |
|--|--|
|string materialId|Which Material are you looking for?|
|RETURNS: [Material]({{site.url}}/Pages/Reference/Material.html)|A new Material asset with the same shader and properties. Returns null if no materials are found with the given id.|





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

