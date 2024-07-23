#include "stereokit.hlsli"

//--diffuse = white

Texture2D    diffuse   : register(t0);
SamplerState diffuse_s : register(s0);

struct vsIn {
	float4 pos      : SV_Position;
	float3 norm     : NORMAL0;
	float2 quadrant : TEXCOORD0;
	float4 color    : COLOR0;
};
struct psIn {
	float4 pos     : SV_Position;
    half2  uv      : TEXCOORD0;
	float3 world   : TEXCOORD1;
	half4  color   : COLOR0;
	uint   view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;
	
	// Extract scale from the matrix
	float4x4 world_mat = sk_inst[id].world;
	float2   scale     = float2(
		length(float3(world_mat._11,world_mat._12,world_mat._13)),
		length(float3(world_mat._21,world_mat._22,world_mat._23))
	);
	// Restore scale to 1
	world_mat[0] = world_mat[0] / scale.x;
	world_mat[1] = world_mat[1] / scale.y;
	// Translate the position using the quadrant (TEXCOORD0) information and 
	// the extracted scale.
	float4 sized_pos;
	sized_pos.xy = input.pos.xy + input.quadrant * scale * 0.5;
	sized_pos.zw = input.pos.zw;
	
	float3 normal = normalize(mul(input.norm, (float3x3)world_mat));
	float4 world  = mul(sized_pos, world_mat);
	o.pos    = mul(world, sk_viewproj[o.view_id]);
    o.uv     = -0.5*input.quadrant+0.5f - input.pos.xy/scale;
	o.world  = world.xyz;
	o.color.rgb = input.color.rgb * sk_inst[id].color.rgb * sk_lighting(normal);
	o.color.a   = input.color.a;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float4 col  = diffuse.Sample(diffuse_s, input.uv);
	float  glow = pow(1-saturate(sk_finger_distance(input.world) / 0.12), 10);
	return float4(lerp(col.rgb * input.color.rgb, half3(1, 1, 1), glow), input.color.a);
}