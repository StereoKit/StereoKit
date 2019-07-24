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
        [DllImport(Util.DllName)]
        static extern IntPtr mesh_create();
        [DllImport(Util.DllName)]
        static extern void mesh_release(IntPtr mesh);
        #endregion

        internal IntPtr _meshInst;
        public Mesh()
        {
            _meshInst = mesh_create();
            if (_meshInst == IntPtr.Zero)
                Console.WriteLine("Couldn't create empty mesh!");
        }
        ~Mesh()
        {
            if (_meshInst == IntPtr.Zero)
                mesh_release(_meshInst);
        }
    }
}
