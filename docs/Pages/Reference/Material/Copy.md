---
layout: default
title: Material.Copy
description: Creates a new Material asset with the same shader and properties! Draw calls with the new Material will not batch together with this one.
---
# [Material]({{site.url}}/Pages/Reference/Material.html).Copy

<div class='signature' markdown='1'>
[Material]({{site.url}}/Pages/Reference/Material.html) Copy()
</div>

|  |  |
|--|--|
|RETURNS: [Material]({{site.url}}/Pages/Reference/Material.html)|A new Material asset with the same shader and properties.|

Creates a new Material asset with the same shader and
properties! Draw calls with the new Material will not batch
together with this one.
<div class='signature' markdown='1'>
static [Material]({{site.url}}/Pages/Reference/Material.html) Copy(string materialId)
</div>

|  |  |
|--|--|
|string materialId|Which Material are you looking for?|
|RETURNS: [Material]({{site.url}}/Pages/Reference/Material.html)|A new Material asset with the same shader and properties. Returns null if no materials are found with the given id.|

Creates a new Material asset with the same shader and
properties! Draw calls with the new Material will not batch
together with the source Material.



