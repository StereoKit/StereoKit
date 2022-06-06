---
layout: default
title: Vec3
description: A vector with 3 components. x, y, z. This can represent a point in space, a directional vector, or any other sort of value with 3 dimensions to it!  StereoKit uses a right-handed coordinate system, where +x is to the right, +y is upwards, and -z is forward.
---
# struct Vec3

A vector with 3 components: x, y, z. This can represent a
point in space, a directional vector, or any other sort of value with
3 dimensions to it!

StereoKit uses a right-handed coordinate system, where +x is to the
right, +y is upwards, and -z is forward.

## Instance Fields and Properties

|  |  |
|--|--|
|float [Length]({{site.url}}/Pages/Reference/Vec3/Length.html)|This is the length, or magnitude of the vector! The distance from the origin to this point. Uses Math.Sqrt, so it's not dirt cheap or anything.|
|float [LengthSq]({{site.url}}/Pages/Reference/Vec3/LengthSq.html)|This is the squared length of the vector! It skips the Sqrt call, and just gives you the squared version for speedy calculations that can work with it squared.|
|float [Magnitude]({{site.url}}/Pages/Reference/Vec3/Magnitude.html)|Magnitude is the length of the vector! The distance from the origin to this point. Uses Math.Sqrt, so it's not dirt cheap or anything.|
|float [MagnitudeSq]({{site.url}}/Pages/Reference/Vec3/MagnitudeSq.html)|This is the squared magnitude of the vector! It skips the Sqrt call, and just gives you the squared version for speedy calculations that can work with it squared.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Normalized]({{site.url}}/Pages/Reference/Vec3/Normalized.html)|Creates a normalized vector (vector with a length of 1) from the current vector. Will not work properly if the vector has a length of zero.|
|Vector3 [v]({{site.url}}/Pages/Reference/Vec3/v.html)|The internal, wrapped System.Numerics type. This can be nice to have around so you can pass its fields as 'ref', which you can't do with properties. You won't often need this, as implicit conversions to System.Numerics types are also provided.|
|float [x]({{site.url}}/Pages/Reference/Vec3/x.html)|X component.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [X0Z]({{site.url}}/Pages/Reference/Vec3/X0Z.html)|This returns a Vec3 that has been flattened to 0 on the Y axis. No other changes are made.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [XY]({{site.url}}/Pages/Reference/Vec3/XY.html)|This extracts a Vec2 from the X and Y axes.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [XY0]({{site.url}}/Pages/Reference/Vec3/XY0.html)|This returns a Vec3 that has been flattened to 0 on the Z axis. No other changes are made.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [XZ]({{site.url}}/Pages/Reference/Vec3/XZ.html)|This extracts a Vec2 from the X and Z axes.|
|float [y]({{site.url}}/Pages/Reference/Vec3/y.html)|Y component.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [YZ]({{site.url}}/Pages/Reference/Vec3/YZ.html)|This extracts a Vec2 from the Y and Z axes.|
|float [z]({{site.url}}/Pages/Reference/Vec3/z.html)|Z component.|

## Instance Methods

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3/Vec3.html)|Creates a vector from x, y, and z values! StereoKit uses a right-handed metric coordinate system, where +x is to the right, +y is upwards, and -z is forward.|
|[InRadius]({{site.url}}/Pages/Reference/Vec3/InRadius.html)|Checks if a point is within a certain radius of this one. This is an easily readable shorthand of the squared distance check.|
|[Normalize]({{site.url}}/Pages/Reference/Vec3/Normalize.html)|Turns this vector into a normalized vector (vector with a length of 1) from the current vector. Will not work properly if the vector has a length of zero.|
|[ToString]({{site.url}}/Pages/Reference/Vec3/ToString.html)|Mostly for debug purposes, this is a decent way to log or inspect the vector in debug mode. Looks like "[x, y, z]"|

## Static Fields and Properties

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Forward]({{site.url}}/Pages/Reference/Vec3/Forward.html)|StereoKit uses a right-handed coordinate system, which means that forward is looking down the -Z axis! This value is the same as `new Vec3(0,0,-1)`. This is NOT the same as UnitZ!|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [One]({{site.url}}/Pages/Reference/Vec3/One.html)|Shorthand for a vector where all values are 1! Same as `new Vec3(1,1,1)`.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Right]({{site.url}}/Pages/Reference/Vec3/Right.html)|When looking forward, this is the direction to the right! In StereoKit, this is the same as `new Vec3(1,0,0)`|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [UnitX]({{site.url}}/Pages/Reference/Vec3/UnitX.html)|A normalized Vector that points down the X axis, this is the same as `new Vec3(1,0,0)`.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [UnitY]({{site.url}}/Pages/Reference/Vec3/UnitY.html)|A normalized Vector that points down the Y axis, this is the same as `new Vec3(0,1,0)`.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [UnitZ]({{site.url}}/Pages/Reference/Vec3/UnitZ.html)|A normalized Vector that points down the Z axis, this is the same as `new Vec3(0,0,1)`. This is NOT the same as Forward!|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Up]({{site.url}}/Pages/Reference/Vec3/Up.html)|A vector representing the up axis. In StereoKit, this is the same as `new Vec3(0,1,0)`.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Zero]({{site.url}}/Pages/Reference/Vec3/Zero.html)|Shorthand for a vector where all values are 0! Same as `new Vec3(0,0,0)`.|

## Static Methods

|  |  |
|--|--|
|[AngleBetween]({{site.url}}/Pages/Reference/Vec3/AngleBetween.html)|Calculates the angle between two vectors in degrees! Vectors do not need to be normalized, and the result will always be positive.|
|[AngleXY]({{site.url}}/Pages/Reference/Vec3/AngleXY.html)|Creates a vector that points out at the given 2D angle! This creates the vector on the XY plane, and allows you to specify a constant z value.|
|[AngleXZ]({{site.url}}/Pages/Reference/Vec3/AngleXZ.html)|Creates a vector that points out at the given 2D angle! This creates the vector on the XZ plane, and allows you to specify a constant y value.|
|[Cross]({{site.url}}/Pages/Reference/Vec3/Cross.html)|The cross product of two vectors!|
|[Distance]({{site.url}}/Pages/Reference/Vec3/Distance.html)|Calculates the distance between two points in space! Make sure they're in the same coordinate space! Uses a Sqrt, so it's not blazing fast, prefer DistanceSq when possible.|
|[DistanceSq]({{site.url}}/Pages/Reference/Vec3/DistanceSq.html)|Calculates the distance between two points in space, but leaves them squared! Make sure they're in the same coordinate space! This is a fast function :)|
|[Dot]({{site.url}}/Pages/Reference/Vec3/Dot.html)|The dot product is an extremely useful operation! One major use is to determine how similar two vectors are. If the vectors are Unit vectors (magnitude/length of 1), then the result will be 1 if the vectors are the same, -1 if they're opposite, and a gradient in-between with 0 being perpendicular. See [Freya Holmer's excellent visualization](https://twitter.com/FreyaHolmer/status/1200807790580768768) of this concept|
|[Lerp]({{site.url}}/Pages/Reference/Vec3/Lerp.html)|Blends (Linear Interpolation) between two vectors, based on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp percent for you.|
|[Max]({{site.url}}/Pages/Reference/Vec3/Max.html)|Returns a vector where each elements is the maximum value for each corresponding pair.|
|[Min]({{site.url}}/Pages/Reference/Vec3/Min.html)|Returns a vector where each elements is the minimum value for each corresponding pair.|
|[PerpendicularRight]({{site.url}}/Pages/Reference/Vec3/PerpendicularRight.html)|Exactly the same as Vec3.Cross, but has some naming mnemonics for getting the order right when trying to find a perpendicular vector using the cross product. This'll also make it more obvious to read if that's what you're actually going for when crossing vectors!  If you consider a forward vector and an up vector, then the direction to the right is pretty trivial to imagine in relation to those vectors!|

## Operators

|  |  |
|--|--|
|[+]({{site.url}}/Pages/Reference/Vec3/op_Addition.html)|Adds matching components together. Commutative.|
|[/]({{site.url}}/Pages/Reference/Vec3/op_Division.html)|A component-wise vector division. Not commutative|
|[Implicit Conversions]({{site.url}}/Pages/Reference/Vec3/op_Implicit.html)|Allows implicit conversion from System.Numerics.Vector3 to StereoKit.Vec3.|
|[*]({{site.url}}/Pages/Reference/Vec3/op_Multiply.html)|A component-wise vector multiplication, same thing as a non-uniform scale. NOT a dot or cross product! Commutative.|
|[-]({{site.url}}/Pages/Reference/Vec3/op_Subtraction.html)|Subtracts matching components from eachother. Not commutative.|
|[-]({{site.url}}/Pages/Reference/Vec3/op_UnaryNegation.html)|Vector negation, returns a vector where each component has been negated.|
