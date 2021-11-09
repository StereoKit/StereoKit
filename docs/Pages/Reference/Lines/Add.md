---
layout: default
title: Lines.Add
description: Adds a line to the environment for the current frame.
---
# [Lines]({{site.url}}/Pages/Reference/Lines.html).Add

<div class='signature' markdown='1'>
```csharp
static void Add(Vec3 start, Vec3 end, Color32 color, float thickness)
```
Adds a line to the environment for the current frame.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) start|Starting point of the line.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) end|End point of the line.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) color|Color for the line, this is embedded in the             vertex color of the line.|
|float thickness|Thickness of the line in meters.|

<div class='signature' markdown='1'>
```csharp
static void Add(Vec3 start, Vec3 end, Color32 colorStart, Color32 colorEnd, float thickness)
```
Adds a line to the environment for the current frame.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) start|Starting point of the line.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) end|End point of the line.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) colorStart|Color for the start of the line, this is             embedded in the vertex color of the line.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) colorEnd|Color for the end of the line, this is             embedded in the vertex color of the line.|
|float thickness|Thickness of the line in meters.|

<div class='signature' markdown='1'>
```csharp
static void Add(Ray ray, float length, Color32 color, float thickness)
```
Adds a line based on a ray to the environment for the
current frame.
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) ray|The ray we want to visualize!|
|float length|How long should the ray be? Actual length             will be ray.direction.Magnitude * length.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) color|Color for the line, this is embedded in the             vertex color of the line.|
|float thickness|Thickness of the line in meters.|

<div class='signature' markdown='1'>
```csharp
static void Add(LinePoint[]& points)
```
Adds a line from a list of line points to the
environment. This does not close the path, so if you want it
closed, you'll have to add an extra point or two at the end
yourself!
</div>

|  |  |
|--|--|
|LinePoint[]& points|An array of line points.|





## Examples

```csharp
Lines.Add(new Vec3(0.1f,0,0), new Vec3(-0.1f,0,0), Color.White, 0.01f);
```
```csharp
Lines.Add(new Vec3(0.1f,0,0), new Vec3(-0.1f,0,0), Color.White, Color.Black, 0.01f);
```
```csharp
Lines.Add(new LinePoint[]{ 
	new LinePoint(new Vec3( 0.1f, 0,     0), Color.White, 0.01f),
	new LinePoint(new Vec3( 0,    0.02f, 0), Color.Black, 0.005f),
	new LinePoint(new Vec3(-0.1f, 0,     0), Color.White, 0.01f),
});
```

