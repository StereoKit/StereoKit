#include <stereokit.h>
using namespace sk;

int main() {
  log_set_filter(log_diagnostic);
  sk_init("StereoKit Linux Test", runtime_flatscreen);
  while (sk_step([]() {})) {}
  sk_shutdown();
}
