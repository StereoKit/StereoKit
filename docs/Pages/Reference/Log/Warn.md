---
layout: default
title: Log.Warn
description: Writes a formatted line to the log using a LogLevel.Warn severity level!
---
# [Log]({{site.url}}/Pages/Reference/Log.html).Warn

<div class='signature' markdown='1'>
static void Warn(string text, Object[] items)
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for for guidance on color tags.|
|Object[] items|Format arguments.|

Writes a formatted line to the log using a LogLevel.Warn
severity level!
<div class='signature' markdown='1'>
static void Warn(string text)
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for for guidance on color tags.|

Writes a formatted line to the log using a LogLevel.Warn
severity level!




## Examples

```csharp
Log.Warn("Warning! <~ylw>{0:0.0}s<~clr> have elapsed since StereoKit start!", Time.Total);
```

