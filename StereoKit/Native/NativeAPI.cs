using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    internal static class NativeAPI
    {
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int     sk_init(string app_name, Runtime preferred_runtime, int fallback = 1);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void    sk_shutdown();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int     sk_step([MarshalAs(UnmanagedType.FunctionPtr)] Action app_update);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Runtime sk_active_runtime();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void    sk_set_settings(Settings settings);

        ///////////////////////////////////////////
        ///
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern float   time_getf();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern double  time_get();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern float   time_elapsedf();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern double  time_elapsed();

        ///////////////////////////////////////////
        
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Quat quat_difference(ref Quat a, ref Quat b);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Quat quat_lookat    (Vec3 from, Vec3 at);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Quat quat_euler     (Vec3 euler_degrees);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Quat quat_lerp      (ref Quat a, ref Quat b, float t);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Quat quat_normalize (ref Quat a);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Quat quat_mul       (ref Quat a, ref Quat b);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Vec3 quat_mul_vec   (ref Quat a, ref Vec3 b);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr mesh_find         (string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr mesh_create       ();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   mesh_set_id       (IntPtr mesh, string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   mesh_release      (IntPtr mesh);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void mesh_set_verts    (IntPtr mesh, vert_t* vertices, int vertex_count);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void mesh_set_inds     (IntPtr mesh, vind_t* indices, int index_count);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   mesh_set_draw_inds(IntPtr mesh, int index_count);

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr mesh_gen_cube        (Vec3 dimensions, int subdivisions = 0);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr mesh_gen_sphere      (float diameter, int subdivisions = 4);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr mesh_gen_rounded_cube(Vec3 dimensions, float edge_radius, int subdivisions);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr tex2d_find                (string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr tex2d_create              (TexType type = TexType.Image, TexFormat format = TexFormat.Rgba32);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr tex2d_create_file         (string file);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr tex2d_create_cubemap_file (string equirectangular_file);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr tex2d_create_cubemap_files(string[] cube_face_file_xxyyzz);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   tex2d_set_id              (IntPtr texture, string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   tex2d_release             (IntPtr texture);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   tex2d_set_colors          (IntPtr texture, int width, int height, void* data);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   tex2d_set_options         (IntPtr texture, TexSample sample = TexSample.Linear, TexAddress address_mode = TexAddress.Wrap, int anisotropy_level = 4);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   tex2d_add_zbuffer         (IntPtr texture, TexFormat format = TexFormat.Depthstencil);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   tex2d_rtarget_clear       (IntPtr render_target, Color32 color);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   tex2d_rtarget_set_active  (IntPtr render_target);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   tex2d_get_data            (IntPtr texture, void* out_data, ulong out_data_size);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr font_find   (string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr font_create (string file);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   font_release(IntPtr font);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr font_get_tex(IntPtr font);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr shader_find        (string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr shader_create      (string hlsl);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr shader_create_file (string filename);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   shader_set_id      (IntPtr shader, string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern string shader_get_name    (IntPtr shader);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int    shader_set_code    (IntPtr shader, string hlsl);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int    shader_set_codefile(IntPtr shader, string filename);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   shader_release     (IntPtr shader);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr material_find            (string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr material_create          (IntPtr shader);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr material_copy            (IntPtr material);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_id          (IntPtr material, string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_release         (IntPtr material);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_alpha_mode  (IntPtr material, AlphaMode mode);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_cull        (IntPtr material, CullMode  mode);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_queue_offset(IntPtr material, int offset);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_float       (IntPtr material, string name, float   value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_color       (IntPtr material, string name, Color   value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_vector      (IntPtr material, string name, Vec4    value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_matrix      (IntPtr material, string name, Matrix  value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int    material_set_texture     (IntPtr material, string name, IntPtr  value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int    material_set_texture_id  (IntPtr material, ulong  id,   IntPtr  value);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_param       (IntPtr material, string name, MaterialParam type, const void *value);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_param_id    (IntPtr material, ulong    id,   MaterialParam type, const void *value);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int    material_get_param       (IntPtr material, string name, MaterialParam type, void *out_value);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int    material_get_param_id    (IntPtr material, ulong    id, MaterialParam type, void *out_value);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_get_param_info  (IntPtr material, int index, out string out_name, out MaterialParam out_type);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int    material_get_param_count (IntPtr material);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   material_set_shader      (IntPtr material, IntPtr shader);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr material_get_shader      (IntPtr material);

        ///////////////////////////////////////////
        
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void transform_initialize  (IntPtr transform);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void transform_set         (IntPtr transform, ref Vec3 position, ref Vec3 scale, ref Quat rotation );
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void transform_set_position(IntPtr transform, ref Vec3 position);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Vec3 transform_get_position(IntPtr transform);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void transform_set_scale   (IntPtr transform, ref Vec3 scale);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Vec3 transform_get_scale   (IntPtr transform);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void transform_set_rotation(IntPtr transform, ref Quat rotation);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Quat transform_get_rotation(IntPtr transform);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void transform_lookat      (IntPtr transform, ref Vec3 at);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Vec3 transform_forward     (IntPtr transform);

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   transform_update    (IntPtr transform);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   transform_matrix_out(IntPtr transform, ref Matrix result);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Matrix transform_matrix    (IntPtr transform);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Vec3   transform_world_to_local    (IntPtr transform, ref Vec3 world_coordinate);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Vec3   transform_local_to_world    (IntPtr transform, ref Vec3 local_coordinate);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Vec3   transform_world_to_local_dir(IntPtr transform, ref Vec3 world_direction);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Vec3   transform_local_to_world_dir(IntPtr transform, ref Vec3 local_direction);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr text_make_style(IntPtr font, float character_height, IntPtr material, TextAlign align);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   text_add_at    (IntPtr style, ref Matrix transform, string text, TextAlign position = TextAlign.XCenter | TextAlign.YCenter, float off_x = 0, float off_y = 0, float off_z = 0);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Vec2   text_size      (IntPtr style, string text);


        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr solid_create          (ref Vec3 position, ref Quat rotation, SolidType type = SolidType.Normal);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_release         (IntPtr solid);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_add_sphere      (IntPtr solid, float diameter,               float kilograms, ref Vec3 offset);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_add_box         (IntPtr solid, ref Vec3  dimensions,             float kilograms, ref Vec3 offset);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_add_capsule     (IntPtr solid, float diameter, float height, float kilograms, ref Vec3 offset);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_set_type        (IntPtr solid, SolidType type);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_set_enabled     (IntPtr solid, int enabled);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_move            (IntPtr solid, ref Vec3 position, ref Quat rotation);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_teleport        (IntPtr solid, ref Vec3 position, ref Quat rotation);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_set_velocity    (IntPtr solid, ref Vec3 meters_per_second);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_set_velocity_ang(IntPtr solid, ref Vec3 radians_per_second);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   solid_get_transform   (IntPtr solid, IntPtr out_transform);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr model_find        (string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr model_create_mesh (IntPtr mesh, IntPtr material);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr model_create_file (string filename);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   model_set_id      (IntPtr model, string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr model_get_material(IntPtr model, int subset);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int    model_subset_count(IntPtr model);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   model_release     (IntPtr model);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr sprite_create     (IntPtr sprite,   SpriteType type = SpriteType.Atlased, string atlas_id = "default");
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr sprite_create_file(string filename, SpriteType type = SpriteType.Atlased, string atlas_id = "default");
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   sprite_set_id     (IntPtr sprite, string id);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   sprite_release    (IntPtr sprite);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern float  sprite_get_aspect (IntPtr sprite);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void   sprite_draw       (IntPtr sprite, ref Matrix transform, Color32 color);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void camera_initialize(IntPtr cam, float fov, float clip_near, float clip_far);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void camera_view      (transform_t &cam_transform, matrix &result);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void camera_proj      (camera_t    &cam,           matrix &result);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_set_camera  (IntPtr cam);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_set_view    (IntPtr cam_transform);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_set_light   (ref Vec3 direction, float intensity, ref Color color);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_set_skytex  (IntPtr sky_texture, int show_sky);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_add_mesh    (IntPtr mesh, IntPtr material, ref Matrix transform, Color color);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_add_mesh_tr (IntPtr mesh, IntPtr material, IntPtr transform, Color color);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_add_model   (IntPtr model, ref Matrix transform, Color color);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_add_model_tr(IntPtr model, IntPtr transform, Color color);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_blit        (IntPtr to_rendertarget, IntPtr material);
        //[DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void render_get_device (void **device, void **context);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int     input_pointer_count(InputSource filter = InputSource.Any);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern Pointer input_pointer      (int index, InputSource filter = InputSource.Any);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr  input_hand         (Handed hand);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern IntPtr  input_mouse        ();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern BtnState input_key         (Key key);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void    input_hand_visible (Handed hand, int visible);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void    input_hand_solid   (Handed hand, int solid);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void    input_hand_material(Handed hand, IntPtr material);

        ///////////////////////////////////////////
        
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void input_subscribe  (InputSource source, InputState evt, InputEventCallback event_callback);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void input_unsubscribe(InputSource source, InputState evt, InputEventCallback event_callback);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void input_fire_event (InputSource source, InputState evt, IntPtr pointer);

        ///////////////////////////////////////////
            
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void log_write     (LogLevel level, string text);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void log_set_filter(LogLevel level);

        ///////////////////////////////////////////

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void ui_init        ();

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void ui_nextline    ();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void ui_sameline    ();
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void ui_reserve_box (Vec2 size);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void ui_space       (float space);

        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void ui_label       (string text);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void ui_image       (IntPtr sprite_image, Vec2 size);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int  ui_button      (string text);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int  ui_affordance  (string text, ref Pose movement, Vec3 at, Vec3 size);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern int  ui_hslider     (string id,   ref float value, float min, float max, float step, float width = 0);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void ui_window_begin(string text, ref Pose pose, Vec2 size);
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)] public static extern void ui_window_end  ();
    }
}
