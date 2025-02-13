using StereoKit;
using StereoKit.Framework;
using System.Threading;
using System.Threading.Tasks;

class TestConcurrentSteppers : ITest
{
	// We don't need anything fancy, just something we can easily add/remove
	class TestStepper : IStepper
	{
		public bool Enabled => true;

		public bool Initialize() => true;
		public void Shutdown  () { }
		public void Step      () { }
	}

	static bool run = false;
	Task taskAdd;
	Task taskRemove;

	public void Initialize()
	{
		run = true;

		// One thread adds, one removes, SK's main thread will step whaterver ones
		// are still around.
		taskAdd    = Task.Run(() => { while (run) { Thread.Sleep(1); SK.AddStepper<TestStepper>(); } });
		taskRemove = Task.Run(() => { while (run) { Thread.Yield(); SK.RemoveStepper(SK.GetStepper<TestStepper>()); } });

		Tests.RunForSeconds(0.25f);
	}

	public void Shutdown()
	{
		run = false;
		taskAdd   .Wait();
		taskRemove.Wait();

		SK.RemoveStepper<TestStepper>();
	}

	public void Step()
	{
	}
}