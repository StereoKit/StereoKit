cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view;
	float4x4 sk_proj;
	float4x4 sk_viewproj;
	float4   sk_light;
	float4   sk_light_color;
	float4   sk_camera_pos;
	float4   sk_camera_dir;
};
cbuffer TransformBuffer : register(b1) {
	float4x4 sk_world;
};
cbuffer ParamBuffer : register(b2) {
	// [param] color color
	float4 _color;
	// [param] float metallic
	float metallic;
	// [param] float roughness
	float roughness;
};
struct vsIn {
	float4 pos   : SV_POSITION;
	float3 norm  : NORMAL;
	float2 uv    : TEXCOORD0;
    float3 color : COLOR0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float3 color : COLOR0;
    float3 normal: NORMAL;
	float2 uv    : TEXCOORD0;
	float3 world : TEXCOORD1;
};

// [texture] diffuse
Texture2D tex : register(t0);
SamplerState tex_sampler;

// [texture] emission
Texture2D tex_emission : register(t1);
SamplerState tex_e_sampler;

// [texture] metal
Texture2D tex_metal : register(t2);
SamplerState tex_metal_sampler;

float DistributionGGX(float3 normal, float3 half_vec, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float NdotL, float NdotV, float roughness);
float3 FresnelSchlick(float NdotV, float3 surfaceColor, float metalness);

psIn vs(vsIn input) {
	psIn output;
	output.world = mul(float4(input.pos.xyz, 1), sk_world).xyz;
	output.pos   = mul(float4(output.world, 1), sk_viewproj);

	output.normal = normalize(mul(float4(input.norm, 0), sk_world).xyz);
	output.uv     = input.uv;
    output.color  = input.color;
	return output;
}

float4 ps(psIn input) : SV_TARGET {
	float3 albedo      = tex         .Sample(tex_sampler,       input.uv).rgb;
	float3 emissive    = tex_emission.Sample(tex_e_sampler,     input.uv).rgb;
	float4 metal_rough = tex_metal   .Sample(tex_metal_sampler, input.uv); // b is metallic, rough is g

	float metal = metal_rough.b * metallic;
	float rough = max(metal_rough.g * roughness, 0.001);

    float3 normal   = normalize(input.normal);
    float3 light    = -normalize(sk_light.xyz);
    float3 half_vec = normalize(light + normal);
    float3 view     = normalize(sk_camera_pos.xyz - input.world);
    float  NdotL    = (dot(normal,light));
    float  NdotV    = (dot(normal,view));

    // Lighting an object is a combination of two types of light reflections,
    // a diffuse reflection, and a specular reflection. These reflections
    // are approximated using functions called BRDFs (Bidirectional Reflectance
    // Distribuition Function).
    
    // For the diffuse BRDF, we'll use Lambert's, since it's simple, effective,
    // and research has said you can't do much better for cheap!
    // https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
    //
    // L = c_diffuse/PI * c_light(n*l)
    //
    // Where:
    // c_diffuse is the diffuse color
    // c_light is the color of the light
    // n is the surface normal
    // l is the vector from the surface to the light position (light direction)
    
    float3 diffuse = (albedo/3.14159) * sk_light_color.rgb*saturate(NdotL);

    // This formula is a common format for the specular BRDF,
    // Unreal calls this a 'generalized microfacet model', as different functions
    // can easily be swapped out for D, F, and G
    // https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
    //
    //          D(h) F(l,h) G(l,v,h)
    // f(l,v) = --------------------
    //              4(n*l)(n*v)
    //
    // Where:
    // D() is specular distribution function
    // F() is fresnel function
    // G() is geometric shadowing function
    // n is the surface normal
    // l is the vector from the surface to the light position (light direction)
    // v is view direction, or direction from the surface to the camera
    // h is the half vector, h = normalize(l + v)
    float3 specular = 
        (DistributionGGX(normal, half_vec, rough) * 
         FresnelSchlick (NdotV, albedo, metal) * 
         GeometrySmith  (max(NdotL, 0), max(NdotV, 0), rough)) 
         //GeometrySchlickGGX(max(NdotV, 0), rough))

        /

        (4 * NdotL * NdotV);

    //float3 result = 0;
    //result = specular;
    //result = GeometrySchlickGGX(NdotV, rough);
    //result = DistributionGGX(normal, half_vec, rough);
    //result = FresnelSchlick(NdotV, albedo, metal);
    //result = metal;
    //return float4(result, _color.a);
	return float4(diffuse + emissive + specular, _color.a); 
}


// https://learnopengl.com/PBR/Theory
// This is a normal distribution function called, Trowbridge-Reitz GGX
float DistributionGGX(float3 normal, float3 half_vec, float roughness)
{
    float roughness2 = roughness*roughness;
    float NdotH      = max(dot(normal, half_vec), 0.0);
    float NdotH2     = NdotH*NdotH;
	
    float nom    = roughness2;
    float denom  = (NdotH2 * (roughness2 - 1.0) + 1.0);
    denom        = 3.14159 * denom * denom;
	
    return nom / denom;
}

// These are Geometry functions
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - roughness) + roughness;
	
    return nom / denom;
}
float GeometrySmith(float NdotL, float NdotV, float roughness)
{
    //float NdotV = max(dot(normal, view), 0.0);
    //float NdotL = max(dot(normal, light_dir), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

// and Fresnel
float3 FresnelSchlick(float NdotV, float3 surfaceColor, float metalness)
{
	float3 F0 = 0.04;//float3(0.04);
	F0        = lerp(F0, surfaceColor, metalness);
    return F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);
}