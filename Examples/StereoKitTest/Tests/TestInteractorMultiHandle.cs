using System;
using StereoKit;

// Validates multi-interactor UI.Handle: 2+ point interactors combine into one
// translate + rotate, can join/leave without the handle jumping, and rotation
// follows where the grab points MOVE (not the interactors' orientations).
//
// Deliberately awkward (these exposed real bugs - don't "simplify" them away):
// the handle rests at a 180-degree turn and is nested in a parent handle with
// its own non-identity pose. Checks use the WORLD pose and verify the grabbed
// direction follows the grab point, so they hold under both.
//
// Run: dotnet run [-test] -start TestInteractorMultiHandle
class TestInteractorMultiHandle : ITest
{
	DefaultInteractors prevDefault;
	Interactor[]       actors;

	// Parent handle (non-identity pose, centered ahead). UIMove.None makes it a
	// nested coordinate frame that can't itself be grabbed.
	Pose parentPose = new Pose(new Vec3(0, 0, -0.5f), Quat.FromAngles(10, 35, 0));
	static readonly Bounds parentBounds = new Bounds(Vec3.Zero, new Vec3(0.1f, 0.1f, 0.1f));

	// The inner multi-interactor handle, in the PARENT's local space. Sits at the
	// parent origin and rests at a 180-degree turn -> a non-identity world rest.
	Pose handlePose;
	Pose worldHandle;   // its world pose, recomputed each frame
	static readonly Pose InnerRestLocal = new Pose(Vec3.Zero, Quat.LookDir(0, 0, 1));
	static readonly Vec3 boxDims        = new Vec3(0.0625f, 0.0625f, 0.0625f);
	Vec3 worldRestCenter;  // world position of the inner handle at rest

	const float grabRadius0 = 0.02f;
	const float grabRadius1 = 0.08f;
	const float ExpectAngle = 90.0f;
	const float PullTilt    = 20.0f;
	static readonly Vec3 TransTo = new Vec3(0.02f, 0.02f, 0.02f);

	struct Cfg
	{
		public float radius, angleY, angleZ, tilt;
		public Vec3  trans;
		public Cfg(float r, float y, float z, float t, Vec3 tr) { radius = r; angleY = y; angleZ = z; tilt = t; trans = tr; }
		public static Cfg Lerp(Cfg a, Cfg b, float t) => new Cfg(
			a.radius + (b.radius - a.radius) * t,
			a.angleY + (b.angleY - a.angleY) * t,
			a.angleZ + (b.angleZ - a.angleZ) * t,
			a.tilt   + (b.tilt   - a.tilt)   * t,
			Vec3.Lerp(a.trans, b.trans, t));
	}

	enum P { Focus, Grab, Pull, Translate, SwingY, NeutralY, SwingZPos, NeutralZ, SwingZNeg, Drop, COUNT }

	static readonly Cfg cfgInitial = new Cfg(grabRadius0, 0, 0, 0, Vec3.Zero);
	static readonly Cfg[] phaseTarget = {
		new Cfg(grabRadius0, 0,            0,            0,        Vec3.Zero), // Focus
		new Cfg(grabRadius0, 0,            0,            0,        Vec3.Zero), // Grab
		new Cfg(grabRadius1, 0,            0,            PullTilt, Vec3.Zero), // Pull
		new Cfg(grabRadius1, 0,            0,            PullTilt, TransTo),   // Translate
		new Cfg(grabRadius1, ExpectAngle,  0,            PullTilt, TransTo),   // SwingY    - +X -> -Z
		new Cfg(grabRadius1, 0,            0,            PullTilt, TransTo),   // NeutralY
		new Cfg(grabRadius1, 0,            ExpectAngle,  PullTilt, TransTo),   // SwingZPos - +X -> +Y
		new Cfg(grabRadius1, 0,            0,            PullTilt, TransTo),   // NeutralZ
		new Cfg(grabRadius1, 0,           -ExpectAngle,  PullTilt, TransTo),   // SwingZNeg - +X -> -Y
		new Cfg(grabRadius1, 0,           -ExpectAngle,  PullTilt, TransTo),   // Drop
	};
	static readonly string[] phaseName = { "focus", "grab", "pull apart", "translate", "swing +90 Y", "neutral", "swing +90 Z", "neutral", "swing -90 Z", "drop one hand" };

	const int   FramesPerPhase  = 10;
	const float SecondsPerPhase = 1.5f;

	int   phase      = 0;
	int   prevPhase  = -1;
	int   phaseFrame = 0;
	float phaseTime  = 0;
	Vec3  expectCenter;
	Vec3  preDropPos;
	Quat  qRest;
	Vec3  localGrabDir;

	public void Initialize()
	{
		prevDefault = Interaction.DefaultInteractors;
		Interaction.DefaultInteractors = DefaultInteractors.None;

		handlePose      = InnerRestLocal;
		worldHandle     = (InnerRestLocal.ToMatrix() * parentPose.ToMatrix()).Pose; // inner world rest pose
		worldRestCenter = worldHandle.position;
		expectCenter    = worldRestCenter;
		qRest           = worldHandle.orientation;
		localGrabDir    = qRest.Inverse * Vec3.Right;

		actors = new Interactor[2];
		for (int i = 0; i < actors.Length; i++)
			actors[i] = Interactor.Create(InteractorType.Point, InteractorEvent.Pinch, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);

		if (Tests.IsTesting)
			Tests.RunForFrames((int)P.COUNT * FramesPerPhase + 2);
	}

	public void Shutdown()
	{
		foreach (Interactor a in actors) a.Destroy();
		Interaction.DefaultInteractors = prevDefault;
	}

	public void Step()
	{
		if (Tests.IsTesting && phase >= (int)P.COUNT) return;

		bool entering = phase != prevPhase;
		prevPhase = phase;
		if (entering && phase == (int)P.Drop) preDropPos = worldHandle.position;

		float progress;
		bool  phaseDone;
		if (Tests.IsTesting)
		{
			phaseFrame++;
			progress  = MathF.Min(1, phaseFrame / (float)FramesPerPhase);
			phaseDone = phaseFrame >= FramesPerPhase;
		}
		else
		{
			phaseTime += Time.Stepf;
			progress   = MathF.Min(1, phaseTime / SecondsPerPhase);
			phaseDone  = phaseTime >= SecondsPerPhase;
		}

		Cfg start = phase == 0 ? cfgInitial : phaseTarget[phase - 1];
		Cfg cfg   = Cfg.Lerp(start, phaseTarget[phase], progress);

		BtnState pinchA, pinchB;
		if (phase == (int)P.Focus)
		{
			pinchA = pinchB = BtnState.Inactive;
		}
		else
		{
			pinchA = (phase == (int)P.Grab && entering) ? (BtnState.Active | BtnState.JustActive) : BtnState.Active;
			pinchB = phase == (int)P.Drop
				? (entering ? BtnState.JustInactive : BtnState.Inactive)
				: pinchA;
		}

		// Grab points are driven in WORLD space around the inner handle's world
		// rest center, regardless of the parent frame it lives in.
		Quat ori    = Quat.FromAngles(cfg.tilt, 0, 0);
		Vec3 center = worldRestCenter + cfg.trans;
		Vec3 offset = Quat.FromAngles(0, cfg.angleY, cfg.angleZ) * new Vec3(cfg.radius, 0, 0);
		expectCenter = center;
		DriveActor(actors[0], center + offset, ori, pinchA);
		DriveActor(actors[1], center - offset, ori, pinchB);

		// Nest the handle in a parent with a non-identity pose. UIMove.None means
		// the parent only supplies the coordinate frame; it never gets grabbed.
		bool grabbed;
		UI.HandleBegin("ParentHandle", ref parentPose, parentBounds, false, UIMove.None);
		{
			grabbed     = UI.Handle("multiHandle", ref handlePose, new Bounds(Vec3.Zero, boxDims), true);
			worldHandle = Hierarchy.ToWorld(handlePose);
		}
		UI.HandleEnd();

		DrawVisuals();
		DrawLabel();

		if (phaseDone)
		{
			if (Tests.IsTesting) CheckPhaseEnd(grabbed);

			phase++;
			phaseFrame = 0;
			phaseTime  = 0;
			if (phase >= (int)P.COUNT && !Tests.IsTesting)
			{
				phase      = 0;
				prevPhase  = -1;
				handlePose = InnerRestLocal;
			}
		}
	}

	void CheckPhaseEnd(bool grabbed)
	{
		switch ((P)phase)
		{
		case P.Grab:
			qRest        = worldHandle.orientation;
			localGrabDir = qRest.Inverse * (actors[0].Motion.position - expectCenter).Normalized;
			Check("grab is stable", Vec3.Distance(worldHandle.position, expectCenter) < 0.01f && grabbed);
			break;

		case P.Pull:
			Check("pull no rotation", RotationDeg(worldHandle.orientation, qRest) < 3.0f);
			Check("pull position",    Vec3.Distance(worldHandle.position, expectCenter) < 0.02f);
			break;

		case P.Translate:
			Check("translate position",    Vec3.Distance(worldHandle.position, expectCenter) < 0.02f);
			Check("translate no rotation", RotationDeg(worldHandle.orientation, qRest) < 3.0f);
			break;

		case P.SwingY:    CheckSwing("swingY",  Vec3.Up);      break;
		case P.NeutralY:  Check("neutralY returns to rest", RotationDeg(worldHandle.orientation, qRest) < 3.0f); break;
		case P.SwingZPos: CheckSwing("swingZ+", Vec3.Forward); break;
		case P.NeutralZ:  Check("neutralZ returns to rest", RotationDeg(worldHandle.orientation, qRest) < 3.0f); break;
		case P.SwingZNeg: CheckSwing("swingZ-", Vec3.Forward); break;

		case P.Drop:
			Check("no jump on drop", Vec3.Distance(worldHandle.position, preDropPos) < 0.01f && grabbed);
			break;
		}
	}

	// Verifies a swing independently of rest orientation and parent frame: the
	// handle's grabbed direction must now point where the grab point moved to.
	void CheckSwing(string name, Vec3 worldAxis)
	{
		Quat  q          = worldHandle.orientation;
		Vec3  grabDirNow = (actors[0].Motion.position - expectCenter).Normalized;
		Vec3  grabbedDir = q * localGrabDir;
		Vec3  axisDir    = q * (qRest.Inverse * worldAxis);
		float swungDeg   = RotationDeg(q, qRest);

		Log.Info($"TestInteractorMultiHandle {name}: swung {swungDeg:F1}deg; grabbed dir {grabbedDir} vs grab point {grabDirNow} (match => follows hands)");
		Check($"{name} angle ~90",       MathF.Abs(swungDeg - ExpectAngle) < 6.0f);
		Check($"{name} follows grab pt", Vec3.AngleBetween(grabbedDir, grabDirNow) < 6.0f);
		Check($"{name} about axis",      Vec3.AngleBetween(axisDir, worldAxis) < 4.0f);
		Check($"{name} position",        Vec3.Distance(worldHandle.position, expectCenter) < 0.02f);
	}

	void Check(string name, bool passed)
	{
		if (!passed)
			Log.Err($"TestInteractorMultiHandle: '{name}' failed on frame {phaseFrame} of phase {phaseName[phase]} (world pose {worldHandle.position}).");
		Tests.Test(() => passed);
	}

	static readonly Color RedActor  = new Color(1.0f, 0.25f, 0.25f);
	static readonly Color BlueActor = new Color(0.3f, 0.5f,  1.0f);

	void DrawVisuals()
	{
		DrawInteractor(actors[0], RedActor);
		DrawInteractor(actors[1], BlueActor);

		Lines.Add(actors[0].Motion.position, actors[1].Motion.position, new Color(1, 1, 1, 0.4f), 0.0015f);

		// Red line points where the handle THINKS its grabbed (red) face is; if
		// it tracks the red sphere, the handle is following the hands.
		float reach = (actors[0].Motion.position - worldHandle.position).Length;
		Lines.Add(worldHandle.position, worldHandle.position + (worldHandle.orientation * localGrabDir) * reach, RedActor, 0.0025f);
	}

	static void DrawInteractor(Interactor actor, Color color)
	{
		bool active = actor.Active != IdHash.None;
		Mesh.Sphere.Draw(Material.Default, Matrix.TS(actor.Motion.position, 0.012f), active ? color : color * 0.4f);
	}

	void DrawLabel()
	{
		Vec3 at = worldHandle.position + new Vec3(0, boxDims.y * 0.5f + 0.04f, 0);
		Text.Add(phaseName[phase], Matrix.TRS(at, Quat.LookAt(at, Input.Head.position), 0.5f), Pivot.BottomCenter, Align.Center);
	}

	static float RotationDeg(Quat a, Quat b)
	{
		float dot = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
		return 2 * MathF.Acos(MathF.Min(1, MathF.Abs(dot))) * (180.0f / MathF.PI);
	}

	static void DriveActor(Interactor actor, Vec3 pos, Quat ori, BtnState pinch)
		=> actor.Update(pos, pos, new Pose(pos, ori), pos, Vec3.Zero, pinch, BtnState.Active);
}
