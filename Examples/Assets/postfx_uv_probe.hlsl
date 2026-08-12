//--name = app/postfx_uv_probe

// Writes its own interpolated uv out as color, so a readback can tell whether
// the postfx subpass spans the pass viewport or the whole attachment.

[[vk::input_attachment_index(0)]] SubpassInput<float4> color;

struct psIn {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

psIn vs(uint id : SV_VertexID) {
	psIn o;
	float2 uv = float2(id & 2, (id << 1) & 2);
	o.pos = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
	o.uv  = uv;
	return o;
}

float4 ps(psIn input) : SV_TARGET {
	float4 c = color.SubpassLoad();
	return float4(input.uv, c.b, 1);
}
