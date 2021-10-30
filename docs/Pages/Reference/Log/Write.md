---
layout: default
title: Log.Write
description: Writes a formatted line to the log with the specified severity level!
---
# [Log]({{site.url}}/Pages/Reference/Log.html).Write

<div class='signature' markdown='1'>
static void Write([LogLevel]({{site.url}}/Pages/Reference/LogLevel.html) level, string text, Object[] items)
</div>

|  |  |
|--|--|
|[LogLevel]({{site.url}}/Pages/Reference/LogLevel.html) level|Severity level of this log message.|
|string text|Formatted text with color tags! See the Log             class docs for for guidance on color tags.|
|Object[] items|Format arguments.|

Writes a formatted line to the log with the specified
severity level!
<div class='signature' markdown='1'>
static void Write([LogLevel]({{site.url}}/Pages/Reference/LogLevel.html) level, string text)
</div>

|  |  |
|--|--|
|[LogLevel]({{site.url}}/Pages/Reference/LogLevel.html) level|Severity level of this log message.|
|string text|Formatted text with color tags! See the Log             class docs for for guidance on color tags.|

Writes a formatted line to the log with the specified
severity level!




## Examples

```csharp
Log.Write(LogLevel.Info, "<~grn>{0:0.0}s<~clr> have elapsed since StereoKit start.", Time.Total);
```

