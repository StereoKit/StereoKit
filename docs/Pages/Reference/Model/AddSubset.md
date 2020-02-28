---
layout: default
title: Model.AddSubset
description: Adds a new subset to the Model, and recalculates the bounds.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).AddSubset

<div class='signature' markdown='1'>
int AddSubset([Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh, [Material]({{site.url}}/Pages/Reference/Material.html) material, Matrix& transform)
</div>

|  |  |
|--|--|
|[Mesh]({{site.url}}/Pages/Reference/Mesh.html) mesh|The Mesh for the subset, may not be null.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|The Material for the subset, may not be null.|
|Matrix& transform|A transform Matrix representing the Mesh's location             relative to the origin of the Model.|
|RETURNS: int|The index of the subset that was just added.|

Adds a new subset to the Model, and recalculates the bounds.




## Examples

```csharp
model.AddSubset(
    Mesh   .GenerateSphere(1),
    Default.Material,
    Matrix .T(0,1,0));
```

