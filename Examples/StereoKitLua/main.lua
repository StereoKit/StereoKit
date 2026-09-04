local sk = require("stereokit")

local settings = sk.SkSettings {
	mode = sk.AppMode.window,
	blendPreference = sk.DisplayBlend.opaque,
	depthMode = sk.DepthMode.default,
	renderMultisample = 4,
}

if not sk.Sk.init(settings) then
	error("StereoKit failed to initialize")
end

local cube = sk.Mesh.find("default/mesh_cube")
local material = sk.Material.find("default/material")
local white = sk.Color128 { r = 1, g = 1, b = 1, a = 1 }
local cube_scale = sk.Vec3 { x = 0.2, y = 0.2, z = 0.2 }
local ui_pose = sk.Pose {
	position = sk.Vec3 { x = 0, y = -0.1, z = -0.55 },
	orientation = sk.Quat { x = 0, y = 0, z = 0, w = 1 },
}

local angle = 0

sk.Sk.run(function()
	angle = angle + 0.025

	local cube_pos = sk.Vec3 {
		x = math.sin(angle) * 0.12,
		y = 0.08,
		z = -0.55,
	}

	cube:draw(
		material,
		sk.Matrix.ts(cube_pos, cube_scale),
		white,
		sk.RenderLayer.n0
	)

	sk.Ui.windowBegin(
		"Lua",
		ui_pose,
		sk.Vec2 { x = 0.32, y = 0 },
		sk.UiWin.normal,
		sk.UiMove.faceUser
	)
	sk.Ui.label("Hello from Lua", sk.Vec2 { x = 0, y = 0 }, true, sk.Align.center)
	if sk.Ui.button("Quit", sk.Vec2 { x = 0.12, y = 0 }, sk.Align.center) then
		sk.Sk.quit()
	end
	sk.Ui.windowEnd()
end)
