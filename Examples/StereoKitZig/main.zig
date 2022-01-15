const std = @import("std");

pub usingnamespace @cImport({
    @cInclude("stereokit.h");
    @cInclude("stereokit_ui.h"); });

pub fn main() !void {
    const settings = std.mem.zeroInit(sk_settings_t, .{
        .app_name = "StereoKit Zig"
    });
    _ = sk_init(settings);

    while (sk_step( step ) == 1) { }
    
    sk_shutdown();
}

export fn step() void {

}