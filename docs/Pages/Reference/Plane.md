---
layout: default
title: Plane
description: Planes are really useful for collisions, intersections, and visibility testing!  This plane is stored using the ax + by + cz + d = 0 formula, where the normal is a,b,c, and the d is, well, d.
---
# Plane

Planes are really useful for collisions, intersections, and
visibility testing!

This plane is stored using the ax + by + cz + d = 0 formula, where the
normal is a,b,c, and the d is, well, d.


## Fields and Properties

|  |  |
|--|--|
|float [d]({{site.url}}/Pages/Reference/Plane/d.html)|The distance/travel along the plane's normal from the origin to the surface of the plane.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [normal]({{site.url}}/Pages/Reference/Plane/normal.html)|The direction the plane is facing.|



## Methods

|  |  |
|--|--|
|[Plane]({{site.url}}/Pages/Reference/Plane/Plane.html)|Creates a Plane directly from the ax + by + cz + d = 0 formula!|
|[Closest]({{site.url}}/Pages/Reference/Plane/Closest.html)|Finds the closest point on this plane to the given point!|
|[Intersect]({{site.url}}/Pages/Reference/Plane/Intersect.html)|Checks the intersection of a ray with this plane!|


