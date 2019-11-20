using System;

namespace StereoKit
{
    /// <summary>
    /// A Mesh is a single collection of triangular faces with extra surface information to enhance 
    /// rendering! StereoKit meshes are composed of a list of vertices, and a list of indices to 
    /// connect the vertices into faces. Nothing more than that is stored here, so typically meshes
    /// are combined with Materials, or added to Models in order to draw them.
    /// 
    /// Mesh vertices are composed of a position, a normal (direction of the vert), a uv coordinate
    /// (for mapping a texture to the mesh's surface), and a 32 bit color containing red, green, blue,
    /// and alpha (transparency).
    /// 
    /// Mesh indices are stored as unsigned shorts, so they cap out at 65,535. This limits meshes to 
    /// 65,535 vertices. I may change this to integers later, now that I think about it...
    /// </summary>
    public class Mesh
    {
        internal IntPtr _meshInst;

        /// <summary>This is a bounding box that encapsulates the Mesh! It's used for collision, visibility
        /// testing, UI layout, and probably other things. While it's normally cacluated from the mesh
        /// vertices, you can also override this to suit your needs.</summary>
        public Bounds Bounds { 
            get => NativeAPI.mesh_get_bounds(_meshInst);
            set => NativeAPI.mesh_set_bounds(_meshInst, value);
        }

        /// <summary>Creates an empty Mesh asset. Use SetVerts and SetInds to add data to it!</summary>
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

        /// <summary>Generates a plane on the XZ axis facing up that is optionally subdivided, pre-sized to the given
        /// dimensions. UV coordinates start at 0,0 at the -X,-Z corer, and go to 1,1 at the +X,+Z corner!</summary>
        /// <param name="dimensions">How large is this plane on the XZ axis, in meters?</param>
        /// <param name="subdivisions">Use this to add extra slices of vertices across the plane. 
        /// This can be useful for some types of vertex-based effects!</param>
        /// <returns>A plane mesh, pre-sized to the given dimensions.</returns>
        public static Mesh GeneratePlane(Vec2 dimensions, int subdivisions = 0)
            => new Mesh(NativeAPI.mesh_gen_plane(dimensions, Vec3.Up, Vec3.Forward, subdivisions));

        /// <summary>Generates a plane with an arbitrary orientation that is optionally subdivided, pre-sized to the given
        /// dimensions. UV coordinates start at the top left indicated with 'planeTopDirection'.</summary>
        /// <param name="dimensions">How large is this plane on the XZ axis, in meters?</param>
        /// <param name="planeNormal">What is the normal of the surface this plane is generated on?</param>
        /// <param name="planeTopDirection">A normal defines the plane, but this is technically a rectangle on the 
        /// plane. So which direction is up? It's important for UVs, but doesn't need to be exact. This function takes
        /// the planeNormal as law, and uses this vector to find the right and up vectors via cross-products.</param>
        /// <param name="subdivisions">Use this to add extra slices of vertices across the plane. 
        /// This can be useful for some types of vertex-based effects!</param>
        /// <returns>A plane mesh, pre-sized to the given dimensions.</returns>
        public static Mesh GeneratePlane(Vec2 dimensions, Vec3 planeNormal, Vec3 planeTopDirection, int subdivisions = 0)
            => new Mesh(NativeAPI.mesh_gen_plane(dimensions, planeNormal, planeTopDirection, subdivisions));

        /// <summary>Generates a flat-shaded cube mesh, pre-sized to the given
        /// dimensions. UV coordinates are projected flat on each face, 0,0 -> 1,1. </summary>
        /// <param name="dimensions">How large is this cube on each axis, in meters?</param>
        /// <param name="subdivisions">Use this to add extra slices of vertices across the cube's 
        /// faces. This can be useful for some types of vertex-based effects!</param>
        /// <returns>A flat-shaded cube mesh, pre-sized to the given dimensions.</returns>
        public static Mesh GenerateCube(Vec3 dimensions, int subdivisions = 0)
            => new Mesh(NativeAPI.mesh_gen_cube(dimensions, subdivisions));

        /// <summary>Generates a cube mesh with rounded corners, pre-sized to the given
        /// dimensions. UV coordinates are 0,0 -> 1,1 on each face, meeting at the middle of the rounded
        /// corners.</summary>
        /// <param name="dimensions">How large is this cube on each axis, in meters?</param>
        /// <param name="edgeRadius">Radius of the corner rounding, in meters.</param>
        /// <param name="subdivisions">How many subdivisions should be used for creating the corners? 
        /// A larger value results in smoother corners, but can decrease performance.</param>
        /// <returns>A cube mesh with rounded corners, pre-sized to the given dimensions.</returns>
        public static Mesh GenerateRoundedCube(Vec3 dimensions, float edgeRadius, int subdivisions = 4)
            => new Mesh(NativeAPI.mesh_gen_rounded_cube(dimensions, edgeRadius, subdivisions));

        /// <summary>Generates a sphere mesh, pre-sized to the given diameter, created 
        /// by sphereifying a subdivided cube! UV coordinates are taken from the initial unspherified 
        /// cube.</summary>
        /// <param name="diameter">The diameter of the sphere in meters, or 2*radius. This is the 
        /// full length from one side to the other.</param>
        /// <param name="subdivisions">How many times should the initial cube be subdivided?</param>
        /// <returns>A sphere mesh, pre-sized to the given diameter, created by sphereifying a 
        /// subdivided cube! UV coordinates are taken from the initial unspherified cube.</returns>
        public static Mesh GenerateSphere(float diameter, int subdivisions = 4)
            => new Mesh(NativeAPI.mesh_gen_sphere(diameter, subdivisions));

        /// <summary>Generates a cylinder mesh, pre-sized to the given diameter and depth,
        /// UV coordinates are from a flattened top view right now. Additional development is needed for 
        /// making better UVs for the edges.</summary>
        /// <param name="diameter">Diameter of the circular part of the cylinder in meters. Diameter is 
        /// 2*radius.</param>
        /// <param name="depth">How tall is this cylinder, in meters?</param>
        /// <param name="direction">What direction do the circular surfaces face? This is the surface normal
        /// for the top, it does not need to be normalized.</param>
        /// <param name="subdivisions">How many vertices compose the edges of the cylinder? More is smoother,
        /// but less performant.</param>
        /// <returns>Returns a cylinder mesh, pre-sized to the given diameter and depth, UV coordinates 
        /// are from a flattened top view right now.</returns>
        public static Mesh GenerateCylinder(float diameter, float depth, Vec3 direction, int subdivisions = 16)
            => new Mesh(NativeAPI.mesh_gen_cylinder(diameter, depth, direction, subdivisions));
        
        /// <summary>Finds the Mesh with the matching id, and returns a reference to it. If no Mesh it found,
        /// it returns null.</summary>
        /// <param name="meshId">Id of the Mesh we're looking for.</param>
        /// <returns>A Mesh with a matching id, or null if none is found.</returns>
        public static Mesh Find(string meshId)
        {
            IntPtr mesh = NativeAPI.mesh_find(meshId);
            return mesh == IntPtr.Zero ? null : new Mesh(mesh);
        }

        public void Draw(Material material, Matrix transform, Color color)
            =>NativeAPI.render_add_mesh(_meshInst, material._materialInst, transform, color);
        public void Draw(Material material, Matrix transform)
            => NativeAPI.render_add_mesh(_meshInst, material._materialInst, transform, Color.White);
    }
}
