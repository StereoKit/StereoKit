using StereoKit;

class DocUI : ITest
{
	public void Initialize()
	{
		Tests.RunForFrames(2);
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		ShowWindowButton   ();
		ShowWindowToggle   ();
		ShowWindowSeparator();
		ShowWindowSlider   ();
		ShowWindowInput    ();
		ShowWindowRadio    ();
		ShowWindowEnabled  ();

		Tests.Screenshot("UI/ButtonWindow.jpg",    1, 500, 400, 50, V.XYZ(.0f,-0.02f,-.2f), V.XYZ(.0f,-0.02f,0));
		Tests.Screenshot("UI/ToggleWindow.jpg",    1, 500, 400, 50, V.XYZ(.3f,-0.04f,-.2f), V.XYZ(.3f,-0.04f,0));
		Tests.Screenshot("UI/SeparatorWindow.jpg", 1, 500, 400, 50, V.XYZ(.6f,-0.07f,-.2f), V.XYZ(.6f,-0.07f,0));
		Tests.Screenshot("UI/SliderWindow.jpg",    1, 500, 400, 45, V.XYZ(.9f,-0.02f,-.2f), V.XYZ(.9f,-0.02f,0));
		Tests.Screenshot("UI/InputWindow.jpg",     1, 500, 400, 45, V.XYZ(1.2f,-0.02f,-.2f), V.XYZ(1.2f,-0.02f,0));
		Tests.Screenshot("UI/RadioWindow.jpg",     1, 500, 500, 55, V.XYZ(1.5f,-0.06f,-.2f), V.XYZ(1.5f,-0.06f,0));
		Tests.Screenshot("UI/EnabledWindow.jpg",   1, 500, 500, 55, V.XYZ(1.8f,-0.07f,-.2f), V.XYZ(1.8f,-0.07f,0));
	}

	/// :CodeSample: UI UI.Button UI.WindowBegin UI.WindowEnd
	/// ### A simple button
	/// 
	/// ![A window with a button]({{site.screen_url}}/UI/ButtonWindow.jpg)
	/// 
	/// This is a complete window with a simple button on it! `UI.Button`
	/// returns true only for the very first frame the button is pressed, so
	/// using the `if(UI.Button())` pattern works very well for executing
	/// code on button press!
	/// 
	Pose windowPoseButton = new Pose(0, 0, 0, Quat.Identity);
	void ShowWindowButton()
	{
		UI.WindowBegin("Window Button", ref windowPoseButton);

		if (UI.Button("Press me!"))
			Log.Info("Button was pressed.");

		UI.WindowEnd();
	}
	/// :End:

	/// :CodeSample: UI.Toggle
	/// ### A toggle button
	/// 
	/// ![A window with a toggle]({{site.screen_url}}/UI/ToggleWindow.jpg)
	/// 
	/// Toggle buttons swap between true and false when you press them! The
	/// function requires a reference to a bool variable where the toggle's
	/// state is stored. This allows you to manage the state yourself, and
	/// it's completely valid for you to change the toggle state separately,
	/// the UI element will update to match.
	/// 
	/// Note that `UI.Toggle` returns true _only_ when the toggle state has
	/// changed, and does _not_ return the current state.
	/// 
	Pose windowPoseToggle = new Pose(.3f, 0, 0, Quat.Identity);
	bool toggleState      = true;
	void ShowWindowToggle()
	{
		UI.WindowBegin("Window Toggle", ref windowPoseToggle);

		if (UI.Toggle("Toggle me!", ref toggleState))
			Log.Info("Toggle just changed.");
		if (toggleState) UI.Label("Toggled On");
		else             UI.Label("Toggled Off");

		UI.WindowEnd();
	}
	/// :End:

	/// :CodeSample: UI.HSeparator UI.Label UI.Text UIWin
	/// ### Separating UI Visually
	/// 
	/// ![A window with text and a separator]({{site.screen_url}}/UI/SeparatorWindow.jpg)
	/// 
	/// A separator is a simple visual element that fills the window
	/// horizontally. It's nothing complicated, but can help create visual
	/// association between groups of UI elements.
	/// 
	Pose windowPoseSeparator = new Pose(.6f, 0, 0, Quat.Identity);
	void ShowWindowSeparator()
	{
		UI.WindowBegin("Window Separator", ref windowPoseSeparator, UIWin.Body);

		UI.Label("Content Header");
		UI.HSeparator();
		UI.Text("A separator can go a long way towards making your content "
		      + "easier to look at!", TextAlign.TopCenter);

		UI.WindowEnd();
	}
	/// :End:

	/// :CodeSample: UI.HSlider
	/// ### Horizontal Sliders
	/// 
	/// ![A window with a slider]({{site.screen_url}}/UI/SliderWindow.jpg)
	/// 
	/// A slider will slide between two values at increments. The function
	/// requires a reference to a float variable where the slider's state is
	/// stored. This allows you to manage the state yourself, and it's
	/// completely valid for you to change the slider state separately, the
	/// UI element will update to match.
	/// 
	/// Note that `UI.HSlider` returns true _only_ when the slider state has
	/// changed, and does _not_ return the current state.
	/// 
	Pose  windowPoseSlider = new Pose(.9f, 0, 0, Quat.Identity);
	float sliderState      = 0.5f;
	void ShowWindowSlider()
	{
		UI.WindowBegin("Window Slider", ref windowPoseSlider);

		if (UI.HSlider("Slider", ref sliderState, 0, 1, 0.1f))
			Log.Info($"Slider value just changed: {sliderState}");

		UI.WindowEnd();
	}
	/// :End:

	/// :CodeSample: UI.Input UI.SameLine
	/// ### Text Input
	/// 
	/// ![A window with a text input]({{site.screen_url}}/UI/InputWindow.jpg)
	/// 
	/// The `UI.Input` element allows users to enter text. Upon selecting the
	/// element, a virtual keyboard will appear on platforms that provide
	/// one.  The function requires a reference to a string variable where
	/// the input's state is stored. This allows you to manage the state
	/// yourself, and it's completely valid for you to change the input state
	/// separately, the UI element will update to match.
	/// 
	/// `UI.Input` will return true on frames where the text has _just_
	/// changed.
	/// 
	Pose   windowPoseInput = new Pose(1.2f, 0, 0, Quat.Identity);
	string inputState      = "Initial text";
	void ShowWindowInput()
	{
		UI.WindowBegin("Window Input", ref windowPoseInput);

		// Add a small label in front of it on the same line
		UI.Label("Text:");
		UI.SameLine();
		if (UI.Input("Text", ref inputState))
			Log.Info($"Input text just changed");

		UI.WindowEnd();
	}
	/// :End:

	/// :CodeSample: UI.Radio
	/// ### Radio button group
	/// 
	/// ![A window with radio buttons]({{site.screen_url}}/UI/RadioWindow.jpg)
	/// 
	/// Radio buttons are a variety of Toggle button that behaves in a manner
	/// more conducive to radio group style behavior. This is an example of
	/// how to implement a small radio button group.
	/// 
	Pose windowPoseRadio = new Pose(1.5f, 0, 0, Quat.Identity);
	int  radioState      = 1;
	void ShowWindowRadio()
	{
		UI.WindowBegin("Window Radio", ref windowPoseRadio);

		if (UI.Radio("Option 1", radioState == 1)) radioState = 1;
		if (UI.Radio("Option 2", radioState == 2)) radioState = 2;
		if (UI.Radio("Option 3", radioState == 3)) radioState = 3;

		UI.WindowEnd();
	}
	/// :End:

	/// :CodeSample: UI.PushEnabled UI.PopEnabled UI.Enabled
	/// ### Enabling and Disabling UI Elements
	/// 
	/// ![A window with labels in various states of enablement]({{site.screen_url}}/UI/EnabledWindow.jpg)
	/// 
	/// UI.Push/PopEnabled allows you to enable and disable groups of UI
	/// elements! This is a hierarchical stack, so by default, all PushEnabled
	/// calls inherit the stack's state.
	/// 
	Pose windowPoseEnabled = new Pose(1.8f, 0, 0, Quat.Identity);
	void ShowWindowEnabled()
	{
		UI.WindowBegin("Window Enabled", ref windowPoseEnabled);

		// Default state of the enabled stack is true
		UI.Label(UI.Enabled ? "Enabled" : "Disabled");

		UI.PushEnabled(false);
		{
			// This label will be disabled
			UI.Label(UI.Enabled?"Enabled":"Disabled");

			UI.PushEnabled(true);
			{
				// This label inherits the state of the parent, so is therefore
				// disabled
				UI.Label(UI.Enabled?"Enabled":"Disabled");
			}
			UI.PopEnabled();

			UI.PushEnabled(true, HierarchyParent.Ignore);
			{
				// This label was enabled, overriding the parent, and so is
				// enabled.
				UI.Label(UI.Enabled ? "Enabled" : "Disabled");
			}
			UI.PopEnabled();
		}
		UI.PopEnabled();

		UI.WindowEnd();
	}
	/// :End:
}