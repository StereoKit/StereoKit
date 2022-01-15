---
layout: default
title: Material.ParamCount
description: The number of shader parameters available to this material, includes global shader variables as well as textures.
---
# [Material]({{site.url}}/Pages/Reference/Material.html).ParamCount

<div class='signature' markdown='1'>
int ParamCount{ get }
</div>

## Description
The number of shader parameters available to this
material, includes global shader variables as well as textures.


## Examples

### Listing parameters in a Material
```csharp
// Iterate using a foreach
Log.Info("Builtin PBR Materials contain these parameters:");
foreach (MatParamInfo info in Material.PBR.GetAllParamInfo())
	Log.Info($"- {info.name} : {info.type}");

// Or with a normal for loop
Log.Info("Builtin Unlit Materials contain these parameters:");
for (int i=0; i<Material.Unlit.ParamCount; i+=1)
{
	MatParamInfo info = Material.Unlit.GetParamInfo(i);
	Log.Info($"- {info.name} : {info.type}");
}
```

