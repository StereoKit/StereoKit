
namespace StereoKit
{
    public class Renderer
    {
        public static void Add(Mesh mesh, Material material, Matrix transform)
            => NativeAPI.render_add_mesh(mesh._meshInst, material._materialInst, ref transform, Color.White);
        public static void Add(Mesh mesh, Material material, Matrix transform, Color color)
            => NativeAPI.render_add_mesh(mesh._meshInst, material._materialInst, ref transform, color);
        
        public static void Add(Model model, Matrix transform)
            => NativeAPI.render_add_model(model._modelInst, ref transform, Color.White);
        public static void Add(Model model, Matrix transform, Color color)
            => NativeAPI.render_add_model(model._modelInst, ref transform, color);

        public static void SetClip(float nearPlane = 0.01f, float farPlane = 50)
            => NativeAPI.render_set_clip(nearPlane, farPlane);
        
        public static void SetView(Matrix cameraTransform)
            => NativeAPI.render_set_view(cameraTransform);
        
        public static void SetSkytex(Tex2D skyTexture, bool showSky = true)
            => NativeAPI.render_set_skytex(skyTexture._texInst, showSky?1:0);
        
        public static void Blit(Tex2D toRendertarget, Material material)
            => NativeAPI.render_blit(toRendertarget._texInst, material._materialInst);
        
    }
}