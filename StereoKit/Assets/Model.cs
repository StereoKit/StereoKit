using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
	/// <summary>A Model is a collection of meshes, materials, and transforms
	/// that make up a visual element! This is a great way to group together
	/// complex objects that have multiple parts in them, and in fact, most
	/// model formats are composed this way already!
	/// 
	/// This class contains a number of methods for creation. If you pass in
	/// a .obj, .stl, , .ply (ASCII), .gltf, or .glb, StereoKit will load
	/// that model from file, and assemble materials and transforms from the
	/// file information. But you can also assemble a model from procedurally 
	/// generated meshes!
	/// 
	/// Because models include an offset transform for each mesh element,
	/// this does have the overhead of an extra matrix multiplication in
	/// order to execute a render command. So if you need speed, and only
	/// have a single mesh with a precalculated transform matrix, it can be
	/// faster to render a Mesh instead of a Model!</summary>
	public class Model
	{
		internal IntPtr _inst;
		private ModelNodeCollection   _nodeCollection;
		private ModelVisualCollection _visualCollection;
		private ModelAnimCollection   _animCollection;

		/// <summary>The number of mesh subsets attached to this model.
		/// </summary>
		[Obsolete("For removal in v0.4. Use Nodes/Visuals instead.")]
		public int SubsetCount => NativeAPI.model_subset_count(_inst);

		/// <summary>This is an enumerable collection of all the nodes in this
		/// Model, ordered non-hierarchically by when they were added. You can
		/// do Linq stuff with it, foreach it, or just treat it like a List or
		/// array!</summary>
		public ModelNodeCollection Nodes => _nodeCollection;
		/// <summary>This is an enumerable collection of all the nodes with
		/// Mesh/Material data in this Model, ordered non-hierarchically by
		/// when they were added. You can do Linq stuff with it, foreach it, or
		/// just treat it like a List or array!</summary>
		public ModelVisualCollection Visuals => _visualCollection;
		/// <summary>An enumerable collection of animations attached to this
		/// Model. You can do Linq stuff with it, foreach it, or just treat it
		/// like a List or array!</summary>
		public ModelAnimCollection Anims => _animCollection;

		/// <summary>This is a bounding box that encapsulates the Model and
		/// all its subsets! It's used for collision, visibility testing, UI
		/// layout, and probably other things. While it's normally calculated
		/// from the mesh bounds, you can also override this to suit your
		/// needs.</summary>
		public Bounds Bounds
		{
			get => NativeAPI.model_get_bounds(_inst);
			set => NativeAPI.model_set_bounds(_inst, value);
		}

		/// <summary>Returns the first root node in the Model's hierarchy.
		/// There may be additional root nodes, and these will be Siblings
		/// of this ModelNode. If there are no nodes present on the Model,
		/// this will be null.</summary>
		public ModelNode RootNode
		{
			get
			{
				int nodeId = NativeAPI.model_node_get_root(_inst);
				return nodeId >= 0
					? new ModelNode(this, nodeId)
					: null;
			}
		}

		#region Constructors
		/// <summary>Creates a single mesh subset Model using the indicated
		/// Mesh and Material! An id will be automatically generated for this
		/// asset.</summary>
		/// <param name="mesh">Any Mesh asset.</param>
		/// <param name="material">Any Material asset.</param>
		public Model(Mesh mesh, Material material)
		{
			_inst = NativeAPI.model_create_mesh(mesh._inst, material._inst);
			_nodeCollection   = new ModelNodeCollection  (this);
			_visualCollection = new ModelVisualCollection(this);
			_animCollection   = new ModelAnimCollection  (this);
		}

		/// <summary>Creates an empty Model object with an automatically 
		/// generated id. Use the AddSubset methods to fill this model out.
		/// </summary>
		public Model()
		{
			_inst = NativeAPI.model_create();
			_nodeCollection   = new ModelNodeCollection  (this);
			_visualCollection = new ModelVisualCollection(this);
			_animCollection   = new ModelAnimCollection  (this);
		}

		/// <summary>Creates a single mesh subset Model using the indicated 
		/// Mesh and Material!</summary>
		/// <param name="id">Uses this as the id, so you can Find it later.
		/// </param>
		/// <param name="mesh">Any Mesh asset.</param>
		/// <param name="material">Any Material asset.</param>
		public Model(string id, Mesh mesh, Material material)
		{
			_inst = NativeAPI.model_create_mesh(mesh._inst, material._inst);
			_nodeCollection   = new ModelNodeCollection  (this);
			_visualCollection = new ModelVisualCollection(this);
			_animCollection   = new ModelAnimCollection  (this);
			if (_inst != IntPtr.Zero)
			{
				NativeAPI.material_set_id(_inst, id);
			}
		}
		internal Model(IntPtr model)
		{
			_inst = model;
			_nodeCollection   = new ModelNodeCollection  (this);
			_visualCollection = new ModelVisualCollection(this);
			_animCollection   = new ModelAnimCollection  (this);
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty model!");
		}
		~Model()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}
		#endregion

		#region Methods

		/// <summary>Creates a shallow copy of a Model asset! Meshes and 
		/// Materials referenced by this Model will be referenced, not
		/// copied.</summary>
		/// <returns>A new shallow copy of a Model.</returns>
		public Model Copy()
			=> new Model(NativeAPI.model_copy(_inst));

		/// <summary>Returns the name of the specific subset! This will be 
		/// the node name of your model asset. If no node name is available,
		/// SteroKit will generate a name in the format of "subsetX", where
		/// X would be the subset index. Note that names are not guaranteed
		/// to be unique (users may assign the same name to multiple nodes).
		/// Some nodes may also produce multiple subsets with the same name,
		/// such as when a node contains a Mesh with multiple Materials, each
		/// Mesh/Material combination will receive a subset with the same
		/// name.</summary>
		/// <param name="subsetIndex">Index of the model subset to get the 
		/// Material for, should be less than SubsetCount.</param>
		/// <returns>See summary for details.</returns>
		[Obsolete("For removal in v0.4. Use Nodes/Visuals.Name instead.")]
		public string GetName(int subsetIndex)
			=> Marshal.PtrToStringAnsi(NativeAPI.model_get_name(_inst, subsetIndex));

		/// <summary>Gets a link to the Material asset used by the model 
		/// subset! Note that this is not necessarily a unique material, and 
		/// could be shared in a number of other places. Consider copying and
		/// replacing it if you intend to modify it!</summary>
		/// <param name="subsetIndex">Index of the model subset to get the 
		/// Material for, should be less than SubsetCount.</param>
		/// <returns>A link to the Material asset used by the model subset at
		/// subsetIndex</returns>
		[Obsolete("For removal in v0.4. Use Nodes/Visuals.Material instead.")]
		public Material GetMaterial(int subsetIndex)
			=> new Material(NativeAPI.model_get_material(_inst, subsetIndex));

		/// <summary>Gets a link to the Mesh asset used by the model subset!
		/// Note that this is not necessarily a unique mesh, and could be 
		/// shared in a number of other places. Consider copying and
		/// replacing it if you intend to modify it!</summary>
		/// <param name="subsetIndex">Index of the model subset to get the
		/// Mesh for, should be less than SubsetCount.</param>
		/// <returns>A link to the Mesh asset used by the model subset at
		/// subsetIndex</returns>
		[Obsolete("For removal in v0.4. Use Nodes/Visuals.Mesh instead.")]
		public Mesh GetMesh(int subsetIndex)
			=> new Mesh(NativeAPI.model_get_mesh(_inst, subsetIndex));

		/// <summary>Gets the transform matrix used by the model subset!
		/// </summary>
		/// <param name="subsetIndex">Index of the model subset to get the 
		/// transform for, should be less than SubsetCount.</param>
		/// <returns>A transform matrix used by the model subset at 
		/// subsetIndex</returns>
		[Obsolete("For removal in v0.4. Use Nodes/Visuals.ModelTransform instead.")]
		public Matrix GetTransform(int subsetIndex)
			=> NativeAPI.model_get_transform(_inst, subsetIndex);

		/// <summary>Changes the Material for the subset to a new one!
		/// </summary>
		/// <param name="subsetIndex">Index of the model subset to replace, 
		/// should be less than SubsetCount.</param>
		/// <param name="material">The new Material, cannot be null.</param>
		[Obsolete("For removal in v0.4. Use Nodes/Visuals.Material instead.")]
		public void SetMaterial(int subsetIndex, Material material)
			=> NativeAPI.model_set_material(_inst, subsetIndex, material._inst);

		/// <summary>Changes the mesh for the subset to a new one!</summary>
		/// <param name="subsetIndex">Index of the model subset to replace, 
		/// should be less than SubsetCount.</param>
		/// <param name="mesh">The new Mesh, cannot be null.</param>
		[Obsolete("For removal in v0.4. Use Nodes/Visuals.Mesh instead.")]
		public void SetMesh(int subsetIndex, Mesh mesh)
			=> NativeAPI.model_set_mesh(_inst, subsetIndex, mesh._inst);

		/// <summary>Changes the transform for the subset to a new one! This 
		/// is in Model space, so it's relative to the origin of the model.
		/// </summary>
		/// <param name="subsetIndex">Index of the transform to replace,
		/// should be less than SubsetCount.</param>
		/// <param name="transform">The new transform.</param>
		[Obsolete("For removal in v0.4. Use Nodes/Visuals.ModelTransform instead.")]
		public void SetTransform(int subsetIndex, in Matrix transform)
			=> NativeAPI.model_set_transform(_inst, subsetIndex, transform);

		/// <summary>Adds a new subset to the Model, and recalculates the 
		/// bounds. A default subset name of "subsetX" will be used, where X
		/// is the subset's index.</summary>
		/// <param name="mesh">The Mesh for the subset, may not be null.
		/// </param>
		/// <param name="material">The Material for the subset, may not be 
		/// null.</param>
		/// <param name="transform">A transform Matrix representing the 
		/// Mesh's location relative to the origin of the Model.</param>
		/// <returns>The index of the subset that was just added.</returns>
		[Obsolete("For removal in v0.4. Use AddNode or Nodes/Visuals.AddChild instead.")]
		public int AddSubset(Mesh mesh, Material material, in Matrix transform)
			=> NativeAPI.model_add_subset(_inst, mesh._inst, material._inst, transform);

		/// <summary>Adds a new subset to the Model, and recalculates the 
		/// bounds.</summary>
		/// <param name="name">The text name of the subset. If this is null,
		/// then a default name of "subsetX" will be used, where X is the 
		/// subset's index.</param>
		/// <param name="mesh">The Mesh for the subset, may not be null.
		/// </param>
		/// <param name="material">The Material for the subset, may not be 
		/// null.</param>
		/// <param name="transform">A transform Matrix representing the 
		/// Mesh's location relative to the origin of the Model.</param>
		/// <returns>The index of the subset that was just added.</returns>
		[Obsolete("For removal in v0.4. Use AddNode or Nodes/Visuals.AddChild instead.")]
		public int AddSubset(string name, Mesh mesh, Material material, in Matrix transform)
			=> NativeAPI.model_add_named_subset(_inst, name, mesh._inst, material._inst, transform);

		/// <summary>Removes and dereferences a subset from the model.
		/// </summary>
		/// <param name="subsetIndex">Index of the subset to remove, should
		/// be less than SubsetCount.</param>
		[Obsolete("For removal in v0.4.")]
		public void RemoveSubset(int subsetIndex)
			=> NativeAPI.model_remove_subset(_inst, subsetIndex);

		/// <summary>Searches for an animation with the given name, and if it's
		/// found, sets it up as the active animation and begins playing it
		/// with the animation mode.</summary>
		/// <param name="animationName">Case sensitive name of the animation.
		/// </param>
		/// <param name="mode">The mode with which to play the animation.
		/// </param>
		public void PlayAnim(string animationName, AnimMode mode)
			=> NativeAPI.model_play_anim(_inst, animationName, mode);
		/// <summary>Sets the animation up as the active animation, and begins
		/// playing it with the animation mode.</summary>
		/// <param name="animation">The new active animation.</param>
		/// <param name="mode">The mode with which to play the animation.
		/// </param>
		public void PlayAnim(Anim animation, AnimMode mode)
		{
			if (animation._model._inst == this._inst)
				NativeAPI.model_play_anim_idx(_inst, animation._animIndex, mode);
		}
		/// <summary>Calling Draw will automatically step the Model's
		/// animation, but if you don't draw the Model, or need access to the
		/// animated nodes before drawing, then you can step the animation
		/// early manually via this method. Animation will only ever be stepped
		/// once per frame, so it's okay to call this multiple times, or in
		/// addition to Draw.</summary>
		public void StepAnim() => NativeAPI.model_step_anim(_inst);
		/// <summary>Searches the list of animations for the first one matching
		/// the given name.</summary>
		/// <param name="name">Case sensitive name of the animation.</param>
		/// <returns>A link to the animation, or null if none is found.</returns>
		public Anim FindAnim(string name)
		{
			int idx = NativeAPI.model_anim_find(_inst, name);
			return idx == -1
				? null
				: new Anim(this, idx);
		}
		/// <summary> This is a link to the currently active animation. If no
		/// animation is active, this value will be null. To set the active
		/// animation, use `PlayAnim`.</summary>
		public Anim ActiveAnim { 
			get
			{
				int idx = NativeAPI.model_anim_active(_inst);
				return idx == -1
					? null
					: new Anim(this, idx);
			}
		}
		/// <summary> This is the current time of the active animation in
		/// seconds, from the start of the animation. If no animation is
		/// active, this will be zero. This will always be a value between
		/// zero and the active animation's `Duration`. For a percentage of
		/// completion, see `AnimCompletion` instead.</summary>
		public float AnimTime
		{
			get => NativeAPI.model_anim_active_time(_inst);
			set => NativeAPI.model_set_anim_time(_inst, value);
		}
		/// <summary> This is the percentage of completion of the active
		/// animation. This will always be a value between 0-1. If no animation
		/// is active, this will be zero. </summary>
		public float AnimCompletion
		{
			get => NativeAPI.model_anim_active_completion(_inst);
			set => NativeAPI.model_set_anim_completion(_inst, value);
		}
		/// <summary> The playback mode of the active animation.</summary>
		public AnimMode AnimMode => NativeAPI.model_anim_active_mode(_inst);


		/// <summary>Checks the intersection point of this ray and a Model's 
		/// visual nodes. This will skip any node that is not flagged as Solid,
		/// as well as any Mesh without collision data. Ray must be in model 
		/// space, intersection point will be in model space too. You can use
		/// the inverse of the mesh's world transform matrix to bring the ray
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
			=> NativeAPI.model_ray_intersect(_inst, modelSpaceRay, out modelSpaceAt) > 0;

		/// <summary>This adds a root node to the `Model`'s node hierarchy! If
		/// There is already an initial root node, this node will still be a
		/// root node, but will be a `Sibling` of the `Model`'s `RootNode`. If
		/// this is the first root node added, you'll be able to access it via
		/// `RootNode`.</summary>
		/// <param name="name">A text name to identify the node. If null is
		/// provided, it will be auto named to "node"+index.</param>
		/// <param name="modelTransform">A Matrix describing this node's
		/// transform in Model space.</param>
		/// <param name="mesh">The Mesh to attach to this Node's visual, if
		/// this is null, then material must also be null.</param>
		/// <param name="material">The Material to attach to this Node's
		/// visual, if this is null, then mesh must also be null.</param>
		/// <param name="solid">A flag that indicates the Mesh for this node
		/// will be used in ray intersection tests. This flag is ignored if no
		/// Mesh is attached.</param>
		/// <returns>This returns the newly added ModelNode, or if there's an
		/// issue with mesh and material being inconsistently null, then this
		/// result will also be null.</returns>
		public ModelNode AddNode(string name, Matrix modelTransform, Mesh mesh = null, Material material = null, bool solid = true)
		{
			return new ModelNode(
				this,
				NativeAPI.model_node_add(_inst, name, modelTransform, mesh != null ? mesh._inst : IntPtr.Zero, material != null ? material._inst : IntPtr.Zero, solid ? 1 : 0));
		}

		/// <summary>Searches the entire list of Nodes, and will return the
		/// first on that matches this name exactly. If no ModelNode is found,
		/// then this will return null. Node Names are not guaranteed to be 
		/// unique.</summary>
		/// <param name="name">Exact name to match against. ASCII only for now.
		/// </param>
		/// <returns>The first matching ModelNode, or null if none are found.
		/// </returns>
		public ModelNode FindNode(string name)
		{
			int nodeId = NativeAPI.model_node_find(_inst, name);
			return nodeId >= 0
				? new ModelNode(this, nodeId)
				: null;
		}

		/// <summary>Examines the visuals as they currently are, and rebuilds
		/// the bounds based on that! This is normally done automatically,
		/// but if you modify a Mesh that this Model is using, the Model
		/// can't see it, and you should call this manually.</summary>
		public void RecalculateBounds()
			=> NativeAPI.model_recalculate_bounds(_inst);

		/// <inheritdoc cref="Model.Draw(Matrix)"/>
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
		public void Draw(Matrix transform, Color colorLinear, RenderLayer layer = RenderLayer.Layer0)
			=> NativeAPI.render_add_model(_inst, transform, colorLinear, layer);

		/// <summary>Adds this Model to the render queue for this frame! If
		/// the Hierarchy has a transform on it, that transform is combined
		/// with the Matrix provided here.</summary>
		/// <param name="transform">A Matrix that will transform the Model
		/// from Model Space into the current Hierarchy Space.</param>
		public void Draw(Matrix transform)
			=> NativeAPI.render_add_model(_inst, transform, Color.White, RenderLayer.Layer0);
		#endregion

		/// <summary>Looks for a Model asset that's already loaded, matching
		/// the given id!</summary>
		/// <param name="modelId">Which Model are you looking for?</param>
		/// <returns>A link to the Model matching 'modelId', null if none is
		/// found.</returns>
		public static Model Find(string modelId)
		{
			IntPtr model = NativeAPI.model_find(modelId);
			return model == IntPtr.Zero ? null : new Model(model);
		}

		/// <summary>Loads a list of mesh and material subsets from a .obj,
		/// .stl, .ply (ASCII), .gltf, or .glb file.</summary>
		/// <param name="file">Name of the file to load! This gets prefixed
		/// with the StereoKit asset folder if no drive letter is specified
		/// in the path.</param>
		/// <param name="shader">The shader to use for the model's materials!
		/// If null, this will
		/// automatically determine the best shader available to use.</param>
		/// <returns>A Model created from the file, or null if the file 
		/// failed to load!</returns>
		public static Model FromFile(string file, Shader shader = null)
		{
			IntPtr final = shader == null ? IntPtr.Zero : shader._inst;
			IntPtr inst = NativeAPI.model_create_file(Encoding.UTF8.GetBytes(file+'\0'), final);
			return inst == IntPtr.Zero ? null : new Model(inst);
		}

		/// <summary>Loads a list of mesh and material subsets from a .obj,
		/// .stl, .ply (ASCII), .gltf, or .glb file stored in memory. Note
		/// that this function won't work well on files that reference other
		/// files, such as .gltf files with references in them.</summary>
		/// <param name="filename">StereoKit still uses the filename of the
		/// data for format discovery, but not asset Id creation. If you 
		/// don't have a real filename for the data, just pass in an
		/// extension with a leading '.' character here, like ".glb".</param>
		/// <param name="data">The binary data of a model file, this is NOT 
		/// a raw array of vertex and index data!</param>
		/// <param name="shader">The shader to use for the model's materials!
		/// If null, this will automatically determine the best shader 
		/// available to use.</param>
		/// <returns>A Model created from the file, or null if the file
		/// failed to load!</returns>
		public static Model FromMemory(string filename, in byte[] data, Shader shader = null)
		{
			IntPtr final = shader == null ? IntPtr.Zero : shader._inst;
			IntPtr inst = NativeAPI.model_create_mem(Encoding.UTF8.GetBytes(filename+'\0'), data, (UIntPtr)data.Length, final);
			return inst == IntPtr.Zero ? null : new Model(inst);
		}

		/// <summary>Creates a single mesh subset Model using the indicated
		/// Mesh and Material! An id will be automatically generated for this
		/// asset.</summary>
		/// <param name="mesh">Any Mesh asset.</param>
		/// <param name="material">Any Material asset.</param>
		/// <returns>A Model composed of a single mesh and Material.</returns>
		public static Model FromMesh(Mesh mesh, Material material)
		{
			IntPtr inst = NativeAPI.model_create_mesh(mesh._inst, material._inst);
			return inst == IntPtr.Zero ? null : new Model(inst);
		}

		/// <summary>Creates a single mesh subset Model using the indicated
		/// Mesh and Material!</summary>
		/// <param name="id">Uses this as the id, so you can Find it later.
		/// </param>
		/// <param name="mesh">Any Mesh asset.</param>
		/// <param name="material">Any Material asset.</param>
		/// <returns>A Model composed of a single mesh and Material.</returns>
		public static Model FromMesh(string id, Mesh mesh, Material material)
		{
			IntPtr inst = NativeAPI.model_create_mesh(mesh._inst, material._inst);
			if (inst != IntPtr.Zero)
			{
				NativeAPI.material_set_id(inst, id);
				return new Model(inst);
			}
			return null;
		}
	}

	/// <summary>This class is a link to a node in a Model's internal
	/// hierarchy tree. It's composed of node information, and links to
	/// the directly adjacent tree nodes.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public class ModelNode
	{
		internal int _nodeId;
		internal Model _model;

		/// <summary>The next ModelNode in the hierarchy, at the same level as
		/// this one. To the "right" on a hierarchy tree. Null if there are no
		/// more ModelNodes in the tree there.</summary>
		public ModelNode Sibling => From(NativeAPI.model_node_sibling(_model._inst, _nodeId));
		/// <summary>The ModelNode above this one ("up") in the hierarchy tree,
		/// or null if this is a root node.</summary>
		public ModelNode Parent => From(NativeAPI.model_node_parent(_model._inst, _nodeId));
		/// <summary>The first child node "below" on the hierarchy tree, or
		/// null if there are none. To see all children, get the Child and then
		/// iterate through its Siblings.</summary>
		public ModelNode Child => From(NativeAPI.model_node_child(_model._inst, _nodeId));

		/// <summary>This is the ASCII name that identifies this ModelNode. It
		/// is generally provided by the Model's file, but in the event no name
		/// (or null name) is provided, the name will default to "node"+index.
		/// Names are not required to be unique.</summary>
		public string Name
		{
			get => Marshal.PtrToStringAnsi(NativeAPI.model_node_get_name(_model._inst, _nodeId));
			set => NativeAPI.model_node_set_name(_model._inst, _nodeId, value);
		}
		/// <summary>A flag that indicates the Mesh for this node will be used
		/// in ray intersection tests. This flag is ignored if no Mesh is 
		/// attached.</summary>
		public bool Solid
		{
			get => NativeAPI.model_node_get_solid(_model._inst, _nodeId) > 0;
			set => NativeAPI.model_node_set_solid(_model._inst, _nodeId, value ? 1 : 0);
		}
		/// <summary>Is this node flagged as visible? By default, this is true
		/// for all nodes with visual elements attached. These nodes will not
		/// be drawn or skinned if you set this flag to false. If a ModelNode
		/// has no visual elements attached to it, it will always return false,
		/// and setting this value will have no effect.</summary>
		public bool Visible
		{
			get => NativeAPI.model_node_get_visible(_model._inst, _nodeId) > 0;
			set => NativeAPI.model_node_set_visible(_model._inst, _nodeId, value ? 1 : 0);
		}
		/// <summary>The transform of this node relative to the Model itself.
		/// This incorporates transforms from all parent nodes. Setting this
		/// transform will update the LocalTransform, as well as all Child
		/// nodes below this one.</summary>
		public Matrix ModelTransform
		{
			get => NativeAPI.model_node_get_transform_model(_model._inst, _nodeId);
			set => NativeAPI.model_node_set_transform_model(_model._inst, _nodeId, value);
		}
		/// <summary>The transform of this node relative to the Parent node.
		/// Setting this transform will update the ModelTransform, as well as
		/// all Child nodes below this one.</summary>
		public Matrix LocalTransform
		{
			get => NativeAPI.model_node_get_transform_local(_model._inst, _nodeId);
			set => NativeAPI.model_node_set_transform_local(_model._inst, _nodeId, value);
		}
		/// <summary>The Mesh associated with this node. May be null, or may
		/// also be re-used elsewhere.</summary>
		public Mesh Mesh
		{
			get { IntPtr ptr = NativeAPI.model_node_get_mesh(_model._inst, _nodeId); return ptr == IntPtr.Zero ? null : new Mesh(ptr); }
			set => NativeAPI.model_node_set_mesh(_model._inst, _nodeId, value._inst);
		}
		/// <summary>The Model associated with this node. May be null, or may
		/// also be re-used elsewhere.</summary>
		public Material Material
		{
			get { IntPtr ptr = NativeAPI.model_node_get_material(_model._inst, _nodeId); return ptr == IntPtr.Zero ? null : new Material(ptr); }
			set => NativeAPI.model_node_set_material(_model._inst, _nodeId, value._inst);
		}

		internal ModelNode(Model model, int nodeId)
		{
			_model = model;
			_nodeId = nodeId;
		}

		/// <summary>Advances this ModelNode class to the next Sibling in the
		/// hierarchy tree. If it cannot, then it remains the same. </summary>
		/// <returns>True if it moved to the Sibling, false if there was no
		/// Sibling to move to.</returns>
		public bool MoveSibling()
		{
			int sibling = NativeAPI.model_node_sibling(_model._inst, _nodeId);
			if (sibling >= 0)
			{
				_nodeId = sibling;
				return true;
			}
			return false;
		}
		/// <summary>Moves this ModelNode class to the Parent up the hierarchy
		/// tree. If it cannot, then it remains the same. </summary>
		/// <returns>True if it moved to the Parent, false if there was no
		/// Parent to move to.</returns>
		public bool MoveParent()
		{
			int sibling = NativeAPI.model_node_parent(_model._inst, _nodeId);
			if (sibling >= 0)
			{
				_nodeId = sibling;
				return true;
			}
			return false;
		}
		/// <summary>Moves this ModelNode class to the first Child of this
		/// node. If it cannot, then it remains the same. </summary>
		/// <returns>True if it moved to the Child, false if there was no
		/// Child to move to.</returns>
		public bool MoveChild()
		{
			int sibling = NativeAPI.model_node_child(_model._inst, _nodeId);
			if (sibling >= 0)
			{
				_nodeId = sibling;
				return true;
			}
			return false;
		}

		/// <summary>Adds a Child node below this node, at the end of the child
		/// chain!</summary>
		/// <param name="name">A text name to identify the node. If null is
		/// provided, it will be auto named to "node"+index.</param>
		/// <param name="localTransform">A Matrix describing this node's
		/// transform in local space relative to the currently selected node.
		/// </param>
		/// <param name="mesh">The Mesh to attach to this Node's visual, if
		/// this is null, then material must also be null.</param>
		/// <param name="material">The Material to attach to this Node's
		/// visual, if this is null, then mesh must also be null.</param>
		/// <param name="solid">A flag that indicates the Mesh for this node
		/// will be used in ray intersection tests. This flag is ignored if no
		/// Mesh is attached.</param>
		/// <returns>This returns the newly added ModelNode, or if there's an
		/// issue with mesh and material being inconsistently null, then this
		/// result will also be null.</returns>
		public ModelNode AddChild(string name, Matrix localTransform, Mesh mesh = null, Material material = null, bool solid = true)
		{
			return new ModelNode(
				_model,
				NativeAPI.model_node_add_child(_model._inst, _nodeId, name, localTransform, mesh != null ? mesh._inst : IntPtr.Zero, material != null ? material._inst : IntPtr.Zero, solid ? 1 : 0));
		}

		private ModelNode From(int nodeId) => nodeId >= 0 ? new ModelNode(_model, nodeId) : null;
	}

	/// <summary>An enumerable for Model's ModelNodes</summary>
	public class ModelNodeCollection : IEnumerable<ModelNode>
	{
		Model _model;
		/// <summary>This is the total number of nodes in the Model.</summary>
		public int Count => NativeAPI.model_node_count(_model._inst);
		public ModelNode this[int index] => new ModelNode(_model, NativeAPI.model_node_index(_model._inst, index));

		internal ModelNodeCollection(Model model) { _model = model; }

		public IEnumerator<ModelNode> GetEnumerator()
		{
			int count = Count;
			for (int i = 0; i < count; i++)
				yield return new ModelNode(_model, NativeAPI.model_node_index(_model._inst, i));
		}
		IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

		/// <summary>This adds a root node to the `Model`'s node hierarchy! If
		/// There is already an initial root node, this node will still be a
		/// root node, but will be a `Sibling` of the `Model`'s `RootNode`. If
		/// this is the first root node added, you'll be able to access it via
		/// `RootNode`.</summary>
		/// <param name="name">A text name to identify the node. If null is
		/// provided, it will be auto named to "node"+index.</param>
		/// <param name="modelTransform">A Matrix describing this node's
		/// transform in Model space.</param>
		/// <param name="mesh">The Mesh to attach to this Node's visual, if
		/// this is null, then material must also be null.</param>
		/// <param name="material">The Material to attach to this Node's
		/// visual, if this is null, then mesh must also be null.</param>
		/// <param name="solid">A flag that indicates the Mesh for this node
		/// will be used in ray intersection tests. This flag is ignored if no
		/// Mesh is attached.</param>
		/// <returns>This returns the newly added ModelNode, or if there's an
		/// issue with mesh and material being inconsistently null, then this
		/// result will also be null.</returns>
		public ModelNode Add(string name, Matrix modelTransform, Mesh mesh = null, Material material = null, bool solid = true)
		{
			return new ModelNode(_model,
				NativeAPI.model_node_add(_model._inst, name, modelTransform, mesh != null ? mesh._inst : IntPtr.Zero, material != null ? material._inst : IntPtr.Zero, solid ? 1 : 0));
		}
	}

	/// <summary>An enumerable for Model's visual ModelNodes</summary>
	public class ModelVisualCollection : IEnumerable<ModelNode>
	{
		Model _model;
		/// <summary>This is the total number of nodes with visual data
		/// attached to them.</summary>
		public int Count => NativeAPI.model_node_visual_count(_model._inst);
		public ModelNode this[int index] => new ModelNode(_model, NativeAPI.model_node_visual_index(_model._inst, index));

		internal ModelVisualCollection(Model model) { _model = model; }

		public IEnumerator<ModelNode> GetEnumerator()
		{
			int count = Count;
			for (int i = 0; i < count; i++)
				yield return new ModelNode(_model, NativeAPI.model_node_visual_index(_model._inst, i));
		}
		IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
	}

	/// <summary>A link to a Model's animation! You can use this to get some
	/// basic information about the animation, or store it for reference. This
	/// maintains a link to the Model asset, and will keep it alive as long as
	/// this object lives.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public class Anim
	{
		internal int   _animIndex;
		internal Model _model;

		internal Anim(Model model, int animIndex)
		{
			_model = model;
			_animIndex = animIndex;
		}

		/// <summary>The name of the animation as provided by the original
		/// asset.</summary>
		public string Name => Marshal.PtrToStringAnsi(NativeAPI.model_anim_get_name(_model._inst, _animIndex));

		/// <summary>The duration of the animation at normal playback speed, in
		/// seconds.</summary>
		public float Duration => NativeAPI.model_anim_get_duration(_model._inst, _animIndex);
	}

	/// <summary>An enumerable for Model's Anims</summary>
	public class ModelAnimCollection : IEnumerable<Anim>
	{
		Model _model;
		/// <summary>This is the total number of animations attached to the
		/// model.</summary>
		public int Count => NativeAPI.model_anim_count(_model._inst);
		public Anim this[int index] => new Anim(_model, index);

		internal ModelAnimCollection(Model model) { _model = model; }

		public IEnumerator<Anim> GetEnumerator()
		{
			int count = Count;
			for (int i = 0; i < count; i++)
				yield return new Anim(_model, i);
		}
		IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
	}
}