#pragma once

// Texture loading, upload, and threading behavior, run with `SKTests -textest`.
// Initializes StereoKit offscreen. Returns 0 on success, the failed check
// count otherwise.
int tex_tests_run();
