---
layout: default
title: Log.Err
description: Writes a formatted line to the log using a LogLevel.Error severity level!
---
# [Log]({{site.url}}/Pages/Reference/Log.html).Err

<div class='signature' markdown='1'>
```csharp
static void Err(string text, Object[] items)
```
Writes a formatted line to the log using a
LogLevel.Error severity level!
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for guidance on color tags.|
|Object[] items|Format arguments.|

<div class='signature' markdown='1'>
```csharp
static void Err(string text)
```
Writes a formatted line to the log using a
LogLevel.Error severity level!
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for guidance on color tags.|





## Examples

```csharp
if (Time.Elapsedf > 0.017f)
	Log.Err("Oh no! Frame time (<~red>{0}<~clr>) has exceeded 17ms! There's no way we'll hit even 60 frames per second!", Time.Elapsedf);
```

