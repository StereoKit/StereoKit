//--name = app/postfx_math
//--mul_color = 1, 1, 1
//--add_color = 0, 0, 0

// A parameterized scene-color transform: out = in * mul + add. Handy for
// testing post-process chains, since chain order changes the math result.
float3 mul_color;
float3 add_color;

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
	return float4(c.rgb * mul_color + add_color, c.a);
}
