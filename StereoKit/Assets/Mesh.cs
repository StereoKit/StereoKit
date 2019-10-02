using System;

namespace StereoKit
{
    public class Mesh
    {
        internal IntPtr _meshInst;
        public Mesh()
        {
            _meshInst = NativeAPI.mesh_create();
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
                NativeAPI.mesh_release(_meshInst);
        }

        public static Mesh GenerateCube(Vec3 dimensions, int subdivisions = 0)
        {
            return new Mesh(NativeAPI.mesh_gen_cube(dimensions, subdivisions));
        }
        public static Mesh GenerateRoundedCube(Vec3 dimensions, float edgeRadius, int subdivisions = 4)
        {
            return new Mesh(NativeAPI.mesh_gen_rounded_cube(dimensions, edgeRadius, subdivisions));
        }
        public static Mesh GenerateSphere(float diameter, int subdivisions = 4)
        {
            return new Mesh(NativeAPI.mesh_gen_sphere(diameter, subdivisions));
        }
        public static Mesh Find(string id)
        {
            IntPtr mesh = NativeAPI.mesh_find(id);
            return mesh == IntPtr.Zero ? null : new Mesh(mesh);
        }
    }
}
