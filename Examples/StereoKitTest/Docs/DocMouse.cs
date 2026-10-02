using StereoKit;
using System;

class DocMouse : ITest
{
	public void Initialize() { }

	public void Shutdown()
	{
		// Switching scenes shouldn't leave the cursor captured, or the view
		// pointed off in whatever direction this scene was left in.
		Input.MouseMode     = MouseMode.Normal;
		Renderer.CameraRoot = Matrix.Identity;
	}

	public void Step()
	{
		MouseLook();

		for (int i = 0; i < 8; i++)
		{
			Vec3 at = Vec3.AngleXZ(i * 45) * 1.5f + Vec3.Up * (i % 2) * 0.3f;
			Mesh.Cube.Draw(Material.Default, Matrix.TS(at, 0.2f));
		}
	}

	/// :CodeSample: Input.MouseMode MouseMode Mouse.posChange
	/// ### Mouse look
	/// `MouseMode.Relative` is what you want for mouse-look style camera
	/// control. The cursor is hidden and pinned in place, so `Mouse.pos` stops
	/// changing, and `Mouse.posChange` becomes the only report of mouse motion.
	/// The pointer never reaches the edge of the screen, so the view can keep
	/// turning as far as the user cares to spin.
	///
	/// Capture the mouse only while the user is actually looking around, and
	/// hand it back when they let go, so the rest of the time they still have a
	/// cursor to click with. Note that `posChange` is an amount of motion rather
	/// than a speed, so unlike a velocity it should _not_ be scaled by
	/// `Time.Stepf`. Doing that would tie the sensitivity to the frame rate.
	float lookYaw;
	float lookPitch;
	void MouseLook()
	{
		// The Simulator mode already provides a mouselook, and XR doesn't need
		// one, so this is only really useful in Window mode.
		if (SK.Settings.mode != AppMode.Window)
			return;

		if (Input.Key(Key.MouseRight).IsJustActive  ()) Input.MouseMode = MouseMode.Relative;
		if (Input.Key(Key.MouseRight).IsJustInactive()) Input.MouseMode = MouseMode.Normal;
		if (Input.MouseMode != MouseMode.Relative) return;

		// Relative mode reports raw mouse units instead of pixels, so this is
		// degrees per unit of motion, and wants tuning by feel.
		const float sensitivity = 0.1f;
		lookYaw   -= Input.Mouse.posChange.x * sensitivity;
		lookPitch -= Input.Mouse.posChange.y * sensitivity;
		// Stop just shy of straight up and down, or the view rolls over the top
		lookPitch  = Math.Clamp(lookPitch, -89.9f, 89.9f);

		Renderer.CameraRoot = Matrix.R(lookPitch, lookYaw, 0);
	}
	/// :End:
}
