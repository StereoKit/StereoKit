---
layout: default
title: Model.SubsetCount
description: The number of mesh subsets attached to this model.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).SubsetCount

<div class='signature' markdown='1'>
int SubsetCount{ get }
</div>

## Description
The number of mesh subsets attached to this model.


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

