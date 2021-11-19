---
layout: default
title: Sprite.FromTex
description: Create a sprite from a Texture object!
---
# [Sprite]({{site.url}}/Pages/Reference/Sprite.html).FromTex

<div class='signature' markdown='1'>
```csharp
static Sprite FromTex(Tex image, SpriteType type, string atlasId)
```
Create a sprite from a Texture object!
</div>

|  |  |
|--|--|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) image|The texture to build a sprite from. Must be a             valid, 2D image!|
|[SpriteType]({{site.url}}/Pages/Reference/SpriteType.html) type|Should this sprite be atlased, or an             individual image? Adding this as an atlased image is better for             performance, but will cause the atlas to be rebuilt! Images that             take up too much space on the atlas, or might be loaded or             unloaded during runtime may be better as Single rather than             Atlased!|
|string atlasId|The name of which atlas the sprite should              belong to, this is only relevant if the SpriteType is Atlased.|
|RETURNS: [Sprite]({{site.url}}/Pages/Reference/Sprite.html)|A Sprite asset, or null if the image failed when adding to the sprite system!|




