using StereoKit;
using System.Runtime.InteropServices;

class DocShaderArrays : ITest
{
	/// :CodeSample: Material.SetData Material
	/// ### Assigning an array in a Shader
	/// This is a bit of a hack until proper shader support for arrays arrives,
	/// but with a few C# marshalling tricks, we can assign array without too
	/// much trouble. Look for improvements to this in later versions of
	/// SteroKit.
	// This struct matches a shader parameter of `float4 offsets[10];`
	[StructLayout(LayoutKind.Sequential)]
	struct ShaderData
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
		public Vec4[] offsets;
	}
	
	Material arrayMaterial = null;
	public void Initialize()
	{
		ShaderData shaderData = new ShaderData();
		shaderData.offsets = new Vec4[10] {
			V.XYZW(0,0,0,0),
			V.XYZW(0.2f,0,0,0),
			V.XYZW(0.4f,0,0,0),
			V.XYZW(0.4f,0.2f,0,0),
			V.XYZW(0.4f,0.4f,0,0),
			V.XYZW(0.4f,0.6f,0,0),
			V.XYZW(0.2f,0.6f,0,0),
			V.XYZW(0,0.6f,0,0),
			V.XYZW(0,0.4f,0,0),
			V.XYZW(0,0.2f,0,0)};
		
		arrayMaterial = new Material(Shader.FromFile("shader_arrays.hlsl"));
		arrayMaterial[MatParamName.DiffuseTex] = Tex.FromFile("test.png");
		arrayMaterial.SetData<ShaderData>("offsets", shaderData);
	}
	/// :End:

	public void Shutdown() { }
	public void Update()
		=> Mesh.Cube.Draw(arrayMaterial, Matrix.TS(0, 0, -0.5f, 0.1f));
}