---
layout: default
title: Sphere
description: Represents a sphere in 3D space! Composed of a center point and a radius, can be used for raycasting, collision, visibility, and other things!
---
# struct Sphere

Represents a sphere in 3D space! Composed of a center point
and a radius, can be used for raycasting, collision, visibility, and
other things!

## Instance Fields and Properties

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [center]({{site.url}}/Pages/Reference/Sphere/center.html)|Center of the sphere.|
|float [Diameter]({{site.url}}/Pages/Reference/Sphere/Diameter.html)|Length of the line passing through the center from one side of the sphere to the other, in meters. Twice the radius.|
|float [radius]({{site.url}}/Pages/Reference/Sphere/radius.html)|Distance from the center, to the surface of the sphere, in meters. Half the diameter.|

## Instance Methods

|  |  |
|--|--|
|[Sphere]({{site.url}}/Pages/Reference/Sphere/Sphere.html)|Creates a sphere using a center point and a diameter!|
|[Contains]({{site.url}}/Pages/Reference/Sphere/Contains.html)|A fast check to see if the given point is contained in or on a sphere!|
|[Intersect]({{site.url}}/Pages/Reference/Sphere/Intersect.html)|Intersects a ray with this sphere, and finds if they intersect, and if so, where that intersection is! This only finds the closest intersection point to the origin of the ray.|
