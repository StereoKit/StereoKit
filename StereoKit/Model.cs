using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public class Model
    {
        #region Imports
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr model_find(string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr model_create_file(string file);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr model_create_mesh(string id, IntPtr mesh, IntPtr material);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void model_release(IntPtr model);
        #endregion

        internal IntPtr _modelInst;
        static int index;
        public Model(string file)
        {
            _modelInst = model_create_file(file);
            if (_modelInst == IntPtr.Zero)
                Log.Write(LogLevel.Warning, "Couldn't load {0}!", file);
        }
        public Model(Mesh mesh, Material material)
        {
            index += 1;
            _modelInst = model_create_mesh("auto/model"+index, mesh._meshInst, material._materialInst);
        }
        public Model(string id, Mesh mesh, Material material)
        {
            _modelInst = model_create_mesh(id, mesh._meshInst, material._materialInst);
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
                model_release(_modelInst);
        }

        public static Model Find(string id)
        {
            IntPtr model = model_find(id);
            return model == IntPtr.Zero ? null : new Model(model);
        }
    }
}
