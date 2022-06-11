---
layout: default
title: HandJoint
description: Contains information to represents a joint on the hand.
---
# struct HandJoint

Contains information to represents a joint on the hand.

## Instance Fields and Properties

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) [orientation]({{site.url}}/Pages/Reference/HandJoint/orientation.html)|The joint's world space orientation, where Forward points to the next joint down the finger, and Up will point towards the back of the hand. On the left hand, Right will point towards the thumb, and on the right hand, Right will point away from the thumb.|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) [Pose]({{site.url}}/Pages/Reference/HandJoint/Pose.html)|Pose position is the center of the joint's world space location. Pose orientation is the world space orientation, where Forward points to the next joint down the finger. On the left hand, Right will point towards the thumb, and on the right hand, Right will point away from the thumb.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [position]({{site.url}}/Pages/Reference/HandJoint/position.html)|The center of the joint's world space location.|
|float [radius]({{site.url}}/Pages/Reference/HandJoint/radius.html)|The distance, in meters, to the surface of the hand from this joint.|

## Instance Methods

|  |  |
|--|--|
|[HandJoint]({{site.url}}/Pages/Reference/HandJoint/HandJoint.html)|You can make a hand joint of your own here, but most likely you'd rather fetch one from `Input.Hand().Get()`!|
