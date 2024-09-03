using StereoKit;
using StereoKit.Framework;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;

class DemoAnim<T>
{
	class TimeCompare : IComparer<(float time, T data)>
		{ public int Compare((float time, T data) a, (float time, T data) b) => a.time.CompareTo(b.time); }

	TimeCompare _comparer = new TimeCompare();
	(float time,T data)[] _frames;
	Func<T,T,float,T>     _lerp;
	float                 _startTime;
	float                 _updated;
	float                 _speed = 1;
	T                     _curr;

	public T Current { get {
		float now = Time.Totalf;
		if (now == _updated)
			return _curr;

		float elapsed = (now - _startTime) * _speed;
		_updated = now;
		_curr = Sample(elapsed);

		return _curr;
	} }

	public bool Playing => (Time.Totalf - _startTime) <= _frames[_frames.Length-1].time;
	public float Duration => _frames[_frames.Length - 1].time;

	public DemoAnim(Func<T, T, float, T> lerp, params (float,T)[] frames)
	{
		_frames    = frames;
		_lerp      = lerp;
		_startTime = Time.Totalf;
	}

	public void Play(float speed = 1)
	{
		_speed = speed;
		_startTime = Time.Totalf;
	}

	T Sample(float time)
	{
		if (time <= _frames[0].time)
			return  _frames[0].data;
		if (time >= _frames[_frames.Length - 1].time)
			return  _frames[_frames.Length - 1].data;

		int item = Array.BinarySearch(_frames, (time,default(T)), _comparer);
		if (item > 0)
			return _frames[item].data;
		else
		{
			item = ~item;
			var   p1  = _frames[item - 1];
			var   p2  = _frames[item];
			float pct = (time - p1.time) / (p2.time - p1.time);
			return _lerp(p1.data, p2.data, pct);
		}
	}
}

class DebugToolWindow : IStepper
{
	public bool Enabled => true;

	List<(float time, Pose        pose)> recordingHead = new List<(float time, Pose        pose)>();
	List<(float time, HandJoint[] pose)> recordingHand = new List<(float time, HandJoint[] pose)>();
	bool recordHead   = false;
	bool recordHand   = false;
	bool showRuler    = false;
	int  screenshotId = 1;

	AvatarSkeleton      skeleton = null;
	ReadyPlayerMeAvatar avatar   = null;
	ThemeEditor         theme    = null;
	RenderCamera        camera   = null;

	DemoAnim<Pose>        headAnim = null;
	DemoAnim<HandJoint[]> handAnim = null;

	HandMenuRadial handMenu;

	HandMenuItem itemDemos;
	HandMenuItem itemConsole;
	HandMenuItem itemToolsSkeleton;
	HandMenuItem itemToolsAvatar;
	HandMenuItem itemToolsTheme;
	HandMenuItem itemToolsCamera;
	HandMenuItem itemToolsRuler;
	HandMenuItem itemRecordHead;
	HandMenuItem itemRecordHand;

	public bool Initialize()
	{
		itemDemos         = new HandMenuItem("Demos",        Sprite.ToggleOn,  () => Program.WindowDemoShow = !Program.WindowDemoShow);
		itemConsole       = new HandMenuItem("Console",      Sprite.ToggleOn,  () => Program.WindowConsoleShow = !Program.WindowConsoleShow);
		itemToolsAvatar   = new HandMenuItem("Avatar",       Sprite.ToggleOff, () => ToggleAvatar() );
		itemToolsSkeleton = new HandMenuItem("Skeleton",     Sprite.ToggleOff, () => skeleton = ToggleTool(skeleton) );
		itemToolsTheme    = new HandMenuItem("Theme Editor", Sprite.ToggleOff, () => theme    = ToggleTool(theme) );
		itemToolsCamera   = new HandMenuItem("Camera",       Sprite.ToggleOff, () => camera   = ToggleTool(camera, () => new RenderCamera(UI.PopupPose(), 1000, 1000)) );
		itemToolsRuler    = new HandMenuItem("Ruler",        Sprite.ToggleOff, () => {showRuler = !showRuler; rulerPose = UI.PopupPose();} );
		itemRecordHead    = new HandMenuItem("Record Head",  Sprite.ToggleOff, () => ToggleRecordHead() );
		itemRecordHand    = new HandMenuItem("Record Hand",  Sprite.ToggleOff, () => ToggleRecordHand() );

		handMenu = new HandMenuRadial(
			new HandRadialLayer("Debug",
				itemDemos,
				itemConsole,
				new HandMenuItem("Tools",     null, null, "Tools"),
				new HandMenuItem("Print",     null, null, "Print"),
				new HandMenuItem("Recording", null, null, "Recording")
				),
			new HandRadialLayer("Tools",
				itemToolsAvatar,
				itemToolsSkeleton,
				itemToolsTheme,
				itemToolsCamera,
				itemToolsRuler,
				new HandMenuItem("Back", null, null, HandMenuAction.Back)
				),
			new HandRadialLayer("Print",
				new HandMenuItem("R Hand Pose",   null, () => HandshotPose(Handed.Right) ),
				new HandMenuItem("L Hand Pose",   null, () => HandshotPose(Handed.Left) ),
				new HandMenuItem("R Finger",      null, () => Log.Info(Input.Hand(Handed.Right)[FingerId.Index, JointId.Tip].position.ToString())),
				new HandMenuItem("L Finger",      null, () => Log.Info(Input.Hand(Handed.Left )[FingerId.Index, JointId.Tip].position.ToString())),
				new HandMenuItem("Cam Screenshot",null, () => CameraPose()),
				new HandMenuItem("Back",          null, null, HandMenuAction.Back)
				),
			new HandRadialLayer("Recording",
				itemRecordHead,
				new HandMenuItem("Play Head",   null, () => headAnim.Play()),
				itemRecordHand,
				new HandMenuItem("Play Hand",   null, () => handAnim.Play()),
				new HandMenuItem("Screenshot",  null, () => HeadshotPose()),
				new HandMenuItem("Back",        null, null, HandMenuAction.Back)
				)
			);

		SK.AddStepper(handMenu);

		return true;
	}

	public void Shutdown()
	{
		SK.RemoveStepper(handMenu);
		SK.RemoveStepper(skeleton);
		SK.RemoveStepper(theme);
		SK.RemoveStepper(camera);
		SK.RemoveStepper(avatar);
	}

	public void Step()
	{
		itemDemos        .image = Program.WindowDemoShow    ? Sprite.ToggleOn : Sprite.ToggleOff;
		itemConsole      .image = Program.WindowConsoleShow ? Sprite.ToggleOn : Sprite.ToggleOff;
		itemToolsAvatar  .image = avatar   != null ? Sprite.ToggleOn : Sprite.ToggleOff;
		itemToolsSkeleton.image = skeleton != null ? Sprite.ToggleOn : Sprite.ToggleOff;
		itemToolsTheme   .image = theme    != null ? Sprite.ToggleOn : Sprite.ToggleOff;
		itemToolsCamera  .image = camera   != null ? Sprite.ToggleOn : Sprite.ToggleOff;
		itemToolsRuler   .image = showRuler  ? Sprite.ToggleOn : Sprite.ToggleOff;
		itemRecordHead   .image = recordHead ? Sprite.ToggleOn : Sprite.ToggleOff;
		itemRecordHand   .image = recordHand ? Sprite.ToggleOn : Sprite.ToggleOff;

		TakeScreenshots();
		if (recordHead)  RecordHead();
		if (recordHand)  RecordHand();
		if (headAnim != null && headAnim.Playing)
			Renderer.CameraRoot = headAnim.Current.ToMatrix();
		if (handAnim != null && handAnim.Playing)
			Input.HandOverride(Handed.Right, handAnim.Current);
		else
			Input.HandClearOverride(Handed.Right);
		if (showRuler) RulerWindow();

		ScreenshotPreview();
	}

	static T ToggleTool<T>(T stepper, Func<T> makeT = null) where T : IStepper
	{
		if (stepper == null)
			return makeT == null ? SK.AddStepper<T>() : SK.AddStepper(makeT());
		else
		{
			SK.RemoveStepper(stepper);
			return default(T);
		}
	}

	void ToggleAvatar()
	{
		if (avatar == null)
			Platform.FilePicker(PickerMode.Open, (filename) => avatar = SK.AddStepper(new ReadyPlayerMeAvatar(Model.FromFile(filename))), null, Assets.ModelFormats);
		else
		{
			SK.RemoveStepper(avatar);
			avatar = null;
		}
	}

	void TakeScreenshots()
	{
		if (Input.Key(Key.Printscreen).IsJustActive() || Input.Key(Key.F9).IsJustActive())
		{
			Renderer.Screenshot("Screenshot" + screenshotId + ".jpg", Input.Head.position, Input.Head.Forward, 1920 * 2, 1080 * 2);
			screenshotId += 1;
		}
	}

	void RecordHead()
	{
		Pose prev = recordingHead[recordingHead.Count - 1].pose;
		Quat diff = Quat.Delta(Input.Head.orientation, prev.orientation);
		if (Vec3.DistanceSq(Input.Head.position, prev.position) > (2 * U.cm * 2 * U.cm) ||
			(diff.w) * (diff.w) < (0.8f * 0.8f))
			recordingHead.Add((Time.Totalf, Input.Head));

	}
	void ToggleRecordHead()
	{
		recordingHead.Add((Time.Totalf, Input.Head));
		recordHead = !recordHead;
		if (!recordHead)
		{
			float  rootTime = recordingHead[0].time;
			string result = "DemoAnim<Pose> anim = new DemoAnim<Pose>(Pose.Lerp,";
			for (int i = 0; i < recordingHead.Count; i++)
			{
				recordingHead[i] = (recordingHead[i].time - rootTime, recordingHead[i].pose);
				var f = recordingHead[i];
				result += $"({f.time}f, new Pose(V.XYZ({f.pose.position.x:0.000}f,{f.pose.position.y:0.000}f,{f.pose.position.z:0.000}f), new Quat({f.pose.orientation.x:0.000}f,{f.pose.orientation.y:0.000}f,{f.pose.orientation.z:0.000}f,{f.pose.orientation.w:0.000}f)))";
				if (i < recordingHead.Count-1)
					result += ",";
			}
			result += ");";
			Log.Info(result);

			headAnim = new DemoAnim<Pose>(Pose.Lerp, recordingHead.ToArray());
			recordingHead.Clear();
		}
	}

	void RecordHand()
	{
		if (recordingHand.Count != 0 && Time.Totalf - recordingHand[recordingHand.Count - 1].time < 0.2f)
			return;

		Hand        h      = Input.Hand(Handed.Right);
		HandJoint[] joints = new HandJoint[27];
		Array.Copy(h.fingers,0,joints,0,25);
		joints[25] = new HandJoint(h.palm.position,  h.palm.orientation,  0);
		joints[26] = new HandJoint(h.wrist.position, h.wrist.orientation, 0);
		recordingHand.Add((Time.Totalf, joints));
	}
	void ToggleRecordHand()
	{
		recordHand = !recordHand;
		if (recordHand) return;

		float         rootTime = recordingHand[0].time;
		StringBuilder result   = new StringBuilder();
		result.Append("DemoAnim<HandJoint[]> anim = new DemoAnim<HandJoint[]>(JointsLerp,");
		for (int i = 0; i < recordingHand.Count; i++)
		{
			recordingHand[i] = (recordingHand[i].time - rootTime, recordingHand[i].pose);
			var f = recordingHand[i];
			result.Append( $"({f.time}f,new HandJoint[]{{" );
			for (int j = 0; j < f.pose.Length; j++)
			{
				result.Append($"new HandJoint(new Vec3({f.pose[j].position.x:0.000}f,{f.pose[j].position.y:0.000}f,{f.pose[j].position.z:0.000}f), new Quat({f.pose[j].orientation.x:0.000}f,{f.pose[j].orientation.y:0.000}f,{f.pose[j].orientation.z:0.000}f,{f.pose[j].orientation.w:0.000}f), {f.pose[j].radius:0.000}f)");
				if (j < f.pose.Length - 1)
					result.Append(",");
			}
			result.Append("})");
			if (i < recordingHand.Count - 1)
				result.Append(",");
		}
		result.Append(");");
		Log.Info(result.ToString());

		handAnim = new DemoAnim<HandJoint[]>(JointsLerp, recordingHand.ToArray());
		recordingHand.Clear();
	}

	void HeadshotPose()
	{
		Vec3 pos = Input.Head.position + Input.Head.Forward * 10 * U.cm;
		Vec3 fwd = pos + Input.Head.Forward;
		Log.Info($"Tests.Screenshot(\"image.jpg\", 600, 600, new Vec3({pos.x:0.000}f, {pos.y:0.000}f, {pos.z:0.000}f), new Vec3({fwd.x:0.000}f, {fwd.y:0.000}f, {fwd.z:0.000}f));");
		PreviewScreenshot(pos, fwd);
	}

	void CameraPose()
	{
		if (camera == null)
		{
			Log.Warn("Camera should be enabled before printing screenshot pose.");
			return;
		}
		Matrix toLocal = Demo.contentPose.Inverse;
		Vec3 pos = toLocal *  camera.CamAt;
		Vec3 fwd = toLocal * (camera.CamAt + camera.CamDir);
		Log.Info($"Tests.Screenshot(\"image.jpg\", 0, 600, 600, {(int)camera.Fov},\n\tDemo.contentPose * new Vec3({pos.x:0.000}f, {pos.y:0.000}f, {pos.z:0.000}f),\n\tDemo.contentPose * new Vec3({fwd.x:0.000}f, {fwd.y:0.000}f, {fwd.z:0.000}f));");
	}

	void HandshotPose(Handed hand)
	{
		Hand h = Input.Hand(hand);
		HandJoint[] joints = new HandJoint[27];
		Array.Copy(h.fingers, 0, joints, 0, 25);
		joints[25] = new HandJoint(h.palm.position, h.palm.orientation, 0);
		joints[26] = new HandJoint(h.wrist.position, h.wrist.orientation, 0);
		recordingHand.Add((Time.Totalf, joints));

		string result = ($"Tests.Hand(new HandJoint[]{{");
		for (int j = 0; j < joints.Length; j++)
		{
			result += $"new HandJoint(V.XYZ({joints[j].position.x:0.000}f,{joints[j].position.y:0.000}f,{joints[j].position.z:0.000}f), new Quat({joints[j].orientation.x:0.000}f,{joints[j].orientation.y:0.000}f,{joints[j].orientation.z:0.000}f,{joints[j].orientation.w:0.000}f), {joints[j].radius:0.000}f)";
			if (j < joints.Length - 1)
				result += ",";
		}
		result+="});";
		Log.Info(result);
	}

	public static HandJoint[] JointsLerp(HandJoint[] a, HandJoint[] b, float t)
	{
		HandJoint[] result = new HandJoint[a.Length];
		for (int i = 0; i < a.Length; i++) { 
			result[i].position    = Vec3  .Lerp (a[i].position,    b[i].position,    t);
			result[i].orientation = Quat  .Slerp(a[i].orientation, b[i].orientation, t);
			result[i].radius      = SKMath.Lerp (a[i].radius,      b[i].radius,      t);
		}
		return result;
	}

	Sprite screenshot;
	Pose   screenshotPose;
	bool   screenshotVisible;
	void PreviewScreenshot(Vec3 from, Vec3 at)
	{
		string path = Path.GetTempFileName();
		path = Path.ChangeExtension(path, "jpg");
		Renderer.Screenshot(path, from, at, 600, 600);
		Task.Run(()=> { 
			Task.Delay(1000).Wait();
			screenshot = Sprite.FromTex(Tex.FromFile(path));
		});

		if (!screenshotVisible) { 
			screenshotPose.position    = Input.Head.position + Input.Head.Forward * 0.3f;
			screenshotPose.orientation = Quat.LookAt(screenshotPose.position, Input.Head.position);
		}
		screenshotVisible = true;
	}
	void ScreenshotPreview()
	{
		if (!screenshotVisible)
			return;

		UI.WindowBegin("Screenshot", ref screenshotPose, new Vec2(20,0)*U.cm);
		if (screenshot != null)
			UI.Image(screenshot, new Vec2(18,0) * U.cm);
		if (UI.Button("Close"))
		{ 
			screenshotVisible = false;
			screenshot = null;
		}
		UI.WindowEnd();
	}

	
	//////////////////
	// Ruler object //
	//////////////////

	static Pose     rulerPose   = new Pose(0, 0, .5f, Quat.Identity);
	static string[] rulerLabels = { "0", "5", "10", "15", "20", "25" };
	static void RulerWindow()
	{
		UI.HandleBegin("Ruler", ref rulerPose, new Bounds(new Vec3(31,4,1)*U.cm), true);
		Color32 color = Color.HSV(.6f, 0.5f, 1);
		Text.Add("Centimeters", Matrix.TS(new Vec3(14.5f, -1.5f, -.6f)*U.cm, .3f), TextAlign.BottomLeft);
		for (int d = 0; d <= 60; d+=1)
		{
			float x    = d/2.0f;
			float size = d%2==0?.5f:0.15f;
			if (d%10 == 0 && d/2 != 30) {
				size = 1;
				Text.Add(rulerLabels[d/10], Matrix.TS(new Vec3(15-x-0.1f, 2-size, -.6f) * U.cm, .2f), TextAlign.BottomLeft);
			}
			Lines.Add(new Vec3(15-x, 1.8f, -.6f)*U.cm, new Vec3(15-x,1.8f-size, -.6f)*U.cm, color, U.mm*0.5f);
		}
		UI.HandleEnd();
	}
}