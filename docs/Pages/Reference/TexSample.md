---
layout: default
title: TexSample
description: How does the shader grab pixels from the texture? Or more specifically, how does the shader grab colors between the provided pixels? If you'd like an in-depth explanation of these topics, check out [this exploration of texture filtering](https.//medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec) by graphics wizard Ben Golus.
---
# TexSample

How does the shader grab pixels from the texture? Or more specifically,
how does the shader grab colors between the provided pixels? If you'd like an
in-depth explanation of these topics, check out [this exploration of texture filtering](https://medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec)
by graphics wizard Ben Golus.




## Static Fields and Properties

|  |  |
|--|--|
|[TexSample]({{site.url}}/Pages/Reference/TexSample.html) [Anisotropic]({{site.url}}/Pages/Reference/TexSample/Anisotropic.html)|This helps reduce texture blurriness when a surface is viewed at an extreme angle!|
|[TexSample]({{site.url}}/Pages/Reference/TexSample.html) [Linear]({{site.url}}/Pages/Reference/TexSample/Linear.html)|Use a linear blend between adjacent pixels, this creates a smooth, blurry look when texture resolution is too low.|
|[TexSample]({{site.url}}/Pages/Reference/TexSample.html) [Point]({{site.url}}/Pages/Reference/TexSample/Point.html)|Choose the nearest pixel's color! This makes your texture look like pixel art if you're too close.|


