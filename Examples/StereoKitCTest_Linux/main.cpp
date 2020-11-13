#include <stereokit.h>
#pragma comment(lib,"StereoKitC.lib")
using namespace sk;

int main() {
  log_set_filter(log_diagnostic);
  sk_init("Fabrica C", runtime_flatscreen);
  while (sk_step([]() {})) {}
  sk_shutdown();
}
