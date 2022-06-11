---
layout: default
title: V
description: This is a collection of vector initialization shorthands. Since math can often get a little long to write, saving a few characters here and there can make a difference in readability. This also comes with some swizzles to make things even shorter! I also don't love the 'new' keyword on Vectors, and this eliminates that.  For example. instead of new Vec3(2.0f, 2.0f, 2.0f) or even Vec3.One * 2.0f, you could write V.XXX(2.0f)
---
# static class V

This is a collection of vector initialization shorthands.
Since math can often get a little long to write, saving a few
characters here and there can make a difference in readability. This
also comes with some swizzles to make things even shorter! I also
don't love the 'new' keyword on Vectors, and this eliminates that.

For example: instead of `new Vec3(2.0f, 2.0f, 2.0f)` or even
`Vec3.One * 2.0f`, you could write `V.XXX(2.0f)`

## Static Methods

|  |  |
|--|--|
|[X0Z]({{site.url}}/Pages/Reference/V/X0Z.html)|Creates a Vec3, this is a straight alternative to `new Vec3(x, 0, z)`|
|[XX]({{site.url}}/Pages/Reference/V/XX.html)|Creates a Vec2 where both components are the same value. This is the same as `new Vec2(x, x)`|
|[XXX]({{site.url}}/Pages/Reference/V/XXX.html)|Creates a Vec3 where all components are the same value. This is the same as `new Vec3(x, x, x)`|
|[XXXX]({{site.url}}/Pages/Reference/V/XXXX.html)|Creates a Vec4 where all components are the same value. This is the same as `new Vec4(x, x, x, x)`|
|[XY]({{site.url}}/Pages/Reference/V/XY.html)|Creates a Vec2, this is a straight alternative to `new Vec2(x, y)`|
|[XY0]({{site.url}}/Pages/Reference/V/XY0.html)|Creates a Vec3, this is a straight alternative to `new Vec3(x, y, 0)`|
|[XYZ]({{site.url}}/Pages/Reference/V/XYZ.html)|Creates a Vec3, this is a straight alternative to `new Vec3(x, y, z)`|
|[XYZW]({{site.url}}/Pages/Reference/V/XYZW.html)|Creates a Vec4, this is a straight alternative to `new Vec4(x, y, z, w)`|
