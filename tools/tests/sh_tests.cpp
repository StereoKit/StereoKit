// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

#include "sh_tests.h"

#include <stereokit.h>

// Internal SH helpers, reached through StereoKitC's public include root.
// These run on exact hand-built coefficients, which the C# tests can't
// reach: their cubemap round trip re-projects everything physically.
#include <spherical_harmonics.h>

#include <math.h>

using namespace sk;

///////////////////////////////////////////

static int sht_failures = 0;

#define SHT_CHECK(condition, description) do { \
	if (condition) { log_infof("[sh_test] pass: %s", description); } \
	else           { log_errf ("[sh_test] FAIL: %s", description); sht_failures += 1; } \
	} while (0)

static float sht_lum(color128 c) { return c.r*0.2126f + c.g*0.7152f + c.b*0.0722f; }

///////////////////////////////////////////

int sh_tests_run() {
	sht_failures = 0;
	log_info("[sh_test] Running spherical harmonics tests");

	vec3 dir   = vec3_normalize({ 1, 2, 0.5f });
	vec3 color = { 1.0f, 0.8f, 0.6f };

	// A light projected by sh_add should extract back out exactly: same
	// direction, same color, no windowing involved.
	spherical_harmonics_t sh = {};
	sh_add(sh, dir, color);
	sh_light_t light = sh_dominant_light(sh);
	SHT_CHECK(vec3_dot(light.dir_to, dir) > 0.9999f,        "dominant light direction round trips");
	SHT_CHECK(fabsf(light.color.r - color.x) < 0.001f &&
	          fabsf(light.color.g - color.y) < 0.001f &&
	          fabsf(light.color.b - color.z) < 0.001f,      "dominant light color round trips");

	// Subtracting that light should zero the SH entirely.
	spherical_harmonics_t remainder = sh;
	sh_subtract_light(remainder, light);
	float residue = 0;
	for (int32_t i = 0; i < 9; i++)
		residue += fabsf(remainder.coefficients[i].x) + fabsf(remainder.coefficients[i].y) + fabsf(remainder.coefficients[i].z);
	SHT_CHECK(residue < 0.001f,                             "subtracting the dominant light zeroes its own projection");

	// A scaled direction subtracts the same light as a normalized one.
	spherical_harmonics_t remainder_scaled = sh;
	sh_light_t            light_scaled     = { light.dir_to * 3.0f, light.color };
	sh_subtract_light(remainder_scaled, light_scaled);
	float scaled_residue = 0;
	for (int32_t i = 0; i < 9; i++)
		scaled_residue += fabsf(remainder_scaled.coefficients[i].x) + fabsf(remainder_scaled.coefficients[i].y) + fabsf(remainder_scaled.coefficients[i].z);
	SHT_CHECK(scaled_residue < 0.001f,                      "subtract_light normalizes an unnormalized direction");

	// A uniform environment has no dominant light, and the direction falls
	// back to straight up.
	spherical_harmonics_t uniform = {};
	uniform.coefficients[0] = { 1, 1, 1 };
	SHT_CHECK(sht_lum(sh_dominant_light(uniform).color) < 0.0001f,         "a uniform environment extracts no light");
	SHT_CHECK(vec3_dot(sh_dominant_dir_to(uniform), { 0, 1, 0 }) > 0.999f, "an empty linear band defaults to a light from above");

	// Pathological SH, a linear band far larger than its DC term supports.
	// The subtraction must clamp so band 0 stays non-negative, and report
	// the smaller light it actually removed.
	spherical_harmonics_t skewed = {};
	sh_add(skewed, dir, color);
	skewed.coefficients[0] = skewed.coefficients[0] * 0.25f;
	spherical_harmonics_t clamped = skewed;
	sh_light_t removed = sh_subtract_light(clamped, sh_dominant_light(skewed));
	SHT_CHECK(clamped.coefficients[0].x >= -0.0001f &&
	          clamped.coefficients[0].y >= -0.0001f &&
	          clamped.coefficients[0].z >= -0.0001f,        "a clamped subtraction keeps band 0 non-negative");
	SHT_CHECK(removed.color.r < color.x,                    "a clamped subtraction reports the smaller removed light");

	// A light over a uniform base splits without disturbing the far side:
	// irradiance opposite the light stays put, and facing it drops.
	spherical_harmonics_t env = uniform;
	sh_add(env, dir, color);
	spherical_harmonics_t split = env;
	sh_subtract_light(split, sh_dominant_light(env));
	float away_before = sht_lum(sh_lookup(env,   -dir));
	float away_after  = sht_lum(sh_lookup(split, -dir));
	SHT_CHECK(fabsf(away_after - away_before) < away_before * 0.25f,     "a split preserves lighting opposite the light");
	SHT_CHECK(sht_lum(sh_lookup(split, dir)) < sht_lum(sh_lookup(env, dir)), "a split removes energy facing the light");

	log_infof("[sh_test] %d failure(s)", sht_failures);
	return sht_failures;
}
