---
layout: default
title: FilePicker.Show
description: Show a file picker to the user! If one is already up, it'll be cancelled out, and this one will replace it.
---
# [FilePicker]({{site.url}}/Pages/Reference/FilePicker.html).Show

<div class='signature' markdown='1'>
static void Show(FilePickerMode mode, Action`1 onSelectFile, FileFilter[] filters)
</div>

|  |  |
|--|--|
|FilePickerMode mode|For opening files, or for saving them?|
|Action`1 onSelectFile|The function to call when the user has selected a file.|
|FileFilter[] filters|What file types should show up in the picker?|

Show a file picker to the user! If one is already up, it'll be cancelled out,
and this one will replace it.
<div class='signature' markdown='1'>
static void Show(FilePickerMode mode, Action`1 onSelectFile, Action onCancel, FileFilter[] filters)
</div>

|  |  |
|--|--|
|FilePickerMode mode|For opening files, or for saving them?|
|Action`1 onSelectFile|The function to call when the user has selected a file.|
|Action onCancel|If the file selection has been cancelled, this'll get called!|
|FileFilter[] filters|What file types should show up in the picker?|

Show a file picker to the user! If one is already up, it'll be cancelled out,
and this one will replace it.
<div class='signature' markdown='1'>
static void Show(FilePickerMode mode, string initialFolder, Action`1 onSelectFile, FileFilter[] filters)
</div>

|  |  |
|--|--|
|FilePickerMode mode|For opening files, or for saving them?|
|string initialFolder|The starting folder. By default (or null), this'll just be             the working directory.|
|Action`1 onSelectFile|The function to call when the user has selected a file.|
|FileFilter[] filters|What file types should show up in the picker?|

Show a file picker to the user! If one is already up, it'll be cancelled out,
and this one will replace it.
<div class='signature' markdown='1'>
static void Show(FilePickerMode mode, string initialFolder, Action`1 onSelectFile, Action onCancel, FileFilter[] filters)
</div>

|  |  |
|--|--|
|FilePickerMode mode|For opening files, or for saving them?|
|string initialFolder|The starting folder. By default (or null), this'll just be             the working directory.|
|Action`1 onSelectFile|The function to call when the user has selected a file.|
|Action onCancel|If the file selection has been cancelled, this'll get called!|
|FileFilter[] filters|What file types should show up in the picker?|

Show a file picker to the user! If one is already up, it'll be cancelled out,
and this one will replace it.



