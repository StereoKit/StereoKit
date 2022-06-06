using System;
using System.Text;

namespace StereoKit
{
	/// <summary>This class is a collection of user interface and interaction
	/// methods! StereoKit uses an Immediate Mode GUI system, which can be very
	/// easy to work with and modify during runtime.
	/// 
	/// You must call the UI method every frame you wish it to be available,
	/// and if you no longer want it to be present, you simply stop calling it!
	/// The id of the element is used to track its state from frame to frame,
	/// so for elements with state, you'll want to avoid changing the id during
	/// runtime! Ids are also scoped per-window, so different windows can
	/// re-use the same id, but a window cannot use the same id twice.
	/// </summary>
	public static class UI
	{
		/// <summary>UI sizing and layout settings. Set only for now</summary>
		public static UISettings Settings { set { NativeAPI.ui_settings(value); } }

		/// <summary>StereoKit will generate a color palette from this gamma
		/// space color, and use it to skin the UI!</summary>
		public static Color ColorScheme { set { NativeAPI.ui_set_color(value); } }

		/// <summary>Shows or hides the collision volumes of the UI! This is
		/// for debug purposes, and can help identify visible and invisible
		/// collision issues.</summary>
		public static bool ShowVolumes { set { NativeAPI.ui_show_volumes(value); } }

		/// <summary>Enables or disables the far ray grab interaction for 
		/// Handle elements like the Windows. It can be enabled and disabled
		/// for individual UI elements, and if this remains disabled at the
		/// start of the next frame, then the hand ray indicators will not be
		/// visible. This is enabled by default. </summary>
		public static bool EnableFarInteract { get => NativeAPI.ui_far_interact_enabled(); set { NativeAPI.ui_enable_far_interact(value); } }

		/// <summary>This is the height of a single line of text with padding in the UI's layout system!</summary>
		public static float LineHeight => NativeAPI.ui_line_height();

		/// <summary>Use LayoutRemaining, removing in v0.4</summary>
		[Obsolete("Use LayoutRemaining, removing in v0.4")]
		public static Vec2 AreaRemaining => NativeAPI.ui_area_remaining();

		/// <summary>How much space is available on the current layout! This is
		/// based on the current layout position, so X will give you the amount
		/// remaining on the current line, and Y will give you distance to the
		/// bottom of the layout, including the current line. These values will
		/// be 0 if you're using 0 for the layout size on that axis.</summary>
		public static Vec2 LayoutRemaining => NativeAPI.ui_layout_remaining();

		/// <summary>The hierarchy local position of the current UI layout
		/// position. The top left point of the next UI element will be start
		/// here!</summary>
		public static Vec3 LayoutAt => NativeAPI.ui_layout_at();

		/// <summary>These are the layout bounds of the most recently reserved
		/// layout space. The Z axis dimensions are always 0. Only UI elements
		/// that affect the surface's layout will report their bounds here. You
		/// can reserve your own layout space via UI.LayoutReserve, and that
		/// call will also report here.</summary>
		public static Bounds LayoutLast => NativeAPI.ui_layout_last();

		/// <summary>Reserves a box of space for an item in the current UI
		/// layout! If either size axis is zero, it will be auto-sized to fill
		/// the current surface horizontally, and fill a single LineHeight
		/// vertically. Returns the Hierarchy local bounds of the space that
		/// was reserved, with a Z axis dimension of 0.</summary>
		/// <param name="size">Size of the layout box in Hierarchy local
		/// meters.</param>
		/// <param name="addPadding">If true, this will add the current padding
		/// value to the total final dimensions of the space that is reserved.
		/// </param>
		/// <param name="depth">This allows you to quickly insert a depth into
		/// the Bounds you're receiving. This will offset on the Z axis in
		/// addition to increasing the dimensions, so that the bounds still
		/// remain sitting on the surface of the UI.
		/// 
		/// This depth value will not be reflected in the bounds provided by 
		/// LayouLast.</param>
		/// <returns>Returns the Hierarchy local bounds of the space that was
		/// reserved, with a Z axis dimension of 0.</returns>
		public static Bounds LayoutReserve(Vec2 size, bool addPadding = false, float depth = 0)
			=> NativeAPI.ui_layout_reserve(size, addPadding ? 1 : 0, depth);

		/// <summary>Tells if the hand was involved in the focus or active
		/// state of the most recent UI element using an id.</summary>
		/// <param name="hand">Which hand we're checking.</param>
		/// <returns>A BtnState that indicated the hand was "just active" this
		/// frame, is currently "active" or if it "just became inactive" this
		/// frame.</returns>
		public static BtnState LastElementHandUsed(Handed hand) => NativeAPI.ui_last_element_hand_used(hand);
		/// <summary>Tells the Active state of the most recent UI element that
		/// used an id.</summary>
		public static BtnState LastElementActive => NativeAPI.ui_last_element_active();
		/// <summary>Tells the Focused state of the most recent UI element that
		/// used an id.</summary>
		public static BtnState LastElementFocused => NativeAPI.ui_last_element_focused();

		/// <summary>Tells if the user is currently interacting with a UI
		/// element! This will be true if the hand has an active or focused UI
		/// element.</summary>
		/// <param name="hand">Which hand is interacting?</param>
		/// <returns>True if the hand has an active or focused UI element.
		/// False otherwise.</returns>
		public static bool IsInteracting(Handed hand)
			=> NativeAPI.ui_is_interacting(hand);

		public static void SetThemeColor(UIColor colorType, Color colorGamma) => NativeAPI.ui_set_theme_color(colorType, colorGamma);
		public static Color GetThemeColor(UIColor colorType, Color colorGamma) => NativeAPI.ui_get_theme_color(colorType);

		/// <summary>This will push a surface into SK's UI layout system. The
		/// surface becomes part of the transform hierarchy, and SK creates a
		/// layout surface for UI content to be placed on and interacted with.
		/// Must be accompanied by a PopSurface call.</summary>
		/// <param name="surfacePose">The Pose of the UI surface, where the
		/// surface forward direction is the same as the Pose's.</param>
		/// <param name="layoutStart">This is an offset from the center of the
		/// coordinate space created by the surfacePose. Vec3.Zero would mean
		/// that content starts at the center of the surfacePose.</param>
		/// <param name="layoutDimensions">The size of the surface area to use
		/// during layout. Like other UI layout sizes, an axis set to zero
		/// means it will auto-expand in that direction.</param>
		public static void PushSurface(Pose surfacePose, Vec3 layoutStart = new Vec3(), Vec2 layoutDimensions = new Vec2())
			=> NativeAPI.ui_push_surface(surfacePose, layoutStart, layoutDimensions);

		/// <summary>This will return to the previous UI layout on the stack.
		/// This must be called after a PushSurface call.</summary>
		public static void PopSurface()
			=> NativeAPI.ui_pop_surface();

		/// <summary>Manually define what area is used for the UI layout. This
		/// is in the current Hierarchy's coordinate space on the X/Y plane.
		/// </summary>
		/// <param name="start">The top left of the layout area, relative to
		/// the current Hierarchy in local meters.</param>
		/// <param name="dimensions">The size of the layout area from the top
		/// left, in local meters.</param>
		public static void LayoutArea(Vec3 start, Vec2 dimensions)
			=> NativeAPI.ui_layout_area(start, dimensions);

		/// <summary>Use LayoutReserve, removing in v0.4</summary>
		[Obsolete("Use LayoutReserve, removing in v0.4")]
		public static void ReserveBox(Vec2 size) 
			=> NativeAPI.ui_layout_reserve(size);

		/// <summary>Moves the current layout position back to the end of the
		/// line that just finished, so it can continue on the same line as the
		/// last element!</summary>
		public static void SameLine() 
			=> NativeAPI.ui_sameline();

		/// <summary>This will advance the layout to the next line. If there's
		/// nothing on the current line, it'll advance to the start of the next
		/// on. But this won't have any affect on an empty line, try UI.Space
		/// for that.</summary>
		public static void NextLine() 
			=> NativeAPI.ui_nextline();

		/// <summary>Adds some space! If we're at the start of a new line,
		/// space is added vertically, otherwise, space is added
		/// horizontally.</summary>
		/// <param name="space">Physical space to shift the layout by.</param>
		public static void Space (float space) 
			=> NativeAPI.ui_space(space);

		[Obsolete("This overload will be removed in v0.4, prefer any other overload of this method.")]
		public static bool VolumeAt(string id, Bounds bounds)
			=> NativeAPI.ui_volume_at_16(id, bounds);

		/// <inheritdoc cref="VolumeAt(string, Bounds, UIConfirm)"/>
		/// <param name="hand">This will be the last unpreoccupied hand found
		/// inside the volume, and is the hand controlling the interaction.
		/// </param>
		/// <param name="focusState">The focus state tells if the element has
		/// a hand inside of the volume that qualifies for focus.</param>
		public static BtnState VolumeAt(string id, Bounds bounds, UIConfirm interactType, out Handed hand, out BtnState focusState)
			=> NativeAPI.ui_volumei_at_16(id, bounds, interactType, out hand, out focusState);

		/// <inheritdoc cref="VolumeAt(string, Bounds, UIConfirm)"/>
		/// <param name="hand">This will be the last unpreoccupied hand found
		/// inside the volume, and is the hand controlling the interaction.
		/// </param>
		public static BtnState VolumeAt(string id, Bounds bounds, UIConfirm interactType, out Handed hand)
			=> NativeAPI.ui_volumei_at_16(id, bounds, interactType, out hand, IntPtr.Zero);
		/// <summary>A volume for helping to build one handed interactions.
		/// This checks for the presence of a hand inside the bounds, and if
		/// found, return that hand along with activation and focus 
		/// information defined by the interactType.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="bounds">Size and position of the volume, relative to
		/// the current Hierarchy.</param>
		/// <param name="interactType">UIConfirm.Pinch will activate when the
		/// hand performs a 'pinch' gesture. UIConfirm.Push will activate 
		/// when the hand enters the volume, and behave the same as element's
		/// focusState.</param>
		/// <returns>Based on the interactType, this is a BtnState that tells
		/// the activation state of the interaction.</returns>
		public static BtnState VolumeAt(string id, Bounds bounds, UIConfirm interactType)
			=> NativeAPI.ui_volumei_at_16(id, bounds, interactType, IntPtr.Zero, IntPtr.Zero);

		/// <summary>This method will be removed in v0.4, use UI.VolumeAt. 
		/// 
		/// This watches a volume of space for pinch interaction 
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
		[Obsolete("This method will be removed in v0.4, use UI.VolumeAt.")]
		public static BtnState InteractVolume(Bounds bounds, out Handed hand)
			=> NativeAPI.ui_interact_volume_at(bounds, out hand);

		/// <summary>This draws a line horizontally across the current
		/// layout. Makes a good separator between sections of UI!</summary>
		public static void HSeparator()
			=> NativeAPI.ui_hseparator();

		/// <summary>Adds some text to the layout! Text uses the UI's current
		/// font settings, which can be changed with UI.Push/PopTextStyle. Can
		/// contain newlines!</summary>
		/// <param name="text">Label text to display. Can contain newlines!
		/// Doesn't use text as id, so it can be non-unique.</param>
		/// <param name="usePadding">Should padding be included for
		/// positioning this text? Sometimes you just want un-padded text!
		/// </param>
		public static void Label (string text, bool usePadding = true) 
			=> NativeAPI.ui_label_16(text, usePadding);

		/// <summary>Adds some text to the layout, but this overload allows you
		/// can specify the size that you want it to use. Text uses the UI's
		/// current font settings, which can be changed with
		/// UI.Push/PopTextStyle. Can contain newlines!</summary>
		/// <param name="text">Label text to display. Can contain newlines!
		/// Doesn't use text as id, so it can be non-unique.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static void Label(string text, Vec2 size)
			=> NativeAPI.ui_label_sz_16(text, size);

		/// <summary>Displays a large chunk of text on the current layout.
		/// This can include new lines and spaces, and will properly wrap
		/// once it fills the entire layout! Text uses the UI's current font
		/// settings, which can be changed with UI.Push/PopTextStyle.</summary>
		/// <param name="text">The text you wish to display, there's no 
		/// additional parsing done to this text, so put it in as you want to
		/// see it!</param>
		/// <param name="textAlign">Where should the text position itself
		/// within its bounds? TextAlign.TopLeft is how most English text is
		/// aligned.</param>
		public static void Text(string text, TextAlign textAlign = TextAlign.TopLeft)
			=> NativeAPI.ui_text_16(text, textAlign);

		/// <summary>Adds an image to the UI!</summary>
		/// <param name="image">A valid sprite.</param>
		/// <param name="size">Size in Hierarchy local meters. If one of the
		/// components is 0, it'll be automatically determined from the other
		/// component and the image's aspect ratio.</param>
		public static void Image (Sprite image, Vec2 size) 
			=> NativeAPI.ui_image(image._inst, size);

		/// <summary>A pressable button! A button will expand to fit the text
		/// provided to it, vertically and horizontally. Text is re-used as the
		/// id. Will return true only on the first frame it is pressed!
		/// </summary>
		/// <param name="text">Text to display on the button and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool Button (string text) 
			=> NativeAPI.ui_button_16(text);

		/// <inheritdoc cref="Button(string)"/>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static bool Button(string text, Vec2 size)
			=> NativeAPI.ui_button_sz_16(text, size);

		/// <summary>A variant of UI.Button that doesn't use the layout system,
		/// and instead goes exactly where you put it.</summary>
		/// <param name="text">Text to display on the button and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool ButtonAt(string text, Vec3 topLeftCorner, Vec2 size)
			=> NativeAPI.ui_button_at_16(text, topLeftCorner, size);

		/// <summary>A pressable button accompanied by an image! The button
		/// will expand to fit the text provided to it, horizontally. Text is
		/// re-used as the id. Will return true only on the first frame it is
		/// pressed! </summary>
		/// <param name="text">Text to display on the button and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="image">This is the image that will be drawn along with
		/// the text. See imageLayout for where the image gets drawn!</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the button. For example, `UIBtnLayout.Left`
		/// will have the image on the left, and text on the right.</param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool ButtonImg(string text, Sprite image, UIBtnLayout imageLayout = UIBtnLayout.Left)
			=> NativeAPI.ui_button_img_16(text, image._inst, imageLayout);

		/// <inheritdoc cref="ButtonImg(string,Sprite,UIBtnLayout)"/>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static bool ButtonImg(string text, Sprite image, UIBtnLayout imageLayout, Vec2 size)
			=> NativeAPI.ui_button_img_sz_16(text, image._inst, imageLayout, size);

		/// <summary>A variant of UI.ButtonImg that doesn't use the layout
		/// system, and instead goes exactly where you put it.</summary>
		/// <param name="text">Text to display on the button and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="image">This is the image that will be drawn along with
		/// the text. See imageLayout for where the image gets drawn!</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the button. For example, `UIBtnLayout.Left`
		/// will have the image on the left, and text on the right.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool ButtonImgAt(string text, Sprite image, UIBtnLayout imageLayout, Vec3 topLeftCorner, Vec2 size)
			=> NativeAPI.ui_button_img_sz_16(text, image._inst, imageLayout, size);

		/// <summary>A Radio is similar to a button, except you can specify if
		/// it looks pressed or not regardless of interaction. This can be
		/// useful for radio-like behavior! Check an enum for a value, and use
		/// that as the 'active' state, Then switch to that enum value if Radio
		/// returns true.</summary>
		/// <param name="text">Text to display on the Radio and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="active">Does this button look like it's pressed?</param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool Radio(string text, bool active)
		{
			int iActive = active?1:0;
			return NativeAPI.ui_toggle_16(text, ref iActive) && iActive>0;
		}

		/// <inheritdoc cref="Radio(string, bool)"/>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static bool Radio(string text, bool active, Vec2 size)
		{
			int iActive = active ? 1 : 0;
			return NativeAPI.ui_toggle_sz_16(text, ref iActive, size) && iActive>0;
		}

		/// <summary>A pressable button! A button will expand to fit the text
		/// provided to it, vertically and horizontally. Text is re-used as the
		/// id. Will return true only on the first frame it is pressed!
		/// </summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="image">An image to display as the face of the button.
		/// </param>
		/// <param name="diameter">The diameter of the button's visual.</param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool ButtonRound(string id, Sprite image, float diameter = 0)
			=> NativeAPI.ui_button_round_16(id, image._inst, diameter);

		/// <summary>A variant of UI.ButtonRound that doesn't use the layout
		/// system, and instead goes exactly where you put it.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="image">An image to display as the face of the button.
		/// </param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="diameter">The diameter of the button's visual.</param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool ButtonRoundAt(string id, Sprite image, Vec3 topLeftCorner, float diameter)
			=> NativeAPI.ui_button_round_at_16(id, image._inst, topLeftCorner, diameter);

		/// <summary>A toggleable button! A button will expand to fit the
		/// text provided to it, vertically and horizontally. Text is re-used 
		/// as the id. Will return true any time the toggle value changes, NOT
		/// the toggle value itself!
		/// </summary>
		/// <param name="text">Text to display on the Toggle and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="value">The current state of the toggle button! True 
		/// means it's toggled on, and false means it's toggled off.</param>
		/// <returns>Will return true any time the toggle value changes, NOT
		/// the toggle value itself!</returns>
		public static bool Toggle (string text, ref bool value)
		{
			int iVal = value?1:0;
			if (NativeAPI.ui_toggle_16(text, ref iVal))
			{
				value = iVal>0?true:false;
				return true;
			}
			return false;
		}

		/// <inheritdoc cref="Toggle(string, ref bool)"/>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static bool Toggle(string text, ref bool value, Vec2 size)
		{
			int iVal = value?1:0;
			if (NativeAPI.ui_toggle_sz_16(text, ref iVal, size))
			{
				value = iVal>0?true:false;
				return true;
			}
			return false;
		}

		/// <summary>A variant of UI.Toggle that doesn't use the layout system,
		/// and instead goes exactly where you put it.</summary>
		/// <param name="text">Text to display on the Toggle and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		/// <returns>Will return true any time the toggle value changes, NOT
		/// the toggle value itself!</returns>
		public static bool ToggleAt(string text, ref bool value, Vec3 topLeftCorner, Vec2 size)
		{
			int iVal = value ? 1 : 0;
			if (NativeAPI.ui_toggle_at_16(text, ref iVal, topLeftCorner, size))
			{
				value = iVal > 0 ? true : false;
				return true;
			}
			return false;
		}

		public static void Model(Model model)
			=> NativeAPI.ui_model(model._inst, Vec2.Zero, 0);
		public static void Model(Model model, Vec2 uiSize, float modelScale = 0)
			=> NativeAPI.ui_model(model._inst, uiSize, modelScale);

		/// <summary>This is an input field where users can input text to the
		/// app! Selecting it will spawn a virtual keyboard, or act as the
		/// keyboard focus. Hitting escape or enter, or focusing another UI
		/// element will remove focus from this Input.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The string that will store the Input's 
		/// content in.</param>
		/// <param name="size">Size of the Input in Hierarchy local meters.
		/// Zero axes will auto-size.</param>
		/// <param name="type">What category of text this Input represents.
		/// This may affect what kind of soft keyboard will be displayed, if
		/// one is shown to the user.</param>
		/// <returns>Returns true every time the contents of 'value' change.
		/// </returns>
		public static bool Input(string id, ref string value, Vec2 size = new Vec2(), TextContext type = TextContext.Text) {
			StringBuilder builder = value != null ? 
				new StringBuilder(value, value.Length + 16) :
				new StringBuilder(16);

			if (NativeAPI.ui_input_16(id, builder, builder.Capacity, size, type)) { 
				value = builder.ToString();
				return true;
			}
			return false;
		}

		/// <summary>This is a simple horizontal progress indicator bar. This
		/// is used by the HSlider to draw the slider bar beneath the
		/// interactive element. Does not include any text or label.</summary>
		/// <param name="percent">A value between 0 and 1 indicating progress
		/// from 0% to 100%.</param>
		/// <param name="width">Physical width of the slider on the window. 0
		/// will fill the remaining amount of window space.</param>
		public static void ProgressBar(float percent, float width = 0)
			=> NativeAPI.ui_progress_bar(percent, width);

		/// <summary>This is a simple horizontal progress indicator bar. This
		/// is used by the HSlider to draw the slider bar beneath the
		/// interactive element. Does not include any text or label.</summary>
		/// <param name="percent">A value between 0 and 1 indicating progress
		/// from 0% to 100%.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static void ProgressBarAt(float percent, Vec3 topLeftCorner, Vec2 size)
			=> NativeAPI.ui_progress_bar_at(percent, topLeftCorner, size);

		/// <summary>A horizontal slider element! You can stick your finger 
		/// in it, and slide the value up and down.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The value that the slider will store slider 
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, left side 
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, right 
		/// side of the slider.</param>
		/// <param name="step">Locks the value to intervals of step. Starts 
		/// at min, and increments by step.</param>
		/// <param name="width">Physical width of the slider on the window. 0
		/// will fill the remaining amount of window space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool HSlider(string id, ref float value, float min, float max, float step, float width = 0, UIConfirm confirmMethod = UIConfirm.Push) 
			=> NativeAPI.ui_hslider_16(id, ref value, min, max, step, width, confirmMethod);

		/// <summary>A horizontal slider element! You can stick your finger 
		/// in it, and slide the value up and down.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The value that the slider will store slider 
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, left side 
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, right 
		/// side of the slider.</param>
		/// <param name="step">Locks the value to intervals of step. Starts 
		/// at min, and increments by step.</param>
		/// <param name="width">Physical width of the slider on the window. 0
		/// will fill the remaining amount of window space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool HSlider(string id, ref double value, double min, double max, double step, float width = 0, UIConfirm confirmMethod = UIConfirm.Push)
			=> NativeAPI.ui_hslider_f64_16(id, ref value, min, max, step, width, confirmMethod);

		/// <summary>A variant of UI.HSlider that doesn't use the layout
		/// system, and instead goes exactly where you put it.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The value that the slider will store slider 
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, left side 
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, right 
		/// side of the slider.</param>
		/// <param name="step">Locks the value to intervals of step. Starts 
		/// at min, and increments by step.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool HSliderAt(string id, ref float value, float min, float max, float step, Vec3 topLeftCorner, Vec2 size, UIConfirm confirmMethod = UIConfirm.Push)
			=> NativeAPI.ui_hslider_at_16(id, ref value, min, max, step, topLeftCorner, size, confirmMethod);

		/// <summary>A variant of UI.HSlider that doesn't use the layout
		/// system, and instead goes exactly where you put it.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The value that the slider will store slider 
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, left side 
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, right 
		/// side of the slider.</param>
		/// <param name="step">Locks the value to intervals of step. Starts 
		/// at min, and increments by step.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool HSliderAt(string id, ref double value, double min, double max, double step, Vec3 topLeftCorner, Vec2 size, UIConfirm confirmMethod = UIConfirm.Push)
			=> NativeAPI.ui_hslider_at_f64_16(id, ref value, min, max, step, topLeftCorner, size, confirmMethod);

		/// <summary>This begins a new UI group with its own layout! Much 
		/// like a window, except with a more flexible handle, and no header.
		/// You can draw the handle, but it will have no text on it.
		/// The pose value is always relative to the current hierarchy stack.
		/// This call will also push the pose transform onto the hierarchy stack, so
		/// any objects drawn up to the corresponding UI.HandleEnd() will get transformed 
		/// by the handle pose. Returns true for every frame the user is grabbing the handle.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="pose">The pose state for the handle! The user will 
		/// be able to grab this handle and move it around. The pose is relative
		/// to the current hierarchy stack.</param>
		/// <param name="handle">Size and location of the handle, relative to 
		/// the pose.</param>
		/// <param name="drawHandle">Should this function draw the handle 
		/// visual for you, or will you draw that yourself?</param>
		/// <param name="moveType">Describes how the handle will move when 
		/// dragged around.</param>
		/// <returns>Returns true for every frame the user is grabbing the 
		/// handle.</returns>
		public static bool HandleBegin (string id, ref Pose pose, Bounds handle, bool drawHandle = false, UIMove moveType = UIMove.Exact)
			=> NativeAPI.ui_handle_begin_16(id, ref pose, handle, drawHandle?1:0, moveType);

		/// <summary>Finishes a handle! Must be called after UI.HandleBegin()
		/// and all elements have been drawn. Pops the pose transform pushed
		/// by UI.HandleBegin() from the hierarchy stack.</summary>
		public static void HandleEnd   ()
			=> NativeAPI.ui_handle_end();

		/// <summary>This begins and ends a handle so you can just use  its 
		/// grabbable/moveable functionality! Behaves much like a window,
		/// except with a more flexible handle, and no header. You can draw 
		/// the handle, but it will have no text on it. Returns true for 
		/// every frame the user is grabbing the handle.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="pose">The pose state for the handle! The user will 
		/// be able to grab this handle and move it around. The pose is relative
		/// to the current hierarchy stack.</param>
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
			bool result = NativeAPI.ui_handle_begin_16(id, ref pose, handle, drawHandle?1:0, moveType);
			NativeAPI.ui_handle_end();
			return result;
		}

		/// <summary>Begins a new window! This will push a pose onto the 
		/// transform stack, and all UI elements will be relative to that new 
		/// pose. The pose is actually the top-center of the window. Must be 
		/// finished with a call to UI.WindowEnd().</summary>
		/// <param name="text">Text to display on the window title and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
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
		/// <param name="text">Text to display on the window title and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
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
		/// prevent id collisions in separate groups. </summary>
		/// <param name="rootId">The root id to use until the following PopId 
		/// call. MUST be unique within current hierarchy.</param>
		public static void PushId(string rootId) 
			=> NativeAPI.ui_push_id_16(rootId);

		/// <summary>Adds a root id to the stack for the following UI 
		/// elements! This id is combined when hashing any following ids, to
		/// prevent id collisions in separate groups.</summary>
		/// <param name="rootId">The root id to use until the following PopId 
		/// call. MUST be unique within current hierarchy.</param>
		public static void PushId(int rootId)
			=> NativeAPI.ui_push_idi(rootId);

		/// <summary>Removes the last root id from the stack, and moves up to 
		/// the one before it!</summary>
		public static void PopId() 
			=> NativeAPI.ui_pop_id();

		/// <summary>When a soft keyboard is visible, interacting with UI
		/// elements will cause the keyboard to close. This function allows you
		/// to change this behavior for certain UI elements, allowing the user
		/// to interact and still preserve the keyboard's presence. Remember
		/// to Pop when you're finished!</summary>
		/// <param name="preserveKeyboard">If true, interacting with elements
		/// will NOT hide the keyboard. If false, interaction will hide the
		/// keyboard.</param>
		public static void PushPreserveKeyboard(bool preserveKeyboard)
			=> NativeAPI.ui_push_preserve_keyboard(preserveKeyboard?1:0);

		/// <summary>This pops the keyboard presentation state to what it was
		/// previously.</summary>
		public static void PopPreserveKeyboard()
			=> NativeAPI.ui_pop_preserve_keyboard();

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

		/// <summary>All UI between PushTint and its matching PopTint will be
		/// tinted with this color. This is implemented by multiplying this
		/// color with the current color of the UI element. The default is a
		/// White (1,1,1,1) identity tint.</summary>
		/// <param name="colorGamma">A normal (gamma corrected) color value.
		/// This is internally converted to linear, so tint multiplication
		/// happens on linear color values.</param>
		public static void PushTint(Color colorGamma)
			=> NativeAPI.ui_push_tint(colorGamma);

		/// <summary>Removes a Tint from the stack, and whatever was below will
		/// then be used as the primary tint.</summary>
		public static void PopTint()
			=> NativeAPI.ui_pop_tint();

		/// <summary>All UI between PushEnabled and its matching PopEnabled
		/// will set the UI to an enabled or disabled state, allowing or
		/// preventing interaction with specific elements. The default state is
		/// true. This currently doesn't have any visual effect, so you may
		/// wish to pair it with a PushTint.</summary>
		/// <param name="enabled">Should the following elements be enabled and
		/// interactable?</param>
		public static void PushEnabled(bool enabled)
			=> NativeAPI.ui_push_enabled(enabled?1:0);

		/// <summary>Removes an 'enabled' state from the stack, and whatever
		/// was below will then be used as the primary enabled state.</summary>
		public static void PopEnabled()
			=> NativeAPI.ui_pop_enabled();

		/// <summary>If you wish to manually draw a Panel, this function will
		/// let you draw one wherever you want!</summary>
		/// <param name="start">The top left corner of the Panel element.</param>
		/// <param name="size">The size of the Panel element, in hierarchy
		/// local meters.</param>
		/// <param name="padding">Only UIPad.Outsize has any affect here.
		/// UIPad.Inside will behave the same as UIPad.None.</param>
		public static void PanelAt(Vec3 start, Vec2 size, UIPad padding = UIPad.Outside) => NativeAPI.ui_panel_at(start, size, padding);

		/// <summary>This will begin a Panel element that will encompass all 
		/// elements drawn between PanelBegin and PanelEnd. This is an entirely
		/// visual element, and is great for visually grouping elements
		/// together. Every Begin must have a matching End.</summary>
		/// <param name="padding">Describes how padding is applied to the
		/// visual element of the Panel.</param>
		public static void PanelBegin(UIPad padding = UIPad.Outside) => NativeAPI.ui_panel_begin(padding);
		
		/// <summary>This will finalize and draw a Panel element.</summary>
		public static void PanelEnd() => NativeAPI.ui_panel_end();

		/// <summary>Override the visual assets attached to a particular UI
		/// element. 
		/// 
		/// Note that StereoKit's default UI assets use a type of quadrant
		/// sizing that is implemented in the Material _and_ the Mesh. You
		/// don't need to use quadrant sizing for your own visuals, but if
		/// you wish to know more, you can read more about the technique
		/// [here](https://playdeck.net/blog/quadrant-sizing-efficient-ui-rendering).
		/// You may also find UI.QuadrantSizeVerts and UI.QuadrantSizeMesh to
		/// be helpful.</summary>
		/// <param name="visual">Which UI visual element to override.</param>
		/// <param name="mesh">The Mesh to use for the UI element's visual
		/// component. The Mesh will be scaled to match the dimensions of the
		/// UI element.</param>
		/// <param name="material">The Material to use when rendering the UI
		/// element. The default Material is specifically designed to work
		/// with quadrant sizing formatted meshes.</param>
		/// <param name="minSize">For some meshes, such as quadrant sized
		/// meshes, there's a minimum size where the mesh turns inside out.
		/// This lets UI elements to accommodate for this minimum size, and
		/// behave somewhat more appropriately.</param>
		public static void SetElementVisual(UIVisual visual, Mesh mesh, Material material = null, Vec2 minSize = default)
			=> NativeAPI.ui_set_element_visual(visual, mesh != null ? mesh._inst : IntPtr.Zero, material != null ? material._inst : IntPtr.Zero, Vec2.Zero);

		/// <summary>This will reposition the vertices to work well with
		/// quadrant resizing shaders. The mesh should generally be centered
		/// around the origin, and face down the -Z axis. This will also 
		/// overwrite any UV coordinates in the verts.
		/// 
		/// You can read more about the technique [here](https://playdeck.net/blog/quadrant-sizing-efficient-ui-rendering).</summary>
		/// <param name="verts">A list of vertices to be modified to fit the
		/// sizing shader.</param>
		/// <param name="overflowPercent">When scaled, should the geometry
		/// stick out past the "box" represented by the scale, or edge up
		/// against it? A value of 0 will mean the geometry will fit entirely
		/// inside the "box", and a value of 1 means the geometry will start at
		/// the boundary of the box and continue outside it.</param>
		public static void QuadrantSizeVerts(Vertex[] verts, float overflowPercent = 0)
			=> NativeAPI.ui_quadrant_size_verts(verts, verts.Length, overflowPercent);

		/// <summary>This will reposition the Mesh's vertices to work well with
		/// quadrant resizing shaders. The mesh should generally be centered
		/// around the origin, and face down the -Z axis. This will also 
		/// overwrite any UV coordinates in the verts.
		/// 
		/// You can read more about the technique [here](https://playdeck.net/blog/quadrant-sizing-efficient-ui-rendering).</summary>
		/// <param name="mesh">The vertices of this Mesh will be retrieved,
		/// modified, and overwritten.</param>
		/// <param name="overflowPercent">When scaled, should the geometry
		/// stick out past the "box" represented by the scale, or edge up
		/// against it? A value of 0 will mean the geometry will fit entirely
		/// inside the "box", and a value of 1 means the geometry will start at
		/// the boundary of the box and continue outside it.</param>
		public static void QuadrantSizeMesh(ref Mesh mesh, float overflowPercent = 0)
			=> NativeAPI.ui_quadrant_size_mesh(mesh._inst, overflowPercent);

		public static ulong StackHash(string id)
			=> NativeAPI.ui_stack_hash_16(id);

		public static void ButtonBehavior(Vec3 windowRelativePos, Vec2 size, string id, out float fingerOffset, out BtnState buttonState, out BtnState focusState)
			=> NativeAPI.ui_button_behavior(windowRelativePos, size, NativeAPI.ui_stack_hash_16(id), out fingerOffset, out buttonState, out focusState);
	}
}
