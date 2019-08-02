using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
    public class Model
    {
        #region Imports
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern IntPtr model_find(string id);
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern IntPtr model_create_file(string file);
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern IntPtr model_create_mesh(string id, IntPtr mesh, IntPtr material);
        [DllImport(Util.DllName)]
        static extern void model_release(IntPtr model);
        #endregion

        internal IntPtr _modelInst;
        public Model(string file)
        {
            _modelInst = model_create_file(file);
            if (_modelInst == IntPtr.Zero)
                Console.WriteLine("Couldn't load {0}!", file);
        }
        public Model(string id, Mesh mesh, Material material)
        {
            _modelInst = model_create_mesh(id, mesh._meshInst, material._materialInst);
            if (_modelInst == IntPtr.Zero)
                Console.WriteLine("Couldn't make {0}!", id);
        }
        private Model(IntPtr model)
        {
            _modelInst = model;
            if (_modelInst == IntPtr.Zero)
                Console.WriteLine("Received an empty model!");
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
