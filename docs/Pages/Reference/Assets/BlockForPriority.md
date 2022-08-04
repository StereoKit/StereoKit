---
layout: default
title: Assets.BlockForPriority
description: This will block the execution of the application until all asset tasks below the priority value have completed loading. To block until all assets are loaded, pass in int.MaxValue for the priority.
---
# [Assets]({{site.url}}/Pages/Reference/Assets.html).BlockForPriority

<div class='signature' markdown='1'>
```csharp
static void BlockForPriority(int priority)
```
This will block the execution of the application until
all asset tasks below the priority value have completed loading.
To block until all assets are loaded, pass in int.MaxValue for the
priority.
</div>

|  |  |
|--|--|
|int priority|Block the app until this priority level is             complete.|




