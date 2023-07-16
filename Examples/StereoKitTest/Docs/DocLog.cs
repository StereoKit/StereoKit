using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DocLog : ITest
{
	public void Initialize()
	{
		/// :CodeSample: Log.Subscribe Log
		/// Then you add the OnLog method into the log events like this in
		/// your initialization code!
		Log.Subscribe(OnLog);
		/// :End:
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		/// :CodeSample: Log.Subscribe Log
		/// And in your Update loop, you can draw the window.
		LogWindow();
		/// And that's it!
		/// :End:
	}

	/// :CodeSample: Log.Subscribe Log
	/// ### An in-application log window
	/// Here's an example of using the Log.Subscribe method to build a simple
	/// logging window. This can be pretty handy to have around somewhere in
	/// your application!
	/// 
	/// Here's the code for the window, and log tracking.
	static Pose logPose = new Pose(0, -0.1f, 0.5f, Quat.LookDir(Vec3.Forward));
	static List<string> logList = new List<string>();
	static float logIndex = 0;
	static string logString = "";
	static void OnLog(LogLevel level, string text)
	{
		logList.Insert(0, text.Length < 100 ? text + "\n" : text.Substring(0, 100) + "...\n");
		UpdateLogStr((int)logIndex);
	}

	static void UpdateLogStr(int index)
	{
		logIndex = Math.Max(Math.Min(index, logList.Count - 1), 0);
		logString = "";
		for (int i = index; i < index + 15 && i < logList.Count; i++)
			logString += logList[i];
	}

	static void LogWindow()
	{
		UI.WindowBegin("Log", ref logPose, new Vec2(40, 0) * U.cm);

		UI.LayoutPushCut(UICut.Right, UI.LineHeight);
		if (UI.VSlider("scroll", ref logIndex, 0, Math.Max(logList.Count - 3, 0), 1))
			UpdateLogStr((int)logIndex);
		UI.LayoutPop();

		UI.Text(logString);
		UI.WindowEnd();
	}
	/// :End:
}
