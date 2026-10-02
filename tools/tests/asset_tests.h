#pragma once

// Asset scheduler dependency tests, run with `SKTests -assettest`. Drives the
// task queue with synthetic tasks for controlled timing; returns 0 on
// success, the number of failed checks otherwise.
int asset_tests_run();
