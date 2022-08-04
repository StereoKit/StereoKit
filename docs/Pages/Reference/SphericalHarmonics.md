---
layout: default
title: SphericalHarmonics
description: Spherical Harmonics are kinda like Fourier, but on a sphere. That doesn't mean terribly much to me, and could be wrong, but check out [here](http.//www.ppsloan.org/publications/StupidSH36.pdf) for more details about how Spherical Harmonics work in this context!  However, the more prctical thing is, SH can be a function that describes a value over the surface of a sphere! This is particularly useful for lighting, since you can basically store the lighting information for a space in this value! This is often used for lightmap data, or a light probe grid, but StereoKit just uses a single SH for the entire scene. It's a gross oversimplification, but looks quite good, and is really fast! That's extremely great when you're trying to hit 60fps, or even 144fps.
---
# struct SphericalHarmonics

Spherical Harmonics are kinda like Fourier, but on a sphere.
That doesn't mean terribly much to me, and could be wrong, but check
out [here](http://www.ppsloan.org/publications/StupidSH36.pdf) for
more details about how Spherical Harmonics work in this context!

However, the more prctical thing is, SH can be a function that
describes a value over the surface of a sphere! This is particularly
useful for lighting, since you can basically store the lighting
information for a space in this value! This is often used for
lightmap data, or a light probe grid, but StereoKit just uses a
single SH for the entire scene. It's a gross oversimplification, but
looks quite good, and is really fast! That's extremely great when
you're trying to hit 60fps, or even 144fps.

## Instance Fields and Properties

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [coefficient1]({{site.url}}/Pages/Reference/SphericalHarmonics/coefficient1.html)|A set of RGB coefficients|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [coefficient2]({{site.url}}/Pages/Reference/SphericalHarmonics/coefficient2.html)|A set of RGB coefficients|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [coefficient3]({{site.url}}/Pages/Reference/SphericalHarmonics/coefficient3.html)|A set of RGB coefficients|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [coefficient4]({{site.url}}/Pages/Reference/SphericalHarmonics/coefficient4.html)|A set of RGB coefficients|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [coefficient5]({{site.url}}/Pages/Reference/SphericalHarmonics/coefficient5.html)|A set of RGB coefficients|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [coefficient6]({{site.url}}/Pages/Reference/SphericalHarmonics/coefficient6.html)|A set of RGB coefficients|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [coefficient7]({{site.url}}/Pages/Reference/SphericalHarmonics/coefficient7.html)|A set of RGB coefficients|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [coefficient8]({{site.url}}/Pages/Reference/SphericalHarmonics/coefficient8.html)|A set of RGB coefficients|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [coefficient9]({{site.url}}/Pages/Reference/SphericalHarmonics/coefficient9.html)|A set of RGB coefficients|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [DominantLightDirection]({{site.url}}/Pages/Reference/SphericalHarmonics/DominantLightDirection.html)|Returns the dominant direction of the light represented by this spherical harmonics data. The direction value is normalized.  You can get the color of the light in this direction by using the struct's Sample method: `light.Sample(-light.DominantLightDirection)`.|

## Instance Methods

|  |  |
|--|--|
|[SphericalHarmonics]({{site.url}}/Pages/Reference/SphericalHarmonics/SphericalHarmonics.html)|Creates a SphericalHarmonic from an array of coefficients. Useful for loading stored data!|
|[Add]({{site.url}}/Pages/Reference/SphericalHarmonics/Add.html)|Adds a 'directional light' to the lighting approximation. This can be used to bake a multiple light setup, or accumulate light from a field of points.|
|[Brightness]({{site.url}}/Pages/Reference/SphericalHarmonics/Brightness.html)|Scales all the SphericalHarmonic's coefficients! This behaves as if you're modifying the brightness of the lighting this object represents.|
|[Sample]({{site.url}}/Pages/Reference/SphericalHarmonics/Sample.html)|Look up the color information in a particular direction!|
|[ToArray]({{site.url}}/Pages/Reference/SphericalHarmonics/ToArray.html)|Converts the SphericalHarmonic into an array of coefficients 9 long. Useful for storing calculated data!|

## Static Methods

|  |  |
|--|--|
|[FromLights]({{site.url}}/Pages/Reference/SphericalHarmonics/FromLights.html)|Creates a SphericalHarmonics approximation of the irradiance given from a set of directional lights!|
