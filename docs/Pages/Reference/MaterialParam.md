---
layout: default
title: MaterialParam
description: TODO. v0.4 This may need significant revision? What type of data does this material parameter need? This is used to tell the shader how large the data is, and where to attach it to on the shader.
---
# enum MaterialParam

TODO: v0.4 This may need significant revision?
What type of data does this material parameter need? This is
used to tell the shader how large the data is, and where to attach it
to on the shader.

## Enum Values

|  |  |
|--|--|
|Color128|A color value described by 4 floating point values. Memory-wise this is the same as a Vector4, but in the shader this variable has a ':color' tag applied to it using StereoKits's shader info syntax, indicating it's a color value. Color values for shaders should be in linear space, not gamma.|
|Float|A single 32 bit float value.|
|Int|A 1 component vector composed of signed integers.|
|Int2|A 2 component vector composed of signed integers.|
|Int3|A 3 component vector composed of signed integers.|
|Int4|A 4 component vector composed of signed integers.|
|Matrix|A 4x4 matrix of floats.|
|Texture|Texture information!|
|UInt|A 1 component vector composed of unsigned integers. This may also be a boolean.|
|UInt2|A 2 component vector composed of unsigned integers.|
|UInt3|A 3 component vector composed of unsigned integers.|
|UInt4|A 4 component vector composed of unsigned integers.|
|Unknown|This data type is not currently recognized. Please report your case on GitHub Issues!|
|Vector|A 4 component vector composed of floating point values. TODO: Remove in v0.4|
|Vector2|A 2 component vector composed of floating point values.|
|Vector3|A 3 component vector composed of floating point values.|
|Vector4|A 4 component vector composed of floating point values.|
