using System;
using System.Runtime.InteropServices;

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
        static extern IntPtr mesh_gen_cube(string id, Vec3 dimensions, int subdivisions);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr mesh_gen_rounded_cube(string id, Vec3 dimensions, float edge_radius, int subdivisions);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr mesh_gen_sphere(string id, float diameter, int subdivisions);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void mesh_release(IntPtr mesh);
        
        #endregion

        internal IntPtr _meshInst;
        public Mesh()
        {
            _meshInst = mesh_create();
            if (_meshInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Couldn't create empty mesh!");
        }
        private Mesh(IntPtr mesh)
        {
            _meshInst = mesh;
            if (_meshInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Received an empty mesh!");
        }
        ~Mesh()
        {
            if (_meshInst == IntPtr.Zero)
                mesh_release(_meshInst);
        }

        public static Mesh GenerateCube(string id, Vec3 dimensions, int subdivisions = 0)
        {
            return new Mesh(mesh_gen_cube(id, dimensions, subdivisions));
        }
        public static Mesh GenerateRoundedCube(string id, Vec3 dimensions, float edgeRadius, int subdivisions = 4)
        {
            return new Mesh(mesh_gen_rounded_cube(id, dimensions, edgeRadius, subdivisions));
        }
        public static Mesh GenerateSphere(string id, float diameter, int subdivisions = 4)
        {
            return new Mesh(mesh_gen_sphere(id, diameter, subdivisions));
        }
        public static Mesh Find(string id)
        {
            IntPtr mesh = mesh_find(id);
            return mesh == IntPtr.Zero ? null : new Mesh(mesh);
        }
    }
}
