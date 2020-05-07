using StereoKit;
using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

class TestShaderCompile : ITest
{
	Material materialHLSL;
	Material materialCompile;
	Material materialFileHLSL;
	Material materialFileCompile;
	Mesh     cubeMesh = Mesh.GenerateRoundedCube(Vec3.One*0.1f, 0.01f);

	public void Initialize()
	{
		// Delete all cached shader files
		string cachePath = Path.Combine(Path.GetTempPath(), "cache");
		if (Directory.Exists(cachePath))
			Array.ForEach(Directory.GetFiles(cachePath), (p) => File.Delete(p));

		// Build a shader from an HLSL code string
		Func<bool> buildFromHLSL = () =>
        {
            Shader shader = Shader.FromHLSL(shaderCode.Replace("[shader_name]", "solid_color_1"));
			materialHLSL = new Material(shader);
			return shader != null;
        };
		// Compile a shader from HLSL code, then turn that into a shader.
		Func<bool> compileAndBuild = () =>
		{
			byte[] file   = Shader.Compile(shaderCode.Replace("[shader_name]", "solid_color_2"));
			Shader shader = Shader.FromMemory(file);
			materialCompile = new Material(shader);
			return shader != null;
		};
		// Load an hlsl shader from file, and a pre-compiled shader from file
		Func<bool> loadFromFile = () =>
		{
			string file1 = Path.Combine(cachePath, "test_shader_1.hlsl");
			string code1 = shaderCode.Replace("[shader_name]", "solid_color_3");
			string file2 = Path.Combine(cachePath, "test_shader_2.sks");
			string code2 = shaderCode.Replace("[shader_name]", "solid_color_4");
			File.WriteAllText (file1, code1);
			File.WriteAllBytes(file2, Shader.Compile(code2));

			// Wait for file to finish writing
			while(!IsFileAvailable(file1) || !IsFileAvailable(file2))
				Thread.Sleep(100);

			Shader shader1 = Shader.FromFile(file1);
			Shader shader2 = Shader.FromFile(file2);
			materialFileHLSL    = new Material(shader1);
			materialFileCompile = new Material(shader2);
			return shader1 != null && shader2 != null;
		};

		// If we're testing, then block and go. If we're not testing, then 
		// lets do this asynchronously!
		if (Tests.IsTesting) {
			Tests.Test(buildFromHLSL);
			Tests.Test(compileAndBuild);
			Tests.Test(loadFromFile);
		} else { 
			Task.Run(buildFromHLSL);
			Task.Run(compileAndBuild);
			Task.Run(loadFromFile);
		}
    }
    public void Shutdown  (){}

    public void Update()
	{
        Draw(materialHLSL,        Vec3.Zero);
		Draw(materialCompile,     Vec3.Up    * 0.2f);
		Draw(materialFileHLSL,    Vec3.Right * 0.2f);
		Draw(materialFileCompile, Vec3.Up    *-0.2f);
	}
	void Draw(Material material, Vec3 at)
	{
		if (material != null)
		{
			cubeMesh.Draw(material, Matrix.T(at));
		}
		else
		{
			Text.Add("Compiling shader...", Matrix.T(at));
		}
	}

	static bool IsFileAvailable(string filePath)
	{
		if (!File.Exists(filePath)) return false;
		try { using (File.Open(filePath, FileMode.Open)) { } }
		catch (IOException) { return false; }
		return true;
	}

	string shaderCode = @"
// [name] app/[shader_name]
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view[2];
	float4x4 sk_proj[2];
	float4x4 sk_viewproj[2];
	float4   sk_lighting_sh[9];
	float4   sk_camera_pos[2];
	float4   sk_camera_dir[2];
	float4   sk_fingertip[2];
	float    sk_time;
};
struct Inst {
	float4x4 world;
	float4   color;
	uint     view_id;
};
cbuffer TransformBuffer : register(b1) {
	Inst sk_inst[682];
};
cbuffer ParamBuffer : register(b2) {
	// [ param ] vector color {1, 1, 1, 1} tags { data for whatever! }
	float4 _color;
};
struct vsIn {
	float4 pos : SV_POSITION;
};
struct psIn {
	float4 pos   : SV_POSITION;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	float3 world   = mul(float4(input.pos.xyz, 1), sk_inst[id].world).xyz;
	output.pos     = mul(float4(world, 1), sk_viewproj[sk_inst[id].view_id]);
	output.view_id = sk_inst[id].view_id;
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	return float4(1,1,1,1); 
}";
}
