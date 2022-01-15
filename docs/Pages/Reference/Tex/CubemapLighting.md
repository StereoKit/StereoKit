---
layout: default
title: Tex.CubemapLighting
description: ONLY valid for cubemap textures! This will calculate a spherical harmonics representation of the cubemap for use with StereoKit's lighting. First call may take a frame or two of time, but subsequent calls will pull from a cached value.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).CubemapLighting

<div class='signature' markdown='1'>
[SphericalHarmonics]({{site.url}}/Pages/Reference/SphericalHarmonics.html) CubemapLighting{ get }
</div>

## Description
ONLY valid for cubemap textures! This will calculate a
spherical harmonics representation of the cubemap for use with
StereoKit's lighting. First call may take a frame or two of time,
but subsequent calls will pull from a cached value.

