---
layout: default
title: Log.Warn
description: Writes a formatted line to the log using a LogLevel.Warn severity level!
---
# [Log]({{site.url}}/Pages/Reference/Log.html).Warn

<div class='signature' markdown='1'>
```csharp
static void Warn(string text, Object[] items)
```
Writes a formatted line to the log using a LogLevel.Warn
severity level!
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for guidance on color tags.|
|Object[] items|Format arguments.|

<div class='signature' markdown='1'>
```csharp
static void Warn(string text)
```
Writes a formatted line to the log using a LogLevel.Warn
severity level!
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for guidance on color tags.|





## Examples

```csharp
Log.Warn("Warning! <~ylw>{0:0.0}s<~clr> have elapsed since StereoKit start!", Time.Total);
```

