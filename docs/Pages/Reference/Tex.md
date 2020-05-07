---
layout: default
title: Tex
description: This is the texture asset class! This encapsulates 2D images, texture arrays, cubemaps, and rendertargets! It can load any image format that stb_image can, (jpg, png, tga, bmp, psd, gif, hdr, pic) plus more later on, and you can also create textures procedurally.
---
# Tex

This is the texture asset class! This encapsulates 2D images,
texture arrays, cubemaps, and rendertargets! It can load any image
format that stb_image can, (jpg, png, tga, bmp, psd, gif, hdr, pic)
plus more later on, and you can also create textures procedurally.



## Instance Methods

|  |  |
|--|--|
|[Tex]({{site.url}}/Pages/Reference/Tex/Tex.html)|Sets up an empty texture container! Fill it with data using SetColors next! Creates a default unique asset Id.|
|[SetColors]({{site.url}}/Pages/Reference/Tex/SetColors.html)|Set the texture's pixels using a pointer to a chunk of memory! This is great if you're pulling in some color data from native code, and don't want to pay the cost of trying to marshal that data around.|


## Static Fields and Properties

|  |  |
|--|--|
|[TexAddress]({{site.url}}/Pages/Reference/TexAddress.html) [AddressMode]({{site.url}}/Pages/Reference/Tex/AddressMode.html)|When looking at a UV texture coordinate on this texture, how do we handle values larger than 1, or less than zero? Do we Wrap to the other side? Clamp it between 0-1, or just keep Mirroring back and forth? Wrap is the default.|
|int [Anisoptropy]({{site.url}}/Pages/Reference/Tex/Anisoptropy.html)|When SampleMode is set to Anisotropic, this is the number of samples the GPU takes to figure out the correct color. Default is 4, and 16 is pretty high.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [Format]({{site.url}}/Pages/Reference/Tex/Format.html)|The StereoKit format this texture was initialized with.|
|int [Height]({{site.url}}/Pages/Reference/Tex/Height.html)|The height of the texture, in pixels.|
|string [Id]({{site.url}}/Pages/Reference/Tex/Id.html)|Allows you to set the Id of the texture to a specific Id.|
|[TexSample]({{site.url}}/Pages/Reference/TexSample.html) [SampleMode]({{site.url}}/Pages/Reference/Tex/SampleMode.html)|When sampling a texture that's stretched, or shrunk beyond its screen size, how do we handle figuring out which color to grab from the texture? Default is Linear.|
|int [Width]({{site.url}}/Pages/Reference/Tex/Width.html)|The width of the texture, in pixels.|


## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Tex/Find.html)|Finds a texture that matches the given Id! Check out the DefaultIds static class for some built-in ids.|
|[FromCubemapEquirectangular]({{site.url}}/Pages/Reference/Tex/FromCubemapEquirectangular.html)|Creates a cubemap texture from a single equirectangular image! You know, the ones that look like an unwrapped globe with the poles all streetched out. It uses some fancy shaders and texture blitting to create 6 faces from the equirectangular image.|
|[FromCubemapFile]({{site.url}}/Pages/Reference/Tex/FromCubemapFile.html)|Creates a cubemap texture from 6 different image files! If you have a single equirectangular image, use Tex.FromEquirectangular instead. Asset Id will be the first filename.|
|[FromFile]({{site.url}}/Pages/Reference/Tex/FromFile.html)|Loads an image file directly into a texture! Supported formats are: jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id will be the same as the filename.|
|[FromMemory]({{site.url}}/Pages/Reference/Tex/FromMemory.html)|Loads an image file stored in memory directly into a texture! Supported formats are: jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id will be the same as the filename.|
|[GenCubemap]({{site.url}}/Pages/Reference/Tex/GenCubemap.html)|Generates a cubemap texture from a gradient and a direction! These are entirely suitable for skyboxes, which you can set via Renderer.SkyTex.|

