---
layout: default
title: Platform
description: This class provides some platform related code that runs cross-platform. You might be able to do many of these things with C#, but you might not be able to do them in as portable a manner as these methods do!
---
# static class Platform

This class provides some platform related code that runs
cross-platform. You might be able to do many of these things with C#,
but you might not be able to do them in as portable a manner as these
methods do!

## Static Fields and Properties

|  |  |
|--|--|
|bool [FilePickerVisible]({{site.url}}/Pages/Reference/Platform/FilePickerVisible.html)|This will check if the file picker interface is currently visible. Some pickers will never show this, as they block the application until the picker has completed.|
|bool [ForceFallbackKeyboard]({{site.url}}/Pages/Reference/Platform/ForceFallbackKeyboard.html)|Force the use of StereoKit's built-in fallback keyboard instead of the system keyboard. This may be great for testing or look and feel matching, but the system keyboard should generally be preferred for accessibility reasons.|
|bool [KeyboardVisible]({{site.url}}/Pages/Reference/Platform/KeyboardVisible.html)|Check if a soft keyboard is currently visible. This may be an OS provided keyboard or StereoKit's fallback keyboard, but will not indicate the presence of a physical keyboard.|

## Static Methods

|  |  |
|--|--|
|[FilePicker]({{site.url}}/Pages/Reference/Platform/FilePicker.html)|Starts a file picker window! This will create a native file picker window if one is available in the current setup, and if it is not, it'll create a fallback filepicker build using StereoKit's UI.  Flatscreen apps will show traditional file pickers, and UWP has an OS provided file picker that works in MR. All others currently use the fallback system.  A note for UWP apps, UWP generally does not have permission to access random files, unless the user has chosen them with the picker! This picker properly handles permissions for individual files on UWP, but may have issues with files that reference other files, such as .gltf files with external textures. See Platform.WriteFile and Platform.ReadFile for manually reading and writing files in a cross-platfom manner.|
|[FilePickerClose]({{site.url}}/Pages/Reference/Platform/FilePickerClose.html)|If the picker is visible, this will close it and immediately trigger a cancel event for the active picker.|
|[KeyboardShow]({{site.url}}/Pages/Reference/Platform/KeyboardShow.html)|Request or hide a soft keyboard for the user to type on. StereoKit will surface OS provided soft keyboards where available, and use a fallback keyboard when not. On systems with physical keyboards, soft keyboards generally will not be shown if the user has interacted with their physical keyboard recently.|
|[ReadFile]({{site.url}}/Pages/Reference/Platform/ReadFile.html)|Reads the entire contents of the file as a UTF-8 string, taking advantage of any permissions that may have been granted by Platform.FilePicker.|
|[ReadFileBytes]({{site.url}}/Pages/Reference/Platform/ReadFileBytes.html)|Reads the entire contents of the file as a byte array, taking advantage of any permissions that may have been granted by Platform.FilePicker. Returns null on failure.|
|[ReadFileText]({{site.url}}/Pages/Reference/Platform/ReadFileText.html)|Reads the entire contents of the file as a UTF-8 string, taking advantage of any permissions that may have been granted by Platform.FilePicker. Returns null on failure.|
|[WriteFile]({{site.url}}/Pages/Reference/Platform/WriteFile.html)|Writes a UTF-8 text file to the filesystem, taking advantage of any permissions that may have been granted by Platform.FilePicker.|
