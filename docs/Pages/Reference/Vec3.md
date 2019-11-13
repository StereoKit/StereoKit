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



