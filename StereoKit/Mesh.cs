using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
    public class Mesh
    {
        #region Imports
        [DllImport("StereoKitC.dll")]
        static extern IntPtr mesh_create();
        [DllImport("StereoKitC.dll", CharSet = CharSet.Ansi)]
        static extern IntPtr mesh_create_file(string file);
        [DllImport("StereoKitC.dll")]
        static extern void mesh_destroy(IntPtr mesh);
        #endregion

        IntPtr _meshInst;
        public Mesh()
        {
            _meshInst = mesh_create();
            if (_meshInst == IntPtr.Zero)
                Console.WriteLine("Couldn't create empty mesh!");
        }
        public Mesh(string file)
        {
            _meshInst = mesh_create_file(file);
            if (_meshInst == IntPtr.Zero)
                Console.WriteLine("Couldn't load {0}!", file);
        }
        ~Mesh()
        {
            if (_meshInst == IntPtr.Zero)
                mesh_destroy(_meshInst);
        }
    }
}
