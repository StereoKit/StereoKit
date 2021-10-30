---
layout: default
title: Log.Info
description: Writes a formatted line to the log using a LogLevel.Info severity level!
---
# [Log]({{site.url}}/Pages/Reference/Log.html).Info

<div class='signature' markdown='1'>
static void Info(string text, Object[] items)
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for for guidance on color tags.|
|Object[] items|Format arguments.|

Writes a formatted line to the log using a LogLevel.Info
severity level!
<div class='signature' markdown='1'>
static void Info(string text)
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for for guidance on color tags.|

Writes a formatted line to the log using a LogLevel.Info
severity level!




## Examples

```csharp
Log.Info("<~grn>{0:0.0}s<~clr> have elapsed since StereoKit start.", Time.Total);
```

