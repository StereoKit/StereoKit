//--name = skt/postfx_vignette
//--strength = 0.4

// Post-process shaders read the scene through a pixel-local SubpassInput
// named 'color', and draw as a bufferless fullscreen triangle from SV_VertexID.
float strength;

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
	float2 p = input.uv - 0.5;
	float  v = saturate(1 - dot(p, p) * strength * 4);
	return float4(c.rgb * v, c.a);
}
