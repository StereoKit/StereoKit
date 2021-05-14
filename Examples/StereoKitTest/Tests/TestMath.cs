using StereoKit;

class TestMath : ITest
{
	const float tolerance = 0.0001f;
	bool TestMatrixDecompose()
	{
		Vec3 translate = new Vec3(1, 2, 3);
		Quat rotate    = Quat.FromAngles(0, 90, 0);
		Vec3 scale     = new Vec3(1,2,1);
		Matrix mat = Matrix.TRS(translate, rotate, scale);

		Vec3 exTranslate = mat.Translation;
		Quat exRotate    = mat.Rotation;
		Vec3 exScale     = mat.Scale;

		Log.Info($"Transform matrix:     {translate  } {rotate  } {scale  }");
		Log.Info($"Matrix sep decompose: {exTranslate} {exRotate} {exScale}");

		if (Vec3.Distance(exTranslate, translate) > tolerance) return false;
		if (Vec3.Distance(exScale,     scale)     > tolerance) return false;
		
		if (!mat.Decompose(out exTranslate, out exRotate, out exScale)) return false;
		Log.Info($"Matrix decompose:     {exTranslate} {exRotate} {exScale}");

		if (Vec3.Distance(exTranslate, translate) > tolerance) return false;
		if (Vec3.Distance(exScale,     scale)     > tolerance) return false;

		return true;
	}

	bool TestMatrixTransform()
	{
		Vec3 translate = new Vec3(10, 0, 0);
		Vec3 rotate    = new Vec3(0,90,0);
		Vec3 scale     = new Vec3(2,2,2);
		Matrix mat = Matrix.TRS(translate, Quat.FromAngles(rotate), scale);
		Log.Info($"Transform matrix: {translate} {rotate} {scale}");

		// Check ray transforms
		Ray ray1 = new Ray(Vec3.Zero, V.XYZ(0,0,1));
		Ray ray2 = new Ray(V.XYZ(1,0,0), V.XYZ(0,0,1));

		Ray tRay1 = mat.Transform(ray1);
		Ray tRay2 = mat * ray2;

		Log.Info($"Ray {ray1} -> {tRay1}");
		Log.Info($"Ray {ray2} -> {tRay2}");

		if (Vec3.Distance(tRay1.position,  V.XYZ(10,0,0))  > tolerance) return false;
		if (Vec3.Distance(tRay1.direction, V.XYZ(2,0,0))   > tolerance) return false;
		if (Vec3.Distance(tRay2.position,  V.XYZ(10,0,-2)) > tolerance) return false;
		if (Vec3.Distance(tRay2.direction, V.XYZ(2,0,0))   > tolerance) return false;

		// Check pose transforms
		Pose pose1 = Pose.Identity;
		Pose pose2 = new Pose(V.XYZ(1,0,0), Quat.FromAngles(0,90,0));

		Pose tPose1 = mat.Transform(pose1);
		Pose tPose2 = mat * pose2;

		Log.Info($"Pose {pose1} -> {tPose1}");
		Log.Info($"Pose {pose2} -> {tPose2}");

		if (Vec3.Distance(tPose1.position, V.XYZ(10, 0, 0))  > tolerance) return false;
		if (Vec3.Distance(tPose1.Forward,  V.XYZ(-1, 0, 0))  > tolerance) return false;
		if (Vec3.Distance(tPose2.position, V.XYZ(10, 0, -2)) > tolerance) return false;
		if (Vec3.Distance(tPose2.Forward,  V.XYZ(0, 0, 1))   > tolerance) return false;

		return true;
	}

	public void Initialize()
	{
		Tests.Test(TestMatrixDecompose);
		Tests.Test(TestMatrixTransform);
	}

	public void Shutdown() { }

	public void Update() { }
}