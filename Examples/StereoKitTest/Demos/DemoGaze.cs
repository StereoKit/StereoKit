using StereoKit;
using System.Collections.Generic;

class DemoGaze : ITest
{
	List<LinePoint> points = new List<LinePoint>();
	Vec3 previous;

	public void Initialize() { }
	public void Shutdown  () { }

	public void Update()
	{
		Plane plane = new Plane(new Vec3(0,0,-1.5f), -Vec3.Forward);
		Pose  gaze  = Input.Gaze;

		if (gaze.Ray.Intersect(plane, out Vec3 at))
		{
			Default.MeshSphere.Draw(Default.Material, Matrix.TS(at, 3*U.cm), new Color(1,0,0));
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
	}
}