using System;

namespace StereoKit
{
    public class Model
    {
        internal IntPtr _modelInst;
        public Model(string file)
        {
            _modelInst = NativeAPI.model_create_file(file);
            if (_modelInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Couldn't load {0}!", file);
        }
        public Model(Mesh mesh, Material material)
        {
            _modelInst = NativeAPI.model_create_mesh(mesh._meshInst, material._materialInst);
        }
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

        public static Model Find(string id)
        {
            IntPtr model = NativeAPI.model_find(id);
            return model == IntPtr.Zero ? null : new Model(model);
        }
    }
}
