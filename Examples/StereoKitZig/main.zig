pub usingnamespace @cImport({
    @cInclude("stereokit.h"); });

pub fn main() !void {
    log_set_filter(log_.diagnostic);
    _ = sk_init("StereoKit Zig", runtime_.flatscreen, 1);

    while (sk_step( step ) == 1) { }
    
    sk_shutdown();
}

export fn step() void {

}