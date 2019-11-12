
namespace StereoKit
{
    /// <summary>Do you need to draw something? Well, you're probably in the right place!
    /// This static class includes a variety of different drawing methods, from rendering 
    /// Models and Meshes, to setting rendering options and drawing to offscreen surfaces!
    /// Even better, it's entirely a static class, so you can call it from anywhere :)</summary>
    public class Renderer
    {
        public static void Add(Mesh mesh, Material material, Matrix transform)
            => NativeAPI.render_add_mesh(mesh._meshInst, material._materialInst, transform, Color.White);
        public static void Add(Mesh mesh, Material material, Matrix transform, Color color)
            => NativeAPI.render_add_mesh(mesh._meshInst, material._materialInst, transform, color);
        
        public static void Add(Model model, Matrix transform)
            => NativeAPI.render_add_model(model._modelInst, transform, Color.White);
        public static void Add(Model model, Matrix transform, Color color)
            => NativeAPI.render_add_model(model._modelInst, transform, color);

        public static void SetClip(float nearPlane = 0.01f, float farPlane = 50)
            => NativeAPI.render_set_clip(nearPlane, farPlane);
        
        public static void SetView(Matrix cameraTransform)
            => NativeAPI.render_set_view(cameraTransform);
        
        public static void SetSkytex(Tex skyTexture, bool showSky = true)
            => NativeAPI.render_set_skytex(skyTexture._texInst, showSky?1:0);
        
        public static void Blit(Tex toRendertarget, Material material)
            => NativeAPI.render_blit(toRendertarget._texInst, material._materialInst);
        
        /// <summary>Schedules a screenshot for the end of the frame! The view will be
        /// rendered from the given position at the given point, with a resolution the same
        /// size as the screen's surface. It'll be saved as a .jpg file at the filename
        /// provided.</summary>
        /// <param name="from">Viewpoint location.</param>
        /// <param name="at">Direction the viewpoint is looking at.</param>
        /// <param name="filename">Filename to write the screenshot to! Note this'll be a 
        /// .jpg regardless of what file extension you use right now.</param>
        public static void Screenshot(Vec3 from, Vec3 at, string filename)
            => NativeAPI.render_screenshot(from, at, filename);
    }
}