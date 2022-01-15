---
layout: default
title: Material.GetParamInfo
description: Gets available shader parameter information for the parameter at the indicated index. Parameters are listed as variables first, then textures.
---
# [Material]({{site.url}}/Pages/Reference/Material.html).GetParamInfo

<div class='signature' markdown='1'>
```csharp
MatParamInfo GetParamInfo(int index)
```
Gets available shader parameter information for the
parameter at the indicated index. Parameters are listed as
variables first, then textures.
</div>

|  |  |
|--|--|
|int index|Index of the shader parameter, bounded by             ParamCount.|
|RETURNS: [MatParamInfo]({{site.url}}/Pages/Reference/MatParamInfo.html)|A structure that contains all the available information about the parameter.|





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

