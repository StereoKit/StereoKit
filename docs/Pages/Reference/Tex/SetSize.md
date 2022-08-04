---
layout: default
title: Tex.SetSize
description: Set the texture's size without providing any color data. In most cases, you should probably just call SetColors instead, but this can be useful if you're adding color data some other way, such as when blitting or rendering to it.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).SetSize

<div class='signature' markdown='1'>
```csharp
void SetSize(int width, int height)
```
Set the texture's size without providing any color data.
In most cases, you should probably just call SetColors instead,
but this can be useful if you're adding color data some other
way, such as when blitting or rendering to it.
</div>

|  |  |
|--|--|
|int width|Width in pixels of the texture. Powers of two             are generally best!|
|int height|Height in pixels of the texture. Powers of             two are generally best!|




