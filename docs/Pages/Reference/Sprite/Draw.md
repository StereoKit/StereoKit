---
layout: default
title: Sprite.Draw
description: Draw the sprite on a quad with the provided transform!
---
# [Sprite]({{site.url}}/Pages/Reference/Sprite.html).Draw

<div class='signature' markdown='1'>
```csharp
void Draw(Matrix& transform, Color32 color)
```
Draw the sprite on a quad with the provided transform!
</div>

|  |  |
|--|--|
|Matrix& transform|A Matrix describing a transform from              model space to world space.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) color|Per-instance color data for this render item.|

<div class='signature' markdown='1'>
```csharp
void Draw(Matrix& transform)
```
Draw the sprite on a quad with the provided transform!
</div>

|  |  |
|--|--|
|Matrix& transform|A Matrix describing a transform from              model space to world space.|

<div class='signature' markdown='1'>
```csharp
void Draw(Matrix& transform, TextAlign anchorPosition)
```
Draws the sprite at the location specified by the
transform matrix. A sprite is always sized in model space as 1 x
Aspect meters on the x and y axes respectively, so scale
appropriately. The 'position' attribute describes what corner of
the sprite you're specifying the transform of.
</div>

|  |  |
|--|--|
|Matrix& transform|A Matrix describing a transform from              model space to world space. A sprite is always sized in model             space as 1 x Aspect meters on the x and y axes respectively, so             scale appropriately and remember that your anchor position may             affect the transform as well.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) anchorPosition|Describes what corner of the sprite             you're specifying the transform of. The 'Anchor' point or             'Origin' of the Sprite.|

<div class='signature' markdown='1'>
```csharp
void Draw(Matrix& transform, TextAlign anchorPosition, Color32 linearColor)
```
Draws the sprite at the location specified by the
transform matrix. A sprite is always sized in model space as 1 x
Aspect meters on the x and y axes respectively, so scale
appropriately. The 'position' attribute describes what corner of
the sprite you're specifying the transform of.
</div>

|  |  |
|--|--|
|Matrix& transform|A Matrix describing a transform from              model space to world space. A sprite is always sized in model             space as 1 x Aspect meters on the x and y axes respectively, so             scale appropriately and remember that your anchor position may             affect the transform as well.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) anchorPosition|Describes what corner of the sprite             you're specifying the transform of. The 'Anchor' point or             'Origin' of the Sprite.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) linearColor|Per-instance color data for this render             item. It is unmodified by StereoKit, and is generally interpreted             as linear.|




