using StereoKit;

class DocBtnState : ITest
{
	public void Initialize()
	{
		/// :CodeSample: BtnState BtnStateExtensions
		BtnState state = Input.Hand(Handed.Right).pinch;

		// You can check a BtnState using bit-flag logic
		if ((state & BtnState.Changed) > 0)
			Log.Info("Pinch state just changed!");

		// Or you can check the same values with the extension methods, no
		// bit flag logic :)
		if (state.IsChanged())
			Log.Info("Pinch state just changed!");
		/// :End:
	}

	public void Shutdown() { }
	public void Update() { }
}