#flag -L../../bin/distribute/bin/Win32/x64/Release
#flag -I../../bin/distribute/include
#flag -lStereoKitC
#include "stereokit.h"

struct C.sk_settings_t {
}

fn C.sk_init(C.sk_settings_t)
fn C.sk_run(onStep fn(), onShutdown fn())

fn main() {
	C.sk_init(C.sk_settings_t{})
	C.sk_run(step, shutdown)
}

fn step() {

}

fn shutdown() {

}