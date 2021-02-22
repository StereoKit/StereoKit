---
layout: default
title: Quat.FromAngles
description: Creates a Roll/Pitch/Yaw rotation (applied in that order) from the provided angles in degrees!
---
# [Quat]({{site.url}}/Pages/Reference/Quat.html).FromAngles

<div class='signature' markdown='1'>
static [Quat]({{site.url}}/Pages/Reference/Quat.html) FromAngles(float pitchXDeg, float yawYDeg, float rollZDeg)
</div>

|  |  |
|--|--|
|float pitchXDeg|Pitch is rotation around the x axis,             measured in degrees.|
|float yawYDeg|Yaw is rotation around the y axis, measured             in degrees.|
|float rollZDeg|Roll is rotation around the z axis,              measured in degrees.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A quaternion representing the given Roll/Pitch/Yaw roatation!|

Creates a Roll/Pitch/Yaw rotation (applied in that
order) from the provided angles in degrees!
<div class='signature' markdown='1'>
static [Quat]({{site.url}}/Pages/Reference/Quat.html) FromAngles([Vec3]({{site.url}}/Pages/Reference/Vec3.html) pitchYawRollDeg)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pitchYawRollDeg|Pitch, yaw, and roll stored as             X, Y, and Z in this Vector. Angle values are in degrees.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A quaternion representing the given Roll/Pitch/Yaw roatation!|

Creates a Roll/Pitch/Yaw rotation (applied in that
order) from the provided angles in degrees!



