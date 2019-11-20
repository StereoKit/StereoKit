---
layout: default
title: Vec3
description: A vector with 3 components. x, y, z. This can represent a point in space, a directional vector, or any other sort of value with 3 dimensions to it!  StereoKit uses a right-handed coordinate system, where +x is to the right, +y is upwards, and -z is forward.
---
# Vec3

A vector with 3 components: x, y, z. This can represent a point in space,
a directional vector, or any other sort of value with 3 dimensions to it!

StereoKit uses a right-handed coordinate system, where +x is to the right, +y is
upwards, and -z is forward.


## Fields and Properties

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Forward]({{site.url}}/Pages/Reference/Vec3/Forward.html)|StereoKit uses a right-handed coordinate system, which means that forward is looking down the -Z axis! This value is the same as `new Vec3(0,0,-1)`|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [One]({{site.url}}/Pages/Reference/Vec3/One.html)|Shorthand for a vector where all values are 1! Same as `new Vec3(1,1,1)`.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Right]({{site.url}}/Pages/Reference/Vec3/Right.html)|When looking forward, this is the direction to the right! In StereoKit, this is the same as `new Vec3(1,0,0)`|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Up]({{site.url}}/Pages/Reference/Vec3/Up.html)|A vector representing the up axis. In StereoKit, this is the same as `new Vec3(0,1,0)`.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Zero]({{site.url}}/Pages/Reference/Vec3/Zero.html)|Shorthand for a vector where all values are 0! Same as `new Vec3(0,0,0)`.|



## Methods

|  |  |
|--|--|
|[AngleXY]({{site.url}}/Pages/Reference/Vec3/AngleXY.html)|Creates a vector that points out at the given 2D angle! This creates the vector on the XY plane, and allows you to specify a constant z value.|
|[AngleXZ]({{site.url}}/Pages/Reference/Vec3/AngleXZ.html)|Creates a vector that points out at the given 2D angle! This creates the vector on the XZ plane, and allows you to specify a constant y value.|
|[Cross]({{site.url}}/Pages/Reference/Vec3/Cross.html)|The cross product of two vectors!|
|[Normalized]({{site.url}}/Pages/Reference/Vec3/Normalized.html)|Creates a normalized vector (vector with a length of 1) from the current vector. Will not work properly if the vector has a length of zero.|
|[PerpendicularRight]({{site.url}}/Pages/Reference/Vec3/PerpendicularRight.html)|Exactly the same as Vec3.Cross, but has some naming memnonics for getting the order right when trying to find a perpendicular vector using the cross product. This'll also make it mor obvious to read if that's what you're actually going for when crossing vectors!  If you consider a forward vector and an up vector, then the direction to the right is pretty trivial to imagine in relation to those vectors!|


