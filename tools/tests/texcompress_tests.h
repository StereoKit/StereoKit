#pragma once

// Texture compression format choice and header alpha detection, run with
// `SKTests -texcompresstest`. The GPU leg initializes StereoKit offscreen.
// Returns 0 on success, the failed check count otherwise.
int texcompress_tests_run();
