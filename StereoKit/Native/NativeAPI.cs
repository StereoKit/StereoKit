// This is a generated file based on stereokit.h! Please don't modify it
// directly :) Instead, modify the header file, and run the StereoKitAPIGen
// project.

using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	internal static partial class NativeAPI
	{
		const string            dll   = "StereoKitC";
		const CharSet           ascii = CharSet.Ansi;
		const CharSet           utf16 = CharSet.Unicode;
		const CallingConvention call  = CallingConvention.Cdecl;

		///////////////////////////////////////////

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate void AppUpdate(IntPtr updateData);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate void AppShutdown(IntPtr shutdownData);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate void AssetOnLoadCallback(IntPtr texture, IntPtr context);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate float AudioGenerator(float sampleTime);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate void PickerCallback(IntPtr callbackData, int confirmed, string filename);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate void PickerCallbackSz(IntPtr callbackData, int confirmed, IntPtr filenamePtr, int filenameLength);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate void InputEventCallback(InputSource source, BtnState inputEvent, in Pointer inPointer);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate void XrPreSessionCreateCallback(IntPtr context);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate void XrPollEventCallback(IntPtr context, IntPtr XrEventDataBuffer);
		[UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate void LogCallback(LogLevel level, string text);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern int                 sk_init             (SKSettings settings);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sk_set_window       (IntPtr window);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sk_set_window_xam   (IntPtr window);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sk_shutdown         ();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sk_quit             ();
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 sk_step             ([MarshalAs(UnmanagedType.FunctionPtr)] Action app_update);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sk_run              ([MarshalAs(UnmanagedType.FunctionPtr)] Action app_update, [MarshalAs(UnmanagedType.FunctionPtr)] Action app_shutdown);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sk_run_data         ([MarshalAs(UnmanagedType.FunctionPtr)] AppUpdate app_update, IntPtr update_data, [MarshalAs(UnmanagedType.FunctionPtr)] AppShutdown app_shutdown, IntPtr shutdown_data);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 sk_is_stepping      ();
		[DllImport(dll, CallingConvention = call                 )] public static extern DisplayMode         sk_active_display_mode();
		[DllImport(dll, CallingConvention = call                 )] public static extern SKSettings          sk_get_settings     ();
		[DllImport(dll, CallingConvention = call                 )] public static extern SystemInfo          sk_system_info      ();
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              sk_version_name     ();
		[DllImport(dll, CallingConvention = call                 )] public static extern ulong               sk_version_id       ();
		[DllImport(dll, CallingConvention = call                 )] public static extern AppFocus            sk_app_focus        ();

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_get_raw        ();
		[DllImport(dll, CallingConvention = call                 )] public static extern float               time_getf_unscaled  ();
		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_get_unscaled   ();
		[DllImport(dll, CallingConvention = call                 )] public static extern float               time_getf           ();
		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_get            ();
		[DllImport(dll, CallingConvention = call                 )] public static extern float               time_elapsedf_unscaled();
		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_elapsed_unscaled();
		[DllImport(dll, CallingConvention = call                 )] public static extern float               time_elapsedf       ();
		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_elapsed        ();
		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_total_raw      ();
		[DllImport(dll, CallingConvention = call                 )] public static extern float               time_totalf_unscaled();
		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_total_unscaled ();
		[DllImport(dll, CallingConvention = call                 )] public static extern float               time_totalf         ();
		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_total          ();
		[DllImport(dll, CallingConvention = call                 )] public static extern float               time_stepf_unscaled ();
		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_step_unscaled  ();
		[DllImport(dll, CallingConvention = call                 )] public static extern float               time_stepf          ();
		[DllImport(dll, CallingConvention = call                 )] public static extern double              time_step           ();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                time_scale          (double scale);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                time_set_time       (double total_seconds, double frame_elapsed_seconds);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                vec3_cross          (in Vec3 a, in Vec3 b);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                quat_difference     (in Quat a, in Quat b);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                quat_lookat         (in Vec3 from, in Vec3 at);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                quat_lookat_up      (in Vec3 from, in Vec3 at, in Vec3 up);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                quat_from_angles    (float pitch_x_deg, float yaw_y_deg, float roll_z_deg);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                quat_slerp          (in Quat a, in Quat b, float t);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                quat_normalize      (in Quat a);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                quat_inverse        (in Quat a);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                quat_mul            (in Quat a, in Quat b);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                quat_mul_vec        (in Quat a, in Vec3 b);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              pose_matrix         (in Pose pose, Vec3 scale);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                pose_matrix_out     (in Pose pose, out Matrix out_result, Vec3 scale);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern void                matrix_inverse      (in Matrix a, out Matrix out_matrix);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              matrix_invert       (in Matrix a);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                matrix_mul          (in Matrix a, in Matrix b, out Matrix out_matrix);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                matrix_mul_point    (in Matrix transform, in Vec3 point);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec4                matrix_mul_point4   (in Matrix transform, in Vec4 point);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                matrix_mul_direction(in Matrix transform, in Vec3 direction);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                matrix_mul_rotation (in Matrix transform, in Quat orientation);
		[DllImport(dll, CallingConvention = call                 )] public static extern Pose                matrix_mul_pose     (in Matrix transform, in Pose pose);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                matrix_transform_pt (Matrix transform, Vec3 point);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec4                matrix_transform_pt4(Matrix transform, Vec4 point);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                matrix_transform_dir(Matrix transform, Vec3 direction);
		[DllImport(dll, CallingConvention = call                 )] public static extern Ray                 matrix_transform_ray(Matrix transform, Ray ray);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                matrix_transform_quat(Matrix transform, Quat rotation);
		[DllImport(dll, CallingConvention = call                 )] public static extern Pose                matrix_transform_pose(Matrix transform, Pose pose);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              matrix_transpose    (Matrix transform);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                matrix_to_angles    (in Matrix transform);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              matrix_trs          (in Vec3 position, in Quat orientation, in Vec3 scale);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              matrix_t            (Vec3 position);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              matrix_r            (Quat orientation);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              matrix_s            (Vec3 scale);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              matrix_ts           (Vec3 position, Vec3 scale);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                matrix_trs_out      (out Matrix out_result, in Vec3 position, in Quat orientation, in Vec3 scale);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              matrix_perspective  (float fov_degrees, float aspect_ratio, float near_clip, float far_clip);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              matrix_orthographic (float width, float height, float near_clip, float far_clip);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 matrix_decompose    (in Matrix transform, out Vec3 out_position, out Vec3 out_scale, out Quat out_orientation);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                matrix_extract_translation(in Matrix transform);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                matrix_extract_scale(in Matrix transform);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                matrix_extract_rotation(in Matrix transform);
		[DllImport(dll, CallingConvention = call                 )] public static extern Pose                matrix_extract_pose (in Matrix transform);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern int                 ray_intersect_plane (Ray ray, Vec3 plane_pt, Vec3 plane_normal, out float out_t);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 ray_from_mouse      (Vec2 screen_pixel_pos, out Ray out_ray);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                ray_point_closest   (Ray ray, Vec3 pt);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern Plane               plane_from_points   (Vec3 p1, Vec3 p2, Vec3 p3);
		[DllImport(dll, CallingConvention = call                 )] public static extern Plane               plane_from_ray      (Ray ray);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 plane_ray_intersect (Plane plane, Ray ray, out Vec3 out_pt);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 plane_line_intersect(Plane plane, Vec3 p1, Vec3 p2, out Vec3 out_pt);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                plane_point_closest (Plane plane, Vec3 pt);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern int                 sphere_ray_intersect(Sphere sphere, Ray ray, out Vec3 out_pt);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 sphere_point_contains(Sphere sphere, Vec3 pt);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern int                 bounds_ray_intersect(Bounds bounds, Ray ray, out Vec3 out_pt);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 bounds_point_contains(Bounds bounds, Vec3 pt);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 bounds_line_contains(Bounds bounds, Vec3 pt1, Vec3 pt2);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 bounds_capsule_contains(Bounds bounds, Vec3 pt1, Vec3 pt2, float radius);
		[DllImport(dll, CallingConvention = call                 )] public static extern Bounds              bounds_grow_to_fit_pt(Bounds bounds, Vec3 pt);
		[DllImport(dll, CallingConvention = call                 )] public static extern Bounds              bounds_grow_to_fit_box(Bounds bounds, Bounds box, in Matrix opt_transform);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern Color               color_hsv           (float hue, float saturation, float value, float transparency);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                color_to_hsv        (in Color color);
		[DllImport(dll, CallingConvention = call                 )] public static extern Color               color_lab           (float l, float a, float b, float transparency);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                color_to_lab        (in Color color);
		[DllImport(dll, CallingConvention = call                 )] public static extern Color               color_to_linear     (Color srgb_gamma_correct);
		[DllImport(dll, CallingConvention = call                 )] public static extern Color               color_to_gamma      (Color srgb_linear);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              gradient_create     ();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              gradient_create_keys([In] GradientKey[] in_arr_keys, int count);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                gradient_add        (IntPtr gradient, Color color_linear, float position);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                gradient_set        (IntPtr gradient, int index, Color color_linear, float position);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                gradient_remove     (IntPtr gradient, int index);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 gradient_count      (IntPtr gradient);
		[DllImport(dll, CallingConvention = call                 )] public static extern Color               gradient_get        (IntPtr gradient, float at);
		[DllImport(dll, CallingConvention = call                 )] public static extern Color32             gradient_get32      (IntPtr gradient, float at);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                gradient_release    (IntPtr gradient);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern SphericalHarmonics  sh_create           ([In] SHLight[] in_arr_lights, int light_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sh_brightness       (ref SphericalHarmonics ref_harmonics, float scale);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sh_add              (ref SphericalHarmonics ref_harmonics, Vec3 light_dir, Vec3 light_color);
		[DllImport(dll, CallingConvention = call                 )] public static extern Color               sh_lookup           (in SphericalHarmonics harmonics, Vec3 normal);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                sh_dominant_dir     (in SphericalHarmonics harmonics);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              mesh_find           (string name);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mesh_create         ();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mesh_copy           (IntPtr mesh);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                mesh_set_id         (IntPtr mesh, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              mesh_get_id         (IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_addref         (IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_release        (IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_draw           (IntPtr mesh, IntPtr material, Matrix transform, Color color_linear, RenderLayer layer);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_set_keep_data  (IntPtr mesh, int keep_data);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 mesh_get_keep_data  (IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_set_data       (IntPtr mesh, [In] Vertex[] in_arr_vertices, int vertex_count, [In] uint[] in_arr_indices, int index_count, int calculate_bounds);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_set_verts      (IntPtr mesh, [In] Vertex[] in_arr_vertices, int vertex_count, int calculate_bounds);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_get_verts      (IntPtr mesh, [Out] Vertex[] out_arr_vertices, out int out_vertex_count, Memory reference_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 mesh_get_vert_count (IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_set_inds       (IntPtr mesh, [In] uint[] in_arr_indices, int index_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_get_inds       (IntPtr mesh, [Out] uint[] out_arr_indices, out int out_index_count, Memory reference_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 mesh_get_ind_count  (IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_set_draw_inds  (IntPtr mesh, int index_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_set_bounds     (IntPtr mesh, in Bounds bounds);
		[DllImport(dll, CallingConvention = call                 )] public static extern Bounds              mesh_get_bounds     (IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 mesh_has_skin       (IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_set_skin       (IntPtr mesh, [In] ushort[] in_arr_bone_ids_4, int bone_id_4_count, [In] Vec4[] in_arr_bone_weights, int bone_weight_count, in Matrix bone_resting_transforms, int bone_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mesh_update_skin    (IntPtr mesh, [In] Matrix[] in_arr_bone_transforms, int bone_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 mesh_ray_intersect  (IntPtr mesh, Ray model_space_ray, out Ray out_pt, out uint out_start_inds, Cull cull_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 mesh_ray_intersect_bvh(IntPtr mesh, Ray model_space_ray, out Ray out_pt, out uint out_start_inds, Cull cull_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 mesh_get_triangle   (IntPtr mesh, uint triangle_index, out Vertex out_a, out Vertex out_b, out Vertex out_c);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mesh_gen_plane      (Vec2 dimensions, Vec3 plane_normal, Vec3 plane_top_direction, int subdivisions, int double_sided);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mesh_gen_circle     (float diameter, Vec3 plane_normal, Vec3 plane_top_direction, int spokes, int double_sided);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mesh_gen_cube       (Vec3 dimensions, int subdivisions);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mesh_gen_sphere     (float diameter, int subdivisions);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mesh_gen_rounded_cube(Vec3 dimensions, float edge_radius, int subdivisions);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mesh_gen_cylinder   (float diameter, float depth, Vec3 direction, int subdivisions);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mesh_gen_cone       (float diameter, float depth, Vec3 direction, int subdivisions);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              tex_find            (string id);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_create          (TexType type, TexFormat format);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_create_color32  ([In] Color32[] in_arr_data, int width, int height, int srgb_data);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_create_color128 ([In] Color[] in_arr_data, int width, int height, int srgb_data);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_create_mem      (byte[] data, UIntPtr data_size, int srgb_data, int priority);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_create_file     ([In] byte[] file_utf8, int srgb_data, int priority);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              tex_create_file_arr ([In] string[] in_arr_files, int file_count, int srgb_data, int priority);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_create_cubemap_file([In] byte[] equirectangular_file_utf8, int srgb_data, out SphericalHarmonics out_sh_lighting_info, int priority);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              tex_create_cubemap_files([In] string[] in_arr_cube_face_file_xxyyzz, int srgb_data, out SphericalHarmonics out_sh_lighting_info, int priority);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                tex_set_id          (IntPtr texture, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              tex_get_id          (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_set_fallback    (IntPtr texture, IntPtr fallback);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_set_surface     (IntPtr texture, IntPtr native_surface, TexType type, long native_fmt, int width, int height, int surface_count, int owned);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_get_surface     (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_addref          (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_release         (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern AssetState          tex_asset_state     (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_on_load         (IntPtr texture, [MarshalAs(UnmanagedType.FunctionPtr)] AssetOnLoadCallback asset_on_load_callback, IntPtr context);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_on_load_remove  (IntPtr texture, [MarshalAs(UnmanagedType.FunctionPtr)] AssetOnLoadCallback asset_on_load_callback);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_set_colors      (IntPtr texture, int width, int height, byte[] data);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_set_color_arr   (IntPtr texture, int width, int height, byte[] data, int data_count, out SphericalHarmonics out_sh_lighting_info, int multisample);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_add_zbuffer     (IntPtr texture, TexFormat format);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_get_data        (IntPtr texture, out IntPtr out_data, out UIntPtr out_data_size);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_get_data_mip    (IntPtr texture, out IntPtr out_data, out UIntPtr out_data_size, int mip_level);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_gen_color       (Color color, int width, int height, TexType type, TexFormat format);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_gen_particle    (int width, int height, float roundness, IntPtr gradient_linear);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_gen_cubemap     (IntPtr gradient, Vec3 gradient_dir, int resolution, out SphericalHarmonics out_sh_lighting_info);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              tex_gen_cubemap_sh  (in SphericalHarmonics lookup, int face_size, float light_spot_size_pct, float light_spot_intensity);
		[DllImport(dll, CallingConvention = call                 )] public static extern TexFormat           tex_get_format      (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 tex_get_width       (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 tex_get_height      (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_set_sample      (IntPtr texture, TexSample sample);
		[DllImport(dll, CallingConvention = call                 )] public static extern TexSample           tex_get_sample      (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_set_address     (IntPtr texture, TexAddress address_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern TexAddress          tex_get_address     (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_set_anisotropy  (IntPtr texture, int anisotropy_level);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 tex_get_anisotropy  (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 tex_get_mips        (IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_set_loading_fallback(IntPtr loading_texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                tex_set_error_fallback(IntPtr error_texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern SphericalHarmonics  tex_get_cubemap_lighting(IntPtr cubemap_texture);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              font_find           (string id);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              font_create         ([In] byte[] file_utf8);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              font_create_files   ([In] string[] in_arr_files, int file_count);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                font_set_id         (IntPtr font, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              font_get_id         (IntPtr font);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                font_addref         (IntPtr font);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                font_release        (IntPtr font);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              font_get_tex        (IntPtr font);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              shader_find         (string id);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              shader_create_file  ([In] byte[] filename_utf8);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              shader_create_mem   (byte[] data, UIntPtr data_size);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                shader_set_id       (IntPtr shader, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              shader_get_id       (IntPtr shader);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              shader_get_name     (IntPtr shader);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                shader_addref       (IntPtr shader);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                shader_release      (IntPtr shader);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              material_find       (string id);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              material_create     (IntPtr shader);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              material_copy       (IntPtr material);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              material_copy_id    (string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_id     (IntPtr material, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              material_get_id     (IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_addref     (IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_release    (IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_set_transparency(IntPtr material, Transparency mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_set_cull   (IntPtr material, Cull mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_set_wireframe(IntPtr material, int wireframe);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_set_depth_test(IntPtr material, DepthTest depth_test_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_set_depth_write(IntPtr material, int write_enabled);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_set_queue_offset(IntPtr material, int offset);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_set_chain  (IntPtr material, IntPtr chain_material);
		[DllImport(dll, CallingConvention = call                 )] public static extern Transparency        material_get_transparency(IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern Cull                material_get_cull   (IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 material_get_wireframe(IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern DepthTest           material_get_depth_test(IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 material_get_depth_write(IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 material_get_queue_offset(IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              material_get_chain  (IntPtr material);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_float  (IntPtr material, string name, float value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_vector2(IntPtr material, string name, Vec2 value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_vector3(IntPtr material, string name, Vec3 value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_color  (IntPtr material, string name, Color color_gamma);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_vector4(IntPtr material, string name, Vec4 value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_vector (IntPtr material, string name, Vec4 value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_int    (IntPtr material, string name, int value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_int2   (IntPtr material, string name, int value1, int value2);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_int3   (IntPtr material, string name, int value1, int value2, int value3);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_int4   (IntPtr material, string name, int value1, int value2, int value3, int value4);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_bool   (IntPtr material, string name, int value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_uint   (IntPtr material, string name, uint value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_uint2  (IntPtr material, string name, uint value1, uint value2);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_uint3  (IntPtr material, string name, uint value1, uint value2, uint value3);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_uint4  (IntPtr material, string name, uint value1, uint value2, uint value3, uint value4);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_matrix (IntPtr material, string name, Matrix value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 material_set_texture(IntPtr material, string name, IntPtr value);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 material_set_texture_id(IntPtr material, ulong id, IntPtr value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern float               material_get_float  (IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern Vec2                material_get_vector2(IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern Vec3                material_get_vector3(IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern Color               material_get_color  (IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern Vec4                material_get_vector4(IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 material_get_int    (IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 material_get_bool   (IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern uint                material_get_uint   (IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern Matrix              material_get_matrix (IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              material_get_texture(IntPtr material, string name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 material_has_param  (IntPtr material, string name, MaterialParam type);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_set_param  (IntPtr material, string name, MaterialParam type, in IntPtr value);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_set_param_id(IntPtr material, ulong id, MaterialParam type, in IntPtr value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 material_get_param  (IntPtr material, string name, MaterialParam type, out IntPtr out_value);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 material_get_param_id(IntPtr material, ulong id, MaterialParam type, out IntPtr out_value);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                material_get_param_info(IntPtr material, int index, out IntPtr out_name, out MaterialParam out_type);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 material_get_param_count(IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_set_shader (IntPtr material, IntPtr shader);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              material_get_shader (IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              material_buffer_create(int register_slot, int size);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_buffer_set_data(IntPtr buffer, in IntPtr buffer_data);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                material_buffer_release(IntPtr buffer);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern TextStyle           text_make_style     (IntPtr font, float character_height, Color color_gamma);
		[DllImport(dll, CallingConvention = call                 )] public static extern TextStyle           text_make_style_shader(IntPtr font, float character_height, IntPtr shader, Color color_gamma);
		[DllImport(dll, CallingConvention = call                 )] public static extern TextStyle           text_make_style_mat (IntPtr font, float character_height, IntPtr material, Color color_gamma);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                text_add_at         ([In] byte[] text_utf8, in Matrix transform, TextStyle style, TextAlign position, TextAlign align, float off_x, float off_y, float off_z, Color vertex_tint_linear);
		[DllImport(dll, CallingConvention = call, CharSet = utf16)] public static extern void                text_add_at_16      (string text_utf16, in Matrix transform, TextStyle style, TextAlign position, TextAlign align, float off_x, float off_y, float off_z, Color vertex_tint_linear);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               text_add_in         ([In] byte[] text_utf8, in Matrix transform, Vec2 size, TextFit fit, TextStyle style, TextAlign position, TextAlign align, float off_x, float off_y, float off_z, Color vertex_tint_linear);
		[DllImport(dll, CallingConvention = call, CharSet = utf16)] public static extern float               text_add_in_16      (string text_utf16, in Matrix transform, Vec2 size, TextFit fit, TextStyle style, TextAlign position, TextAlign align, float off_x, float off_y, float off_z, Color vertex_tint_linear);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec2                text_size           ([In] byte[] text_utf8, TextStyle style);
		[DllImport(dll, CallingConvention = call, CharSet = utf16)] public static extern Vec2                text_size_16        (string text_utf16, TextStyle style);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec2                text_char_at        ([In] byte[] text_utf8, TextStyle style, int char_index, Vec2 opt_size, TextFit fit, TextAlign position, TextAlign align);
		[DllImport(dll, CallingConvention = call, CharSet = utf16)] public static extern Vec2                text_char_at_16     (string text_utf16, TextStyle style, int char_index, Vec2 opt_size, TextFit fit, TextAlign position, TextAlign align);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              text_style_get_material(TextStyle style);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               text_style_get_char_height(TextStyle style);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              solid_create        (in Vec3 position, in Quat rotation, SolidType type);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_release       (IntPtr solid);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                solid_set_id        (IntPtr solid, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              solid_get_id        (IntPtr solid);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_add_sphere    (IntPtr solid, float diameter, float kilograms, in Vec3 offset);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_add_box       (IntPtr solid, in Vec3 dimensions, float kilograms, in Vec3 offset);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_add_capsule   (IntPtr solid, float diameter, float height, float kilograms, in Vec3 offset);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_set_type      (IntPtr solid, SolidType type);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_set_enabled   (IntPtr solid, int enabled);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_move          (IntPtr solid, in Vec3 position, in Quat rotation);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_teleport      (IntPtr solid, in Vec3 position, in Quat rotation);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_set_velocity  (IntPtr solid, in Vec3 meters_per_second);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_set_velocity_ang(IntPtr solid, in Vec3 radians_per_second);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                solid_get_pose      (IntPtr solid, out Pose out_pose);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              model_find          (string id);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_copy          (IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_create        ();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_create_mesh   (IntPtr mesh, IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_create_mem    ([In] byte[] filename_utf8, byte[] data, UIntPtr data_size, IntPtr shader);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_create_file   ([In] byte[] filename_utf8, IntPtr shader);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                model_set_id        (IntPtr model, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              model_get_id        (IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_addref        (IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_release       (IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_draw          (IntPtr model, Matrix transform, Color color_linear, RenderLayer layer);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_recalculate_bounds(IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_set_bounds    (IntPtr model, in Bounds bounds);
		[DllImport(dll, CallingConvention = call                 )] public static extern Bounds              model_get_bounds    (IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_ray_intersect (IntPtr model, Ray model_space_ray, out Ray out_pt, Cull cull_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_ray_intersect_bvh(IntPtr model, Ray model_space_ray, out Ray out_pt, Cull cull_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_ray_intersect_bvh_detailed(IntPtr model, Ray model_space_ray, out Ray out_pt, IntPtr out_mesh, out Matrix out_matrix, out uint out_start_inds, Cull cull_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_step_anim     (IntPtr model);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 model_play_anim     (IntPtr model, string animation_name, AnimMode mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_play_anim_idx (IntPtr model, int index, AnimMode mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_set_anim_time (IntPtr model, float time);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_set_anim_completion(IntPtr model, float percent);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 model_anim_find     (IntPtr model, string animation_name);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_anim_count    (IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_anim_active   (IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern AnimMode            model_anim_active_mode(IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               model_anim_active_time(IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               model_anim_active_completion(IntPtr model);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              model_anim_get_name (IntPtr model, int index);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               model_anim_get_duration(IntPtr model, int index);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              model_get_name      (IntPtr model, int subset);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_get_material  (IntPtr model, int subset);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_get_mesh      (IntPtr model, int subset);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              model_get_transform (IntPtr model, int subset);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_set_material  (IntPtr model, int subset, IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_set_mesh      (IntPtr model, int subset, IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_set_transform (IntPtr model, int subset, in Matrix transform);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_remove_subset (IntPtr model, int subset);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 model_add_named_subset(IntPtr model, string name, IntPtr mesh, IntPtr material, in Matrix transform);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_add_subset    (IntPtr model, IntPtr mesh, IntPtr material, in Matrix transform);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_subset_count  (IntPtr model);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 model_node_add      (IntPtr model, string name, Matrix model_transform, IntPtr mesh, IntPtr material, int solid);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 model_node_add_child(IntPtr model, int parent, string name, Matrix local_transform, IntPtr mesh, IntPtr material, int solid);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 model_node_find     (IntPtr model, string name);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_sibling  (IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_parent   (IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_child    (IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_count    (IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_index    (IntPtr model, int index);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_visual_count(IntPtr model);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_visual_index(IntPtr model, int index);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_iterate  (IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_get_root (IntPtr model);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              model_node_get_name (IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_get_solid(IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_get_visible(IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_node_get_material(IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_node_get_mesh (IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              model_node_get_transform_model(IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              model_node_get_transform_local(IntPtr model, int node);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                model_node_set_name (IntPtr model, int node, string name);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_node_set_solid(IntPtr model, int node, int solid);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_node_set_visible(IntPtr model, int node, int visible);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_node_set_material(IntPtr model, int node, IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_node_set_mesh (IntPtr model, int node, IntPtr mesh);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_node_set_transform_model(IntPtr model, int node, Matrix transform_model_space);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_node_set_transform_local(IntPtr model, int node, Matrix transform_local_space);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              model_node_info_get (IntPtr model, int node, [In] byte[] info_key_utf8);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_node_info_set (IntPtr model, int node, [In] byte[] info_key_utf8, [In] byte[] info_value_utf8);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_info_remove(IntPtr model, int node, [In] byte[] info_key_utf8);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                model_node_info_clear(IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_info_count(IntPtr model, int node);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 model_node_info_iterate(IntPtr model, int node, ref int ref_iterator, [Out] byte[] out_key_utf8, [Out] byte[] out_value_utf8);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              sprite_create       (IntPtr sprite, SpriteType type, string atlas_id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              sprite_create_file  ([In] byte[] filename_utf8, SpriteType type, string atlas_id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                sprite_set_id       (IntPtr sprite, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              sprite_get_id       (IntPtr sprite);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sprite_addref       (IntPtr sprite);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sprite_release      (IntPtr sprite);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               sprite_get_aspect   (IntPtr sprite);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 sprite_get_width    (IntPtr sprite);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 sprite_get_height   (IntPtr sprite);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec2                sprite_get_dimensions_normalized(IntPtr sprite);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sprite_draw         (IntPtr sprite, in Matrix transform, Color32 color);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sprite_draw_at      (IntPtr sprite, Matrix transform, TextAlign anchor_position, Color32 color);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern void                line_add            (Vec3 start, Vec3 end, Color32 color_start, Color32 color_end, float thickness);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                line_addv           (LinePoint start, LinePoint end);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                line_add_axis       (Pose pose, float size);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                line_add_list       (in Vec3 points, int count, Color32 color, float thickness);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                line_add_listv      ([In] LinePoint[] in_arr_points, int count);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_clip     (float near_plane, float far_plane);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_fov      (float field_of_view_degrees);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_ortho_clip(float near_plane, float far_plane);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_ortho_size(float viewport_height_meters);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_projection(Projection proj);
		[DllImport(dll, CallingConvention = call                 )] public static extern Projection          render_get_projection();
		[DllImport(dll, CallingConvention = call                 )] public static extern Matrix              render_get_cam_root ();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_cam_root (in Matrix cam_root);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_skytex   (IntPtr sky_texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              render_get_skytex   ();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_skylight (in SphericalHarmonics light_info);
		[DllImport(dll, CallingConvention = call                 )] public static extern SphericalHarmonics  render_get_skylight ();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_filter   (RenderLayer layer_filter);
		[DllImport(dll, CallingConvention = call                 )] public static extern RenderLayer         render_get_filter   ();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_override_capture_filter(int use_override_filter, RenderLayer layer_filter);
		[DllImport(dll, CallingConvention = call                 )] public static extern RenderLayer         render_get_capture_filter();
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 render_has_capture_filter();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_set_clear_color(Color color_gamma);
		[DllImport(dll, CallingConvention = call                 )] public static extern Color               render_get_clear_color();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_enable_skytex(int show_sky);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 render_enabled_skytex();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_global_texture(int register_slot, IntPtr texture);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_add_mesh     (IntPtr mesh, IntPtr material, in Matrix transform, Color color_linear, RenderLayer layer);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_add_model    (IntPtr model, in Matrix transform, Color color_linear, RenderLayer layer);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_blit         (IntPtr to_rendertarget, IntPtr material);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                render_screenshot   (string file, Vec3 from_viewpt, Vec3 at, int width, int height, float field_of_view_degrees);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_to           (IntPtr to_rendertarget, in Matrix camera, in Matrix projection, RenderLayer layer_filter, RenderClear clear, Rect viewport);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_material_to  (IntPtr to_rendertarget, IntPtr override_material, in Matrix camera, in Matrix projection, RenderLayer layer_filter, RenderClear clear, Rect viewport);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                render_get_device   (IntPtr device, IntPtr context);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern void                hierarchy_push      (in Matrix transform);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                hierarchy_pop       ();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                hierarchy_set_enabled(int enabled);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 hierarchy_is_enabled();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              hierarchy_to_world  ();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              hierarchy_to_local  ();
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                hierarchy_to_local_point(in Vec3 world_pt);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                hierarchy_to_local_direction(in Vec3 world_dir);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                hierarchy_to_local_rotation(in Quat world_orientation);
		[DllImport(dll, CallingConvention = call                 )] public static extern Pose                hierarchy_to_local_pose(in Pose world_pose);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                hierarchy_to_world_point(in Vec3 local_pt);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                hierarchy_to_world_direction(in Vec3 local_dir);
		[DllImport(dll, CallingConvention = call                 )] public static extern Quat                hierarchy_to_world_rotation(in Quat local_orientation);
		[DllImport(dll, CallingConvention = call                 )] public static extern Pose                hierarchy_to_world_pose(in Pose local_pose);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              sound_find          (string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                sound_set_id        (IntPtr sound, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              sound_get_id        (IntPtr sound);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              sound_create        ([In] byte[] filename_utf8);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              sound_create_stream (float buffer_duration);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              sound_create_samples([In] float[] in_arr_samples_at_48000s, ulong sample_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              sound_generate      ([MarshalAs(UnmanagedType.FunctionPtr)] AudioGenerator audio_generator, float duration);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sound_write_samples (IntPtr sound, [In] float[] in_arr_samples, ulong sample_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern ulong               sound_read_samples  (IntPtr sound, [Out] float[] out_arr_samples, ulong sample_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern ulong               sound_unread_samples(IntPtr sound);
		[DllImport(dll, CallingConvention = call                 )] public static extern ulong               sound_total_samples (IntPtr sound);
		[DllImport(dll, CallingConvention = call                 )] public static extern ulong               sound_cursor_samples(IntPtr sound);
		[DllImport(dll, CallingConvention = call                 )] public static extern SoundInst           sound_play          (IntPtr sound, Vec3 at, float volume);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               sound_duration      (IntPtr sound);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sound_addref        (IntPtr sound);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sound_release       (IntPtr sound);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sound_inst_stop     (SoundInst sound_inst);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 sound_inst_is_playing(SoundInst sound_inst);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sound_inst_set_pos  (SoundInst sound_inst, Vec3 pos);
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec3                sound_inst_get_pos  (SoundInst sound_inst);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                sound_inst_set_volume(SoundInst sound_inst, float volume);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               sound_inst_get_volume(SoundInst sound_inst);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern int                 mic_device_count    ();
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              mic_device_name     (int index);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 mic_start           (string device_name);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                mic_stop            ();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              mic_get_stream      ();
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 mic_is_recording    ();

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern void                platform_file_picker(PickerMode mode, IntPtr callback_data, [MarshalAs(UnmanagedType.FunctionPtr)] PickerCallback picker_callback, in FileFilter filters, int filter_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                platform_file_picker_sz(PickerMode mode, IntPtr callback_data, [MarshalAs(UnmanagedType.FunctionPtr)] PickerCallbackSz picker_callback_sz, [In] FileFilter[] in_arr_filters, int filter_count);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                platform_file_picker_close();
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 platform_file_picker_visible();
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 platform_read_file  ([In] byte[] filename_utf8, out IntPtr out_data, out UIntPtr out_size);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 platform_write_file ([In] byte[] filename_utf8, byte[] data, UIntPtr size);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 platform_write_file_text([In] byte[] filename_utf8, [In] byte[] text_utf8);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 platform_keyboard_get_force_fallback();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                platform_keyboard_set_force_fallback(int force_fallback);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                platform_keyboard_show(int visible, TextContext type);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 platform_keyboard_visible();

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern int                 input_pointer_count (InputSource filter);
		[DllImport(dll, CallingConvention = call                 )] public static extern Pointer             input_pointer       (int index, InputSource filter);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              input_hand          (Handed hand);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                input_hand_override (Handed hand, [In] HandJoint[] in_arr_hand_joints);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              input_controller    (Handed hand);
		[DllImport(dll, CallingConvention = call                 )] public static extern BtnState            input_controller_menu();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              input_head          ();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              input_eyes          ();
		[DllImport(dll, CallingConvention = call                 )] public static extern BtnState            input_eyes_tracked  ();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              input_mouse         ();
		[DllImport(dll, CallingConvention = call                 )] public static extern BtnState            input_key           (Key key);
		[DllImport(dll, CallingConvention = call                 )] public static extern uint                input_text_consume  ();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                input_text_reset    ();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                input_hand_visible  (Handed hand, int visible);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                input_hand_solid    (Handed hand, int solid);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                input_hand_material (Handed hand, IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                input_subscribe     (InputSource source, BtnState input_event, [MarshalAs(UnmanagedType.FunctionPtr)] InputEventCallback input_event_callback);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                input_unsubscribe   (InputSource source, BtnState input_event, [MarshalAs(UnmanagedType.FunctionPtr)] InputEventCallback input_event_callback);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                input_fire_event    (InputSource source, BtnState input_event, in Pointer pointer);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern int                 world_has_bounds    ();
		[DllImport(dll, CallingConvention = call                 )] public static extern Vec2                world_get_bounds_size();
		[DllImport(dll, CallingConvention = call                 )] public static extern Pose                world_get_bounds_pose();
		[DllImport(dll, CallingConvention = call                 )] public static extern Pose                world_from_spatial_graph([MarshalAs(UnmanagedType.LPArray, SizeConst = 16)] byte[] spatial_graph_node_id, int dynamic, long qpc_time);
		[DllImport(dll, CallingConvention = call                 )] public static extern Pose                world_from_perception_anchor(IntPtr perception_spatial_anchor);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 world_try_from_spatial_graph([MarshalAs(UnmanagedType.LPArray, SizeConst = 16)] byte[] spatial_graph_node_id, int dynamic, long qpc_time, out Pose out_pose);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 world_try_from_perception_anchor(IntPtr perception_spatial_anchor, out Pose out_pose);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 world_raycast       (Ray ray, out Ray out_intersection);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                world_set_occlusion_enabled(int enabled);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 world_get_occlusion_enabled();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                world_set_raycast_enabled(int enabled);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 world_get_raycast_enabled();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                world_set_occlusion_material(IntPtr material);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              world_get_occlusion_material();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                world_set_refresh_type(WorldRefresh refresh_type);
		[DllImport(dll, CallingConvention = call                 )] public static extern WorldRefresh        world_get_refresh_type();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                world_set_refresh_radius(float radius_meters);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               world_get_refresh_radius();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                world_set_refresh_interval(float every_seconds);
		[DllImport(dll, CallingConvention = call                 )] public static extern float               world_get_refresh_interval();

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern BackendXRType       backend_xr_get_type ();
		[DllImport(dll, CallingConvention = call                 )] public static extern ulong               backend_openxr_get_instance();
		[DllImport(dll, CallingConvention = call                 )] public static extern ulong               backend_openxr_get_session();
		[DllImport(dll, CallingConvention = call                 )] public static extern ulong               backend_openxr_get_system_id();
		[DllImport(dll, CallingConvention = call                 )] public static extern ulong               backend_openxr_get_space();
		[DllImport(dll, CallingConvention = call                 )] public static extern long                backend_openxr_get_time();
		[DllImport(dll, CallingConvention = call                 )] public static extern long                backend_openxr_get_eyes_sample_time();
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              backend_openxr_get_function(string function_name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern int                 backend_openxr_ext_enabled(string extension_name);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                backend_openxr_ext_request(string extension_name);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                backend_openxr_use_minimum_exts(int use_minimum_exts);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                backend_openxr_composition_layer(IntPtr XrCompositionLayerBaseHeader, int layer_size, int sort_order);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                backend_openxr_add_callback_pre_session_create([MarshalAs(UnmanagedType.FunctionPtr)] XrPreSessionCreateCallback xr_pre_session_create_callback, IntPtr context);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                backend_openxr_add_callback_poll_event([MarshalAs(UnmanagedType.FunctionPtr)] XrPollEventCallback xr_poll_event_callback, IntPtr context);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                backend_openxr_remove_callback_poll_event([MarshalAs(UnmanagedType.FunctionPtr)] XrPollEventCallback xr_poll_event_callback);
		[DllImport(dll, CallingConvention = call                 )] public static extern BackendPlatform     backend_platform_get();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_android_get_java_vm();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_android_get_activity();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_android_get_jni_env();
		[DllImport(dll, CallingConvention = call                 )] public static extern BackendGraphics     backend_graphics_get();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_d3d11_get_d3d_device();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_d3d11_get_d3d_context();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_opengl_wgl_get_hdc();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_opengl_wgl_get_hglrc();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_opengl_glx_get_context();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_opengl_glx_get_display();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_opengl_glx_get_drawable();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_opengl_egl_get_context();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_opengl_egl_get_config();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              backend_opengl_egl_get_display();

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_diag            (string text);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_diagf           (string text);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_info            (string text);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_infof           (string text);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_warn            (string text);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_warnf           (string text);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_err             (string text);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_errf            (string text);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_writef          (LogLevel level, string text);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                log_write           (LogLevel level, string text);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                log_set_filter      (LogLevel level);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                log_set_colors      (LogColors colors);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                log_subscribe       ([MarshalAs(UnmanagedType.FunctionPtr)] LogCallback log_callback);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                log_unsubscribe     ([MarshalAs(UnmanagedType.FunctionPtr)] LogCallback log_callback);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern void                assets_releaseref_threadsafe(IntPtr asset);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 assets_current_task ();
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 assets_total_tasks  ();
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 assets_current_task_priority();
		[DllImport(dll, CallingConvention = call                 )] public static extern void                assets_block_for_priority(int priority);
		[DllImport(dll, CallingConvention = call                 )] public static extern int                 assets_count        ();
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr              assets_get_index    (int index);
		[DllImport(dll, CallingConvention = call                 )] public static extern AssetType           assets_get_type     (int index);

		///////////////////////////////////////////

		[DllImport(dll, CallingConvention = call                 )] public static extern AssetType           asset_get_type      (IntPtr asset);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern void                asset_set_id        (IntPtr asset, string id);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr              asset_get_id        (IntPtr asset);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                asset_addref        (IntPtr asset);
		[DllImport(dll, CallingConvention = call                 )] public static extern void                asset_release       (IntPtr asset);
	}
}
