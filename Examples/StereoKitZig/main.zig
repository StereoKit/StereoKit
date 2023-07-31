const sk = @import("StereoKit");

var cubeMesh: sk.Mesh = .{};
var material: sk.Material = .{};

pub fn main() void {
    _ = sk.init(.{
        .appName = "StereoKit Zig",
    });

    cubeMesh = sk.Mesh.find("default/mesh_cube");
    material = sk.Material.find("default/material");
    defer cubeMesh.release();
    defer material.release();

    sk.run(step, null);
}

var pose: sk.Pose = .{ .position = sk.xyz(0, 0, 0), .orientation = .{ .x = 0, .y = 0, .z = 0, .w = 1 } };
export fn step() void {
    cubeMesh.draw(material, sk.Matrix.ts(sk.xyz(0, 0, -0.5), sk.xyz(0.1, 0.1, 0.1)), sk.rgba(1, 1, 1, 1), sk.RenderLayer.Layer0);

    sk.UI.windowBegin("Testing", &pose, sk.xy(0, 0), sk.UIWin.Normal, sk.UIMove.FaceUser);
    sk.UI.label("Hello!", sk.true32);
    sk.UI.hseparator();
    if (sk.UI.button("Press me") == sk.true32) {}
    sk.UI.windowEnd();
}
