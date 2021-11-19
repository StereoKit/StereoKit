---
layout: default
title: Material.SetColor
description: Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!
---
# [Material]({{site.url}}/Pages/Reference/Material.html).SetColor

<div class='signature' markdown='1'>
```csharp
void SetColor(string name, Color32 colorGamma)
```
Sets a shader parameter with the given name to the
provided value. If no parameter is found, nothing happens, and
the value is not set!
</div>

|  |  |
|--|--|
|string name|Name of the shader parameter.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) colorGamma|The gamma space color for the shader             to use.|

<div class='signature' markdown='1'>
```csharp
void SetColor(string name, Color colorGamma)
```
Sets a shader parameter with the given name to the
provided value. If no parameter is found, nothing happens, and
the value is not set!
</div>

|  |  |
|--|--|
|string name|Name of the shader parameter.|
|[Color]({{site.url}}/Pages/Reference/Color.html) colorGamma|The gamma space color for the shader             to use.|




