#pragma once

// Pure spherical harmonics math tests, run with `SKTests -shtest`. Needs no
// StereoKit init; returns 0 on success, the failed check count otherwise.
int sh_tests_run();
