using System;
using System.Collections.Generic;
using System.Linq;

namespace StereoKit.Framework
{
	public class LogWindow : IStepper
	{
		struct LogItem
		{
			public LogLevel level;
			public string   text;
			public int      count;
		}
		public bool Enabled => true;

		public Pose pose = Pose.Identity;

		TextStyle styleDiag;
		TextStyle styleInfo;
		TextStyle styleWarn;
		TextStyle styleErr;

		List<LogItem> items = new List<LogItem>();
		const int maxItems = 1000;
		static float logIndex = 0;

		public LogWindow()
			=> Log.Subscribe(OnLog);

		public bool Initialize()
		{
			styleDiag = TextStyle.FromFont(Font.Default, 0.008f, Color.HSV(1, 0, 0.7f));
			styleInfo = TextStyle.FromFont(Font.Default, 0.008f, Color.HSV(1, 0, 1));
			styleWarn = TextStyle.FromFont(Font.Default, 0.008f, Color.HSV(0.17f, 0.7f, 1));
			styleErr  = TextStyle.FromFont(Font.Default, 0.008f, Color.HSV(1, 0.7f, 1));
			return true;
		}
		public void Shutdown()
			=> Log.Unsubscribe(OnLog);
		

		void OnLog(LogLevel level, string logText)
		{
			if (items.Count>0 && items[items.Count-1].text == logText) {
				LogItem item = items[items.Count - 1];
				item.count++;
				items[items.Count - 1] = item;
				return;
			}
			items.Add(new LogItem { level = level, text = logText, count = 1 });
			if (logIndex != 0) logIndex += 1;
		}

		public void Step()
		{
			UI.WindowBegin("Log", ref pose, new Vec2(40, 0) * U.cm);

			UI.LayoutPushCut(UICut.Right, UI.LineHeight*0.6f);
			if (UI.VSlider("scroll", ref logIndex, 0, Math.Max(items.Count - 3, 0), 1))
				logIndex = Math.Max(Math.Min((int)logIndex, items.Count - 1), 0);
			UI.LayoutPop();

			Vec2 textSize = V.XY(UI.LayoutRemaining.x, 0.024f);

			int index = (int)logIndex;
			const int count = 10;
			for (int i = index; i < index + count && i < items.Count; i++)
			{
				LogItem item = items[(items.Count - 1) - i];
				UI.PushTextStyle(item.level switch
				{
					LogLevel.Error     => styleErr,
					LogLevel.Warning   => styleWarn,
					LogLevel.Info      => styleInfo,
					LogLevel.Diagnostic=> styleDiag,
					_                  => styleInfo,
				});
				UI.Text(item.text, TextAlign.TopLeft, TextFit.Wrap | TextFit.Clip, textSize);
				UI.PopTextStyle();

				if (item.count > 1)
				{
					Bounds b  = UI.LayoutLast;
					Vec3   at = b.center - V.XYZ(b.dimensions.x / 2, 0, 0.004f);
					Text.Add(item.count.ToString(), Matrix.T(at), styleInfo);
				}
			}

			UI.WindowEnd();
		}
	}
}
