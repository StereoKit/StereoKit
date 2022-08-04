---
layout: default
title: AssetState
description: StereoKit uses an asynchronous loading system to prevent assets from blocking execution! This means that asset loading systems will return an asset to you right away, even though it is still being processed in the background.
---
# enum AssetState

StereoKit uses an asynchronous loading system to prevent assets from
blocking execution! This means that asset loading systems will return
an asset to you right away, even though it is still being processed
in the background.

## Enum Values

|  |  |
|--|--|
|Error|An unknown error occurred when trying to load the asset! Check the logs for additional details.|
|ErrorNotFound|The asset data was not found! This is most likely an issue with a bad file path, or file permissions. Check the logs for additional details.|
|ErrorUnsupported|This asset encountered an issue when parsing the source data. Either the format is unrecognized by StereoKit, or the data may be corrupt. Check the logs for additional details.|
|Loaded|This asset is completely loaded without issues, and is ready for use!|
|LoadedMeta|This asset is still loading, but some of the higher level data is already available for inspection without blocking the app. Attempting to access the core asset data will result in blocking the app's execution until that data is loaded!|
|Loading|This asset is currently queued for loading, but hasn't received any data yet. Attempting to access metadata or asset data will result in blocking the app's execution until that data is loaded!|
|None|This asset is in its default state. It has not been told to load anything, nor does it have any data!|
