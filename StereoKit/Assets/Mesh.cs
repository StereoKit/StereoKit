using System;
using System.Collections.Generic;
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
	public class Mesh : IAsset
	{
		internal IntPtr                    _inst;
		private  List<Assets.CallbackData> _callbacks;

		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managing your assets, or finding
		/// them later on!</summary>
		public string Id
		{
			get => Marshal.PtrToStringAnsi(NativeAPI.mesh_get_id(_inst));
			set => NativeAPI.mesh_set_id(_inst, value);
		}

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

		/// <summary>This tells you the current state of the Mesh asset.
		/// A Mesh starts in the None state, transitions to LoadedMeta when
		/// bounds are available (async path), and Loaded once GPU upload
		/// completes.</summary>
		public AssetState AssetState => NativeAPI.mesh_asset_state(_inst);

		/// <summary>This event fires when the Mesh has finished loading
		/// and has data ready for rendering.</summary>
		public event Action<Mesh> OnLoaded {
			add {
				if (_callbacks == null) _callbacks = new List<Assets.CallbackData>();
				AssetOnLoadCallback callback = (a, _) => { NativeAPI.mesh_addref(a); value(new Mesh(a)); };
				_callbacks.Add(new Assets.CallbackData { action = value, callback = callback });
				NativeAPI.mesh_on_load(_inst, callback, IntPtr.Zero);
			}
			remove {
				if (_callbacks == null) throw new NullReferenceException();
				int i = _callbacks.FindIndex(d => (Action<Mesh>)d.action == value);
				if (i < 0) throw new KeyNotFoundException();
				NativeAPI.mesh_on_load_remove(_inst, _callbacks[i].callback);
				_callbacks.RemoveAt(i);
			}
		}

		/// <summary>Creates an empty Mesh asset. Use SetVerts and SetInds to
		/// add data to it!</summary>
		public Mesh()
		{
			_inst = NativeAPI.mesh_create();
			if (_inst == IntPtr.Zero)
				Log.Err("Couldn't create empty mesh!");
		}
		/// <summary>Creates a Mesh asset and sets its vertex and index
		/// data with control over upload behavior. This is a shorthand
		/// for creating a Mesh and calling SetData on it.</summary>
		/// <param name="vertices">An array of vertices for the mesh.
		/// Null is okay here, but may require a special shader.</param>
		/// <param name="indices">A list of face indices, must be a
		/// multiple of 3.</param>
		/// <param name="flags">Flags controlling upload behavior. See
		/// MeshData for options.</param>
		/// <param name="priority">Loading priority for async upload.
		/// Lower values load sooner.</param>
		public Mesh(Vertex[] vertices, uint[] indices, MeshData flags = MeshData.CalcBounds, int priority = 0) : this()
			=> SetData(vertices, indices, flags, priority);
		internal Mesh(IntPtr mesh)
		{
			_inst = mesh;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty mesh!");
		}
		/// <summary>Release reference to the StereoKit asset.</summary>
		~Mesh()
		{
			if (_callbacks != null)
			{
				foreach (var cb in _callbacks)
					NativeAPI.mesh_on_load_remove(_inst, cb.callback);
				_callbacks = null;
			}
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		/// <summary>Assigns the vertices and indices for this Mesh! This will
		/// create a vertex buffer and index buffer object on the graphics
		/// card. If you're calling this a second time, the buffers will be
		/// marked as dynamic and re-allocated. If you're calling this a third
		/// time, the buffer will only re-allocate if the buffer is too small,
		/// otherwise it just copies in the data!
		/// 
		/// Remember to set all the relevant values! Your material will often
		/// show black if the Normals or Colors are left at their default
		/// values.
		/// 
		/// Calling SetData is slightly more efficient than calling SetVerts
		/// and SetInds separately.</summary>
		/// <param name="vertices">An array of vertices to add to the mesh.
		/// Remember to set all the relevant values! Your material will often
		/// show black if the Normals or Colors are left at their default
		/// values. Null is okay here, but may require a special shader.</param>
		/// <param name="indices">A list of face indices, must be a multiple of
		/// 3. Each index represents a vertex from the provided vertex array.
		/// </param>
		/// <param name="calculateBounds">If true, this will also update the
		/// Mesh's bounds based on the vertices provided. Since this does
		/// require iterating through all the verts with some logic, there is
		/// performance cost to doing this. If you're updating a mesh
		/// frequently or need all the performance you can get, setting this to
		/// false is a nice way to gain some speed!</param>
		public void SetData(Vertex[] vertices, uint[] indices, bool calculateBounds = true)
			=> NativeAPI.mesh_set_data(_inst, vertices, vertices?.Length ?? 0, indices, indices.Length,
				calculateBounds ? MeshData.CalcBounds : MeshData.None, 0);

		/// <summary>Assigns the vertices and indices for this Mesh with
		/// control over upload behavior via flags. Upload is synchronous
		/// by default — pass MeshData.Async for background upload.</summary>
		/// <param name="vertices">An array of vertices to add to the mesh.
		/// Remember to set all the relevant values! Your material will often
		/// show black if the Normals or Colors are left at their default
		/// values. Null is okay here, but may require a special shader.</param>
		/// <param name="indices">A list of face indices, must be a multiple of
		/// 3. Each index represents a vertex from the provided vertex array.
		/// </param>
		/// <param name="flags">Flags controlling upload behavior. See
		/// MeshData for options.</param>
		/// <param name="priority">Loading priority for async upload. Lower
		/// values load sooner.</param>
		public void SetData(Vertex[] vertices, uint[] indices, MeshData flags, int priority = 0)
			=> NativeAPI.mesh_set_data(_inst, vertices, vertices?.Length ?? 0, indices, indices.Length, flags, priority);

		/// <summary>Assigns vertices with a custom vertex format along with
		/// face indices for this Mesh in a single call! The format is derived
		/// from T's [VertComponent] tagged fields, see SetVerts for details.
		///
		/// Calling SetData is slightly more efficient than calling SetVerts
		/// and SetInds separately.</summary>
		/// <param name="vertices">An array of vertices to add to the mesh.
		/// </param>
		/// <param name="indices">A list of face indices, must be a multiple of
		/// 3. Each index represents a vertex from the provided vertex array.
		/// </param>
		/// <param name="calculateBounds">If true, this will also update the
		/// Mesh's bounds based on the vertices provided. This requires the
		/// format to contain a float3 position component.</param>
		public void SetData<T>(T[] vertices, uint[] indices, bool calculateBounds = true) where T : unmanaged
			=> SetData(vertices, indices, calculateBounds ? MeshData.CalcBounds : MeshData.None, 0);

		/// <summary>Assigns vertices with a custom vertex format along with
		/// face indices for this Mesh in a single call, with control over
		/// upload behavior via flags! Upload is synchronous by default — pass
		/// MeshData.Async for background upload. The format is derived from
		/// T's [VertComponent] tagged fields, see SetVerts for details.
		/// </summary>
		/// <param name="vertices">An array of vertices to add to the mesh.
		/// </param>
		/// <param name="indices">A list of face indices, must be a multiple of
		/// 3. Each index represents a vertex from the provided vertex array.
		/// </param>
		/// <param name="flags">Flags controlling upload behavior. See
		/// MeshData for options.</param>
		/// <param name="priority">Loading priority for async upload. Lower
		/// values load sooner.</param>
		public void SetData<T>(T[] vertices, uint[] indices, MeshData flags, int priority = 0) where T : unmanaged
		{
			VertComponent[] format = VertLayout<T>.Components;
			if (vertices == null)
			{
				NativeAPI.mesh_set_data_fmt(_inst, format, format.Length, IntPtr.Zero, 0, indices, indices.Length, flags, priority);
				return;
			}
			GCHandle pin = GCHandle.Alloc(vertices, GCHandleType.Pinned);
			try     { NativeAPI.mesh_set_data_fmt(_inst, format, format.Length, pin.AddrOfPinnedObject(), vertices.Length, indices, indices.Length, flags, priority); }
			finally { pin.Free(); }
		}

		/// <summary>Assigns the vertices for this Mesh! This will create a
		/// vertex buffer object on the graphics card. If you're
		/// calling this a second time, the buffer will be marked as dynamic
		/// and re-allocated. If you're calling this a third time, the buffer
		/// will only re-allocate if the buffer is too small, otherwise it 
		/// just copies in the data!
		/// 
		/// Remember to set all the relevant values! Your material will often
		/// show black if the Normals or Colors are left at their default
		/// values.</summary>
		/// <param name="vertices">An array of vertices to add to the mesh.
		/// Remember to set all the relevant values! Your material will often
		/// show black if the Normals or Colors are left at their default
		/// values.</param>
		/// <param name="calculateBounds">If true, this will also update the
		/// Mesh's bounds based on the vertices provided. Since this does
		/// require iterating through all the verts with some logic, there is
		/// performance cost to doing this. If you're updating a mesh
		/// frequently or need all the performance you can get, setting this to
		/// false is a nice way to gain some speed!</param>
		public void SetVerts(Vertex[] vertices, bool calculateBounds = true)
			=> NativeAPI.mesh_set_verts(_inst, vertices, vertices.Length, calculateBounds);

		/// <summary>This marshalls the Mesh's vertex data into an array. If
		/// KeepData is false, then the Mesh is _not_ storing verts on the CPU,
		/// and this information will _not_ be available.
		///
		/// Due to the way marshalling works, this is _not_ a cheap function!
		/// </summary>
		/// <returns>An array of vertices representing the Mesh, or null if
		/// KeepData is false.</returns>
		public Vertex[] GetVerts()
		{
			NativeAPI.mesh_get_verts(_inst, out IntPtr ptr, out int size, Memory.Reference);
			if (ptr == IntPtr.Zero)
				return null;

			int szStruct = Marshal.SizeOf(typeof(Vertex));
			Vertex[] result = new Vertex[size];
			// AHHHHHH
			for (uint i = 0; i < size; i++)
				result[i] = Marshal.PtrToStructure<Vertex>(new IntPtr(ptr.ToInt64() + (szStruct * i)));
			return result;
		}

		/// <summary>Assigns vertices with a custom vertex format to this Mesh!
		/// The format is derived from T's fields, each of which must be tagged
		/// with a [VertComponent] attribute describing what it is. The shader
		/// this Mesh is drawn with must be one that works with the components
		/// this format provides, StereoKit's built-in shaders all expect
		/// position, normal, texcoord and color.
		///
		/// A T that doesn't exactly describe its own memory layout will throw
		/// an ArgumentException here, see [VertComponent] docs for the rules.
		/// </summary>
		/// <param name="vertices">An array of vertices to add to the mesh.
		/// </param>
		/// <param name="calculateBounds">If true, this will also update the
		/// Mesh's bounds based on the vertices provided. This requires the
		/// format to contain a float3 position component.</param>
		public void SetVerts<T>(T[] vertices, bool calculateBounds = true) where T : unmanaged
		{
			VertComponent[] format = VertLayout<T>.Components;
			GCHandle        pin    = GCHandle.Alloc(vertices, GCHandleType.Pinned);
			try     { NativeAPI.mesh_set_verts_fmt(_inst, format, format.Length, pin.AddrOfPinnedObject(), vertices.Length, calculateBounds); }
			finally { pin.Free(); }
		}

		/// <summary>This marshalls the vertex data of a custom format Mesh
		/// into an array of T. T's [VertComponent] derived format must exactly
		/// match the format the Mesh was created with, and KeepData must be
		/// true for vertex data to be available.
		///
		/// Due to the way marshalling works, this is _not_ a cheap function!
		/// </summary>
		/// <returns>An array of vertices representing the Mesh, or null if
		/// KeepData is false.</returns>
		public T[] GetVerts<T>() where T : unmanaged
		{
			NativeAPI.mesh_get_verts_fmt(_inst, out IntPtr fmtPtr, out int fmtCount, out IntPtr dataPtr, out int count, Memory.Reference);
			if (dataPtr == IntPtr.Zero)
				return null;

			VertComponent[] expected = VertLayout<T>.Components;
			if (fmtCount != expected.Length)
				throw new InvalidOperationException($"This Mesh's vertex format has {fmtCount} components, but {typeof(T).Name} describes {expected.Length}!");
			int szComp = Marshal.SizeOf(typeof(VertComponent));
			for (int i = 0; i < fmtCount; i++)
			{
				VertComponent comp = Marshal.PtrToStructure<VertComponent>(new IntPtr(fmtPtr.ToInt64() + (szComp * i)));
				if (comp.Format   != expected[i].Format   || comp.Count        != expected[i].Count ||
					comp.Semantic != expected[i].Semantic || comp.SemanticSlot != expected[i].SemanticSlot)
					throw new InvalidOperationException($"This Mesh's vertex format doesn't match component {i} of {typeof(T).Name}!");
			}

			// Bulk copy the raw bytes across via a pinned destination array.
			T[]      result = new T[count];
			int      bytes  = count * Marshal.SizeOf<T>();
			byte[]   buffer = new byte[bytes];
			Marshal.Copy(dataPtr, buffer, 0, bytes);
			GCHandle pin    = GCHandle.Alloc(result, GCHandleType.Pinned);
			try     { Marshal.Copy(buffer, 0, pin.AddrOfPinnedObject(), bytes); }
			finally { pin.Free(); }
			return result;
		}

		/// <summary>Assigns the face indices for this Mesh! Faces are always
		/// triangles, there are only ever three indices per face. This
		/// function will create a index buffer object on the graphics card. If
		/// you're calling this a second time, the buffer will be marked as
		/// dynamic and re-allocated. If you're calling this a third time, the
		/// buffer will only re-allocate if the buffer is too small, otherwise
		/// it just copies in the data!</summary>
		/// <param name="indices">A list of face indices, must be a multiple of
		/// 3. Each index represents a vertex from the array assigned using
		/// SetVerts.</param>
		public void SetInds (uint[] indices)
			=>NativeAPI.mesh_set_inds(_inst, indices, indices.Length);

		/// <summary>This marshalls the Mesh's index data into an array. If
		/// KeepData is false, then the Mesh is _not_ storing indices on the
		/// CPU, and this information will _not_ be available.
		/// 
		/// Due to the way marshalling works, this is _not_ a cheap function!
		/// </summary>
		/// <returns>An array of indices representing the Mesh, or null if
		/// KeepData is false.</returns>
		public uint[] GetInds()
		{
			NativeAPI.mesh_get_inds(_inst, out IntPtr ptr, out int size, Memory.Reference);
			if (ptr == IntPtr.Zero)
				return null;

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
			=> NativeAPI.mesh_ray_intersect(_inst, modelSpaceRay, Cull.Back, out modelSpaceAt, IntPtr.Zero);

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
		public bool Intersect(Ray modelSpaceRay, out Ray modelSpaceAt, out uint outStartInds)
			=> NativeAPI.mesh_ray_intersect_bvh(_inst, modelSpaceRay, Cull.Back, out modelSpaceAt, out outStartInds);

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
		/// <param name="modelSpaceAt">The intersection point of the ray and
		/// the mesh, if an intersection occurs. This is in model space, and
		/// must be transformed back into world space later.</param>
		/// <returns>True if an intersection occurs, false otherwise!
		/// </returns>
		public bool Intersect(Ray modelSpaceRay, out Vec3 modelSpaceAt)
		{
			bool result = NativeAPI.mesh_ray_intersect(_inst, modelSpaceRay, Cull.Back, out Ray intersection, IntPtr.Zero);
			modelSpaceAt = intersection.position;
			return result;
		}
		
		/// <summary>Indicates whether this Mesh has CPU skinning data
		/// attached. A Mesh gains skin data when SetSkin is called, or
		/// when it's loaded from a skinned glTF.</summary>
		public bool HasSkin => NativeAPI.mesh_has_skin(_inst);

		/// <summary>Creates an independent duplicate of this Mesh.
		/// Vertices, indices, bounds, and (if present) skin data are
		/// copied; the new Mesh has its own GPU buffers and shares no
		/// state with the source.
		///
		/// This is useful when one source mesh is shared across N
		/// animated entities: UpdateSkin mutates the target mesh's
		/// vertex buffer in place, so each entity needs its own Mesh
		/// instance to deform independently.
		///
		/// The source Mesh must have KeepData set to true.</summary>
		/// <returns>A new Mesh that shares no GPU state with this one.
		/// </returns>
		public Mesh Copy()
			=> new Mesh(NativeAPI.mesh_copy(_inst));

		/// <summary>Attaches CPU skinning data to this Mesh. Once skin
		/// data is set, call UpdateSkin each frame with the current
		/// bone palette to deform the vertex buffer.
		///
		/// KeepData must be true and vertex data must already be set
		/// before calling this — the deformation runs on the CPU and
		/// needs a copy of the rest-pose vertices to work from.
		///
		/// The bone palette passed to UpdateSkin is expected to be
		/// bone world transforms in the same coordinate system the
		/// resting transforms were authored in. The skinning matrix
		/// for bone `i` is computed as
		/// `bonePalette[i] * inverse(boneRestingTransforms[i])`.
		/// </summary>
		/// <param name="boneIds">Per-vertex bone indices, packed 4 per
		/// vertex (so this array has length VertCount * 4). Each index
		/// references a slot in the bone palette and resting transforms.
		/// </param>
		/// <param name="boneWeights">Per-vertex bone weights, one Vec4
		/// per vertex (length must equal VertCount). The four
		/// components correspond to the four bone ids for that vertex.
		/// Weights should sum to ~1 for a stable result.</param>
		/// <param name="boneRestingTransforms">Bind-pose transform for
		/// each bone, expressed in the mesh's model space. StereoKit
		/// inverts these internally to produce the inverse-bind
		/// matrices used by the skinning math.</param>
		public void SetSkin(ushort[] boneIds, Vec4[] boneWeights, Matrix[] boneRestingTransforms)
			=> NativeAPI.mesh_set_skin(_inst, boneIds, boneIds.Length / 4, boneWeights, boneWeights.Length, boneRestingTransforms, boneRestingTransforms.Length);

		/// <summary>Drives the per-frame CPU deformation for a skinned
		/// Mesh. SetSkin must have been called first. This walks every
		/// vertex, blends the bone transforms by weight, and re-uploads
		/// the deformed vertices to the GPU.
		///
		/// `bonePalette` holds the current world-space transform for
		/// each bone, in the same coordinate system the resting
		/// transforms passed to SetSkin were authored in. Its length
		/// must match the bone count supplied to SetSkin.
		///
		/// Because deformation mutates this Mesh's vertex buffer in
		/// place, two entities driven by different bone palettes need
		/// their own Mesh instance — use Copy on a shared source mesh
		/// to get per-instance deformation.</summary>
		/// <param name="bonePalette">World-space transform per bone for
		/// this frame. Length must match the bone count supplied to
		/// SetSkin.</param>
		public void UpdateSkin(Matrix[] bonePalette)
			=> NativeAPI.mesh_update_skin(_inst, bonePalette, bonePalette.Length);

		/// <summary>Retrieves the vertices associated with a particular
		/// triangle on the Mesh.</summary>
		/// <param name="triangleIndex">Starting index of the triangle, should
		/// be a multiple of 3.</param>
		/// <param name="a">The first vertex of the found triangle</param>
		/// <param name="b">The second vertex of the found triangle</param>
		/// <param name="c">The third vertex of the found triangle</param>
		/// <returns>Returns true if triangle index was valid</returns>
		public bool GetTriangle(uint triangleIndex, out Vertex a, out Vertex b, out Vertex c)
			=> NativeAPI.mesh_get_triangle(_inst, triangleIndex, out a, out b, out c);

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
		/// coordinates start at 0,0 at the -X,-Z corner, and go to 1,1 at the
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
		/// <param name="doubleSided">Should both sides of the plane be 
		/// rendered?</param>
		/// <returns>A plane mesh, pre-sized to the given dimensions.</returns>
		public static Mesh GeneratePlane(Vec2 dimensions, int subdivisions = 0, bool doubleSided = false)
			=> new Mesh(NativeAPI.mesh_gen_plane(dimensions, Vec3.Up, Vec3.Forward, subdivisions, doubleSided));

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
		/// <param name="doubleSided">Should both sides of the plane be 
		/// rendered?</param>
		/// <returns>A plane mesh, pre-sized to the given dimensions.</returns>
		public static Mesh GeneratePlane(Vec2 dimensions, Vec3 planeNormal, Vec3 planeTopDirection, int subdivisions = 0, bool doubleSided = false)
			=> new Mesh(NativeAPI.mesh_gen_plane(dimensions, planeNormal, planeTopDirection, subdivisions, doubleSided));

		/// <summary>Generates a circle on the XZ axis facing up that is 
		/// pre-sized to the given diameter. UV coordinates correspond to a unit 
		/// circle centered at 0.5, 0.5! That is, the right-most point on the 
		/// circle has UV coordinates 1, 0.5 and the top-most point has UV 
		/// coordinates 0.5, 1.
		/// 
		/// NOTE: This generates a completely new Mesh asset on the GPU, and
		/// is best done during 'initialization' of your app/scene.</summary>
		/// <param name="diameter">The diameter of the circle in meters, or 
		/// 2*radius. This is the full length from one side to the other.
		/// </param>
		/// <param name="spokes">How many vertices compose the circumference of 
		/// the circle? Clamps to a minimum of 3. More is smoother, but less 
		/// performant.</param>
		/// <param name="doubleSided">Should both sides of the circle be 
		/// rendered?</param>
		/// <returns>A circle mesh, pre-sized to the given dimensions.</returns>
		public static Mesh GenerateCircle(float diameter, int spokes = 16, bool doubleSided = false)
			=> new Mesh(NativeAPI.mesh_gen_circle(diameter, Vec3.Up, Vec3.Forward, spokes, doubleSided));

		/// <summary>Generates a circle with an arbitrary orientation that is
		/// pre-sized to the given diameter. UV coordinates start at the top 
		/// left indicated with 'planeTopDirection' and correspond to a unit 
		/// circle centered at 0.5, 0.5.
		/// 
		/// NOTE: This generates a completely new Mesh asset on the GPU, and
		/// is best done during 'initialization' of your app/scene.</summary>
		/// <param name="diameter">The diameter of the circle in meters, or 
		/// 2*radius. This is the full length from one side to the other.
		/// </param>
		/// <param name="planeNormal">What is the normal of the surface this
		/// circle is generated on?</param>
		/// <param name="planeTopDirection">A normal defines the plane, but 
		/// this is technically a rectangle on the 
		/// plane. So which direction is up? It's important for UVs, but 
		/// doesn't need to be exact. This function takes the planeNormal as
		/// law, and uses this vector to find the right and up vectors via
		/// cross-products.</param>
		/// <param name="spokes">How many vertices compose the circumference of 
		/// the circle? Clamps to a minimum of 3. More is smoother, but less 
		/// performant.</param>
		/// <param name="doubleSided">Should both sides of the circle be 
		/// rendered?</param>
		/// <returns>A circle mesh, pre-sized to the given dimensions.</returns>
		public static Mesh GenerateCircle(float diameter, Vec3 planeNormal, Vec3 planeTopDirection, int spokes = 16, bool doubleSided = false)
			=> new Mesh(NativeAPI.mesh_gen_circle(diameter, planeNormal, planeTopDirection, spokes, doubleSided));

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
		/// reference to it. If no Mesh is found, it returns null.</summary>
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
