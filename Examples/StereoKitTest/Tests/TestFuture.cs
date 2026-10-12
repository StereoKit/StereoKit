// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

using StereoKit;
using System.Threading.Tasks;

class TestFuture : ITest
{
	const int MaxFrames = 60;

	Future            polled;
	Task<FutureState> waitTask;
	int               frames;

	public void Initialize()
	{
		Future invalid = default;
		Tests.Test(() => invalid.State == FutureState.None && invalid.IsDone);

		Future waited = Renderer.GPUFuture();
		Tests.Test(() => waited.Wait() == FutureState.Ready && waited.State == FutureState.Ready);

		// The async pattern apps are expected to use, waiting off the main thread
		polled   = Renderer.GPUFuture();
		waitTask = Task.Run(() => Renderer.GPUFuture().Wait());

		frames = 1;
		Tests.RunForFrames(frames);
	}

	public void Shutdown() { }

	public void Step()
	{
		bool finished = polled.IsDone && waitTask.IsCompleted;
		if (!finished && frames < MaxFrames) { frames += 1; Tests.RunForFrames(frames); return; }

		Tests.Test(() => polled.State == FutureState.Ready);
		Tests.Test(() => waitTask.IsCompleted && waitTask.Result == FutureState.Ready);
	}
}
