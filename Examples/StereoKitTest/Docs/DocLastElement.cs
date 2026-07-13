using StereoKit;

class DocLastElement : ITest
{
	float sliderVal  = 0.5f;
	Pose  windowPose = new Pose(0,0,-0.5f, Quat.LookDir(0,0,1));

	public void Step()
	{
		// Point a single custom far-ray interactor at the slider. Frame 0 is a
		// warm-up (a freshly-created interactor can't focus yet), then frames 1-3
		// run focus -> activate -> hold. The ray starts below and a little in
		// front of the window so it reads nicely in the screenshot.
		Vec3     dir   = (rayTarget - rayOrigin).Normalized;
		BtnState pinch = frame switch {
			0 => BtnState.Inactive,                     // warm-up
			1 => BtnState.Inactive,                     // focus
			2 => BtnState.Active | BtnState.JustActive, // activate
			_ => BtnState.Active };                     // hold
		ray.Update(rayOrigin, rayOrigin + dir * 100, new Pose(rayOrigin, Quat.LookDir(dir)), rayOrigin, Vec3.Zero, pinch, BtnState.Active);
		rayVisible = 1;
		Tests.DrawInteractorRay(ray, ref rayVisible, ref rayActive);

		/// :CodeSample: UI.LastElementSourceActive UI.LastElementSourceFocused UI.LastElementActive UI.LastElementFocused
		/// ### Checking UI element status
		/// It can sometimes be nice to know how the user is interacting with a
		/// particular UI element! The UI.LastElementX functions can be used to
		/// query a bit of this information, but only for _the most recent_ UI
		/// element that **uses an id**!
		///
		/// ![A window containing the status of a UI element]({{site.screen_url}}/UI/LastElementAPI.jpg)
		///
		/// So in this example, we're querying the information for the "Slider"
		/// UI element. Note that UI.Text does NOT use an id, which is why this
		/// works.
		UI.WindowBegin("Last Element API", ref windowPose);

		UI.HSlider("Slider", ref sliderVal, 0, 1, 0.1f, 0, UIConfirm.Pinch);
		UI.Text("Element Info:", Align.TopCenter);
		if (UI.LastElementSourceActive (InteractorSource.HandLeft  | InteractorSource.ControllerLeft ).IsActive()) UI.Label("Left Active");
		if (UI.LastElementSourceActive (InteractorSource.HandRight | InteractorSource.ControllerRight).IsActive()) UI.Label("Right Active");
		if (UI.LastElementSourceFocused(InteractorSource.HandLeft  | InteractorSource.ControllerLeft ).IsActive()) UI.Label("Left Focused");
		if (UI.LastElementSourceFocused(InteractorSource.HandRight | InteractorSource.ControllerRight).IsActive()) UI.Label("Right Focused");
		if (UI.LastElementFocused.IsActive()) UI.Label("Focused");
		if (UI.LastElementActive .IsActive()) UI.Label("Active");

		UI.WindowEnd();
		/// :End:

		Tests.Screenshot("UI/LastElementAPI.jpg", 3, 500, 500, 90, V.XYZ(0, -0.10f, -0.35f), V.XYZ(0, -0.10f, -0.5f));
		frame++;
	}

	DefaultInteractors oldInteractors;
	Interactor         ray;
	Vec3               rayOrigin  = V.XYZ(0.05f, -0.2f, -0.35f); // below and a bit in front of the window
	Vec3               rayTarget;
	float              rayVisible = 0;
	float              rayActive  = 0;
	int                frame      = 0;
	public void Initialize()
	{
		oldInteractors = Interaction.DefaultInteractors;
		Interaction.DefaultInteractors = DefaultInteractors.None;

		ray       = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.HandRight, 0.015f, 0);
		rayTarget = windowPose.position + V.XYZ(0, -0.03f, 0); // Slider handle location, may need updated if UI changes
		Tests.RunForFrames(4);
	}
	public void Shutdown()
	{
		ray.Destroy();
		Interaction.DefaultInteractors = oldInteractors;
	}
}
