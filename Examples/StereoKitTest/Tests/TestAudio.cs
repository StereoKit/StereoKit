using StereoKit;
using System;
using System.IO;

class TestAudio : ITest
{
	Pose      winPose    = new Pose(0.2f, 0, -0.5f, Quat.FromAngles(0, 180, 0));
	Sound     sound      = Sound.Generate((x) => (((float)Math.Sin(x * (91.3458)) * 47453.5453f)%1.0f)*2.0f-1.0f, 5);
	SoundInst inst;
	float     distance   = 1;
	float     volume     = 80;
	bool      rotate     = false;
	bool      behind     = false;
	float     rotateTime = 0;
	float     intensity  = 0;

	public void Initialize()
	{
		Renderer.SetClip(0.08f,250);
	}

	public void Shutdown()
	{
		inst.Stop();
		Platform.FilePickerClose();
	}

	public void Step()
	{
		UI.WindowBegin("Sound Tester", ref winPose, new Vec2(0.3f,0));
		if (UI.Button("Browse"))
			Platform.FilePicker(PickerMode.Open, (f) => { sound = Sound.FromFile(f); sound.Decibels = volume; }, null, ".wav", ".mp3");
		UI.SameLine();
		UI.Label(Path.GetFileName( sound.Id ));

		UI.Label($"Distance {distance}", new Vec2(0.1f,0));
		UI.SameLine();
		UI.HSlider("Distance", ref distance, 0.1f, 200.1f, 1);

		UI.Label($"Volume {volume}", new Vec2(0.1f, 0));
		UI.SameLine();
		if (UI.HSlider("Volume", ref volume, 10, 200, 1))
			sound.Decibels = volume;

		if (UI.Button("Play")) inst = sound.Play(GetPos());
		UI.SameLine();

		if (UI.Toggle("Rotate", ref rotate)) { rotateTime = Time.Totalf; }
		UI.SameLine();
		UI.Toggle("Behind", ref behind);

		UI.WindowEnd();

		float scale = SKMath.Lerp(0.3f, 4, (volume / 200.0f) * (volume / 200.0f));
		intensity = SKMath.Lerp(intensity, inst.Intensity, 0.1f);
		Mesh.Sphere.Draw(Material.Unlit, Matrix.TS(GetPos(), SKMath.Lerp(0.1f, 1, intensity) * scale));
		inst.Position = GetPos();
	}

	Vec3 GetPos()
	{
		float start = behind ? 270 : 90;
		return rotate
			? Vec3.AngleXZ((Time.Totalf-rotateTime) * 45 + start)*-distance
			: Vec3.AngleXZ(start)*-distance;
	}
}