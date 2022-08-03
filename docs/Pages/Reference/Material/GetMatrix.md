---
layout: default
title: Material.GetMatrix
description: Gets the value of a shader parameter with the given name. If no parameter is found, a default value of Matrix.Identity will be returned.
---
# [Material]({{site.url}}/Pages/Reference/Material.html).GetMatrix

<div class='signature' markdown='1'>
```csharp
Matrix GetMatrix(string name)
```
Gets the value of a shader parameter with the given name.
If no parameter is found, a default value of Matrix.Identity will
be returned.
</div>

|  |  |
|--|--|
|string name|Name of the shader parameter.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|The parameter's value, if present, otherwise Matrix.Identity.|




