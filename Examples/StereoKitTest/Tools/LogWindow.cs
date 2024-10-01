// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using System;
using System.Collections.Generic;
using System.Diagnostics;

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
		public bool Enabled { get; set; } = true;

		public Pose pose = Pose.Identity;

		TextStyle styleDiag;
		TextStyle styleInfo;
		TextStyle styleWarn;
		TextStyle styleErr;
		TextStyle styleGraph;

		List<LogItem> items = new List<LogItem>();
		static float  logIndex = 0;

		DiagGraph fpsGraph        = new DiagGraph(5f, 32f);
		DiagGraph ramGraph        = new DiagGraph(0,0);
		DiagGraph managedRamGraph = new DiagGraph(0,0);
		ulong lastMemPoll = 0;

		public LogWindow()
			=> Log.Subscribe(OnLog);

		public bool Initialize()
		{
			styleDiag  = TextStyle.FromFont(Font.Default, 0.008f, Color.HSV(1, 0, 0.7f));
			styleInfo  = TextStyle.FromFont(Font.Default, 0.008f, Color.HSV(1, 0, 1));
			styleWarn  = TextStyle.FromFont(Font.Default, 0.008f, Color.HSV(0.17f, 0.7f, 1));
			styleErr   = TextStyle.FromFont(Font.Default, 0.008f, Color.HSV(1, 0.7f, 1));
			styleGraph = TextStyle.FromFont(Font.Default, 1,      Color.HSV(1, 0, 1));
			return true;
		}
		public void Shutdown()
			=> Log.Unsubscribe(OnLog);
		

		void OnLog(LogLevel level, string logText)
		{
			if (level == LogLevel.Error)
				level = LogLevel.Error;
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
			if (!Enabled) return;

			UI.WindowBegin("Log", ref pose, new Vec2(40, 0) * U.cm);

			DrawLogs();
			UI.HSeparator();
			DrawGraphs();

			UI.WindowEnd();
		}

		void DrawLogs()
		{
			Vec2 textSize = V.XY(UI.LayoutRemaining.x, 0.024f);
			const int count = 10;

			UI.LayoutPushCut(UICut.Top, textSize.y * count);
			UI.LayoutPushCut(UICut.Right, UI.LineHeight*0.6f);
			if (UI.VSlider("scroll", ref logIndex, 0, Math.Max(items.Count - 3, 0), 1))
				logIndex = Math.Max(Math.Min((int)logIndex, items.Count - 1), 0);
			UI.LayoutPop();

			Vec3 start = UI.LayoutAt;
			UI.LayoutReserve(V.XY(textSize.x, textSize.y*count));

			int index = (int)logIndex;
			for (int i = index; i < index + count && i < items.Count; i++)
			{
				LogItem item = items[(items.Count - 1) - i];
				TextStyle ts = item.level switch
				{
					LogLevel.Error     => styleErr,
					LogLevel.Warning   => styleWarn,
					LogLevel.Info      => styleInfo,
					LogLevel.Diagnostic=> styleDiag,
					_                  => styleInfo,
				};
				float y = (i - index) * -textSize.y;
				Text.Add(item.text, Matrix.T(start + new Vec3(0,y, -.004f)), textSize, TextFit.Clip | TextFit.Wrap, ts, TextAlign.TopLeft, TextAlign.CenterLeft);

				if (item.count > 1)
				{
					Vec3 at = V.XYZ(start.x - textSize.x, start.y + y, start.z-0.014f);
					Text.Add(item.count.ToString(), Matrix.T(at), V.XY(textSize.y, textSize.y), TextFit.Clip, styleInfo, TextAlign.TopRight, TextAlign.Center);
				}
			}
			UI.LayoutPop();
		}

		void DrawGraphs()
		{
			#if !WINDOWS_UWP
			ulong memPollIdx = (ulong)(Time.Total * 2);
			if (lastMemPoll != memPollIdx)
			{
				lastMemPoll = memPollIdx;
				using (Process process = Process.GetCurrentProcess())
				{
					long managedMem = GC.GetTotalMemory(false);
					long workingMem = process.WorkingSet64;
					ramGraph       .Add(workingMem / (1024.0f * 1024.0f));
					managedRamGraph.Add(managedMem / (1024.0f * 1024.0f));
				}
			}
			#endif

			float thirdWidth = (UI.LayoutRemaining.x - UI.Settings.gutter*2) / 3.0f;
			Vec2  graphSize  = V.XY(thirdWidth, UI.LineHeight);

			fpsGraph.Add(Time.Stepf*1000);
			fpsGraph.Draw(styleGraph, "Frame (ms)", UI.LayoutAt + V.XYZ(0,0,-0.004f), graphSize);
			UI.LayoutReserve(graphSize);
			UI.SameLine();

			managedRamGraph.Draw(styleGraph, "Managed (mb)", UI.LayoutAt + V.XYZ(0,0,-0.004f), graphSize);
			UI.LayoutReserve(graphSize);
			UI.SameLine();

			ramGraph.Draw(styleGraph, "Total (mb)", UI.LayoutAt + V.XYZ(0, 0, -0.004f), graphSize);
			UI.LayoutReserve(graphSize);
		}

		struct DiagGraph
		{
			float[]     buffer;
			LinePoint[] points;
			int         curr;
			float       min;
			float       max;
			bool        calcMin;
			bool        calcMax;

			public DiagGraph(float min, float max)
			{
				this.min = min;
				this.max = max;
				calcMin = min == 0;
				calcMax = max == 0;
				curr = 0;

				buffer = new float[100];
				points = new LinePoint[100];
				for (int i = 0; i < points.Length; i++)
				{
					points[i].color     = new Color32(255,255,255,100);
					points[i].thickness = 0.001f;
				}
			}

			public void Add(float value)
			{
				buffer[curr] = value;
				curr = (curr + 1) % buffer.Length;
			}

			public void Draw(TextStyle style, string label, Vec3 at, Vec2 size)
			{
				const float labelWidth = 0.01f;
				float       textScale  = size.y * 0.15f;
				size.x -= labelWidth;

				float frameMax = float.MinValue;
				float frameMin = float.MaxValue;
				float step     = 1f/(points.Length-1);
				for (int i = 0; i < points.Length; i++)
				{
					float pct = i * step;
					int   idx = (curr + i) % points.Length;
					float val = Math.Clamp((buffer[idx] - min) / (max - min), 0.0f, 1.0f);
					points[i].pt = new Vec3(at.x - pct*size.x, at.y-size.y + val*size.y, at.z);
					if (buffer[idx] > frameMax) frameMax = buffer[idx];
					if (buffer[idx] < frameMin) frameMin = buffer[idx];
				}
				Lines.Add(points);

				Text.Add(((int)max).ToString(), Matrix.TS(at.x-size.x, at.y,        at.z, textScale), V.XY(labelWidth*textScale, 1), TextFit.Overflow, style, TextAlign.TopLeft,    TextAlign.TopLeft);
				Text.Add(((int)min).ToString(), Matrix.TS(at.x-size.x, at.y-size.y, at.z, textScale), V.XY(labelWidth*textScale, 1), TextFit.Overflow, style, TextAlign.BottomLeft, TextAlign.BottomLeft);
				Text.Add(label,                 Matrix.TS(at.x,        at.y,        at.z, textScale),                                                  style, TextAlign.TopLeft,    TextAlign.TopLeft);

				Lines.Add(at+V.XY0(-Text.SizeLayout(label, style).x*textScale - 0.002f, 0), at + V.XY0(-size.x, 0), new Color32(255,255,255,20), 0.001f);
				Lines.Add(at+V.XY0(0, -size.y), at + V.XY0(-size.x, -size.y), new Color32(255, 255, 255, 20), 0.001f);

				if (calcMin) min = frameMin;
				if (calcMax) max = frameMax;
			}
		}
	}
}
