---
layout: default
title: UI.LayoutLast
description: These are the layout bounds of the most recently reserved layout space. The Z axis dimensions are always 0. Only UI elements that affect the surface's layout will report their bounds here. You can reserve your own layout space via UI.LayoutReserve, and that call will also report here.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).LayoutLast

<div class='signature' markdown='1'>
static [Bounds]({{site.url}}/Pages/Reference/Bounds.html) LayoutLast{ get }
</div>

## Description
These are the layout bounds of the most recently reserved
layout space. The Z axis dimensions are always 0. Only UI elements
that affect the surface's layout will report their bounds here. You
can reserve your own layout space via UI.LayoutReserve, and that
call will also report here.

