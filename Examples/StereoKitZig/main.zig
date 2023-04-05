const std = @import("std");

const sk = @cImport({
    @cInclude("stereokit.h");
    @cInclude("stereokit_ui.h");
});

var cubeMesh : sk.mesh_t     = null;
var material : sk.material_t = null;

pub fn main() !void {
    _ = sk.sk_init(std.mem.zeroInit(sk.sk_settings_t, .{
        .app_name = "StereoKit Zig"
    }));

    cubeMesh = sk.mesh_find    (sk.default_id_mesh_cube);
    material = sk.material_find(sk.default_id_material);

    sk.sk_run(step, null);
}

export fn step() void {
    sk.mesh_draw(cubeMesh, material, 
        sk.matrix_ts(sk.vec3{.x=0, .y=0, .z=-0.5}, sk.vec3{.x=0.1, .y=0.1, .z=0.1}), 
        sk.color128{.r=1,.g=1,.b=1,.a=1}, sk.render_layer_0);
}