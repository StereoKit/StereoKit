---
layout: default
title: Material.SetData
description: This allows you to set more complex shader data types such as structs. Note the SK doesn't guard against setting data of the wrong size here, so pay extra attention to the size of your data here, and ensure it matched up with the shader!
---
# [Material]({{site.url}}/Pages/Reference/Material.html).SetData

<div class='signature' markdown='1'>
```csharp
void SetData(string name, T& serializableData)
```
This allows you to set more complex shader data types such
as structs. Note the SK doesn't guard against setting data of the
wrong size here, so pay extra attention to the size of your data
here, and ensure it matched up with the shader!
</div>

|  |  |
|--|--|
|string name|Name of the shader parameter.|
|T& serializableData|New value for the parameter.|





## Examples

### Assigning an array in a Shader
This is a bit of a hack until proper shader support for arrays arrives,
but with a few C# marshalling tricks, we can assign array without too
much trouble. Look for improvements to this in later versions of
SteroKit.
```csharp
// This struct matches a shader parameter of `float4 offsets[10];`
[StructLayout(LayoutKind.Sequential)]
struct ShaderData
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
	public Vec4[] offsets;
}

Material arrayMaterial = null;
public void Initialize()
{
	ShaderData shaderData = new ShaderData();
	shaderData.offsets = new Vec4[10] {
		V.XYZW(0,0,0,0),
		V.XYZW(0.2f,0,0,0),
		V.XYZW(0.4f,0,0,0),
		V.XYZW(0.4f,0.2f,0,0),
		V.XYZW(0.4f,0.4f,0,0),
		V.XYZW(0.4f,0.6f,0,0),
		V.XYZW(0.2f,0.6f,0,0),
		V.XYZW(0,0.6f,0,0),
		V.XYZW(0,0.4f,0,0),
		V.XYZW(0,0.2f,0,0)};
	
	arrayMaterial = new Material(Shader.FromFile("shader_arrays.hlsl"));
	arrayMaterial[MatParamName.DiffuseTex] = Tex.FromFile("test.png");
	arrayMaterial.SetData<ShaderData>("offsets", shaderData);
}
```

