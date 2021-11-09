---
layout: default
title: HandJoint.HandJoint
description: You can make a hand joint of your own here, but most likely you'd rather fetch one from Input.Hand().Get()!
---
# [HandJoint]({{site.url}}/Pages/Reference/HandJoint.html).HandJoint

<div class='signature' markdown='1'>
```csharp
void HandJoint(Vec3 position, Quat orientation, float radius)
```
You can make a hand joint of your own here, but most
likely you'd rather fetch one from `Input.Hand().Get()`!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) position|The center of the joint's world space              location.|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) orientation|The joint's world space orientation,             where Forward points to the next joint down the finger, and Up             will point towards the back of the hand. On the left hand, Right             will point towards the thumb, and on the right hand, Right will             point away from the thumb.|
|float radius|The distance, in meters, to the surface of             the hand from this joint.|




