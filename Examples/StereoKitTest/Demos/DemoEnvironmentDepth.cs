// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using System;
using StereoKit;

class DemoEnvironmentDepth : ITest
{
	enum EyeMode
	{
		Left,
		Right,
		Both,
	}

	enum DemoMode
	{
		PointCloud,
		Occlusion,
	}

	string title       = "Environment Depth";
	string description = "Environment depth visualization. Switch between a point cloud view and depth-based occlusion of virtual objects.";

	Pose controlsPose  = (Matrix.T(-0.22f, 0.02f, 0) * Demo.contentPose).Pose;

	// Point cloud
	Material pointCloudMatL;
	Material pointCloudMatR;
	Mesh     pointMesh;

	// Occlusion
	Model    occlusionModel;
	Material occlusionMat;
	Pose     modelPose;
	float    modelScale = 0.25f;

	DemoMode demoMode     = DemoMode.PointCloud;
	EyeMode  eyeMode      = EyeMode.Both;
	float    pointSize    = 0.008f;
	int      sampleStep   = 2;
	float    nearClip     = 0.1f;
	float    opacity      = 0.25f;
	float    depthScale   = 1.0f;
	bool     handRemoval  = false;
	bool     colorByDepth = false;

	bool                   hasFrame;
	EnvironmentDepth.Frame latestFrame;
	uint                   meshWidth;
	uint                   meshHeight;
	int                    meshStep;

	public void Initialize()
	{
		// Point cloud materials
		Shader pointShader = Shader.FromFile("env_depth_point_cloud.hlsl");
		pointCloudMatL = new(pointShader) { Transparency = Transparency.Blend, DepthWrite = false };
		pointCloudMatR = new(pointShader) { Transparency = Transparency.Blend, DepthWrite = false };
		pointCloudMatL["screen_size"] = 0.0f;
		pointCloudMatR["screen_size"] = 0.0f;

		// Occlusion material & model
		Shader occlusionShader = Shader.FromFile("env_depth_occlusion.hlsl");
		occlusionMat   = new(occlusionShader);
		occlusionModel = Model.FromFile("DamagedHelmet.gltf");

		// Place the helmet 1m in front of the user at head height
		modelPose = new Pose(0, 0, -0.6f, Quat.LookDir(0, 0, 1));

		if (!EnvironmentDepth.IsAvailable)
		{
			Log.Warn("Environment depth is not available on this runtime.");
			return;
		}

		EnvironmentDepth.Start();
	}

	public void Shutdown()
	{
		if (EnvironmentDepth.IsRunning)
			EnvironmentDepth.Stop();
	}

	public void Step()
	{
		EnvironmentDepth.Frame frame = default;
		bool gotFrame = EnvironmentDepth.TryGetLatestFrame(out frame) && frame.Texture != null;
		if (gotFrame)
		{
			hasFrame = true;
			latestFrame = frame;
			if (demoMode is DemoMode.PointCloud)
				EnsurePointMesh(frame.Width, frame.Height, sampleStep);
		}

		if (demoMode is DemoMode.PointCloud)
			DrawPointCloud();
		else
			DrawOcclusion();

		DrawControls();
	}

	void DrawPointCloud()
	{
		if (!hasFrame || pointMesh is null)
			return;

		if (eyeMode is EyeMode.Left or EyeMode.Both)
		{
			SetPointCloudEyeParams(pointCloudMatL, latestFrame.Left, 0, new Color(0.40f, 0.85f, 1.0f, 1));
			pointMesh.Draw(pointCloudMatL, Matrix.Identity);
		}
		if (eyeMode is EyeMode.Right or EyeMode.Both)
		{
			SetPointCloudEyeParams(pointCloudMatR, latestFrame.Right, 1, new Color(1.0f, 0.75f, 0.35f, 1));
			pointMesh.Draw(pointCloudMatR, Matrix.Identity);
		}
	}

	void DrawOcclusion()
	{
		if (!hasFrame || occlusionModel is null)
			return;

		UI.Handle("OcclusionModel", ref modelPose, occlusionModel.Bounds * modelScale);
		SetOcclusionParams(occlusionMat);
		occlusionModel.Draw(occlusionMat, modelPose.ToMatrix(modelScale), Color.White);
	}

	Matrix BuildDepthViewProj(EnvironmentDepth.View eye)
	{
		// View matrix transforms world -> depth eye space
		Matrix viewMat = eye.Pose.ToMatrix().Inverse;

		// Projection matrix from asymmetric FOV tangents
		float l = MathF.Tan(eye.Fov.left   * Units.deg2rad);
		float r = MathF.Tan(eye.Fov.right  * Units.deg2rad);
		float t = MathF.Tan(eye.Fov.top    * Units.deg2rad);
		float b = MathF.Tan(eye.Fov.bottom * Units.deg2rad);

		// Build an infinite reverse-Z or standard projection.
		// For depth comparison we use the OpenGL-style projection that
		// matches the depth texture's z_window encoding.
		float n = latestFrame.NearZ;
		float width  = r - l;
		float height = t - b;

		Matrix projectionMat;
		if (float.IsInfinity(latestFrame.FarZ))
		{
			// Infinite far plane (OpenGL convention: z_ndc ∈ [-1,1], z_window = (z_ndc+1) / 2)
			// Using tangent values directly, so no near-plane multiplier on [0][0] and [1][1]
			projectionMat = new(
				2.0f / width,    0,                 0,      0,
				0,               2.0f / height,     0,      0,
				(r + l) / width, (t + b) / height, -1,     -1,
				0,               0,                -2 * n,  0
			);
		}
		else
		{
			float f = latestFrame.FarZ;
			projectionMat = new(
				2.0f / width,    0,                0,                     0,
				0,               2.0f / height,    0,                     0,
				(r + l) / width, (t + b) / height, -(f + n) / (f - n),   -1,
				0,               0,                -2 * f * n / (f - n),  0
			);
		}

		return viewMat * projectionMat;
	}

	void SetOcclusionParams(Material mat)
	{
		mat["env_depth"]  = latestFrame.Texture;
		mat["depth_near"] = latestFrame.NearZ;
		mat["depth_far"]  = latestFrame.FarZ;

		mat["depth_view_proj_l"] = BuildDepthViewProj(latestFrame.Left);
		mat["depth_view_proj_r"] = BuildDepthViewProj(latestFrame.Right);
	}

	void SetPointCloudEyeParams(Material mat, EnvironmentDepth.View eye, int eyeLayer, Color color)
	{
		mat[MatParamName.DiffuseTex] = latestFrame.Texture;
		mat["point_size"]     = pointSize;
		mat["depth_near"]     = latestFrame.NearZ;
		mat["depth_far"]      = latestFrame.FarZ;
		mat["depth_scale"]    = depthScale;
		mat["near_clip"]      = nearClip;
		mat["eye_layer"]      = (float)eyeLayer;
		mat["eye_pose"]       = eye.Pose.ToMatrix();
		mat["color_by_depth"] = colorByDepth ? 1.0f : 0.0f;

		float leftTan   = MathF.Tan(eye.Fov.left   * Units.deg2rad);
		float rightTan  = MathF.Tan(eye.Fov.right  * Units.deg2rad);
		float topTan    = MathF.Tan(eye.Fov.top    * Units.deg2rad);
		float bottomTan = MathF.Tan(eye.Fov.bottom * Units.deg2rad);
		mat["eye_tans"] = new Vec4(leftTan, rightTan, topTan, bottomTan);

		color.a = opacity;
		mat[MatParamName.ColorTint] = color;
	}

	void EnsurePointMesh(uint width, uint height, int step)
	{
		if (width == meshWidth && height == meshHeight && step == meshStep && pointMesh != null)
			return;

		int samplesX   = Math.Max(1, (int)Math.Ceiling(width / (float)step));
		int samplesY   = Math.Max(1, (int)Math.Ceiling(height / (float)step));
		int pointCount = samplesX * samplesY;
		Vertex[] verts = new Vertex[pointCount * 4];
		uint[] inds    = new uint[pointCount * 6];

		int p = 0;
		for (int y = 0; y < samplesY; y++)
		{
			for (int x = 0; x < samplesX; x++)
			{
				int sampleX = Math.Min((x * step), (int)width - 1);
				int sampleY = Math.Min((y * step), (int)height - 1);
				float u     = (sampleX + 0.5f) / width;
				float v     = (sampleY + 0.5f) / height;

				int vi = p * 4;
				verts[vi + 0] = new(Vec3.Zero, V.XYZ(u, v, 0), V.XY(-0.5f,  0.5f), Color.White);
				verts[vi + 1] = new(Vec3.Zero, V.XYZ(u, v, 0), V.XY( 0.5f,  0.5f), Color.White);
				verts[vi + 2] = new(Vec3.Zero, V.XYZ(u, v, 0), V.XY( 0.5f, -0.5f), Color.White);
				verts[vi + 3] = new(Vec3.Zero, V.XYZ(u, v, 0), V.XY(-0.5f, -0.5f), Color.White);

				uint ii = (uint)(p * 6);
				uint baseVert = (uint)vi;
				inds[ii + 0] = baseVert + 2;
				inds[ii + 1] = baseVert + 1;
				inds[ii + 2] = baseVert + 0;
				inds[ii + 3] = baseVert + 3;
				inds[ii + 4] = baseVert + 2;
				inds[ii + 5] = baseVert + 0;

				p++;
			}
		}

		pointMesh ??= new Mesh();
		pointMesh.SetInds(inds);
		pointMesh.SetVerts(verts);

		meshWidth  = width;
		meshHeight = height;
		meshStep   = step;
	}

	void SliderRow(string label, string id, ref float value, float min, float max, float step, string fmt)
	{
		float labelW  = 10 * U.cm;
		float sliderW = 13 * U.cm;
		float valueW  =  5 * U.cm;

		UI.PushId(id);
		UI.Label(label, V.XY(labelW, 0));
		UI.SameLine();
		UI.HSlider(id, ref value, min, max, step, sliderW);
		UI.SameLine();
		UI.Label(string.Format(fmt, value), V.XY(valueW, 0));
		UI.PopId();
	}

	void DrawControls()
	{
		UI.WindowBegin("Environment Depth", ref controlsPose, new Vec2(35 * U.cm, 0));

		UI.Label($"Available: {EnvironmentDepth.IsAvailable} | Running: {EnvironmentDepth.IsRunning}");
		if (hasFrame)
			UI.Label($"Size: {latestFrame.Width}x{latestFrame.Height}  Near/Far: {latestFrame.NearZ:0.##}/{latestFrame.FarZ:0.##}");

		// Mode selector
		UI.HSeparator();
		if (UI.Radio("Point Cloud", demoMode == DemoMode.PointCloud)) demoMode = DemoMode.PointCloud;
		UI.SameLine();
		if (UI.Radio("Occlusion", demoMode == DemoMode.Occlusion)) demoMode = DemoMode.Occlusion;

		UI.HSeparator();
		if (demoMode == DemoMode.PointCloud)
		{
			if (UI.Radio("Left Eye", eyeMode == EyeMode.Left)) eyeMode = EyeMode.Left;
			UI.SameLine();
			if (UI.Radio("Right Eye", eyeMode == EyeMode.Right)) eyeMode = EyeMode.Right;
			UI.SameLine();
			if (UI.Radio("Both Eyes", eyeMode == EyeMode.Both)) eyeMode = EyeMode.Both;

			float step = sampleStep;
			SliderRow("Point Size",   "pointsize",  ref pointSize, 0.001f, 0.03f, 0,    "{0:0.000}");
			SliderRow("Sample Step",  "samplestep", ref step,      1,      16,    1,    "{0:0}");
			SliderRow("Near Clip (m)","nearclip",   ref nearClip,  0.1f,   2.0f,  0,    "{0:0.00}");
			SliderRow("Opacity",      "opacity",    ref opacity,   0.0f,   1.0f,  0,    "{0:0.00}");
			SliderRow("Depth Scale",  "depthscale", ref depthScale,0.25f,  4.0f,  0.25f,"{0:0.00}");
			if ((int)step != sampleStep)
			{
				sampleStep = (int)step;
				if (hasFrame)
					EnsurePointMesh(latestFrame.Width, latestFrame.Height, sampleStep);
			}

			UI.HSeparator();
			if (EnvironmentDepth.SupportsHandRemoval)
			{
				if (UI.Toggle("Hand Removal", ref handRemoval))
					EnvironmentDepth.SetHandRemoval(handRemoval);
				UI.SameLine();
			}
			UI.Toggle("Color by Depth", ref colorByDepth);
		}
		else // Occlusion
		{
			SliderRow("Model Scale", "modelscale", ref modelScale, 0.05f, 1.0f, 0, "{0:0.00}");

			UI.HSeparator();
			if (EnvironmentDepth.SupportsHandRemoval)
			{
				if (UI.Toggle("Hand Removal", ref handRemoval))
					EnvironmentDepth.SetHandRemoval(handRemoval);
			}
		}

		UI.HSeparator();
		if (!EnvironmentDepth.IsRunning)
		{
			if (UI.Button("Start Depth Capture"))
				EnvironmentDepth.Start();
		}
		else
		{
			if (UI.Button("Stop Depth Capture"))
				EnvironmentDepth.Stop();
		}

		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0.0f, 0.05f), V.XYZ(0.95f, 0.75f, 0.8f)));
	}
}