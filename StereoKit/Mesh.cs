using System;

namespace StereoKit
{
    public class Mesh
    {
        internal IntPtr _meshInst;
        public Mesh(string id)
        {
            _meshInst = NativeAPI.mesh_create(id);
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

        public static Mesh GenerateCube(string id, Vec3 dimensions, int subdivisions = 0)
        {
            return new Mesh(NativeAPI.mesh_gen_cube(id, dimensions, subdivisions));
        }
        public static Mesh GenerateRoundedCube(string id, Vec3 dimensions, float edgeRadius, int subdivisions = 4)
        {
            return new Mesh(NativeAPI.mesh_gen_rounded_cube(id, dimensions, edgeRadius, subdivisions));
        }
        public static Mesh GenerateSphere(string id, float diameter, int subdivisions = 4)
        {
            return new Mesh(NativeAPI.mesh_gen_sphere(id, diameter, subdivisions));
        }
        public static Mesh Find(string id)
        {
            IntPtr mesh = NativeAPI.mesh_find(id);
            return mesh == IntPtr.Zero ? null : new Mesh(mesh);
        }
    }
}
