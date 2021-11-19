---
layout: default
title: Platform.ReadFile
description: Reads the entire contents of the file as a UTF-8 string, taking advantage of any permissions that may have been granted by Platform.FilePicker.
---
# [Platform]({{site.url}}/Pages/Reference/Platform.html).ReadFile

<div class='signature' markdown='1'>
```csharp
static bool ReadFile(string filename, String& data)
```
Reads the entire contents of the file as a UTF-8 string,
taking advantage of any permissions that may have been granted by
Platform.FilePicker.
</div>

|  |  |
|--|--|
|string filename|Path to the file. Not affected by Assets             folder path.|
|String& data|A UTF-8 encoded string representing the             contents of the file. Will be null on failure.|
|RETURNS: bool|True on success, False on failure.|

<div class='signature' markdown='1'>
```csharp
static bool ReadFile(string filename, Byte[]& data)
```
Reads the entire contents of the file as a byte array,
taking advantage of any permissions that may have been granted by
Platform.FilePicker.
</div>

|  |  |
|--|--|
|string filename|Path to the file. Not affected by Assets             folder path.|
|Byte[]& data|A UTF-8 encoded string representing the             contents of the file. Will be null on failure.|
|RETURNS: bool|True on success, False on failure.|





## Examples

### Read Custom Files
```csharp
Platform.FilePicker(PickerMode.Open, file => {
	// On some platforms (like UWP), you may encounter permission
	// issues when trying to read or write to an arbitrary file.
	//
	// StereoKit's `Platform.FilePicker` and `Platform.ReadFile`
	// work together to avoid this permission issue, where the
	// FilePicker will grant permission to the ReadFile method.
	// C#'s built-in `File.ReadAllText` would fail on UWP here.
	if (Platform.ReadFile(file, out string text))
		Log.Info(text);
}, null, ".txt");
```

