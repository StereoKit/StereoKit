---
layout: default
title: Gradient
description: A Gradient is a sparse collection of color keys that are used to represent a ramp of colors! This class is largely just storing colors and allowing you to sample between them.  Since the Gradient is just interpolating values, you can use whatever color space you want here, as long as it's linear and not gamma! Gamma space RGB can't math properly at all. It can be RGB(linear), HSV, LAB, just remember which one you have, and be sure to convert it appropriately later. Data is stored as float colors, so this'll be a high accuracy blend!
---
# class Gradient

A Gradient is a sparse collection of color keys that are
used to represent a ramp of colors! This class is largely just
storing colors and allowing you to sample between them.

Since the Gradient is just interpolating values, you can use whatever
color space you want here, as long as it's linear and not gamma!
Gamma space RGB can't math properly at all. It can be RGB(linear),
HSV, LAB, just remember which one you have, and be sure to convert it
appropriately later. Data is stored as float colors, so this'll be a
high accuracy blend!

## Instance Fields and Properties

|  |  |
|--|--|
|int [Count]({{site.url}}/Pages/Reference/Gradient/Count.html)|The number of color keys present in this gradient.|

## Instance Methods

|  |  |
|--|--|
|[Gradient]({{site.url}}/Pages/Reference/Gradient/Gradient.html)|Creates a new, completely empty gradient.|
|[Add]({{site.url}}/Pages/Reference/Gradient/Add.html)|This adds a color key into the list. It'll get inserted to the right slot based on its position.|
|[Get]({{site.url}}/Pages/Reference/Gradient/Get.html)|Samples the gradient's color at the given position!|
|[Get32]({{site.url}}/Pages/Reference/Gradient/Get32.html)|Samples the gradient's color at the given position, and converts it to a 32 bit color. If your RGBA color values are outside of the 0-1 range, then you'll get some issues as they're converted to 0-255 range bytes!|
|[Remove]({{site.url}}/Pages/Reference/Gradient/Remove.html)|Removes the color key at the given index!|
|[Set]({{site.url}}/Pages/Reference/Gradient/Set.html)|Updates the color key at the given index! This will NOT re-order color keys if they are moved past another key's position, which could lead to strange behavior.|
