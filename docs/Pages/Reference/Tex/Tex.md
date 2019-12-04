---
layout: default
title: Tex.Tex
description: Sets up an empty texture container! Fill it with data using SetColors next! Creates a default unique asset Id.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).Tex

<div class='signature' markdown='1'>
 Tex([TexType]({{site.url}}/Pages/Reference/TexType.html) textureType, [TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) textureFormat)
</div>

|  |  |
|--|--|
|[TexType]({{site.url}}/Pages/Reference/TexType.html) textureType|What type of texture is it? Just a 2D Image? A Cubemap? Should it have mip-maps?|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) textureFormat|What information is the texture composed of? 32 bit colors, 64 bit colors, etc.|

Sets up an empty texture container! Fill it with data using SetColors next! Creates a default unique asset Id.
<div class='signature' markdown='1'>
 Tex(string file)
</div>

|  |  |
|--|--|
|string file|An absolute filename, or a filename relative to the assets folder. Supports jpg, png, tga,             bmp, psd, gif, hdr, pic|

Loads an image file directly into a texture! Supported formats are: jpg, png, tga, bmp, psd, gif,
hdr, pic. Asset Id will be the same as the filename.
<div class='signature' markdown='1'>
 Tex(String[] cubeFaceFiles_xxyyzz)
</div>

|  |  |
|--|--|
|String[] cubeFaceFiles_xxyyzz|6 image filenames, in order of +X, -X, +Y, -Y, +Z, -Z.|

Creates a cubemap texture from 6 different image files! If you have a single equirectangular image, use
Tex.FromEquirectangular instead. Asset Id will be the first filename.



