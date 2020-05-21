import os

fn main() {

	// Get all .v files that are not build.v
	mut files := ""
	all_files := os.ls('.') or {return}
	for file in all_files {
		if file.ends_with(".v") && file != "build.v" {
			files += file + " "
		}
	}

	// Set up our build folder with some DLLs we need
	os.mkdir("bin") or {}
	os.cp("../../bin/distribute/bin/x64/StereoKitC.dll", "bin/StereoKitC.dll") or {}
	os.cp("../../bin/distribute/bin/x64/LeapC.dll", "bin/LeapC.dll") or {}

	// And build
	os.exec("v -o bin/app.exe run " + files) or { }
}