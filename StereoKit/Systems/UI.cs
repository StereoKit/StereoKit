
namespace StereoKit
{
    /// <summary>This class is a collection of user interface and interaction methods! StereoKit
    /// uses an Immediate Mode gui system, which can be very easy to work with and modify during
    /// runtime.
    /// 
    /// You must call the UI method every frame you wish it to be available, and if you no longer
    /// want it to be present, you simply stop calling it! The id of the element is used to track
    /// its state from frame to frame, so for elements with state, you'll want to avoid changing 
    /// the id during runtime! Ids are also scoped per-window, so different windows can re-use the
    /// same id, but a window cannot use the same id twice.</summary>
    public static class UI
    {
        public static UISettings Settings { set { NativeAPI.ui_settings(value); } }
        public static Color      ColorScheme { set{ NativeAPI.ui_set_color(value); } }

        /// <summary>This is the height of a single line of text with padding in the UI's layout system!</summary>
        public static float LineHeight { get =>NativeAPI.ui_line_height(); }

        public static void LayoutArea(Vec3 start, Vec2 dimensions)
            => NativeAPI.ui_layout_area(start, dimensions);

        public static void NextLine   ()            { NativeAPI.ui_nextline(); }
        /// <summary>Moves the current layout position back to the end of the line that just finished,
        /// so it can continue on the same line as the last element!</summary>
        public static void SameLine   ()            { NativeAPI.ui_sameline(); }
        public static void ReserveBox (Vec2  size)  { NativeAPI.ui_reserve_box(size); }
        /// <summary>Adds some space! If we're at the start of a new line, space is added vertically, 
        /// otherwise, space is added horizontally.</summary>
        /// <param name="space">Physical space to shift the layout by.</param>
        public static void Space      (float space) { NativeAPI.ui_space(space); }

        /// <summary>Adds some text to the layout! Text uses the UI's current font settings 
        /// (which are currently not exposed). Can contain newlines! May have trouble with
        /// non-latin characters. Will advance layout to next line.</summary>
        /// <param name="text">Label text to display. Can contain newlines! May have trouble with
        /// non-latin characters. Doesn't use text as id, so it can be non-unique.</param>
        /// <param name="usePadding">Should padding be included for positioning this text?
        /// Sometimes you just want un-padded text!</param>
        public static void Label      (string text, bool usePadding = true) => NativeAPI.ui_label(text, usePadding);
        public static void Image      (Sprite image, Vec2 size) { NativeAPI.ui_image(image._spriteInst, size); }
        /// <summary>A pressable button! A button will expand to fit the text provided to it,
        /// vertically and horizontally. Text is re-used as the id. Will return true only on 
        /// the first frame it is pressed!</summary>
        /// <param name="text">Text to display on the button, should be per-window unique as it will be used as the element id.</param>
        /// <returns>Will return true only on the first frame it is pressed!</returns>
        public static bool Button     (string text) { return NativeAPI.ui_button(text); }
        /// <summary>A pressable button! A button will expand to fit the text provided to it,
        /// vertically and horizontally. Text is re-used as the id. Will return true only on 
        /// the first frame it is pressed!</summary>
        /// <param name="text">Text to display on the button, should be per-window unique as it
        /// will be used as the element id.</param>
        /// <param name="diameter">The diameter of the button's visual.</param>
        /// <returns>Will return true only on the first frame it is pressed!</returns>
        public static bool ButtonRound(string text, float diameter = 0) => NativeAPI.ui_button_round(text, diameter);
        /// <summary>A toggleable button! A button will expand to fit the text provided to it,
        /// vertically and horizontally. Text is re-used as the id. Will return true any time 
        /// the toggle value changes!</summary>
        /// <param name="text">Text to display on the button, should be per-window unique as it 
        /// will be used as the element id.</param>
        /// <param name="value">The current state of the toggle button! True means it's toggled
        /// on, and false means it's toggled off.</param>
        /// <returns>Will return true any time the toggle value changes!</returns>
        public static bool Toggle     (string text, ref bool value) => NativeAPI.ui_toggle(text, ref value);
        public static bool AffordanceBegin (string text, ref Pose movement, Vec3 center, Vec3 dimensions, bool draw = false) { return NativeAPI.ui_affordance_begin(text, ref movement, center, dimensions, draw); }
        public static bool AffordanceEnd   () { return NativeAPI.ui_affordance_end(); }
        /// <summary>A horizontal slider element! You can stick your finger in it, and slide the
        /// value up and down.</summary>
        /// <param name="id">A per-window unique id for tracking element state.</param>
        /// <param name="value">The value that the slider will store slider state in.</param>
        /// <param name="min">The minimum value the slider can set, left side of the slider.</param>
        /// <param name="max">The maximum value the slider can set, right side of the slider.</param>
        /// <param name="step">Locks the value to intervals of step. Starts at min, and increments by step.</param>
        /// <param name="width">Physical width of the slider on the window.</param>
        /// <returns>Returns true any time the value changes.</returns>
        public static bool HSlider    (string id,   ref float value, float min, float max, float step, float width = 0) { return NativeAPI.ui_hslider(id, ref value, min, max, step, width); }
        /// <summary>Begins a new window! This will push a pose onto the transform stack, and all UI 
        /// elements will be relative to that new pose. The pose is actually the top-center
        /// of the window. Must be finished with a call to UI.WindowEnd().</summary>
        /// <param name="text">Text to display on the window title, should be unique as it 
        /// will be used as the window's id.</param>
        /// <param name="pose">The pose state for the window! If showHeader is true, the user
        /// will be able to grab this header and move it around.</param>
        /// <param name="size">Physical size of the window! Should be set to a non-zero value, otherwise
        /// it'll default to 32mm. If y is zero, it'll expand to contain all elements within it.</param>
        /// <param name="showHeader">Should the window show a header bar? Header bar includes a title,
        /// and is grabbable when it's visible.</param>
        public static void WindowBegin(string text, ref Pose pose, Vec2 size, bool showHeader = true) => NativeAPI.ui_window_begin(text, ref pose, size, showHeader);
        /// <summary>Finishes a window! Must be called after UI.WindowBegin() and all elements
        /// have been drawn.</summary>
        public static void WindowEnd  () => NativeAPI.ui_window_end();
    }
}
