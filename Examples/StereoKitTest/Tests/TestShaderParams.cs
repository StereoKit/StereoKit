using StereoKit;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
struct ShaderParamsData
{
	public uint a_x;
	public uint a_y;
	public int b_x;
	public int b_y;
};

class TestShaderParams : ITest
{
	Material _material;

	public void Initialize()
	{
		_material = new Material(Shader.FromFile("shader_param_types.hlsl"));
		_material[MatParamName.DiffuseTex] = Tex.FromFile("test.png");
		_material["on_off"] = true;
		_material["color"] = Color.White*0.5f;

		ShaderParamsData data = new ShaderParamsData{ a_x=1, a_y=2, b_x=3, b_y=4 };
		_material.SetData("data", data);

		_material.SetInt("id_set", 1,2,3,4);
		_material.SetUInt("id_set2", 1,2,3,4);
	}

	public void Shutdown() { }

	public void Update()
	{
		Mesh.Cube.Draw(_material, Matrix.Identity);
	}
}