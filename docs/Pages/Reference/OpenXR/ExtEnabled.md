---
layout: default
title: OpenXR.ExtEnabled
description: This tells if an OpenXR extension has been requested and successfully loaded by the runtime. This MUST only be called after SK.Initialize.
---
# [OpenXR]({{site.url}}/Pages/Reference/OpenXR.html).ExtEnabled

<div class='signature' markdown='1'>
```csharp
static bool ExtEnabled(string extensionName)
```
This tells if an OpenXR extension has been requested
and successfully loaded by the runtime. This MUST only be
called after SK.Initialize.
</div>

|  |  |
|--|--|
|string extensionName|The extension name as listed in the             OpenXR spec. For example: "XR_EXT_hand_tracking".|
|RETURNS: bool|If the extension is available to use.|




