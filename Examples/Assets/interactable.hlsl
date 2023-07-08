#include "stereokit.hlsli"

//--color:color = 1, 1, 1, 1
float4 color;

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos       : SV_POSITION;
    float3 world_pos : TEXCOORD0;
	float4 color     : COLOR0;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	o.world_pos = mul(float4(input.pos.xyz,  1), sk_inst[id].world).xyz;
	o.pos       = mul(float4(o.world_pos,    1), sk_viewproj[o.view_id]);
	o.color     = input.col * color * sk_inst[id].color;
	return o;
}

float finger_distance(float3 world_pos) {
	return min(
        distance(sk_fingertip[0].xyz, world_pos),
        distance(sk_fingertip[1].xyz, world_pos));
}

float4 ps(psIn input) : SV_TARGET {
    float dist = saturate(1-(finger_distance(input.world_pos)/0.2));
	return input.color * pow(dist, 12); 
}