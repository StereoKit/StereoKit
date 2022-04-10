const std = @import("std");

const c = @cImport({
    @cInclude("stereokit.h");
    @cInclude("stereokit_ui.h");
});

pub fn main() !void {
    _ = c.sk_init(std.mem.zeroInit(c.sk_settings_t, .{
        .app_name = "StereoKit Zig"
    }));

    c.sk_run(step, null);
}

export fn step() void {

}