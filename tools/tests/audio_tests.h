#pragma once

// Deterministic offline audio tests, run with `SKTests -audiotest`.
// Returns 0 on success, the number of failed checks otherwise.
int audio_tests_run();

// Live-device concurrency stress, run with `SKTests -audiostress`. Meant for
// TSan/ASan builds, hammers the voice API against the real audio callback;
// success is completing without a crash or sanitizer report.
int audio_stress_run();

// Offline mixer benchmark, run with `SKTests -audiobench`. Times full block
// renders across voice loads as a % of realtime; build Release for real numbers.
int audio_bench_run();

// Offline listening renders, `SKTests -audiorender <out_dir> [ambi.wav]`:
// stereo wavs of orbiting sources through both render paths, for A/B.
int audio_render_run(const char* out_dir, const char* ambi_file);
