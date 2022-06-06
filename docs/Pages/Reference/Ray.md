---
layout: default
title: Ray
description: A position and a direction indicating a ray through space! This is a great tool for intersection testing with geometrical shapes.
---
# struct Ray

A position and a direction indicating a ray through space!
This is a great tool for intersection testing with geometrical
shapes.

## Instance Fields and Properties

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [direction]({{site.url}}/Pages/Reference/Ray/direction.html)|The direction the ray is facing, typically does not require being a unit vector, or normalized direction.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [position]({{site.url}}/Pages/Reference/Ray/position.html)|The position or origin point of the Ray.|

## Instance Methods

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray/Ray.html)|Basic initialization constructor! Just copies the parameters into the fields.|
|[At]({{site.url}}/Pages/Reference/Ray/At.html)|Gets a point along the ray! This is basically just position + direction*percent. If Ray.direction is normalized, then percent is functionally distance, and can be used to find the point a certain distance out along the ray.|
|[Closest]({{site.url}}/Pages/Reference/Ray/Closest.html)|Calculates the point on the Ray that's closest to the given point! This can be in front of, or behind the ray's starting position.|
|[Intersect]({{site.url}}/Pages/Reference/Ray/Intersect.html)|Checks the intersection of this ray with a plane!|
|[ToString]({{site.url}}/Pages/Reference/Ray/ToString.html)|Mostly for debug purposes, this is a decent way to log or inspect the Ray in debug mode. Looks like "[position], [direction]"|

## Static Methods

|  |  |
|--|--|
|[FromTo]({{site.url}}/Pages/Reference/Ray/FromTo.html)|A convenience function that creates a ray from point a, towards point b. Resulting direction is not normalized.|
