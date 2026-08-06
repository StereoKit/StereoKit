//--name = sk/cubemap_ggx
// Specular prefilter for IBL: builds the roughness mip chain that
// stereokit_pbr.hlsli samples via Lazarov's roughness -> mip curve.
// Runs through sk_renderer's skr_tex_generate_mips render path: each
// destination mip convolves the previous one, 'src_tex' is a single-mip view
// of that previous mip (nothing else is readable), the parameters below
// arrive per destination mip, and SV_ViewID selects the cubemap face.
//
// The convolution is a deterministic spherical-gaussian quadrature, not
// stochastic GGX importance sampling. Taps stay within one source texel, so
// concentrated sources like an HDRI sun are always fully averaged, where
// stochastic sampling hits them at random and sparkles. Cascading small
// gaussians reproduces the GGX core (Manson & Sloan); the heavy tail is
// lost, but mip chain truncation drops it anyway.

static const float PI = 3.14159265359;

uint2 src_size;      // Source mip dimensions
uint2 dst_size;      // Destination mip dimensions
uint  src_mip_level; // Mip index the source view refers to
uint  mip_max;       // Total mip count of the texture
uint  _pad[2];

TextureCube<float4> src_tex     : register(t1);
SamplerState        src_sampler : register(s1);

struct psIn {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

// Convert UV coordinates to cubemap direction for a specific face
// Faces: +X=0, -X=1, +Y=2, -Y=3, +Z=4, -Z=5
float3 uv_to_direction(float2 uv, uint face) {
	float2 ndc = uv * 2.0 - 1.0;
	if (face == 0) return normalize(float3( 1.0, -ndc.y, -ndc.x));
	if (face == 1) return normalize(float3(-1.0, -ndc.y,  ndc.x));
	if (face == 2) return normalize(float3( ndc.x,  1.0,  ndc.y));
	if (face == 3) return normalize(float3( ndc.x, -1.0, -ndc.y));
	if (face == 4) return normalize(float3( ndc.x, -ndc.y,  1.0));
	               return normalize(float3(-ndc.x, -ndc.y, -1.0));
}

// Fullscreen triangle vertex shader, with the face index from SV_ViewID.
// Mipgen renders with a negative-height (D3D-style) viewport where clip +1 is
// the top row, so uv.y flips here. Without it, mips store vertically mirrored.
psIn vs(uint id : SV_VertexID) {
	psIn output;
	output.uv  = float2(id & 2, (id << 1) & 2);
	output.pos = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
	return output;
}

float4 ps(psIn input, uint face : SV_ViewID) : SV_Target {
	float3 n = uv_to_direction(input.uv, face);

	// Invert stereokit_pbr.hlsli's roughness -> mip curve (mip_norm =
	// r * (1.7 - 0.7*r)), so each mip stores the roughness it's sampled at.
	float norm_curr = float(src_mip_level + 1) / float(mip_max - 1);
	float norm_prev = float(src_mip_level    ) / float(mip_max - 1);
	float r_curr    = (1.7 - sqrt(2.89 - 2.8 * norm_curr)) / 1.4;
	float r_prev    = (1.7 - sqrt(2.89 - 2.8 * norm_prev)) / 1.4;

	// Angular std dev of each mip's lobe: treating GGX as vMF-like gaussians,
	// std tracks alpha (= r^2), and convolution sums variance.
	float s_rough_curr = r_curr * r_curr * 0.7071;
	float s_rough_prev = r_prev * r_prev * 0.7071;

	// Band-limit floor: content sharper than a mip's own texel can't display
	// smoothly, an HDR sun in one texel magnifies into square bilinear stars.
	// A max, not a sum, so representable lobes still store their exact target.
	const float band = 0.6; // min lobe std dev, in a mip's own texels
	// The base level only carries the box downsample's own blur, std 1/sqrt(12)
	// texels. Assuming 'band' there over-subtracts and under-blurs mip 1.
	float band_src     = src_mip_level == 0 ? 0.289 : band;
	float texel_src    = (PI * 0.5) / src_size.x;
	float texel_dst    = (PI * 0.5) / dst_size.x;
	float target_curr2 = max(s_rough_curr * s_rough_curr, (band * texel_dst) * (band * texel_dst));
	float target_prev2 = max(s_rough_prev * s_rough_prev, (band_src * texel_src) * (band_src * texel_src));
	float sigma        = sqrt(max(target_curr2 - target_prev2, 1e-8));

	// Gaussian tap grid on the tangent plane, out to 4 sigma. It widens from
	// 9x9 to keep taps within one source texel, or the lattice beats against
	// the texel grid and prints source resolution into the result. At the
	// 17x17 cap the extent shrinks instead, truncation degrades more gently.
	float extent  = min(4.0 * sigma, 1.2); // cap keeps the lobe on the front hemisphere
	int   half_n  = 4;
	float spacing = extent / 4.0;
	if (spacing > texel_src) {
		half_n  = min((int)ceil(extent / texel_src), 8);
		spacing = extent / half_n;
		if (spacing > texel_src) {
			spacing = texel_src;
			extent  = spacing * half_n;
		}
	}

	// Build orthonormal basis around n.
	float3 up      = abs(n.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 tangent = normalize(cross(up, n));
	float3 bitang  = cross(n, tangent);

	// HDR sources image the kernel itself into the mip, so the window must be
	// radial (a square boundary shows as a box) and fade smoothly (no ring).
	float  R2    = extent * extent;
	float  gauss = -0.5 / (sigma * sigma);
	float3 color = 0;
	float  total = 0;
	for (int y = -half_n; y <= half_n; y++) {
	for (int x = -half_n; x <= half_n; x++) {
		float2 o  = float2(x, y) * spacing;
		float  r2 = dot(o, o);
		if (r2 >= R2) continue;
		// The exponential map lands taps at their exact lattice angle (radial
		// kernel and window, no squaring along lattice diagonals), and sinc
		// is also each cell's solid angle, weighting the gaussian for free.
		float  r    = sqrt(r2);
		float  sinc = r > 1e-4 ? sin(r) / r : 1.0;
		float  w    = exp(r2 * gauss) * (1.0 - smoothstep(0.4 * R2, R2, r2)) * sinc;
		float3 d    = n * cos(r) + (o.x * tangent + o.y * bitang) * sinc;
		color      += src_tex.SampleLevel(src_sampler, d, 0).rgb * w;
		total      += w;
	} }

	return float4(color / total, 1);
}
