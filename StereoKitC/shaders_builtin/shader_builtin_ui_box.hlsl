#include "stereokit.hlsli"

//--name = sk/default_ui_box

float4 color                = {.6, .6, .6, 1};
float  border_size          = 0.005;
float  border_size_grow     = 0.01;
float  border_affect_radius = 0.2;

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4      pos   : SV_POSITION;
	float2      uv    : TEXCOORD0;
	min16float4 color : COLOR0;
	float3      world : TEXCOORD1;
	float2      scale : TEXCOORD2;
};

psIn vs(vsIn input, sk_ids_t ids) {
	psIn o;
	// Extract scale from the matrix
	float4x4 world_mat = sk_inst[ids.inst].world;
	float3   scale     = float3(
		length(float3(world_mat._11,world_mat._12,world_mat._13)),
		length(float3(world_mat._21,world_mat._22,world_mat._23)),
		length(float3(world_mat._31,world_mat._32,world_mat._33)));

	// Switch scale axes based on the model's normal
	if      (abs(input.norm.y) > 0.75) o.scale = scale.xz;
	else if (abs(input.norm.x) > 0.75) o.scale = scale.zy;
	else                               o.scale = scale.xy;

	float4 world = mul(input .pos, sk_inst    [ids.inst].world);
	o.pos        = mul(world,      sk_viewproj[ids.view]);
	o.world      = world.xyz;

	o.uv    = input.uv-0.5;
	o.color = color * input.col * sk_inst[ids.inst].color;
	return o;
}
min16float4 ps(psIn input) : SV_TARGET {
	min16float glow = sk_finger_glow(input.world);
	
	float  border_grow = glow * border_size_grow + border_size;
	float2 border_pos  = (0.5-abs(input.uv)) * input.scale;
	float  corner      = -(min(border_pos.x, border_pos.y)-border_grow);

	if (max(glow, corner) <= 0.0)
		discard;

	// corner / fwidth(corner) will antialias the edges. This requires some
	// kind of transparency on the material, alpha to coverage, or
	// Transparency.MSAA seems to work pretty nicely here.
	input.color.a *= max(glow*2, corner / fwidth(corner));

	return min16float4(lerp(input.color.rgb, min16float3(1, 1, 1), glow), input.color.a);
}