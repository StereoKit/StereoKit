---
layout: default
title: Platform.ReadFileBytes
description: Reads the entire contents of the file as a byte array, taking advantage of any permissions that may have been granted by Platform.FilePicker. Returns null on failure.
---
# [Platform]({{site.url}}/Pages/Reference/Platform.html).ReadFileBytes

<div class='signature' markdown='1'>
static Byte[] ReadFileBytes(string filename)
</div>

|  |  |
|--|--|
|string filename|Path to the file. Not affected by Assets             folder path.|
|RETURNS: Byte[]|A byte array if successful, null if not.|

Reads the entire contents of the file as a byte array,
taking advantage of any permissions that may have been granted by
Platform.FilePicker. Returns null on failure.



