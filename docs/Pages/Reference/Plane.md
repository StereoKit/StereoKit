---
layout: default
title: Plane
description: Planes are really useful for collisions, intersections, and visibility testing!  This plane is stored using the ax + by + cz + d = 0 formula, where the normal is a,b,c, and the d is, well, d.
---
# struct Plane

Planes are really useful for collisions, intersections, and
visibility testing!

This plane is stored using the ax + by + cz + d = 0 formula, where
the normal is a,b,c, and the d is, well, d.

## Instance Fields and Properties

|  |  |
|--|--|
|float [d]({{site.url}}/Pages/Reference/Plane/d.html)|The distance/travel along the plane's normal from the origin to the surface of the plane.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [normal]({{site.url}}/Pages/Reference/Plane/normal.html)|The direction the plane is facing.|
|[Plane]({{site.url}}/Pages/Reference/Plane.html) [p]({{site.url}}/Pages/Reference/Plane/p.html)|The internal, wrapped System.Numerics type. This can be nice to have around so you can pass its fields as 'ref', which you can't do with properties. You won't often need this, as implicit conversions to System.Numerics types are also provided.|

## Instance Methods

|  |  |
|--|--|
|[Plane]({{site.url}}/Pages/Reference/Plane/Plane.html)|Creates a Plane directly from the ax + by + cz + d = 0 formula!|
|[Closest]({{site.url}}/Pages/Reference/Plane/Closest.html)|Finds the closest point on this plane to the given point!|
|[Intersect]({{site.url}}/Pages/Reference/Plane/Intersect.html)|Checks the intersection of a ray with this plane!|

## Static Methods

|  |  |
|--|--|
|[FromPoint]({{site.url}}/Pages/Reference/Plane/FromPoint.html)|Creates a plane from a normal, and any point on the plane!|
|[FromPoints]({{site.url}}/Pages/Reference/Plane/FromPoints.html)|Creates a plane from 3 points that are directly on that plane.|
