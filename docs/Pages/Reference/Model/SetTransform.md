---
layout: default
title: Model.SetTransform
description: Changes the transform for the subset to a new one! This is in Model space, so it's relative to the origin of the model.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).SetTransform

<div class='signature' markdown='1'>
```csharp
void SetTransform(int subsetIndex, Matrix& transform)
```
Changes the transform for the subset to a new one! This
is in Model space, so it's relative to the origin of the model.
</div>

|  |  |
|--|--|
|int subsetIndex|Index of the transform to replace,             should be less than SubsetCount.|
|Matrix& transform|The new transform.|




