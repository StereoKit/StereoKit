---
layout: default
title: Material.DepthTest
description: How does this material interact with the ZBuffer? Generally DepthTest.Less would be normal behavior. don't draw objects that are occluded. But this can also be used to achieve some interesting effects, like you could use DepthTest.Greater to draw a glow that indicates an object is behind something.
---
# [Material]({{site.url}}/Pages/Reference/Material.html).DepthTest

<div class='signature' markdown='1'>
[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) DepthTest{ get set }
</div>

## Description
How does this material interact with the ZBuffer?
Generally DepthTest.Less would be normal behavior: don't draw
objects that are occluded. But this can also be used to achieve
some interesting effects, like you could use DepthTest.Greater
to draw a glow that indicates an object is behind something.

