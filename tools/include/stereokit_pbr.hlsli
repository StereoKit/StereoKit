#ifndef _STEREOKIT_PBR_HLSLI
#define _STEREOKIT_PBR_HLSLI

#include <stereokit.hlsli>

///////////////////////////////////////////

float skpbr_mip_level(float ndotv) {
	float2 dx    = ddx(ndotv * sk_cubemap_i.x);
	float2 dy    = ddy(ndotv * sk_cubemap_i.y);
	float  delta = max(dot(dx, dx), dot(dy, dy));
	return 0.5 * log2(delta);
}

///////////////////////////////////////////

float3 skpbr_fresnel_schlick_roughness(float ndotv, float3 F0, float roughness) {
	return F0 + (max(1 - roughness, F0) - F0) * pow(1.0 - ndotv, 5.0);
}

///////////////////////////////////////////

// See: https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
float2 skpbr_brdf_appx(half roughness, half ndotv) {
	const half4 c0   = { -1, -0.0275, -0.572,  0.022 };
	const half4 c1   = {  1,  0.0425,  1.04,  -0.04  };
	half4       r    = roughness * c0 + c1;
	half        a004 = min(r.x * r.x, exp2(-9.28 * ndotv)) * r.x + r.y;
	half2       AB   = half2(-1.04, 1.04) * a004 + r.zw;
	return AB;
}

///////////////////////////////////////////

float4 skpbr_shade(float4 albedo, float3 irradiance, float3 ao, float metal, float rough, float3 view_dir, float3 surface_normal) {
	float3 view        = normalize(view_dir);
	float3 reflection  = reflect(-view, surface_normal);
	float  ndotv       = max(0, dot(surface_normal, view));

	float3 F0 = lerp(0.04, albedo.rgb, metal);
	float3 F  = skpbr_fresnel_schlick_roughness(ndotv, F0, rough);
	float3 kS = F;

	float mip = (1 - pow(1 - rough, 2)) * sk_cubemap_i.z;
	mip = max(mip, skpbr_mip_level(ndotv));
	float3 prefilteredColor = sk_cubemap.SampleLevel(sk_cubemap_s, reflection, mip).rgb;
	float2 envBRDF          = skpbr_brdf_appx(rough, ndotv);
	float3 specular         = prefilteredColor * (F * envBRDF.x + envBRDF.y);

	float3 kD = 1 - kS;
	kD *= 1.0 - metal;

	float3 diffuse = albedo.rgb * irradiance * ao;
	float3 color   = (kD * diffuse + specular*ao);

	return float4(color, albedo.a);
}

///////////////////////////////////////////

#endif