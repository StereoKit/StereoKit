using System;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
	internal static partial class NativeAPI
	{
		const CharSet cSet = CharSet.Ansi;
		
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr tex_create_cubemap_file ([In] byte[] equirectangular_file_utf8, int srgb_data, IntPtr out_sh_lighting_info, int priority);
		[DllImport(dll, CallingConvention = call, CharSet = ascii)] public static extern IntPtr tex_create_cubemap_files([In] string[] in_arr_cube_face_file_xxyyzz, int srgb_data, IntPtr out_sh_lighting_info, int priority);
		[DllImport(dll, CallingConvention = call                 )] public static extern IntPtr tex_gen_cubemap         (IntPtr gradient, Vec3 gradient_dir, int resolution, IntPtr out_sh_lighting_info);
		[DllImport(dll, CallingConvention = call                 )] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, [In] Color32[] data);
		[DllImport(dll, CallingConvention = call                 )] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, [In] ushort[] data);
		[DllImport(dll, CallingConvention = call                 )] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, [In] float[] data);
		[DllImport(dll, CallingConvention = call                 )] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, [In] Color[] data);
		[DllImport(dll, CallingConvention = call                 )] public static extern void   tex_set_colors          (IntPtr texture, int width, int height, IntPtr data);

		[DllImport(dll, CallingConvention = call                 )] public static extern void   mesh_get_verts          (IntPtr mesh, out IntPtr out_arr_vertices, out int out_vertex_count, Memory reference_mode);
		[DllImport(dll, CallingConvention = call                 )] public static extern void   mesh_get_inds           (IntPtr mesh, out IntPtr out_arr_indices, out int out_index_count, Memory reference_mode);

		[DllImport(dll, CallingConvention = call                 )] public static extern void   input_hand_override     (Handed hand, IntPtr in_arr_hand_joints);
		
		///////////////////////////////////////////

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  ui_quadrant_size_verts ([In, Out] Vertex[] ref_vertices, int vertex_count, float overflow);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  ui_quadrant_size_mesh  (IntPtr ref_mesh, float overflow);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  ui_show_volumes        (bool show);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  ui_enable_far_interact (bool enable);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool  ui_far_interact_enabled();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  ui_settings            (UISettings settings);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  ui_set_color           (Color color);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  ui_set_theme_color     (UIColor color_type, Color color_gamma);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Color ui_get_theme_color     (UIColor color_type);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void  ui_set_element_visual  (UIVisual element_visual, IntPtr mesh, IntPtr material, Vec2 min_size);
		
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_text_style       (TextStyle style);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_text_style        ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_tint             (Color tint_gamma);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_tint              ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_enabled          (int enabled);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_enabled           ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_preserve_keyboard(int preserve_keyboard);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_preserve_keyboard ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_push_surface          (Pose surface_pose, Vec3 layout_start, Vec2 layout_dimensions);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_pop_surface           ();
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern ulong ui_push_id_16   (string id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]            public static extern ulong ui_push_idi     (int id);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]            public static extern void  ui_pop_id       ();
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern ulong ui_stack_hash_16(string str);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_layout_area     (Vec3 start, Vec2 dimensions);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec2   ui_layout_remaining();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec3   ui_layout_at       ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds ui_layout_last     ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Bounds ui_layout_reserve  (Vec2 size, int add_padding = 0, float depth = 0);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState ui_last_element_hand_used(Handed hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState ui_last_element_active();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern BtnState ui_last_element_focused();

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern Vec2   ui_area_remaining();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_nextline      ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_sameline      ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern float  ui_line_height   ();
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void   ui_space         (float space);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern bool ui_is_interacting (Handed hand);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)] public static extern void ui_button_behavior(Vec3 window_relative_pos, Vec2 size, ulong id, out float finger_offset, out BtnState button_state, out BtnState focus_state);

		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern BtnState ui_volumei_at_16     (string id, Bounds bounds, UIConfirm interact_type, IntPtr out_opt_hand, IntPtr out_opt_focus_state);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern BtnState ui_volumei_at_16     (string id, Bounds bounds, UIConfirm interact_type, out Handed out_opt_hand, IntPtr out_opt_focus_state);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern BtnState ui_volumei_at_16     (string id, Bounds bounds, UIConfirm interact_type, out Handed out_opt_hand, out BtnState out_opt_focus_state);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_volume_at_16      (string id, Bounds bounds);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern BtnState ui_interact_volume_at(Bounds bounds, out Handed out_hand);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_button_at_16      (string text, Vec3 window_relative_pos, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_button_img_16     (string text, IntPtr image, UIBtnLayout image_layout, Vec3 window_relative_pos, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_button_round_at_16(string text, IntPtr image_sprite, Vec3 window_relative_pos, float diameter);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_toggle_at_16      (string text, ref int pressed, Vec3 window_relative_pos, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void     ui_progress_bar_at   (float percent, Vec3 window_relative_pos, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_hslider_at_16     (string id,   ref float value, float min, float max, float step, Vec3 window_relative_pos, Vec2 size, UIConfirm confirm_method);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool     ui_hslider_at_f64_16 (string id,   ref double value, double min, double max, double step, Vec3 window_relative_pos, Vec2 size, UIConfirm confirm_method);

		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_hseparator     ();
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_label_16       (string text, bool use_padding);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_label_sz_16    (string text, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_text_16        (string text, TextAlign text_align);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_image          (IntPtr sprite_image, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_16      (string text);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_sz_16   (string text, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_img_16   (string text, IntPtr image, UIBtnLayout image_layout);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_img_sz_16(string text, IntPtr image, UIBtnLayout image_layout, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_button_round_16(string id, IntPtr image, float diameter);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_toggle_16      (string text, ref int pressed);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_toggle_sz_16   (string text, ref int pressed, Vec2 size);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_model          (IntPtr model, Vec2 ui_size, float model_scale);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_input_16       (string id,   StringBuilder buffer, int buffer_size, Vec2 size, TextContext type);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_progress_bar   (float percent, float width);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_hslider_16     (string id,   ref float value, float min, float max, float step, float width, UIConfirm confirm_method);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_hslider_f64_16 (string id,   ref double value, double min, double max, double step, float width, UIConfirm confirm_method);
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern bool ui_handle_begin_16(string text, ref Pose movement, Bounds handle, int draw, UIMove move_type);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_handle_end     ();
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)] public static extern void ui_window_begin_16(string text, ref Pose pose, Vec2 size, UIWin window_type, UIMove move_type);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_window_end     ();
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_panel_at       (Vec3 start, Vec2 size, UIPad padding);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_panel_begin    (UIPad padding);
		[DllImport(dll, CharSet = cSet,            CallingConvention = call)] public static extern void ui_panel_end      ();
	}
}
