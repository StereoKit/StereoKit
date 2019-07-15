cbuffer TransformBuffer : register(b0) {
	float4x4 world;
	float4x4 viewproj;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
	float2 uv   : TEXCOORD0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float3 color : COLOR0;
	float2 uv    : TEXCOORD0;
};

Texture2D tex;
SamplerState tex_sampler;

psIn vs(vsIn input) {
	psIn output;
	output.pos = input.pos;
	output.pos = mul(float4(input.pos.xyz, 1), world);
	output.pos = mul(output.pos, viewproj);

	float3 normal = normalize(mul(float4(input.norm, 0), world).xyz);

	output.uv    = input.uv;
	output.color = lerp(float3(0,0,0.1), float3(1,1,1), saturate(dot(normal, float3(0,1,0))));
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float3 col = tex.Sample(tex_sampler, input.uv).rgb;
	return float4(input.color * col, 1); 
}