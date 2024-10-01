// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

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
		/// <summary>UI sizing and layout settings.</summary>
		public static UISettings Settings { get => NativeAPI.ui_get_settings(); set { NativeAPI.ui_settings(value); } }

		/// <summary>StereoKit will generate a color palette from this gamma
		/// space color, and use it to skin the UI! To explicitly adjust
		/// individual theme colors, see UI.SetThemeColor.</summary>
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

		/// <summary>This is the UIMove that is provided to UI windows that
		/// StereoKit itself manages, such as the fallback filepicker and
		/// soft keyboard.</summary>
		public static UIMove SystemMoveType { get => NativeAPI.ui_system_get_move_type(); set { NativeAPI.ui_system_set_move_type(value); } }

		/// <summary>This is the height of a single line of text with padding
		/// in the UI's layout system!</summary>
		public static float LineHeight => NativeAPI.ui_line_height();

		/// <summary>This returns the TextStyle that's on top of the UI's
		/// stack, according to UI.Push/PopTextStyle.</summary>
		public static TextStyle TextStyle => NativeAPI.ui_get_text_style();

		/// <summary>This returns the current state of the UI's enabled status
		/// stack, set by `UI.Push/PopEnabled`.</summary>
		public static bool Enabled => NativeAPI.ui_is_enabled();

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
			=> NativeAPI.ui_layout_reserve(size, addPadding, depth);

		/// <summary>Reserves a box of space for an item in the current UI
		/// layout! If either size axis is zero, it will be auto-sized to fill
		/// the current surface horizontally, and fill a single LineHeight
		/// vertically. Returns the Hierarchy local bounds of the space that
		/// was reserved, with a Z axis dimension of 0.</summary>
		/// <param name="width">Width of the layout box in Hierarchy local
		/// meters.</param>
		/// <param name="height">Height of the layout box in Hierarchy local
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
		public static Bounds LayoutReserve(float width, float height, bool addPadding = false, float depth = 0)
			=> NativeAPI.ui_layout_reserve(new Vec2(width, height), addPadding, depth);

		/// <summary>This pushes a layout rect onto the layout stack. All UI
		/// elements using the layout system will now exist inside this layout
		/// area! Note that some UI elements such as Windows will already be
		/// managing a layout of their own on the stack.</summary>
		/// <param name="start">The top left position of the layout. Note that
		/// Windows have their origin at the top center, the left side of a
		/// window is X+, and content advances to the X- direction.</param>
		/// <param name="dimensions">The total size of the layout area. A value
		/// of zero means the layout will expand in that axis, but may prevent
		/// certain types of layout "Cuts".</param>
		/// <param name="addMargin">Adds a spacing margin to the interior of
		/// the layout. Most of the time you won't need this, but may be useful
		/// when working without a Window.</param>
		public static void LayoutPush(Vec3 start, Vec2 dimensions, bool addMargin = false) => NativeAPI.ui_layout_push(start, dimensions, addMargin);
		/// <summary>This cuts off a portion of the current layout area, and
		/// pushes that new area onto the layout stack. Left and Top cuts will
		/// always work, but Right and Bottom cuts can only exist inside of a
		/// parent layout with an explicit size, auto-resizing prevents these
		/// cuts.
		/// All UI elements using the layout system will now exist inside this
		/// layout area! Note that some UI elements such as Windows will already be
		/// managing a layout of their own on the stack.</summary>
		/// <param name="cutTo">Which side of the current layout should the cut
		/// happen to? Note that Right and Bottom will require explicit sizes
		/// in the parent layout, not auto-sizes.</param>
		/// <param name="sizeMeters">The size of the layout cut, in meters.
		/// </param>
		/// <param name="addMargin">Adds a spacing margin to the interior of
		/// the layout. Most of the time you won't need this, but may be useful
		/// when working without a Window.</param>
		public static void LayoutPushCut(UICut cutTo, float sizeMeters, bool addMargin = false) => NativeAPI.ui_layout_push_cut(cutTo, sizeMeters, addMargin);
		/// <summary>This removes a layout from the layout stack that was
		/// previously added using LayoutPush, or LayoutPushCut.</summary>
		public static void LayoutPop() => NativeAPI.ui_layout_pop();

		/// <summary>Tells if the hand was involved in the active state of the
		/// most recently called UI element using an id. Active state is
		/// frequently a single frame in the case of Buttons, but could be many
		/// in the case of Sliders or Handles.</summary>
		/// <param name="hand">Which hand we're checking.</param>
		/// <returns>A BtnState that indicated the hand was "just active" this
		/// frame, is currently "active" or if it "just became inactive" this
		/// frame.</returns>
		public static BtnState LastElementHandActive(Handed hand) => NativeAPI.ui_last_element_hand_active(hand);
		/// <summary>Tells if the hand was involved in the focus state of the
		/// most recently called UI element using an id. Focus occurs when the
		/// hand is in or near an element, in such a way that indicates the
		/// user may be about to interact with it.</summary>
		/// <param name="hand">Which hand we're checking.</param>
		/// <returns>A BtnState that indicated the hand was "just focused" this
		/// frame, is currently "focused" or if it "just became focused" this
		/// frame.</returns>
		public static BtnState LastElementHandFocused(Handed hand) => NativeAPI.ui_last_element_hand_focused(hand);
		/// <summary>Tells the Active state of the most recently called UI
		/// element that used an id.</summary>
		public static BtnState LastElementActive => NativeAPI.ui_last_element_active();
		/// <summary>Tells the Focused state of the most recently called UI
		/// element that used an id.</summary>
		public static BtnState LastElementFocused => NativeAPI.ui_last_element_focused();

		/// <summary>Tells if the user is currently interacting with a UI
		/// element! This will be true if the hand has an active or focused UI
		/// element.</summary>
		/// <param name="hand">Which hand is interacting?</param>
		/// <returns>True if the hand has an active or focused UI element.
		/// False otherwise.</returns>
		public static bool IsInteracting(Handed hand)
			=> NativeAPI.ui_is_interacting(hand);

		/// <summary>This allows you to explicitly set a theme color, for finer
		/// grained control over the UI appearance. Each theme type is still
		/// used by many different UI elements. This will automatically
		/// generate colors for different UI element states.</summary>
		/// <param name="colorCategory">The category of UI elements that will
		/// be affected by this theme color. This can be a value _past_
		/// UIColor.Max if you need extra UIColor slots for your own custom UI
		/// elements.</param>
		/// <param name="colorGamma">The gamma corrected color that should be
		/// applied to this theme color category in its normal resting state.
		/// Active and disabled colors will be generated based on this color.
		/// </param>
		public static void SetThemeColor(UIColor colorCategory, Color colorGamma) => NativeAPI.ui_set_theme_color(colorCategory, colorGamma);
		/// <summary>This allows you to explicitly set a theme color, for finer
		/// grained control over the UI appearance. Each theme type is still
		/// used by many different UI elements. This applies specifically to
		/// one state of this color category, and does not modify the others.
		/// </summary>
		/// <param name="colorCategory">The category of UI elements that will
		/// be affected by this theme color. This can be a value _past_
		/// UIColor.Max if you need extra UIColor slots for your own custom UI
		/// elements.</param>
		/// <param name="colorState">The state of the UI element this color
		/// should apply to.</param>
		/// <param name="colorGamma">The gamma corrected color that should be
		/// applied to this theme color category in the indicated state.
		/// </param>
		public static void SetThemeColor(UIColor colorCategory, UIColorState colorState, Color colorGamma) => NativeAPI.ui_set_theme_color_state(colorCategory, colorState, colorGamma);
		/// <summary>This allows you to inspect the current normal color of the
		/// theme color category! If you set the color with UI.ColorScheme,
		/// this will be one of the generated colors, and not necessarily the
		/// color that was provided there.</summary>
		/// <param name="colorCategory">The category of UI elements that are
		/// affected by this theme color. This can be a value _past_
		/// UIColor.Max if you need extra UIColor slots for your own custom UI
		/// elements. If the theme slot is empty, the color will be pulled from
		/// UIColor.None.</param>
		/// <returns>The gamma space color for the theme color category in its
		/// normal state.</returns>
		public static Color GetThemeColor(UIColor colorCategory) => NativeAPI.ui_get_theme_color(colorCategory);
		/// <summary>This allows you to inspect the current color of the theme
		/// color category in a specific state! If you set the color with
		/// UI.ColorScheme, or without specifying a state, this may be a
		/// generated color, and not necessarily the color that was provided
		/// there.</summary>
		/// <param name="colorCategory">The category of UI elements that are
		/// affected by this theme color. This can be a value _past_
		/// UIColor.Max if you need extra UIColor slots for your own custom UI
		/// elements. If the theme slot is empty, the color will be pulled from
		/// UIColor.None.</param>
		/// <param name="colorState">The state of the UI element this color
		/// applies to.</param>
		/// <returns>The gamma space color for the theme color category in the
		/// indicated state.</returns>
		public static Color GetThemeColor(UIColor colorCategory, UIColorState colorState) => NativeAPI.ui_get_theme_color_state(colorCategory, colorState);

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
		public static void LayoutArea(Vec3 start, Vec2 dimensions, bool addMargin = true)
			=> NativeAPI.ui_layout_area(start, dimensions, addMargin);

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

		/// <summary>Adds some vertical space to the current line! All UI
		/// following elements on this line will be offset.</summary>
		/// <param name="verticalSpace">Space in meters to shift the layout by.
		/// </param>
		public static void VSpace(float verticalSpace)
			=> NativeAPI.ui_vspace(verticalSpace);

		/// <summary>Adds some horizontal space to the current line!</summary>
		/// <param name="horizontalSpace">Space in meters to shift the layout
		/// by.</param>
		public static void HSpace(float horizontalSpace)
			=> NativeAPI.ui_hspace(horizontalSpace);

		/// <inheritdoc cref="VolumeAt(string, Bounds, UIConfirm)"/>
		/// <param name="hand">This will be the last unpreoccupied hand found
		/// inside the volume, and is the hand controlling the interaction.
		/// </param>
		/// <param name="focusState">The focus state tells if the element has
		/// a hand inside of the volume that qualifies for focus.</param>
		public static BtnState VolumeAt(string id, Bounds bounds, UIConfirm interactType, out Handed hand, out BtnState focusState)
			=> NativeAPI.ui_volume_at_16(id, bounds, interactType, out hand, out focusState);

		/// <inheritdoc cref="VolumeAt(string, Bounds, UIConfirm)"/>
		/// <param name="hand">This will be the last unpreoccupied hand found
		/// inside the volume, and is the hand controlling the interaction.
		/// </param>
		public static BtnState VolumeAt(string id, Bounds bounds, UIConfirm interactType, out Handed hand)
			=> NativeAPI.ui_volume_at_16(id, bounds, interactType, out hand, IntPtr.Zero);
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
			=> NativeAPI.ui_volume_at_16(id, bounds, interactType, IntPtr.Zero, IntPtr.Zero);

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
		/// <param name="usePadding">Should padding be included for
		/// positioning this text? Sometimes you just want un-padded text!
		/// </param>
		public static void Label(string text, Vec2 size, bool usePadding = true)
			=> NativeAPI.ui_label_sz_16(text, size, usePadding);

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
			=> NativeAPI.ui_text_16(text, IntPtr.Zero, UIScroll.None, 0, textAlign, TextFit.Wrap);

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
		/// <param name="fit">Describe how the text should behave when one of
		/// its size dimensions conflicts with the provided 'size' parameter.
		/// `UI.Text` uses `TextFit.Wrap` by default.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static void Text(string text, TextAlign textAlign, TextFit fit, Vec2 size)
			=> NativeAPI.ui_text_sz_16(text, IntPtr.Zero, UIScroll.None, size, textAlign, fit);

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
		/// <param name="fit">Describe how the text should behave when one of
		/// its size dimensions conflicts with the provided 'size' parameter.
		/// `UI.Text` uses `TextFit.Wrap` by default.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		public static void TextAt(string text, TextAlign textAlign, TextFit fit, Vec3 topLeftCorner, Vec2 size)
			=> NativeAPI.ui_text_at_16(text, IntPtr.Zero, UIScroll.None, textAlign, fit, topLeftCorner, size);

		/// <summary>A scrolling text element! This is for reading large chunks
		/// of text that may be too long to fit in the available space. It
		/// requires a size, as well as a place to store the current scroll
		/// value. Text uses the UI's current font settings, which can be
		/// changed with UI.Push/PopTextStyle.</summary>
		/// <param name="text">The text you wish to display, there's no
		/// additional parsing done to this text, so put it in as you want to
		/// see it!</param>
		/// <param name="scroll">This is the current scroll value of the text,
		/// in meters, _not_ percent.</param>
		/// <param name="scrollDirection">What scroll bars are allowed to show
		/// on this text? Vertical, horizontal, both?</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <param name="textAlign">Where should the text position itself
		/// within its bounds? TextAlign.TopLeft is how most English text is
		/// aligned.</param>
		/// <param name="fit">Describe how the text should behave when one of
		/// its size dimensions conflicts with the provided 'size' parameter.
		/// `UI.Text` uses `TextFit.Wrap` by default, and this scrolling
		/// overload will always add `TextFit.Clip` internally.</param>
		/// <returns>Returns true if any of the scroll bars have changed this
		/// frame.</returns>
		public static bool Text(string text, ref Vec2 scroll, UIScroll scrollDirection, Vec2 size, TextAlign textAlign = TextAlign.TopLeft, TextFit fit = TextFit.Wrap)
			=> NativeAPI.ui_text_sz_16(text, ref scroll, scrollDirection, size, textAlign, fit);

		/// <inheritdoc cref="Text(string, ref Vec2, UIScroll, Vec2, TextAlign, TextFit)"/>
		/// <param name="height">The vertical height of this Text element,
		/// width will automatically take the remainder of the current layout
		/// width.</param>
		public static bool Text(string text, ref Vec2 scroll, UIScroll scrollDirection, float height, TextAlign textAlign = TextAlign.TopLeft, TextFit fit = TextFit.Wrap)
			=> NativeAPI.ui_text_16(text, ref scroll, scrollDirection, height, textAlign, fit);

		/// <inheritdoc cref="Text(string, ref Vec2, UIScroll, Vec2, TextAlign, TextFit)"/>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		public static bool TextAt(string text, ref Vec2 scroll, UIScroll scrollDirection, TextAlign textAlign, TextFit fit, Vec3 topLeftCorner, Vec2 size)
			=> NativeAPI.ui_text_at_16(text, ref scroll, scrollDirection, textAlign, fit, topLeftCorner, size);

		/// <summary>Adds an image to the UI!</summary>
		/// <param name="image">A valid sprite.</param>
		/// <param name="size">Size in Hierarchy local meters. If one of the
		/// components is 0, it'll be automatically determined from the other
		/// component and the image's aspect ratio.</param>
		public static void Image (Sprite image, Vec2 size) 
			=> NativeAPI.ui_image(image?._inst ?? IntPtr.Zero, size);

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
		/// space.</param>
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
			=> NativeAPI.ui_button_img_16(text, image?._inst ?? IntPtr.Zero, imageLayout, new Color(1,1,1,1));

		/// <summary>A pressable button accompanied by an image! The button
		/// will expand to fit the text provided to it, horizontally. Text is
		/// re-used as the id. Will return true only on the first frame it is
		/// pressed! Image can be tinted by passing a custom color</summary>
		/// <param name="text">Text to display on the button and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="image">This is the image that will be drawn along with
		/// the text. See imageLayout for where the image gets drawn!</param>
		/// <param name="imageTint">The Sprite's color will be multiplied by
		/// this tint. The default is White(1,1,1,1).</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the button. For example, `UIBtnLayout.Left`
		/// will have the image on the left, and text on the right.</param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool ButtonImg(string text, Sprite image, Color imageTint, UIBtnLayout imageLayout = UIBtnLayout.Left)
			=> NativeAPI.ui_button_img_16(text, image?._inst ?? IntPtr.Zero, imageLayout, imageTint);

		/// <inheritdoc cref="ButtonImg(string,Sprite,UIBtnLayout)"/>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static bool ButtonImg(string text, Sprite image, UIBtnLayout imageLayout, Vec2 size)
			=> NativeAPI.ui_button_img_sz_16(text, image?._inst ?? IntPtr.Zero, imageLayout, size, new Color(1,1,1,1));

		/// <inheritdoc cref="ButtonImg(string,Sprite,Color,UIBtnLayout)"/>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static bool ButtonImg(string text, Sprite image, Color imageTint, UIBtnLayout imageLayout, Vec2 size)
			=> NativeAPI.ui_button_img_sz_16(text, image?._inst ?? IntPtr.Zero, imageLayout, size, imageTint);

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
		/// space.</param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool ButtonImgAt(string text, Sprite image, UIBtnLayout imageLayout, Vec3 topLeftCorner, Vec2 size)
			=> NativeAPI.ui_button_img_at_16(text, image?._inst ?? IntPtr.Zero, imageLayout, topLeftCorner, size, new Color(1, 1, 1, 1));

		/// <inheritdoc cref="ButtonImgAt(string,Sprite,UIBtnLayout,Vec3,Vec2)"/>
		/// <param name="imageTint">The Sprite's color will be multiplied by
		/// this tint. The default is White(1,1,1,1).</param>
		public static bool ButtonImgAt(string text, Sprite image, Color imageTint, UIBtnLayout imageLayout, Vec3 topLeftCorner, Vec2 size)
			=> NativeAPI.ui_button_img_at_16(text, image?._inst ?? IntPtr.Zero, imageLayout, topLeftCorner, size, imageTint);

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
			// Android (only Android!) does something strange here in the most
			// straightforward implementation of this code. There may be some
			// issue with marshalling that prevents the ref value from
			// resolving before the condition is tested?
			//
			// So, this doesn't work:
			// NativeAPI.ui_toggle_img_16(text, ref active, Default.SpriteRadioOff._inst, Default.SpriteRadioOn._inst, UIBtnLayout.Left) && active;

			bool value = active;
			return NativeAPI.ui_toggle_img_16(text, ref value, Default.SpriteRadioOff._inst, Default.SpriteRadioOn._inst, UIBtnLayout.Left) && !active;
		}

		/// <inheritdoc cref="Radio(string, bool)"/>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static bool Radio(string text, bool active, Vec2 size)
		{
			bool value = active;
			return NativeAPI.ui_toggle_img_sz_16(text, ref value, Default.SpriteRadioOff._inst, Default.SpriteRadioOn._inst, UIBtnLayout.Left, size) && !active;
		}

		/// <summary>A Radio is similar to a button, except you can specify if
		/// it looks pressed or not regardless of interaction. This can be
		/// useful for radio-like behavior! Check an enum for a value, and use
		/// that as the 'active' state, Then switch to that enum value if Radio
		/// returns true. This version allows you to override the images used
		/// by the Radio.</summary>
		/// <param name="text">Text to display on the Radio and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="active">Does this button look like it's pressed?</param>
		/// <param name="imageOff">Image to use when the radio value is
		/// false.</param>
		/// <param name="imageOn">Image to use when the radio value is
		/// true.</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the radio. For example,
		/// `UIBtnLayout.Left` will have the image on the left, and text on the
		/// right.</param>
		/// <returns>Will return true only on the first frame it is pressed!
		/// </returns>
		public static bool Radio(string text, bool active, Sprite imageOff, Sprite imageOn, UIBtnLayout imageLayout = UIBtnLayout.Left)
		{
			bool value = active;
			return NativeAPI.ui_toggle_img_16(text, ref value, imageOff?._inst ?? IntPtr.Zero, imageOn?._inst ?? IntPtr.Zero, imageLayout) && !active;
		}

		/// <inheritdoc cref="Radio(string, bool, Sprite, Sprite, UIBtnLayout)"/>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static bool Radio(string text, bool active, Sprite imageOff, Sprite imageOn, UIBtnLayout imageLayout, Vec2 size)
		{
			bool value = active;
			return NativeAPI.ui_toggle_img_sz_16(text, ref value, imageOff?._inst ?? IntPtr.Zero, imageOn?._inst ?? IntPtr.Zero, imageLayout, size) && !active;
		}

		/// <inheritdoc cref="Radio(string, bool, Sprite, Sprite, UIBtnLayout)"/>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		public static bool RadioAt(string text, bool active, Sprite imageOff, Sprite imageOn, UIBtnLayout imageLayout, Vec3 topLeftCorner, Vec2 size)
		{
			bool value = active;
			return NativeAPI.ui_toggle_img_at_16(text, ref value, imageOff?._inst ?? IntPtr.Zero, imageOn?._inst ?? IntPtr.Zero, imageLayout, topLeftCorner, size) && !active;
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
			=> NativeAPI.ui_button_round_16(id, image?._inst ?? IntPtr.Zero, diameter);

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
			=> NativeAPI.ui_button_round_at_16(id, image?._inst ?? IntPtr.Zero, topLeftCorner, diameter);

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
			=> NativeAPI.ui_toggle_16(text, ref value);


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
		/// <param name="image">Image to use for the button, this will be used
		/// regardless of the toggle value.</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the button. For example, `UIBtnLayout.Left`
		/// will have the image on the left, and text on the right.</param>
		/// <returns>Will return true any time the toggle value changes, NOT
		/// the toggle value itself!</returns>
		public static bool Toggle(string text, ref bool value, Sprite image, UIBtnLayout imageLayout = UIBtnLayout.Left)
			=> Toggle(text, ref value, image, image, imageLayout);


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
		/// <param name="toggleOff">Image to use when the toggle value is
		/// false.</param>
		/// <param name="toggleOn">Image to use when the toggle value is
		/// true.</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the button. For example, `UIBtnLayout.Left`
		/// will have the image on the left, and text on the right.</param>
		/// <returns>Will return true any time the toggle value changes, NOT
		/// the toggle value itself!</returns>
		public static bool Toggle(string text, ref bool value, Sprite toggleOff, Sprite toggleOn, UIBtnLayout imageLayout = UIBtnLayout.Left)
			=> NativeAPI.ui_toggle_img_16(text, ref value, toggleOff?._inst ?? IntPtr.Zero, toggleOn?._inst ?? IntPtr.Zero, imageLayout);

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
		/// <param name="image">Image to use for the button, this will be used
		/// regardless of the toggle value.</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the button. For example, `UIBtnLayout.Left`
		/// will have the image on the left, and text on the right.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		/// <returns>Will return true any time the toggle value changes, NOT
		/// the toggle value itself!</returns>
		public static bool Toggle(string text, ref bool value, Sprite image, UIBtnLayout imageLayout, Vec2 size)
			=> Toggle(text, ref value, image, image, imageLayout, size);

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
		/// <param name="toggleOff">Image to use when the toggle value is
		/// false.</param>
		/// <param name="toggleOn">Image to use when the toggle value is
		/// true.</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the button. For example, `UIBtnLayout.Left`
		/// will have the image on the left, and text on the right.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		/// <returns>Will return true any time the toggle value changes, NOT
		/// the toggle value itself!</returns>
		public static bool Toggle(string text, ref bool value, Sprite toggleOff, Sprite toggleOn, UIBtnLayout imageLayout, Vec2 size)
			=> NativeAPI.ui_toggle_img_sz_16(text, ref value, toggleOff?._inst ?? IntPtr.Zero, toggleOn?._inst ?? IntPtr.Zero, imageLayout, size);

		/// <inheritdoc cref="Toggle(string, ref bool)"/>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space. If an axis is left as zero, it will be auto-calculated. For
		/// X this is the remaining width of the current layout, and for Y this
		/// is UI.LineHeight.</param>
		public static bool Toggle(string text, ref bool value, Vec2 size)
			=> NativeAPI.ui_toggle_sz_16(text, ref value, size);

		/// <summary>A variant of UI.Toggle that doesn't use the layout system,
		/// and instead goes exactly where you put it.</summary>
		/// <param name="text">Text to display on the Toggle and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="value">The current state of the toggle button! True 
		/// means it's toggled on, and false means it's toggled off.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <returns>Will return true any time the toggle value changes, NOT
		/// the toggle value itself!</returns>
		public static bool ToggleAt(string text, ref bool value, Vec3 topLeftCorner, Vec2 size)
			=> NativeAPI.ui_toggle_at_16(text, ref value, topLeftCorner, size);

		/// <summary>A variant of UI.Toggle that doesn't use the layout system,
		/// and instead goes exactly where you put it.</summary>
		/// <param name="text">Text to display on the Toggle and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="value">The current state of the toggle button! True 
		/// means it's toggled on, and false means it's toggled off.</param>
		/// <param name="image">Image to use for the button, this will be used
		/// regardless of the toggle value.</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the button. For example, `UIBtnLayout.Left`
		/// will have the image on the left, and text on the right.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <returns>Will return true any time the toggle value changes, NOT
		/// the toggle value itself!</returns>
		public static bool ToggleAt(string text, ref bool value, Sprite image, UIBtnLayout imageLayout, Vec3 topLeftCorner, Vec2 size)
			=> ToggleAt(text, ref value, image, image, imageLayout, topLeftCorner, size);

		/// <summary>A variant of UI.Toggle that doesn't use the layout system,
		/// and instead goes exactly where you put it.</summary>
		/// <param name="text">Text to display on the Toggle and id for
		/// tracking element state. MUST be unique within current hierarchy.
		/// </param>
		/// <param name="value">The current state of the toggle button! True 
		/// means it's toggled on, and false means it's toggled off.</param>
		/// <param name="toggleOff">Image to use when the toggle value is
		/// false.</param>
		/// <param name="toggleOn">Image to use when the toggle value is
		/// true.</param>
		/// <param name="imageLayout">This enum specifies how the text and
		/// image should be laid out on the button. For example, `UIBtnLayout.Left`
		/// will have the image on the left, and text on the right.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <returns>Will return true any time the toggle value changes, NOT
		/// the toggle value itself!</returns>
		public static bool ToggleAt(string text, ref bool value, Sprite toggleOff, Sprite toggleOn, UIBtnLayout imageLayout, Vec3 topLeftCorner, Vec2 size)
		=> NativeAPI.ui_toggle_img_at_16(text, ref value, toggleOff?._inst ?? IntPtr.Zero, toggleOn?._inst ?? IntPtr.Zero, imageLayout, topLeftCorner, size);

		/// <summary>This adds a non-interactive Model to the UI panel layout.
		/// </summary>
		/// <param name="model">The Model to use.</param>
		public static void Model(Model model)
			=> NativeAPI.ui_model(model?._inst ?? IntPtr.Zero, Vec2.Zero, 0);
		/// <summary>This adds a non-interactive Model to the UI panel layout,
		/// and allows you to specify its size.</summary>
		/// <param name="model">The Model to use.</param>
		/// <param name="uiSize">The size this element should take from the
		/// layout.</param>
		/// <param name="modelScale">0 will auto-scale the model to fit the
		/// layout space, but you can specify a different scale in case you'd
		/// like a different size.</param>
		public static void Model(Model model, Vec2 uiSize, float modelScale = 0)
			=> NativeAPI.ui_model(model?._inst ?? IntPtr.Zero, uiSize, modelScale);

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
			StringBuilder builder = value != null
				? new StringBuilder(value, value.Length + 16)
				: new StringBuilder(16);

			if (NativeAPI.ui_input_16(id, builder, builder.Capacity, size, type))
			{
				value = builder.ToString();
				return true;
			}
			return false;
		}

		/// <summary>This is an input field where users can input text to the
		/// app! Selecting it will spawn a virtual keyboard, or act as the
		/// keyboard focus. Hitting escape or enter, or focusing another UI
		/// element will remove focus from this Input.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The string that will store the Input's
		/// content in.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <param name="type">What category of text this Input represents.
		/// This may affect what kind of soft keyboard will be displayed, if
		/// one is shown to the user.</param>
		/// <returns>Returns true every time the contents of 'value' change.
		/// </returns>
		public static bool InputAt(string id, ref string value, Vec3 topLeftCorner, Vec2 size, TextContext type = TextContext.Text)
		{
			StringBuilder builder = value != null
				? new StringBuilder(value, value.Length + 16)
				: new StringBuilder(16);

			if (NativeAPI.ui_input_at_16(id, builder, builder.Capacity, topLeftCorner, size, type))
			{
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
		[Obsolete("Use HProgressBar instead")]
		public static void ProgressBar(float percent, float width = 0)
			=> HProgressBar(percent, width, false);

		/// <summary>This is a simple horizontal progress indicator bar. This
		/// is used by the HSlider to draw the slider bar beneath the
		/// interactive element. Does not include any text or label.</summary>
		/// <param name="percent">A value between 0 and 1 indicating progress
		/// from 0% to 100%.</param>
		/// <param name="width">Physical width of the slider on the window. 0
		/// will fill the remaining amount of window space.</param>
		/// <param name="flipFillDirection">By default, this fills from left to
		/// right. This allows you to flip the fill direction to right to left.
		/// </param>
		public static void HProgressBar(float percent, float width = 0, bool flipFillDirection = false)
			=> NativeAPI.ui_hprogress_bar(percent, width, flipFillDirection);

		/// <summary>This is a simple vertical progress indicator bar. This
		/// is used by the VSlider to draw the slider bar beneath the
		/// interactive element. Does not include any text or label.</summary>
		/// <param name="percent">A value between 0 and 1 indicating progress
		/// from 0% to 100%.</param>
		/// <param name="height">Physical height of the slider on the window. 0
		/// will fill the remaining amount of window space.</param>
		/// <param name="flipFillDirection">By default, this fills from top to
		/// bottom. This allows you to flip the fill direction to bottom to 
		/// top.</param>
		public static void VProgressBar(float percent, float height = 0, bool flipFillDirection = false)
			=> NativeAPI.ui_vprogress_bar(percent, height, flipFillDirection);

		/// <summary>This is a simple horizontal progress indicator bar. This
		/// is used by the HSlider to draw the slider bar beneath the
		/// interactive element. Does not include any text or label.</summary>
		/// <param name="percent">A value between 0 and 1 indicating progress
		/// from 0% to 100%.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		public static void ProgressBarAt(float percent, Vec3 topLeftCorner, Vec2 size, UIDir barDirection = UIDir.Horizontal, bool flipFillDirection = false)
			=> NativeAPI.ui_progress_bar_at(percent, topLeftCorner, size, barDirection, flipFillDirection);

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
		/// <param name="step">Locks the value to increments of step. Starts
		/// at min, and increments by step. 0 is valid, and means "don't lock
		/// to increments".</param>
		/// <param name="width">Physical width of the slider on the window. 0
		/// will fill the remaining amount of window space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <param name="notifyOn">Allows you to modify the behavior of the
		/// return value.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool HSlider(string id, ref float value, float min, float max, float step = 0, float width = 0, UIConfirm confirmMethod = UIConfirm.Push, UINotify notifyOn = UINotify.Change) 
			=> NativeAPI.ui_hslider_16(id, ref value, min, max, step, width, confirmMethod, notifyOn);

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
		/// <param name="step">Locks the value to increments of step. Starts
		/// at min, and increments by step. 0 is valid, and means "don't lock
		/// to increments".</param>
		/// <param name="width">Physical width of the slider on the window. 0
		/// will fill the remaining amount of window space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <param name="notifyOn">Allows you to modify the behavior of the
		/// return value.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool HSlider(string id, ref double value, double min, double max, double step = 0, float width = 0, UIConfirm confirmMethod = UIConfirm.Push, UINotify notifyOn = UINotify.Change)
			=> NativeAPI.ui_hslider_f64_16(id, ref value, min, max, step, width, confirmMethod, notifyOn);

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
		/// <param name="step">Locks the value to increments of step. Starts
		/// at min, and increments by step. 0 is valid, and means "don't lock
		/// to increments".</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <param name="notifyOn">Allows you to modify the behavior of the
		/// return value.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool HSliderAt(string id, ref float value, float min, float max, float step, Vec3 topLeftCorner, Vec2 size, UIConfirm confirmMethod = UIConfirm.Push, UINotify notifyOn = UINotify.Change)
			=> NativeAPI.ui_hslider_at_16(id, ref value, min, max, step, topLeftCorner, size, confirmMethod, notifyOn);

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
		/// <param name="step">Locks the value to increments of step. Starts
		/// at min, and increments by step. 0 is valid, and means "don't lock
		/// to increments".</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <param name="notifyOn">Allows you to modify the behavior of the
		/// return value.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool HSliderAt(string id, ref double value, double min, double max, double step, Vec3 topLeftCorner, Vec2 size, UIConfirm confirmMethod = UIConfirm.Push, UINotify notifyOn = UINotify.Change)
			=> NativeAPI.ui_hslider_at_f64_16(id, ref value, min, max, step, topLeftCorner, size, confirmMethod, notifyOn);

		/// <summary>A vertical slider element! You can stick your finger
		/// in it, and slide the value up and down.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The value that the slider will store slider
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, top side
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, bottom
		/// side of the slider.</param>
		/// <param name="step">Locks the value to increments of step. Starts
		/// at min, and increments by step. 0 is valid, and means "don't lock
		/// to increments".</param>
		/// <param name="height">Physical width of the slider on the window. 0
		/// will fill the remaining amount of window space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <param name="notifyOn">Allows you to modify the behavior of the
		/// return value.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool VSlider(string id, ref float value, float min, float max, float step = 0, float height = 0, UIConfirm confirmMethod = UIConfirm.Push, UINotify notifyOn = UINotify.Change) 
			=> NativeAPI.ui_vslider_16(id, ref value, min, max, step, height, confirmMethod, notifyOn);

		/// <summary>A vertical slider element! You can stick your finger
		/// in it, and slide the value up and down.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The value that the slider will store slider
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, top side
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, bottom
		/// side of the slider.</param>
		/// <param name="step">Locks the value to increments of step. Starts
		/// at min, and increments by step. 0 is valid, and means "don't lock
		/// to increments".</param>
		/// <param name="height">Physical height of the slider on the window. 0
		/// will fill the remaining amount of window space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <param name="notifyOn">Allows you to modify the behavior of the
		/// return value.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool VSlider(string id, ref double value, double min, double max, double step = 0, float height = 0, UIConfirm confirmMethod = UIConfirm.Push, UINotify notifyOn = UINotify.Change)
			=> NativeAPI.ui_vslider_f64_16(id, ref value, min, max, step, height, confirmMethod, notifyOn);

		/// <summary>A variant of UI.VSlider that doesn't use the layout
		/// system, and instead goes exactly where you put it.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The value that the slider will store slider
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, top side
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, bottom
		/// side of the slider.</param>
		/// <param name="step">Locks the value to increments of step. Starts
		/// at min, and increments by step. 0 is valid, and means "don't lock
		/// to increments".</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <param name="notifyOn">Allows you to modify the behavior of the
		/// return value.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool VSliderAt(string id, ref float value, float min, float max, float step, Vec3 topLeftCorner, Vec2 size, UIConfirm confirmMethod = UIConfirm.Push, UINotify notifyOn = UINotify.Change)
			=> NativeAPI.ui_vslider_at_16(id, ref value, min, max, step, topLeftCorner, size, confirmMethod, notifyOn);

		/// <summary>A variant of UI.VSlider that doesn't use the layout
		/// system, and instead goes exactly where you put it.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="value">The value that the slider will store slider
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, top side
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, bottom
		/// side of the slider.</param>
		/// <param name="step">Locks the value to intervals of step. Starts
		/// at min, and increments by step.</param>
		/// <param name="topLeftCorner">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <param name="notifyOn">Allows you to modify the behavior of the
		/// return value.</param>
		/// <returns>Returns true any time the value changes.</returns>
		public static bool VSliderAt(string id, ref double value, double min, double max, double step, Vec3 topLeftCorner, Vec2 size, UIConfirm confirmMethod = UIConfirm.Push, UINotify notifyOn = UINotify.Change)
			=> NativeAPI.ui_vslider_at_f64_16(id, ref value, min, max, step, topLeftCorner, size, confirmMethod, notifyOn);

		/// <summary>This begins a new UI group with its own layout! Much like
		/// a window, except with a more flexible handle, and no header. You
		/// can draw the handle, but it will have no text on it. The pose value
		/// is always relative to the current hierarchy stack. This call will
		/// also push the pose transform onto the hierarchy stack, so any
		/// objects drawn up to the corresponding UI.HandleEnd() will get
		/// transformed by the handle pose. Returns true for every frame the
		/// user is grabbing the handle.</summary>
		/// <param name="id">An id for tracking element state. MUST be unique
		/// within current hierarchy.</param>
		/// <param name="pose">The pose state for the handle! The user will be
		/// able to grab this handle and move it around. The pose is relative
		/// to the current hierarchy stack.</param>
		/// <param name="handle">Size and location of the handle, relative to
		/// the pose.</param>
		/// <param name="drawHandle">Should this function draw the handle
		/// visual for you, or will you draw that yourself?</param>
		/// <param name="moveType">Describes how the handle will move when
		/// dragged around.</param>
		/// <param name="allowedGestures">Which hand gestures are used for
		/// interacting with this Handle?</param>
		/// <returns>Returns true for every frame the user is grabbing the
		/// handle.</returns>
		public static bool HandleBegin (string id, ref Pose pose, Bounds handle, bool drawHandle = false, UIMove moveType = UIMove.Exact, UIGesture allowedGestures = UIGesture.Pinch)
			=> NativeAPI.ui_handle_begin_16(id, ref pose, handle, drawHandle, moveType, allowedGestures);

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
		/// <param name="allowedGestures">Which hand gestures are used for
		/// interacting with this Handle?</param>
		/// <returns>Returns true for every frame the user is grabbing the 
		/// handle.</returns>
		public static bool Handle(string id, ref Pose pose, Bounds handle, bool drawHandle = false, UIMove moveType = UIMove.Exact, UIGesture allowedGestures = UIGesture.Pinch)
		{
			bool result = NativeAPI.ui_handle_begin_16(id, ref pose, handle, drawHandle, moveType, allowedGestures);
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
			=> NativeAPI.ui_push_preserve_keyboard(preserveKeyboard);

		/// <summary>This pops the keyboard presentation state to what it was
		/// previously.</summary>
		public static void PopPreserveKeyboard()
			=> NativeAPI.ui_pop_preserve_keyboard();

		/// <summary>This pushes an enabled status for grab auras onto the
		/// stack. Grab auras are an extra space and visual element that goes
		/// around Window elements to make them easier to grab. MUST be matched
		/// by a PopGrabAura call.</summary>
		/// <param name="enabled">Is the grab aura enabled or not?</param>
		public static void PushGrabAura(bool enabled)
			=> NativeAPI.ui_push_grab_aura(enabled);

		/// <summary>This removes an enabled status for grab auras from the
		/// stack, returning it to the state before the previous PushGrabAura
		/// call. Grab auras are an extra space and visual element that goes
		/// around Window elements to make them easier to grab.</summary>
		public static void PopGrabAura()
			=> NativeAPI.ui_pop_grab_aura();

		/// <summary>This retreives the top of the grab aura enablement stack,
		/// in case you need to know if the current window will have an aura.
		/// </summary>
		/// <returns>The enabled value at the top of the stack.</returns>
		public static bool GrabAuraEnabled()
			=> NativeAPI.ui_grab_aura_enabled();

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
		/// true.</summary>
		/// <param name="enabled">Should the following elements be enabled and
		/// interactable?</param>
		/// <param name="ignoreParent">Do we want to ignore or inherit the
		/// state of the current stack?</param>
		[Obsolete("Use override with HierarchyParent parameter")]
		public static void PushEnabled(bool enabled, bool ignoreParent)
			=> NativeAPI.ui_push_enabled(enabled, ignoreParent ? HierarchyParent.Ignore : HierarchyParent.Inherit);

		/// <summary>All UI between PushEnabled and its matching PopEnabled
		/// will set the UI to an enabled or disabled state, allowing or
		/// preventing interaction with specific elements. The default state is
		/// true.</summary>
		/// <param name="enabled">Should the following elements be enabled and
		/// interactable?</param>
		/// <param name="parentBehavior">Do we want to ignore or inherit the
		/// state of the current stack?</param>
		public static void PushEnabled(bool enabled, HierarchyParent parentBehavior = HierarchyParent.Inherit)
			=> NativeAPI.ui_push_enabled(enabled, parentBehavior);

		/// <summary>Removes an 'enabled' state from the stack, and whatever
		/// was below will then be used as the primary enabled state.</summary>
		public static void PopEnabled()
			=> NativeAPI.ui_pop_enabled();

		/// <summary>If you wish to manually draw a Panel, this function will
		/// let you draw one wherever you want!</summary>
		/// <param name="start">The top left corner of the Panel element.</param>
		/// <param name="size">The size of the Panel element, in hierarchy
		/// local meters.</param>
		/// <param name="padding">Only UIPad.Outsize has any effect here.
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
		/// <param name="visual">Which UI visual element to override. This can
		/// be a value _past_ UIVisual.Max if you need extra UIVisual slots for
		/// your own custom UI elements.</param>
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
			=> NativeAPI.ui_set_element_visual(visual, mesh?._inst ?? IntPtr.Zero, material?._inst ?? IntPtr.Zero, minSize);

		/// <summary>This allows you to override the color category that a UI
		/// element is assigned to.</summary>
		/// <param name="visual">The UI element type to set the color category
		/// of. This can be a value _past_ UIVisual.Max if you need extra
		/// UIVisual slots for your own custom UI elements.</param>
		/// <param name="colorCategory">The category of color to assign to this
		/// UI element. Use UI.SetThemeColor in combination with this to assign
		/// a specific color. This can be a value _past_ UIColor.Max if you
		/// need extra UIColor slots for your own custom UI elements.</param>
		public static void SetElementColor(UIVisual visual, UIColor colorCategory)
			=> NativeAPI.ui_set_element_color(visual, colorCategory);

		/// <summary>This sets the sound that a particulat UI element will make
		/// when you interact with it. One sound when the interaction starts,
		/// and one when it ends.</summary>
		/// <param name="visual">The UI element to apply the sounds to. This
		/// can be a value _past_ UIVisual.Max if you need extra UIVisual slots
		/// for your own custom UI elements.</param>
		/// <param name="activate">The sound made when the interaction begins.
		/// A null sound will fall back to the default sound.</param>
		/// <param name="deactivate">The sound made when the interaction ends.
		/// A null sound will fall back to the default sound.</param>
		public static void SetElementSound(UIVisual visual, Sound activate, Sound deactivate)
			=> NativeAPI.ui_set_element_sound(visual, activate?._inst ?? IntPtr.Zero, deactivate?._inst ?? IntPtr.Zero);

		/// <summary>This will draw a visual element from StereoKit's theming
		/// system, while paying attention to certain factors such as enabled/
		/// disabled, tinting and more.</summary>
		/// <param name="elementVisual">The element type to draw. This can
		/// be a value _past_ UIVisual.Max to use extra UIVisual slots for
		/// your own custom UI elements. If these slots are empty, SK will fall
		/// back to UIVisual.Default.</param>
		/// <param name="elementColor">If you wish to use the coloring from a
		/// different element, you can use this to override the theme color
		/// used when drawing. This can be a value _past_ UIVisual.Max to use
		/// extra UIVisual slots for your own custom UI elements. If these
		/// slots are empty, SK will fall back to UIVisual.Default.</param>
		/// <param name="start">This is the top left corner of the UI
		/// element relative to the current Hierarchy.</param>
		/// <param name="size">The layout size for this element in Hierarchy
		/// space.</param>
		/// <param name="focus">The amount of visual focus this element
		/// currently has, where 0 is unfocused, and 1 is active. You can
		/// acquire a good focus value from `UI.GetAnimFocus`.</param>
		public static void DrawElement(UIVisual elementVisual, UIVisual elementColor, Vec3 start, Vec3 size, float focus)
			=> NativeAPI.ui_draw_element_color(elementVisual, elementColor, start, size, focus);

		/// <inheritdoc cref="DrawElement(UIVisual, UIVisual, Vec3, Vec3, float)"/>
		public static void DrawElement(UIVisual elementVisual, Vec3 start, Vec3 size, float focus)
			=> NativeAPI.ui_draw_element(elementVisual, start, size, focus);

		/// <summary>This will get a final linear draw color for a particular
		/// UI element type with a particular focus value. This obeys the
		/// current hierarchy of tinting and enabled states.</summary>
		/// <param name="elementVisual">Get the color from this element type.
		/// This can be a value _past_ UIVisual.Max to use extra UIVisual slots
		/// for your own custom UI elements. If these slots are empty, SK will
		/// fall back to UIVisual.Default.</param>
		/// <param name="focus">The amount of visual focus this element
		/// currently has, where 0 is unfocused, and 1 is active. You can
		/// acquire a good focus value from `UI.GetAnimFocus`</param>
		/// <returns>A linear color good for tinting UI meshes.</returns>
		public static Color GetElementColor(UIVisual elementVisual, float focus)
			=> NativeAPI.ui_get_element_color(elementVisual, focus);

		/// <summary>This resolves a UI element with an ID and its current
		/// states into a nicely animated focus value.</summary>
		/// <param name="id">The hierarchical id of the UI element we're
		/// checking the focus of, this can be created with `UI.StackHash`.
		/// </param>
		/// <param name="focusState">The current focus state of the UI element.
		/// </param>
		/// <param name="activationState">The current activation status of the
		/// UI element.</param>
		/// <returns>A focus value in the realm of 0-1, where 0 is unfocused,
		/// and 1 is active.</returns>
		public static float GetAnimFocus(IdHash id, BtnState focusState, BtnState activationState)
			=> NativeAPI.ui_get_anim_focus(id, focusState, activationState);

		/// <summary>This creates a Pose that is friendly towards UI popup
		/// windows, or windows that are created due to some type of user
		/// interaction. The fallback file picker and soft keyboard both use
		/// this function to position themselves!</summary>
		/// <param name="shift">A positional shift from the default location,
		/// this is useful to account for the height of the window, and center
		/// or offset this pose. A value of [0,-0.1,0] may be a good starting
		/// point.</param>
		/// <returns>A pose between the UI or hand that is currently active,
		/// and the user's head. Good for popup windows.</returns>
		public static Pose PopupPose(Vec3 shift = default)
			=> NativeAPI.ui_popup_pose(shift);

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
			=> NativeAPI.ui_quadrant_size_mesh(mesh?._inst ?? IntPtr.Zero, overflowPercent);

		/// <summary>This generates a quadrantified mesh meant for UI buttons
		/// by sweeping a lathe over the rounded corners of a rectangle! Note
		/// that this mesh is quadrantified, so it requires special shaders to
		/// draw properly!</summary>
		/// <param name="roundedCorners">A bit-flag indicating which corners
		/// should be rounded, and which should be sharp!</param>
		/// <param name="cornerRadius">The radius of each rounded corner.</param>
		/// <param name="cornerResolution">How many slices/verts go into each corner?
		/// More is smoother, but more expensive to render.</param>
		/// <param name="deleteFlatSides">If two adjacent corners are sharp, should
		/// we skip connecting them with triangles? If this edge will always be
		/// covered, then deleting these faces may save you some performance.</param>
		/// <param name="quadrantify">Does this generate a mesh compatible with
		/// StereoKit's quadrant shader system, or is this just a traditional
		/// mesh? In most cases, this should be true, but UI elements such as
		/// the rounded button may be exceptions.</param>
		/// <param name="lathePts">The lathe points to sweep around the edge.</param>
		/// <returns>The final Mesh, ready for use in SK's theming system.</returns>
		public static Mesh GenQuadrantMesh(UICorner roundedCorners, float cornerRadius, uint cornerResolution, bool deleteFlatSides, bool quadrantify, params UILathePt[] lathePts)
		{
			IntPtr result = NativeAPI.ui_gen_quadrant_mesh(roundedCorners, cornerRadius, cornerResolution, deleteFlatSides, quadrantify, lathePts, lathePts.Length);
			return result != IntPtr.Zero ? new Mesh(result) : null;
		}

		/// <inheritdoc cref="GenQuadrantMesh(UICorner, float, uint, bool, bool, UILathePt[])"/>
		public static Mesh GenQuadrantMesh(UICorner roundedCorners, float cornerRadius, uint cornerResolution, bool deleteFlatSides, params UILathePt[] lathePts)
		{
			IntPtr result = NativeAPI.ui_gen_quadrant_mesh(roundedCorners, cornerRadius, cornerResolution, deleteFlatSides, true, lathePts, lathePts.Length);
			return result != IntPtr.Zero ? new Mesh(result) : null;
		}

		/// <summary>This will hash the given text based id into a hash for use
		/// with certain StereoKit UI functions. This includes the hash of the
		/// current id stack.</summary>
		/// <param name="id">Text to hash along with the current id stack.
		/// </param>
		/// <returns>An integer based hash id for use with SK UI.</returns>
		public static ulong StackHash(string id)
			=> NativeAPI.ui_stack_hash_16(id);

		/// <summary>This is the core functionality of StereoKit's buttons,
		/// without any of the rendering parts! If you're trying to create your
		/// own pressable UI elements, or do more extreme customization of the
		/// look and feel of UI elements, then this function will provide a lot
		/// of complex pressing functionality for you!</summary>
		/// <param name="windowRelativePos">The layout position of the
		/// pressable area.</param>
		/// <param name="size">The size of the pressable area.</param>
		/// <param name="id">The id for this pressable element to track its
		/// state with.</param>
		/// <param name="fingerOffset">This is the current distance of the
		/// finger, within the pressable volume, from the bottom of the button.
		/// </param>
		/// <param name="buttonState">This is the current frame's "active"
		/// state for the button.</param>
		/// <param name="focusState">This is the current frame's "focus" state
		/// for the button.</param>
		public static void ButtonBehavior(Vec3 windowRelativePos, Vec2 size, string id, out float fingerOffset, out BtnState buttonState, out BtnState focusState)
			=> NativeAPI.ui_button_behavior(windowRelativePos, size, NativeAPI.ui_stack_hash_16(id), out fingerOffset, out buttonState, out focusState, out _);

		/// <inheritdoc cref="ButtonBehavior(Vec3, Vec2, string, out float, out BtnState, out BtnState)"/>
		/// <param name="hand">Id of the hand that interacted with the button.
		/// This will be -1 if no interaction has occurred.</param>
		public static void ButtonBehavior(Vec3 windowRelativePos, Vec2 size, string id, out float fingerOffset, out BtnState buttonState, out BtnState focusState, out int hand)
			=> NativeAPI.ui_button_behavior(windowRelativePos, size, NativeAPI.ui_stack_hash_16(id), out fingerOffset, out buttonState, out focusState, out hand);

		/// <summary>This is the core functionality of StereoKit's buttons,
		/// without any of the rendering parts! If you're trying to create your
		/// own pressable UI elements, or do more extreme customization of the
		/// look and feel of UI elements, then this function will provide a lot
		/// of complex pressing functionality for you!
		/// This overload allows for customizing the depth of the button, which
		/// otherwise would use UISettings.depth for its values.</summary>
		/// <param name="windowRelativePos">The layout position of the
		/// pressable area.</param>
		/// <param name="size">The size of the pressable area.</param>
		/// <param name="id">The id for this pressable element to track its
		/// state with.</param>
		/// <param name="buttonDepth">This is the z axis depth of the pressable
		/// area.</param>
		/// <param name="buttonActivationDepth">This is the depth at which the
		/// button will activate. Normally this is 1/2 of buttonDepth.</param>
		/// <param name="fingerOffset">This is the current distance of the
		/// finger, within the pressable volume, from the bottom of the button.
		/// </param>
		/// <param name="buttonState">This is the current frame's "active"
		/// state for the button.</param>
		/// <param name="focusState">This is the current frame's "focus" state
		/// for the button.</param>
		/// <param name="hand">Id of the hand that interacted with the button.
		/// This will be -1 if no interaction has occurred.</param>
		public static void ButtonBehavior(Vec3 windowRelativePos, Vec2 size, string id, float buttonDepth, float buttonActivationDepth, out float fingerOffset, out BtnState buttonState, out BtnState focusState, out int hand)
			=> NativeAPI.ui_button_behavior_depth(windowRelativePos, size, NativeAPI.ui_stack_hash_16(id), buttonDepth, buttonActivationDepth, out fingerOffset, out buttonState, out focusState, out hand);

		/// <summary>This is the core functionality of StereoKit's slider
		/// elements, without any of the rendering parts! If you're trying to
		/// create your own sliding UI elements, or do more extreme
		/// customization of the look and feel of slider UI elements, then this
		/// function will provide a lot of complex pressing functionality for
		/// you!</summary>
		/// <param name="windowRelativePos">The layout position of the
		/// pressable area.</param>
		/// <param name="size">The size of the pressable area.</param>
		/// <param name="id">The id for this pressable element to track its
		/// state with.</param>
		/// <param name="value">The value that the slider will store slider
		/// state in.</param>
		/// <param name="min">The minimum value the slider can set, left side
		/// of the slider.</param>
		/// <param name="max">The maximum value the slider can set, right
		/// side of the slider.</param>
		/// <param name="buttonSizeVisual">This is the visual size of the
		/// element representing the touchable area of the slider. This is used
		/// to calculate the center of the button's placement without going
		/// outside the provided bounds.</param>
		/// <param name="buttonSizeInteract">The size of the interactive touch
		/// element of the slider. Set this to zero to use the entire area as a
		/// touchable surface.</param>
		/// <param name="confirmMethod">How should the slider be activated?
		/// Push will be a push-button the user must press first, and pinch
		/// will be a tab that the user must pinch and drag around.</param>
		/// <param name="data">This is data about the slider interaction, you
		/// can use this for visualizing the slider behavior, or reacting to
		/// its events.</param>
		public static void SliderBehavior(Vec3 windowRelativePos, Vec2 size, IdHash id, ref Vec2 value, Vec2 min, Vec2 max, Vec2 buttonSizeVisual, Vec2 buttonSizeInteract, UIConfirm confirmMethod, out UISliderData data)
			=> NativeAPI.ui_slider_behavior(windowRelativePos, size, id, ref value, min, max, buttonSizeVisual, buttonSizeInteract, confirmMethod, out data);
	}
}
