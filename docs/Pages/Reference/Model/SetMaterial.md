---
layout: default
title: Model.SetMaterial
description: Changes the Material for the subset to a new one!
---
# [Model]({{site.url}}/Pages/Reference/Model.html).SetMaterial

<div class='signature' markdown='1'>
```csharp
void SetMaterial(int subsetIndex, Material material)
```
Changes the Material for the subset to a new one!
</div>

|  |  |
|--|--|
|int subsetIndex|Index of the model subset to replace,              should be less than SubsetCount.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|The new Material, cannot be null.|





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

