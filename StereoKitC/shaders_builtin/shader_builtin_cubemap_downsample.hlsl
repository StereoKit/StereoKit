//--name = sk/cubemap_downsample
// Box filters a source cubemap into the destination's mip 0 via
// skr_renderer_blit, one face per multiview layer (SV_ViewID). Handles any
// reduction ratio in one pass, so a reflection's base level can come
// straight from a much larger environment cubemap.

//--size_ratio = 1
float size_ratio; // source face size / destination face size

//--source = cubemap
TextureCube<float4> source   : register(t0);
SamplerState        source_s : register(s0);

struct psIn {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

// Convert UV coordinates to cubemap direction for a specific face
// Faces: +X=0, -X=1, +Y=2, -Y=3, +Z=4, -Z=5
float3 uv_to_direction(float2 uv, uint face) {
	float2 ndc = uv * 2.0 - 1.0;
	if (face == 0) return float3( 1.0, -ndc.y, -ndc.x);
	if (face == 1) return float3(-1.0, -ndc.y,  ndc.x);
	if (face == 2) return float3( ndc.x,  1.0,  ndc.y);
	if (face == 3) return float3( ndc.x, -1.0, -ndc.y);
	if (face == 4) return float3( ndc.x, -ndc.y,  1.0);
	               return float3(-ndc.x, -ndc.y, -1.0);
}

// Fullscreen triangle vertex shader, with the face index from SV_ViewID.
// skr_renderer_blit uses a negative-height (D3D-style) viewport where clip +1
// is the top row, so uv.y flips here, same as the equirect and mipgen paths.
psIn vs(uint id : SV_VertexID) {
	psIn output;
	output.uv  = float2(id & 2, (id << 1) & 2);
	output.pos = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
	return output;
}

// Each bilinear tap on a texel corner averages a 2x2 source block, so an NxN
// grid at 2-texel spacing box filters the destination texel's whole footprint.
// Undersampling would alias, turning bright spots into sparkle clusters.
float4 ps(psIn input, uint face : SV_ViewID) : SV_Target {
	if (size_ratio <= 2.0) {
		return float4(source.SampleLevel(source_s, uv_to_direction(input.uv, face), 0).rgb, 1);
	}

	// Past the 64:1 tap cap, taps spread out and alias mildly.
	int    n     = min((int)ceil(size_ratio * 0.5), 32);
	float  width = fwidth(input.uv.x); // destination texel size, in uv
	float3 color = 0;
	for (int y = 0; y < n; y++) {
	for (int x = 0; x < n; x++) {
		float2 o = ((float2(x, y) + 0.5) / n - 0.5) * width;
		color += source.SampleLevel(source_s, uv_to_direction(input.uv + o, face), 0).rgb;
	} }
	return float4(color / (n * n), 1);
}
