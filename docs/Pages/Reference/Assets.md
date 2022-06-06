---
layout: default
title: Assets
description: If you want to manage loading assets, this is the class for you!
---
# static class Assets

If you want to manage loading assets, this is the class for
you!

## Static Fields and Properties

|  |  |
|--|--|
|int [CurrentTask]({{site.url}}/Pages/Reference/Assets/CurrentTask.html)|This is the index of the current asset loading task. Note that to load one asset, multiple tasks are generated.|
|int [CurrentTaskPriority]({{site.url}}/Pages/Reference/Assets/CurrentTaskPriority.html)|StereoKit processes tasks in order of priority. This returns the priority of the current task, and can be used to wait until all tasks within a certain priority range have been completed.|
|String[] [ModelFormats]({{site.url}}/Pages/Reference/Assets/ModelFormats.html)|A list of supported model format extensions. This pairs pretty well with `Platform.FilePicker` when attempting to load a `Model`!|
|String[] [TextureFormats]({{site.url}}/Pages/Reference/Assets/TextureFormats.html)|A list of supported texture format extensions. This pairs pretty well with `Platform.FilePicker` when attempting to load a `Tex`!|
|int [TotalTasks]({{site.url}}/Pages/Reference/Assets/TotalTasks.html)|This is the total number of tasks that have been added to the loading system, including all completed and pending tasks. Note that to load one asset, multiple tasks are generated.|

## Static Methods

|  |  |
|--|--|
|[BlockForPriority]({{site.url}}/Pages/Reference/Assets/BlockForPriority.html)|This will block the execution of the application until all asset tasks below the priority value have completed loading. To block until all assets are loaded, pass in int.MaxValue for the priority.|
