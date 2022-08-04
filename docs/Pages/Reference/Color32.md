---
layout: default
title: Color32
description: A 32 bit color struct! This is often directly used by StereoKit data structures, and so is often necessary for setting texture data, or mesh data. Note that the Color type implicitly converts to Color32, so you can use the static methods there to create Color32 values!
---
# struct Color32

A 32 bit color struct! This is often directly used by StereoKit data
structures, and so is often necessary for setting texture data, or mesh data.
Note that the Color type implicitly converts to Color32, so you can use the
static methods there to create Color32 values!

## Instance Fields and Properties

|  |  |
|--|--|
|Byte [a]({{site.url}}/Pages/Reference/Color32/a.html)|Color components in the range of 0-255.|
|Byte [b]({{site.url}}/Pages/Reference/Color32/b.html)|Color components in the range of 0-255.|
|Byte [g]({{site.url}}/Pages/Reference/Color32/g.html)|Color components in the range of 0-255.|
|Byte [r]({{site.url}}/Pages/Reference/Color32/r.html)|Color components in the range of 0-255.|

## Instance Methods

|  |  |
|--|--|
|[ToString]({{site.url}}/Pages/Reference/Color32/ToString.html)|Mostly for debug purposes, this is a decent way to log or inspect the color in debug mode. Looks like "[r, g, b, a]"|

## Static Fields and Properties

|  |  |
|--|--|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) [Black]({{site.url}}/Pages/Reference/Color32/Black.html)|Pure opaque black! Same as (0,0,0,255).|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) [BlackTransparent]({{site.url}}/Pages/Reference/Color32/BlackTransparent.html)|Pure transparent black! Same as (0,0,0,0).|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) [White]({{site.url}}/Pages/Reference/Color32/White.html)|Pure opaque white! Same as (255,255,255,255).|

## Static Methods

|  |  |
|--|--|
|[Hex]({{site.url}}/Pages/Reference/Color32/Hex.html)|Create a color from an integer based hex value! This can make it easier to copy over colors from the web. This isn't a string function though, so you'll need to fill it out the whole way. Ex: `Color.Hex(0x0000FFFF)` would be RGBA(0,0,255,255).|

## Examples

```csharp
// You can create a color using Red, Green, Blue, Alpha values,
// but it's often a great recipe for making a bad color.
Color32 color = new Color32(255, 0, 0, 255); // Red

// Hue, Saturation, Value, Alpha is a more natural way of picking
// colors. You can use Color's HSV function, plus the implicit
// conversion to make a Color32!
color = Color.HSV(0, 1, 1, 1); // Red

// And there's a few static colors available if you need 'em.
color = Color32.White;
```

### Creating color from hex values
```csharp
Color   hex128 = Color  .Hex(0xFF0000FF); // Opaque Red
Color32 hex32  = Color32.Hex(0x00FF00FF); // Opaque Green
```

