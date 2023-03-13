const std = @import("std");
const sk = @import("StereoKit");

//var cubeMesh: sk.mesh_t = null;
//var material: sk.material_t = null;

var cubeMesh: sk.Mesh = .{};
var material: sk.Material = .{};

pub fn main() !void {
    _ = sk.SK.init(.{});
    //_ = sk.sk_init(std.mem.zeroInit(sk.sk_settings_t, .{ .app_name = "StereoKit Zig" }));

    //cubeMesh = sk.mesh_find(sk.default_id_mesh_cube);
    //material = sk.material_find(sk.default_id_material);

    sk.Mesh.find("default/mesh_sphere");

    sk.SK.run(step, null);
}

export fn step() void {
    cubeMesh.draw(material, sk.Matrix.ts(.{ 0, 0, -0.5 }, .{ 0.1, 0.1, 0.1 }), .{ 1, 1, 1, 1 }, sk.RenderLayer.Layer0);
    //sk.mesh_draw(cubeMesh, material, sk.matrix_ts(sk.vec3{ .x = 0, .y = 0, .z = -0.5 }, sk.vec3{ .x = 0.1, .y = 0.1, .z = 0.1 }), sk.color128{ .r = 1, .g = 1, .b = 1, .a = 1 }, sk.render_layer_0);
}
