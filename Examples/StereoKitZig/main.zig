pub usingnamespace @cImport({
    @cInclude("stereokit.h");
    @cInclude("stereokit_ui.h"); });

pub fn main() !void {
    _ = sk_init(sk_settings_t {
        .app_name = "StereoKit Zig",
        .assets_folder = "",
        .display_preference = display_mode_.mixedreality,
        .blend_preference = display_blend_.none,
        .no_flatscreen_fallback = 0,
        .disable_flatscreen_mr_sim = 0,
        .depth_mode = depth_mode_.balanced,
        .flatscreen_pos_x = 0,
        .flatscreen_pos_y = 0,
        .flatscreen_width = 0,
        .flatscreen_height = 0,
        .log_filter = log_.diagnostic,
        .android_java_vm = null,
        .android_activity = null,
        .overlay_app = 0,
        .overlay_priority = 0,
    });

    while (sk_step( step ) == 1) { }
    
    sk_shutdown();
}

export fn step() void {

}