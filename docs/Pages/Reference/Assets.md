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

## Examples

### Simple Asset Browser
A full asset browser might have a few more features, but here's a quick
and dirty window that will provide a filtered list of the current
live assets!

![An overly simple asset browser window]({{site.screen_url}}/TinyAssetBrowser.jpg)
```csharp
List<IAsset> filteredAssets = new List<IAsset>();
Type         filterType     = typeof(IAsset);
Pose         filterWindow   = new Pose(0.5f, 0, -0.4f, Quat.LookDir(-1, 0, 1));

void UpdateFilter(Type type)
{
	filterType = type;
	filteredAssets.Clear();
	
	// Here's where the magic happens! `Assets.Type` can take a Type, or a
	// generic <T>, and will give a list of all assets that match that
	// type!
	filteredAssets.AddRange(Assets.Type(filterType));
}

public void AssetWindow()
{
	UI.WindowBegin("Asset Browser", ref filterWindow, V.XY(0.3f,0));

	UI.Label("Filter:");

	// A radio button selection for what to filter by
	if (UI.Radio("All",      filterType == typeof(IAsset  ))) UpdateFilter(typeof(IAsset));
	UI.SameLine();
	if (UI.Radio("Font",     filterType == typeof(Font    ))) UpdateFilter(typeof(Font));
	UI.SameLine();
	if (UI.Radio("Material", filterType == typeof(Material))) UpdateFilter(typeof(Material));
	UI.SameLine();
	if (UI.Radio("Mesh",     filterType == typeof(Mesh    ))) UpdateFilter(typeof(Mesh));
	UI.SameLine();
	if (UI.Radio("Model",    filterType == typeof(Model   ))) UpdateFilter(typeof(Model));
	UI.SameLine();
	if (UI.Radio("Shader",   filterType == typeof(Shader  ))) UpdateFilter(typeof(Shader));
	UI.SameLine();
	if (UI.Radio("Solid",    filterType == typeof(Solid   ))) UpdateFilter(typeof(Solid));
	UI.SameLine();
	if (UI.Radio("Sound",    filterType == typeof(Sound   ))) UpdateFilter(typeof(Sound));
	UI.SameLine();
	if (UI.Radio("Sprite",   filterType == typeof(Sprite  ))) UpdateFilter(typeof(Sprite));
	UI.SameLine();
	if (UI.Radio("Tex",      filterType == typeof(Tex     ))) UpdateFilter(typeof(Tex));

	UI.HSeparator();

	// Not a complicated visualization here, we're just showing whatever
	// text based id was provided for the asset! Some Ids may just be null.
	foreach(var asset in filteredAssets)
		if (!string.IsNullOrEmpty(asset.Id))
			UI.Label(asset.Id);
	
	UI.WindowEnd();
}

public void Initialize()
{
	UpdateFilter(typeof(Sound));
	
```
:CodeSample: Assets Assets.All
### Enumerating all Assets
With Assets.All, you can take a peek at all the currently loaded
Assets! Here's a quick example of iterating through all assets and
dumping a quick summary to the log.
```csharp
foreach (var asset in Assets.All)
	Log.Info($"{asset.GetType().Name} - {asset.Id}");
```

