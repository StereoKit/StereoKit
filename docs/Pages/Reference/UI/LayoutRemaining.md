---
layout: default
title: UI.LayoutRemaining
description: How much space is available on the current layout! This is based on the current layout position, so X will give you the amount remaining on the current line, and Y will give you distance to the bottom of the layout, including the current line. These values will be 0 if you're using 0 for the layout size on that axis.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).LayoutRemaining

<div class='signature' markdown='1'>
static [Vec2]({{site.url}}/Pages/Reference/Vec2.html) LayoutRemaining{ get }
</div>

## Description
How much space is available on the current layout! This is
based on the current layout position, so X will give you the amount
remaining on the current line, and Y will give you distance to the
bottom of the layout, including the current line. These values will
be 0 if you're using 0 for the layout size on that axis.

