---
layout: default
title: Quat
description: Quaternions are efficient and robust mathematical objects for representing rotations! Understanding the details of how a quaterion works is not generally necessary for using them effectively, so don't worry too much if they seem weird to you. They're weird to me too.  If you're interested in learning the details though, 3Blue1Brown and Ben Eater have an [excellent interactive lesson](https.//eater.net/quaternions) about them!
---
# Quat

## Description
Quaternions are efficient and robust mathematical objects for
representing rotations! Understanding the details of how a quaterion works
is not generally necessary for using them effectively, so don't worry too
much if they seem weird to you. They're weird to me too.

If you're interested in learning the details though, 3Blue1Brown and Ben Eater
have an [excellent interactive lesson](https://eater.net/quaternions) about them!


## Fields

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) [Identity]({{site.url}}/Pages/Reference/Quat/Identity.html)|This is the 'multiply by one!' of the quaternion rotation world. It's basically a default, no rotation quaternion.|



## Methods

|  |  |
|--|--|
|[Difference]({{site.url}}/Pages/Reference/Quat/Difference.html)|This gives a relative rotation between the first and second quaternion rotations. Remember that order is important here!|
|[FromAngles]({{site.url}}/Pages/Reference/Quat/FromAngles.html)|Creates a Roll/Pitch/Yaw rotation from the provided angles in degrees!|
|[LookAt]({{site.url}}/Pages/Reference/Quat/LookAt.html)|Creates a rotation from a resting direction, to a direction indicated by the direction of the two vectors provided to the function! This is a great function for camera style rotation, when you know where a camera is, and where you want to look at. This prevents roll on the Z axis, Up is always (0,1,0).|
|[LookDir]({{site.url}}/Pages/Reference/Quat/LookDir.html)|Creates a rotation from a resting direction, to the given direction! This prevents roll on the Z axis, Up is always (0,1,0)|


