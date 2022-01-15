---
layout: default
title: MatParamInfo.type
description: This is the data type that StereoKit recognizes the parameter to be.
---
# [MatParamInfo]({{site.url}}/Pages/Reference/MatParamInfo.html).type

<div class='signature' markdown='1'>
[MaterialParam]({{site.url}}/Pages/Reference/MaterialParam.html) type
</div>

## Description
This is the data type that StereoKit recognizes the
parameter to be.


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

