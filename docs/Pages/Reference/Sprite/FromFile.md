---
layout: default
title: Sprite.FromFile
description: Create a sprite from an image file! This loads a Texture from file, and then uses that Texture as the source for the Sprite.
---
# [Sprite]({{site.url}}/Pages/Reference/Sprite.html).FromFile

<div class='signature' markdown='1'>
```csharp
static Sprite FromFile(string file, SpriteType type, string atlasId)
```
Create a sprite from an image file! This loads a Texture
from file, and then uses that Texture as the source for the
Sprite.
</div>

|  |  |
|--|--|
|string file|The filename of the image, an absolute              filename, or a filename relative to the assets folder. Supports              jpg, png, tga, bmp, psd, gif, hdr, pic.|
|[SpriteType]({{site.url}}/Pages/Reference/SpriteType.html) type|Should this sprite be atlased, or an             individual image? Adding this as an atlased image is better for             performance, but will cause the atlas to be rebuilt! Images that             take up too much space on the atlas, or might be loaded or              unloaded during runtime may be better as Single rather than             Atlased!|
|string atlasId|The name of which atlas the sprite should             belong to, this is only relevant if the SpriteType is Atlased.|
|RETURNS: [Sprite]({{site.url}}/Pages/Reference/Sprite.html)|A Sprite asset, or null if the image failed to load!|




