#include "stereokit.h"

#include "assets.h"
#include "render.h"
#include "d3d.h"
#include "win32.h"
#include "openxr.h"

#include <thread> // sleep_for

using namespace std;

sk_runtime_ sk_runtime = sk_runtime_flatscreen;
bool sk_focused = true;
bool sk_run     = true;

float  sk_timevf = 0;
double sk_timev  = 0;
double sk_time_start    = 0;
double sk_timev_elapsed  = 0;
float  sk_timev_elapsedf = 0;
long long sk_timev_raw = 0;

tex2d_t    sk_default_tex;
shader_t   sk_default_shader;
material_t sk_default_material;
void sk_create_defaults();
void sk_destroy_defaults();

bool sk_init(const char *app_name, sk_runtime_ runtime) {
	sk_runtime = runtime;

	d3d_init();
	sk_create_defaults();
	
	bool result = true;
	switch (sk_runtime) {
	case sk_runtime_flatscreen:   result = win32_init (app_name); break;
	case sk_runtime_mixedreality: result = openxr_init(app_name); break;
	}
	if (!result)
		return false;

	render_initialize();
	return true;
}

void sk_shutdown() {
	render_shutdown();
	switch (sk_runtime) {
	case sk_runtime_flatscreen:   win32_shutdown (); break;
	case sk_runtime_mixedreality: openxr_shutdown(); break;
	}
	//assets_shutdown_check();
	sk_destroy_defaults();
	d3d_shutdown();
}

void sk_update_timer() {
	FILETIME time;
	GetSystemTimePreciseAsFileTime(&time);
	sk_timev_raw = (LONGLONG)time.dwLowDateTime + ((LONGLONG)(time.dwHighDateTime) << 32LL);
	double time_curr = sk_timev_raw / 10000000.0;

	if (sk_time_start == 0)
		sk_time_start = time_curr;
	double new_time = time_curr - sk_time_start;
	sk_timev_elapsed  = new_time - sk_timev;
	sk_timev          = new_time;
	sk_timev_elapsedf = (float)sk_timev_elapsed;
	sk_timevf         = (float)sk_timev;
}

bool sk_step(void (*app_update)(void)) {
	switch (sk_runtime) {
	case sk_runtime_flatscreen:   win32_step_begin (); break;
	case sk_runtime_mixedreality: openxr_step_begin(); break;
	}
	
	sk_update_timer();
	app_update();

	d3d_render_begin();
	switch (sk_runtime) {
	case sk_runtime_flatscreen:   win32_step_end (); break;
	case sk_runtime_mixedreality: openxr_step_end(); break;
	}
	d3d_render_end();

	this_thread::sleep_for(chrono::milliseconds(sk_focused?1:250));

	return sk_run;
}

float  sk_timef        (){ return sk_timevf; };
double sk_time         (){ return sk_timev; };
float  sk_time_elapsedf(){ return sk_timev_elapsedf; };
double sk_time_elapsed (){ return sk_timev_elapsed; };

void sk_create_defaults() {
	sk_default_tex = tex2d_create("default/tex2d");
	uint8_t tex_colors[4 * 4];
	memset(tex_colors, 255, sizeof(uint8_t) * 4 * 4);
	tex2d_set_colors(sk_default_tex, 2, 2, tex_colors);

	sk_default_shader = shader_create("default/shader", R"_(cbuffer TransformBuffer : register(b0) {
	float4x4 world;
	float4x4 viewproj;
};
cbuffer ParamBuffer : register(b1) {
	// [param] color color
	float4 _color;
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

// [texture] diffuse
Texture2D tex : register(t0);
SamplerState tex_sampler;

// [texture] emission
Texture2D tex_emission : register(t1);
SamplerState tex_e_sampler;

psIn vs(vsIn input) {
	psIn output;
	output.pos = input.pos;
	output.pos = mul(float4(input.pos.xyz, 1), world);
	output.pos = mul(output.pos, viewproj);

	float3 normal = normalize(mul(float4(input.norm, 0), world).xyz);

	output.uv    = input.uv;
	output.color = lerp(float3(0.1,0.1,0.2), float3(1,1,1), saturate(dot(normal, normalize(float3(1,2,1)))));
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float3 col = tex.Sample(tex_sampler, input.uv).rgb;
	float3 em  = tex_emission.Sample(tex_e_sampler, input.uv).rgb;

	col = col * input.color * _color.rgb;
	col += em;

	return float4(col, _color.a); 
})_");

	sk_default_material = material_create("default/material", sk_default_shader);
	material_set_texture(sk_default_material, "diffuse", sk_default_tex);
}

void sk_destroy_defaults() {
	material_release(sk_default_material);
	shader_release  (sk_default_shader);
	tex2d_release   (sk_default_tex);
}