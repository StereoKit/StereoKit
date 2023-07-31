
pub fn xyzw(x: f32, y: f32, z: f32, w: f32) Vec3 {
	return .{ .x = x, .y = y, .z = z, .w = w };
}

pub fn xyz(x: f32, y: f32, z: f32) Vec3 {
	return .{ .x = x, .y = y, .z = z };
}

pub fn xy(x: f32, y: f32) Vec2 {
	return .{ .x = x, .y = y };
}

pub fn rgba(r: f32, g: f32, b: f32, a: f32) Color {
	return .{ .r = r, .g = g, .b = b, .a = a };
}

pub fn rgba32(x: u8, y: u8, z: u8, w: u8) Color32 {
	return .{ .x = x, .y = y, .z = z, .w = w };
}