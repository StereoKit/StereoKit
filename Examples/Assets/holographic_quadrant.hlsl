#include "stereokit.hlsli"

//--name = holographic/quadrant
//--color:color = .2, .2, .2, 1

float4 color;

struct vsIn {
	float4 pos      : SV_Position;
	float3 norm     : NORMAL0;
	float2 quadrant : TEXCOORD0;
	float4 color    : COLOR0;
};
struct psIn {
	float4 pos        : SV_Position;
	float3 normal     : NORMAL0;
	float4 light_edge : COLOR0;
	float4 inst_col   : COLOR1;
	float4 world      : TEXCOORD0;
	float  alpha      : TEXCOORD1;
	float  glow_mask  : TEXCOORD2;
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

	o.inst_col       = sk_inst[id].color;
	o.light_edge.rgb = Lighting(o.normal);
	o.light_edge.a   = input.color.a;
	o.alpha          = input.color.b > 0.5 ? 1 : (o.inst_col.a-1) * 0.5;
	o.glow_mask      = input.color.g;
	return o;
}

static const int    ir_gradient_count = 5;
static const float3 ir_gradient[] = { float3(0,0,0), float3(0,0,0),  float3(0.065,0.018,0.429), float3(0,0,0), float3(0.013,  0.117,  0.597), float3(0,0,0) };
float3 GetIridescence(uint view_id, float3 world_pos, float3 surface_normal) {
	float3 dir = normalize(sk_camera_pos[view_id].xyz - world_pos);
	float  ir  = saturate(dot(dir, surface_normal));

	float pct  = ir * ir_gradient_count;
	int   g    = pct;
	pct = smoothstep(0, 1, frac(pct));
	float3 curr = ir_gradient[g];
	float3 next = ir_gradient[g+1];
	return lerp(curr, next, pct);
}

float4 ps(psIn input) : SV_TARGET {
	FingerDist dist = FingerDistanceInfo(input.world.xyz, input.normal);

	const float ring_max_dist = 0.14;
	const float ring_initial  = 0.006;
	const float ring_grow     = 0.03;
	float glow_amt    = saturate(dist.from_finger / ring_max_dist);
	float glow_radius = ring_initial + glow_amt * ring_grow;
	float glow        = saturate(1-(dist.on_plane / glow_radius)) * (1-glow_amt) * input.glow_mask;

	float3 iridescence = GetIridescence(input.view_id, input.world.xyz, input.normal);
	float  edge        = saturate((input.light_edge.a - 0.15) / fwidth(input.light_edge.a));

	float4 col = lerp(color, input.inst_col, edge) * input.inst_col.a;
	col.rgb = col.rgb * input.light_edge.rgb + iridescence*(0.2 + glow*8);
	col.a = max(glow, input.alpha);

	return col;
}