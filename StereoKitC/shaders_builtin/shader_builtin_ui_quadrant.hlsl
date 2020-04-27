// [name] sk/default_ui_quadrant

#include <stereokit>

cbuffer ParamBuffer : register(b2) {
	// [param] color color {1, 1, 1, 1}
	float4 _color;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float4 color : COLOR0;
	float3 norm : NORMAL;
	float2 uv   : TEXCOORD0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float4 color : COLOR0;
	float4 world : TEXCOORD1;
	float3 normal: NORMAL;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;

	// Extract scale from the matrix
	float4x4 world_mat = sk_inst[id].world;
	float2 scale = float2(
		length(world_mat._11_12_13),
		length(world_mat._21_22_23)
	);
	world_mat[0] = world_mat[0] / scale.x;
	world_mat[1] = world_mat[1] / scale.y;
	output.world.zw = input.pos.zw;
	output.world.xy = input.pos.xy + input.uv * scale * 0.5;

	output.world = mul(output.world, world_mat);
	output.pos   = mul(output.world, sk_viewproj[sk_inst[id].view_id]);

	output.normal = normalize(mul(input.norm, (float3x3)world_mat));

	output.view_id    = sk_inst[id].view_id;
	output.color      = lerp(_color, sk_inst[id].color, input.color.a);
	output.color.rgb *= Lighting(output.normal);
	return output;
}

float4 ps(psIn input) : SV_TARGET {
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