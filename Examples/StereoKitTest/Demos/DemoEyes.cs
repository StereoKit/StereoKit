using System;
using StereoKit;
using System.Collections.Generic;

class DemoEyes : ITest
{
	string title       = "Eye Tracking";
	string description = "If the hardware supports it, and permissions are granted, eye tracking is as simple as grabbing Input.Eyes!\n\nThis scene is raycasting your eye ray at the indicated plane, and the dot's red/green color indicates eye tracking availability! On flatscreen you can simulate eye tracking with Alt+Mouse.";

	List<LinePoint> points = new List<LinePoint>();
	Vec3 previous;

	long   lastEyesSampleTime;
	double demoStartTime;
	int    uniqueSamplesCount;

	public void Initialize()
	{
		demoStartTime      = Time.Total;
		uniqueSamplesCount = 0;
		lastEyesSampleTime = -1;
	}
	public void Shutdown  () { }

	public void Step()
	{
		Plane  plane    = new Plane(new Vec3(0.5f,0,-0.5f), V.XYZ(-0.5f,0,0.5f));
		Matrix quadPose = Matrix.TRS(new Vec3(0.54f, -0.2f, -0.468f), Quat.LookDir(plane.normal), 0.5f);
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

		Demo.ShowSummary(title, description);

		if (Backend.XRType == BackendXRType.OpenXR)
		{
			if (Backend.OpenXR.EyesSampleTime != lastEyesSampleTime)
			{
				lastEyesSampleTime = Backend.OpenXR.EyesSampleTime;
				uniqueSamplesCount++;
			}

			double sampleFrequency = uniqueSamplesCount / (Time.Total - demoStartTime);
			Text.Add($"Eye tracker sampling frequency: {sampleFrequency:0.#} Hz", Matrix.T(V.XYZ(0, -0.75f, -0.1f)) * quadPose);
		}
	}
}