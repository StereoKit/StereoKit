using StereoKit;

class DocPlatformFile : ITest
{
	Pose windowPose = Pose.Identity;

	public void Initialize() {}
	public void Shutdown  () {}

	public void Update()
	{
		UI.WindowBegin("Settings", ref windowPose);
		if (UI.Button("Open"))
		{
			/// :CodeSample: Platform.FilePicker Platform.ReadFile
			/// ### Read Custom Files
			Platform.FilePicker(PickerMode.Open, file => {
				// On some platforms (like UWP), you may encounter permission
				// issues when trying to read or write to an arbitrary file.
				//
				// StereoKit's `Platform.FilePicker` and `Platform.ReadFile`
				// work together to avoid this permission issue, where the
				// FilePicker will grant permission to the ReadFile method.
				// C#'s built-in `File.ReadAllText` would fail on UWP here.
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
				// On some platforms (like UWP), you may encounter permission
				// issues when trying to read or write to an arbitrary file.
				//
				// StereoKit's `Platform.FilePicker` and `Platform.WriteFile`
				// work together to avoid this permission issue, where the
				// FilePicker will grant permission to the WriteFile method.
				// C#'s built-in `File.WriteAllText` would fail on UWP here.
				Platform.WriteFile(file, "Text for the file.\n- Thanks!");
			}, null, ".txt");
			/// :End:
		}
		UI.WindowEnd();
	}
}