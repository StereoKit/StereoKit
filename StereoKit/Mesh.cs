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
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr mesh_create();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr mesh_find(string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr mesh_gen_cube(string id, Vec3 size, int subdivisions);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void mesh_release(IntPtr mesh);
        
        #endregion

        internal IntPtr _meshInst;
        public Mesh()
        {
            _meshInst = mesh_create();
            if (_meshInst == IntPtr.Zero)
                Console.WriteLine("Couldn't create empty mesh!");
        }
        private Mesh(IntPtr mesh)
        {
            _meshInst = mesh;
            if (_meshInst == IntPtr.Zero)
                Console.WriteLine("Received an empty mesh!");
        }
        ~Mesh()
        {
            if (_meshInst == IntPtr.Zero)
                mesh_release(_meshInst);
        }

        public static Mesh GenerateCube(string id, Vec3 size, int subdivisions = 0)
        {
            return new Mesh(mesh_gen_cube(id, size, subdivisions));
        }
        public static Mesh Find(string id)
        {
            IntPtr mesh = mesh_find(id);
            return mesh == IntPtr.Zero ? null : new Mesh(mesh);
        }
    }
}
