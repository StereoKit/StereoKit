---
layout: default
title: Renderer
description: Do you need to draw something? Well, you're probably in the right place! This static class includes a variety of different drawing methods, from rendering Models and Meshes, to setting rendering options and drawing to offscreen surfaces! Even better, it's entirely a static class, so you can call it from anywhere .)
---
# Renderer

Do you need to draw something? Well, you're probably in the right place!
This static class includes a variety of different drawing methods, from rendering
Models and Meshes, to setting rendering options and drawing to offscreen surfaces!
Even better, it's entirely a static class, so you can call it from anywhere :)




## Static Fields and Properties

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) [CameraRoot]({{site.url}}/Pages/Reference/Renderer/CameraRoot.html)|Sets and gets the root transform of the camera! This will be the identity matrix by default. The user's head  location will then be relative to this point. This is great to use if you're trying to do teleportation, redirected walking, or just shifting the floor around.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) [ClearColor]({{site.url}}/Pages/Reference/Renderer/ClearColor.html)|This is the color the renderer will clear the screen to when beginning to draw a new frame. This is ignored on displays with transparent screens|
|bool [EnableSky]({{site.url}}/Pages/Reference/Renderer/EnableSky.html)|Enables or disables rendering of the skybox texture! It's enabled by default on Opaque displays, and completely unavailable for transparent displays.|
|[SphericalHarmonics]({{site.url}}/Pages/Reference/SphericalHarmonics.html) [SkyLight]({{site.url}}/Pages/Reference/Renderer/SkyLight.html)|Sets the lighting information for the scene! You can build one through `SphericalHarmonics.FromLights`, or grab one from `Tex.FromEquirectangular` or `Tex.GenCubemap`|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) [SkyTex]({{site.url}}/Pages/Reference/Renderer/SkyTex.html)|Set a cubemap skybox texture for rendering a background! This is only visible on Opaque displays, since transparent displays have the real world behind them already! StereoKit has a a default procedurally generated skybox. You can load one with `Tex.FromEquirectangular`, `Tex.GenCubemap`. If you're trying to affect the lighting, see `Renderer.SkyLight`.|


## Static Methods

|  |  |
|--|--|
|[Add]({{site.url}}/Pages/Reference/Renderer/Add.html)|Adds a mesh to the render queue for this frame! If the Hierarchy has a transform on it, that transform is combined with the Matrix provided here.|
|[Blit]({{site.url}}/Pages/Reference/Renderer/Blit.html)|Renders a Material onto a rendertarget texture! StereoKit uses a 4 vert quad stretched over the surface of the texture, and renders the material onto it to the texture.|
|[Screenshot]({{site.url}}/Pages/Reference/Renderer/Screenshot.html)|Schedules a screenshot for the end of the frame! The view will be rendered from the given position at the given point, with a resolution the same size as the screen's surface. It'll be saved as a .jpg file at the filename provided.|
|[SetClip]({{site.url}}/Pages/Reference/Renderer/SetClip.html)|Set the near and far clipping planes of the camera! These are important to z-buffer quality, especially when using low bit depth z-buffers as recommended for devices like the HoloLens. The smaller the range between the near and far planes, the better your z-buffer will look! If you see flickering on objects that are overlapping, try making the range smaller.|
|[SetFOV]({{site.url}}/Pages/Reference/Renderer/SetFOV.html)|Only works for flatscreen! This updates the camera's projection matrix with a new field of view.|

