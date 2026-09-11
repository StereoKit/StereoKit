#ifndef _STEREOKIT_PBR_HLSLI
#define _STEREOKIT_PBR_HLSLI

#include <stereokit.hlsli>

///////////////////////////////////////////

min16float3 sk_pbr_fresnel_schlick_roughness(min16float ndotv, min16float3 F0, min16float roughness) {
	// Exact pow(1 - ndotv, 5) by squaring, like Filament's pow5. Three half muls
	// beat the classic exp2 fit now that transcendentals don't co-issue for free.
	min16float f  = 1.0h - ndotv;
	min16float f2 = f * f;
	return F0 + max(1.0h - roughness - F0, 0.0h) * (f2 * f2 * f);
}

///////////////////////////////////////////

// Karis, "Real Shading in Unreal Engine 4" (SIGGRAPH 2013)
// See: https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
min16float2 sk_pbr_brdf_appx(min16float roughness, min16float ndotv) {
	const min16float4 c0   = { -1, -0.0275, -0.572,  0.022 };
	const min16float4 c1   = {  1,  0.0425,  1.04,  -0.04  };
	min16float4       r    = roughness * c0 + c1;
	min16float        a004 = min(r.x * r.x, exp2(-9.28 * ndotv)) * r.x + r.y;
	min16float2       AB   = min16float2(-1.04, 1.04) * a004 + r.zw;
	return AB;
}

///////////////////////////////////////////

// log2 read straight off the float exponent field, good to ~0.03. Plenty for
// picking a mip, and keeps a transcendental out of the pre-fetch dep chain.
float sk_log2_fast(float x) { return (float)asint(x) * 1.1920929e-7 - 126.9426950; }

///////////////////////////////////////////

min16float4 sk_pbr_shade(min16float4 albedo, min16float3 irradiance, min16float ao, min16float metal, min16float rough, float3 view_dir, min16float3 surface_normal) {
	// View direction and reflection must stay float for precision
	float3     view       = normalize(view_dir);
	float3     normal     = (float3)surface_normal;
	float      ndotv_full = dot(normal, view);
	float3     reflection = normal * (2*ndotv_full) - view; // reflect(-view, normal)
	min16float ndotv      = (min16float)max(0, ndotv_full);

	// Pre-compute specular AA kernel from screen-space normal derivatives.
	// All ddx/ddy calls stay in the same WQM region; only the sqrt is
	// deferred until after the cubemap fetch is issued.
	// Tokuyoshi, "Improved Geometric Specular Antialiasing" (JCGT 2021)
	min16float3 dndu            = ddx(surface_normal);
	min16float3 dndv            = ddy(surface_normal);
	min16float  variance        = 0.25h * (dot(dndu, dndu) + dot(dndv, dndv));
	min16float  kernelRoughness = min(variance, 0.18h);

	// Issue cubemap fetch from pre-AA roughness to shorten the dependent
	// read chain, removing v_sqrt (~16 clk) from the critical path.
	// The mip is an approximation anyway (Lagarde 2014); Tokuyoshi's
	// kernel only adds roughness at silhouette edges where the pre-filtered
	// map / BRDF lobe mismatch is already largest.
	// Lazarov, "Getting More Physical in Call of Duty: Black Ops II"
	// (SIGGRAPH 2013)
	float mip = (float)(rough * (1.7h - 0.7h * rough)) * sk_cubemap_i.z;
	// Footprint clamp: variance already tracks the normal's angular step per
	// pixel, and sk_cubemap_i.w packs the constants (texel density, reflection
	// doubling). Catches glint aliasing on curvature that roughness misses.
	mip = max(mip, 0.5 * sk_log2_fast((float)variance) + sk_cubemap_i.w);
	min16float3  prefilteredColor = (min16float3)sk_cubemap.SampleLevel(sk_cubemap_s, reflection, mip).rgb;

	// Apply specular AA after the cubemap is in flight, so sqrt runs hidden
	// under cubemap memory latency instead of extending the dep chain.
	rough = sqrt(saturate(rough * rough + kernelRoughness));

	min16float3 F0 = lerp(0.04h, albedo.rgb, metal);
	min16float3 F  = sk_pbr_fresnel_schlick_roughness(ndotv, F0, rough);

	min16float2 envBRDF  = sk_pbr_brdf_appx(rough, ndotv);
	min16float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

	// Multi-scattering energy compensation: recovers energy lost to
	// inter-reflections at high roughness, brightening rough metals.
	// Fdez-Aguera, "A Multiple-Scattering Microfacet Model for Real-Time
	// Image Based Lighting" (SIGGRAPH 2019)
	// 1/Ess - 1 in closed form: envBRDF.x + envBRDF.y is exactly 1 - 0.55*rough
	// (a004 cancels), which a cubic fits to 0.008 with no rcp and no ndotv term.
	min16float  boost              = rough * (0.64474h + rough * (-0.16204h + rough * 0.73952h));
	min16float3 energyCompensation = 1.0h + F0 * boost;
	specular *= energyCompensation;

	// Diffuse weight is plain Disney-style (1 - metal). Attenuating by Fresnel
	// too ((1 - F) * (1 - metal)) loses up to ~20% energy at mid-metallic.
	min16float kD = 1.0h - metal;

	min16float3 diffuse = albedo.rgb * irradiance;
	// Gotanda (tri-Ace, 2014): roughness-dependent retroreflection boost
	// approximating Disney/Burley diffuse for IBL. Near-free.
	// diffuse *= 1.0h + 0.5h * rough;
	min16float3 color   = (kD * diffuse + specular) * ao;

	return min16float4(color, albedo.a);
}

///////////////////////////////////////////

#endif

