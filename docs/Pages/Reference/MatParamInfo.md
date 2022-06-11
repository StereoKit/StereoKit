---
layout: default
title: MatParamInfo
description: Information and details about the shader parameters available on a Material. Currently only the text name and data type of the parameter are surfaced here. This contains textures as well as global constant buffer variables.
---
# struct MatParamInfo

Information and details about the shader parameters
available on a Material. Currently only the text name and data type
of the parameter are surfaced here. This contains textures as well as
global constant buffer variables.

## Instance Fields and Properties

|  |  |
|--|--|
|string [name]({{site.url}}/Pages/Reference/MatParamInfo/name.html)|The name of the shader parameter, as provided inside of the shader file itself. These are the 'global' variables defined in the shader, as well as textures. The shader compiler will output a list of parameter names when compiling, so check the output window after building if you're uncertain what you'll find.  See `MatParamName` for a list of "standardized" parameter names.|
|[MaterialParam]({{site.url}}/Pages/Reference/MaterialParam.html) [type]({{site.url}}/Pages/Reference/MatParamInfo/type.html)|This is the data type that StereoKit recognizes the parameter to be.|

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

