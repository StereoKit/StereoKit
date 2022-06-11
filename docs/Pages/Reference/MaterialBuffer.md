---
layout: default
title: MaterialBuffer
description: This is a chunk of memory that will get bound to all shaders at a particular register slot. StereoKit uses this to provide engine values to the shader, and you can also use this to drive graphical shader systems of your own!  For example, if your application has a custom lighting system, fog, wind, or some other system that multiple shaders might need to refer to, this is the perfect tool to use.  The type 'T' for this buffer must be a struct that uses the [StructLayout(LayoutKind.Sequential)] attribute for proper copying. It should also match the layout of your equivalent cbuffer in the shader file. Note that shaders often have [specific byte alignment](https.//docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules) requirements! Example.  C# csharp [StructLayout(LayoutKind.Sequential)] struct BufferData { Vec3  windDirection; float windStrength }   HLSL c cbuffer BufferData . register(b3) { float3 windDirection; float  windStrength; } 
---
# class MaterialBuffer

This is a chunk of memory that will get bound to all shaders
at a particular register slot. StereoKit uses this to provide engine
values to the shader, and you can also use this to drive graphical
shader systems of your own!

For example, if your application has a custom lighting system, fog,
wind, or some other system that multiple shaders might need to refer
to, this is the perfect tool to use.

The type 'T' for this buffer must be a struct that uses the
`[StructLayout(LayoutKind.Sequential)]` attribute for proper copying.
It should also match the layout of your equivalent cbuffer in the
shader file. Note that shaders often have [specific byte alignment](https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules)
requirements! Example:

C#
```csharp
[StructLayout(LayoutKind.Sequential)]
struct BufferData {
Vec3  windDirection;
float windStrength
}
```

HLSL
```c
cbuffer BufferData : register(b3) {
float3 windDirection;
float  windStrength;
}
```

## Instance Methods

|  |  |
|--|--|
|[MaterialBuffer]({{site.url}}/Pages/Reference/MaterialBuffer/MaterialBuffer.html)|Create a new global MaterialBuffer bound to the register slot id. All shaders will have access to the data provided via this instance's `Set`.|
|[Set]({{site.url}}/Pages/Reference/MaterialBuffer/Set.html)|This will upload your data to the GPU for shaders to use.|
