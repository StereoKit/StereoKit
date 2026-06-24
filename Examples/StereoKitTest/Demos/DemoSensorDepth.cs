// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using System;
using StereoKit;

class DemoSensorDepth : ITest
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

	enum ColorMode
	{
		Eye,
		Depth,
		Confidence,
	}

	string title       = "Sensor Depth";
	string description = "Sensor depth visualization. Switch between a point cloud view and automatic World.Occlusion depth-based occlusion.";

	Pose controlsPose  = (Matrix.T(-0.22f, 0.02f, 0) * Demo.contentPose).Pose;

	// Point cloud
	Material pointCloudMatL;
	Material pointCloudMatR;
	Mesh     pointMesh;

	// Occlusion
	Model    model;
	Pose     modelPose;
	float    modelScale = 0.25f;

	DemoMode demoMode     = DemoMode.PointCloud;
	EyeMode  eyeMode      = EyeMode.Both;
	float    pointSize    = 0.008f;
	int      sampleStep   = 2;
	float    nearClip     = 0.1f;
	float    opacity      = 0.25f;
	float    depthScale   = 1.0f;
	ColorMode colorMode   = ColorMode.Eye;
	bool      useRawDepth = false;
	bool      flipVertical = true;
	bool     handToggle   = true;

	SensorDepthCaps  caps;
	bool             hasFrame;
	SensorDepthFrame latestFrame;
	uint             meshWidth;
	uint             meshHeight;
	int              meshStep;
	Tex              depthTex;
	Tex              confidenceTex;
	byte[]           confLeft;
	byte[]           confRight;

	public void Initialize()
	{
		// Point cloud materials
		Shader pointShader = Shader.FromFile("sensor_depth_point_cloud.hlsl");
		pointCloudMatL = new(pointShader) { Transparency = Transparency.Blend, DepthWrite = false };
		pointCloudMatR = new(pointShader) { Transparency = Transparency.Blend, DepthWrite = false };
		pointCloudMatL["screen_size"] = 0.0f;
		pointCloudMatR["screen_size"] = 0.0f;

		model = Model.FromFile("DamagedHelmet.gltf");

		// Place the helmet 1m in front of the user at head height
		modelPose = new Pose(0, 0, -0.6f, Quat.LookDir(0, 0, 1));

		depthTex = null;
		Permission.Request(PermissionType.SceneFine);
	}

	public void Shutdown()
	{
		if (demoMode == DemoMode.Occlusion)
			World.Occlusion = OcclusionCaps.None;

		if (Sensor.Depth.IsRunning)
			Sensor.Depth.Stop();
	}

	public void Step()
	{
		if (Sensor.Depth.IsAvailable && !Sensor.Depth.IsRunning)
			Sensor.Depth.Start();

		caps = Sensor.Depth.GetCapabilities();

		if (demoMode == DemoMode.PointCloud)
		{
			if (Sensor.Depth.TryGetLatestFrame(out SensorDepthFrame frame))
			{
				hasFrame    = true;
				latestFrame = frame;
				EnsurePointMesh(frame.width, frame.height, sampleStep);
				UpdateDepthTexture(frame);
				UpdateConfidenceTexture(frame);
			}

			DrawPointCloud();
		}
		else
		{
			DrawOcclusion();
		}

		DrawControls();
	}

	void UpdateDepthTexture(SensorDepthFrame frame)
	{
		if (depthTex != null) return;
		depthTex = Sensor.Depth.Texture;
		if (depthTex == null) return;

		pointCloudMatL[MatParamName.DiffuseTex] = depthTex;
		pointCloudMatR[MatParamName.DiffuseTex] = depthTex;
		// 0 = ndc; 1 = metric meters
		float format = frame.depthFormat == SensorDepthFormat.MetersR32 ? 1.0f : 0.0f;
		pointCloudMatL["depth_format"] = format;
		pointCloudMatR["depth_format"] = format;
	}

	void UpdateConfidenceTexture(SensorDepthFrame frame)
	{
		if (colorMode != ColorMode.Confidence || !caps.HasFlag(SensorDepthCaps.Confidence))
			return;

		SensorDepthImage image = useRawDepth ? SensorDepthImage.RawConfidence : SensorDepthImage.SmoothConfidence;
		if (!Sensor.Depth.TryGetLatestData(out SensorDepthFrame _, ref confLeft,  0, image)) return;
		if (!Sensor.Depth.TryGetLatestData(out SensorDepthFrame _, ref confRight, 1, image)) return;

		// Confidence range/direction are runtime-defined (the extension leaves them
		// unspecified), so normalize to the observed max for a usable gradient.
		byte max = 1;
		foreach (byte v in confLeft)  if (v > max) max = v;
		foreach (byte v in confRight) if (v > max) max = v;
		float confScale = 255.0f / max;

		if (confidenceTex == null)
			confidenceTex = new Tex(TexType.Image, TexFormat.R8) { SampleMode = TexSample.Point, AddressMode = TexAddress.Clamp };
		confidenceTex.SetColors((int)frame.width, (int)frame.height, new byte[][] { confLeft, confRight }, 1);
		pointCloudMatL["confidence"] = confidenceTex; pointCloudMatL["conf_scale"] = confScale;
		pointCloudMatR["confidence"] = confidenceTex; pointCloudMatR["conf_scale"] = confScale;
	}

	void ApplyDepthCaps()
	{
		if (!Sensor.Depth.IsRunning)
			return;

		SensorDepthCaps c = useRawDepth && caps.HasFlag(SensorDepthCaps.RawDepth)
			? SensorDepthCaps.RawDepth
			: SensorDepthCaps.SmoothDepth;
		if (!handToggle && caps.HasFlag(SensorDepthCaps.HandRemoval))
			c |= SensorDepthCaps.HandRemoval;
		if (colorMode == ColorMode.Confidence && caps.HasFlag(SensorDepthCaps.Confidence))
			c |= SensorDepthCaps.Confidence;
		Sensor.Depth.SetCapabilities(c);
	}

	void DrawPointCloud()
	{
		if (!hasFrame || pointMesh is null)
			return;

		bool stereo = latestFrame.viewCount >= 2;
		if (!stereo || (eyeMode is EyeMode.Left or EyeMode.Both))
		{
			SetPointCloudEyeParams(pointCloudMatL, latestFrame.views[0], 0, new Color(0.40f, 0.85f, 1.0f, 1));
			pointMesh.Draw(pointCloudMatL, Matrix.Identity);
		}
		if (stereo && (eyeMode is EyeMode.Right or EyeMode.Both))
		{
			SetPointCloudEyeParams(pointCloudMatR, latestFrame.views[1], 1, new Color(1.0f, 0.75f, 0.35f, 1));
			pointMesh.Draw(pointCloudMatR, Matrix.Identity);
		}
	}

	void DrawOcclusion()
	{
		if (model is null)
			return;

		UI.Handle("OcclusionModel", ref modelPose, model.Bounds * modelScale);
		model.Draw(modelPose.ToMatrix(modelScale), Color.White);
	}

	void SetPointCloudEyeParams(Material mat, SensorDepthView eye, int eyeLayer, Color color)
	{
		mat["point_size"]     = pointSize;
		mat["depth_near"]     = latestFrame.nearZ;
		mat["depth_far"]      = latestFrame.farZ;
		mat["depth_scale"]    = depthScale;
		mat["near_clip"]      = nearClip;
		mat["eye_layer"]      = (float)eyeLayer;
		mat["eye_pose"]       = eye.pose.ToMatrix();
		mat["color_mode"]     = (float)(int)colorMode;
		mat["flip_v"]         = flipVertical ? 1.0f : 0.0f;

		float leftTan   = MathF.Tan(eye.fov.left   * Units.deg2rad);
		float rightTan  = MathF.Tan(eye.fov.right  * Units.deg2rad);
		float topTan    = MathF.Tan(eye.fov.top    * Units.deg2rad);
		float bottomTan = MathF.Tan(eye.fov.bottom * Units.deg2rad);
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
		var verts      = new Vertex[pointCount * 4];
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
		pointMesh.SetData(verts, inds);

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

	void SwitchToMode(DemoMode newMode)
	{
		if (demoMode == newMode) return;

		if (demoMode == DemoMode.Occlusion)
			World.Occlusion = OcclusionCaps.None;

		demoMode = newMode;

		if (newMode == DemoMode.Occlusion)
			World.Occlusion = OcclusionCaps.Depth | (handToggle ? OcclusionCaps.Hands : OcclusionCaps.None);
	}

	void DrawControls()
	{
		UI.WindowBegin("Sensor Depth", ref controlsPose, new Vec2(35 * U.cm, 0));

		UI.Label($"Available: {Sensor.Depth.IsAvailable} | Running: {Sensor.Depth.IsRunning}");
		if (hasFrame)
			UI.Label($"Size: {latestFrame.width}x{latestFrame.height}  Near/Far: {latestFrame.nearZ:0.##}/{latestFrame.farZ:0.##}");

		// Mode selector
		UI.HSeparator();
		if (UI.Radio("Point Cloud", demoMode == DemoMode.PointCloud)) SwitchToMode(DemoMode.PointCloud);
		if (World.OcclusionCapabilities.HasFlag(OcclusionCaps.Depth))
		{
			UI.SameLine();
			if (UI.Radio("Occlusion", demoMode == DemoMode.Occlusion)) SwitchToMode(DemoMode.Occlusion);
		}

		UI.HSeparator();
		if (demoMode == DemoMode.PointCloud)
		{
			if (hasFrame && latestFrame.viewCount >= 2)
			{
				if (UI.Radio("Left Eye", eyeMode == EyeMode.Left)) eyeMode = EyeMode.Left;
				UI.SameLine();
				if (UI.Radio("Right Eye", eyeMode == EyeMode.Right)) eyeMode = EyeMode.Right;
				UI.SameLine();
				if (UI.Radio("Both Eyes", eyeMode == EyeMode.Both)) eyeMode = EyeMode.Both;
			}

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
					EnsurePointMesh(latestFrame.width, latestFrame.height, sampleStep);
			}

			// Raw vs smooth depth - shown only where the backend exposes both
			if (caps.HasFlag(SensorDepthCaps.RawDepth))
			{
				UI.HSeparator();
				UI.Label("Depth Source");
				if (UI.Radio("Smooth", !useRawDepth)) { useRawDepth = false; ApplyDepthCaps(); }
				UI.SameLine();
				if (UI.Radio("Raw", useRawDepth)) { useRawDepth = true; ApplyDepthCaps(); }
			}

			UI.HSeparator();
			UI.Label("Color By");
			if (UI.Radio("Eye", colorMode == ColorMode.Eye)) { colorMode = ColorMode.Eye; ApplyDepthCaps(); }
			UI.SameLine();
			if (UI.Radio("Depth", colorMode == ColorMode.Depth)) { colorMode = ColorMode.Depth; ApplyDepthCaps(); }
			if (caps.HasFlag(SensorDepthCaps.Confidence))
			{
				UI.SameLine();
				if (UI.Radio("Confidence", colorMode == ColorMode.Confidence)) { colorMode = ColorMode.Confidence; ApplyDepthCaps(); }
			}

			// Hand removal - shown only where the backend supports it
			if (caps.HasFlag(SensorDepthCaps.HandRemoval))
			{
				UI.HSeparator();
				if (UI.Toggle("Show Hands", ref handToggle))
					ApplyDepthCaps();
			}

			UI.HSeparator();
			if (!Sensor.Depth.IsRunning)
			{
				if (UI.Button("Start Depth Capture"))
					Sensor.Depth.Start();
			}
			else
			{
				if (UI.Button("Stop Depth Capture"))
				{
					Sensor.Depth.Stop();
					depthTex = null;
				}
			}
		}
		else // Occlusion
		{
			SliderRow("Model Scale", "modelscale", ref modelScale, 0.05f, 1.0f, 0, "{0:0.00}");

			if (World.OcclusionCapabilities.HasFlag(OcclusionCaps.Hands))
			{
				UI.HSeparator();
				if (UI.Toggle("Hand Occlusion", ref handToggle))
					World.Occlusion = OcclusionCaps.Depth | (handToggle ? OcclusionCaps.Hands : OcclusionCaps.None);
			}
		}

		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0.0f, 0.05f), V.XYZ(0.95f, 0.75f, 0.8f)));
	}
}
