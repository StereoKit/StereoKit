using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
    public class Renderer
    {
        #region Imports
        [DllImport(Util.DllName)]
        static extern void render_add_mesh(IntPtr mesh, IntPtr material, IntPtr transform);
        [DllImport(Util.DllName)]
        static extern void render_set_camera(IntPtr cam, IntPtr cam_transform);
        #endregion

        public static void Add(Mesh mesh, Material material, Transform transform)
        {
            render_add_mesh(mesh._meshInst, material._materialInst, transform._transformInst);
        }
        public static void SetCamera(Camera camera, Transform cameraTransform)
        {
            render_set_camera(camera._cameraInst, cameraTransform._transformInst);
        }
    }
}
