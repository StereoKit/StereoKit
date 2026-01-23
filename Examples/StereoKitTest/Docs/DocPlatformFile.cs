using StereoKit;

class DocPlatformFile : ITest
{
	Pose windowPose = Pose.Identity;

	public void Initialize() {}
	public void Shutdown  () {}

	public void Step()
	{
		UI.WindowBegin("Settings", ref windowPose);
		if (UI.Button("Open"))
		{
			/// :CodeSample: Platform.FilePicker Platform.ReadFile
			/// ### Read Custom Files
			Platform.FilePicker(PickerMode.Open, file => {
				// On some platforms, using StereoKit's Platform.ReadFile
				// instead of C#'s File IO functions may help bypass permission
				// issues.
				if (Platform.ReadFile(file, out string text))
					Log.Info(text);
			}, null, ".txt");
			/// :End:
		}

		if (UI.Button("Save"))
		{
			/// :CodeSample: Platform.FilePicker Platform.WriteFile
			/// ### Write Custom Files
			Platform.FilePicker(PickerMode.Save, file => {
				// On some platforms, using StereoKit's Platform.WriteFile
				// instead of C#'s File IO functions may help bypass permission
				// issues.
				Platform.WriteFile(file, "Text for the file.\n- Thanks!");
			}, null, ".txt");
			/// :End:
		}
		UI.WindowEnd();
	}
}