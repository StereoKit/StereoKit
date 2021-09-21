---
layout: default
title: Hand.pinchPt
description: This is an approximation of where the center of a 'pinch' gesture occurs, and is used internally by StereoKit for some tasks, such as UI. For simulated hands, this position will give you the most stable pinch location possible. For real hands, it'll be pretty close to the stablest point you'll get. This is especially important for when the user begins and ends their pinch action, as you'll often see a lot of extra movement in the fingers then.
---
# [Hand]({{site.url}}/Pages/Reference/Hand.html).pinchPt

<div class='signature' markdown='1'>
[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pinchPt
</div>

## Description
This is an approximation of where the center of a
'pinch' gesture occurs, and is used internally by StereoKit for
some tasks, such as UI. For simulated hands, this position will
give you the most stable pinch location possible. For real hands,
it'll be pretty close to the stablest point you'll get. This is
especially important for when the user begins and ends their
pinch action, as you'll often see a lot of extra movement in the
fingers then.

