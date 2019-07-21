using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using StereoKit;

namespace StereoKitCSTest
{
    class Program
    {
        static Mesh cube;

        static void Main(string[] args)
        {
            StereoKitApp kit = new StereoKitApp("CSharp OpenXR", Runtime.Flatscreen);

            Initialize();

            while (kit.Step(()=>{

            }));
        }

        static void Initialize()
        {
            cube = new Mesh("Assets/cube.obj");
        }
    }
}
