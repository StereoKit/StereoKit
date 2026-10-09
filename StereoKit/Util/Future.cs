// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A Future tracks work that finishes later, like GPU commands
	/// that are still running. You can check on it each step with `State` or
	/// `IsDone`, or block until it's done with `Wait`. From async code, run
	/// `Wait` on a background task and await that, so no frame stalls.
	///
	/// Futures never need releasing, and any thread may create, check, or
	/// wait on one. A `default` Future is invalid and counts as done.
	/// Failures show up as a negative `FutureState` rather than as
	/// exceptions, so check the state when the outcome matters.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public readonly struct Future
	{
		// Mirrors future_t, which is passed to and from native by value
		readonly IntPtr _slot;
		readonly ulong  _generation;

		/// <summary>Where the work is at right now, without blocking. Any
		/// thread may ask.</summary>
		public FutureState State => NativeAPI.future_check(this);

		/// <summary>True once the work is no longer Pending, including when
		/// it failed. Check `State` to tell those apart.</summary>
		public bool IsDone => State != FutureState.Pending;

		/// <summary>Blocks the calling thread until the work is done. Avoid
		/// this on the main thread when you can, since it stalls the frame
		/// until the GPU catches up.</summary>
		/// <returns>How the work ended, Ready, Failed, or None for an
		/// invalid Future.</returns>
		public FutureState Wait() => NativeAPI.future_wait(this);
	}
}
