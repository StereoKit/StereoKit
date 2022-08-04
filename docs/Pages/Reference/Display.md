---
layout: default
title: Display
description: TODO. remove this in v0.4 This describes the type of display tech used on a Mixed Reality device. This will be replaced by DisplayBlend in v0.4.
---
# enum Display

TODO: remove this in v0.4
This describes the type of display tech used on a Mixed
Reality device. This will be replaced by `DisplayBlend` in v0.4.

## Enum Values

|  |  |
|--|--|
|Additive|This display is transparent, and adds light on top of the real world. This is equivalent to a HoloLens type of device.|
|AnyTransparent|This matches either transparent display type! Additive or Blend. For use when you just want to see the world behind your application.|
|Blend|This is a physically opaque display, but with a camera passthrough displaying the world behind it anyhow. This would be like a Varjo XR-1, or phone-camera based AR.|
|None|Default value, when using this as a search type, it will fall back to default behavior which defers to platform preference.|
|Opaque|This display is opaque, with no view into the real world! This is equivalent to a VR headset, or a PC screen.|
|Passthrough|Use Display.Blend instead, to be removed in v0.4|
