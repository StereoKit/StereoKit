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
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi)]
        static extern IntPtr model_create_file(string file);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi)]
        static extern IntPtr model_create_mesh(string id, IntPtr mesh, IntPtr material);
        [DllImport(NativeLib.DllName)]
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
        ~Model()
        {
            if (_modelInst != IntPtr.Zero)
                model_release(_modelInst);
        }
    }
}
