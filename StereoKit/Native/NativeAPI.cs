using System;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
	internal static class NativeAPI
	{
		const string            dll  = "StereoKitC";
		const CharSet           cSet = CharSet.Ansi;
		const CallingConvention call = CallingConvention.Cdecl;

		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool        sk_init(SKSettings settings);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void        sk_set_window(IntPtr window);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void        sk_set_window_xam(IntPtr window);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void        sk_shutdown();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void        sk_shutdown_unsafe();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void        sk_quit(QuitReason quitReason = QuitReason.User);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool        sk_step([MarshalAs(UnmanagedType.FunctionPtr)] Action app_update);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void        sk_run([MarshalAs(UnmanagedType.FunctionPtr)] Action app_update, [MarshalAs(UnmanagedType.FunctionPtr)] Action app_shutdown);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool        sk_is_stepping();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern DisplayMode sk_active_display_mode();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern SystemInfo  sk_system_info();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr      sk_version_name();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong       sk_version_id();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern AppFocus    sk_app_focus();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern QuitReason  sk_get_quit_reason();

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern DisplayType    device_display_get_type        ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern DisplayBlend   device_display_get_blend       ();
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool           device_display_set_blend       (DisplayBlend blend);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool           device_display_valid_blend     (DisplayBlend blend);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float          device_display_get_refresh_rate();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int            device_display_get_width       ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int            device_display_get_height      ();
		//[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern FovInfo        device_display_get_fov         ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern DeviceTracking device_get_tracking            ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr         device_get_name                ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr         device_get_runtime             ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr         device_get_gpu                 ();
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool           device_has_eye_gaze            ();
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool           device_has_hand_tracking       ();

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float       time_totalf_unscaled();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern double      time_total_unscaled();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float       time_totalf();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern double      time_total();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float       time_stepf_unscaled();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern double      time_step_unscaled();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float       time_stepf();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern double      time_step();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void        time_scale(double scale);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void        time_set_time(double total_seconds, double frame_elapsed_seconds=0);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong       time_frame();

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Quat quat_difference (in Quat a, in Quat b);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Quat quat_lookat     (in Vec3 from, in Vec3 at);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Quat quat_lookat_up  (in Vec3 from, in Vec3 at, in Vec3 up);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pose   matrix_transform_pose  (Matrix transform, Pose pose);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Matrix matrix_trs             (in Vec3 position, in Quat orientation, in Vec3 scale);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3   matrix_extract_scale   (in Matrix transform);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Quat   matrix_extract_rotation(in Matrix transform);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pose   matrix_extract_pose    (in Matrix transform);

		///////////////////////////////////////////

		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   plane_ray_intersect    (Plane plane, Ray ray, out Vec3 out_pt);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   plane_line_intersect   (Plane plane, Vec3 p1, Vec3 p2, out Vec3 out_pt);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3   plane_point_closest    (Plane plane, Vec3 pt);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   sphere_ray_intersect   (Sphere sphere, Ray ray, out Vec3 out_pt);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   sphere_point_contains  (Sphere sphere, Vec3 pt);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   bounds_ray_intersect   (Bounds bounds, Ray ray, out Vec3 out_pt);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   bounds_point_contains  (Bounds bounds, Vec3 pt);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   bounds_line_contains   (Bounds bounds, Vec3 linePt1, Vec3 linePt2);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   bounds_capsule_contains(Bounds bounds, Vec3 linePt1, Vec3 linePt2, float radius);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds bounds_grow_to_fit_pt  (Bounds bounds, Vec3 pt);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds bounds_grow_to_fit_box (Bounds bounds, Bounds box, in Matrix opt_transform);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds bounds_grow_to_fit_box (Bounds bounds, Bounds box, IntPtr opt_transform);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds bounds_transform       (Bounds bounds, Matrix transform);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3   ray_point_closest      (Ray ray, Vec3 pt);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   ray_from_mouse         (Vec2 screen_pixel_pos, out Ray out_ray);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color color_hsv   (float hue, float saturation, float value, float transparency);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3  color_to_hsv(in Color color);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color color_lab   (float l, float a, float b, float transparency);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3  color_to_lab(in Color color);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color color_to_linear(Color srgb_gamma_correct);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color color_to_gamma (Color srgb_linear);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr  gradient_create ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr  gradient_create_keys([In] GradientKey[] keys, int count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void    gradient_add    (IntPtr gradient, Color color, float position);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void    gradient_set    (IntPtr gradient, int index, Color color, float position);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void    gradient_remove (IntPtr gradient, int index);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int     gradient_count  (IntPtr gradient);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color   gradient_get    (IntPtr gradient, float at);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color32 gradient_get32  (IntPtr gradient, float at);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void    gradient_destroy(IntPtr gradient);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern SphericalHarmonics sh_create      ([In] SHLight[] lights, int light_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               sh_brightness  (ref SphericalHarmonics harmonics, float scale);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               sh_add         (ref SphericalHarmonics harmonics, Vec3 light_dir, Vec3 light_color);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color              sh_lookup      (in  SphericalHarmonics harmonics, Vec3 normal);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3               sh_dominant_dir(in  SphericalHarmonics harmonics);


		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mesh_find            (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mesh_create          ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_set_id          (IntPtr mesh, string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mesh_get_id          (IntPtr mesh);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_addref          (IntPtr mesh);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_release         (IntPtr mesh);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_set_keep_data   (IntPtr mesh, [MarshalAs(UnmanagedType.Bool)] bool keep_data);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   mesh_get_keep_data   (IntPtr mesh);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_set_data        (IntPtr mesh, [In] Vertex[] vertices, int vertex_count, [In] uint[] indices, int index_count, [MarshalAs(UnmanagedType.Bool)] bool calculate_bounds);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_set_verts       (IntPtr mesh, [In] Vertex[] vertices, int vertex_count, [MarshalAs(UnmanagedType.Bool)] bool calculate_bounds);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_get_verts       (IntPtr mesh, out IntPtr out_vertices, out int out_vertex_count, Memory reference_mode);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    mesh_get_vert_count  (IntPtr mesh);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_set_inds        (IntPtr mesh, [In] uint[] indices, int index_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_get_inds        (IntPtr mesh, out IntPtr out_indices,  out int out_index_count, Memory reference_mode); // [Out, MarshalAs(unmanagedType:UnmanagedType.LPArray, SizeParamIndex=2)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    mesh_get_ind_count   (IntPtr mesh);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_set_draw_inds   (IntPtr mesh, int index_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mesh_set_bounds      (IntPtr mesh, in Bounds bounds);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds mesh_get_bounds      (IntPtr mesh);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   mesh_ray_intersect   (IntPtr mesh, Ray model_space_ray, Cull cull_mode, out Ray out_pt, IntPtr out_start_inds);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   mesh_ray_intersect   (IntPtr mesh, Ray model_space_ray, Cull cull_mode, out Ray out_pt, out uint out_start_inds);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   mesh_get_triangle    (IntPtr mesh, uint triangle_index, out Vertex a, out Vertex b, out Vertex c);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mesh_gen_plane       (Vec2 dimensions, Vec3 plane_normal, Vec3 plane_top_direction, int subdivisions, [MarshalAs(UnmanagedType.Bool)] bool double_sided);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mesh_gen_circle      (float diameter,  Vec3 plane_normal, Vec3 plane_top_direction, int spokes, [MarshalAs(UnmanagedType.Bool)] bool double_sided);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mesh_gen_cube        (Vec3 dimensions, int subdivisions = 0);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mesh_gen_sphere      (float diameter, int subdivisions = 4);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mesh_gen_rounded_cube(Vec3 dimensions, float edge_radius, int subdivisions);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mesh_gen_cylinder    (float diameter, float depth, Vec3 direction, int subdivisions);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_find                (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create              (TexType type = TexType.Image, TexFormat format = TexFormat.Rgba32);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_rendertarget (int width, int height, int msaa, TexFormat color_format, TexFormat depth_format);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_color32      ([In] Color32[] data, int width, int height, [MarshalAs(UnmanagedType.Bool)] bool srgb_data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_color128     ([In] Color[]   data, int width, int height, [MarshalAs(UnmanagedType.Bool)] bool srgb_data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_mem          ([In] byte[] data, UIntPtr data_size, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, int priority);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_file         ([In] byte[] file_utf8, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, int priority);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_file_arr     (string[] files, int file_count, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, int priority);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_cubemap_file ([In] byte[] equirectangular_file_utf8, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, IntPtr lighting_info, int priority);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_cubemap_file ([In] byte[] equirectangular_file_utf8, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, out SphericalHarmonics lighting_info, int priority);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_cubemap_files(string[] cube_face_file_xxyyzz, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, IntPtr lighting_info, int priority);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_create_cubemap_files(string[] cube_face_file_xxyyzz, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, out SphericalHarmonics lighting_info, int priority);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_id              (IntPtr texture, string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_get_id              (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_fallback        (IntPtr texture, IntPtr fallback);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_release             (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_addref              (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern AssetState tex_asset_state     (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_on_load             (IntPtr texture, AssetOnLoadCallback on_load, IntPtr context);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_on_load_remove      (IntPtr texture, AssetOnLoadCallback on_load);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, IntPtr data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, [In] Color32[] data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, [In] Color[] data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, [In] byte[] data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, [In] ushort[] data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, [In] float[] data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_mem             (IntPtr texture, [In] byte[] data, UIntPtr data_size, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, [MarshalAs(UnmanagedType.Bool)] bool blocking, int priority);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_surface         (IntPtr texture, IntPtr native_surface, TexType type, long native_fmt, int width, int height, int surface_count, int multisample, int framebuffer_multisample, [MarshalAs(UnmanagedType.Bool)] bool owned);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_get_surface         (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_add_zbuffer         (IntPtr texture, TexFormat format = TexFormat.DepthStencil);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_set_zbuffer         (IntPtr texture, IntPtr depth_texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_get_zbuffer         (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   tex_get_data            (IntPtr texture, IntPtr out_data, UIntPtr out_data_size, int mip_level);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_gen_color           (Color color, int width, int height, TexType type, TexFormat format);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_gen_particle        (int width, int height, float roundness, IntPtr gradient);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_gen_cubemap         (IntPtr gradient, Vec3 gradient_dir, int resolution, IntPtr lighting_info);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_gen_cubemap         (IntPtr gradient, Vec3 gradient_dir, int resolution, out SphericalHarmonics lighting_info);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr tex_gen_cubemap_sh      (in SphericalHarmonics lighting, int resolution, float light_spot_size_pct, float light_spot_intensity);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern TexFormat  tex_get_format      (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int        tex_get_width       (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int        tex_get_height      (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void       tex_set_sample      (IntPtr texture, TexSample sample = TexSample.Linear);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern TexSample  tex_get_sample      (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void       tex_set_address     (IntPtr texture, TexAddress address_mode = TexAddress.Wrap);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern TexAddress tex_get_address     (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void       tex_set_anisotropy  (IntPtr texture, int anisotropy_level = 4);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int        tex_get_anisotropy  (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int        tex_get_mips        (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void       tex_set_loading_fallback(IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void       tex_set_error_fallback  (IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern SphericalHarmonics tex_get_cubemap_lighting(IntPtr cubemap_texture);
		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr font_find        (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr font_create      ([In] byte[] file_utf8);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr font_create_files(string[] file, int file_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   font_set_id      (IntPtr font, string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr font_get_id      (IntPtr font);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   font_release     (IntPtr font);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr font_get_tex     (IntPtr font);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr font_create_family(string font_family);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr shader_find        (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr shader_create_file ([In] byte[] filename_utf8);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr shader_create_mem  ([In] byte[] data, UIntPtr data_size);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   shader_set_id      (IntPtr shader, string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr shader_get_id      (IntPtr shader);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr shader_get_name    (IntPtr shader);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   shader_release     (IntPtr shader);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr material_find            (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr material_create          (IntPtr shader);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr material_copy            (IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr material_copy_id         (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_id          (IntPtr material, string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr material_get_id          (IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_release         (IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_transparency(IntPtr material, Transparency mode);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_cull        (IntPtr material, Cull         mode);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_wireframe   (IntPtr material, [MarshalAs(UnmanagedType.Bool)] bool wireframe);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_depth_test  (IntPtr material, DepthTest    depth_test_mode);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_depth_write (IntPtr material, [MarshalAs(UnmanagedType.Bool)] bool write_enabled);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_queue_offset(IntPtr material, int          offset);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_chain       (IntPtr material, IntPtr       chain_material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Transparency material_get_transparency(IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Cull         material_get_cull        (IntPtr material);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool         material_get_wireframe   (IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern DepthTest    material_get_depth_test  (IntPtr material);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool         material_get_depth_write (IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int          material_get_queue_offset(IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr       material_get_chain       (IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_float       (IntPtr material, string name, float  value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_color       (IntPtr material, string name, Color  value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_vector4     (IntPtr material, string name, Vec4   value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_vector3     (IntPtr material, string name, Vec3   value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_vector2     (IntPtr material, string name, Vec2   value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_int         (IntPtr material, string name, int  value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_int2        (IntPtr material, string name, int  value1, int value2);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_int3        (IntPtr material, string name, int  value1, int value2, int value3);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_int4        (IntPtr material, string name, int  value1, int value2, int value3, int value4);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_bool        (IntPtr material, string name, [MarshalAs(UnmanagedType.Bool)] bool value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_uint        (IntPtr material, string name, uint value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_uint2       (IntPtr material, string name, uint value1, uint value2);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_uint3       (IntPtr material, string name, uint value1, uint value2, uint value3);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_uint4       (IntPtr material, string name, uint value1, uint value2, uint value3, uint value4);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_matrix      (IntPtr material, string name, Matrix value);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   material_set_texture     (IntPtr material, string name, IntPtr value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float  material_get_float       (IntPtr material, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color  material_get_color       (IntPtr material, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec4   material_get_vector4     (IntPtr material, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3   material_get_vector3     (IntPtr material, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec2   material_get_vector2     (IntPtr material, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    material_get_int         (IntPtr material, string name);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   material_get_bool        (IntPtr material, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern uint   material_get_uint        (IntPtr material, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Matrix material_get_matrix      (IntPtr material, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr material_get_texture     (IntPtr material, string name);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   material_set_texture_id  (IntPtr material, ulong  id,   IntPtr value);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   material_has_param       (IntPtr material, string name, MaterialParam type);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_param       (IntPtr material, string name, MaterialParam type, IntPtr value);
		//[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_param_id    (IntPtr material, ulong    id,   MaterialParam type, const void *value);
		//[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    material_get_param       (IntPtr material, string name, MaterialParam type, void *out_value);
		//[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    material_get_param_id    (IntPtr material, ulong    id, MaterialParam type, void *out_value);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_get_param_info  (IntPtr material, int index, out IntPtr out_name, out MaterialParam out_type);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    material_get_param_count (IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_set_shader      (IntPtr material, IntPtr shader);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr material_get_shader      (IntPtr material);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr material_buffer_create  (int slot, int size);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_buffer_set_data(IntPtr buffer, IntPtr data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   material_buffer_release (IntPtr buffer);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern TextStyle text_make_style                (IntPtr font, float character_height, Color color);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern TextStyle text_make_style_shader         (IntPtr font, float character_height, IntPtr shader, Color color);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern TextStyle text_make_style_mat            (IntPtr font, float character_height, IntPtr material, Color color);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void      text_add_at_16                 (string text, in Matrix transform, TextStyle style, TextAlign position, TextAlign align, float off_x, float off_y, float off_z, Color vertex_tint_linear);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern float     text_add_in_16                 (string text, in Matrix transform, Vec2 size, TextFit fit, TextStyle style, TextAlign position, TextAlign align, float off_x, float off_y, float off_z, Color vertex_tint_linear);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern Vec2      text_size_layout_16            (string text, TextStyle style);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern Vec2      text_size_layout_constrained_16(string text, TextStyle style, float max_width);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern Vec2      text_size_render               (Vec2 layout_size, TextStyle style, out float out_y_offset);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float     text_style_get_line_height_pct(TextStyle style);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      text_style_set_line_height_pct(TextStyle style, float height_percent);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float     text_style_get_layout_height  (TextStyle style);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      text_style_set_layout_height  (TextStyle style, float height_meters);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float     text_style_get_total_height   (TextStyle style);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      text_style_set_total_height   (TextStyle style, float height_meters);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr    text_style_get_material       (TextStyle style);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float     text_style_get_ascender       (TextStyle style);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float     text_style_get_descender      (TextStyle style);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float     text_style_get_cap_height     (TextStyle style);


		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_find              (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_copy              (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_create            ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_create_mesh       (IntPtr mesh, IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_create_mem        ([In] byte[] filename_utf8, [In] byte[] data, UIntPtr data_size, IntPtr shader);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_create_file       ([In] byte[] filename_utf8, IntPtr shader);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_set_id            (IntPtr model, string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_get_id            (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_addref            (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_release           (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_recalculate_bounds(IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_recalculate_bounds_exact(IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_set_bounds        (IntPtr model, in Bounds bounds);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds model_get_bounds        (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   model_ray_intersect     (IntPtr model, Ray model_space_ray, Cull cull_mode, out Ray out_pt);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     model_step_anim             (IntPtr model);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool     model_play_anim             (IntPtr model, string animation_name, AnimMode mode);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     model_play_anim_idx         (IntPtr model, int index,             AnimMode mode);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     model_set_anim_time         (IntPtr model, float time);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     model_set_anim_completion   (IntPtr model, float percent);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int      model_anim_find             (IntPtr model, string animation_name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int      model_anim_count            (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int      model_anim_active           (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern AnimMode model_anim_active_mode      (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float    model_anim_active_time      (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float    model_anim_active_completion(IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr   model_anim_get_name         (IntPtr model, int index);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float    model_anim_get_duration     (IntPtr model, int index);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_add                (IntPtr model,             string name, Matrix model_transform, IntPtr mesh, IntPtr material, int solid);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_add_child          (IntPtr model, int parent, string name, Matrix local_transform, IntPtr mesh, IntPtr material, int solid);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_find               (IntPtr model, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_sibling            (IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_parent             (IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_child              (IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_count              (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_index              (IntPtr model, int index);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_visual_count       (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_visual_index       (IntPtr model, int index);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_iterate            (IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_get_root           (IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_node_get_name           (IntPtr model, int node);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   model_node_get_solid          (IntPtr model, int node);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   model_node_get_visible        (IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_node_get_material       (IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_node_get_mesh           (IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Matrix model_node_get_transform_model(IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Matrix model_node_get_transform_local(IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_node_set_name           (IntPtr model, int node, string name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_node_set_solid          (IntPtr model, int node, [MarshalAs(UnmanagedType.Bool)] bool solid);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_node_set_visible        (IntPtr model, int node, [MarshalAs(UnmanagedType.Bool)] bool visible);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_node_set_material       (IntPtr model, int node, IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_node_set_mesh           (IntPtr model, int node, IntPtr mesh);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_node_set_transform_model(IntPtr model, int node, Matrix transform_model_space);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_node_set_transform_local(IntPtr model, int node, Matrix transform_local_space);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr model_node_info_get           (IntPtr model, int node, [In] byte[] info_key_u8);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_node_info_set           (IntPtr model, int node, [In] byte[] info_key_u8, [In] byte[] info_value_u8);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   model_node_info_remove        (IntPtr model, int node, [In] byte[] info_key_u8);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   model_node_info_clear         (IntPtr model, int node);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    model_node_info_count         (IntPtr model, int node);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   model_node_info_iterate       (IntPtr model, int node, ref int ref_iterator, out IntPtr out_key_utf8, out IntPtr out_value_utf8);

		 ///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr sprite_find       (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr sprite_create     (IntPtr sprite,   SpriteType type = SpriteType.Atlased, string atlas_id = "default");
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr sprite_create_file([In] byte[] filename_utf8, SpriteType type = SpriteType.Atlased, string atlas_id = "default");
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   sprite_set_id     (IntPtr sprite, string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr sprite_get_id     (IntPtr sprite);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   sprite_release    (IntPtr sprite);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float  sprite_get_aspect (IntPtr sprite);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   sprite_draw       (IntPtr sprite, Matrix transform, TextAlign position, Color32 color);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    sprite_get_width  (IntPtr sprite);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    sprite_get_height (IntPtr sprite);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec2   sprite_get_dimensions_normalized(IntPtr sprite);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void line_add(Vec3 start, Vec3 end, Color32 color_start, Color32 color_end, float thickness);
		//[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void line_addv(line_point_t start, line_point_t end);
		//[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void line_add_list(const Vec3* points, int count, Color32 color, float thickness);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void line_add_listv([In] LinePoint[] points, int count);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_clip       (float near_plane, float far_plane);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_fov        (float field_of_view_degrees);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_ortho_clip (float near_plane, float far_plane);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_ortho_size (float viewport_height_meters);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_projection (Projection proj);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Projection         render_get_projection ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Matrix             render_get_cam_root   ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_cam_root   (in Matrix cam_root);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_skytex     (IntPtr sky_texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr             render_get_skytex     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_skymaterial(IntPtr sky_material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr             render_get_skymaterial();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_skylight   (in SphericalHarmonics lighting_info);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern RenderLayer        render_get_filter     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_filter     (RenderLayer layer_filter);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_scaling    (float display_tex_scale);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float              render_get_scaling    ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_viewport_scaling(float viewport_rect_scale);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float              render_get_viewport_scaling();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_multisample(int display_tex_multisample);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int                render_get_multisample();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_override_capture_filter([MarshalAs(UnmanagedType.Bool)] bool use_override_filter, RenderLayer layer_filter);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern RenderLayer        render_get_capture_filter     ();
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool               render_has_capture_filter     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern SphericalHarmonics render_get_skylight   ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_set_clear_color(Color color);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color              render_get_clear_color();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_enable_skytex  ([MarshalAs(UnmanagedType.Bool)] bool show_sky);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool               render_enabled_skytex ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_global_texture (int register_slot, IntPtr texture);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_add_mesh       (IntPtr mesh, IntPtr material, in Matrix transform, Color color, RenderLayer layer);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_add_model      (IntPtr model, in Matrix transform, Color color, RenderLayer layer);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_add_model_mat  (IntPtr model, IntPtr material_override, in Matrix transform, Color color, RenderLayer layer);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_blit           (IntPtr to_rendertarget, IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_screenshot     ([In] byte[] file_utf8, int file_quality_100, Pose viewpoint, int width, int height, float field_of_view_degrees);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_screenshot_capture  ([MarshalAs(UnmanagedType.FunctionPtr)] RenderOnScreenshotCallback render_on_screenshot_callback, Pose viewpoint, int width, int height, float fov_degrees, TexFormat tex_format, IntPtr context);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_screenshot_viewpoint([MarshalAs(UnmanagedType.FunctionPtr)] RenderOnScreenshotCallback render_on_screenshot_callback, Matrix camera, Matrix projection, int width, int height, RenderLayer layer_filter, RenderClear clear, Rect viewport, TexFormat tex_format, IntPtr context);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_to             (IntPtr to_rendertarget, in Matrix camera, in Matrix projection, RenderLayer layer_filter, RenderClear clear, Rect viewport);
		//[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void render_get_device  (void **device, void **context);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr             render_get_primary_list();

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr             render_list_create       ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_list_addref       (IntPtr list);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_list_release      (IntPtr list);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_list_clear        (IntPtr list);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int                render_list_item_count   (IntPtr list);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int                render_list_prev_count   (IntPtr list);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_list_add_mesh     (IntPtr list, IntPtr mesh, IntPtr material,           Matrix transform, Color color_linear, RenderLayer layer);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_list_add_model    (IntPtr list, IntPtr model,                           Matrix transform, Color color_linear, RenderLayer layer);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_list_add_model_mat(IntPtr list, IntPtr model, IntPtr material_override, Matrix transform, Color color_linear, RenderLayer layer);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_list_draw_now     (IntPtr list, IntPtr to_rendertarget, Matrix camera, Matrix projection, Rect viewport_px, RenderLayer layer_filter, RenderClear clear);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_list_push         (IntPtr list);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void               render_list_pop          ();

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void hierarchy_push(in Matrix transform, HierarchyParent parentBehavior);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void hierarchy_pop();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void hierarchy_set_enabled([MarshalAs(UnmanagedType.Bool)] bool enabled);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool hierarchy_is_enabled();
		//[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ref Matrix hierarchy_to_world();
		//[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ref Matrix hierarchy_to_local();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3 hierarchy_to_local_point    (in Vec3 world_pt);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3 hierarchy_to_local_direction(in Vec3 world_dir);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Quat hierarchy_to_local_rotation (in Quat world_orientation);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pose hierarchy_to_local_pose     (in Pose world_pose);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Ray  hierarchy_to_local_ray      (   Ray  world_ray);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3 hierarchy_to_world_point    (in Vec3 local_pt);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3 hierarchy_to_world_direction(in Vec3 local_dir);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Quat hierarchy_to_world_rotation (in Quat local_orientation);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pose hierarchy_to_world_pose     (in Pose local_pose);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Ray  hierarchy_to_world_ray      (   Ray  local_ray);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr    sound_find          (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      sound_set_id        (IntPtr sound, string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr    sound_get_id        (IntPtr sound);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr    sound_create        ([In] byte[] filename_utf8);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr    sound_create_stream (float buffer_duration);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr    sound_create_samples([In] float[] samples_at_48000s, ulong sample_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr    sound_generate      ([MarshalAs(UnmanagedType.FunctionPtr)] AudioGenerator function, float duration);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      sound_write_samples (IntPtr sound, [In ] float[] samples,     ulong sample_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      sound_write_samples (IntPtr sound, IntPtr        samples,     ulong sample_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong     sound_read_samples  (IntPtr sound, [Out] float[] out_samples, ulong sample_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong     sound_read_samples  (IntPtr sound, IntPtr        out_samples, ulong sample_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong     sound_unread_samples(IntPtr sound);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong     sound_total_samples (IntPtr sound);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong     sound_cursor_samples(IntPtr sound);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float     sound_get_decibels  (IntPtr sound);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      sound_set_decibels  (IntPtr sound, float decibels);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern SoundInst sound_play          (IntPtr sound, Vec3 at, float volume);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float     sound_duration      (IntPtr sound);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      sound_release       (IntPtr sound);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  sound_inst_stop         (SoundInst sound_inst);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool  sound_inst_is_playing   (SoundInst sound_inst);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  sound_inst_set_pos      (SoundInst sound_inst, Vec3 pos);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3  sound_inst_get_pos      (SoundInst sound_inst);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  sound_inst_set_volume   (SoundInst sound_inst, float volume);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float sound_inst_get_volume   (SoundInst sound_inst);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float sound_inst_get_intensity(SoundInst sound_inst);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int    mic_device_count();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mic_device_name (int index);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   mic_start       (string device_name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   mic_stop        ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr mic_get_stream  ();
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   mic_is_recording();

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void platform_file_picker_sz     (PickerMode mode, IntPtr callback_data, [MarshalAs(UnmanagedType.FunctionPtr)] PickerCallback on_callback, [In] FileFilter[] filters, int filter_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void platform_file_picker_close  ();
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool platform_file_picker_visible();
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool platform_read_file          ([In] byte[] name_utf8, out IntPtr out_data, out UIntPtr out_size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool platform_write_file         ([In] byte[] name_utf8, [In] byte[] data, UIntPtr size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool platform_write_file_text    ([In] byte[] name_utf8, [In] byte[] text_utf8);

		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool platform_keyboard_get_force_fallback();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void platform_keyboard_set_force_fallback([MarshalAs(UnmanagedType.Bool)] bool force_fallback);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool platform_keyboard_visible           ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void platform_keyboard_show              ([MarshalAs(UnmanagedType.Bool)] bool show, TextContext inputType);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool platform_keyboard_set_layout(TextContext keyboard_type, string[] keyboard_layout, int layouts_num);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int        input_pointer_count  (InputSource filter = InputSource.Any);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pointer    input_pointer        (int index, InputSource filter = InputSource.Any);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr     input_hand           (Handed hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern HandSource input_hand_source    (Handed hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr     input_controller     (Handed hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState   input_controller_menu();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void       input_controller_model_set(Handed hand, IntPtr model);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr     input_controller_model_get(Handed hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void       input_hand_override  (Handed hand, [In] HandJoint[] hand_joints);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void       input_hand_override  (Handed hand, IntPtr hand_joints);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr     input_mouse          ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr     input_head           ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr     input_eyes           ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState   input_eyes_tracked   ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState   input_key            (Key key);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern HandSimId input_hand_sim_pose_add   ([In] Pose[] in_arr_hand_joints_25, ControllerKey button1, ControllerKey and_button2, Key or_hotkey1, Key and_hotkey2);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      input_hand_sim_pose_remove(HandSimId id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      input_hand_sim_pose_clear ();

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     input_key_inject_press  (Key key);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     input_key_inject_release(Key key);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern uint     input_text_consume ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     input_text_reset   ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     input_text_inject_char(uint character);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     input_hand_visible (Handed hand, [MarshalAs(UnmanagedType.Bool)] bool visible);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void     input_hand_material(Handed hand, IntPtr material);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              anchor_find         ([In] byte[] asset_id_utf8);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              anchor_create       (Pose pose);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                anchor_set_id       (IntPtr anchor, [In] byte[] asset_id_utf8);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              anchor_get_id       (IntPtr anchor);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                anchor_addref       (IntPtr anchor);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                anchor_release      (IntPtr anchor);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CallingConvention = call                 )] public static extern bool                anchor_try_set_persistent(IntPtr anchor, [MarshalAs(UnmanagedType.Bool)] bool persistent);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CallingConvention = call                 )] public static extern bool                anchor_get_persistent(IntPtr anchor);
		[DllImport(dll, CallingConvention = call                 )] public static extern Pose                anchor_get_pose     (IntPtr anchor);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CallingConvention = call                 )] public static extern bool                anchor_get_changed  (IntPtr anchor);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              anchor_get_name     (IntPtr anchor);
		[DllImport(dll, CallingConvention = call                 )] public static extern BtnState            anchor_get_tracked  (IntPtr anchor);

		[DllImport(dll, CallingConvention = call                 )] public static extern void                anchor_clear_stored ();
		[DllImport(dll, CallingConvention = call                 )] public static extern AnchorCaps          anchor_get_capabilities();
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 anchor_get_count    ();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              anchor_get_index    (int index);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 anchor_get_new_count();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              anchor_get_new_index(int index);
		[DllImport(dll, CallingConvention = call                 )] public static extern bool                anchor_get_perception_anchor(IntPtr anchor, out IntPtr perception_spatial_anchor);

		///////////////////////////////////////////

		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool         world_has_bounds                ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec2         world_get_bounds_size           ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pose         world_get_bounds_pose           ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pose         world_from_spatial_graph        ([MarshalAs(UnmanagedType.LPArray, SizeConst = 16)] byte[] spatial_graph_node_id, [MarshalAs(UnmanagedType.Bool)] bool dynamic, long time);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pose         world_from_perception_anchor    (IntPtr perception_spatial_anchor);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool         world_try_from_spatial_graph    ([MarshalAs(UnmanagedType.LPArray, SizeConst = 16)] byte[] spatial_graph_node_id, [MarshalAs(UnmanagedType.Bool)] bool dynamic, long time, out Pose out_pose);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool         world_try_from_perception_anchor(IntPtr perception_spatial_anchor, out Pose out_pose);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool         world_raycast                   (Ray ray, out Ray out_intersection);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void         world_set_occlusion_enabled     ([MarshalAs(UnmanagedType.Bool)] bool enabled);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool         world_get_occlusion_enabled     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void         world_set_raycast_enabled       ([MarshalAs(UnmanagedType.Bool)] bool enabled);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool         world_get_raycast_enabled       ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void         world_set_occlusion_material    (IntPtr material);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr       world_get_occlusion_material    ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void         world_set_refresh_type          (WorldRefresh refresh_type);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern WorldRefresh world_get_refresh_type          ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void         world_set_refresh_radius        (float radius_meters);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float        world_get_refresh_radius        ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void         world_set_refresh_interval      (float every_seconds);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float        world_get_refresh_interval      ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern OriginMode   world_get_origin_mode           ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pose         world_get_origin_offset         ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void         world_set_origin_offset         (Pose offset);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState     world_get_tracked               ();

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void input_subscribe  (InputSource source, BtnState evt, InputEventCallback event_callback);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void input_unsubscribe(InputSource source, BtnState evt, InputEventCallback event_callback);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void input_fire_event (InputSource source, BtnState evt, IntPtr pointer);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BackendXRType backend_xr_get_type         ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong         backend_openxr_get_instance ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong         backend_openxr_get_session  ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong         backend_openxr_get_system_id();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern ulong         backend_openxr_get_space    ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern long          backend_openxr_get_time     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern long          backend_openxr_get_eyes_sample_time();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr        backend_openxr_get_function(string function_name);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool          backend_openxr_ext_enabled (string extension_name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void          backend_openxr_ext_request (string extension_name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void          backend_openxr_ext_exclude (string extension_name);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void          backend_openxr_use_minimum_exts([MarshalAs(UnmanagedType.Bool)] bool use_minimum_exts);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void          backend_openxr_composition_layer(IntPtr XrCompositionLayerBaseHeader, int data_size, int sort_order);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void          backend_openxr_end_frame_chain  (IntPtr XrBaseHeader, int data_size);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void          backend_openxr_add_callback_pre_session_create([MarshalAs(UnmanagedType.FunctionPtr)] XRPreSessionCreateCallback on_pre_session_create, IntPtr context);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void          backend_openxr_add_callback_poll_event        ([MarshalAs(UnmanagedType.FunctionPtr)] XRPollEventCallback on_poll_event, IntPtr context);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void          backend_openxr_remove_callback_poll_event     ([MarshalAs(UnmanagedType.FunctionPtr)] XRPollEventCallback on_poll_event);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void          backend_openxr_set_hand_joint_scale           (float joint_scale_factor);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BackendPlatform backend_platform_get        ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_android_get_java_vm ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_android_get_activity();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_android_get_jni_env ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BackendGraphics backend_graphics_get           ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_d3d11_get_d3d_device   ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_d3d11_get_d3d_context  ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_opengl_wgl_get_hdc     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_opengl_wgl_get_hglrc   ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_opengl_glx_get_context ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_opengl_glx_get_display ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_opengl_glx_get_drawable();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_opengl_egl_get_context ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr          backend_opengl_egl_get_display ();

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void log_write      (LogLevel level, string text);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void log_set_filter (LogLevel level);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void log_subscribe  (LogCallbackData on_log, IntPtr context);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void log_unsubscribe(LogCallbackData on_log, IntPtr context);
		
		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      assets_releaseref_threadsafe(IntPtr asset);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int       assets_current_task         ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int       assets_total_tasks          ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int       assets_current_task_priority();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      assets_block_for_priority   (int priority);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern int       assets_count                ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr    assets_get_index            (int index);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern AssetType assets_get_type             (int index);
		
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern AssetType asset_get_type              (IntPtr asset);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      asset_set_id                (IntPtr asset, string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr    asset_get_id                (IntPtr asset);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      asset_addref                (IntPtr asset);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void      asset_release               (IntPtr asset);

		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_quadrant_size_verts  ([In, Out] Vertex[] ref_vertices, int vertex_count, float overflow);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_quadrant_size_mesh   (IntPtr ref_mesh, float overflow);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern IntPtr ui_gen_quadrant_mesh    (UICorner rounded_corners, float corner_radius, uint corner_resolution, [MarshalAs(UnmanagedType.Bool)] bool delete_flat_sides, [MarshalAs(UnmanagedType.Bool)] bool quadrantify, [In] UILathePt[] lathe_pts, int lathe_pt_count);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_show_volumes         ([MarshalAs(UnmanagedType.Bool)] bool show);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_enable_far_interact  ([MarshalAs(UnmanagedType.Bool)] bool enable);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool   ui_far_interact_enabled ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern UIMove ui_system_get_move_type ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_system_set_move_type (UIMove move_type);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_settings             (UISettings settings);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern UISettings ui_get_settings     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_set_color            (Color color);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_set_theme_color      (UIColor color_type, Color color_gamma);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color  ui_get_theme_color      (UIColor color_type);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_set_theme_color_state(UIColor color_type, UIColorState state, Color color_gamma);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color  ui_get_theme_color_state(UIColor color_type, UIColorState state);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_set_element_visual   (UIVisual element_visual, IntPtr mesh, IntPtr material, Vec2 min_size);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_set_element_color    (UIVisual element_visual, UIColor color_type);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_set_element_sound    (UIVisual element_visual, IntPtr activate, IntPtr deactivate);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Pose   ui_popup_pose           (Vec3 shift);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_draw_element         (UIVisual element_visual,                         Vec3 start, Vec3 size, float focus);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_draw_element_color   (UIVisual element_visual, UIVisual element_color, Vec3 start, Vec3 size, float focus);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color  ui_get_element_color    (UIVisual element_visual,                                                float focus);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float  ui_get_anim_focus       (IdHash id, BtnState focus_state, BtnState activation_state);


		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_grab_aura        ([MarshalAs(UnmanagedType.Bool)] bool enabled);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_grab_aura         ();
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool ui_grab_aura_enabled     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_text_style       (TextStyle style);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_text_style        ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern TextStyle ui_get_text_style   ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_tint             (Color tint_gamma);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_tint              ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_enabled          ([MarshalAs(UnmanagedType.Bool)] bool enabled, HierarchyParent parentBehavior);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_enabled           ();
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool ui_is_enabled            ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_preserve_keyboard([MarshalAs(UnmanagedType.Bool)] bool preserve_keyboard);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_preserve_keyboard ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_surface          (Pose surface_pose, Vec3 layout_start, Vec2 layout_dimensions);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_surface           ();
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern IdHash ui_push_id_16   (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]            public static extern IdHash ui_push_idi     (int id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]            public static extern void   ui_pop_id       ();
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern IdHash ui_stack_hash_16(string str);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_layout_area     (Vec3 start, Vec2 dimensions, [MarshalAs(UnmanagedType.Bool)] bool add_margin);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec2   ui_layout_remaining();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3   ui_layout_at       ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds ui_layout_last     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds ui_layout_reserve  (Vec2 size, [MarshalAs(UnmanagedType.Bool)] bool add_padding, float depth);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_layout_push     (Vec3 start, Vec2 dimensions, [MarshalAs(UnmanagedType.Bool)] bool add_margin);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_layout_push_cut (UICut cut_to, float size, [MarshalAs(UnmanagedType.Bool)] bool add_margin);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_layout_pop      ();

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState ui_last_element_hand_active (Handed hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState ui_last_element_hand_focused(Handed hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState ui_last_element_active();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState ui_last_element_focused();

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_nextline      ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_sameline      ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float  ui_line_height   ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_vspace        (float vertical_space);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_hspace        (float horizontal_space);

		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool ui_is_interacting       (Handed hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_button_behavior      (Vec3 window_relative_pos, Vec2 size, IdHash id, out float finger_offset, out BtnState button_state, out BtnState focus_state, out int hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_button_behavior_depth(Vec3 window_relative_pos, Vec2 size, IdHash id, float button_depth, float button_activation_depth, out float finger_offset, out BtnState button_state, out BtnState focus_state, out int hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_slider_behavior      (Vec3 window_relative_pos, Vec2 size, IdHash id, ref Vec2 value, Vec2 min, Vec2 max, Vec2 button_size_visual, Vec2 button_size_interact, UIConfirm confirm_method, out UISliderData data);

		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern BtnState ui_volume_at_16      (string id, Bounds bounds, UIConfirm interact_type, IntPtr out_opt_hand, IntPtr out_opt_focus_state);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern BtnState ui_volume_at_16      (string id, Bounds bounds, UIConfirm interact_type, out Handed out_opt_hand, IntPtr out_opt_focus_state);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern BtnState ui_volume_at_16      (string id, Bounds bounds, UIConfirm interact_type, out Handed out_opt_hand, out BtnState out_opt_focus_state);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_button_at_16      (string text, Vec3 window_relative_pos, Vec2 size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_button_img_16     (string text, IntPtr image, UIBtnLayout image_layout, Vec3 window_relative_pos, Vec2 size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_button_round_at_16(string text, IntPtr image_sprite, Vec3 window_relative_pos, float diameter);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_toggle_at_16      (string text, [MarshalAs(UnmanagedType.Bool)] ref bool pressed, Vec3 window_relative_pos, Vec2 size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_toggle_img_at_16  (string text, [MarshalAs(UnmanagedType.Bool)] ref bool pressed, IntPtr toggle_off, IntPtr toggle_on, UIBtnLayout image_layout, Vec3 window_relative_pos, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void     ui_progress_bar_at   (float percent, Vec3 window_relative_pos, Vec2 size, UIDir bar_direction, [MarshalAs(UnmanagedType.Bool)] bool flip_fill_dir);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_hslider_at_16     (string id,   ref float  value, float  min, float  max, float  step, Vec3 window_relative_pos, Vec2 size, UIConfirm confirm_method, UINotify notify_on);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_hslider_at_f64_16 (string id,   ref double value, double min, double max, double step, Vec3 window_relative_pos, Vec2 size, UIConfirm confirm_method, UINotify notify_on);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_vslider_at_16     (string id,   ref float  value, float  min, float  max, float  step, Vec3 window_relative_pos, Vec2 size, UIConfirm confirm_method, UINotify notify_on);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_vslider_at_f64_16 (string id,   ref double value, double min, double max, double step, Vec3 window_relative_pos, Vec2 size, UIConfirm confirm_method, UINotify notify_on);

		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_hseparator      ();
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_label_16        (string text, [MarshalAs(UnmanagedType.Bool)] bool use_padding);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_label_sz_16     (string text, Vec2 size, [MarshalAs(UnmanagedType.Bool)] bool use_padding);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_text_16         (string text, ref Vec2 scroll, UIScroll scrollDirection, float height, TextAlign text_align, TextFit fit);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_text_16         (string text, IntPtr   scroll, UIScroll scrollDirection, float height, TextAlign text_align, TextFit fit);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_text_sz_16      (string text, ref Vec2 scroll, UIScroll scrollDirection, Vec2  size,   TextAlign text_align, TextFit fit);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_text_sz_16      (string text, IntPtr   scroll, UIScroll scrollDirection, Vec2  size,   TextAlign text_align, TextFit fit);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_text_at_16      (string text, ref Vec2 scroll, UIScroll scrollDirection, TextAlign text_align, TextFit fit, Vec3 window_relative_pos, Vec2 size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_text_at_16      (string text, IntPtr   scroll, UIScroll scrollDirection, TextAlign text_align, TextFit fit, Vec3 window_relative_pos, Vec2 size);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_image           (IntPtr sprite_image, Vec2 size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_16       (string text);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_sz_16    (string text, Vec2 size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_img_16   (string text, IntPtr image, UIBtnLayout image_layout, Color image_tint);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_img_sz_16(string text, IntPtr image, UIBtnLayout image_layout, Vec2 size, Color image_tint);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_img_at_16(string text, IntPtr image, UIBtnLayout image_layout, Vec3 window_relative_pos, Vec2 size, Color image_tint);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_round_16 (string id,   IntPtr image, float diameter);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_toggle_16       (string text, [MarshalAs(UnmanagedType.Bool)] ref bool pressed);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_toggle_sz_16    (string text, [MarshalAs(UnmanagedType.Bool)] ref bool pressed, Vec2 size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_toggle_img_16   (string text, [MarshalAs(UnmanagedType.Bool)] ref bool pressed, IntPtr toggle_off, IntPtr toggle_on, UIBtnLayout image_layout);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_toggle_img_sz_16(string text, [MarshalAs(UnmanagedType.Bool)] ref bool pressed, IntPtr toggle_off, IntPtr toggle_on, UIBtnLayout image_layout, Vec2 size);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_model           (IntPtr model, Vec2 ui_size, float model_scale);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_input_16        (string id,   StringBuilder buffer, int buffer_size, Vec2 size, TextContext type);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_input_at_16     (string id,   StringBuilder buffer, int buffer_size, Vec3 window_relative_pos, Vec2 size, TextContext type);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_hprogress_bar   (float percent, float width, [MarshalAs(UnmanagedType.Bool)] bool flip_fill_direction);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_vprogress_bar   (float percent, float width, [MarshalAs(UnmanagedType.Bool)] bool flip_fill_direction);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_hslider_16      (string id,   ref float  value, float  min, float  max, float  step, float width,  UIConfirm confirm_method, UINotify notify_on);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_hslider_f64_16  (string id,   ref double value, double min, double max, double step, float width,  UIConfirm confirm_method, UINotify notify_on);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_vslider_16      (string id,   ref float  value, float  min, float  max, float  step, float height, UIConfirm confirm_method, UINotify notify_on);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_vslider_f64_16  (string id,   ref double value, double min, double max, double step, float height, UIConfirm confirm_method, UINotify notify_on);
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_handle_begin_16 (string text, ref Pose movement, Bounds handle, [MarshalAs(UnmanagedType.Bool)] bool draw, UIMove move_type, UIGesture allowed_gestures);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_handle_end      ();
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_window_begin_16 (string text, ref Pose pose, Vec2 size, UIWin window_type, UIMove move_type);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_window_end      ();
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_panel_at        (Vec3 start, Vec2 size, UIPad padding);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_panel_begin     (UIPad padding);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_panel_end       ();
	}
}
