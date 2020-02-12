using StereoKit;
using System;
using System.Collections.Generic;

class DemoAnim<T>
{
    class TimeCompare : IComparer<(float time, T data)>
        { public int Compare((float time, T data) a, (float time, T data) b) => a.time.CompareTo(b.time); }

    TimeCompare _comparer = new TimeCompare();
    (float time,T data)[] _frames;
    Func<T,T,float,T>     _lerp;
    float                 _startTime;
    float                 _updated;
    T                     _curr;

    public T Current { get {
        float now = Time.Totalf;
        if (now == _updated)
            return _curr;

        float elapsed = now - _startTime;
        _updated = now;
        _curr = Sample(elapsed);

        return _curr;
    } }

    public bool Playing => (Time.Totalf - _startTime) <= _frames[_frames.Length-1].time;

    public DemoAnim(Func<T, T, float, T> lerp, params (float,T)[] frames)
    {
        _frames    = frames;
        _lerp      = lerp;
        _startTime = Time.Totalf;
    }

    public void Play()
    {
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
    static List<(float time, Pose pose)> recording = new List<(float time, Pose pose)>();
    static bool recordOn     = false;
    static Pose pose         = new Pose(0, 0.2f, -.5f, Quat.LookAt(new Vec3(0, 0.2f, -.5f), Vec3.Zero));
    static bool screenshots  = false;
    static int  screenshotId = 1;

    static DemoAnim<Pose> headAnim = new DemoAnim<Pose>(Pose.Lerp,  
        (0, new Pose(new Vec3(0, 0.2f, 0.4f), Quat.LookAt(new Vec3(0, 0.2f, 0.4f), Vec3.Zero))), 
        (2, new Pose(new Vec3(0, 0.4f, 0.4f), Quat.LookAt(new Vec3(0, 0.4f, 0.4f), Vec3.Zero))),
        (4, new Pose(new Vec3(0, 0.2f, 0.4f), Quat.LookAt(new Vec3(0, 0.2f, 0.4f), Vec3.Zero))));

    public static void Step()
    {
        UI.WindowBegin("Helper", ref pose, new Vec2(20, 0)*Units.cm2m);
        if (UI.Button("Print Screenshot Pose")) HeadshotPose();
        if (UI.Toggle("Record Head", ref recordOn)) ToggleRecordHead();
        if (headAnim != null && UI.Button("Play")) headAnim.Play();
        UI.Toggle("Enable Screenshots", ref screenshots);
        UI.WindowEnd();

        if (screenshots)    TakeScreenshots();
        if (recordOn)       RecordHead();
        if (headAnim.Playing)
            Renderer.SetView(headAnim.Current.ToMatrix());
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
            Renderer.Screenshot(Input.Head.position, Input.Head.Forward, 1920 * 2, 1080 * 2, "Screenshot" + screenshotId + ".jpg");
            screenshotId += 1;
        }
    }

    static void RecordHead()
    {
        Pose prev = recording[recording.Count - 1].pose;
        Quat diff = Quat.Difference(Input.Head.orientation, prev.orientation);
        if (Vec3.DistanceSq(Input.Head.position, prev.position) > (2 * Units.cm2m * 2 * Units.cm2m) ||
            (diff.w) * (diff.w) < (0.8f * 0.8f))
            recording.Add((Time.Totalf, Input.Head));
        
    }

    static void ToggleRecordHead()
    {
        recording.Add((Time.Totalf, Input.Head ));
        if (!recordOn)
        {
            float  rootTime = recording[0].time;
            string result = "DemoAnim<Pose> anim = new DemoAnim<Pose>(Pose.Lerp,";
            for (int i = 0; i < recording.Count; i++)
            {
                recording[i] = (recording[i].time - rootTime, recording[i].pose);
                var f = recording[i];
                result += $"({f.time}f, new Pose(new Vec3({f.pose.position.x}f,{f.pose.position.y}f,{f.pose.position.z}f), new Quat({f.pose.orientation.x}f,{f.pose.orientation.y}f,{f.pose.orientation.z}f,{f.pose.orientation.w}f))),";
            }
            result += ");";
            Log.Info(result);

            headAnim = new DemoAnim<Pose>(Pose.Lerp, recording.ToArray());
            recording.Clear();
        }
    }

    static void HeadshotPose()
    {
        Vec3 pos = Input.Head.position;
        Vec3 fwd = pos + Input.Head.Forward;
        Log.Info($"if (Demos.TestMode) Renderer.Screenshot(new Vec3({pos.x}f, {pos.y}f, {pos.z}f), new Vec3({fwd.x}f, {fwd.y}f, {fwd.z}f), 600, 600, \"../../../docs/img/screenshots/image.jpg\");");
    }
}