using StereoKit;

class DocVec : ITest
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

    static bool TestLerp()
    {
        /// :CodeSample: Vec3.Lerp
        Vec3 position    = new Vec3(3, 2, 5);
        Vec3 destination = new Vec3(3, 2, 8);
        position = Vec3.Lerp(position, destination, 0.5f * Time.Elapsedf);
        /// :End:
        
        Vec3 lerp = Vec3.Lerp(position, destination, 0.5f);
        Log.Info("Lerp: {0}, expected <3,2,6.5>", lerp);
        return lerp.x == 3 && lerp.y == 2 && lerp.z == 6.5f;
    }

    static bool TestDot()
    {
        /// :CodeSample: Vec3.Dot
        Pose head        = Input.Head;
        Vec3 objPosition = new Vec3(0,0,-1);
        bool visible     = Vec3.Dot(head.Forward, objPosition-head.position) > 0;
        if (visible)
            Log.Info("Object's position is in front of the user!");
        /// :End:

        bool dotPerpendicular = 0 == Vec3.Dot(new Vec3(1,1,0), new Vec3(1,-1,0));
        bool dotInvisible     = Vec3.Dot(head.Forward, new Vec3(0,0,1) - head.position) < 0;
        return visible && dotPerpendicular && dotInvisible;
    }

    public void Initialize()
    {
        Tests.Test(TestDistance);
        Tests.Test(TestDistanceSq);
        Tests.Test(TestLerp);
        Tests.Test(TestDot);
    }

    public void Shutdown(){}

    public void Update(){}
}