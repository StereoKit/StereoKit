---
layout: default
title: Tex.AddressMode
description: When looking at a UV texture coordinate on this texture, how do we handle values larger than 1, or less than zero? Do we Wrap to the other side? Clamp it between 0-1, or just keep Mirroring back and forth? Wrap is the default.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).AddressMode

<div class='signature' markdown='1'>
[TexAddress]({{site.url}}/Pages/Reference/TexAddress.html) AddressMode{ get set }
</div>

## Description
When looking at a UV texture coordinate on this texture,
how do we handle values larger than 1, or less than zero? Do we
Wrap to the other side? Clamp it between 0-1, or just keep
Mirroring back and forth? Wrap is the default.

