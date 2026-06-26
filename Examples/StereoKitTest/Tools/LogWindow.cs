// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2023-2026 Nick Klingensmith
// Copyright (c) 2023-2026 Qualcomm Technologies, Inc.

using System;
using System.Collections.Generic;

namespace StereoKit.Framework
{
	public class LogWindow : IStepper
	{
		struct LogItem
		{
			public LogLevel level;
			public string   text;
			public int      count;
			public float    height; // Cached layout height (m), incl. padding.
		}
		// One entry per log line that intersects the viewport. Rebuilt only
		// when the scroll, content, or width changes - not every frame.
		struct VisibleLine
		{
			public int   index; // Index into 'items'.
			public float y;     // Entry top, viewport-relative (0 = viewport top, down is -).
		}
		private bool enabled = false;
		public  bool Enabled { get => enabled; set { enabled = value; pose = UI.PopupPose(); } }

		public Pose pose = Pose.Identity;

		TextStyle styleDiag;
		TextStyle styleInfo;
		TextStyle styleWarn;
		TextStyle styleErr;
		TextStyle styleGraph;

		// lineSpacing sets the distance between every line equally - both separate
		// entries and lines within a wrapped entry - so the two look identical.
		const float lineSpacing = 1.4f; // Line spacing as a fraction of font height (TextStyle.LineHeightPct).
		// Sizes in lines of log text, scaled to meters in Initialize so they track
		// the font size.
		const float viewLines  = 16;   // Viewport height, in lines.
		const float badgeLines = 1.5f; // Badge width, in lines.

		float rowPad     = 0; // Per-entry padding (m); derived from lineSpacing.
		float viewHeight = 0; // Viewport height (m); viewLines scaled in Initialize.
		float badgeSize  = 0; // Badge box width (m); badgeLines scaled in Initialize.

		List<LogItem>     items   = new List<LogItem>();
		List<VisibleLine> visible = new List<VisibleLine>();

		float scroll        = 0;     // Scroll offset (m) from the top (oldest) of the content.
		float scrollMax     = 0;     // Largest valid scroll, so the newest line sits at the bottom.
		float contentHeight = 0;     // Total height (m) of every log item.
		bool  stickBottom   = true;  // Keep the view pinned to the newest line.
		int   measuredCount = 0;     // How many items have a valid cached height.
		float measureWidth  = 0;     // Width the cached heights were measured against.
		float cachedScroll  = -1;    // Scroll value the visible list was built for.
		bool  viewDirty     = true;

		DiagGraph fpsGraph     = new DiagGraph(0, 0);
		DiagGraph cpuPerfGraph = new DiagGraph(0, 0);
		DiagGraph gpuPerfGraph = new DiagGraph(0, 0);

		public LogWindow()
		{
			Log.Subscribe(OnLog);
			Enabled = true;
		}

		public bool Initialize()
		{
			Font logFnt = Font.FromFamily("monospace");
			styleDiag  = TextStyle.FromFont(logFnt, 0.008f, Color.HSV(1, 0, 0.7f));
			styleInfo  = TextStyle.FromFont(logFnt, 0.008f, Color.HSV(1, 0, 1));
			styleWarn  = TextStyle.FromFont(logFnt, 0.008f, Color.HSV(0.17f, 0.7f, 1));
			styleErr   = TextStyle.FromFont(logFnt, 0.008f, Color.HSV(1, 0.7f, 1));
			styleGraph = TextStyle.FromFont(logFnt, 1,      Color.HSV(1, 0, 1));
			
			// Ensure the text is on the UI render layer
			styleDiag .RenderLayer = RenderLayer.UI;
			styleInfo .RenderLayer = RenderLayer.UI;
			styleWarn .RenderLayer = RenderLayer.UI;
			styleErr  .RenderLayer = RenderLayer.UI;
			styleGraph.RenderLayer = RenderLayer.UI;

			// Share the line spacing across every log style.
			styleDiag.LineHeightPct = lineSpacing;
			styleInfo.LineHeightPct = lineSpacing;
			styleWarn.LineHeightPct = lineSpacing;
			styleErr .LineHeightPct = lineSpacing;

			// One line baseline to baseline; rowPad tops a single entry up to a full
			// line (SizeLayout only returns the layout height), matching wrapped lines.
			float line = styleInfo.TotalHeight * lineSpacing;
			rowPad     = line - styleInfo.LayoutHeight;
			viewHeight = line * viewLines;
			badgeSize  = line * badgeLines;
			return true;
		}
		public void Shutdown()
			=> Log.Unsubscribe(OnLog);
		

		void OnLog(LogLevel level, string logText)
		{
			// Collapse a repeat of the previous line into a count on that entry.
			int last = items.Count - 1;
			if (last >= 0 && items[last].text == logText)
			{
				LogItem item = items[last];
				item.count++;
				items[last] = item;
				return;
			}
			// New items append at the bottom, so existing scroll offsets stay valid;
			// DrawLogs measures the new height and follows it if pinned to the bottom.
			items.Add(new LogItem { level = level, text = logText, count = 1 });
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

		TextStyle StyleFor(LogLevel level) => level switch
		{
			LogLevel.Error      => styleErr,
			LogLevel.Warning    => styleWarn,
			LogLevel.Info       => styleInfo,
			LogLevel.Diagnostic => styleDiag,
			_                   => styleInfo,
		};

		// Fills 'visible' with each item that intersects the viewport, walking the
		// content from the oldest (top) to newest (bottom). Each line stores only
		// its top offset; clipping at both viewport edges is handled at draw time
		// (see DrawLogs), so partially visible lines at the top and bottom are kept.
		void RebuildVisible()
		{
			visible.Clear();
			float o = 0; // Content offset (m) of the current item's top.
			for (int i = 0; i < items.Count; i++)
			{
				float h    = items[i].height;
				float topY = scroll - o; // Item top, viewport-relative (0 = top, down is -).
				o += h;

				if (topY >= h)           continue; // Entirely above the viewport.
				if (topY <= -viewHeight) break;    // Entirely below the viewport.

				visible.Add(new VisibleLine { index = i, y = topY });
			}
		}

		void DrawLogs()
		{
			float width     = UI.LayoutRemaining.x;
			float sliderW   = UI.LineHeight*0.6f;
			float textWidth = width - sliderW; // Stop short of the slider.

			// Measure & cache the layout height of any item not yet measured (or all
			// of them if the width changed, since wrapping depends on it), keeping
			// contentHeight and scrollMax current. Cheap unless something changed.
			if (textWidth != measureWidth) { measuredCount = 0; measureWidth = textWidth; contentHeight = 0; }
			if (measuredCount != items.Count)
			{
				for (int i = measuredCount; i < items.Count; i++)
				{
					LogItem item = items[i];
					item.height   = Text.SizeLayout(item.text, StyleFor(item.level), textWidth).y + rowPad;
					items[i]      = item;
					contentHeight += item.height;
				}
				measuredCount = items.Count;
				scrollMax     = Math.Max(0, contentHeight - viewHeight);
				viewDirty     = true;
			}

			if (stickBottom) scroll = scrollMax;

			UI.LayoutPushCut(UICut.Top, viewHeight);
			UI.LayoutPushCut(UICut.Right, sliderW);
			// The slider handle tracks the content directly: bottom of the track
			// is the newest line, top is the oldest.
			if (UI.VSlider("scroll", ref scroll, 0, scrollMax))
				stickBottom = scroll >= scrollMax - 0.0001f;
			UI.LayoutPop();

			// Rebuild the cached visible list only when the scroll or content moved.
			scroll = Math.Clamp(scroll, 0, scrollMax);
			if (scroll != cachedScroll || viewDirty)
			{
				RebuildVisible();
				cachedScroll = scroll;
				viewDirty    = false;
			}

			Vec3 start = UI.LayoutAt;
			UI.LayoutReserve(V.XY(width, viewHeight));

			// Text.Add's 'size' is also its clip rectangle, and it's computed from
			// the pivot *before* offX/offY are applied - so the offset slides the text
			// independently of the clip box. We anchor the box to the whole viewport
			// and push each line into place with offY, which clips top *and* bottom in
			// one uniform top-anchored pass. The badge shares the text's plane so it
			// can't parallax-drift against it.
			float  z        = start.z - 0.004f;
			Matrix textTr   = Matrix.T(start.x,             start.y, z);
			Matrix badgeTr  = Matrix.T(start.x - textWidth, start.y, z);
			Vec2   textBox  = V.XY(textWidth, viewHeight);
			Vec2   badgeBox = V.XY(badgeSize, viewHeight);
			foreach (VisibleLine line in visible)
			{
				LogItem item = items[line.index];
				Text.Add(item.text, textTr, textBox, TextFit.Clip | TextFit.Wrap, StyleFor(item.level), Pivot.TopLeft, Align.TopLeft, 0, line.y);

				// Right-aligned duplicate-count badge, just inside the slider.
				if (item.count > 1)
					Text.Add(item.count.ToString(), badgeTr, badgeBox, TextFit.Clip, styleInfo, Pivot.TopRight, Align.TopRight, 0, line.y);
			}
			UI.LayoutPop();
		}

		void DrawGraphs()
		{
			float frameMs = 1000.0f / Device.DisplayRefreshRate;
			fpsGraph.min      = frameMs * 0.5f;
			fpsGraph.max      = frameMs * 1.5f;
			cpuPerfGraph.max  = frameMs * 1.5f;
			gpuPerfGraph.max  = frameMs * 1.5f;

			float thirdWidth = (UI.LayoutRemaining.x - UI.Settings.gutter*2) / 3.0f;
			Vec2  graphSize  = V.XY(thirdWidth, UI.LineHeight);

			void Graph(ref DiagGraph graph, string label, float ms)
			{
				graph.Add(ms);
				graph.Draw(styleGraph, label, UI.LayoutAt + V.XYZ(0,0,-0.004f), graphSize);
				UI.LayoutReserve(graphSize);
				UI.SameLine();
			}
			Graph(ref fpsGraph,     "Frame (ms)", Time.Stepf*1000);
			Graph(ref cpuPerfGraph, "CPU (ms)",   Time.PerfCPUus / 1000.0f);
			Graph(ref gpuPerfGraph, "GPU (ms)",   Time.PerfGPUus / 1000.0f);
		}

		struct DiagGraph
		{
			float[]     buffer;
			LinePoint[] points;
			int         curr;
			public float min;
			public float max;
			float       avg;

			public DiagGraph(float min, float max)
			{
				this.min = min;
				this.max = max;
				curr = 0;
				avg  = 0;

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
				avg = avg + (value - avg) * 0.1f;
			}

			public void Draw(TextStyle style, string label, Vec3 at, Vec2 size)
			{
				const float labelWidth = 0.01f;
				float       textScale  = size.y * 0.15f;
				size.x -= labelWidth;

				float step = 1f/(points.Length-1);
				for (int i = 0; i < points.Length; i++)
				{
					float pct = i * step;
					int   idx = (curr + i) % points.Length;
					float val = Math.Clamp((buffer[idx] - min) / (max - min), 0.0f, 1.0f);
					points[i].pt = new Vec3(at.x - pct*size.x, at.y-size.y + val*size.y, at.z);
				}
				Lines.Add(points);

				string display = $"{label} {avg:F3}";
				Text.Add(((int)max).ToString(), Matrix.TS(at.x-size.x, at.y,        at.z, textScale), V.XY(labelWidth*textScale, 1), TextFit.Overflow, style, Pivot.TopLeft,    Align.TopLeft);
				Text.Add(((int)min).ToString(), Matrix.TS(at.x-size.x, at.y-size.y, at.z, textScale), V.XY(labelWidth*textScale, 1), TextFit.Overflow, style, Pivot.BottomLeft, Align.BottomLeft);
				Text.Add(display,               Matrix.TS(at.x,        at.y,        at.z, textScale),                                                  style, Pivot.TopLeft,    Align.TopLeft);
				Lines.Add(at+V.XY0(-Text.SizeLayout(display, style).x*textScale - 0.002f, 0), at + V.XY0(-size.x, 0), new Color32(255,255,255,20), 0.001f);
				Lines.Add(at+V.XY0(0, -size.y), at + V.XY0(-size.x, -size.y), new Color32(255, 255, 255, 20), 0.001f);
			}
		}
	}
}
