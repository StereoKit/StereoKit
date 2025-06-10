// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

using StereoKit.Framework;
using StereoKit;

class DemoLayers : ITest
{
	string title       = "Composition Layers";
	string description = "OpenXR allows for a variety of surfaces to be composed onto the screen. StereoKit by default manages just a single 'Projection Layer', but you can also submit additional layers with special shapes (quad layers), or special color buffers (like video)!";

	Pose windowPose  = (Demo.contentPose * Matrix.T(-0.2f, 0, 0)).Pose;
	Pose previewPose = (Demo.contentPose * Matrix.T( 0.2f, 0, 0)).Pose;

	XrCompLayers.Swapchain swapchain  = null;
	RenderList             renderList = new RenderList();
	float                  sortOrder  = 1;

	public void Initialize()
	{
		// XrCompLayers _must_ be added to SK's steppers before StereoKit
		// initialization! See Program.cs for where this project adds it.

		// We can only do composition layers with OpenXR as our runtime!
		// XrCompLayers will fail nicely, but we do want a warning log if an
		// actual failure happens, not if it's simply not available.
		if (Backend.XRType == BackendXRType.OpenXR)
		{
			if (XrCompLayers.TryMakeSwapchain(512, 512, TexFormat.Rgba32, XrCompLayers.UsageFlags.COLOR_ATTACHMENT_BIT, false, out XrCompLayers.XrSwapchain handle))
				swapchain = new XrCompLayers.Swapchain(handle, TexFormat.Rgba32, 512, 512);
			if (swapchain == null)
				Log.Warn("Failed to make OpenXR Layer swapchain!");
		}

		// Create a small scene we can render to our layer.
		Material mat = Material.Default.Copy();
		mat[MatParamName.DiffuseTex] = Tex.FromFile("floor.png");
		renderList.Add(Mesh.Sphere, mat, Matrix.S(0.1f), Color.White);
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		const float size = 0.3f;

		// Handle for making the quad layer interactive.
		UI.Handle("QuadLayer", ref previewPose, new Bounds(size, size, 0.04f));
		Mesh.Cube.Draw(Material.UIBox, previewPose.ToMatrix(V.XYZ(size, size, 0.04f)));

		if (swapchain != null)
		{
			// Draw our scene to the swapchain, and submit it with our layer!
			Color oldColor = Renderer.ClearColor;
			Renderer.ClearColor = Color.White;
			swapchain .Acquire();
			renderList.DrawNow(swapchain.RenderTarget, Matrix.LookAt(Vec3.AngleXZ(Time.Totalf * 90), Vec3.Zero), Matrix.Orthographic(0.2f, 0.2f, 0.01f, 50));
			swapchain .Release();
			Renderer.ClearColor = oldColor;
			XrCompLayers.SubmitQuadLayer(previewPose, V.XY(size, size), swapchain.handle, new Rect(0, 0, swapchain.width, swapchain.height), 0, (int)sortOrder);
		}
		else
			Text.Add("This requires an OpenXR runtime!", previewPose.ToMatrix(), V.XY(size, size), TextFit.Wrap);

		// Now some simple settings for our scene.
		UI.WindowBegin("Composition Layers", ref windowPose, V.XY(0.2f,0));
		UI.Label($"Sort Order {(int)sortOrder}", V.XY(0.1f,0));
		UI.SameLine();
		UI.HSlider("Sort Order", ref sortOrder, -1, 1, 1);
		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.08f), V.XYZ(.7f, .3f, 0.1f)));
	}
}