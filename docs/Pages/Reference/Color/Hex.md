---
layout: default
title: Color.Hex
description: Create a color from an integer based hex value! This can make it easier to copy over colors from the web. This isn't a string function though, so you'll need to fill it out the whole way. Ex. Color.Hex(0x0000FFFF) would be RGBA(0,0,1,1).
---
# [Color]({{site.url}}/Pages/Reference/Color.html).Hex

<div class='signature' markdown='1'>
```csharp
static Color Hex(uint hexValue)
```
Create a color from an integer based hex value! This can
make it easier to copy over colors from the web. This isn't a
string function though, so you'll need to fill it out the whole
way. Ex: `Color.Hex(0x0000FFFF)` would be RGBA(0,0,1,1).
</div>

|  |  |
|--|--|
|uint hexValue|An integer representing RGBA hex values!             Like: `0x0000FFFF`.|
|RETURNS: [Color]({{site.url}}/Pages/Reference/Color.html)|A 128 bit Color value.|





## Examples

### Creating color from hex values
```csharp
Color   hex128 = Color  .Hex(0xFF0000FF); // Opaque Red
Color32 hex32  = Color32.Hex(0x00FF00FF); // Opaque Green
```

