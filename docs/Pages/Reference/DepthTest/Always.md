---
layout: default
title: DepthTest.Always
description: Don't look at the zbuffer at all, just draw everything, always, all the time! At this point, the order at which the mesh gets drawn will be  super important, so don't forget about Material.QueueOffset!
---
# [DepthTest]({{site.url}}/Pages/Reference/DepthTest.html).Always

<div class='signature' markdown='1'>
static [DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) Always
</div>

## Description
Don't look at the zbuffer at all, just draw everything,
always, all the time! At this point, the order at which the mesh
gets drawn will be  super important, so don't forget about
`Material.QueueOffset`!

