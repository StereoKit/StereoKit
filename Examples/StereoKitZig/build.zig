const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    var exe = b.addExecutable(.{
        .name = "StereoKitZigTest",
        .root_source_file = .{ .path = "main.zig" },
        .target = target,
        .optimize = optimize,
    });

    const stereokit_module = b.createModule(.{
        .source_file = .{ .path = "StereoKit/StereoKit.zig" },
        .dependencies = &.{},
    });

    exe.addIncludePath("../../bin/distribute/include");
    exe.addLibraryPath("../../bin/distribute/bin/Win32/x64/Release");
    exe.addModule("StereoKit", stereokit_module);
    exe.linkSystemLibrary("StereoKitC");

    b.installArtifact(exe);
    b.installFile("../../bin/distribute/bin/Win32/x64/Release/StereoKitC.dll", "bin/StereoKitC.dll");

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
