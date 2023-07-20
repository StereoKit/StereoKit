const std = @import("std");
const SK = @import("StereoKit");

var cubeMesh: SK.Mesh = .{};
var material: SK.Material = .{};

pub fn main() !void {
    std.debug.print("bool size: {}", .{@sizeOf(bool)});
    _ = SK.SK.init(.{
        .appName = "StereoKit Zig",
    });

    cubeMesh = SK.Mesh.find("default/mesh_cube");
    material = SK.Material.find("default/material");

    SK.SK.run(step, null);
}

export fn step() void {
    cubeMesh.draw(material, SK.Matrix.ts(.{ .x = 0, .y = 0, .z = -0.5 }, .{ .x = 0.1, .y = 0.1, .z = 0.1 }), .{ .r = 1, .g = 1, .b = 1, .a = 1 }, SK.RenderLayer.Layer0);
}
