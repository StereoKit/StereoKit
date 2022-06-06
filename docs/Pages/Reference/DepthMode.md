---
layout: default
title: DepthMode
description: This is used to determine what kind of depth buffer StereoKit uses!
---
# enum DepthMode

This is used to determine what kind of depth buffer
StereoKit uses!

## Enum Values

|  |  |
|--|--|
|Balanced|Default mode, uses 16 bit on mobile devices like HoloLens and Quest, and 32 bit on higher powered platforms like PC. If you need a far view distance even on mobile devices, prefer D32 or Stencil instead.|
|D16|16 bit depth buffer, this is fast and recommended for devices like the HoloLens. This is especially important for fast depth based reprojection. Far view distances will suffer here though, so keep your clipping far plane as close as possible.|
|D32|32 bit depth buffer, should look great at any distance! If you must have the best, then this is the best. If you're interested in this one, Stencil may also be plenty for you, as 24 bit depth is also pretty peachy.|
|Stencil|24 bit depth buffer with 8 bits of stencil data. 24 bits is generally plenty for a depth buffer, so using the rest for stencil can open up some nice options! StereoKit has limited stencil support right now though (v0.3).|
