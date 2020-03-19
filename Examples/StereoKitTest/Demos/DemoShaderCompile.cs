using StereoKit;
using System;
using System.Threading.Tasks;

class DemoShaderCompile : ITest
{
    Material customMaterial;
	Mesh     cubeMesh = Mesh.GenerateRoundedCube(Vec3.One*0.1f, 0.01f);

    public void Initialize()
	{
        Action compile = () =>
        {
            Shader shader = Shader.FromHLSL(@"
// [name] app/solid_color
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
}");
			customMaterial = new Material(shader);
        };

		// If we're testing, then block and compile. If we're not testing,
		// then lets do this asynchronously!
        if (Tests.IsTesting)
			compile();
		else
			Task.Run(compile);
    }
    public void Shutdown  (){}

    public void Update()
	{
        if (customMaterial != null)
		{ 
			cubeMesh.Draw(customMaterial, Matrix.Identity);
		}
		else
		{
			Text.Add("Compiling shader...", Matrix.Identity);
		}
    }
}
