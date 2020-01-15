---
layout: default
title: Bounds
description: Bounds is an axis aligned bounding box type that can be used for storing the sizes of objects, calculating containment, intersections, and more!  While the constructor uses a center+dimensions for creating a bounds, don't forget the static From* methods that allow you to define a Bounds from different types of data!
---
# Bounds

Bounds is an axis aligned bounding box type that can be used for
storing the sizes of objects, calculating containment, intersections, and
more!

While the constructor uses a center+dimensions for creating a bounds, don't
forget the static From* methods that allow you to define a Bounds from different
types of data!


## Instance Fields and Properties

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [center]({{site.url}}/Pages/Reference/Bounds/center.html)|The exact center of the Bounds!|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [dimensions]({{site.url}}/Pages/Reference/Bounds/dimensions.html)|The total size of the box, from one end to the other. This is the width, height, and depth of the Bounds.|


## Instance Methods

|  |  |
|--|--|
|[Bounds]({{site.url}}/Pages/Reference/Bounds/Bounds.html)|Creates a bounding box object!|
|[Contains]({{site.url}}/Pages/Reference/Bounds/Contains.html)|Does the Bounds contain the given point? This includes points that are on the surface of the Bounds.|
|[Intersect]({{site.url}}/Pages/Reference/Bounds/Intersect.html)|Calculate the intersection between a Ray, and these bounds. Returns false if no intersection occurred, and 'at' will contain the nearest intersection point to the start of the ray if an intersection is found!|



## Static Methods

|  |  |
|--|--|
|[FromCorner]({{site.url}}/Pages/Reference/Bounds/FromCorner.html)|Create a bounding box from a corner, plus box dimensions.|
|[FromCorners]({{site.url}}/Pages/Reference/Bounds/FromCorners.html)|Create a bounding box between two corner points.|

