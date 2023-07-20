const std = @import("std");
const sk = @import("StereoKit");

var cubeMesh: sk.Mesh = .{};
var material: sk.Material = .{};

pub fn main() !void {
    std.debug.print("bool size: {}", .{@sizeOf(bool)});
    _ = sk.init(.{
        .appName = "StereoKit Zig",
    });

    cubeMesh = sk.Mesh.find("default/mesh_cube");
    material = sk.Material.find("default/material");

    sk.run(step, null);
}

export fn step() void {
    cubeMesh.draw(material, sk.Matrix.ts(.{ .x = 0, .y = 0, .z = -0.5 }, .{ .x = 0.1, .y = 0.1, .z = 0.1 }), .{ .r = 1, .g = 1, .b = 1, .a = 1 }, SK.RenderLayer.Layer0);
}
