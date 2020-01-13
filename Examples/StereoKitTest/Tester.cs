using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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