---
layout: default
title: Vec2
description: A vector with 2 components. x and y. This can represent a point in 2D space, a directional vector, or any other sort of value with 2 dimensions to it!
---
# Vec2

A vector with 2 components: x and y. This can represent a point in 2D space,
a directional vector, or any other sort of value with 2 dimensions to it!


## Fields and Properties

|  |  |
|--|--|
|float [Magnitude]({{site.url}}/Pages/Reference/Vec2/Magnitude.html)|Magnitude is the length of the vector! Or the distance from the origin to this point. Uses Math.Sqrt, so it's not dirt cheap or anything.|
|float [MagnitudeSq]({{site.url}}/Pages/Reference/Vec2/MagnitudeSq.html)|This is the squared magnitude of the vector! It skips the Sqrt call, and just gives you the squared version for speedy calculations that can work with it squared.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [One]({{site.url}}/Pages/Reference/Vec2/One.html)|A Vec2 with all components at one, same as new Vec2(1,1).|
|float [x]({{site.url}}/Pages/Reference/Vec2/x.html)|Vector components.|
|float [y]({{site.url}}/Pages/Reference/Vec2/y.html)|Vector components.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [Zero]({{site.url}}/Pages/Reference/Vec2/Zero.html)|A Vec2 with all components at zero, same as new Vec2(0,0).|



## Methods

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2/Vec2.html)|A basic constructor, just copies the values in!|
|[Normalize]({{site.url}}/Pages/Reference/Vec2/Normalize.html)|Turns this vector into a normalized vector (vector with a length of 1) from the current vector. Will not work properly if the vector has a length of zero.|
|[Normalized]({{site.url}}/Pages/Reference/Vec2/Normalized.html)|Creates a normalized vector (vector with a length of 1) from the current vector. Will not work properly if the vector has a length of zero.|


