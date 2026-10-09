#pragma once

// future_t tests, run with `SKTests -futuretest`. Covers the zero future,
// GPU futures, stale handles, and many threads making and waiting on futures
// while frames run; returns 0 on success, the number of failed checks otherwise.
int future_tests_run();
