// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoPBR : ITest
{
	string title       = "PBR Shaders";
	string description = "Shaders!";

	const int  materialGrid = 6;
	Material[] pbrMaterials;
	Model      pbrModel;
	Mesh       sphereMesh;
	Material   selectionMat;

	Tex oldSkyTex;
	SphericalHarmonics oldSkyLight;

	Pose modelPose = Pose.Identity;

	public void Initialize()
	{
		oldSkyTex   = Renderer.SkyTex;
		oldSkyLight = Renderer.SkyLight;
		sphereMesh  = Mesh.GenerateSphere(1, 7);
		Renderer.SkyTex = Tex.FromCubemap(@"old_depot.hdr");
		Renderer.SkyTex.OnLoaded += t => Renderer.SkyLight = t.CubemapLighting;
		selectionMat = new Material("interactable.hlsl");
		selectionMat.Transparency = Transparency.Add;
		selectionMat.DepthTest = DepthTest.Equal;

		pbrModel = Model.FromFile("DamagedHelmet.gltf");
		pbrMaterials = new Material[materialGrid*materialGrid];
		for (int y = 0; y < materialGrid; y++) {
		for (int x = 0; x < materialGrid; x++) {
			int i = x + y * materialGrid;
			pbrMaterials[i] = new Material(Default.ShaderPbr);
			pbrMaterials[i][MatParamName.ColorTint      ] = Color.HSV(0.3f, 0.8f, 0.8f);
			pbrMaterials[i][MatParamName.MetallicAmount ] = x/(float)(materialGrid-1);
			pbrMaterials[i][MatParamName.RoughnessAmount] = y/(float)(materialGrid-1);
		} }

		/// :CodeSample: Material.GetAllParamInfo Material.GetParamInfo Material.ParamCount MatParamInfo MatParamInfo.name MatParamInfo.type
		/// ### Listing parameters in a Material

		// Iterate using a foreach
		Log.Info("Builtin PBR Materials contain these parameters:");
		foreach (MatParamInfo info in Material.PBR.GetAllParamInfo())
			Log.Info($"- {info.type,8}: {info.name}");

		// Or with a normal for loop
		Log.Info("Builtin Unlit Materials contain these parameters:");
		for (int i=0; i<Material.Unlit.ParamCount; i+=1)
		{
			MatParamInfo info = Material.Unlit.GetParamInfo(i);
			Log.Info($"- {info.type,8}: {info.name}");
		}
		/// :End:
	}

	public void Shutdown()
	{
		Renderer.SkyTex   = oldSkyTex;
		Renderer.SkyLight = oldSkyLight;
	}

	public void Step()
	{
		Tests.Screenshot("PBRBalls.jpg", 1024, 1024, new Vec3(0, 0, -0.1f), new Vec3(0, 0, -1));

		Hierarchy.Push(Matrix.T(0,0,-1));

		if (!Tests.IsTesting) {
			UI.HandleBegin("Model", ref modelPose, pbrModel.Bounds * .2f, allowedGestures: UIGesture.PinchGrip);
			pbrModel.Draw(Matrix.TRS(Vec3.Zero, Quat.FromAngles(0, 180, 0), 0.2f));
			pbrModel.Draw(selectionMat, Matrix.TRS(Vec3.Zero, Quat.FromAngles(0, 180, 0), 0.2f), new Color(1,1,1,1));
			UI.HandleEnd();
		}

		for (int y = 0; y < materialGrid; y++) {
		for (int x = 0; x < materialGrid; x++) {
			Renderer.Add(sphereMesh, pbrMaterials[x+y*materialGrid], Matrix.TS(new Vec3(x-materialGrid/2.0f + 0.5f,y-materialGrid/2.0f + 0.5f,-1)/4.0f, 0.2f));
		} }
		Text.Add("Metallic -->",  Matrix.TRS(new Vec3(0, materialGrid/8.0f+0.2f,  -0.25f), Quat.FromAngles(0,180, 0 ), 4));
		Text.Add("Roughness -->", Matrix.TRS(new Vec3(materialGrid/-8.0f-0.2f, 0, -0.25f), Quat.FromAngles(0,180,-90), 4));
		Hierarchy.Pop();

		Demo.ShowSummary(title, description, new Bounds(V.XYZ(0.1f, .2f, 0.65f), V.XYZ(2.0f, 2.0f, .2f)));
	}
}
