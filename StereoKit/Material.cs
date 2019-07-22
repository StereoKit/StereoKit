using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
    public class Material
    {
        #region Imports
        [DllImport(Util.DllName)]
        static extern IntPtr material_create(IntPtr shader, IntPtr texture);
        [DllImport(Util.DllName)]
        static extern void material_release(IntPtr material);
        #endregion

        internal IntPtr _materialInst;
        public Material(Shader shader, Tex2D texture)
        {
            _materialInst = material_create(shader._shaderInst, texture._texInst);
            if (_materialInst == IntPtr.Zero)
                Console.WriteLine("Couldn't create material!");
        }
        ~Material()
        {
            if (_materialInst == IntPtr.Zero)
                material_release(_materialInst);
        }
    }
}
