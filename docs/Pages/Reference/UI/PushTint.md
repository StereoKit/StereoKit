---
layout: default
title: UI.PushTint
description: All UI between PushTint and its matching PopTint will be tinted with this color. This is implemented by multiplying this color with the current color of the UI element. The default is a White (1,1,1,1) identity tint.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).PushTint

<div class='signature' markdown='1'>
```csharp
static void PushTint(Color colorGamma)
```
All UI between PushTint and its matching PopTint will be
tinted with this color. This is implemented by multiplying this
color with the current color of the UI element. The default is a
White (1,1,1,1) identity tint.
</div>

|  |  |
|--|--|
|[Color]({{site.url}}/Pages/Reference/Color.html) colorGamma|A normal (gamma corrected) color value.             This is internally converted to linear, so tint multiplication             happens on linear color values.|




