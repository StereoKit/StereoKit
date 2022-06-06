---
layout: default
title: Matrix
description: A Matrix in StereoKit is a 4x4 grid of numbers that is used to represent a transformation for any sort of position or vector! This is an oversimplification of what a matrix actually is, but it's accurate in this case.  Matrices are really useful for transforms because you can chain together all sorts of transforms into a single Matrix! A Matrix transform really shines when applied to many positions, as the more expensive operations get cached within the matrix values.  Multiple matrix transforms can be combined by multiplying them. In StereoKit, to create a matrix that first scales an object, followed by rotating it, and finally translating it you would use this order.  Matrix M = Matrix.S(...) * Matrix.R(...) * Matrix.T(...);  This order is related to the fact that StereoKit uses row-major order to store matrices. Note that in other 3D frameworks and certain 3D math references you may find column-major matrices, which would need the reverse order (i.e. T*R*S), so please keep this in mind when creating transformations.  Matrices are prominently used within shaders for mesh transforms!
---
# struct Matrix

A Matrix in StereoKit is a 4x4 grid of numbers that is used
to represent a transformation for any sort of position or vector!
This is an oversimplification of what a matrix actually is, but it's
accurate in this case.

Matrices are really useful for transforms because you can chain
together all sorts of transforms into a single Matrix! A Matrix
transform really shines when applied to many positions, as the more
expensive operations get cached within the matrix values.

Multiple matrix transforms can be combined by multiplying them. In
StereoKit, to create a matrix that first scales an object, followed by
rotating it, and finally translating it you would use this order:

`Matrix M = Matrix.S(...) * Matrix.R(...) * Matrix.T(...);`

This order is related to the fact that StereoKit uses row-major order
to store matrices. Note that in other 3D frameworks and certain 3D math
references you may find column-major matrices, which would need the
reverse order (i.e. T*R*S), so please keep this in mind when creating
transformations.

Matrices are prominently used within shaders for mesh transforms!

## Instance Fields and Properties

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) [Inverse]({{site.url}}/Pages/Reference/Matrix/Inverse.html)|Creates an inverse matrix! If the matrix takes a point from a -> b, then its inverse takes the point from b -> a.|
|Matrix4x4 [m]({{site.url}}/Pages/Reference/Matrix/m.html)|The internal, wrapped System.Numerics type. This can be nice to have around so you can pass its fields as 'ref', which you can't do with properties. You won't often need this, as implicit conversions to System.Numerics types are also provided.|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) [Pose]({{site.url}}/Pages/Reference/Matrix/Pose.html)|Extracts translation and rotation information from the transform matrix, and makes a Pose from it! Not exactly fast. This is backed by Decompose, so if you need any additional info, it's better to just call Decompose instead.|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) [Rotation]({{site.url}}/Pages/Reference/Matrix/Rotation.html)|A slow function that returns the rotation quaternion embedded in this transform matrix. This is backed by Decompose, so if you need any additional info, it's better to just call Decompose instead.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Scale]({{site.url}}/Pages/Reference/Matrix/Scale.html)|Returns the scale embedded in this transform matrix. Not exactly cheap, requires 3 sqrt calls, but is cheaper than calling Decompose.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [Translation]({{site.url}}/Pages/Reference/Matrix/Translation.html)|A fast Property that will return or set the translation component embedded in this transform matrix.|

## Instance Methods

|  |  |
|--|--|
|[Decompose]({{site.url}}/Pages/Reference/Matrix/Decompose.html)|Returns this transformation matrix to its original translation, rotation and scale components. Not exactly a cheap function. If this is not a transform matrix, there's a chance this call will fail, and return false.|
|[Invert]({{site.url}}/Pages/Reference/Matrix/Invert.html)|Inverts this Matrix! If the matrix takes a point from a -> b, then its inverse takes the point from b -> a.|
|[Transform]({{site.url}}/Pages/Reference/Matrix/Transform.html)|Transforms a point through the Matrix! This is basically just multiplying a vector (x,y,z,1) with the Matrix.|
|[TransformNormal]({{site.url}}/Pages/Reference/Matrix/TransformNormal.html)|Transforms a point through the Matrix, but excluding translation! This is great for transforming vectors that are -directions- rather than points in space. Use this to transform normals and directions. The same as multiplying (x,y,z,0) with the Matrix.|

## Static Fields and Properties

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) [Identity]({{site.url}}/Pages/Reference/Matrix/Identity.html)|An identity Matrix is the matrix equivalent of '1'! Transforming anything by this will leave it at the exact same place.|

## Static Methods

|  |  |
|--|--|
|[Orthographic]({{site.url}}/Pages/Reference/Matrix/Orthographic.html)|This creates a matrix used for projecting 3D geometry onto a 2D surface for rasterization. Orthographic projection matrices will preserve parallel lines. This is great for 2D scenes or content.|
|[Perspective]({{site.url}}/Pages/Reference/Matrix/Perspective.html)|This creates a matrix used for projecting 3D geometry onto a 2D surface for rasterization. Perspective projection matrices will cause parallel lines to converge at the horizon. This is great for normal looking content.|
|[R]({{site.url}}/Pages/Reference/Matrix/R.html)|Create a rotation matrix from a Quaternion.|
|[S]({{site.url}}/Pages/Reference/Matrix/S.html)|Creates a scaling Matrix, where scale can be different on each axis (non-uniform).|
|[T]({{site.url}}/Pages/Reference/Matrix/T.html)|Translate. Creates a translation Matrix!|
|[TR]({{site.url}}/Pages/Reference/Matrix/TR.html)|Translate, Rotate. Creates a transform Matrix using these components!|
|[TRS]({{site.url}}/Pages/Reference/Matrix/TRS.html)|Translate, Rotate, Scale. Creates a transform Matrix using all these components!|
|[TS]({{site.url}}/Pages/Reference/Matrix/TS.html)|Translate, Scale. Creates a transform Matrix using both these components!|

## Operators

|  |  |
|--|--|
|[Implicit Conversions]({{site.url}}/Pages/Reference/Matrix/op_Implicit.html)|Allows implicit conversion from System.Numerics.Matrix4x4 to StereoKit.Matrix.|
|[*]({{site.url}}/Pages/Reference/Matrix/op_Multiply.html)|Multiplies two matrices together! This is a great way to combine transform operations. Note that StereoKit's matrices are row-major, and multiplication order is important! To translate, then scale, multiple in order of 'translate * scale'.|
