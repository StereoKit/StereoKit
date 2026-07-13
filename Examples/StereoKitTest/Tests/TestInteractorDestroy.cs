// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using StereoKit;

/// <summary>Users are allowed to destroy StereoKit's system interactors via
/// Interactor.Destroy! This shouldn't crash the input or UI systems, they
/// should just quietly stop interacting until a mode switch recreates them.
/// This guards against a real issue where the rust bindings implicitly
/// destroyed system interactors, crashing interactor mode updates.</summary>
internal class TestInteractorDestroy : ITest
{
	int  frame = 0;
	Pose windowPose = new Pose(0, 0, -0.5f, Quat.FromAngles(0, 180, 0));
	DefaultInteractors prevInteractors;

	public void Initialize()
	{
		prevInteractors = Interaction.DefaultInteractors;
		Tests.RunForFrames(8);
	}
	public void Shutdown()
	{
		// Finish restoring the interactors we destroyed, the mode switch back
		// from None recreates them. See the end of Step.
		Interaction.DefaultInteractors = prevInteractors;
	}

	public void Step()
	{
		frame++;
		// Destroying interactors is a blocker when visiting this scene
		// interactively, so only do it during automated test runs.
		if (Tests.IsTesting)
		{
			if (frame == 4)
			{
				foreach (Interactor i in Interactor.All)
					i.Destroy();
				Log.Info("Destroyed all system interactors");
			}
			// Interactors only recreate via a mode switch, so spend the last
			// frames at None, and Shutdown will switch back to recreate them.
			if (frame == 6)
				Interaction.DefaultInteractors = DefaultInteractors.None;
		}

		// UI and interactor queries should still behave with no interactors
		// present.
		UI.WindowBegin("Interactor Destroy", ref windowPose);
		UI.Button("Still alive?");
		UI.WindowEnd();
		foreach (Interactor i in Interactor.All)
			_ = i.Tracked;
	}
}
