#pragma once

// Spatial entity registry tests, run with `SKTests -spatialtest`. Drives the
// registry through a fake backend whose async operations the test completes
// by hand; returns 0 on success, the number of failed checks otherwise.
int spatial_tests_run();
