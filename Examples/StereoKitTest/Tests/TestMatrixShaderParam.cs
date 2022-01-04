using StereoKit;

class TestHatrixShaderParam : ITest
{
	Material _material;

	public void Initialize()
	{
		_material = new Material(Shader.FromFile("matrix_param.hlsl"));
		_material[MatParamName.DiffuseTex] = Tex.FromFile("test.png");
		_material["custom_transform"] = (Matrix)System.Numerics.Matrix4x4.Transpose(Matrix.TS(-0.25f, 0, 0, 0.25f));
	}

	public void Shutdown() { }

	public void Update()
	{
		Mesh.Cube.Draw(_material, Matrix.Identity);

		Tests.Screenshot("Tests/MatrixParam.jpg", 600, 600, 90, V.XYZ(0, 0, .5f), V.XYZ(0, 0, 0));
	}
}