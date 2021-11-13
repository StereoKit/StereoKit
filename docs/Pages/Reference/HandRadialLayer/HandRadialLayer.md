---
layout: default
title: HandRadialLayer.HandRadialLayer
description: Creates a menu layer, this overload will calculate a backAngle if there are any back actions present in the item list.
---
# [HandRadialLayer]({{site.url}}/Pages/Reference/HandRadialLayer.html).HandRadialLayer

<div class='signature' markdown='1'>
```csharp
void HandRadialLayer(string name, HandMenuItem[] items)
```
Creates a menu layer, this overload will calculate a backAngle
if there are any back actions present in the item list.
</div>

|  |  |
|--|--|
|string name|Name of the layer, this is used for layer traversal, so             make sure you get the spelling right! Perhaps use const strings             for these.|
|HandMenuItem[] items|A list of menu items to display in this menu layer.|

<div class='signature' markdown='1'>
```csharp
void HandRadialLayer(string name, float startAngle, HandMenuItem[] items)
```
Creates a menu layer with an angle offset for the layer's rotation!
</div>

|  |  |
|--|--|
|string name|Name of the layer, this is used for layer traversal, so             make sure you get the spelling right! Perhaps use const strings             for these.|
|float startAngle|An angle offset for the layer, if you want a specific             orientation for the menu's contents. Note this may not behave as             expected if you're setting this manually and using the backAngle              as well.|
|HandMenuItem[] items|A list of menu items to display in this menu layer.|




