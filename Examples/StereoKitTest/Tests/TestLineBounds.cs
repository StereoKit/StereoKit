using StereoKit;

class TestLineBounds : ITest
{
	bool TestLineBoundsContains()
	{
		Bounds b = new Bounds(Vec3.UnitY*0.5f, Vec3.One*0.02f);
		if (
			b.Contains(b.center - Vec3.UnitX, b.center + Vec3.UnitX) == false ||
			b.Contains(b.center - Vec3.UnitX, b.center) == false ||
			b.Contains(b.center, b.center) == false)
			return false;

		Vec3 off = Vec3.UnitY*0.03f;
		if (
			b.Contains(b.center - Vec3.UnitX + off, b.center + Vec3.UnitX + off) == true)
			return false;

		return true;
	}


	public void Initialize()
	{
		Tests.Test(TestLineBoundsContains);
	}

	public void Shutdown() { }

	public void Update() { }
}