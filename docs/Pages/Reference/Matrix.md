---
layout: default
title: Matrix
description: A Matrix in StereoKit is a 4x4 grid of numbers that is used to represent a transformation for any sort of position or vector! This is an oversimplification of what a matrix actually is, but it's accurate in this case.  Matrices are really useful for transforms because you can chain together all sorts of transforms into a single Matrix! A Matrix transform really shines when applied to many positions, as the more expensive operations get cached within the matrix values.  Matrices are prominently used within shaders for mesh transforms!
---
# Matrix

A Matrix in StereoKit is a 4x4 grid of numbers that is used
to represent a transformation for any sort of position or vector!
This is an oversimplification of what a matrix actually is, but it's
accurate in this case.

Matrices are really useful for transforms because you can chain
together all sorts of transforms into a single Matrix! A Matrix
transform really shines when applied to many positions, as the more
expensive operations get cached within the matrix values.

Matrices are prominently used within shaders for mesh transforms!



## Instance Methods

|  |  |
|--|--|
|[Invert]({{site.url}}/Pages/Reference/Matrix/Invert.html)|Inverts this Matrix! If the matrix takes a point from a -> b, then its inverse takes the point from b -> a.|
|[Transform]({{site.url}}/Pages/Reference/Matrix/Transform.html)|Transforms a point through the Matrix! This is basically just multiplying a vector (x,y,z,1) with the Matrix.|
|[TransformNormal]({{site.url}}/Pages/Reference/Matrix/TransformNormal.html)|Transforms a point through the Matrix, but excluding translation! This is great for transforming vectors that are -directions- rather than points in space. Use this to transform normals and directions. The same as multiplying (x,y,z,0) with the Matrix.|


## Static Fields and Properties

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) [Identity]({{site.url}}/Pages/Reference/Matrix/Identity.html)|An identity Matrix is the matrix equivalent of '1'! Transforming anything by this will leave it at the exact same place.|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) [Inverse]({{site.url}}/Pages/Reference/Matrix/Inverse.html)|Creates an inverse matrix! If the matrix takes a point from a -> b, then its inverse takes the point from b -> a.|


## Static Methods

|  |  |
|--|--|
|[R]({{site.url}}/Pages/Reference/Matrix/R.html)|Create a rotation matrix from a Quaternion.|
|[S]({{site.url}}/Pages/Reference/Matrix/S.html)|Creates a scaling Matrix, where scale can be different on each axis (non-uniform).|
|[T]({{site.url}}/Pages/Reference/Matrix/T.html)|Translate. Creates a translation Matrix!|
|[TR]({{site.url}}/Pages/Reference/Matrix/TR.html)|Translate, Rotate. Creates a transform Matrix using these components!|
|[TRS]({{site.url}}/Pages/Reference/Matrix/TRS.html)|Translate, Rotate, Scale. Creates a transform Matrix using all these components!|
|[TS]({{site.url}}/Pages/Reference/Matrix/TS.html)|Translate, Scale. Creates a transform Matrix using both these components!|

