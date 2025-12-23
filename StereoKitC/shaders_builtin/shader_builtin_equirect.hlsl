//--name = sk/equirect_convert
// Converts an equirectangular (lat/long) panorama to all 6 cubemap faces in a
// single draw call using instancing.

//--source = white

Texture2D    source   : register(t0);
SamplerState source_s : register(s0);

struct psIn {
	float4 pos   : SV_POSITION;
	float2 uv    : TEXCOORD0;
	uint   layer : SV_RenderTargetArrayIndex;
};

// Convert UV coordinates to cubemap direction for a specific face
float3 uv_to_direction(float2 uv, uint face) {
	// UV goes from 0 to 1, convert to -1 to 1
	float2 ndc = uv * 2.0 - 1.0;

	// Map to cubemap face direction
	// Faces: +X=0, -X=1, +Y=2, -Y=3, +Z=4, -Z=5
	float3 dir;
	if      (face == 0) { dir = float3( 1.0, -ndc.y, -ndc.x); } // +X
	else if (face == 1) { dir = float3(-1.0, -ndc.y,  ndc.x); } // -X
	else if (face == 2) { dir = float3( ndc.x,  1.0,  ndc.y); } // +Y
	else if (face == 3) { dir = float3( ndc.x, -1.0, -ndc.y); } // -Y
	else if (face == 4) { dir = float3( ndc.x, -ndc.y,  1.0); } // +Z
	else                { dir = float3(-ndc.x, -ndc.y, -1.0); } // -Z

	return normalize(dir);
}

// Convert 3D direction to equirectangular UV coordinates
float2 direction_to_equirect_uv(float3 dir) {
	dir = normalize(dir);

	// Calculate spherical coordinates
	// theta: azimuth angle (longitude) from -PI to PI
	// phi: polar angle (latitude) from 0 to PI
	float theta = atan2(dir.z, dir.x);
	float phi   = acos(dir.y);

	// Convert to UV coordinates (0 to 1)
	const float PI = 3.14159265359;
	float u = (theta / (2.0 * PI)) + 0.5;
	float v = phi / PI;

	return float2(u, v);
}

// Vertex shader - fullscreen triangle per cubemap face
psIn vs(uint id : SV_VertexID, uint instance_id : SV_InstanceID) {
	psIn output;

	// Generate fullscreen triangle
	output.uv    = float2(id & 2, (id << 1) & 2);
	output.pos   = float4(output.uv * 2.0 - 1.0, 0, 1);
	output.layer = instance_id; // Which cubemap face to render to

	return output;
}

// Pixel shader - sample from equirectangular map
float4 ps(psIn input) : SV_Target {
	// Get the 3D direction for this pixel on the cubemap face
	float3 dir = uv_to_direction(input.uv, input.layer);

	// Convert direction to equirectangular UV coordinates
	float2 equirect_uv = direction_to_equirect_uv(dir);

	// Sample from the equirectangular texture
	return source.Sample(source_s, equirect_uv);
}