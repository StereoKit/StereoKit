---
layout: default
title: Vec2
description: A vector with 2 components. x and y. This can represent a point in 2D space, a directional vector, or any other sort of value with 2 dimensions to it!
---
# struct Vec2

A vector with 2 components: x and y. This can represent a
point in 2D space, a directional vector, or any other sort of value
with 2 dimensions to it!

## Instance Fields and Properties

|  |  |
|--|--|
|float [Length]({{site.url}}/Pages/Reference/Vec2/Length.html)|This is the length of the vector! Or the distance from the origin to this point. Uses Math.Sqrt, so it's not dirt cheap or anything.|
|float [LengthSq]({{site.url}}/Pages/Reference/Vec2/LengthSq.html)|This is the squared length/magnitude of the vector! It skips the Sqrt call, and just gives you the squared version for speedy calculations that can work with it squared.|
|float [Magnitude]({{site.url}}/Pages/Reference/Vec2/Magnitude.html)|Magnitude is the length of the vector! Or the distance from the origin to this point. Uses Math.Sqrt, so it's not dirt cheap or anything.|
|float [MagnitudeSq]({{site.url}}/Pages/Reference/Vec2/MagnitudeSq.html)|This is the squared magnitude of the vector! It skips the Sqrt call, and just gives you the squared version for speedy calculations that can work with it squared.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [Normalized]({{site.url}}/Pages/Reference/Vec2/Normalized.html)|Creates a normalized vector (vector with a length of 1) from the current vector. Will not work properly if the vector has a length of zero.|
|Vector2 [v]({{site.url}}/Pages/Reference/Vec2/v.html)|The internal, wrapped System.Numerics type. This can be nice to have around so you can pass its fields as 'ref', which you can't do with properties. You won't often need this, as implicit conversions to System.Numerics types are also provided.|
|float [x]({{site.url}}/Pages/Reference/Vec2/x.html)|X component.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [X0Y]({{site.url}}/Pages/Reference/Vec2/X0Y.html)|Promotes this Vec2 to a Vec3, using 0 for the Y axis.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [XY0]({{site.url}}/Pages/Reference/Vec2/XY0.html)|Promotes this Vec2 to a Vec3, using 0 for the Z axis.|
|float [y]({{site.url}}/Pages/Reference/Vec2/y.html)|Y component.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [YX]({{site.url}}/Pages/Reference/Vec2/YX.html)|A transpose swizzle property, returns (y,x)|

## Instance Methods

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2/Vec2.html)|A basic constructor, just copies the values in!|
|[Angle]({{site.url}}/Pages/Reference/Vec2/Angle.html)|Returns the counter-clockwise degrees from [1,0]. Resulting value is between 0 and 360. Vector does not need to be normalized.|
|[InRadius]({{site.url}}/Pages/Reference/Vec2/InRadius.html)|Checks if a point is within a certain radius of this one. This is an easily readable shorthand of the squared distance check.|
|[Normalize]({{site.url}}/Pages/Reference/Vec2/Normalize.html)|Turns this vector into a normalized vector (vector with a length of 1) from the current vector. Will not work properly if the vector has a length of zero.|
|[ToString]({{site.url}}/Pages/Reference/Vec2/ToString.html)|Mostly for debug purposes, this is a decent way to log or inspect the vector in debug mode. Looks like "[x, y]"|

## Static Fields and Properties

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [One]({{site.url}}/Pages/Reference/Vec2/One.html)|A Vec2 with all components at one, this is the same as `new Vec2(1,1)`.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [UnitX]({{site.url}}/Pages/Reference/Vec2/UnitX.html)|A normalized Vector that points down the X axis, this is the same as `new Vec2(1,0)`.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [UnitY]({{site.url}}/Pages/Reference/Vec2/UnitY.html)|A normalized Vector that points down the Y axis, this is the same as `new Vec2(0,1)`.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [Zero]({{site.url}}/Pages/Reference/Vec2/Zero.html)|A Vec2 with all components at zero, this is the same as `new Vec2(0,0)`.|

## Static Methods

|  |  |
|--|--|
|[AngleBetween]({{site.url}}/Pages/Reference/Vec2/AngleBetween.html)|Calculates a signed angle between two vectors in degrees! Sign will be positive if B is counter-clockwise (left) of A, and negative if B is clockwise (right) of A. Vectors do not need to be normalized. NOTE: Since this will return a positive or negative angle, order of parameters matters!|
|[Distance]({{site.url}}/Pages/Reference/Vec2/Distance.html)|Calculates the distance between two points in space! Make sure they're in the same coordinate space! Uses a Sqrt, so it's not blazing fast, prefer DistanceSq when possible.|
|[DistanceSq]({{site.url}}/Pages/Reference/Vec2/DistanceSq.html)|Calculates the distance between two points in space, but leaves them squared! Make sure they're in the same coordinate space! This is a fast function :)|
|[Dot]({{site.url}}/Pages/Reference/Vec2/Dot.html)|The dot product is an extremely useful operation! One major use is to determine how similar two vectors are. If the vectors are Unit vectors (magnitude/length of 1), then the result will be 1 if the vectors are the same, -1 if they're opposite, and a gradient in-between with 0 being perpendicular. See [Freya Holmer's excellent visualization](https://twitter.com/FreyaHolmer/status/1200807790580768768) of this concept|
|[FromAngle]({{site.url}}/Pages/Reference/Vec2/FromAngle.html)|Creates a vector pointing in the direction of the angle, with a length of 1. Angles are counter-clockwise, and start from (1,0), so an angle of 90 will be (0,1).|
|[Lerp]({{site.url}}/Pages/Reference/Vec2/Lerp.html)|Blends (Linear Interpolation) between two vectors, based on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp percent for you.|
|[Max]({{site.url}}/Pages/Reference/Vec2/Max.html)|Returns a vector where each elements is the maximum value for each corresponding pair.|
|[Min]({{site.url}}/Pages/Reference/Vec2/Min.html)|Returns a vector where each elements is the minimum value for each corresponding pair.|

## Operators

|  |  |
|--|--|
|[+]({{site.url}}/Pages/Reference/Vec2/op_Addition.html)|Adds matching components together. Commutative.|
|[/]({{site.url}}/Pages/Reference/Vec2/op_Division.html)|A component-wise vector division. Not commutative|
|[Implicit Conversions]({{site.url}}/Pages/Reference/Vec2/op_Implicit.html)|Allows implicit conversion from System.Numerics.Vector2 to StereoKit.Vec2.|
|[*]({{site.url}}/Pages/Reference/Vec2/op_Multiply.html)|A component-wise vector multiplication, same thing as a non-uniform scale. NOT a dot product! Commutative.|
|[-]({{site.url}}/Pages/Reference/Vec2/op_Subtraction.html)|Subtracts matching components from eachother. Not commutative.|
|[-]({{site.url}}/Pages/Reference/Vec2/op_UnaryNegation.html)|Vector negation, returns a vector where each component has been negated.|
