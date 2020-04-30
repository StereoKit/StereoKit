// [name] app/floor

#include <stereokit>

cbuffer ParamBuffer : register(b2) {
	// [param] color color {1, 1, 1, 1}
	float4 _color;
	// [param] vector radius {5, 10, 0, 0}
	float4 radius;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
	float4 col  : COLOR;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float4 color : COLOR0;
	float4 world : TEXCOORD0;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn output;
	output.world = mul(input.pos,    sk_inst[id].world);
	output.pos   = mul(output.world, sk_viewproj[sk_inst[id].view_id]);

	float3 normal = normalize(mul(input.norm, (float3x3)sk_inst[id].world));

	output.view_id = sk_inst[id].view_id;
	output.color   = _color * input.col * sk_inst[id].color;
	output.color.rgb *= Lighting(normal);
	return output;
}
float4 ps(psIn input) : SV_TARGET{
	// This line algorithm is inspired by :
	// http://madebyevan.com/shaders/grid/

	// Minor grid lines
	float2 step = 1 / fwidth(input.world.xz);
	float2 grid = abs(frac(input.world.xz - 0.5) - 0.5) * step; // minor grid lines
	float  dist = sqrt(dot(input.world.xz, input.world.xz)); // transparency falloff
	float  size = min(grid.x, grid.y);
	float  val = 1.0 - min(size, 1.0);
	val *= saturate(1 - ((dist - radius.x)/radius.y));

	// Major axis lines
	const float extraThickness = 0.5;
	float2 axes = (abs(input.world.xz)) * step - extraThickness;
	size = min(axes.x, axes.y);
	float  axisVal = 1.0 - min(size, 1.0);
	axisVal *= saturate(1 - ((dist - radius.x*1.5)/(radius.y*1.5)));

	// combine, and drop transparent pixels
	val = max(val, axisVal);
	if (val <= 0) discard;
		
	return float4(0, 0, 0, val) * input.color;
}