using StereoKit;
using System;

class Tester
{
    public static void Test(Func<bool> testFunction)
    {
        if (!testFunction()) {
            Log.Err("Test failed for {0}!", testFunction.Method.Name);
            Environment.Exit(-1);
        }
    }
}