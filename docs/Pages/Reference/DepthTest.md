---
layout: default
title: DepthTest
description: Depth test describes how this material looks at and responds to depth information in the zbuffer! The default is Less, which means if the material pixel's depth is Less than the existing depth data, (basically, is this in front of some other object) it will draw that pixel. Similarly, Greater would only draw the material if it's 'behind' the depth buffer. Always would just draw all the time, and not read from the depth buffer at all.
---
# DepthTest

Depth test describes how this material looks at and responds
to depth information in the zbuffer! The default is Less, which means
if the material pixel's depth is Less than the existing depth data,
(basically, is this in front of some other object) it will draw that
pixel. Similarly, Greater would only draw the material if it's
'behind' the depth buffer. Always would just draw all the time, and
not read from the depth buffer at all.




## Static Fields and Properties

|  |  |
|--|--|
|[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) [Always]({{site.url}}/Pages/Reference/DepthTest/Always.html)|Don't look at the zbuffer at all, just draw everything, always, all the time! At this poit, the order at which the mesh gets drawn will be  super important, so don't forget about `Material.QueueOffset`!|
|[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) [Equal]({{site.url}}/Pages/Reference/DepthTest/Equal.html)|Only draw pixels if they're at exactly the same depth as the zbuffer!|
|[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) [Greater]({{site.url}}/Pages/Reference/DepthTest/Greater.html)|Pixels in front of the zbuffer will be discarded! This is opposite of how things normally work. Great for drawing indicators that something is occluded by a wall or other geometry.|
|[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) [GreaterOrEqual]({{site.url}}/Pages/Reference/DepthTest/GreaterOrEqual.html)|Pixels in front of (or exactly at) the zbuffer will be discarded! This is opposite of how things normally work. Great for drawing indicators that something is occluded by a wall or other geometry.|
|[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) [Less]({{site.url}}/Pages/Reference/DepthTest/Less.html)|Default behavior, pixels behind the depth buffer will be discarded, and pixels in front of it will be drawn.|
|[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) [LessOrEqual]({{site.url}}/Pages/Reference/DepthTest/LessOrEqual.html)|Pixels behind the depth buffer will be discarded, and pixels in front of, or at the depth buffer's value it will be drawn. This could be great for things that might be sitting exactly on a floor or wall.|
|[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) [Never]({{site.url}}/Pages/Reference/DepthTest/Never.html)|Never draw a pixel, regardless of what's in the zbuffer. I can think of better ways to do this, but uhh, this is here for completeness! Maybe you can find a use for it.|
|[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) [NotEqual]({{site.url}}/Pages/Reference/DepthTest/NotEqual.html)|Draw any pixel that's not exactly at the value in the zbuffer.|


