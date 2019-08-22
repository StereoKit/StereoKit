using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public class Renderer
    {
        #region Imports
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void render_add_mesh(IntPtr mesh, IntPtr material, IntPtr transform);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void render_add_model(IntPtr model, IntPtr transform);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void render_set_camera(IntPtr cam);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void render_set_view(IntPtr transform);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void render_set_skytex(IntPtr sky_texture, int show_sky);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void render_blit(IntPtr to_rendertarget, IntPtr material);
        #endregion

        public static void Add(Mesh mesh, Material material, Transform transform)
        {
            render_add_mesh(mesh._meshInst, material._materialInst, transform._transformInst);
        }
        public static void Add(Model model, Transform transform)
        {
            render_add_model(model._modelInst, transform._transformInst);
        }
        public static void SetCamera(Camera camera)
        {
            render_set_camera(camera._cameraInst);
        }
        public static void SetView(Transform cameraTransform)
        {
            render_set_view(cameraTransform._transformInst);
        }
        public static void SetSkytex(Tex2D skyTexture, bool showSky = true)
        {
            render_set_skytex(skyTexture._texInst, showSky?1:0);
        }
        public static void Blit(Tex2D toRendertarget, Material material)
        {
            render_blit(toRendertarget._texInst, material._materialInst);
        }
    }
}
