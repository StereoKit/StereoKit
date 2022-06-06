using System;
using StereoKit;
using System.Collections.Generic;

class DemoEyes : ITest
{
	Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	string description = "If the hardware supports it, and permissions are granted, eye tracking is as simple as grabbing Input.Eyes!\n\nThis scene is raycasting your eye ray at the indicated plane, and the dot's red/green color indicates eye tracking availability! On flatscreen you can simulate eye tracking with Alt+Mouse.";
	Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
	string title       = "Eye Tracking";

	List<LinePoint> points = new List<LinePoint>();
	Vec3 previous;

	long lastEyesSampleTime;
	DateTime demoStartTime;
	int uniqueSamplesCount;

	public void Initialize()
	{
		demoStartTime = DateTime.UtcNow;
		uniqueSamplesCount = 0;
		lastEyesSampleTime = -1;
	}
	public void Shutdown  () { }

	public void Update()
	{
		Plane plane = new Plane(new Vec3(0.5f,0,-0.5f), V.XYZ(-0.5f,0,0.5f));
		Matrix quadPose = Matrix.TRS(new Vec3(0.54f, 0, -0.468f), Quat.LookDir(plane.normal), 0.5f);
		Mesh.Quad.Draw(Material.Default, quadPose);
		if (Input.Eyes.Ray.Intersect(plane, out Vec3 at))
		{
			Color stateColor = Input.EyesTracked.IsActive() 
				? new Color(0,1,0)
				: new Color(1,0,0);
			Default.MeshSphere.Draw(Default.Material, Matrix.TS(at, 3*U.cm), stateColor);
			if (Vec3.DistanceSq(at, previous) > U.cm*U.cm) {
				previous = at;
				points.Add(new LinePoint { pt = at, color = Color.White });
				if (points.Count > 20)
					points.RemoveAt(0);
			}

			LinePoint pt = points[points.Count - 1];
			pt.pt = at;
			points[points.Count - 1] = pt;
		}

		for (int i = 0; i < points.Count; i++) { 
			LinePoint pt = points[i];
			pt.thickness = (i / (float)points.Count) * 3 * U.cm;
			points[i] = pt;
		}

		Lines.Add(points.ToArray());

		Text.Add(title, titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);

		if (Backend.XRType == BackendXRType.OpenXR)
		{
			if (Backend.OpenXR.EyesSampleTime != lastEyesSampleTime)
            {
				lastEyesSampleTime = Backend.OpenXR.EyesSampleTime;
				uniqueSamplesCount++;
            }

			double sampleFrequency = uniqueSamplesCount / (DateTime.UtcNow - demoStartTime).TotalSeconds;
			Text.Add($"Eye tracker sampling frequency: {sampleFrequency:0.#} Hz", Matrix.T(V.XYZ(0, -0.55f, -0.1f)) * quadPose);
		}
	}
}