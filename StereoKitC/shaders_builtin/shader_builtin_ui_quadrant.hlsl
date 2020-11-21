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
	psIn output;

	// Extract scale from the matrix
	float4x4 world_mat = sk_inst[id].world;
	float2 scale = float2(
		length(world_mat._11_12_13),
		length(world_mat._21_22_23)
	);
	// Restore scale to 1
	world_mat[0] = world_mat[0] / scale.x;
	world_mat[1] = world_mat[1] / scale.y;
	// Translate the position using the quadrant (TEXCOORD0) information and 
	// the extracted scale.
	float4 sized_pos;
	sized_pos.xy = input.pos.xy + input.quadrant * scale * 0.5;
	sized_pos.zw = input.pos.zw;

	output.world  = mul(sized_pos, world_mat);
	output.pos    = mul(output.world, sk_viewproj[sk_inst[id].view_id]);
	output.normal = normalize(mul(input.norm, (float3x3)world_mat));

	output.view_id    = sk_inst[id].view_id;
	output.color      = lerp(color, sk_inst[id].color, input.color.a);
	output.color.rgb *= Lighting(output.normal);
	return output;
}

float4 ps(psIn input) : SV_TARGET{
	float dist = 1;
	float ring = 0;
	for	(int i=0;i<2;i++) {
		float3 delta = sk_fingertip[i].xyz - input.world.xyz;
		float3 norm  = normalize(delta);
		float  d     = dot(delta,delta) / (0.08 * 0.08);
		ring = max( ring, min(1, 1 - abs(max(0,dot(input.normal, norm))-0.9)*200*d) );
		dist = min( dist, d );
	}

	float  pct = pow(1-dist, 5);
	float4 col = float4(lerp(input.color.rgb, float3(1,1,1), pct*0.6 + (ring*pct)), input.color.a);

	return col; 
}