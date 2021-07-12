---
layout: default
title: Tex
description: This is the texture asset class! This encapsulates 2D images, texture arrays, cubemaps, and rendertargets! It can load any image format that stb_image can, (jpg, png, tga, bmp, psd, gif, hdr, pic) plus more later on, and you can also create textures procedurally.
---
# class Tex

This is the texture asset class! This encapsulates 2D images,
texture arrays, cubemaps, and rendertargets! It can load any image
format that stb_image can, (jpg, png, tga, bmp, psd, gif, hdr, pic)
plus more later on, and you can also create textures procedurally.


## Instance Fields and Properties

|  |  |
|--|--|
|[TexAddress]({{site.url}}/Pages/Reference/TexAddress.html) [AddressMode]({{site.url}}/Pages/Reference/Tex/AddressMode.html)|When looking at a UV texture coordinate on this texture, how do we handle values larger than 1, or less than zero? Do we Wrap to the other side? Clamp it between 0-1, or just keep Mirroring back and forth? Wrap is the default.|
|int [Anisoptropy]({{site.url}}/Pages/Reference/Tex/Anisoptropy.html)|When SampleMode is set to Anisotropic, this is the number of samples the GPU takes to figure out the correct color. Default is 4, and 16 is pretty high.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [Format]({{site.url}}/Pages/Reference/Tex/Format.html)|The StereoKit format this texture was initialized with.|
|int [Height]({{site.url}}/Pages/Reference/Tex/Height.html)|The height of the texture, in pixels.|
|string [Id]({{site.url}}/Pages/Reference/Tex/Id.html)|Allows you to set the Id of the texture to a specific Id.|
|[TexSample]({{site.url}}/Pages/Reference/TexSample.html) [SampleMode]({{site.url}}/Pages/Reference/Tex/SampleMode.html)|When sampling a texture that's stretched, or shrunk beyond its screen size, how do we handle figuring out which color to grab from the texture? Default is Linear.|
|int [Width]({{site.url}}/Pages/Reference/Tex/Width.html)|The width of the texture, in pixels.|


## Instance Methods

|  |  |
|--|--|
|[Tex]({{site.url}}/Pages/Reference/Tex/Tex.html)|Sets up an empty texture container! Fill it with data using SetColors next! Creates a default unique asset Id.|
|[AddZBuffer]({{site.url}}/Pages/Reference/Tex/AddZBuffer.html)|Only applicable if this texture is a rendertarget! This creates and attaches a zbuffer surface to the texture for use when rendering to it.|
|[SetColors]({{site.url}}/Pages/Reference/Tex/SetColors.html)|Set the texture's pixels using a pointer to a chunk of memory! This is great if you're pulling in some color data from native code, and don't want to pay the cost of trying to marshal that data around.|
|[SetSize]({{site.url}}/Pages/Reference/Tex/SetSize.html)|Set the texture's size without providing any color data. In most cases, you should probably just call SetColors instead, but this can be useful if you're adding color data some other way, such as when blitting or rendering to it.|


## Static Fields and Properties

|  |  |
|--|--|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) [Black]({{site.url}}/Pages/Reference/Tex/Black.html)|See `Default.TexBlack`|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) [Flat]({{site.url}}/Pages/Reference/Tex/Flat.html)|See `Default.TexFlat`|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) [Gray]({{site.url}}/Pages/Reference/Tex/Gray.html)|See `Default.TexGray`|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) [Rough]({{site.url}}/Pages/Reference/Tex/Rough.html)|See `Default.TexRough`|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) [White]({{site.url}}/Pages/Reference/Tex/White.html)|See `Default.Tex`|


## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Tex/Find.html)|Finds a texture that matches the given Id! Check out the DefaultIds static class for some built-in ids.|
|[FromColors]({{site.url}}/Pages/Reference/Tex/FromColors.html)|Creates a texture and sets the texture's pixels using a color array! This will be an image of type `TexType.Image`, and a format of `TexFormat.Rgba32` or `TexFormat.Rgba32Linear` depending on the value of the sRGBData parameter.|
|[FromCubemapEquirectangular]({{site.url}}/Pages/Reference/Tex/FromCubemapEquirectangular.html)|Creates a cubemap texture from a single equirectangular image! You know, the ones that look like an unwrapped globe with the poles all streetched out. It uses some fancy shaders and texture blitting to create 6 faces from the equirectangular image.|
|[FromCubemapFile]({{site.url}}/Pages/Reference/Tex/FromCubemapFile.html)|Creates a cubemap texture from 6 different image files! If you have a single equirectangular image, use Tex.FromEquirectangular instead. Asset Id will be the first filename.|
|[FromFile]({{site.url}}/Pages/Reference/Tex/FromFile.html)|Loads an image file directly into a texture! Supported formats are: jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id will be the same as the filename.|
|[FromFiles]({{site.url}}/Pages/Reference/Tex/FromFiles.html)|Loads an array of image files directly into a single array texture! Array textures are often useful for shader effects, layering, material merging, weird stuff, and will generally need a specific shader to support it. Supported formats are: jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id will be the hash of all the filenames merged consecutively.|
|[FromMemory]({{site.url}}/Pages/Reference/Tex/FromMemory.html)|Loads an image file stored in memory directly into a texture! Supported formats are: jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id will be the same as the filename.|
|[GenCubemap]({{site.url}}/Pages/Reference/Tex/GenCubemap.html)|Generates a cubemap texture from a gradient and a direction! These are entirely suitable for skyboxes, which you can set via Renderer.SkyTex.|

