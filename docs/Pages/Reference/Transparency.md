---
layout: default
title: Transparency
description: Also known as 'alpha' for those in the know. But there's actually more than one type of transparency in rendering! The horrors. We're keepin' it fairly simple for now, so you get three options!
---
# enum Transparency

Also known as 'alpha' for those in the know. But there's
actually more than one type of transparency in rendering! The
horrors. We're keepin' it fairly simple for now, so you get three
options!

## Enum Values

|  |  |
|--|--|
|Add|This will straight up add the pixel color to the color buffer! This usually looks -really- glowy, so it makes for good particles or lighting effects.|
|Blend|This will blend with the pixels behind it. This is transparent! You may not want to write to the z-buffer, and it's slower than opaque materials.|
|None|Not actually transparent! This is opaque! Solid! It's the default option, and it's the fastest option! Opaque objects write to the z-buffer, the occlude pixels behind them, and they can be used as input to important Mixed Reality features like Late Stage Reprojection that'll make your view more stable!|
