---
layout: default
title: Model.GetMaterial
description: Gets a link to the Material asset used by the model subset! Note that this is not necessarily a unique material, and could be shared in a number of other places. Consider copying and replacing it if you intend to modify it!
---
# [Model]({{site.url}}/Pages/Reference/Model.html).GetMaterial

<div class='signature' markdown='1'>
```csharp
Material GetMaterial(int subsetIndex)
```
Gets a link to the Material asset used by the model
subset! Note that this is not necessarily a unique material, and
could be shared in a number of other places. Consider copying and
replacing it if you intend to modify it!
</div>

|  |  |
|--|--|
|int subsetIndex|Index of the model subset to get the              Material for, should be less than SubsetCount.|
|RETURNS: [Material]({{site.url}}/Pages/Reference/Material.html)|A link to the Material asset used by the model subset at subsetIndex|





## Examples

```csharp
for (int i = 0; i < model.SubsetCount; i++)
{
	// GetMaterial will often returned a shared resource, so 
	// copy it if you don't wish to change all assets that 
	// share it.
	Material mat = model.GetMaterial(i).Copy();
	mat[MatParamName.ColorTint] = Color.HSV(0, 1, 1);
	model.SetMaterial(i, mat);
}
```

