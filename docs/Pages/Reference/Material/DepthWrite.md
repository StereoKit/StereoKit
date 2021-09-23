---
layout: default
title: Material.DepthWrite
description: Should this material write to the ZBuffer? For opaque objects, this generally should be true. But transparent objects writing to the ZBuffer can be problematic and cause draw order issues. Note that turning this off can mean that this material won't get properly accounted for when the MR system is performing late stage reprojection.  Not writing to the buffer can also be faster! .)
---
# [Material]({{site.url}}/Pages/Reference/Material.html).DepthWrite

<div class='signature' markdown='1'>
bool DepthWrite{ get set }
</div>

## Description
Should this material write to the ZBuffer? For opaque
objects, this generally should be true. But transparent objects
writing to the ZBuffer can be problematic and cause draw order
issues. Note that turning this off can mean that this material
won't get properly accounted for when the MR system is performing
late stage reprojection.

Not writing to the buffer can also be faster! :)

