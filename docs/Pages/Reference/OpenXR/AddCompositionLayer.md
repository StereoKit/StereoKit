---
layout: default
title: OpenXR.AddCompositionLayer
description: This allows you to add XrCompositionLayers to the list that StereoKit submits to xrEndFrame. You must call this every frame you wish the layer to be included.
---
# [OpenXR]({{site.url}}/Pages/Reference/OpenXR.html).AddCompositionLayer

<div class='signature' markdown='1'>
```csharp
static void AddCompositionLayer(T XrCompositionLayerX, int sortOrder)
```
This allows you to add XrCompositionLayers to the list
that StereoKit submits to xrEndFrame. You must call this every
frame you wish the layer to be included.
</div>

|  |  |
|--|--|
|T XrCompositionLayerX|A serializable             XrCompositionLayer struct that follows the             XrCompositionLayerBaseHeader data pattern.|
|int sortOrder|An sort order value for sorting with             other composition layers in the list. The primary projection             layer that StereoKit renders to is at 0, -1 would be before it,             and +1 would be after.|




