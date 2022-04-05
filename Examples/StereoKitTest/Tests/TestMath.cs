using StereoKit;
using System;

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

	bool TestAngleDist()
	{
		float angleDistA = SKMath.AngleDist(0, 359);
		float angleDistB = SKMath.AngleDist(359, 0);
		float angleDistC = SKMath.AngleDist(359, 720);
		float angleDistD = SKMath.AngleDist(-60, 70);
		float angleDistE = SKMath.AngleDist(-60, 140);

		Log.Info($"AngleDist 0 to 359: {angleDistA}");
		Log.Info($"AngleDist 359 to 0: {angleDistB}");
		Log.Info($"AngleDist 359 to 720: {angleDistC}");
		Log.Info($"AngleDist -60 to 70: {angleDistD}");
		Log.Info($"AngleDist -60 to 140: {angleDistE}");

		if (MathF.Abs(angleDistA - angleDistB) > tolerance) return false;
		if (MathF.Abs(angleDistA - angleDistC) > tolerance) return false;
		if (MathF.Abs(angleDistA - 1)          > tolerance) return false;
		if (MathF.Abs(angleDistD - 130)        > tolerance) return false;
		if (MathF.Abs(angleDistE - 160)        > tolerance) return false;
		return true;
	}

	bool TestVector2Angles()
	{
		float angle43 = Vec2.AngleBetween(Vec2.FromAngle(0  ), Vec2.FromAngle(43));
		float angle16 = Vec2.AngleBetween(Vec2.FromAngle(27 ), Vec2.FromAngle(43));
		float angle74 = Vec2.AngleBetween(Vec2.FromAngle(117), Vec2.FromAngle(43));

		Log.Info($"Vec2.AngleBetween(Vec2.FromAngle(0  ), Vec2.FromAngle(43)): {angle43} - expected 43");
		Log.Info($"Vec2.AngleBetween(Vec2.FromAngle(27 ), Vec2.FromAngle(43)): {angle16} - expected 16");
		Log.Info($"Vec2.AngleBetween(Vec2.FromAngle(117), Vec2.FromAngle(43)): {angle74} - expected -74");

		if (MathF.Abs(angle43 - 43) > tolerance) return false;
		if (MathF.Abs(angle16 - 16) > tolerance) return false;
		if (MathF.Abs(angle74 + 74) > tolerance) return false;
		return true;
	}

	bool TestVector3Angles()
	{
		float angle43 = Vec3.AngleBetween(Vec3.AngleXZ(0  ), Vec3.AngleXZ(43));
		float angle16 = Vec3.AngleBetween(Vec3.AngleXZ(27 ), Vec3.AngleXZ(43));
		float angle74 = Vec3.AngleBetween(Vec3.AngleXY(117), Vec3.AngleXY(43));

		Log.Info($"Vec3.AngleBetween(Vec3.AngleXZ(0  ), Vec3.AngleXZ(43)): {angle43} - expected 43");
		Log.Info($"Vec3.AngleBetween(Vec3.AngleXZ(27 ), Vec3.AngleXZ(43)): {angle16} - expected 16");
		Log.Info($"Vec3.AngleBetween(Vec3.AngleXY(117), Vec3.AngleXY(43)): {angle74} - expected 74");

		if (MathF.Abs(angle43 - 43) > tolerance) return false;
		if (MathF.Abs(angle16 - 16) > tolerance) return false;
		if (MathF.Abs(angle74 - 74) > tolerance) return false;
		return true;
	}

	bool TestMatrixOrder()
	{
		// StereoKit is Row-Major, this test verifies that matrix
		// multiplication happens in the correct order and with the correct
		// results!

		Matrix s = Matrix.S(2);
		Matrix t = Matrix.T(1, 2, 3);
		Matrix a = s * t;
		Matrix b = t * s;

		Log.Info($@"Scale * Translate = A
|{s.m.M11} {s.m.M12} {s.m.M13} {s.m.M14}|   |{t.m.M11} {t.m.M12} {t.m.M13} {t.m.M14}|   |{a.m.M11} {a.m.M12} {a.m.M13} {a.m.M14}|
|{s.m.M21} {s.m.M22} {s.m.M23} {s.m.M24}|   |{t.m.M21} {t.m.M22} {t.m.M23} {t.m.M24}|   |{a.m.M21} {a.m.M22} {a.m.M23} {a.m.M24}|
|{s.m.M31} {s.m.M32} {s.m.M33} {s.m.M34}| X |{t.m.M31} {t.m.M32} {t.m.M33} {t.m.M34}| = |{a.m.M31} {a.m.M32} {a.m.M33} {a.m.M34}|
|{s.m.M41} {s.m.M42} {s.m.M43} {s.m.M44}|   |{t.m.M41} {t.m.M42} {t.m.M43} {t.m.M44}|   |{a.m.M41} {a.m.M42} {a.m.M43} {a.m.M44}|");

		Log.Info($@"Translate * Scale = B
|{t.m.M11} {t.m.M12} {t.m.M13} {t.m.M14}|   |{s.m.M11} {s.m.M12} {s.m.M13} {s.m.M14}|   |{b.m.M11} {b.m.M12} {b.m.M13} {b.m.M14}|
|{t.m.M21} {t.m.M22} {t.m.M23} {t.m.M24}|   |{s.m.M21} {s.m.M22} {s.m.M23} {s.m.M24}|   |{b.m.M21} {b.m.M22} {b.m.M23} {b.m.M24}|
|{t.m.M31} {t.m.M32} {t.m.M33} {t.m.M34}| X |{s.m.M31} {s.m.M32} {s.m.M33} {s.m.M34}| = |{b.m.M31} {b.m.M32} {b.m.M33} {b.m.M34}|
|{t.m.M41} {t.m.M42} {t.m.M43} {t.m.M44}|   |{s.m.M41} {s.m.M42} {s.m.M43} {s.m.M44}|   |{b.m.M41} {b.m.M42} {b.m.M43} {b.m.M44}|");

		if (a.m.M41 != 1 || a.m.M42 != 2 || a.m.M43 != 3 || a.m.M44 != 1 || a.m.M11 != 2 || a.m.M22 != 2 || a.m.M33 != 2) return false;
		if (b.m.M41 != 2 || b.m.M42 != 4 || b.m.M43 != 6 || b.m.M44 != 1 || b.m.M11 != 2 || b.m.M22 != 2 || b.m.M33 != 2) return false;

		return true;
	}

	bool TestQuaternionRotation()
	{
		// TODO: This is backwards!!
		Quat rot =
			Quat.FromAngles(0, 0, 90) *
			Quat.FromAngles(90, 0, 0) *
			Quat.FromAngles(0, 90, 0);
		Vec3 result = rot * Vec3.Right;
		if (Vec3.Distance(result, V.XYZ(-1, 0, 0)) > 0.001f) return false;

		return true;
	}

	public void Initialize()
	{
		Tests.Test(TestMatrixDecompose);
		Tests.Test(TestMatrixTransform);
		Tests.Test(TestMatrixOrder);
		Tests.Test(TestAngleDist);
		Tests.Test(TestVector2Angles);
		Tests.Test(TestVector3Angles);
		Tests.Test(TestQuaternionRotation);
	}

	public void Shutdown() { }

	public void Update() { }
}