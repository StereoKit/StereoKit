---
layout: default
title: Log.Write
description: Writes a formatted line to the log with the specified severity level!
---
# [Log]({{site.url}}/Pages/Reference/Log.html).Write

<div class='signature' markdown='1'>
```csharp
static void Write(LogLevel level, string text, Object[] items)
```
Writes a formatted line to the log with the specified
severity level!
</div>

|  |  |
|--|--|
|[LogLevel]({{site.url}}/Pages/Reference/LogLevel.html) level|Severity level of this log message.|
|string text|Formatted text with color tags! See the Log             class docs for guidance on color tags.|
|Object[] items|Format arguments.|

<div class='signature' markdown='1'>
```csharp
static void Write(LogLevel level, string text)
```
Writes a formatted line to the log with the specified
severity level!
</div>

|  |  |
|--|--|
|[LogLevel]({{site.url}}/Pages/Reference/LogLevel.html) level|Severity level of this log message.|
|string text|Formatted text with color tags! See the Log             class docs for guidance on color tags.|





## Examples

```csharp
Log.Write(LogLevel.Info, "<~grn>{0:0.0}s<~clr> have elapsed since StereoKit start.", Time.Total);
```

