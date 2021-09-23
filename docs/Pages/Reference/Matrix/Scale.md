---
layout: default
title: Matrix.Scale
description: Returns the scale embedded in this transform matrix. Not exactly cheap, requires 3 sqrt calls, but is cheaper than calling Decompose.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).Scale

<div class='signature' markdown='1'>
[Vec3]({{site.url}}/Pages/Reference/Vec3.html) Scale{ get }
</div>

## Description
Returns the scale embedded in this transform matrix. Not
exactly cheap, requires 3 sqrt calls, but is cheaper than calling
Decompose.

