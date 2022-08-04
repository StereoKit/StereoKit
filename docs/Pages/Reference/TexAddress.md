---
layout: default
title: TexAddress
description: What happens when the shader asks for a texture coordinate that's outside the texture?? Believe it or not, this happens plenty often!
---
# enum TexAddress

What happens when the shader asks for a texture coordinate
that's outside the texture?? Believe it or not, this happens plenty
often!

## Enum Values

|  |  |
|--|--|
|Clamp|Clamp the UV coordinates to the edge of the texture! This'll create color streaks that continue to forever. This is actually really great for non-looping textures that you know will always be accessed on the 0-1 range.|
|Mirror|Like Wrap, but it reflects the image each time! Who needs this? I'm not sure!! But the graphics card can do it, so now you can too!|
|Wrap|Wrap the UV coordinate around to the other side of the texture! This is basically like a looping texture, and is an excellent default. If you can see weird bits of color at the edges of your texture, this may be due to Wrap blending the color with the other side of the texture, Clamp may be better in such cases.|
