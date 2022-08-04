---
layout: default
title: TextAlign
description: A bit-flag enum for describing alignment or positioning. Items can be combined using the '|' operator, like so.
---
# enum TextAlign

A bit-flag enum for describing alignment or positioning.
Items can be combined using the '|' operator, like so:

## Enum Values

|  |  |
|--|--|
|BottomCenter|Center on the X axis, and bottom on the Y axis. This is a combination of XCenter and YBottom.|
|BottomLeft|Start on the left of the X axis, and bottom on the Y axis. This is a combination of XLeft and YBottom.|
|BottomRight|Start on the right of the X axis, and bottom on the Y axis.This is a combination of XRight and YBottom.|
|Center|Center on both X and Y axes. This is a combination of XCenter and YCenter.|
|CenterLeft|Start on the left of the X axis, center on the Y axis. This is a combination of XLeft and YCenter.|
|CenterRight|Start on the right of the X axis, center on the Y axis. This is a combination of XRight and YCenter.|
|TopCenter|Center on the X axis, and top on the Y axis. This is a combination of XCenter and YTop.|
|TopLeft|Start on the left of the X axis, and top on the Y axis. This is a combination of XLeft and YTop.|
|TopRight|Start on the right of the X axis, and top on the Y axis. This is a combination of XRight and YTop.|
|XCenter|On the x axis, the item should be centered.|
|XLeft|On the x axis, this item should start on the left.|
|XRight|On the x axis, this item should start on the right.|
|YBottom|On the y axis, this item should start on the bottom.|
|YCenter|On the y axis, the item should be centered.|
|YTop|On the y axis, this item should start at the top.|
