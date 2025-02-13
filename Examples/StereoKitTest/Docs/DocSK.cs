// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

using StereoKit;

class DocSK : ITest
{
	static void QuitLifecycle() {
		// :CodeSample: SK.Quit QuitReason SK.QuitReason
		// ### Quitting and Reasons
		//
		// `SK.Quit` allows you to exit the application, with an optionally
		// provided reason! You can check the `SK.QuitReason` to see if
		// StereoKit's reason for exiting was benign, like a User request, or
		// if something more severe happened like an OpenXR runtime crash!
		if (!SK.Initialize())
			return;

		SK.Run(() =>
		{
			if (Input.Key(Key.Esc).IsJustActive())
				SK.Quit(QuitReason.User);
		});

		Log.Info($"{SK.QuitReason}");
		// :End:
	}

	// Our sample is lifetime code, so we unfortunately can't run it in this
	// context! This does still benefit from refactoring and syntax checking
	// though.
	public void Step      () { }
	public void Initialize() { }
	public void Shutdown  () { }
}