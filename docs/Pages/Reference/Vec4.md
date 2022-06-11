---
layout: default
title: Vec4
description: A vector with 4 components. x, y, z, and w. Can be useful for things like shaders, where the registers are aligned to 4 float vectors.  This is a wrapper on System.Numerics.Vector4, so it's SIMD optimized, and can be cast to and from implicitly.
---
# struct Vec4

A vector with 4 components: x, y, z, and w. Can be useful
for things like shaders, where the registers are aligned to 4 float
vectors.

This is a wrapper on System.Numerics.Vector4, so it's SIMD optimized,
and can be cast to and from implicitly.

## Instance Fields and Properties

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) [Quat]({{site.url}}/Pages/Reference/Vec4/Quat.html)|A Vec4 and a Quat are only really different by name and purpose. So, if you need to do Quat math with your Vec4, or visa versa, who am I to judge?|
|Vector4 [v]({{site.url}}/Pages/Reference/Vec4/v.html)|The internal, wrapped System.Numerics type. This can be nice to have around so you can pass its fields as 'ref', which you can't do with properties. You won't often need this, as implicit conversions to System.Numerics types are also provided.|
|float [w]({{site.url}}/Pages/Reference/Vec4/w.html)|W component.|
|float [x]({{site.url}}/Pages/Reference/Vec4/x.html)|X component.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [XY]({{site.url}}/Pages/Reference/Vec4/XY.html)|This extracts a Vec2 from the X and Y axes.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [XYZ]({{site.url}}/Pages/Reference/Vec4/XYZ.html)|This extracts a Vec3 from the X, Y, and Z axes.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [XZ]({{site.url}}/Pages/Reference/Vec4/XZ.html)|This extracts a Vec2 from the X and Z axes.|
|float [y]({{site.url}}/Pages/Reference/Vec4/y.html)|Y component.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [YZ]({{site.url}}/Pages/Reference/Vec4/YZ.html)|This extracts a Vec2 from the Y and Z axes.|
|float [z]({{site.url}}/Pages/Reference/Vec4/z.html)|Z component.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [ZW]({{site.url}}/Pages/Reference/Vec4/ZW.html)|This extracts a Vec2 from the Z and W axes.|

## Instance Methods

|  |  |
|--|--|
|[Vec4]({{site.url}}/Pages/Reference/Vec4/Vec4.html)|A basic constructor, just copies the values in!|
|[ToString]({{site.url}}/Pages/Reference/Vec4/ToString.html)|Mostly for debug purposes, this is a decent way to log or inspect the vector in debug mode. Looks like "[x, y, z, w]"|

## Static Fields and Properties

|  |  |
|--|--|
|[Vec4]({{site.url}}/Pages/Reference/Vec4.html) [UnitW]({{site.url}}/Pages/Reference/Vec4/UnitW.html)|A normalized Vector that points down the W axis, this is the same as `new Vec4(0,1,0,0)`.|
|[Vec4]({{site.url}}/Pages/Reference/Vec4.html) [UnitX]({{site.url}}/Pages/Reference/Vec4/UnitX.html)|A normalized Vector that points down the X axis, this is the same as `new Vec4(1,0,0,0)`.|
|[Vec4]({{site.url}}/Pages/Reference/Vec4.html) [UnitY]({{site.url}}/Pages/Reference/Vec4/UnitY.html)|A normalized Vector that points down the Y axis, this is the same as `new Vec4(0,1,0,0)`.|
|[Vec4]({{site.url}}/Pages/Reference/Vec4.html) [UnitZ]({{site.url}}/Pages/Reference/Vec4/UnitZ.html)|A normalized Vector that points down the Z axis, this is the same as `new Vec4(0,1,0,0)`.|

## Static Methods

|  |  |
|--|--|
|[Dot]({{site.url}}/Pages/Reference/Vec4/Dot.html)|What's a dot product do for 4D vectors, you might ask? Well, I'm no mathematician, so hopefully you are! I've never used it before. Whatever you're doing with this function, it's SIMD fast!|
|[Lerp]({{site.url}}/Pages/Reference/Vec4/Lerp.html)|Blends (Linear Interpolation) between two vectors, based on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp percent for you.|
|[Max]({{site.url}}/Pages/Reference/Vec4/Max.html)|Returns a vector where each elements is the maximum value for each corresponding pair.|
|[Min]({{site.url}}/Pages/Reference/Vec4/Min.html)|Returns a vector where each elements is the minimum value for each corresponding pair.|

## Operators

|  |  |
|--|--|
|[+]({{site.url}}/Pages/Reference/Vec4/op_Addition.html)|Adds matching components together. Commutative.|
|[/]({{site.url}}/Pages/Reference/Vec4/op_Division.html)|A component-wise vector division. Not commutative|
|[Implicit Conversions]({{site.url}}/Pages/Reference/Vec4/op_Implicit.html)|Allows implicit conversion from System.Numerics.Vector4 to StereoKit.Vec4.|
|[*]({{site.url}}/Pages/Reference/Vec4/op_Multiply.html)|A component-wise vector multiplication, same thing as a non-uniform scale. NOT a dot product! Commutative.|
|[-]({{site.url}}/Pages/Reference/Vec4/op_Subtraction.html)|Subtracts matching components from eachother. Not commutative.|
|[-]({{site.url}}/Pages/Reference/Vec4/op_UnaryNegation.html)|Vector negation, returns a vector where each component has been negated.|
