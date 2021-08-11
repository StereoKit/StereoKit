﻿using System.Text;

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
		/// <summary>UI sizing and layout settings. Set only for now</summary>
		public static UISettings Settings    { set { NativeAPI.ui_settings(value); } }

		/// <summary>StereoKit will generate a color palette from this gamma
		/// space color, and use it to skin the UI!</summary>
		public static Color      ColorScheme { set { NativeAPI.ui_set_color(value); } }

		/// <summary>Shows or hides the collision volumes of the UI! This is
		/// for debug purposes, and can help identify visible and invisible
		/// collision issues.</summary>
		public static bool       ShowVolumes { set { NativeAPI.ui_show_volumes(value); } }

		/// <summary>Enables or disables the far ray grab interaction for 
		/// Handle elements like the Windows. It can be enabled and disabled
		/// for individual UI elements, and if this remains disabled at the
		/// start of the next frame, then the hand ray indicators will not be
		/// visible. This is enabled by default. </summary>
		public static bool       EnableFarInteract { get => NativeAPI.ui_far_interact_enabled(); set { NativeAPI.ui_enable_far_interact(value); } }

		/// <summary>This is the height of a single line of text with padding in the UI's layout system!</summary>
		public static float      LineHeight => NativeAPI.ui_line_height();

		/// <summary>How much space is available on the current layout! This is based on the current
		/// layout position, so X will give you the amount remaining on the current line, and Y will
		/// give you distance to the bottom of the layout, including the current line. These values
		/// will be 0 if you're using 0 for the layout size on that axis.</summary>
		public static Vec2       AreaRemaining => NativeAPI.ui_area_remaining();

		/// <summary>Tells if the user is currently interacting with a UI element! This will be true
		/// if the hand has an active or focused UI element.</summary>
		/// <param name="hand">Which hand is interacting?</param>
		/// <returns>True if the hand has an active or focused UI element. False otherwise.</returns>
		public static bool IsInteracting(Handed hand)
			=> NativeAPI.ui_is_interacting(hand);

		/// <summary>Manually define what area is used for the UI layout. This is in
		/// the current Hierarchy's coordinate space on the X/Y plane.</summary>
		/// <param name="start">The top left of the layout area, relative to the current Hierarchy in local meters.</param>
		/// <param name="dimensions">The size of the layout area from the top left, in local meters.</param>
		public static void LayoutArea(Vec3 start, Vec2 dimensions)
			=> NativeAPI.ui_layout_area(start, dimensions);

		/// <summary>Reserves a box of space for an item in the current UI layout! This doesn't
		/// advance to the next line after it, use UI.NextLine for that.</summary>
		/// <param name="size">Size of the box in Hierarchy local meters.</param>
		public static void ReserveBox(Vec2 size) 
			=> NativeAPI.ui_reserve_box(size);

		/// <summary>Moves the current layout position back to the end of the line that just finished,
		/// so it can continue on the same line as the last element!</summary>
		public static void SameLine() 
			=> NativeAPI.ui_sameline();

		/// <summary>This will advance the layout to the next line. If there's nothing on the current
		/// line, it'll advance to the start of the next on. But this won't have any affect on an
		/// empty line, try UI.Space for that.</summary>
		public static void NextLine() 
			=> NativeAPI.ui_nextline();

		/// <summary>Adds some space! If we're at the start of a new line, space is added vertically, 
		/// otherwise, space is added horizontally.</summary>
		/// <param name="space">Physical space to shift the layout by.</param>
		public static void Space (float space) 
			=> NativeAPI.ui_space(space);

		/// <summary>An invisible volume that will trigger when a finger enters it!</summary>
		/// <param name="id">A per-window unique id for tracking element state.</param>
		/// <param name="bounds">Size and position of the volume, relative to the current Hierarchy.</param>
		/// <returns>True on the first frame a finger has entered the volume, false otherwise.</returns>
		public static bool VolumeAt(string id, Bounds bounds)
			=> NativeAPI.ui_volume_at_16(id, bounds);

		/// <summary>This watches a volume of space for pinch interaction 
		/// events! If a hand is inside the space indicated by the bounds,
		/// this function will return that hand's pinch state, as well as
		/// indicate which hand did it through the out parameter.
		/// 
		/// Note that since this only provides the hand's pinch state, it 
		/// won't give you JustActive and JustInactive notifications for 
		/// when the hand enters or leaves the volume.</summary>
		/// <param name="bounds">A UI hierarchy space bounding volume.</param>
		/// <param name="hand">This will be the last hand that provides a 
		/// pinch state within this volume. That means that if both hands are
		/// pinching in this volume, it will provide the Right hand.</param>
		/// <returns>This will be the pinch state of the last hand that
		/// provides a pinch state within this volume. That means that if
		/// both hands are pinching in this volume, it will provide the pinch
		/// state of the Right hand.</returns>
		public static BtnState InteractVolume(Bounds bounds, out Handed hand)
			=> NativeAPI.ui_interact_volume_at(bounds, out hand);

		/// <summary>This draws a line horizontally across the current
		/// layout. Makes a good separator between sections of UI!</summary>
		public static void HSeparator()
			=> NativeAPI.ui_hseparator();

		/// <summary>Adds some text to the layout! Text uses the UI's current
		/// font settings (which are currently not exposed). Can contain
		/// newlines! May have trouble with non-latin characters. Will
		/// advance layout to next line.</summary>
		/// <param name="text">Label text to display. Can contain newlines!
		/// May have trouble with non-latin characters. Doesn't use text as
		/// id, so it can be non-unique.</param>
		/// <param name="usePadding">Should padding be included for
		/// positioning this text? Sometimes you just want un-padded text!
		/// </param>
		public static void Label (string text, bool usePadding = true) 
			=> NativeAPI.ui_label_16(text, usePadding);

		public static void Label(string text, Vec2 size)
			=> NativeAPI.ui_label_sz_16(text, size);

		/// <summary>Displays a large chunk of text on the current layout.
		/// This can include new lines and spaces, and will properly wrap
		/// once it fills the entire layout!</summary>
		/// <param name="text">The text you wish to display, there's no 
		/// additional parsing done to this text, so put it in as you want to
		/// see it!</param>
		public static void Text(string text)
			=> NativeAPI.ui_text_16(text);

		/// <summary>Adds an image to the UI!</summary>
		/// <param name="image">A valid sprite.</param>
		/// <param name="size">Size in Hierarchy local meters. If one of the components is 0, 
		/// it'll be automatically determined from the other component and the image's aspect
		/// ratio.</param>
		public static void Image (Sprite image, Vec2 size) 
			=> NativeAPI.ui_image(image._inst, size);

		/// <summary>A pressable button! A button will expand to fit the text provided to it,
		/// vertically and horizontally. Text is re-used as the id. Will return true only on 
		/// the first frame it is pressed!</summary>
		/// <param name="text">Text to display on the button, should be per-window unique as 
		/// it will be used as the element id.</param>
		/// <returns>Will return true only on the first frame it is pressed!</returns>
		public static bool Button (string text) 
			=> NativeAPI.ui_button_16(text);

		public static bool Button(string text, Vec2 size)
			=> NativeAPI.ui_button_sz_16(text, size);

		public static bool ButtonAt(string text, Vec3 windowRelativeCorner, Vec2 size)
			=> NativeAPI.ui_button_at_16(text, windowRelativeCorner, size);

		/// <summary>A Radio is similar to a button, except you can specify if it looks pressed
		/// or not regardless of interaction. This can be useful for radio-like behavior! Check
		/// an enum for a value, and use that as the 'active' state, Then switch to that enum 
		/// value if Radio returns true.</summary>
		/// <param name="text">Text to display on the radio, should be per-window unique as 
		/// it will be used as the element id.</param>
		/// <param name="active">Does this button look like it's pressed?</param>
		/// <returns>Will return true only on the first frame it is pressed!</returns>
		public static bool Radio (string text, bool active) 
			=> NativeAPI.ui_toggle_16(text, ref active) && active;

		public static bool Radio(string text, bool active, Vec2 size)
			=> NativeAPI.ui_toggle_sz_16(text, ref active, size) && active;

		/// <summary>A pressable button! A button will expand to fit the text provided to it,
		/// vertically and horizontally. Text is re-used as the id. Will return true only on 
		/// the first frame it is pressed!</summary>
		/// <param name="id">A per-window unique id for tracking element state.</param>
		/// <param name="image">An image to display as the face of the button.</param>
		/// <param name="diameter">The diameter of the button's visual.</param>
		/// <returns>Will return true only on the first frame it is pressed!</returns>
		public static bool ButtonRound(string id, Sprite image, float diameter = 0)
			=> NativeAPI.ui_button_round_16(id, image._inst, diameter);

		public static bool ButtonRoundAt(string text, Sprite sprite, Vec3 windowRelativeCorner, float diameter)
			=> NativeAPI.ui_button_round_at_16(text, sprite._inst, windowRelativeCorner, diameter);

		/// <summary>A toggleable button! A button will expand to fit the
		/// text provided to it, vertically and horizontally. Text is re-used 
		/// as the id. Will return true any time the toggle value changes!
		/// </summary>
		/// <param name="text">Text to display on the button, should be per-
		/// window unique as it will be used as the element id.</param>
		/// <param name="value">The current state of the toggle button! True 
		/// means it's toggled on, and false means it's toggled off.</param>
		/// <returns>Will return true any time the toggle value changes!
		/// </returns>
		public static bool Toggle (string text, ref bool value)
			=> NativeAPI.ui_toggle_16(text, ref value);

		public static bool Toggle(string text, ref bool value, Vec2 size)
			=> NativeAPI.ui_toggle_sz_16(text, ref value, size);

		public static bool ToggleAt(string text, ref bool pressed, Vec3 windowRelativeCorner, Vec2 size)
			=> NativeAPI.ui_toggle_at_16(text, ref pressed, windowRelativeCorner, size);

		public static void Model(Model model)
			=> NativeAPI.ui_model(model._inst, Vec2.Zero, 0);
		public static void Model(Model model, Vec2 uiSize, float modelScale = 0)
			=> NativeAPI.ui_model(model._inst, uiSize, modelScale);

		/// <summary>This is an input field where users can input text to the
		/// app! Selecting it will spawn a virtual keyboard, or act as the
		/// keyboard focus. Hitting escape or enter, or focusing another UI
		/// element will remove focus from this Input.</summary>
		/// <param name="id">A per-window unique id for tracking element 
		/// state.</param>
		/// <param name="value">The string that will store the Input's 
		/// content in.</param>
		/// <param name="size">Size of the Input in Hierarchy local meters.
		/// </param>
		/// <returns>Returns true every time the contents of 'value' change.
		/// </returns>
		public static bool Input(string id, ref string value, Vec2 size) {
			StringBuilder builder = value != null ? 
				new StringBuilder(value, value.Length + 4) :
				new StringBuilder(4);

			if (NativeAPI.ui_input_16(id, builder, builder.Capacity, size)) { 
				value = builder.ToString();
				return true;
			}
			return false;
		}

		/// <summary>This is an input field where users can input text to the
		/// app! Selecting it will spawn a virtual keyboard, or act as the
		/// keyboard focus. Hitting escape or enter, or focusing another UI
		/// element will remove focus from this Input.</summary>
		/// <param name="id">A per-window unique id for tracking element 
		/// state.</param>
		/// <param name="value">The string that will store the Input's 
		/// content in.</param>
		/// <returns>Returns true every time the contents of 'value' change.
		/// </returns>
		public static bool Input(string id, ref string value)
			=> Input(id, ref value, Vec2.Zero);

		/// <summary>A horizontal slider element! You can stick your finger 
		/// in it, and slide the value up and down.</summary>
		/// <param name="id">A per-window unique id for tracking element 
		/// state.</param>
		/// <param name="value">The value that the slider will store slider 
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, left side 
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, right 
		/// side of the slider.</param>
		/// <param name="step">Locks the value to intervals of step. Starts 
		/// at min, and increments by step.</param>
		/// <param name="width">Physical width of the slider on the window.
		/// </param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool HSlider(string id, ref float value, float min, float max, float step, float width = 0, UIConfirm confirmMethod = UIConfirm.Push) 
			=> NativeAPI.ui_hslider_16(id, ref value, min, max, step, width, confirmMethod);

		public static bool HSliderAt(string id, ref float value, float min, float max, float step, Vec3 windowRelativeCorner, Vec2 size, UIConfirm confirmMethod = UIConfirm.Push)
			=> NativeAPI.ui_hslider_at_16(id, ref value, min, max, step, windowRelativeCorner, size, confirmMethod);

		/// <summary>This begins a new UI group with its own layout! Much 
		/// like a window, except with a more flexible handle, and no header.
		/// You can draw the handle, but it will have no text on it. Returns 
		/// true for every frame the user is grabbing the handle.</summary>
		/// <param name="id">Id of the handle group.</param>
		/// <param name="pose">The pose state for the handle! The user will 
		/// be able to grab this handle and move it around.</param>
		/// <param name="handle">Size and location of the handle, relative to 
		/// the pose.</param>
		/// <param name="drawHandle">Should this function draw the handle 
		/// visual for you, or will you draw that yourself?</param>
		/// <param name="moveType">Describes how the handle will move when 
		/// dragged around.</param>
		/// <returns>Returns true for every frame the user is grabbing the 
		/// handle.</returns>
		public static bool HandleBegin (string id, ref Pose pose, Bounds handle, bool drawHandle = false, UIMove moveType = UIMove.Exact)
			=> NativeAPI.ui_handle_begin_16(id, ref pose, handle, drawHandle, moveType);

		/// <summary>Finishes a handle! Must be called after UI.HandleBegin()
		/// and all elements have been drawn.</summary>
		public static void HandleEnd   ()
			=> NativeAPI.ui_handle_end();

		/// <summary>This begins and ends a handle so you can just use  its 
		/// grabbable/moveable functionality! Behaves much like a window,
		/// except with a more flexible handle, and no header. You can draw 
		/// the handle, but it will have no text on it. Returns true for 
		/// every frame the user is grabbing the handle.</summary>
		/// <param name="id">Id of the handle group.</param>
		/// <param name="pose">The pose state for the handle! The user will 
		/// be able to grab this handle and move it around.</param>
		/// <param name="handle">Size and location of the handle, relative to 
		/// the pose.</param>
		/// <param name="drawHandle">Should this function draw the handle for 
		/// you, or will you draw that yourself?</param>
		/// <param name="moveType">Describes how the handle will move when 
		/// dragged around.</param>
		/// <returns>Returns true for every frame the user is grabbing the 
		/// handle.</returns>
		public static bool Handle(string id, ref Pose pose, Bounds handle, bool drawHandle = false, UIMove moveType = UIMove.Exact)
		{
			bool result = NativeAPI.ui_handle_begin_16(id, ref pose, handle, drawHandle, moveType);
			NativeAPI.ui_handle_end();
			return result;
		}

		/// <summary>Begins a new window! This will push a pose onto the 
		/// transform stack, and all UI elements will be relative to that new 
		/// pose. The pose is actually the top-center of the window. Must be 
		/// finished with a call to UI.WindowEnd().</summary>
		/// <param name="text">Text to display on the window title, should be 
		/// unique as it will be used as the window's id.</param>
		/// <param name="pose">The pose state for the window! If showHeader 
		/// is true, the user will be able to grab this header and move it 
		/// around.</param>
		/// <param name="size">Physical size of the window! If either 
		/// dimension is 0, then the size on that axis will be auto-
		/// calculated based on the content provided during the previous 
		/// frame.</param>
		/// <param name="windowType">Describes how the window should be drawn,
		/// use a header, a body, neither, or both?</param>
		/// <param name="moveType">Describes how the window will move when 
		/// dragged around.</param>
		public static void WindowBegin(string text, ref Pose pose, Vec2 size, UIWin windowType = UIWin.Normal, UIMove moveType = UIMove.FaceUser)
			=> NativeAPI.ui_window_begin_16(text, ref pose, size, windowType, moveType);

		/// <summary>Begins a new window! This will push a pose onto the 
		/// transform stack, and all UI elements will be relative to that new 
		/// pose. The pose is actually the top-center of the window. Must be 
		/// finished with a call to UI.WindowEnd(). This override omits the
		/// size value, so the size will be auto-calculated based on the
		/// content provided during the previous frame.</summary>
		/// <param name="text">Text to display on the window title, should be 
		/// unique as it will be used as the window's id.</param>
		/// <param name="pose">The pose state for the window! If showHeader 
		/// is true, the user will be able to grab this header and move it 
		/// around.</param>
		/// <param name="windowType">Describes how the window should be drawn,
		/// use a header, a body, neither, or both?</param>
		/// <param name="moveType">Describes how the window will move when 
		/// dragged around.</param>
		public static void WindowBegin(string text, ref Pose pose, UIWin windowType = UIWin.Normal, UIMove moveType = UIMove.FaceUser)
			=> NativeAPI.ui_window_begin_16(text, ref pose, Vec2.Zero, windowType, moveType);

		/// <summary>Finishes a window! Must be called after UI.WindowBegin()
		/// and all elements have been drawn.</summary>
		public static void WindowEnd()
			=> NativeAPI.ui_window_end();

		/// <summary>Adds a root id to the stack for the following UI 
		/// elements! This id is combined when hashing any following ids, to
		/// prevent id collisions in separate groups.</summary>
		/// <param name="rootId">The root id to use until the following PopId 
		/// call.</param>
		public static void PushId(string rootId) 
			=> NativeAPI.ui_push_id_16(rootId);

		/// <summary>Adds a root id to the stack for the following UI 
		/// elements! This id is combined when hashing any following ids, to
		/// prevent id collisions in separate groups.</summary>
		/// <param name="rootId">The root id to use until the following PopId 
		/// call.</param>
		public static void PushId(int rootId)
			=> NativeAPI.ui_push_idi(rootId);

		/// <summary>Removes the last root id from the stack, and moves up to 
		/// the one before it!</summary>
		public static void PopId() 
			=> NativeAPI.ui_pop_id();

		/// <summary>This pushes a Text Style onto the style stack! All text
		/// elements rendered by the GUI system will now use this styling.
		/// </summary>
		/// <param name="style">A valid TextStyle to use.</param>
		public static void PushTextStyle(TextStyle style) 
			=> NativeAPI.ui_push_text_style(style);

		/// <summary>Removes a TextStyle from the stack, and whatever was 
		/// below will then be used as the GUI's primary font.</summary>
		public static void PopTextStyle() 
			=> NativeAPI.ui_pop_text_style();

		public static ulong StackHash(string id)
			=> NativeAPI.ui_stack_hash_16(id);

		public static void ButtonBehavior(Vec3 windowRelativePos, Vec2 size, string id, out float fingerOffset, out BtnState buttonState, out BtnState focusState)
			=> NativeAPI.ui_button_behavior(windowRelativePos, size, NativeAPI.ui_stack_hash_16(id), out fingerOffset, out buttonState, out focusState);
	}
}
