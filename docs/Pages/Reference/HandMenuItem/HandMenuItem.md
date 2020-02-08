---
layout: default
title: HandMenuItem.HandMenuItem
description: A menu item that'll load another layer when selected.
---
# [HandMenuItem]({{site.url}}/Pages/Reference/HandMenuItem.html).HandMenuItem

<div class='signature' markdown='1'>
 HandMenuItem(string name, [Sprite]({{site.url}}/Pages/Reference/Sprite.html) image, Action callback, string layerName)
</div>

|  |  |
|--|--|
|string name|Display name of the item.|
|[Sprite]({{site.url}}/Pages/Reference/Sprite.html) image|Display image of the item, null is fine!|
|Action callback|The callback that should be performed when this menu             item is selected.|
|string layerName|This is the layer name used to find the next layer              for the menu! Get the spelling right, try using const strings!|

A menu item that'll load another layer when selected.
<div class='signature' markdown='1'>
 HandMenuItem(string name, [Sprite]({{site.url}}/Pages/Reference/Sprite.html) image, Action callback, [HandMenuAction]({{site.url}}/Pages/Reference/HandMenuAction.html) action)
</div>

|  |  |
|--|--|
|string name|Display name of the item.|
|[Sprite]({{site.url}}/Pages/Reference/Sprite.html) image|Display image of the item, null is fine!|
|Action callback|The callback that should be performed when this menu             item is selected.|
|[HandMenuAction]({{site.url}}/Pages/Reference/HandMenuAction.html) action|Describes the menu related behavior of this menu item,             should it close the menu? Open another layer? Go back to the             previous menu?|

Makes a menu item!



