---
layout: default
title: Platform.ReadFileText
description: Reads the entire contents of the file as a UTF-8 string, taking advantage of any permissions that may have been granted by Platform.FilePicker. Returns null on failure.
---
# [Platform]({{site.url}}/Pages/Reference/Platform.html).ReadFileText

<div class='signature' markdown='1'>
```csharp
static string ReadFileText(string filename)
```
Reads the entire contents of the file as a UTF-8 string,
taking advantage of any permissions that may have been granted by
Platform.FilePicker. Returns null on failure.
</div>

|  |  |
|--|--|
|string filename|Path to the file. Not affected by Assets             folder path.|
|RETURNS: string|A UTF-8 encoded string if successful, null if not.|




