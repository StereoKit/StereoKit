using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>
	/// A Mesh is a single collection of triangular faces with extra surface
	/// information to enhance rendering! StereoKit meshes are composed of a
	/// list of vertices, and a list of indices to connect the vertices into
	/// faces. Nothing more than that is stored here, so typically meshes are
	/// combined with Materials, or added to Models in order to draw them.
	/// 
	/// Mesh vertices are composed of a position, a normal (direction of the
	/// vert), a uv coordinate (for mapping a texture to the mesh's surface),
	/// and a 32 bit color containing red, green, blue, and alpha
	/// (transparency).
	/// 
	/// Mesh indices are stored as unsigned ints, so you can have a mesh with
	/// a fudgeton of verts! 4 billion or so :)
	/// </summary>
	public class Mesh
	{
		internal IntPtr _inst;

		/// <summary>This is a bounding box that encapsulates the Mesh! It's
		/// used for collision, visibility testing, UI layout, and probably 
		/// other things. While it's normally calculated from the mesh vertices,
		/// you can also override this to suit your needs.</summary>
		public Bounds Bounds { 
			get => NativeAPI.mesh_get_bounds(_inst);
			set => NativeAPI.mesh_set_bounds(_inst, value);
		}

		/// <summary>Should StereoKit keep the mesh data on the CPU for later
		/// access, or collision detection? Defaults to true. If you set this 
		/// to false before setting data, the data won't be stored. If you 
		/// call this after setting data, that stored data will be freed! If 
		/// you set this to true again later on, it will not contain data 
		/// until it's set again.</summary>
		public bool KeepData {
			get => NativeAPI.mesh_get_keep_data(_inst);
			set => NativeAPI.mesh_set_keep_data(_inst, value);
		}

		/// <summary>The number of vertices stored in this Mesh! This is
		/// available to you regardless of whether or not KeepData is set.
		/// </summary>
		public int VertCount => NativeAPI.mesh_get_vert_count(_inst);

		/// <summary>The number of indices stored in this Mesh! This is
		/// available to you regardless of whether or not KeepData is set.
		/// </summary>
		public int IndCount => NativeAPI.mesh_get_ind_count(_inst);

		/// <summary>Creates an empty Mesh asset. Use SetVerts and SetInds to
		/// add data to it!</summary>
		public Mesh()
		{
			_inst = NativeAPI.mesh_create();
			if (_inst == IntPtr.Zero)
				Log.Err("Couldn't create empty mesh!");
		}
		internal Mesh(IntPtr mesh)
		{
			_inst = mesh;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty mesh!");
		}
		~Mesh()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		/// <summary>Assigns the vertices for this Mesh! This will create a
		/// vertex buffer object on the graphics card right away. If you're
		/// calling this a second time, the buffer will be marked as dynamic
		/// and re-allocated. If you're calling this a third time, the buffer
		/// will only re-allocate if the buffer is too small, otherwise it 
		/// just copies in the data!</summary>
		/// 
		/// Remember to set all the relevant values! Your material will often
		/// show black if the Normals or Colors are left at their default
		/// values.
		/// <param name="verts">An array of vertices to add to the mesh.
		/// Remember to set all the relevant values! Your material will often
		/// show black if the Normals or Colors are left at their default
		/// values.</param>
		public void SetVerts(Vertex[] verts)
			=>NativeAPI.mesh_set_verts(_inst, verts, verts.Length);

		public Vertex[] GetVerts()
		{
			NativeAPI.mesh_get_verts(_inst, out IntPtr ptr, out int size, Memory.Reference);
			int szStruct = Marshal.SizeOf(typeof(Vertex));
			Vertex[] result = new Vertex[size];
			// AHHHHHH
			for (uint i = 0; i < size; i++)
				result[i] = Marshal.PtrToStructure<Vertex>(new IntPtr(ptr.ToInt64() + (szStruct * i)));
			return result;
		}

		/// <summary>Assigns the face indices for this Mesh! Faces are always
		/// triangles, there are only ever three indices per face. This
		/// function will create a index buffer object on the graphics card
		/// right away. If you're calling this a second time, the buffer will
		/// be marked as dynamic and re-allocated. If you're calling this a
		/// third time, the buffer will only re-allocate if the buffer is too
		/// small, otherwise it just copies in the data!</summary>
		/// <param name="inds">A list of face indices, must be a multiple of
		/// 3. Each index represents a vertex from the array assigned using
		/// SetVerts.</param>
		public void SetInds (uint[] inds)
			=>NativeAPI.mesh_set_inds(_inst, inds, inds.Length);

		public uint[] GetInds()
		{
			NativeAPI.mesh_get_inds(_inst, out IntPtr ptr, out int size, Memory.Reference);
			int szStruct = Marshal.SizeOf(typeof(uint));
			uint[] result = new uint[size];
			// AHHHHHH
			for (uint i = 0; i < size; i++)
				result[i] = Marshal.PtrToStructure<uint>(new IntPtr(ptr.ToInt64() + (szStruct * i)));
			return result;
		}

		/// <summary>Checks the intersection point of this ray and a Mesh 
		/// with collision data stored on the CPU. A mesh without collision
		/// data will always return false. Ray must be in model space, 
		/// intersection point will be in model space too. You can use the
		/// inverse of the mesh's world transform matrix to bring the ray
		/// into model space, see the example in the docs!</summary>
		/// <param name="modelSpaceRay">Ray must be in model space, the
		/// intersection point will be in model space too. You can use the
		/// inverse of the mesh's world transform matrix to bring the ray
		/// into model space, see the example in the docs!</param>
		/// <param name="modelSpaceAt">The intersection point and surface
		/// direction of the ray and the mesh, if an intersection occurs.
		/// This is in model space, and must be transformed back into world
		/// space later. Direction is not guaranteed to be normalized, 
		/// especially if your own model->world transform contains scale/skew
		/// in it.</param>
		/// <returns>True if an intersection occurs, false otherwise!
		/// </returns>
		public bool Intersect(Ray modelSpaceRay, out Ray modelSpaceAt)
			=> NativeAPI.mesh_ray_intersect(_inst, modelSpaceRay, out modelSpaceAt, IntPtr.Zero) > 0;

		/// <summary>Checks the intersection point of this ray and a Mesh 
		/// with collision data stored on the CPU. A mesh without collision
		/// data will always return false. Ray must be in model space, 
		/// intersection point will be in model space too. You can use the
		/// inverse of the mesh's world transform matrix to bring the ray
		/// into model space, see the example in the docs!</summary>
		/// <param name="modelSpaceRay">Ray must be in model space, the
		/// intersection point will be in model space too. You can use the
		/// inverse of the mesh's world transform matrix to bring the ray
		/// into model space, see the example in the docs!</param>
		/// <param name="modelSpaceAt">The intersection point and surface
		/// direction of the ray and the mesh, if an intersection occurs.
		/// This is in model space, and must be transformed back into world
		/// space later. Direction is not guaranteed to be normalized, 
		/// especially if your own model->world transform contains scale/skew
		/// in it.</param>
		/// <param name="outStartInds">The index of the first index of the triangle that was hit</param>
		/// <returns>True if an intersection occurs, false otherwise!
		/// </returns>
		public bool Intersect(Ray modelSpaceRay, out Ray modelSpaceAt,out uint outStartInds)
			=> NativeAPI.mesh_ray_intersect(_inst, modelSpaceRay, out modelSpaceAt, out outStartInds) > 0;

		/// <summary>Retrieves the vertices associated with a particular
		/// triangle on the Mesh.</summary>
		/// <param name="triangleIndex">Starting index of the triangle, should
		/// be a multiple of 3.</param>
		/// <param name="a">The first vertex of the found triangle</param>
		/// <param name="b">The second vertex of the found triangle</param>
		/// <param name="c">The third vertex of the found triangle</param>
		/// <returns>Returns true if triangle index was valid</returns>
		public bool GetTriangle(uint triangleIndex, out Vertex a, out Vertex b, out Vertex c)
			=> NativeAPI.mesh_get_triangle(_inst, triangleIndex, out a, out b, out c) == 1;

		// TODO: Remove in v0.4
		[Obsolete("Removing in v0.4, replace with the Mesh.Intersect overload with a Ray output.")]
		public bool Intersect(Ray modelSpaceRay, out Vec3 modelSpaceAt)
		{
			bool result = NativeAPI.mesh_ray_intersect(_inst, modelSpaceRay, out Ray intersection, IntPtr.Zero) > 0;
			modelSpaceAt = intersection.position;
			return result;
		}

		/// <inheritdoc cref="Mesh.Draw(Material, Matrix)"/>
		/// <param name="colorLinear">A per-instance linear space color value
		/// to pass into the shader! Normally this gets used like a material
		/// tint. If you're  adventurous and don't need per-instance colors,
		/// this is a great spot to pack in extra per-instance data for the
		/// shader!</param>
		/// <param name="layer">All visuals are rendered using a layer 
		/// bit-flag. By default, all layers are rendered, but this can be 
		/// useful for filtering out objects for different rendering 
		/// purposes! For example: rendering a mesh over the user's head from
		/// a 3rd person perspective, but filtering it out from the 1st
		/// person perspective.</param>
		public void Draw(Material material, Matrix transform, Color colorLinear, RenderLayer layer = RenderLayer.Layer0)
			=> NativeAPI.render_add_mesh(_inst, material._inst, transform, colorLinear, layer);

		/// <summary>Adds a mesh to the render queue for this frame! If the
		/// Hierarchy has a transform on it, that transform is combined with
		/// the Matrix provided here.</summary>
		/// <param name="material">A Material to apply to the Mesh.</param>
		/// <param name="transform">A Matrix that will transform the mesh 
		/// from Model Space into the current Hierarchy Space.</param>
		public void Draw(Material material, Matrix transform)
			=> NativeAPI.render_add_mesh(_inst, material._inst, transform, Color.White, RenderLayer.Layer0);

		/// <summary>Generates a plane on the XZ axis facing up that is
		/// optionally subdivided, pre-sized to the given dimensions. UV
		/// coordinates start at 0,0 at the -X,-Z corer, and go to 1,1 at the
		/// +X,+Z corner!
		/// 
		/// NOTE: This generates a completely new Mesh asset on the GPU, and
		/// is best done during 'initialization' of your app/scene. You may
		/// also be interested in using the pre-generated `Mesh.Quad` asset
		/// if it already meets your needs.</summary>
		/// <param name="dimensions">How large is this plane on the XZ axis,
		/// in meters?</param>
		/// <param name="subdivisions">Use this to add extra slices of 
		/// vertices across the plane. This can be useful for some types of
		/// vertex-based effects!</param>
		/// <returns>A plane mesh, pre-sized to the given dimensions.</returns>
		public static Mesh GeneratePlane(Vec2 dimensions, int subdivisions = 0)
			=> new Mesh(NativeAPI.mesh_gen_plane(dimensions, Vec3.Up, Vec3.Forward, subdivisions));

		/// <summary>Generates a plane with an arbitrary orientation that is
		/// optionally subdivided, pre-sized to the given dimensions. UV 
		/// coordinates start at the top left indicated with 
		/// 'planeTopDirection'.
		/// 
		/// NOTE: This generates a completely new Mesh asset on the GPU, and
		/// is best done during 'initialization' of your app/scene. You may
		/// also be interested in using the pre-generated `Mesh.Quad` asset
		/// if it already meets your needs.</summary>
		/// <param name="dimensions">How large is this plane on the XZ axis, 
		/// in meters?</param>
		/// <param name="planeNormal">What is the normal of the surface this
		/// plane is generated on?</param>
		/// <param name="planeTopDirection">A normal defines the plane, but 
		/// this is technically a rectangle on the 
		/// plane. So which direction is up? It's important for UVs, but 
		/// doesn't need to be exact. This function takes the planeNormal as
		/// law, and uses this vector to find the right and up vectors via
		/// cross-products.</param>
		/// <param name="subdivisions">Use this to add extra slices of 
		/// vertices across the plane. This can be useful for some types of
		/// vertex-based effects!</param>
		/// <returns>A plane mesh, pre-sized to the given dimensions.</returns>
		public static Mesh GeneratePlane(Vec2 dimensions, Vec3 planeNormal, Vec3 planeTopDirection, int subdivisions = 0)
			=> new Mesh(NativeAPI.mesh_gen_plane(dimensions, planeNormal, planeTopDirection, subdivisions));

		/// <summary>Generates a flat-shaded cube mesh, pre-sized to the
		/// given dimensions. UV coordinates are projected flat on each face,
		/// 0,0 -> 1,1.
		/// 
		/// NOTE: This generates a completely new Mesh asset on the GPU, and
		/// is best done during 'initialization' of your app/scene. You may
		/// also be interested in using the pre-generated `Mesh.Cube` asset
		/// if it already meets your needs.</summary>
		/// <param name="dimensions">How large is this cube on each axis, in 
		/// meters?</param>
		/// <param name="subdivisions">Use this to add extra slices of
		/// vertices across the cube's 
		/// faces. This can be useful for some types of vertex-based effects
		/// !</param>
		/// <returns>A flat-shaded cube mesh, pre-sized to the given 
		/// dimensions.</returns>
		public static Mesh GenerateCube(Vec3 dimensions, int subdivisions = 0)
			=> new Mesh(NativeAPI.mesh_gen_cube(dimensions, subdivisions));

		/// <summary>Generates a cube mesh with rounded corners, pre-sized to
		/// the given dimensions. UV coordinates are 0,0 -> 1,1 on each face,
		/// meeting at the middle of the rounded corners.
		/// 
		/// NOTE: This generates a completely new Mesh asset on the GPU, and
		/// is best done during 'initialization' of your app/scene.</summary>
		/// <param name="dimensions">How large is this cube on each axis, in
		/// meters?</param>
		/// <param name="edgeRadius">Radius of the corner rounding, in
		/// meters.</param>
		/// <param name="subdivisions">How many subdivisions should be used
		/// for creating the corners? 
		/// A larger value results in smoother corners, but can decrease
		/// performance.</param>
		/// <returns>A cube mesh with rounded corners, pre-sized to the given
		/// dimensions.</returns>
		public static Mesh GenerateRoundedCube(Vec3 dimensions, float edgeRadius, int subdivisions = 4)
			=> new Mesh(NativeAPI.mesh_gen_rounded_cube(dimensions, edgeRadius, subdivisions));

		/// <summary>Generates a sphere mesh, pre-sized to the given
		/// diameter, created by sphereifying a subdivided cube! UV
		/// coordinates are taken from the initial unspherified cube.
		/// 
		/// NOTE: This generates a completely new Mesh asset on the GPU, and
		/// is best done during 'initialization' of your app/scene. You may
		/// also be interested in using the pre-generated `Mesh.Sphere` asset
		/// if it already meets your needs.
		/// </summary>
		/// <param name="diameter">The diameter of the sphere in meters, or 
		/// 2*radius. This is the full length from one side to the other.
		/// </param>
		/// <param name="subdivisions">How many times should the initial cube
		/// be subdivided?</param>
		/// <returns>A sphere mesh, pre-sized to the given diameter, created
		/// by sphereifying a subdivided cube! UV coordinates are taken from
		/// the initial unspherified cube.</returns>
		public static Mesh GenerateSphere(float diameter, int subdivisions = 4)
			=> new Mesh(NativeAPI.mesh_gen_sphere(diameter, subdivisions));

		/// <summary>Generates a cylinder mesh, pre-sized to the given 
		/// diameter and depth, UV coordinates are from a flattened top view
		/// right now. Additional development is needed for making better UVs
		/// for the edges.
		/// 
		/// NOTE: This generates a completely new Mesh asset on the GPU, and
		/// is best done during 'initialization' of your app/scene.</summary>
		/// <param name="diameter">Diameter of the circular part of the
		/// cylinder in meters. Diameter is 2*radius.</param>
		/// <param name="depth">How tall is this cylinder, in meters?</param>
		/// <param name="direction">What direction do the circular surfaces 
		/// face? This is the surface normal for the top, it does not need to
		/// be normalized.</param>
		/// <param name="subdivisions">How many vertices compose the edges of
		/// the cylinder? More is smoother, but less performant.</param>
		/// <returns>Returns a cylinder mesh, pre-sized to the given diameter
		/// and depth, UV coordinates are from a flattened top view right 
		/// now.</returns>
		public static Mesh GenerateCylinder(float diameter, float depth, Vec3 direction, int subdivisions = 16)
			=> new Mesh(NativeAPI.mesh_gen_cylinder(diameter, depth, direction, subdivisions));

		/// <summary>Finds the Mesh with the matching id, and returns a 
		/// reference to it. If no Mesh it found, it returns null.</summary>
		/// <param name="meshId">Id of the Mesh we're looking for.</param>
		/// <returns>A Mesh with a matching id, or null if none is found.
		/// </returns>
		public static Mesh Find(string meshId)
		{
			IntPtr mesh = NativeAPI.mesh_find(meshId);
			return mesh == IntPtr.Zero ? null : new Mesh(mesh);
		}

		/// <inheritdoc cref="Default.MeshSphere" />
		public static Mesh Sphere => Default.MeshSphere;
		/// <inheritdoc cref="Default.MeshCube" />
		public static Mesh Cube => Default.MeshCube;
		/// <inheritdoc cref="Default.MeshQuad" />
		public static Mesh Quad => Default.MeshQuad;
	}
}
