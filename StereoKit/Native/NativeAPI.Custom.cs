// Hand-written overloads and alternate signatures for generated P/Invoke bindings.
// Some signatures just aren't worth trying to build into the binding system!

using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	internal static partial class NativeAPI
	{
		// Generated signature uses IntPtr for opt/nullable, these provide typed versions
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern Bounds bounds_grow_to_fit_box(Bounds bounds, Bounds box, in Matrix opt_box_transform);

		// tex_set_colors overloads for different array types
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void tex_set_colors(IntPtr texture, int width, int height, [In] Color32[] data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void tex_set_colors(IntPtr texture, int width, int height, [In] Color[] data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void tex_set_colors(IntPtr texture, int width, int height, [In] byte[] data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void tex_set_colors(IntPtr texture, int width, int height, [In] ushort[] data);
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void tex_set_colors(IntPtr texture, int width, int height, [In] float[] data);

		// tex_create_mem with byte array
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern IntPtr tex_create_mem([In] byte[] data, UIntPtr data_size, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, int priority);
		// tex_set_mem overload with byte[] data
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void tex_set_mem(IntPtr texture, [In] byte[] data, UIntPtr data_size, [MarshalAs(UnmanagedType.Bool)] bool srgb_data, [MarshalAs(UnmanagedType.Bool)] bool blocking, int priority);


		// tex_add_zbuffer with byte array
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void tex_add_zbuffer(IntPtr texture, TexFormat format, [In] byte[] data);

		// tex_set_color_arr variants - out_sh_lighting_info is opt/nullable
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void tex_set_color_arr(IntPtr texture, int width, int height, IntPtr array_data, int array_count, int multisample, IntPtr out_sh_lighting_info);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void tex_set_color_arr_mips(IntPtr texture, int width, int height, IntPtr array_data, int array_count, int mip_count, int multisample, IntPtr out_sh_lighting_info);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern IntPtr tex_gen_cubemap(IntPtr gradient, Vec3 gradient_dir, int resolution, IntPtr out_sh_lighting_info);


		// shader_create_mem with byte array
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern IntPtr shader_create_mem([In] byte[] data, UIntPtr data_size);

		// model_create_mem with byte array data
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern IntPtr model_create_mem([MarshalAs(UnmanagedType.LPUTF8Str)] string filename_utf8, [In] byte[] data, UIntPtr data_size, IntPtr shader);

		// sound_write_samples with IntPtr (for native buffers)
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void sound_write_samples(IntPtr sound, IntPtr in_arr_samples, ulong sample_count);

		// sound_read_samples with IntPtr (for native buffers)
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern ulong sound_read_samples(IntPtr sound, IntPtr out_arr_samples, ulong sample_count);

		// bounds_ray_contains with Matrix pointer (for transform chains)
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern bool bounds_ray_contains(Bounds bounds, Ray ray, IntPtr transform);


		// Log callback overloads using LogCallbackData (internal delegate with context)
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void log_subscribe([MarshalAs(UnmanagedType.FunctionPtr)] LogCallbackData log_callback, IntPtr context);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void log_unsubscribe([MarshalAs(UnmanagedType.FunctionPtr)] LogCallbackData log_callback, IntPtr context);

		// File picker overload using PickerCallback (same sig as PickerCallbackSz)
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void platform_file_picker_sz(PickerMode mode, IntPtr callback_data, [MarshalAs(UnmanagedType.FunctionPtr)] PickerCallback picker_callback_sz, [In] FileFilter[] in_arr_filters, int filter_count);


		// UI text functions with ref Vec2 scroll, opt/nullable
		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)]
		public static extern bool ui_text_16([MarshalAs(UnmanagedType.LPWStr)] string text, ref Vec2 opt_ref_scroll, UIScroll scroll_direction, float height, Align text_align, TextFit fit);

		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)]
		public static extern bool ui_text_sz_16([MarshalAs(UnmanagedType.LPWStr)] string text, ref Vec2 opt_ref_scroll, UIScroll scroll_direction, Vec2 size, Align text_align, TextFit fit);

		[return: MarshalAs(UnmanagedType.Bool)]
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)]
		public static extern bool ui_text_at_16([MarshalAs(UnmanagedType.LPWStr)] string text, ref Vec2 opt_ref_scroll, UIScroll scroll_direction, Align text_align, TextFit fit, Vec3 window_relative_pos, Vec2 size);


		// UI window functions with ref Pose, opt/nullable
		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)]
		public static extern void ui_window_begin_16([MarshalAs(UnmanagedType.LPWStr)] string text, ref Pose opt_pose, Vec2 size, UIWin window_type, UIMove move_type);

		// UI button/slider behavior - opt/nullable
		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void ui_button_behavior(Vec3 window_relative_pos, Vec2 size, IdHash id, out float out_finger_offset, out BtnState out_button_state, out BtnState out_focus_state, out int out_opt_hand);

		[DllImport(dll, CharSet = cSet, CallingConvention = call)]
		public static extern void ui_button_behavior_depth(Vec3 window_relative_pos, Vec2 size, IdHash id, float button_depth, float button_activation_depth, out float out_finger_offset, out BtnState out_button_state, out BtnState out_focus_state, out int out_opt_hand);

		[DllImport(dll, CharSet = CharSet.Unicode, CallingConvention = call)]
		public static extern BtnState ui_volume_at_16([MarshalAs(UnmanagedType.LPWStr)] string id, Bounds bounds, UIConfirm interact_type, out int out_opt_hand, out BtnState out_opt_focus_state);
	}
}
