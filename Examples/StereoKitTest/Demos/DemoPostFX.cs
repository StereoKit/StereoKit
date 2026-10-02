// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using StereoKit;
using System.Collections.Generic;

class DemoPostFX : ITest
{
	string title       = "Post Processing";
	string description = "On mobile hardware, post processing can be VERY expensive! StereoKit supports 'subpass' based post processing, accepting some limitations to enable a subset of post processing effects that do run fast (in-tile) on mobile hardware!";

	// Helmet and settings sit side by side, centered together in front.
	Pose windowPose = Matrix.T(0.175f, 0.1f, 0) * Demo.contentPose.Pose;

	Material vignetteMat;
	Material invertMat;
	Material fogMat;
	bool     vignetteOn;
	bool     invertOn;
	bool     fogOn;
	float    vignetteStrength = 0.4f;
	float    fogDensity       = 0.5f;

	Model model;

	public void Initialize()
	{
		/// :CodeSample: Renderer.SetPostProcess
		/// ### Setting a post-process chain
		/// A post-process effect is just a Material! Its shader reads the
		/// scene through an input attachment named 'color', and its
		/// parameters can be changed live, like any other Material. The
		/// chain renders in argument order.
		Material vignette = new Material("postfx_vignette.hlsl");
		vignette["strength"] = 0.4f;
		Renderer.SetPostProcess(vignette);
		///
		/// And when you're done with it:
		Renderer.SetPostProcess();
		/// :End:
		vignetteMat = vignette;

		invertMat = new Material("postfx_math.hlsl");
		invertMat["mul_color"] = new Vec3(-1, -1, -1);
		invertMat["add_color"] = new Vec3( 1,  1,  1);

		fogMat = new Material("postfx_depth_fog.hlsl");
		fogMat["fog_density"] = fogDensity;

		/// :CodeSample: RenderSettings Renderer.RenderTo
		/// ### Rendering a viewpoint with post-processing
		/// RenderSettings works with Renderer.RenderTo and RenderList.DrawNow,
		/// and can carry a post-process chain that applies to just that pass!
		Tex target = Tex.RenderTarget(512, 512);
		Renderer.RenderTo(target, Matrix.T(0, 0, 1), Matrix.Perspective(90, 1, 0.1f, 50),
			new RenderSettings { clearColor  = Color.Black,
			                     postProcess = new Material[] { vignette } });
		/// :End:

		model = Model.FromFile("DamagedHelmet.gltf");

		// Testing takes a screenshot of this demo, and screenshots follow the
		// display's post-process chain - turn some effects on for the shot!
		if (Tests.IsTesting)
		{
			vignetteOn = true;
			fogOn      = true;
			ApplyChain();
		}
	}

	// Chain order is the array order - fog reads scene depth first, then the
	// invert flips colors, and the vignette darkens the corners last.
	void ApplyChain()
	{
		List<Material> chain = new List<Material>();
		if (fogOn     ) chain.Add(fogMat);
		if (invertOn  ) chain.Add(invertMat);
		if (vignetteOn) chain.Add(vignetteMat);
		Renderer.SetPostProcess(chain.ToArray());
	}

	public void Shutdown()
	{
		Renderer.SetPostProcess();
	}

	public void Step()
	{
		UI.WindowBegin("Post Processing", ref windowPose, new Vec2(0.28f, 0));

		if (UI.Toggle("Vignette", ref vignetteOn)) ApplyChain();
		if (vignetteOn && UI.HSlider("strength", ref vignetteStrength, 0, 1))
			vignetteMat["strength"] = vignetteStrength;

		if (UI.Toggle("Invert", ref invertOn)) ApplyChain();

		if (UI.Toggle("Depth fog", ref fogOn)) ApplyChain();
		if (fogOn && UI.HSlider("density", ref fogDensity, 0, 2))
			fogMat["fog_density"] = fogDensity;

		UI.WindowEnd();

		// Left of the settings window, turned to a 3/4 view (contentPose already
		// yaws 180, so 20 here lands the helmet at a 200 world yaw).
		model.Draw(Matrix.TRS(V.XYZ(0.175f, 0, -0.1f), Quat.FromAngles(0, 20, 0), 0.175f) * Demo.contentPose);

		Tests.Screenshot("Demos/PostFX.jpg", 600, 600, V.XYZ(-0.175f, -0.1f, -0.08f), V.XYZ(-0.175f, -0.1f, -0.5f));
		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.02f), V.XYZ(.65f, .45f, 0.6f)));
	}
}
