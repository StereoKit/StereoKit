---
layout: default
title: OpenXR.GetFunction
description: This is basically xrGetInstanceProcAddr from OpenXR, you can use this to get and call functions from an extension you've loaded. This uses Marshal.GetDelegateForFunctionPointer to turn the result into a delegate that you can call.
---
# [OpenXR]({{site.url}}/Pages/Reference/OpenXR.html).GetFunction

<div class='signature' markdown='1'>
```csharp
static TDelegate GetFunction(string functionName)
```
This is basically `xrGetInstanceProcAddr` from OpenXR,
you can use this to get and call functions from an extension
you've loaded. This uses `Marshal.GetDelegateForFunctionPointer`
to turn the result into a delegate that you can call.
</div>

|  |  |
|--|--|
|string functionName||
|RETURNS: TDelegate|A delegate, or null on failure.|




