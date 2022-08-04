---
layout: default
title: Renderer
description: Do you need to draw something? Well, you're probably in the right place! This static class includes a variety of different drawing methods, from rendering Models and Meshes, to setting rendering options and drawing to offscreen surfaces! Even better, it's entirely a static class, so you can call it from anywhere .)
---
# static class Renderer

Do you need to draw something? Well, you're probably in the right place!
This static class includes a variety of different drawing methods, from rendering
Models and Meshes, to setting rendering options and drawing to offscreen surfaces!
Even better, it's entirely a static class, so you can call it from anywhere :)

## Static Fields and Properties

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) [CameraRoot]({{site.url}}/Pages/Reference/Renderer/CameraRoot.html)|Sets and gets the root transform of the camera! This will be the identity matrix by default. The user's head  location will then be relative to this point. This is great to use if you're trying to do teleportation, redirected walking, or just shifting the floor around.|
|[RenderLayer]({{site.url}}/Pages/Reference/RenderLayer.html) [CaptureFilter]({{site.url}}/Pages/Reference/Renderer/CaptureFilter.html)|This is the current render layer mask for Mixed Reality Capture, or 2nd person observer rendering. By default, this is directly linked to Renderer.LayerFilter, but this behavior can be overridden via `Renderer.OverrideCaptureFilter`.|
|[Color]({{site.url}}/Pages/Reference/Color.html) [ClearColor]({{site.url}}/Pages/Reference/Renderer/ClearColor.html)|This is the gamma space color the renderer will clear the screen to when beginning to draw a new frame. This is ignored on displays with transparent screens|
|bool [EnableSky]({{site.url}}/Pages/Reference/Renderer/EnableSky.html)|Enables or disables rendering of the skybox texture! It's enabled by default on Opaque displays, and completely unavailable for transparent displays.|
|bool [HasCaptureFilter]({{site.url}}/Pages/Reference/Renderer/HasCaptureFilter.html)|This tells if CaptureFilter has been overridden to a specific value via `Renderer.OverrideCaptureFilter`.|
|[RenderLayer]({{site.url}}/Pages/Reference/RenderLayer.html) [LayerFilter]({{site.url}}/Pages/Reference/Renderer/LayerFilter.html)|By default, StereoKit renders all layers. This is a bit flag that allows you to change which layers StereoKit renders for the primary viewpoint. To change what layers a visual is on, use a Draw method that includes a RenderLayer as a parameter.|
|[Projection]({{site.url}}/Pages/Reference/Projection.html) [Projection]({{site.url}}/Pages/Reference/Renderer/Projection.html)|For flatscreen applications only! This allows you to change the camera projection between perspective and orthographic projection. This may be of interest for some category of UI work, but is generally a niche piece of functionality.  Swapping between perspective and orthographic will also switch the clipping planes and field of view to the values associated with that mode. See `SetClip`/`SetFov` for perspective, and `SetOrthoClip`/`SetOrthoSize` for orthographic.|
|[SphericalHarmonics]({{site.url}}/Pages/Reference/SphericalHarmonics.html) [SkyLight]({{site.url}}/Pages/Reference/Renderer/SkyLight.html)|Sets the lighting information for the scene! You can build one through `SphericalHarmonics.FromLights`, or grab one from `Tex.FromEquirectangular` or `Tex.GenCubemap`|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) [SkyTex]({{site.url}}/Pages/Reference/Renderer/SkyTex.html)|Set a cubemap skybox texture for rendering a background! This is only visible on Opaque displays, since transparent displays have the real world behind them already! StereoKit has a a default procedurally generated skybox. You can load one with `Tex.FromEquirectangular`, `Tex.GenCubemap`. If you're trying to affect the lighting, see `Renderer.SkyLight`.|

## Static Methods

|  |  |
|--|--|
|[Add]({{site.url}}/Pages/Reference/Renderer/Add.html)|Adds a mesh to the render queue for this frame! If the Hierarchy has a transform on it, that transform is combined with the Matrix provided here.|
|[Blit]({{site.url}}/Pages/Reference/Renderer/Blit.html)|Renders a Material onto a rendertarget texture! StereoKit uses a 4 vert quad stretched over the surface of the texture, and renders the material onto it to the texture.|
|[OverrideCaptureFilter]({{site.url}}/Pages/Reference/Renderer/OverrideCaptureFilter.html)|The CaptureFilter is a layer mask for Mixed Reality Capture, or 2nd person observer rendering. On HoloLens and WMR, this is the video rendering feature. This allows you to hide, or reveal certain draw calls when rendering video output.  By default, the CaptureFilter will always be the same as `Render.LayerFilter`, overriding this will mean this filter no longer updates with `LayerFilter`.|
|[RenderTo]({{site.url}}/Pages/Reference/Renderer/RenderTo.html)|This renders the current scene to the indicated rendertarget texture, from the specified viewpoint. This call enqueues a render that occurs immediately before the screen itself is rendered.|
|[Screenshot]({{site.url}}/Pages/Reference/Renderer/Screenshot.html)|Schedules a screenshot for the end of the frame! The view will be rendered from the given position at the given point, with a resolution the same size as the screen's surface. It'll be saved as a .jpg file at the filename provided.|
|[SetClip]({{site.url}}/Pages/Reference/Renderer/SetClip.html)|Set the near and far clipping planes of the camera! These are important to z-buffer quality, especially when using low bit depth z-buffers as recommended for devices like the HoloLens. The smaller the range between the near and far planes, the better your z-buffer will look! If you see flickering on objects that are overlapping, try making the range smaller.  These values only affect perspective mode projection, which is the default projection mode.|
|[SetFOV]({{site.url}}/Pages/Reference/Renderer/SetFOV.html)|Only works for flatscreen! This updates the camera's projection matrix with a new field of view.  This value only affects perspective mode projection, which is the default projection mode.|
|[SetOrthoClip]({{site.url}}/Pages/Reference/Renderer/SetOrthoClip.html)|Set the near and far clipping planes of the camera! These are important to z-buffer quality, especially when using low bit depth z-buffers as recommended for devices like the HoloLens. The smaller the range between the near and far planes, the better your z-buffer will look! If you see flickering on objects that are overlapping, try making the range smaller.  These values only affect orthographic mode projection, which is only available in flatscreen.|
|[SetOrthoSize]({{site.url}}/Pages/Reference/Renderer/SetOrthoSize.html)|This sets the size of the orthographic projection's viewport. You can use this feature to zoom in and out of the scene.  This value only affects orthographic mode projection, which is only available in flatscreen.|
