---
layout: default
title: Platform.FilePicker
description: Starts a file picker window! This will create a native file picker window if one is available in the current setup, and if it is not, it'll create a fallback filepicker build using StereoKit's UI.  Flatscreen apps will show traditional file pickers, and UWP has an OS provided file picker that works in MR. All others currently use the fallback system.  A note for UWP apps, UWP generally does not have permission to access random files, unless the user has chosen them with the picker! This picker properly handles permissions for individual files on UWP, but may have issues with files that reference other files, such as .gltf files with external textures. See Platform.WriteFile and Platform.ReadFile for manually reading and writing files in a cross-platfom manner.
---
# [Platform]({{site.url}}/Pages/Reference/Platform.html).FilePicker

<div class='signature' markdown='1'>
```csharp
static void FilePicker(PickerMode mode, Action`1 onSelectFile, Action onCancel, String[] filters)
```
Starts a file picker window! This will create a native
file picker window if one is available in the current setup, and
if it is not, it'll create a fallback filepicker build using
StereoKit's UI.

Flatscreen apps will show traditional file pickers, and UWP has
an OS provided file picker that works in MR. All others currently
use the fallback system.

A note for UWP apps, UWP generally does not have permission to
access random files, unless the user has chosen them with the
picker! This picker properly handles permissions for individual
files on UWP, but may have issues with files that reference other
files, such as .gltf files with external textures. See
Platform.WriteFile and Platform.ReadFile for manually reading and
writing files in a cross-platfom manner.
</div>

|  |  |
|--|--|
|[PickerMode]({{site.url}}/Pages/Reference/PickerMode.html) mode|Are we trying to Open a file, or Save a file?             This changes the appearance and behavior of the picker to support             the specified action.|
|Action`1 onSelectFile|This Action will be called with the             proper filename when the picker has successfully completed! On a             cancel or close event, this Action is not called.|
|Action onCancel|If the user cancels the file picker, or              the picker is closed via FilePickerClose, this Action is called.|
|String[] filters|A list of file extensions that the picker             should filter for. This is in the format of ".glb" and is case             insensitive.|

<div class='signature' markdown='1'>
```csharp
static void FilePicker(PickerMode mode, Action`2 onComplete, String[] filters)
```
Starts a file picker window! This will create a native
file picker window if one is available in the current setup, and
if it is not, it'll create a fallback filepicker build using
StereoKit's UI.

Flatscreen apps will show traditional file pickers, and UWP has
an OS provided file picker that works in MR. All others currently
use the fallback system. Some pickers will block the system and
return right away, but others will stick around and let users
continue to interact with the app.

A note for UWP apps, UWP generally does not have permission to
access random files, unless the user has chosen them with the
picker! This picker properly handles permissions for individual
files on UWP, but may have issues with files that reference other
files, such as .gltf files with external textures. See
Platform.WriteFile and Platform.ReadFile for manually reading and
writing files in a cross-platfom manner.
</div>

|  |  |
|--|--|
|[PickerMode]({{site.url}}/Pages/Reference/PickerMode.html) mode|Are we trying to Open a file, or Save a file?             This changes the appearance and behavior of the picker to support             the specified action.|
|Action`2 onComplete|This action will be called when the file             picker has finished, either via a cancel event, or from a confirm             event. First parameter is a bool, where true indicates the              presence of a valid filename, and false indicates a failure or              cancel event.|
|String[] filters|A list of file extensions that the picker             should filter for. This is in the format of ".glb" and is case             insensitive.|





## Examples

### Opening a Model
This is a simple button that will open a 3D model selection
file picker, and make a call to OnLoadModel after a file has
been successfully picked!
```csharp
if (UI.Button("Open Model") && !Platform.FilePickerVisible) {
	Platform.FilePicker(PickerMode.Open, OnLoadModel, null, Assets.ModelFormats);
}
```
Once you have the filename, it's simply a matter of loading it
from file. This is an example of async loading a model, and
calculating a scale value that ensures the model is a reasonable
size.
```csharp
private void OnLoadModel(string filename)
{
	model      = Model.FromFile(filename);
	modelTask  = Assets.CurrentTask;
	modelScale = 1 / model.Bounds.dimensions.Magnitude;
	if (model.Anims.Count > 0)
		model.PlayAnim(model.Anims[0], AnimMode.Loop);
}
```
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
### Write Custom Files
```csharp
Platform.FilePicker(PickerMode.Save, file => {
	// On some platforms (like UWP), you may encounter permission
	// issues when trying to read or write to an arbitrary file.
	//
	// StereoKit's `Platform.FilePicker` and `Platform.WriteFile`
	// work together to avoid this permission issue, where the
	// FilePicker will grant permission to the WriteFile method.
	// C#'s built-in `File.WriteAllText` would fail on UWP here.
	Platform.WriteFile(file, "Text for the file.\n- Thanks!");
}, null, ".txt");
```

