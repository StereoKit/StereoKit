const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});
    const mode   = b.standardReleaseOptions();

    var exe = b.addExecutable("main", "main.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);

    exe.addIncludeDir("../../bin/distribute/include");
    exe.addLibPath   ("../../bin/distribute/bin/Win32/x64/Release");

    b.installFile("../../bin/distribute/bin/Win32/x64/Release/StereoKitC.dll", "bin/StereoKitC.dll");

    exe.linkSystemLibrary("c");
    exe.linkSystemLibrary("StereoKitC");
    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
