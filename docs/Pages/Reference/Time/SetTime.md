---
layout: default
title: Time.SetTime
description: This allows you to override the application time! The application will progress from this time using the current timescale.
---
# [Time]({{site.url}}/Pages/Reference/Time.html).SetTime

<div class='signature' markdown='1'>
```csharp
static void SetTime(double totalSeconds, double frameElapsedSeconds)
```
This allows you to override the application time! The application
will progress from this time using the current timescale.
</div>

|  |  |
|--|--|
|double totalSeconds|What time should it now be? The app will progress from this point in time.|
|double frameElapsedSeconds|How long was the previous frame? This is a number often used             in motion calculations. If left to zero, it'll use the previous frame's time, and if the previous             frame's time was also zero, it'll use 1/90.|




