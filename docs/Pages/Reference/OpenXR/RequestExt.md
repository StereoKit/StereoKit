---
layout: default
title: OpenXR.RequestExt
description: Requests that OpenXR load a particular extension. This MUST be called before SK.Initialize. Note that it's entirely possible that your extension will not load on certain runtimes, so be sure to check ExtEnabled to see if it's available to use.
---
# [OpenXR]({{site.url}}/Pages/Reference/OpenXR.html).RequestExt

<div class='signature' markdown='1'>
```csharp
static void RequestExt(string extensionName)
```
Requests that OpenXR load a particular extension. This
MUST be called before SK.Initialize. Note that it's entirely
possible that your extension will not load on certain runtimes,
so be sure to check ExtEnabled to see if it's available to use.
</div>

|  |  |
|--|--|
|string extensionName|The extension name as listed in the             OpenXR spec. For example: "XR_EXT_hand_tracking".|




