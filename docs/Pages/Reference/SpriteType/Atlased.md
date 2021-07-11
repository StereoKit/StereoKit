---
layout: default
title: SpriteType.Atlased
description: The sprite will be batched onto an atlas texture so all sprites can be drawn in a single pass. This is excellent for performance! The only thing to watch out for here, adding a sprite to an atlas will rebuild the atlas texture! This can be a bit expensive, so it's recommended to add all sprites to an atlas at start, rather than during runtime. Also, if an image is too large, it may take up too much space on the atlas, and may be better as a Single sprite type.
---
# [SpriteType]({{site.url}}/Pages/Reference/SpriteType.html).Atlased

<div class='signature' markdown='1'>
static [SpriteType]({{site.url}}/Pages/Reference/SpriteType.html) Atlased
</div>

## Description
The sprite will be batched onto an atlas texture so all sprites can be
drawn in a single pass. This is excellent for performance! The only thing to
watch out for here, adding a sprite to an atlas will rebuild the atlas texture!
This can be a bit expensive, so it's recommended to add all sprites to an atlas
at start, rather than during runtime. Also, if an image is too large, it may take
up too much space on the atlas, and may be better as a Single sprite type.

