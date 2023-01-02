﻿using StereoKit;
using System;
using System.Collections.Generic;

class DemoAssets : ITest
{
	string title       = "Asset Enumeration";
	string description = "If you need to take a peek at what's currently loaded, StereoKit has a couple tools in the Assets class!\n\nThis demo is just a quick illustration of how to enumerate through your Assets.";

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
	int          filterScroll   = 0;
	const int    filterScrollCt = 12;

	void UpdateFilter(Type type)
	{
		filterType   = type;
		filterScroll = 0;
		filteredAssets.Clear();
		
		// Here's where the magic happens! `Assets.Type` can take a Type, or a
		// generic <T>, and will give a list of all assets that match that
		// type!
		filteredAssets.AddRange(Assets.Type(filterType));
	}

	public void AssetWindow()
	{
		UISettings settings = UI.Settings;
		float height = filterScrollCt * (UI.LineHeight + settings.gutter) + settings.padding * 2;
		UI.WindowBegin("Asset Browser", ref filterWindow, V.XY(0.5f, height));

		UI.LayoutPushCut(UICut.Left, 0.1f);
		UI.PanelAt(UI.LayoutAt, UI.LayoutRemaining);

		UI.Label("Filter");

		UI.HSeparator();

		// A radio button selection for what to filter by
		Vec2 size = new Vec2(0.08f, 0);
		if (UI.Radio("Model",    filterType == typeof(Model   ), size)) UpdateFilter(typeof(Model));
		UI.SameLine();
		if (UI.Radio("Mesh",     filterType == typeof(Mesh    ), size)) UpdateFilter(typeof(Mesh));
		UI.SameLine();
		if (UI.Radio("Material", filterType == typeof(Material), size)) UpdateFilter(typeof(Material));
		UI.SameLine();
		if (UI.Radio("Sprite",   filterType == typeof(Sprite  ), size)) UpdateFilter(typeof(Sprite));
		UI.SameLine();
		if (UI.Radio("Sound",    filterType == typeof(Sound   ), size)) UpdateFilter(typeof(Sound));
		UI.SameLine();
		if (UI.Radio("Font",     filterType == typeof(Font    ), size)) UpdateFilter(typeof(Font));
		UI.SameLine();
		if (UI.Radio("Shader",   filterType == typeof(Shader  ), size)) UpdateFilter(typeof(Shader));
		UI.SameLine();
		if (UI.Radio("Solid",    filterType == typeof(Solid   ), size)) UpdateFilter(typeof(Solid));
		UI.SameLine();
		if (UI.Radio("Tex",      filterType == typeof(Tex     ), size)) UpdateFilter(typeof(Tex));
		UI.SameLine();
		if (UI.Radio("All",      filterType == typeof(IAsset  ), size)) UpdateFilter(typeof(IAsset));

		UI.LayoutPop();

		UI.LayoutPushCut(UICut.Right, 0.03f + settings.padding*2);
			UI.PushEnabled(filterScroll - filterScrollCt >= 0);
			if (UI.Button("^", V.XX(0.03f))) filterScroll -= filterScrollCt;
			UI.PopEnabled();
			UI.LayoutPushCut(UICut.Bottom, 0.03f + settings.padding * 2);
				UI.PushEnabled(filterScroll + filterScrollCt < filteredAssets.Count);
				if (UI.Button("v", V.XX(0.03f))) filterScroll += filterScrollCt;
				UI.PopEnabled();
			UI.LayoutPop();
		UI.LayoutPop();


		// We can visualize some of these assets, and just draw a label for
		// some others.
		for (int i = filterScroll; i < Math.Min(filteredAssets.Count, filterScroll + filterScrollCt); i++)
		{
			IAsset asset = filteredAssets[i];
			UI.PushId(i);
			switch (asset)
			{
				case Mesh     item: VisualizeMesh    (item); break;
				case Material item: VisualizeMaterial(item); break;
				case Sprite   item: VisualizeSprite  (item); break;
				case Model    item: VisualizeModel   (item); break;
				case Sound    item: VisualizeSound   (item); break;
			}
			UI.PopId();
			UI.Label(string.IsNullOrEmpty(asset.Id) ? "(null)" : asset.Id);
		}
		
		UI.WindowEnd();
	}

	void VisualizeMesh(Mesh item)
	{
		Bounds meshSize = item.Bounds;
		Bounds b        = UI.LayoutReserve(V.XX(UI.LineHeight), false, UI.LineHeight);
		float  scale    = (1.0f/meshSize.dimensions.Length);
		item.Draw(Material.Default, Matrix.TS(b.center+meshSize.center*scale, b.dimensions*scale));

		UI.SameLine();
	}

	void VisualizeMaterial(Material item)
	{
		// Default Materials have a number of special effect shaders that don't
		// visualize in a generic way.
		if (!string.IsNullOrEmpty(item.Id) && (item.Id.StartsWith("render/") || item.Id.StartsWith("default/")))
			return;

		Bounds b = UI.LayoutReserve(V.XX(UI.LineHeight), false, UI.LineHeight);
		Mesh.Sphere.Draw(item, Matrix.TS(b.center, b.dimensions));

		UI.SameLine();
	}

	void VisualizeSprite(Sprite item)
	{
		UI.Image(item, V.XX(UI.LineHeight));
		UI.SameLine();
	}

	void VisualizeModel(Model item)
	{
		UI.Model(item, V.XX(UI.LineHeight));
		UI.SameLine();
	}

	void VisualizeSound(Sound item)
	{
		if (UI.Button(">", V.XX(UI.LineHeight)))
			item.Play(Hierarchy.ToWorld(UI.LayoutLast.center));
		UI.SameLine();
	}
	/// :End:

	public void Initialize()
	{
		UpdateFilter(typeof(Mesh));
		
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

		Demo.ShowSummary(title, description);
	}
}
