---
layout: default
title: Vec2
description: A vector with 2 components. x and y. This can represent a point in 2D space, a directional vector, or any other sort of value with 2 dimensions to it!
---
# Vec2

A vector with 2 components: x and y. This can represent a
point in 2D space, a directional vector, or any other sort of value
with 2 dimensions to it!


## Instance Fields and Properties

|  |  |
|--|--|
|float [x]({{site.url}}/Pages/Reference/Vec2/x.html)|Vector components.|
|float [y]({{site.url}}/Pages/Reference/Vec2/y.html)|Vector components.|


## Instance Methods

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2/Vec2.html)|A basic constructor, just copies the values in!|
|[Angle]({{site.url}}/Pages/Reference/Vec2/Angle.html)|Returns the counter-clockwise degrees from [1,0]. Resulting value is between 0 and 360. Vector does not need to be normalized.|
|[Normalize]({{site.url}}/Pages/Reference/Vec2/Normalize.html)|Turns this vector into a normalized vector (vector with a length of 1) from the current vector. Will not work properly if the vector has a length of zero.|
|[Normalized]({{site.url}}/Pages/Reference/Vec2/Normalized.html)|Creates a normalized vector (vector with a length of 1) from the current vector. Will not work properly if the vector has a length of zero.|


## Static Fields and Properties

|  |  |
|--|--|
|float [Magnitude]({{site.url}}/Pages/Reference/Vec2/Magnitude.html)|Magnitude is the length of the vector! Or the distance from the origin to this point. Uses Math.Sqrt, so it's not dirt cheap or anything.|
|float [MagnitudeSq]({{site.url}}/Pages/Reference/Vec2/MagnitudeSq.html)|This is the squared magnitude of the vector! It skips the Sqrt call, and just gives you the squared version for speedy calculations that can work with it squared.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [One]({{site.url}}/Pages/Reference/Vec2/One.html)|A Vec2 with all components at one, same as new Vec2(1,1).|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [Zero]({{site.url}}/Pages/Reference/Vec2/Zero.html)|A Vec2 with all components at zero, same as new Vec2(0,0).|


## Static Methods

|  |  |
|--|--|
|[AngleBetween]({{site.url}}/Pages/Reference/Vec2/AngleBetween.html)|Calculates a signed angle between two vectors! Sign will be positive if B is counter-clockwise (left) of A, and negative if B is clockwise (right) of A. Vectors do not need to be normalized.|
|[Distance]({{site.url}}/Pages/Reference/Vec2/Distance.html)|Calculates the distance between two points in space! Make sure they're in the same coordinate space! Uses a Sqrt, so it's not blazing fast, prefer DistanceSq when possible.|
|[DistanceSq]({{site.url}}/Pages/Reference/Vec2/DistanceSq.html)|Calculates the distance between two points in space, but leaves them squared! Make sure they're in the same coordinate space! This is a fast function :)|
|[Dot]({{site.url}}/Pages/Reference/Vec2/Dot.html)|The dot product is an extremely useful operation! One major use is to determine how similar two vectors are. If the vectors are Unit vectors (magnitude/length of 1), then the result will be 1 if the vectors are the same, -1 if they're opposite, and a gradient in-between with 0 being perpendicular. See [Freya Holmer's excellent visualization](https://twitter.com/FreyaHolmer/status/1200807790580768768) of this concept|

