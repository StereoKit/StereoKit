---
layout: default
title: Transparency
description: Also known as 'alpha' for those in the know. But there's actually more than one type of transparency in rendering! The horrors. We're keepin' it fairly simple for now, so you get three options!
---
# Transparency

Also known as 'alpha' for those in the know. But there's actually more than
one type of transparency in rendering! The horrors. We're keepin' it fairly simple for
now, so you get three options!


## Fields and Properties

|  |  |
|--|--|
|[Transparency]({{site.url}}/Pages/Reference/Transparency.html) [Blend]({{site.url}}/Pages/Reference/Transparency/Blend.html)|This will blend with the pixels behind it. This is transparent! It doesn't write to the z-buffer, and it's slower than opaque materials.|
|[Transparency]({{site.url}}/Pages/Reference/Transparency.html) [Clip]({{site.url}}/Pages/Reference/Transparency/Clip.html)|This is sort of transparent! It can sample a texture, and discard pixels that are below a certain threshold. It doesn't blend with colors behind it, but it's pretty fast, and can write to the z-buffer no problem!|
|[Transparency]({{site.url}}/Pages/Reference/Transparency.html) [None]({{site.url}}/Pages/Reference/Transparency/None.html)|Not actually transparent! This is opaque! Solid! It's the default option, and it's the fastest option! Opaque objects write to the z-buffer, the occlude pixels behind them, and they can be used as input to important Mixed Reality features like Late Stage Reprojection that'll make your view more stable!|



