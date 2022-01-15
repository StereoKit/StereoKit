---
layout: default
title: MatParamInfo.name
description: The name of the shader parameter, as provided inside of the shader file itself. These are the 'global' variables defined in the shader, as well as textures. The shader compiler will output a list of parameter names when compiling, so check the output window after building if you're uncertain what you'll find.  See MatParamName for a list of "standardized" parameter names.
---
# [MatParamInfo]({{site.url}}/Pages/Reference/MatParamInfo.html).name

<div class='signature' markdown='1'>
string name
</div>

## Description
The name of the shader parameter, as provided inside of
the shader file itself. These are the 'global' variables defined
in the shader, as well as textures. The shader compiler will
output a list of parameter names when compiling, so check the
output window after building if you're uncertain what you'll
find.

See `MatParamName` for a list of "standardized" parameter names.


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

