#include "stereokit.hlsli"

//--name = sk/default_ui_quadrant_aura
//--color:color = 1, 1, 1, 1

float4 color;

struct vsIn {
	float4 pos      : SV_Position;
	float3 norm     : NORMAL0;
	float2 quadrant : TEXCOORD0;
	float4 color    : COLOR0;
};
struct psIn : sk_ps_input_t {
	float4 pos     : SV_Position;
	float3 world   : TEXCOORD0;
	half3  color   : COLOR0;
};

psIn vs(vsIn input, sk_vs_input_t sk_in) {
	psIn o;
	uint view_id = sk_view_init(sk_in, o);
	uint id      = sk_inst_id  (sk_in);
	
	// Extract scale from the matrix
	float4x4 world_mat = sk_inst[id].world;
	float3   scale     = float3(
		length(float3(world_mat._11,world_mat._12,world_mat._13)),
		length(float3(world_mat._21,world_mat._22,world_mat._23)),
		length(float3(world_mat._31,world_mat._32,world_mat._33))
	);
	// Restore scale to 1
	world_mat[0] = world_mat[0] / scale.x;
	world_mat[1] = world_mat[1] / scale.y;
	world_mat[2] = world_mat[2] / scale.z;
	// Translate the position using the quadrant (TEXCOORD0) information and
	// the extracted scale.
	float4 sized_pos;
	sized_pos.xy = input.pos.xy + input.quadrant * scale.xy * 0.5;
	sized_pos.zw = input.pos.zw;
	
	sized_pos.xyz += input.norm * sk_inst[id].color.a * 0.002;

	float4 world = mul(sized_pos, world_mat);
	float3 normal = normalize(mul(input.norm, (float3x3) world_mat));
	o.pos   = mul(world, sk_viewproj[view_id]);
	o.world = world.xyz;
	o.color = lerp(color.rgb, sk_inst[id].color.rgb, input.color.a) * sk_lighting(normal);
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float  glow = pow(saturate(1 - sk_finger_distance(input.world) / 0.12), 2) * 0.2;
	return float4(lerp(input.color.rgb, float3(2,2,2), glow), 1);
}