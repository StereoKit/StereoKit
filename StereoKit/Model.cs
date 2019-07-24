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
        static extern IntPtr model_create_file(string file);
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
        ~Model()
        {
            if (_modelInst != IntPtr.Zero)
                model_release(_modelInst);
        }
    }
}
