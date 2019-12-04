
using System;

namespace StereoKit
{
    /// <summary>Do you need to draw something? Well, you're probably in the right place!
    /// This static class includes a variety of different drawing methods, from rendering 
    /// Models and Meshes, to setting rendering options and drawing to offscreen surfaces!
    /// Even better, it's entirely a static class, so you can call it from anywhere :)</summary>
    public class Renderer
    {
        /// <summary>Set a cubemap skybox texture for rendering a background! This is only visible on Opaque
        /// displays, since transparent displays have the real world behind them already! StereoKit has a
        /// a default procedurally generated skybox. You can load one with Tex.FromEquirectangular, or 
        /// the Tex constructor that takes the cubeFaceFiles_xxyyzz parameter.</summary>
        public static Tex SkyTex
        {
            get  { IntPtr ptr = NativeAPI.render_get_skytex(); return ptr == IntPtr.Zero ? null : new Tex(ptr); }
            set => NativeAPI.render_set_skytex(value._inst);
        }

        /// <summary>Enables or disables rendering of the skybox texture! It's enabled by default on Opaque
        /// displays, and completely unavailable for transparent displays.</summary>
        public static bool EnableSky
        {
            get => NativeAPI.render_enabled_skytex();
            set => NativeAPI.render_enable_skytex(value);
        }

        public static void Add(Mesh mesh, Material material, Matrix transform)
            => NativeAPI.render_add_mesh(mesh._meshInst, material._materialInst, transform, Color.White);
        public static void Add(Mesh mesh, Material material, Matrix transform, Color color)
            => NativeAPI.render_add_mesh(mesh._meshInst, material._materialInst, transform, color);
        
        public static void Add(Model model, Matrix transform)
            => NativeAPI.render_add_model(model._modelInst, transform, Color.White);
        public static void Add(Model model, Matrix transform, Color color)
            => NativeAPI.render_add_model(model._modelInst, transform, color);

        /// <summary>Set the near and far clipping planes of the camera! These are important
        /// to z-buffer quality, especially when using low bit depth z-buffers as recommended
        /// for devices like the HoloLens. The smaller the range between the near and far planes,
        /// the better your z-buffer will look! If you see flickering on objects that are overlapping,
        /// try making the range smaller. </summary>
        /// <param name="nearPlane">The GPU discards pixels that are too close to the camera, this
        /// is that distance! It must be larger than zero, due to the projection math, which also means
        /// that numbers too close to zero may also produce weird results? This has an enforced minimum
        /// of 0.0001.</param>
        /// <param name="farPlane">At what distance from the camera does the GPU discard pixel? This is
        /// not true distance, but rather Z-axis distance from zero in View Space coordinates!</param>
        public static void SetClip(float nearPlane = 0.01f, float farPlane = 50)
            => NativeAPI.render_set_clip(nearPlane, farPlane);
        
        /// <summary>Sets the position of the camera! This only works for Flatscreen experiences, as
        /// the MR runtime will control the camera when active.</summary>
        /// <param name="cameraTransform">A transform matrix representing a transform to the camera's 
        /// orientation! This is not the final view transform, StereoKit will invert this matrix for use 
        /// in the transform into view space. A regular Matrix.TRS call will work great here!</param>
        public static void SetView(Matrix cameraTransform)
            => NativeAPI.render_set_view(cameraTransform);

        public static void Blit(Tex toRendertarget, Material material)
            => NativeAPI.render_blit(toRendertarget._inst, material._materialInst);

        /// <summary>Schedules a screenshot for the end of the frame! The view will be
        /// rendered from the given position at the given point, with a resolution the same
        /// size as the screen's surface. It'll be saved as a .jpg file at the filename
        /// provided.</summary>
        /// <param name="from">Viewpoint location.</param>
        /// <param name="at">Direction the viewpoint is looking at.</param>
        /// <param name="width">Size of the screenshot horizontally, in pixels.</param>
        /// <param name="height">Size of the screenshot vertically, in pixels.</param>
        /// <param name="filename">Filename to write the screenshot to! Note this'll be a 
        /// .jpg regardless of what file extension you use right now.</param>
        public static void Screenshot(Vec3 from, Vec3 at, int width, int height, string filename)
            => NativeAPI.render_screenshot(from, at, width, height, filename);

    }
}