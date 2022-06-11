---
layout: default
title: Mesh.GetTriangle
description: Retrieves the vertices associated with a particular triangle on the Mesh.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).GetTriangle

<div class='signature' markdown='1'>
```csharp
bool GetTriangle(uint triangleIndex, Vertex& a, Vertex& b, Vertex& c)
```
Retrieves the vertices associated with a particular
triangle on the Mesh.
</div>

|  |  |
|--|--|
|uint triangleIndex|Starting index of the triangle, should             be a multiple of 3.|
|Vertex& a|The first vertex of the found triangle|
|Vertex& b|The second vertex of the found triangle|
|Vertex& c|The third vertex of the found triangle|
|RETURNS: bool|Returns true if triangle index was valid|




