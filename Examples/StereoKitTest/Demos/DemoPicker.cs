// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;
using System;

class DemoPicker : ITest
{
	string title       = "File Picker";
	string description = "Applications need to save and load files at runtime! StereoKit has a cross-platform, MR compatible file picker built in, Platform.FilePicker.\n\nOn systems/conditions where a native file picker is available, that's what you'll get! Otherwise, StereoKit will fall back to a custom picker built with StereoKit's UI.";

	Model    model      = null;
	float    modelScale = 1;
	int      modelTask  = 0;
	bool     showNodes  = false;
	bool     editNodes  = false;
	bool     showModel  = true;
	Pose     modelPose  = (Matrix.T(-0.4f,-0.3f,0) * Demo.contentPose).Pose;
	Pose     menuPose   = Demo.contentPose.Pose;
	Material jointMaterial;
	ViewMode viewMode;

	public enum ViewMode
	{
		Visuals,
		Tools,
		Anim,
	}

	public static void ModelInspector(Model model, Pose modelPose, Material jointMaterial, ref float modelScale, ref ViewMode mode, ref bool showNodes, ref bool showModel, ref bool editNodes)
	{
		UI.PanelAt(UI.LayoutAt, new Vec2(UI.LayoutRemaining.x, UI.LineHeight));
		if (UI.Radio("Visuals",   mode == ViewMode.Visuals)) mode = ViewMode.Visuals;
		UI.SameLine();
		if (UI.Radio("Tools",     mode == ViewMode.Tools  )) mode = ViewMode.Tools;
		UI.SameLine();
		UI.PushEnabled(model.Anims.Count > 0);
		if (UI.Radio("Animation", mode == ViewMode.Anim   )) mode = ViewMode.Anim;
		UI.PopEnabled();

		if (mode == ViewMode.Tools)
		{
			UI.Toggle("Show Nodes", ref showNodes);
			UI.Toggle("Edit Nodes", ref editNodes);
			UI.Toggle("Show Model", ref showModel);
		}
		else if (mode == ViewMode.Visuals)
		{
			foreach (var node in model.Visuals)
			{
				bool vis = node.Visible;
				if (UI.Toggle(node.Name, ref vis, Sprite.RadioOff, Sprite.RadioOn, UIBtnLayout.CenterNoText, V.XY(UI.LineHeight, UI.LineHeight)))
					node.Visible = vis;

				UI.SameLine();
				Bounds b = UI.LayoutReserve(new Vec2(UI.LineHeight, 0));
				b.center.z -= b.dimensions.x * 0.5f;
				if (node.Material != null)
					Mesh.Sphere.Draw(node.Material, Matrix.TS(b.center, b.dimensions.x * 0.5f));

				UI.SameLine();
				b = UI.LayoutReserve(new Vec2(UI.LineHeight, 0));
				b.center.z -= b.dimensions.x * 0.5f;
				Mesh m = node.Mesh;
				if (m != null)
					m.Draw(Material.Default, Matrix.TS(b.center, b.dimensions.x * 0.5f * (1.0f /m.Bounds.dimensions.Magnitude)));

				UI.SameLine();
				UI.Label(node.Name);
			}
		}
		else if (mode == ViewMode.Anim)
		{
			UI.PushEnabled(model.Anims.Count > 0);
			foreach (Anim anim in model.Anims)
			{
				if (UI.ButtonImg(anim.Name, Sprite.ArrowRight))
					model.PlayAnim(anim, AnimMode.Loop);
			}
			float animScrub = model.AnimCompletion;
			if (UI.HSlider("Scrub", ref animScrub, 0, 1, 0))
			{
				if (model.AnimMode != AnimMode.Manual)
					model.PlayAnim(model.ActiveAnim, AnimMode.Manual);
				model.AnimCompletion = animScrub;
			}

			if (UI.ButtonImg("Play", Sprite.ArrowRight, UIBtnLayout.CenterNoText))
			{
				model.PlayAnim(model.ActiveAnim, AnimMode.Loop);
				model.AnimCompletion = animScrub;
			}
			UI.SameLine();
			if (UI.ButtonImg("Stop", Sprite.Close, UIBtnLayout.CenterNoText))
			{
				model.PlayAnim(model.ActiveAnim, AnimMode.Manual);
				model.AnimCompletion = animScrub;
			}
			UI.SameLine();
			if (UI.ButtonImg("Reset", Sprite.Backspace, UIBtnLayout.CenterNoText))
			{
				model.AnimCompletion = 0;
			}
			UI.PopEnabled();
		}

		Hierarchy.Push(modelPose.ToMatrix(modelScale), HierarchyParent.Ignore);
		// Step Animation manually in case showModel is false, and the
		// call to Draw gets skipped.
		model.StepAnim();
		if (showModel) model.Draw(Matrix.Identity);
		if (showNodes) ShowNodes(model, jointMaterial);
		if (editNodes) EditNodes(model, Default.MaterialUIBox);
		Hierarchy.Pop();
	}

	public void Initialize()
	{
		jointMaterial = Material.Default.Copy();
		jointMaterial.DepthTest   = DepthTest.Always;
		jointMaterial.QueueOffset = 1;
	}
	public void Shutdown() => Platform.FilePickerClose();

	public void Step() {
		UI.WindowBegin("Settings", ref menuPose);

		/// :CodeSample: Platform.FilePicker Platform.FilePickerVisible
		/// ### Opening a Model
		/// This is a simple button that will open a 3D model selection
		/// file picker, and make a call to OnLoadModel after a file has
		/// been successfully picked!
		if (UI.Button("Open Model") && !Platform.FilePickerVisible) {
			Platform.FilePicker(PickerMode.Open, OnLoadModel, null, Assets.ModelFormats);
		}
		/// :End:
		UI.SameLine();
		string id = model?.Id ?? "";
		int min = Math.Max(0, id.Length - 20);
		int len = id.Length - min;
		UI.Label(id.Substring(min, len));

		float percent = (Assets.CurrentTask - modelTask) / (float)(Assets.TotalTasks - modelTask);
		if (percent < 1)
			UI.HProgressBar(percent);

		if (model != null)
			ModelInspector(model, modelPose, jointMaterial, ref modelScale, ref viewMode, ref showNodes, ref showModel, ref editNodes);

		UI.WindowEnd();
		Demo.ShowSummary(title, description, new Bounds(V.XY0(0,-0.1f), V.XYZ(.34f, .34f, .1f)));
	}

	/// :CodeSample: Platform.FilePicker Platform.FilePickerVisible
	/// Once you have the filename, it's simply a matter of loading it
	/// from file. This is an example of async loading a model, and 
	/// calculating a scale value that ensures the model is a reasonable
	/// size.
	private void OnLoadModel(string filename)
	{
		model      = Model.FromFile(filename);
		modelTask  = Assets.CurrentTask;
		modelScale = 1 / model.Bounds.dimensions.Magnitude;
		if (model.Anims.Count > 0)
			model.PlayAnim(model.Anims[0], AnimMode.Loop);
	}
	/// :End:

	private static void ShowNodes(Model model, Material jointMaterial)
	{
		Vec3 headLocal = Hierarchy.ToLocal(Input.Head.position);
		Vec3 downLocal = Hierarchy.ToLocalDirection(-Vec3.UnitY).Normalized;
		float scale = Hierarchy.ToLocalDirection(Vec3.UnitX).Magnitude;
		for (int n = 0; n < model.Nodes.Count; n++)
		{
			var node = model.Nodes[n];
			Matrix nodeTransform = Matrix.S(0.025f * scale) * node.ModelTransform;
			Mesh.Sphere.Draw(jointMaterial, nodeTransform);

			Vec3 textPos = nodeTransform.Translation + downLocal * (0.025f * scale);
			Text.Add(node.Name, Matrix.TRS(textPos,Quat.LookAt(textPos, headLocal), scale), Pivot.TopCenter, Align.TopCenter);
		}
	}

	private static void EditNodes(Model model, Material jointMaterial)
	{
		float scale = Hierarchy.ToLocalDirection(Vec3.UnitX).Magnitude;

		for (int n = 0; n < model.Nodes.Count; n++)
		{
			ModelNode node = model.Nodes[n];
			Bounds    b    = node.Mesh?.Bounds * node.ModelTransform.Scale ?? new Bounds(Vec3.One * 0.1f * scale);
			Pose      pose = node.ModelTransform.Pose;
			if (UI.HandleBegin(node.Name, ref pose, b))
				node.ModelTransform = pose.ToMatrix(node.ModelTransform.Scale);
			if (UI.LastElementFocused.IsActive())
				Mesh.Cube.Draw(jointMaterial, Matrix.TS(b.center, b.dimensions));
			UI.HandleEnd();
		}
	}
}