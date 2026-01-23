using StereoKit;

class TestShaderAssign : ITest
{
	Material mat1;
	Material mat2;
	Material mat3;
	Material mat4;

	public void Initialize()
	{
		mat1 = Material.Default.Copy();
		mat1.Shader = Shader.PBR;

		mat2 = Material.Find("sk/lighting/skybox_material").Copy();
		mat2.Shader = Shader.UIBox;

		mat3 = mat2.Copy();
		mat3.Shader = Shader.UI;

		mat4 = mat3.Copy();
		mat4.Shader = null;
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		Mesh.Cube.Draw(mat1, Matrix.TS(-1.5f,0,-1.5f,0.75f));
		Mesh.Cube.Draw(mat2, Matrix.TS(-0.5f,0,-1.5f,0.75f));
		Mesh.Cube.Draw(mat3, Matrix.TS( 0.5f,0,-1.5f,0.75f));
		Mesh.Cube.Draw(mat4, Matrix.TS( 1.5f,0,-1.5f,0.75f));
	}
}