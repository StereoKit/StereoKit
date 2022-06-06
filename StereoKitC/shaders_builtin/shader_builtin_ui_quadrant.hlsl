#include "stereokit.hlsli"

//--name = sk/default_ui_quadrant
//--color:color = 1, 1, 1, 1

float4 color;

struct vsIn {
	float4 pos      : SV_Position;
	float3 norm     : NORMAL0;
	float2 quadrant : TEXCOORD0;
	float4 color    : COLOR0;
};
struct psIn {
	float4 pos     : SV_Position;
	float3 normal  : NORMAL0;
	float4 color   : COLOR0;
	float4 world   : TEXCOORD1;
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

	o.world  = mul(sized_pos, world_mat);
	o.pos    = mul(o.world, sk_viewproj[o.view_id]);
	o.normal = normalize(mul(input.norm, (float3x3)world_mat));

	o.color      = lerp(color, sk_inst[id].color, input.color.a) * sk_inst[id].color.a;
	o.color.rgb *= Lighting(o.normal);
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float  glow = FingerGlow(input.world.xyz, input.normal);
	float4 col  = float4(lerp(input.color.rgb, float3(2,2,2), glow), input.color.a);

	return col; 
}