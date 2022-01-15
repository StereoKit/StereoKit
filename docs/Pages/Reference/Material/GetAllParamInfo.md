---
layout: default
title: Material.GetAllParamInfo
description: Gets an enumerable list of all parameter information on the Material. This includes all global shader variables and textures.
---
# [Material]({{site.url}}/Pages/Reference/Material.html).GetAllParamInfo

<div class='signature' markdown='1'>
```csharp
IEnumerable`1 GetAllParamInfo()
```
Gets an enumerable list of all parameter information on
the Material. This includes all global shader variables and
textures.
</div>

|  |  |
|--|--|
|RETURNS: IEnumerable`1|A pretty standard IEnumarable of MatParamInfo that can be used with foreach.|





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

