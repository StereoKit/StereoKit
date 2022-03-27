#include <stereokit.hlsli>

//--name = app/point_cloud

//--point_size = 0.01
float point_size;
//--screen_size = 0
float screen_size;

struct vsIn {
	float4 pos  : SV_POSITION;
	float2 off  : TEXCOORD0;
	float4 color: COLOR0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float2 uv    : TEXCOORD0;
	float4 color : COLOR0;
	uint view_id : SV_RenderTargetArrayIndex;
};

psIn vs(vsIn input, uint id : SV_InstanceID) {
	psIn o;
	o.view_id = id % sk_view_count;
	id        = id / sk_view_count;

	float4 world = mul(input.pos, sk_inst[id].world);
	float4 view  = mul(world, sk_view[o.view_id]);
	if (screen_size <= 0.1)
		view.xy = point_size * input.off + view.xy;
	o.pos        = mul(view, sk_proj[o.view_id]); 
	o.uv         = input.off + 0.5;
	o.color      = input.color;

	if (screen_size > 0.1) {
		float  aspect = sk_proj[o.view_id]._m11 / sk_proj[o.view_id]._m00;
		o.pos.xy = ( point_size * input.off / float2(aspect,1) ) *o.pos.w + o.pos.xy;
	}

	return o;
}
float4 ps(psIn input) : SV_TARGET{
	return input.color;
}