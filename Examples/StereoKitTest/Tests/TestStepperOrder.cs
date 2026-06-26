using StereoKit;
using StereoKit.Framework;
using System.Collections.Generic;

class TestStepperOrder : ITest
{
	// A running record of the order steppers and the app's Step ran in. It's
	// cleared at the very start of each frame and snapshotted at the very end,
	// so `captured` always holds one complete, clean frame.
	static List<string> order    = new List<string>();
	static List<string> captured = new List<string>();

	// An IStepper that logs its priority tag when stepped. Priority is supplied
	// via the StepperPriority attribute on each derived type, since the
	// attribute is type-level metadata.
	abstract class OrderStepper : IStepper
	{
		public abstract string Tag { get; }
		public bool Enabled       => true;
		public bool Initialize    () => true;
		public void Shutdown      () { }
		public void Step          () => order.Add(Tag);
	}

	// Runs first each frame (lowest priority) and clears the log. Records
	// nothing itself.
	[StepperPriority(-1000)] class StepperReset : IStepper
	{
		public bool Enabled    => true;
		public bool Initialize () => true;
		public void Shutdown   () { }
		public void Step       () => order.Clear();
	}

	// Runs last each frame (highest priority) and snapshots the completed
	// frame. Records nothing itself.
	[StepperPriority(1000)] class StepperCapture : IStepper
	{
		public bool Enabled    => true;
		public bool Initialize () => true;
		public void Shutdown   () { }
		public void Step       () { captured = new List<string>(order); }
	}

	[StepperPriority(-10)] class StepperNeg10 : OrderStepper { public override string Tag => "-10"; }
	[StepperPriority( -1)] class StepperNeg1  : OrderStepper { public override string Tag => "-1";  }
	/* default 0 */        class StepperZero  : OrderStepper { public override string Tag => "0";   }
	[StepperPriority(  1)] class StepperPos1  : OrderStepper { public override string Tag => "1";   }
	[StepperPriority( 10)] class StepperPos10 : OrderStepper { public override string Tag => "10";  }

	public void Initialize()
	{
		// Add in a deliberately scrambled order to prove sorting, not add-order,
		// determines execution order.
		SK.AddStepper<StepperPos1  >();
		SK.AddStepper<StepperNeg1  >();
		SK.AddStepper<StepperCapture>();
		SK.AddStepper<StepperPos10 >();
		SK.AddStepper<StepperZero  >();
		SK.AddStepper<StepperReset >();
		SK.AddStepper<StepperNeg10 >();

		// Steppers are initialized at the start of the next frame, so let a few
		// frames run to capture a complete clean frame.
		Tests.RunForFrames(4);
	}

	public void Step()
	{
		// The pre-app steppers (priority <= 0) have already run this frame; mark
		// the app step here, then the post-app steppers (priority > 0) append
		// after us.
		order.Add("app");
	}

	public void Shutdown()
	{
		SK.RemoveStepper<StepperReset  >();
		SK.RemoveStepper<StepperCapture>();
		SK.RemoveStepper<StepperNeg10  >();
		SK.RemoveStepper<StepperNeg1   >();
		SK.RemoveStepper<StepperZero   >();
		SK.RemoveStepper<StepperPos1   >();
		SK.RemoveStepper<StepperPos10  >();

		// A full clean frame: negative/zero priorities sorted ascending, then
		// the app Step, then positive priorities sorted ascending.
		string[] expected = { "-10", "-1", "0", "app", "1", "10" };
		Tests.Test(() =>
		{
			if (captured.Count != expected.Length) return false;
			for (int i = 0; i < expected.Length; i++)
				if (captured[i] != expected[i]) return false;
			return true;
		});
	}
}
