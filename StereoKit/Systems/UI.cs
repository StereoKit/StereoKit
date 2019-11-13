
namespace StereoKit
{
    public static class UI
    {
        public static ref UISettings Settings => ref NativeAPI.ui_settings();

        public static void NextLine   ()            { NativeAPI.ui_nextline(); }
        public static void SameLine   ()            { NativeAPI.ui_sameline(); }
        public static void ReserveBox (Vec2  size)  { NativeAPI.ui_reserve_box(size); }
        public static void Space      (float space) { NativeAPI.ui_space(space); }

        public static void Label      (string text) { NativeAPI.ui_label(text); }
        public static void Image      (Sprite image, Vec2 size) { NativeAPI.ui_image(image._spriteInst, size); }
        public static bool Button     (string text) { return NativeAPI.ui_button(text); }
        public static bool ButtonRound(string text, float diameter = 0) => NativeAPI.ui_button_round(text, diameter);
        public static bool Toggle     (string text, ref bool value) => NativeAPI.ui_toggle(text, ref value);
        public static bool Affordance (string text, ref Pose movement, Vec3 at, Vec3 size) { return NativeAPI.ui_affordance(text, ref movement, at, size); }
        public static bool HSlider    (string id,   ref float value, float min, float max, float step, float width = 0) { return NativeAPI.ui_hslider(id, ref value, min, max, step, width); }
        public static void WindowBegin(string text, ref Pose pose, Vec2 size) { NativeAPI.ui_window_begin(text, ref pose, size); }
        public static void WindowEnd  () { NativeAPI.ui_window_end(); }
    }
}
