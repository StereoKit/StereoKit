---
layout: default
title: TextAlign
description: A bit-flag enum for describing alignment or positioning. Items can be combined using the '|' operator, like so.  TextAlign alignment = TextAlign.XLeft | TextAlign.YTop;  Avoid combining multiple items of the same axis, and note that a few items, like Center are already a combination of multiple flags.
---
# TextAlign

A bit-flag enum for describing alignment or positioning. Items can be
combined using the '|' operator, like so:

`TextAlign alignment = TextAlign.XLeft | TextAlign.YTop;`

Avoid combining multiple items of the same axis, and note that a few items,
like `Center` are already a combination of multiple flags.




## Static Fields and Properties

|  |  |
|--|--|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) [Center]({{site.url}}/Pages/Reference/TextAlign/Center.html)|A combination of XCenter and YCenter.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) [XCenter]({{site.url}}/Pages/Reference/TextAlign/XCenter.html)|On the x axis, the item should be centered.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) [XLeft]({{site.url}}/Pages/Reference/TextAlign/XLeft.html)|On the x axis, this item should start on the left.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) [XRight]({{site.url}}/Pages/Reference/TextAlign/XRight.html)|On the x axis, this item should start on the right.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) [YBottom]({{site.url}}/Pages/Reference/TextAlign/YBottom.html)|On the y axis, this item should start on the bottom.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) [YCenter]({{site.url}}/Pages/Reference/TextAlign/YCenter.html)|On the y axis, the item should be centered.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) [YTop]({{site.url}}/Pages/Reference/TextAlign/YTop.html)|On the y axis, this item should start at the top.|


