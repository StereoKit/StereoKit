---
layout: default
title: SKMath
description: This class contains some convenience math functions! StereoKit typically uses floats instead of doubles, so you won't need to cast to and from using these methods.
---
# static class SKMath

This class contains some convenience math functions!
StereoKit typically uses floats instead of doubles, so you won't need
to cast to and from using these methods.

## Static Fields and Properties

|  |  |
|--|--|
|float [Pi]({{site.url}}/Pages/Reference/SKMath/Pi.html)|The mathematical constant, Pi!|
|float [Tau]({{site.url}}/Pages/Reference/SKMath/Tau.html)|Tau is 2*Pi, there are excellent arguments that Tau can make certain formulas more readable!|

## Static Methods

|  |  |
|--|--|
|[AngleDist]({{site.url}}/Pages/Reference/SKMath/AngleDist.html)|Calculates the minimum angle 'distance' between two angles. This covers wraparound cases like: the minimum distance between 10 and 350 is 20.|
|[Cos]({{site.url}}/Pages/Reference/SKMath/Cos.html)|Same as Math.Cos|
|[Exp]({{site.url}}/Pages/Reference/SKMath/Exp.html)|Same as Math.Exp|
|[Lerp]({{site.url}}/Pages/Reference/SKMath/Lerp.html)|Blends (Linear Interpolation) between two scalars, based on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp percent for you.|
|[Mod]({{site.url}}/Pages/Reference/SKMath/Mod.html)|Modulus, works better than '%' for negative values.|
|[Sin]({{site.url}}/Pages/Reference/SKMath/Sin.html)|Same as Math.Sin|
|[Sqrt]({{site.url}}/Pages/Reference/SKMath/Sqrt.html)|Same as Math.Sqrt|
