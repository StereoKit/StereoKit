using StereoKit;
using System;
using System.Collections.Generic;

class DemoAssets : ITest
{
	Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	string description = "If you need to take a peek at what's currently loaded, StereoKit has a couple tools in the Assets class!\n\nThis demo is just a quick illustration of how to enumerate through your Assets.";
	Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
	string title       = "Asset Enumeration";

	/// :CodeSample: Assets Assets.Type
	/// ### Simple Asset Browser
	/// A full asset browser might have a few more features, but here's a quick
	/// and dirty window that will provide a filtered list of the current
	/// live assets!
	///
	/// ![An overly simple asset browser window]({{site.screen_url}}/TinyAssetBrowser.jpg)
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
		
		/// :CodeSample: Assets Assets.All
		/// ### Enumerating all Assets
		/// With Assets.All, you can take a peek at all the currently loaded
		/// Assets! Here's a quick example of iterating through all assets and
		/// dumping a quick summary to the log.
		foreach (var asset in Assets.All)
			Log.Info($"{asset.GetType().Name} - {asset.Id}");
		/// :End:

		Tests.RunForFrames(2);
	}

	public void Shutdown() { }

	public void Update()
	{
		AssetWindow();

		Tests.Screenshot("TinyAssetBrowser.jpg", 1, 400, 400, 90, filterWindow.position + V.XYZ(-0.16f, -0.16f, 0.16f), filterWindow.position - V.XYZ(0, 0.16f, 0));

		Text.Add(title,       titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
	}
}
