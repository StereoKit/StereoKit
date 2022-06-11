---
layout: default
title: Sprite
description: A Sprite is an image that's set up for direct 2D rendering, without using a mesh or model! This is technically a wrapper over a texture, but it also includes atlasing functionality, which can be pretty important to performance! This is used a lot in UI, for image rendering.  Atlasing is not currently implemented, it'll swap to Single for now. But here's how it works!  StereoKit will batch your sprites into an atlas if you ask it to! This puts all the images on a single texture to significantly reduce draw calls when many images are present. Any time you add a sprite to an atlas, it'll be marked as dirty and rebuilt at the end of the frame. So it can be a good idea to add all your images to the atlas on initialize rather than during execution!  Since rendering is atlas based, you also have only one material per atlas. So this is why you might wish to put a sprite in one atlas or another, so you can apply different
---
# class Sprite

A Sprite is an image that's set up for direct 2D rendering,
without using a mesh or model! This is technically a wrapper over a
texture, but it also includes atlasing functionality, which can be
pretty important to performance! This is used a lot in UI, for image
rendering.

Atlasing is not currently implemented, it'll swap to Single for now.
But here's how it works!

StereoKit will batch your sprites into an atlas if you ask it to!
This puts all the images on a single texture to significantly reduce
draw calls when many images are present. Any time you add a sprite to
an atlas, it'll be marked as dirty and rebuilt at the end of the
frame. So it can be a good idea to add all your images to the atlas
on initialize rather than during execution!

Since rendering is atlas based, you also have only one material per
atlas. So this is why you might wish to put a sprite in one atlas or
another, so you can apply different

## Instance Fields and Properties

|  |  |
|--|--|
|float [Aspect]({{site.url}}/Pages/Reference/Sprite/Aspect.html)|The aspect ratio of the sprite! This is width/height. You may also be interested in the NormalizedDimensions property, which are normalized to the 0-1 range.|
|int [Height]({{site.url}}/Pages/Reference/Sprite/Height.html)|Height of the sprite, in pixels.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [NormalizedDimensions]({{site.url}}/Pages/Reference/Sprite/NormalizedDimensions.html)|Width and height of the sprite, normalized so the maximum value is 1.|
|int [Width]({{site.url}}/Pages/Reference/Sprite/Width.html)|Width of the sprite, in pixels.|

## Instance Methods

|  |  |
|--|--|
|[Draw]({{site.url}}/Pages/Reference/Sprite/Draw.html)|Draw the sprite on a quad with the provided transform!|

## Static Methods

|  |  |
|--|--|
|[FromFile]({{site.url}}/Pages/Reference/Sprite/FromFile.html)|Create a sprite from an image file! This loads a Texture from file, and then uses that Texture as the source for the Sprite.|
|[FromTex]({{site.url}}/Pages/Reference/Sprite/FromTex.html)|Create a sprite from a Texture object!|
