#flag -L../../bin/distribute/bin/x64
#flag -I../../bin/distribute/include
#flag -lStereoKitC
#include "stereokit.h"

fn C.sk_init(charptr, int, int)
fn C.sk_step(onStep fn()) bool
fn C.sk_shutdown()

fn main() {
	C.sk_init("StereoKit V", 0, 0)
	for C.sk_step(step) { }
	C.sk_shutdown()
}

fn step() {

}