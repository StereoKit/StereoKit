// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2023-2024 Qualcomm Technologies, Inc.

using StereoKit;
using System;
using System.Collections.Generic;
using System.IO;

class Light
{
	public Pose pose;
	public Vec3 color;
}
enum LightMode
{
	World,
	Lights,
	Image,
}

class DemoLighting : ITest
{
	string title       = "Lighting Editor";
	string description = "";

	static List<Light> lights         = new List<Light>();
	static Tex         cubemap        = null;
	static bool        cubelightDirty = false;
	static string      cubemapFile    = "";

	Pose       windowPose    = (Demo.contentPose * Matrix.T(-0.2f, 0, 0)).Pose;
	Pose       previewPose   = (Demo.contentPose * Matrix.T(0.2f, -0.1f, 0)).Pose;
	Pose       lightToolPose = new Pose((Demo.contentPose * Matrix.T(0.0f, -0.1f, 0)).Translation, Quat.Identity);
	LightMode  mode          = LightMode.Lights;
	Model      previewModel  = Model.FromFile("DamagedHelmet.gltf");
	Mesh       lightMesh     = Mesh.GenerateSphere(1);
	Material   lightProbeMat;
	Material   lightSrcMat   = new Material(Default.ShaderUnlit);

	public void Initialize() {
		lightProbeMat = Material.PBR.Copy();
		lightProbeMat[MatParamName.RoughnessAmount] = 0.0f;
		lightProbeMat[MatParamName.MetallicAmount] = 0.0f;
		lightProbeMat[MatParamName.ColorTint] = new Color(0,0,0,1);
	}
	public void Shutdown() => Platform.FilePickerClose();
	public void Step()
	{
		UI.WindowBegin("Lighting Source", ref windowPose);
		UI.Label("Mode");
		UI.SameLine();
		if (UI.Radio("World", mode == LightMode.World)) mode = LightMode.World;
		UI.SameLine();
		if (UI.Radio("Lights", mode == LightMode.Lights)) mode = LightMode.Lights;
		UI.SameLine();
		if (UI.Radio("Image",  mode == LightMode.Image))  mode = LightMode.Image;

		UI.HSeparator();

		if (mode == LightMode.World)
		{
			UI.PushEnabled(Lighting.ModeAvailable(LightingMode.World));
			bool useEstimation = Lighting.Mode == LightingMode.World;
			if (UI.Toggle("Use Light Estimation", ref useEstimation)) Lighting.Mode = useEstimation ? LightingMode.World : LightingMode.Manual;
			UI.PopEnabled();
		}
		
		if (mode == LightMode.Lights)
		{
			if (UI.Button("Add"))
			{
				lights.Add(new Light
				{
					pose = new Pose(Vec3.Up * 25 * U.cm, Quat.LookDir(-Vec3.Forward)),
					color = Vec3.One
				});
				UpdateLights();
			}

			if (UI.Button("Remove") && lights.Count > 1)
			{
				lights.RemoveAt(lights.Count - 1);
				UpdateLights();
			}
		}

		if (mode == LightMode.Image)
		{
			if (!Platform.FilePickerVisible && UI.Button("Browse"))
				Platform.FilePicker(PickerMode.Open, LoadSkyImage, null, Assets.TextureFormats);
			UI.Label(cubemapFile);
		}

		UI.HSeparator();

		if (UI.Button("Print Lighting Code"))
		{
			Vec3[] c = Lighting.Ambient.ToArray();
			string shStr = "new SphericalHarmonics(new Vec3[]{";
			for (int i = 0; i < c.Length; i++)
				shStr += $"new Vec3({c[i].x:F2}f, {c[i].y:F2}f, {c[i].z:F2}f),";
			shStr += "});";
			Log.Info(shStr);
		}

		UI.WindowEnd();

			
		UI.Handle("Preview", ref previewPose, previewModel.Bounds*0.1f);
		previewModel.Draw(previewPose.ToMatrix(0.1f));

		UI.Handle("Light Tool", ref lightToolPose, new Bounds(Vec3.One * 0.12f));
		Hierarchy.Push(Matrix.T(lightToolPose.position));
		lightMesh.Draw(lightProbeMat, Matrix.S(0.04f));
		
		Lighting.GetDirectional(out Vec3 lightDir, out Color lightColor);
		Lines.Add(-lightDir * 0.02f, -lightDir * 0.06f, lightColor, lightColor, 0.005f);
		DrawSH(Lighting.Ambient, 0.02f, 0.06f);
		if (mode == LightMode.Lights)
		{ 
			bool needsUpdate = false;
			for (int i = 0; i < lights.Count; i++) {
				needsUpdate = LightHandle(i) || needsUpdate;
			}
			if (needsUpdate)
				UpdateLights();
		}
		Hierarchy.Pop();

		if (cubelightDirty && cubemap.AssetState == AssetState.Loaded) {
			Lighting.Ambient = cubemap.CubemapLighting;
			cubelightDirty = false;
		}

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.08f), V.XYZ(.7f, .3f, 0.1f)));
	}

	static void DrawSH(SphericalHarmonics sh, float min, float max, int sampleCount = 512, float sampleSize = 0.001f)
	{
		float range = max - min;
		float phi   = (float)(Math.PI * (3.0f - Math.Sqrt(5)));
		float y, r, theta;

		for (int i = 0; i < sampleCount; i++)
		{
			y     = 1 - (i / (float)(sampleCount - 1)) * 2;  // y goes from 1 to -1
			r     = (float)Math.Sqrt(1 - y * y);             // radius at y
			theta = phi * i;                                 // golden angle increment

			Vec3 dir = new Vec3((float)Math.Cos(theta) * r, y, (float)Math.Sin(theta) * r);
			Color c = sh.Sample(dir);

			// Get intensity as grayscale, would be better as some perceptual
			// brightness.
			float intensity = c.r * 0.3f + c.g * 0.59f + c.b * 0.11f;

			Mesh.Cube.Draw(Material.Unlit, Matrix.TS(dir * (min + range * intensity), sampleSize), c);
		}
	}

	bool LightHandle(int i)
	{
		UI.PushId("window"+i);
		bool dirty = UI.HandleBegin("Color", ref lights[i].pose, new Bounds(Vec3.One * 3 * U.cm));
		UI.LayoutArea(new Vec3(6,-3,0)*U.cm, new Vec2(10, 0)*U.cm);
		if (lights[i].pose.position.Length > 0.5f)
			lights[i].pose.position = lights[i].pose.position.Normalized * 0.5f;

		lightMesh.Draw(lightSrcMat, Matrix.TS(Vec3.Zero, 3* U.cm), Color.HSV(lights[i].color));

		float h = lights[i].color.x;
		float s = lights[i].color.y;
		float v = lights[i].color.z;
		dirty = UI.HSlider("H", ref h, 0, 1, 0, 10 * U.cm) || dirty;
		dirty = UI.HSlider("S", ref s, 0, 1, 0, 10 * U.cm) || dirty;
		dirty = UI.HSlider("V", ref v, 0, 1, 0, 10 * U.cm) || dirty;
		lights[i].color = new Vec3(h, s, v);

		UI.HandleEnd();
		Lines.Add(
			lights[i].pose.position, Vec3.Zero,
			Color.HSV(lights[i].color) * LightIntensity(lights[i].pose.position) * 0.5f,
			U.mm);

		UI.PopId();
		return dirty;
	}

	void LoadSkyImage(string file)
	{
		cubemapFile = Path.GetFileName(file);
		cubemap     = Tex.FromCubemap(file);

		Renderer.SkyTex     = cubemap;
		Lighting.Reflection = cubemap;
		cubelightDirty      = true;
	}

	void UpdateLights()
	{
		SphericalHarmonics lighting = SphericalHarmonics.FromLights(lights
			.ConvertAll(a => new SHLight{ 
				directionTo = a.pose.position.Normalized,
				color       = Color.HSV(a.color) * LightIntensity(a.pose.position) })
			.ToArray());

		Renderer.SkyTex     = Tex.GenCubemap(lighting);
		Lighting.Reflection = Renderer.SkyTex;
		Lighting.Ambient    = lighting;
	}

	float LightIntensity(Vec3 pos)
		=> Math.Max(0, 2 - pos.Magnitude * 4);
}