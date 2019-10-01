
namespace StereoKit
{
    public static class UI
    {
        public static void Init       () { NativeAPI.sk_ui_init(); }

        public static void NextLine   ()            { NativeAPI.sk_ui_nextline(); }
        public static void SameLine   ()            { NativeAPI.sk_ui_sameline(); }
        public static void ReserveBox (Vec2  size)  { NativeAPI.sk_ui_reserve_box(size); }
        public static void Space      (float space) { NativeAPI.sk_ui_space(space); }

        public static void Label      (string text) { NativeAPI.sk_ui_label(text); }
        public static void Image      (Sprite image, Vec2 size) { NativeAPI.sk_ui_image(image._spriteInst, size); }
        public static bool Button     (string text) { return NativeAPI.sk_ui_button(text) > 0; }
        public static bool Affordance (string text, ref Pose movement, Vec3 at, Vec3 size) { return NativeAPI.sk_ui_affordance(text, ref movement, at, size) > 0; }
        public static bool HSlider    (string id,   ref float value, float min, float max, float step, float width = 0) { return NativeAPI.sk_ui_hslider(id, ref value, min, max, step, width) > 0; }
        public static void WindowBegin(string text, ref Pose pose, Vec2 size) { NativeAPI.sk_ui_window_begin(text, ref pose, size); }
        public static void WindowEnd  () { NativeAPI.sk_ui_window_end(); }
    }
}
