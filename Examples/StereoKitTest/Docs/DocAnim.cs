using StereoKit;

class DocAnim : ITest
{
	Model model;

	public void Initialize()
	{
		/// :CodeSample: Model Anim AnimMode Model.FromFile Model.Anims Anim.Name Anim.Duration Model.PlayAnim Model.FindAnim
		/// ### Loading an animated Model
		/// Here, we're loading a Model that we know has the animations "Idle"
		/// and "Jump". This sample shows some options, but only a single call
		/// to PlayAnim is necessary to start an animation.
		Model model = Model.FromFile("Cosmonaut.glb");

		// You can look at the model's animations:
		foreach (Anim anim in model.Anims)
			Log.Info($"Animation: {anim.Name} {anim.Duration}s");

		// You can play an animation like this
		model.PlayAnim("Jump", AnimMode.Once);

		// Or you can find and store the animations in advance
		Anim jumpAnim = model.FindAnim("Idle");
		if (jumpAnim != null)
			model.PlayAnim(jumpAnim, AnimMode.Loop);
		/// :End:

		this.model = model;

		Time.SetTime(model.ActiveAnim.Duration * 0.3f);
	}

	public void Shutdown()
	{
	}

	public void Update()
	{
		/// :CodeSample: Model Model.AnimCompletion Model.ActiveAnim Model.AnimTime Model.AnimMode Anim.Name
		/// ### Animation progress bar
		/// A really simple progress bar visualization for the Model's active
		/// animation.
		/// 
		/// ![Model with progress bar]({{site.screen_url}}/AnimProgress.jpg)
		model.Draw(Matrix.Identity);

		Hierarchy.Push(Matrix.T(0.5f, 1, -.25f));

		// This is a pair of green lines that show the current progress through
		// the animation.
		float progress = model.AnimCompletion;
		Lines.Add(V.XY0(0, 0), V.XY0(-progress, 0),  new Color(0,1,0,1.0f), 2*U.cm);
		Lines.Add(V.XY0(-progress, 0), V.XY0(-1, 0), new Color(0,1,0,0.2f), 2*U.cm);

		// These are some labels for the progress bar that tell us more about
		// the active animation.
		Text.Add($"{model.ActiveAnim.Name} : {model.AnimMode}", Matrix.TS(0, -2*U.cm, 0, 3),        TextAlign.TopLeft);
		Text.Add($"{model.AnimTime:F1}s",                       Matrix.TS(-progress, 2*U.cm, 0, 3), TextAlign.BottomCenter);

		Hierarchy.Pop();
		/// :End:

		Tests.Screenshot("AnimProgress.jpg", 400, 600, V.XYZ(0, .9f, -1.2f), V.XYZ(0, .9f, 0));
	}
}