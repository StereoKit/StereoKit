---
layout: default
title: Matrix
description: A Matrix in StereoKit is a 4x4 grid of numbers that is used to represent a transformation for any sort of position or vector! This is an oversimplification of what a matrix actually is, but it's accurate in this case.  Matrices are really useful for transforms because you can chain together all sorts of transforms into a single Matrix! A Matrix transform really shines when applied to many positions, as the more expensive operations get cached within the matrix values.  Matrices are prominently used within shaders for mesh transforms!
---
# Matrix

A Matrix in StereoKit is a 4x4 grid of numbers that is used to represent
a transformation for any sort of position or vector! This is an oversimplification
of what a matrix actually is, but it's accurate in this case.

Matrices are really useful for transforms because you can chain together all sorts
of transforms into a single Matrix! A Matrix transform really shines when applied to
many positions, as the more expensive operations get cached within the matrix values.

Matrices are prominently used within shaders for mesh transforms!


## Fields and Properties

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) [Identity]({{site.url}}/Pages/Reference/Matrix/Identity.html)|An identity Matrix is the matrix equivalent of '1'! Transforming anything by this will leave it at the exact same place.|



## Methods

|  |  |
|--|--|
|[Inverse]({{site.url}}/Pages/Reference/Matrix/Inverse.html)|Creates an inverse matrix! If the matrix takes a point from a -> b, then its inverse takes the point from b -> a.|
|[T]({{site.url}}/Pages/Reference/Matrix/T.html)|Translate. Creates a translation Matrix!|
|[TransformDirection]({{site.url}}/Pages/Reference/Matrix/TransformDirection.html)|Transforms a point through the Matrix, but excluding translation! This is great for transforming vectors that are -directions- rather than points in space. Use this to transform normals and directions. The same as multiplying (x,y,z,0) with the Matrix.|
|[TransformPoint]({{site.url}}/Pages/Reference/Matrix/TransformPoint.html)|Transforms a point through the Matrix! This is basically just multiplying a vector (x,y,z,1) with the Matrix.|
|[TRS]({{site.url}}/Pages/Reference/Matrix/TRS.html)|Translate, Rotate, Scale. Creates a transform Matrix using all these components!|
|[TS]({{site.url}}/Pages/Reference/Matrix/TS.html)|Translate, Scale. Creates a transform Matrix using both these components!|


