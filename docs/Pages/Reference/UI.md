---
layout: default
title: UI
description: This class is a collection of user interface and interaction methods! StereoKit uses an Immediate Mode GUI system, which can be very easy to work with and modify during runtime.  You must call the UI method every frame you wish it to be available, and if you no longer want it to be present, you simply stop calling it! The id of the element is used to track its state from frame to frame, so for elements with state, you'll want to avoid changing the id during runtime! Ids are also scoped per-window, so different windows can re-use the same id, but a window cannot use the same id twice.
---
# static class UI

This class is a collection of user interface and interaction
methods! StereoKit uses an Immediate Mode GUI system, which can be very
easy to work with and modify during runtime.

You must call the UI method every frame you wish it to be available,
and if you no longer want it to be present, you simply stop calling it!
The id of the element is used to track its state from frame to frame,
so for elements with state, you'll want to avoid changing the id during
runtime! Ids are also scoped per-window, so different windows can
re-use the same id, but a window cannot use the same id twice.

## Static Fields and Properties

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [AreaRemaining]({{site.url}}/Pages/Reference/UI/AreaRemaining.html)|Use LayoutRemaining, removing in v0.4|
|[Color]({{site.url}}/Pages/Reference/Color.html) [ColorScheme]({{site.url}}/Pages/Reference/UI/ColorScheme.html)|StereoKit will generate a color palette from this gamma space color, and use it to skin the UI!|
|bool [EnableFarInteract]({{site.url}}/Pages/Reference/UI/EnableFarInteract.html)|Enables or disables the far ray grab interaction for Handle elements like the Windows. It can be enabled and disabled for individual UI elements, and if this remains disabled at the start of the next frame, then the hand ray indicators will not be visible. This is enabled by default.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [LastElementActive]({{site.url}}/Pages/Reference/UI/LastElementActive.html)|Tells the Active state of the most recent UI element that used an id.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [LastElementFocused]({{site.url}}/Pages/Reference/UI/LastElementFocused.html)|Tells the Focused state of the most recent UI element that used an id.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [LayoutAt]({{site.url}}/Pages/Reference/UI/LayoutAt.html)|The hierarchy local position of the current UI layout position. The top left point of the next UI element will be start here!|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) [LayoutLast]({{site.url}}/Pages/Reference/UI/LayoutLast.html)|These are the layout bounds of the most recently reserved layout space. The Z axis dimensions are always 0. Only UI elements that affect the surface's layout will report their bounds here. You can reserve your own layout space via UI.LayoutReserve, and that call will also report here.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [LayoutRemaining]({{site.url}}/Pages/Reference/UI/LayoutRemaining.html)|How much space is available on the current layout! This is based on the current layout position, so X will give you the amount remaining on the current line, and Y will give you distance to the bottom of the layout, including the current line. These values will be 0 if you're using 0 for the layout size on that axis.|
|float [LineHeight]({{site.url}}/Pages/Reference/UI/LineHeight.html)|This is the height of a single line of text with padding in the UI's layout system!|
|[UISettings]({{site.url}}/Pages/Reference/UISettings.html) [Settings]({{site.url}}/Pages/Reference/UI/Settings.html)|UI sizing and layout settings. Set only for now|
|bool [ShowVolumes]({{site.url}}/Pages/Reference/UI/ShowVolumes.html)|Shows or hides the collision volumes of the UI! This is for debug purposes, and can help identify visible and invisible collision issues.|

## Static Methods

|  |  |
|--|--|
|[Button]({{site.url}}/Pages/Reference/UI/Button.html)|A pressable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true only on the first frame it is pressed!|
|[ButtonAt]({{site.url}}/Pages/Reference/UI/ButtonAt.html)|A variant of UI.Button that doesn't use the layout system, and instead goes exactly where you put it.|
|[ButtonImg]({{site.url}}/Pages/Reference/UI/ButtonImg.html)|A pressable button accompanied by an image! The button will expand to fit the text provided to it, horizontally. Text is re-used as the id. Will return true only on the first frame it is pressed!|
|[ButtonImgAt]({{site.url}}/Pages/Reference/UI/ButtonImgAt.html)|A variant of UI.ButtonImg that doesn't use the layout system, and instead goes exactly where you put it.|
|[ButtonRound]({{site.url}}/Pages/Reference/UI/ButtonRound.html)|A pressable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true only on the first frame it is pressed!|
|[ButtonRoundAt]({{site.url}}/Pages/Reference/UI/ButtonRoundAt.html)|A variant of UI.ButtonRound that doesn't use the layout system, and instead goes exactly where you put it.|
|[Handle]({{site.url}}/Pages/Reference/UI/Handle.html)|This begins and ends a handle so you can just use  its grabbable/moveable functionality! Behaves much like a window, except with a more flexible handle, and no header. You can draw the handle, but it will have no text on it. Returns true for every frame the user is grabbing the handle.|
|[HandleBegin]({{site.url}}/Pages/Reference/UI/HandleBegin.html)|This begins a new UI group with its own layout! Much like a window, except with a more flexible handle, and no header. You can draw the handle, but it will have no text on it. The pose value is always relative to the current hierarchy stack. This call will also push the pose transform onto the hierarchy stack, so any objects drawn up to the corresponding UI.HandleEnd() will get transformed by the handle pose. Returns true for every frame the user is grabbing the handle.|
|[HandleEnd]({{site.url}}/Pages/Reference/UI/HandleEnd.html)|Finishes a handle! Must be called after UI.HandleBegin() and all elements have been drawn. Pops the pose transform pushed by UI.HandleBegin() from the hierarchy stack.|
|[HSeparator]({{site.url}}/Pages/Reference/UI/HSeparator.html)|This draws a line horizontally across the current layout. Makes a good separator between sections of UI!|
|[HSlider]({{site.url}}/Pages/Reference/UI/HSlider.html)|A horizontal slider element! You can stick your finger in it, and slide the value up and down.|
|[HSliderAt]({{site.url}}/Pages/Reference/UI/HSliderAt.html)|A variant of UI.HSlider that doesn't use the layout system, and instead goes exactly where you put it.|
|[Image]({{site.url}}/Pages/Reference/UI/Image.html)|Adds an image to the UI!|
|[Input]({{site.url}}/Pages/Reference/UI/Input.html)|This is an input field where users can input text to the app! Selecting it will spawn a virtual keyboard, or act as the keyboard focus. Hitting escape or enter, or focusing another UI element will remove focus from this Input.|
|[InteractVolume]({{site.url}}/Pages/Reference/UI/InteractVolume.html)|This method will be removed in v0.4, use UI.VolumeAt.  This watches a volume of space for pinch interaction events! If a hand is inside the space indicated by the bounds, this function will return that hand's pinch state, as well as indicate which hand did it through the out parameter.  Note that since this only provides the hand's pinch state, it won't give you JustActive and JustInactive notifications for when the hand enters or leaves the volume.|
|[IsInteracting]({{site.url}}/Pages/Reference/UI/IsInteracting.html)|Tells if the user is currently interacting with a UI element! This will be true if the hand has an active or focused UI element.|
|[Label]({{site.url}}/Pages/Reference/UI/Label.html)|Adds some text to the layout! Text uses the UI's current font settings, which can be changed with UI.Push/PopTextStyle. Can contain newlines!|
|[LastElementHandUsed]({{site.url}}/Pages/Reference/UI/LastElementHandUsed.html)|Tells if the hand was involved in the focus or active state of the most recent UI element using an id.|
|[LayoutArea]({{site.url}}/Pages/Reference/UI/LayoutArea.html)|Manually define what area is used for the UI layout. This is in the current Hierarchy's coordinate space on the X/Y plane.|
|[LayoutReserve]({{site.url}}/Pages/Reference/UI/LayoutReserve.html)|Reserves a box of space for an item in the current UI layout! If either size axis is zero, it will be auto-sized to fill the current surface horizontally, and fill a single LineHeight vertically. Returns the Hierarchy local bounds of the space that was reserved, with a Z axis dimension of 0.|
|[NextLine]({{site.url}}/Pages/Reference/UI/NextLine.html)|This will advance the layout to the next line. If there's nothing on the current line, it'll advance to the start of the next on. But this won't have any affect on an empty line, try UI.Space for that.|
|[PanelAt]({{site.url}}/Pages/Reference/UI/PanelAt.html)|If you wish to manually draw a Panel, this function will let you draw one wherever you want!|
|[PanelBegin]({{site.url}}/Pages/Reference/UI/PanelBegin.html)|This will begin a Panel element that will encompass all elements drawn between PanelBegin and PanelEnd. This is an entirely visual element, and is great for visually grouping elements together. Every Begin must have a matching End.|
|[PanelEnd]({{site.url}}/Pages/Reference/UI/PanelEnd.html)|This will finalize and draw a Panel element.|
|[PopEnabled]({{site.url}}/Pages/Reference/UI/PopEnabled.html)|Removes an 'enabled' state from the stack, and whatever was below will then be used as the primary enabled state.|
|[PopId]({{site.url}}/Pages/Reference/UI/PopId.html)|Removes the last root id from the stack, and moves up to the one before it!|
|[PopPreserveKeyboard]({{site.url}}/Pages/Reference/UI/PopPreserveKeyboard.html)|This pops the keyboard presentation state to what it was previously.|
|[PopSurface]({{site.url}}/Pages/Reference/UI/PopSurface.html)|This will return to the previous UI layout on the stack. This must be called after a PushSurface call.|
|[PopTextStyle]({{site.url}}/Pages/Reference/UI/PopTextStyle.html)|Removes a TextStyle from the stack, and whatever was below will then be used as the GUI's primary font.|
|[PopTint]({{site.url}}/Pages/Reference/UI/PopTint.html)|Removes a Tint from the stack, and whatever was below will then be used as the primary tint.|
|[ProgressBar]({{site.url}}/Pages/Reference/UI/ProgressBar.html)|This is a simple horizontal progress indicator bar. This is used by the HSlider to draw the slider bar beneath the interactive element. Does not include any text or label.|
|[ProgressBarAt]({{site.url}}/Pages/Reference/UI/ProgressBarAt.html)|This is a simple horizontal progress indicator bar. This is used by the HSlider to draw the slider bar beneath the interactive element. Does not include any text or label.|
|[PushEnabled]({{site.url}}/Pages/Reference/UI/PushEnabled.html)|All UI between PushEnabled and its matching PopEnabled will set the UI to an enabled or disabled state, allowing or preventing interaction with specific elements. The default state is true. This currently doesn't have any visual effect, so you may wish to pair it with a PushTint.|
|[PushId]({{site.url}}/Pages/Reference/UI/PushId.html)|Adds a root id to the stack for the following UI elements! This id is combined when hashing any following ids, to prevent id collisions in separate groups.|
|[PushPreserveKeyboard]({{site.url}}/Pages/Reference/UI/PushPreserveKeyboard.html)|When a soft keyboard is visible, interacting with UI elements will cause the keyboard to close. This function allows you to change this behavior for certain UI elements, allowing the user to interact and still preserve the keyboard's presence. Remember to Pop when you're finished!|
|[PushSurface]({{site.url}}/Pages/Reference/UI/PushSurface.html)|This will push a surface into SK's UI layout system. The surface becomes part of the transform hierarchy, and SK creates a layout surface for UI content to be placed on and interacted with. Must be accompanied by a PopSurface call.|
|[PushTextStyle]({{site.url}}/Pages/Reference/UI/PushTextStyle.html)|This pushes a Text Style onto the style stack! All text elements rendered by the GUI system will now use this styling.|
|[PushTint]({{site.url}}/Pages/Reference/UI/PushTint.html)|All UI between PushTint and its matching PopTint will be tinted with this color. This is implemented by multiplying this color with the current color of the UI element. The default is a White (1,1,1,1) identity tint.|
|[QuadrantSizeMesh]({{site.url}}/Pages/Reference/UI/QuadrantSizeMesh.html)|This will reposition the Mesh's vertices to work well with quadrant resizing shaders. The mesh should generally be centered around the origin, and face down the -Z axis. This will also overwrite any UV coordinates in the verts.  You can read more about the technique [here](https://playdeck.net/blog/quadrant-sizing-efficient-ui-rendering).|
|[QuadrantSizeVerts]({{site.url}}/Pages/Reference/UI/QuadrantSizeVerts.html)|This will reposition the vertices to work well with quadrant resizing shaders. The mesh should generally be centered around the origin, and face down the -Z axis. This will also overwrite any UV coordinates in the verts.  You can read more about the technique [here](https://playdeck.net/blog/quadrant-sizing-efficient-ui-rendering).|
|[Radio]({{site.url}}/Pages/Reference/UI/Radio.html)|A Radio is similar to a button, except you can specify if it looks pressed or not regardless of interaction. This can be useful for radio-like behavior! Check an enum for a value, and use that as the 'active' state, Then switch to that enum value if Radio returns true.|
|[ReserveBox]({{site.url}}/Pages/Reference/UI/ReserveBox.html)|Use LayoutReserve, removing in v0.4|
|[SameLine]({{site.url}}/Pages/Reference/UI/SameLine.html)|Moves the current layout position back to the end of the line that just finished, so it can continue on the same line as the last element!|
|[SetElementVisual]({{site.url}}/Pages/Reference/UI/SetElementVisual.html)|Override the visual assets attached to a particular UI element.  Note that StereoKit's default UI assets use a type of quadrant sizing that is implemented in the Material _and_ the Mesh. You don't need to use quadrant sizing for your own visuals, but if you wish to know more, you can read more about the technique [here](https://playdeck.net/blog/quadrant-sizing-efficient-ui-rendering). You may also find UI.QuadrantSizeVerts and UI.QuadrantSizeMesh to be helpful.|
|[Space]({{site.url}}/Pages/Reference/UI/Space.html)|Adds some space! If we're at the start of a new line, space is added vertically, otherwise, space is added horizontally.|
|[Text]({{site.url}}/Pages/Reference/UI/Text.html)|Displays a large chunk of text on the current layout. This can include new lines and spaces, and will properly wrap once it fills the entire layout! Text uses the UI's current font settings, which can be changed with UI.Push/PopTextStyle.|
|[Toggle]({{site.url}}/Pages/Reference/UI/Toggle.html)|A toggleable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true any time the toggle value changes, NOT the toggle value itself!|
|[ToggleAt]({{site.url}}/Pages/Reference/UI/ToggleAt.html)|A variant of UI.Toggle that doesn't use the layout system, and instead goes exactly where you put it.|
|[VolumeAt]({{site.url}}/Pages/Reference/UI/VolumeAt.html)|A volume for helping to build one handed interactions. This checks for the presence of a hand inside the bounds, and if found, return that hand along with activation and focus information defined by the interactType.|
|[WindowBegin]({{site.url}}/Pages/Reference/UI/WindowBegin.html)|Begins a new window! This will push a pose onto the transform stack, and all UI elements will be relative to that new pose. The pose is actually the top-center of the window. Must be finished with a call to UI.WindowEnd().|
|[WindowEnd]({{site.url}}/Pages/Reference/UI/WindowEnd.html)|Finishes a window! Must be called after UI.WindowBegin() and all elements have been drawn.|

## Examples

### A simple button

![A window with a button]({{site.screen_url}}/UI/ButtonWindow.jpg)

This is a complete window with a simple button on it! `UI.Button`
returns true only for the very first frame the button is pressed, so
using the `if(UI.Button())` pattern works very well for executing
code on button press!

```csharp
Pose windowPoseButton = new Pose(0, 0, 0, Quat.Identity);
void ShowWindowButton()
{
	UI.WindowBegin("Window Button", ref windowPoseButton);

	if (UI.Button("Press me!"))
		Log.Info("Button was pressed.");

	UI.WindowEnd();
}
```

