using StereoKit;

class DocVec : ITest
{
	static bool TestDistance()
	{
		/// :CodeSample: Vec3.Distance
		/// ### Distance between two points
		/// 
		/// Distance does use a Sqrt call, so only use it if you definitely
		/// need the actual distance. Otherwise, consider DistanceSq.
		Vec3  pointA   = new Vec3(3,2,5);
		Vec3  pointB   = new Vec3(3,2,8);
		float distance = Vec3.Distance(pointA, pointB);
		/// :End:

		Log.Info("Distance: {0}, expected 3", distance);
		return distance == 3;
	}

	static bool TestDistanceSq()
	{
		/// :CodeSample: Vec3.DistanceSq
		Vec3 pointA = new Vec3(3, 2, 5);
		Vec3 pointB = new Vec3(3, 2, 8);

		float distanceSquared = Vec3.DistanceSq(pointA, pointB);
		if (distanceSquared < 4*4) { 
			Log.Info("Distance is less than 4");
		}
		/// :End:

		Log.Info("DistanceSq: {0}, expected 9", distanceSquared);
		return distanceSquared == 9;
	}

	static bool TestAngles()
	{
		/// :CodeSample: Vec2.Angle
		Vec2 point = new Vec2(1, 0);
		float angle0 = point.Angle();

		point = new Vec2(0, 1);
		float angle90 = point.Angle();

		point = new Vec2(-1, 0);
		float angle180 = point.Angle();

		point = new Vec2(0, -1);
		float angle270 = point.Angle();
		/// :End:

		point = new Vec2(2.75f, 2.75f);
		float angle45 = point.Angle();

		Log.Info($"Angles 0 == {angle0}, Angles 45 == {angle45}, 90 == {angle90}, 180 == {angle180}, 270 == {angle270}");
		return angle0 == 0 && angle90 == 90 && angle180 == 180 && angle270 == 270;
	}

	static bool TestAngleBetween()
	{
		/// :CodeSample: Vec2.AngleBetween
		Vec2 directionA = new Vec2( 1, 1);
		Vec2 directionB = new Vec2(-1, 1);
		float angle90 = Vec2.AngleBetween(directionA, directionB);

		directionA = new Vec2(1, 1);
		directionB = new Vec2(0,-2);
		float angleNegative135 = Vec2.AngleBetween(directionA, directionB);
		/// :End:

		directionA = new Vec2(1, 1);
		directionB = new Vec2(0, 1);
		float angle45 = Vec2.AngleBetween(directionA, directionB);

		Log.Info($"AngleBetween 45 == {angle45}, 90 == {angle90}, -135 == {angleNegative135}");
		return angle45 == 45 && angle90 == 90 && angleNegative135 == -135;
	}

	public void Initialize()
	{
		Tests.Test(TestDistance);
		Tests.Test(TestDistanceSq);
		Tests.Test(TestAngles);
		Tests.Test(TestAngleBetween);
	}

	public void Shutdown(){}

	public void Update(){}
}