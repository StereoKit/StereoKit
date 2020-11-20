using StereoKit;

class DemoPBR : ITest
{
	const int  materialGrid = 7;
	Material[] pbrMaterials;
	Model      pbrModel;
	Mesh       sphereMesh;

	Tex oldSkyTex;

	public void Initialize()
	{
		oldSkyTex  = Renderer.SkyTex;
		sphereMesh = Mesh.GenerateSphere(1, 5);
		Renderer.SkyTex = Tex.FromCubemapEquirectangular("old_depot.jpg");
		

		pbrModel = Model.FromFile("DamagedHelmet.gltf", Default.ShaderPbr);
		pbrMaterials = new Material[materialGrid*materialGrid];
		for (int y = 0; y < materialGrid; y++) {
		for (int x = 0; x < materialGrid; x++) {
			int i = x + y * materialGrid;
			pbrMaterials[i] = new Material(Default.ShaderPbr);
			pbrMaterials[i][MatParamName.MetallicAmount ] = x/(float)(materialGrid-1);
			pbrMaterials[i][MatParamName.RoughnessAmount] = y/(float)(materialGrid-1);
		} }
	}

	public void Shutdown()
	{
		Renderer.SkyTex = oldSkyTex;
	}

	public void Update()
	{
		Hierarchy.Push(Matrix.T(0,0,-1));
		pbrModel.Draw(Matrix.TRS(Vec3.Zero, Quat.FromAngles(0,180,0), 0.2f));
		for (int y = 0; y < materialGrid; y++) {
		for (int x = 0; x < materialGrid; x++) {
			Renderer.Add(sphereMesh, pbrMaterials[x+y*materialGrid], Matrix.TS(new Vec3(x-materialGrid/2.0f + 0.5f,y-materialGrid/2.0f + 0.5f,-1)/4.0f, 0.2f));
		} }
		Text.Add("Metallic -->",  Matrix.TRS(new Vec3(0, materialGrid/8.0f+0.2f, -0.25f), Quat.FromAngles(0,180,0), 4));
		Text.Add("Roughness -->", Matrix.TRS(new Vec3(materialGrid/-8.0f-0.2f, 0, -0.25f), Quat.FromAngles(0,180,-90), 4));
		Hierarchy.Pop();
	}
}
