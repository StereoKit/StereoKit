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

class DebugToolWindow
{
	static List<(float time, Pose        pose)> recordingHead = new List<(float time, Pose        pose)>();
	static List<(float time, HandJoint[] pose)> recordingHand = new List<(float time, HandJoint[] pose)>();
	static bool recordHead   = false;
	static bool recordHand   = false;
	static Pose pose         = new Pose(0, 0.3f, .5f, Quat.LookAt(new Vec3(0, 0.3f, .5f), new Vec3(0, 0.3f, 0)));
	static bool screenshots  = false;
	static int  screenshotId = 1;
	static AvatarSkeleton avatar = null;

	static DemoAnim<Pose>        headAnim = null;
	static DemoAnim<HandJoint[]> handAnim = null;

	public static void Step()
	{
		UI.WindowBegin("Helper", ref pose, new Vec2(20, 0)*U.cm);
		if (UI.Button("Print Screenshot Pose") || Input.Key(Key.F7).IsJustActive()) HeadshotPose();
		if (UI.Button("Print R Hand Pose") || Input.Key(Key.F8).IsJustActive()) HandshotPose(Handed.Right);
		if (UI.Button("Print L Hand Pose") || Input.Key(Key.F9).IsJustActive()) HandshotPose(Handed.Left);
		if (UI.Button("Print R Finger")) Log.Info(Input.Hand(Handed.Right)[FingerId.Index,JointId.Tip].position.ToString());
		if (UI.Toggle("Record Head", ref recordHead)) ToggleRecordHead();
		if (headAnim != null) {
			UI.SameLine();
			if (UI.Button("Play1")) headAnim.Play();
		}
		if (UI.Toggle("Record Hand", ref recordHand)) ToggleRecordHand();
		if (handAnim != null) {
			UI.SameLine();
			if (UI.Button("Play2")) handAnim.Play();
		}
		UI.Toggle("Enable Screenshots", ref screenshots);
		bool showAvatar = avatar != null;
		if (UI.Toggle("Show Skeleton", ref showAvatar))
		{
			if (showAvatar) avatar = SK.AddStepper<AvatarSkeleton>();
			else { SK.RemoveStepper(avatar); avatar = null; }
		}
		UI.WindowEnd();

		if (screenshots) TakeScreenshots();
		if (recordHead)  RecordHead();
		if (recordHand)  RecordHand();
		if (headAnim != null && headAnim.Playing)
			Renderer.CameraRoot = headAnim.Current.ToMatrix();
		if (handAnim != null && handAnim.Playing)
			Input.HandOverride(Handed.Right, handAnim.Current);
		else
			Input.HandClearOverride(Handed.Right);

		ScreenshotPreview();
	}

	static void TakeScreenshots()
	{
		// Take a screenshot on the first frame both hands are gripped
		bool valid =
			Input.Hand(Handed.Left ).IsTracked &&
			Input.Hand(Handed.Right).IsTracked;
		BtnState right = Input.Hand(Handed.Right).grip;
		BtnState left  = Input.Hand(Handed.Left ).grip;
		if (valid && left.IsActive() && right.IsActive() && (left.IsJustActive() || right.IsJustActive()))
		{
			Renderer.Screenshot("Screenshot" + screenshotId + ".jpg", Input.Head.position, Input.Head.Forward, 1920 * 2, 1080 * 2);
			screenshotId += 1;
		}
	}

	static void RecordHead()
	{
		Pose prev = recordingHead[recordingHead.Count - 1].pose;
		Quat diff = Quat.Difference(Input.Head.orientation, prev.orientation);
		if (Vec3.DistanceSq(Input.Head.position, prev.position) > (2 * U.cm * 2 * U.cm) ||
			(diff.w) * (diff.w) < (0.8f * 0.8f))
			recordingHead.Add((Time.Totalf, Input.Head));

	}
	static void ToggleRecordHead()
	{
		recordingHead.Add((Time.Totalf, Input.Head));
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

	static void RecordHand()
	{
		if (Time.Totalf - recordingHand[recordingHand.Count-1].time > 0.2f) {
			Hand        h      = Input.Hand(Handed.Right);
			HandJoint[] joints = new HandJoint[27];
			Array.Copy(h.fingers,0,joints,0,25);
			joints[25] = new HandJoint(h.palm.position,  h.palm.orientation,  0);
			joints[26] = new HandJoint(h.wrist.position, h.wrist.orientation, 0);
			recordingHand.Add((Time.Totalf, joints));
		}
	}
	static void ToggleRecordHand()
	{
		Hand        h      = Input.Hand(Handed.Right);
		HandJoint[] joints = new HandJoint[27];
		Array.Copy(h.fingers,0,joints,0,25);
		joints[25] = new HandJoint(h.palm.position,  h.palm.orientation,  0);
		joints[26] = new HandJoint(h.wrist.position, h.wrist.orientation, 0);
		recordingHand.Add((Time.Totalf, joints));
		if (!recordHand)
		{
			float  rootTime = recordingHand[0].time;
			StringBuilder result = new StringBuilder();
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
	}

	static void HeadshotPose()
	{
		Vec3 pos = Input.Head.position + Input.Head.Forward * 10 * U.cm;
		Vec3 fwd = pos + Input.Head.Forward;
		Log.Info($"Tests.Screenshot(\"image.jpg\", 600, 600, new Vec3({pos.x:0.000}f, {pos.y:0.000}f, {pos.z:0.000}f), new Vec3({fwd.x:0.000}f, {fwd.y:0.000}f, {fwd.z:0.000}f));");
		PreviewScreenshot(pos, fwd);
	}
	static void HandshotPose(Handed hand)
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

	static Sprite screenshot;
	static Pose   screenshotPose;
	static bool   screenshotVisible;
	static void PreviewScreenshot(Vec3 from, Vec3 at)
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
	static void ScreenshotPreview()
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
}