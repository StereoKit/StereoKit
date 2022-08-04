---
layout: default
title: Time
description: This class contains time information for the current session and frame!
---
# static class Time

This class contains time information for the current session and frame!

## Static Fields and Properties

|  |  |
|--|--|
|double [Elapsed]({{site.url}}/Pages/Reference/Time/Elapsed.html)|How much time elapsed during the last frame? 64 bit time precision.|
|float [Elapsedf]({{site.url}}/Pages/Reference/Time/Elapsedf.html)|How much time elapsed during the last frame? 32 bit time precision.|
|double [ElapsedUnscaled]({{site.url}}/Pages/Reference/Time/ElapsedUnscaled.html)|How much time elapsed during the last frame? 64 bit time precision. This version is unaffected by the Time.Scale value!|
|float [ElapsedUnscaledf]({{site.url}}/Pages/Reference/Time/ElapsedUnscaledf.html)|How much time elapsed during the last frame? 32 bit time precision. This version is unaffected by the Time.Scale value!|
|double [Scale]({{site.url}}/Pages/Reference/Time/Scale.html)|Time is scaled by this value! Want time to pass slower? Set it to 0.5! Faster? Try 2!|
|double [Total]({{site.url}}/Pages/Reference/Time/Total.html)|How much time has elapsed since StereoKit was initialized? 64 bit time precision.|
|float [Totalf]({{site.url}}/Pages/Reference/Time/Totalf.html)|How much time has elapsed since StereoKit was initialized? 32 bit time precision.|
|double [TotalUnscaled]({{site.url}}/Pages/Reference/Time/TotalUnscaled.html)|How much time has elapsed since StereoKit was initialized? 64 bit time precision. This version is unaffected by the Time.Scale value!|
|float [TotalUnscaledf]({{site.url}}/Pages/Reference/Time/TotalUnscaledf.html)|How much time has elapsed since StereoKit was initialized? 32 bit time precision. This version is unaffected by the Time.Scale value!|

## Static Methods

|  |  |
|--|--|
|[SetTime]({{site.url}}/Pages/Reference/Time/SetTime.html)|This allows you to override the application time! The application will progress from this time using the current timescale.|
