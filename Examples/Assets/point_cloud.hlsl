#include <stereokit.hlsli>

//--name = app/point_cloud

//--point_size = 0.01
float point_size;
//--screen_size = 0
float screen_size;

// These inputs pair with the custom vertex format in DemoPointCloud.cs.
// Each point is 4 sequential verts, quad corners come from the vert id.
struct vsIn {
	float4 pos  : SV_POSITION;
	float4 color: COLOR0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float4 color : COLOR0;
};

// Corner ids 0,1,2,3 wind clockwise from the top left:
// (-.5,.5) (.5,.5) (.5,-.5) (-.5,-.5)
float2 corner_off(uint vert_id) {
	uint corner = vert_id % 4;
	return float2(
		((corner + 1) & 2) ? .5 : -.5,
		( corner      & 2) ?-.5 :  .5);
}

psIn vs(vsIn input, uint vert_id : SV_VertexID, sk_ids_t ids) {
	psIn o;

	float2 off   = corner_off(vert_id);
	float4 world = mul(input.pos, sk_inst[ids.inst].world);
	float4 view  = mul(world, sk_view[ids.view]);
	if (screen_size <= 0.1)
		view.xy = point_size * off + view.xy;
	o.pos        = mul(view, sk_proj[ids.view]);
	o.color      = input.color;

	if (screen_size > 0.1) {
		// sk_aspect_ratio, not manual element access: Adreno's multiview
		// linker fails when ViewIndex isn't terminal in an access chain.
		float  aspect = sk_aspect_ratio(ids.view);
		o.pos.xy = ( point_size * off / float2(aspect,1) ) *o.pos.w + o.pos.xy;
	}

	return o;
}
float4 ps(psIn input) : SV_TARGET{
	return input.color;
}