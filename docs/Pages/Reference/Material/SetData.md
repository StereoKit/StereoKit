---
layout: default
title: Material.SetData
description: This allows you to set more complex shader data types such as structs. Note the SK doesn't guard against setting data of the wrong size here, so pay extra attention to the size of your data here, and ensure it matched up with the shader!
---
# [Material]({{site.url}}/Pages/Reference/Material.html).SetData

<div class='signature' markdown='1'>
```csharp
void SetData(string name, T& serializableData)
```
This allows you to set more complex shader data types such
as structs. Note the SK doesn't guard against setting data of the
wrong size here, so pay extra attention to the size of your data
here, and ensure it matched up with the shader!
</div>

|  |  |
|--|--|
|string name|Name of the shader parameter.|
|T& serializableData|New value for the parameter.|




