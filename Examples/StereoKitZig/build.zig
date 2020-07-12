const Builder = @import("std").build.Builder;
const builtin = @import("builtin");

pub fn build(b: *Builder) void {
    const mode = b.standardReleaseOptions();

    var exe = b.addExecutable("main", "main.zig");
    exe.setBuildMode(mode);

    exe.addIncludeDir("../../bin/distribute/include");
    exe.addLibPath   ("../../bin/distribute/bin/x64");

    b.installFile("../../bin/distribute/bin/x64/StereoKitC.dll", "bin/StereoKitC.dll");
    b.installFile("../../bin/distribute/bin/x64/LeapC.dll",      "bin/LeapC.dll");

    exe.linkSystemLibrary("c");
    exe.linkSystemLibrary("StereoKitC");
    exe.install();

    const play = b.step("run", "Run the app");
    const run = exe.run();
    run.step.dependOn(b.getInstallStep());
    play.dependOn(&run.step);
}
