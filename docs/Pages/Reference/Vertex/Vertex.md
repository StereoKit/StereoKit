---
layout: default
title: Vertex.Vertex
description: Create a new Vertex, use the overloads to take advantage of default values. Vertex color defaults to White. UV defaults to (0,0).
---
# [Vertex]({{site.url}}/Pages/Reference/Vertex.html).Vertex

<div class='signature' markdown='1'>
```csharp
void Vertex(Vec3 position, Vec3 normal)
```
Create a new Vertex, use the overloads to take advantage
of default values. Vertex color defaults to White. UV defaults to
(0,0).
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) position|Location of the Vertex, this is typically             meters in Model space.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) normal|The direction the Vertex is facing. Never             leave this as zero, or your lighting may turn out black! A good             default value if you _don't_ know what to put here is (0,1,0),             but a Mesh composed entirely of this value will have flat             lighting.|

<div class='signature' markdown='1'>
```csharp
void Vertex(Vec3 position, Vec3 normal, Vec2 textureCoordinates)
```
Create a new Vertex, use the overloads to take advantage
of default values. Vertex color defaults to White.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) position|Location of the Vertex, this is typically             meters in Model space.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) normal|The direction the Vertex is facing. Never             leave this as zero, or your lighting may turn out black! A good             default value if you _don't_ know what to put here is (0,1,0),             but a Mesh composed entirely of this value will have flat             lighting.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) textureCoordinates|What part of a texture is this             Vertex anchored to? (0,0) is top left of the texture, and (1,1)             is the bottom right.|

<div class='signature' markdown='1'>
```csharp
void Vertex(Vec3 position, Vec3 normal, Vec2 textureCoordinates, Color32 color)
```
Create a new Vertex, use the overloads to take advantage
of default values.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) position|Location of the Vertex, this is typically             meters in Model space.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) normal|The direction the Vertex is facing. Never             leave this as zero, or your lighting may turn out black! A good             default value if you _don't_ know what to put here is (0,1,0),             but a Mesh composed entirely of this value will have flat             lighting.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) textureCoordinates|What part of a texture is this             Vertex anchored to? (0,0) is top left of the texture, and (1,1)             is the bottom right.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) color|The color of the Vertex, StereoKit's default             shaders treat this as a multiplicative modifier for the             Material's albedo/diffuse color, but different shaders sometimes             treat this value differently. A good default here is white, black             will cause your model to turn out completely black.|




