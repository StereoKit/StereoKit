#include "stereokit.hlsli"

//--name = sk/default_ui_box
//--color:color = .6, .6, .6, 1
//--border_size = 0.005
//--border_size_grow = 0.01
//--border_affect_radius = 0.2
float4       color;
float        border_size;
float        border_size_grow;
float        border_affect_radius;

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
	float4 world : TEXCOORD1;
	float2 scale : TEXCOORD2;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	// Extract scale from the matrix
	float4x4 world_mat = sk_inst[id].world;
	float3   scale     = float3(
		length(float3(world_mat._11,world_mat._12,world_mat._13)),
		length(float3(world_mat._21,world_mat._22,world_mat._23)),
		length(float3(world_mat._31,world_mat._32,world_mat._33)));

	// Switch scale axes based on the model's normal
	if      (abs(input.norm.y) > 0.75) o.scale = scale.xz;
	else if (abs(input.norm.x) > 0.75) o.scale = scale.zy;
	else                               o.scale = scale.xy;

	o.world = mul(input .pos, sk_inst    [id].world);
	o.pos   = mul(o.world,    sk_viewproj[o.view_id]);

	o.uv    = input.uv-0.5;
	o.color = color * input.col * sk_inst[id].color;
	return o;
}
float4 ps(psIn input) : SV_TARGET {
	float  glow = pow(1 - saturate(sk_finger_distance(input.world.xyz) / 0.12), 10);
	
	float  border_grow = glow * border_size_grow + border_size;
	float2 border_pos  = (0.5-abs(input.uv)) * input.scale;
	float  corner      = -(min(border_pos.x, border_pos.y)-border_grow);

	if (max(glow, corner) <= 0.0)
		discard;

	// corner / fwidth(corner) will antialias the edges. This requires some
	// kind of transparency on the material, alpha to coverage, or
	// Transparency.MSAA seems to work pretty nicely here.
	input.color.a *= max(glow*2, corner / fwidth(corner));

	return float4(lerp(input.color.rgb, float3(1, 1, 1), glow), input.color.a);
}