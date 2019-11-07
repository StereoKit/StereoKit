using System;

namespace StereoKit
{
    /// <summary>A Model is a collection of meshes, materials, and transforms that 
    /// make up a visual element! This is a great way to group together complex 
    /// objects that have multiple parts in them, and in fact, most model formats
    /// are composed this way already!
    /// 
    /// This class contains a number of methods for creation. If you pass in a .obj,
    /// .gltf, or .glb, StereoKit will load that model from file, and assemble materials
    /// and transforms from the file information. But you can also assemble a model
    /// from procedurally generated meshes!
    /// 
    /// Because models include an offset transform for each mesh element, this does have the 
    /// overhead of an extra matrix multiplication in order to execute a render command.
    /// So if you need speed, and only have a single mesh with a precalculated transform
    /// matrix, it can be faster to render a Mesh instead of a Model!</summary>
    public class Model
    {
        internal IntPtr _modelInst;

        /// <summary>The number of mesh subsets attached to this model.</summary>
        public int SubsetCount => NativeAPI.model_subset_count(_modelInst);

        #region Constructors
        /// <summary>Loads a list of mesh and material subsets from a .obj, .gltf, or .glb file.</summary>
        /// <param name="file">Name of the file to load! This gets prefixed with the StereoKit asset
        /// folder if no drive letter is specified in the path.</param>
        public Model(string file)
        {
            _modelInst = NativeAPI.model_create_file(file);
            if (_modelInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Couldn't load {0}!", file);
        }
        /// <summary>Creates a single mesh subset Model using the indicated Mesh and Material! An
        /// id will be automatically generated for this asset.</summary>
        /// <param name="mesh">Any Mesh asset.</param>
        /// <param name="material">Any Material asset.</param>
        public Model(Mesh mesh, Material material)
        {
            _modelInst = NativeAPI.model_create_mesh(mesh._meshInst, material._materialInst);
        }
        /// <summary>Creates a single mesh subset Model using the indicated Mesh and Material!</summary>
        /// <param name="id">Uses this as the id, so you can Find it later.</param>
        /// <param name="mesh">Any Mesh asset.</param>
        /// <param name="material">Any Material asset.</param>
        public Model(string id, Mesh mesh, Material material)
        {
            _modelInst = NativeAPI.model_create_mesh(mesh._meshInst, material._materialInst);
            NativeAPI.material_set_id(_modelInst, id);
        }
        private Model(IntPtr model)
        {
            _modelInst = model;
            if (_modelInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Received an empty model!");
        }
        ~Model()
        {
            if (_modelInst != IntPtr.Zero)
                NativeAPI.model_release(_modelInst);
        }
        #endregion

        #region Methods
        /// <summary>Gets a link to the Material asset used by the mesh subset!</summary>
        /// <param name="subsetIndex">Index of the mesh subset to get the material for, should be less than SubsetCount.</param>
        /// <returns>A link to the Material asset used by the mesh subset at subsetIndex</returns>
        public Material GetMaterial(int subsetIndex) => new Material(NativeAPI.model_get_material(_modelInst, subsetIndex));
        #endregion

        public static Model Find(string id)
        {
            IntPtr model = NativeAPI.model_find(id);
            return model == IntPtr.Zero ? null : new Model(model);
        }
    }
}
