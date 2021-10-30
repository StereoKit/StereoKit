---
layout: default
title: Log.Err
description: Writes a formatted line to the log using a LogLevel.Error severity level!
---
# [Log]({{site.url}}/Pages/Reference/Log.html).Err

<div class='signature' markdown='1'>
static void Err(string text, Object[] items)
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for for guidance on color tags.|
|Object[] items|Format arguments.|

Writes a formatted line to the log using a
LogLevel.Error severity level!
<div class='signature' markdown='1'>
static void Err(string text)
</div>

|  |  |
|--|--|
|string text|Formatted text with color tags! See the Log             class docs for for guidance on color tags.|

Writes a formatted line to the log using a
LogLevel.Error severity level!




## Examples

```csharp
if (Time.Elapsedf > 0.017f)
	Log.Err("Oh no! Frame time (<~red>{0}<~clr>) has exceeded 17ms! There's no way we'll hit even 60 frames per second!", Time.Elapsedf);
```

