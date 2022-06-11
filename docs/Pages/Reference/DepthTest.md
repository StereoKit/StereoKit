---
layout: default
title: DepthTest
description: Depth test describes how this material looks at and responds to depth information in the zbuffer! The default is Less, which means if the material pixel's depth is Less than the existing depth data, (basically, is this in front of some other object) it will draw that pixel. Similarly, Greater would only draw the material if it's 'behind' the depth buffer. Always would just draw all the time, and not read from the depth buffer at all.
---
# enum DepthTest

Depth test describes how this material looks at and responds
to depth information in the zbuffer! The default is Less, which means
if the material pixel's depth is Less than the existing depth data,
(basically, is this in front of some other object) it will draw that
pixel. Similarly, Greater would only draw the material if it's
'behind' the depth buffer. Always would just draw all the time, and
not read from the depth buffer at all.

## Enum Values

|  |  |
|--|--|
|Always|Don't look at the zbuffer at all, just draw everything, always, all the time! At this point, the order at which the mesh gets drawn will be  super important, so don't forget about `Material.QueueOffset`!|
|Equal|Only draw pixels if they're at exactly the same depth as the zbuffer!|
|Greater|Pixels in front of the zbuffer will be discarded! This is opposite of how things normally work. Great for drawing indicators that something is occluded by a wall or other geometry.|
|GreaterOrEq|Pixels in front of (or exactly at) the zbuffer will be discarded! This is opposite of how things normally work. Great for drawing indicators that something is occluded by a wall or other geometry.|
|Less|Default behavior, pixels behind the depth buffer will be discarded, and pixels in front of it will be drawn.|
|LessOrEq|Pixels behind the depth buffer will be discarded, and pixels in front of, or at the depth buffer's value it will be drawn. This could be great for things that might be sitting exactly on a floor or wall.|
|Never|Never draw a pixel, regardless of what's in the zbuffer. I can think of better ways to do this, but uhh, this is here for completeness! Maybe you can find a use for it.|
|NotEqual|Draw any pixel that's not exactly at the value in the zbuffer.|
