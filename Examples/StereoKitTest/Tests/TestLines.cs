using StereoKit;

class TestLines : ITest
{
	public void Initialize(){}
	public void Shutdown(){}
	public void Update() {
		Tests.Screenshot("Tests/LineRenderer.jpg", 600, 600, V.XYZ(0,0,0.5f), Vec3.Zero);

		// A ring of lines that goes out past the edge of the screen
		Color32 c = Color32.White;
		for (int i = 0; i < 16; i++)
		{
			Vec3 at = Vec3.AngleXY((i/16.0f)*360.0f) * 0.5f;
			Lines.Add(at-V.XYZ(0,0,0.2f), at+V.XYZ(0,0,0.2f), c, 0.01f);
		}

		// A Circle
		c = new Color(1, .5f, .5f, 1).ToLinear();
		LinePoint[] points = new LinePoint[17];
		for (int i = 0; i < points.Length; i++)
		{
			points[i] = new LinePoint( 
				Vec3.AngleXY((i / (float)(points.Length-1)) * 360.0f) * 0.4f,
				c, 0.01f);
		}
		Lines.Add(points);

		// A square, worst case for this system
		c = new Color(.5f, 1, .5f, 1).ToLinear();
		Lines.Add(new LinePoint[] { 
			new LinePoint(V.XY0(-0.25f, 0.25f), c, 0.01f),
			new LinePoint(V.XY0( 0.25f, 0.25f), c, 0.01f),
			new LinePoint(V.XY0( 0.25f,-0.25f), c, 0.01f),
			new LinePoint(V.XY0(-0.25f,-0.25f), c, 0.01f),
			new LinePoint(V.XY0(-0.25f, 0.25f), c, 0.01f),
		});

		// A beveled square
		c = new Color(.5f, .5f, 1, 1).ToLinear();
		Lines.Add(new LinePoint[] {
			new LinePoint(V.XY0(-0.2f, 0.19f), c, 0.01f),
			new LinePoint(V.XY0(-0.19f, 0.2f), c, 0.01f),
			new LinePoint(V.XY0( 0.19f, 0.2f), c, 0.01f),
			new LinePoint(V.XY0( 0.2f, 0.19f), c, 0.01f),
			new LinePoint(V.XY0( 0.2f,-0.19f), c, 0.01f),
			new LinePoint(V.XY0( 0.19f,-0.2f), c, 0.01f),
			new LinePoint(V.XY0(-0.19f,-0.2f), c, 0.01f),
			new LinePoint(V.XY0(-0.2f,-0.19f), c, 0.01f),
			new LinePoint(V.XY0(-0.2f, 0.19f), c, 0.01f),
		});

		// an Icosohedron
		float t  = (1.0f + SKMath.Sqrt(5.0f)) / 2.0f;
		float s  = 0.08f;
		float s2 = 0.3f;
		int   h  = 0;
		Vec3  v;

		Hierarchy.Push(Matrix.R(Quat.FromAngles(10,10,10)));
		v = V.XYZ(-1,  t,  0) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ( 1,  t,  0) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ(-1, -t,  0) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ( 1, -t,  0) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ( 0, -1,  t) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ( 0,  1,  t) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ( 0, -1, -t) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ( 0,  1, -t) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ( t,  0, -1) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ( t,  0,  1) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ(-t,  0, -1) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		v = V.XYZ(-t,  0,  1) * s; Lines.Add(v, v*s2, Color.HSV((h++) / 12.0f, 0.8f, 0.8f).ToLinear(), 0.01f);
		Hierarchy.Pop();
	}
}
