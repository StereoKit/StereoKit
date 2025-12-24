#include "stereokit.hlsli"

//--color:color = 1, 1, 1, 1
//--grid_size = 0.004
//--dot_percent = 0.1
//--show_radius = 0.04

float4 color;
float  grid_size;
float  dot_percent;
float  show_radius;

struct vsIn {
	float4 pos  : SV_Position;
	float3 norm : NORMAL0;
	float2 uv   : TEXCOORD0;
	float4 col  : COLOR0;
};
struct psIn {
	float4 pos       : SV_POSITION;
	float3 world_pos : TEXCOORD0;
	float3 model_pos : TEXCOORD1;
	half3  normal    : NORMAL0;
	half4  color     : COLOR0;
	uint   view_id   : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float4x4 world_mat = sk_inst[id].world;
	float3   scale     = float3(
		length(float3(world_mat._11,world_mat._12,world_mat._13)),
		length(float3(world_mat._21,world_mat._22,world_mat._23)),
		length(float3(world_mat._31,world_mat._32,world_mat._33)));
		
	o.model_pos = input.pos.xyz * scale;
	o.world_pos = mul(float4(input.pos.xyz, 1), world_mat).xyz;
	o.pos       = mul(float4(o.world_pos,   1), sk_viewproj[o.view_id]);
	o.color     = input.col * color * sk_inst[id].color;
	o.normal    = input.norm;
	return o;
}

float triplanar_dots(float3 normal, float3 position) {
	// Calculate the largest axis, and invert it. So a (0.8,0.2,0.2) will
	// become a (0,1,1)
	float3 n       = abs(normal);
	float  largest = max(n.x, max(n.y, n.z));
	float3 blend   = n != largest.xxx;
	
	// This can allow for smoother transitions between planes, but uses a high
	// value 'pow' call
	//float3 blend = pow(abs(normal), 100);
	//blend = 1 - blend/dot(blend, 1);
	
	// Find our position in the grid
	float3 cell_pos = fmod(abs(position), grid_size) / grid_size - 0.5;
	
	// Get the dot intensity by finding the distance on our blend axes in the
	// grid cell.
	return 1 - saturate((length(blend * cell_pos) - dot_percent) * 10);
}

float4 ps(psIn input) : SV_TARGET {
	float dist = sk_finger_distance(input.world_pos);
	if (dist > show_radius) discard;
	
	float dots     = triplanar_dots(input.normal, input.model_pos);
	float dist_pct = saturate(dist/show_radius);
	float ring     = pow(dist_pct, 22) * saturate((0.99-dist_pct) * 10);
	return input.color * (1-dist_pct) * dots + ring;
}