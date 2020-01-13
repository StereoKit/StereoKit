using StereoKit;

class DocVec : IDemo
{
    static bool TestDistance()
    {
        /// :CodeSample: Vec3.Distance
        Vec3 pointA = new Vec3(3,2,5);
        Vec3 pointB = new Vec3(3,2,8);
        float distance = Vec3.Distance(pointA, pointB);
        /// :End:
        
        Log.Info("Distance: {0}, expected 3", distance);
        return distance == 3;
    }

    static bool TestDistanceSq()
    {
        /// :CodeSample: Vec3.Distance
        Vec3 pointA = new Vec3(3, 2, 5);
        Vec3 pointB = new Vec3(3, 2, 8);
        float distanceSq = Vec3.DistanceSq(pointA, pointB);
        if (distanceSq < 4*4) { 
            Log.Info("Distance is less than 4");
        }
        /// :End:
        
        Log.Info("DistanceSq: {0}, expected 9", distanceSq);
        return distanceSq == 9;
    }

    static bool TestLerp()
    {
        /// :CodeSample: Vec3.Distance
        Vec3 position    = new Vec3(3, 2, 5);
        Vec3 destination = new Vec3(3, 2, 8);
        position = Vec3.Lerp(position, destination, 0.5f * Time.Elapsedf);
        /// :End:
        
        Vec3 lerp = Vec3.Lerp(position, destination, 0.5f);
        Log.Info("Lerp: {0}, expected <3,2,6.5>", lerp);
        return lerp.x == 3 && lerp.y == 2 && lerp.z == 6.5f;
    }

    public void Initialize()
    {
        Tester.Test(TestDistance);
        Tester.Test(TestDistanceSq);
        Tester.Test(TestLerp);
    }

    public void Shutdown(){}

    public void Update(){}
}