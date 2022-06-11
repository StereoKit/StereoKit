---
layout: default
title: TexSample
description: How does the shader grab pixels from the texture? Or more specifically, how does the shader grab colors between the provided pixels? If you'd like an in-depth explanation of these topics, check out [this exploration of texture filtering](https.//medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec) by graphics wizard Ben Golus.
---
# enum TexSample

How does the shader grab pixels from the texture? Or more
specifically, how does the shader grab colors between the provided
pixels? If you'd like an in-depth explanation of these topics, check
out [this exploration of texture filtering](https://medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec)
by graphics wizard Ben Golus.

## Enum Values

|  |  |
|--|--|
|Anisotropic|This helps reduce texture blurriness when a surface is viewed at an extreme angle!|
|Linear|Use a linear blend between adjacent pixels, this creates a smooth, blurry look when texture resolution is too low.|
|Point|Choose the nearest pixel's color! This makes your texture look like pixel art if you're too close.|
