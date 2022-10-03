using StereoKit;

class DocQuat : ITest
{
	Pose spherePose;
	Quat sphereDelta;
	Vec3 oldPalmPos;

	Material matDev;

	public void Initialize() 
	{
		spherePose = new Pose(-1,0,1, Quat.Identity);
		sphereDelta = Quat.Identity;

		matDev = Material.Default.Copy();
		matDev.SetTexture("diffuse", Tex.DevTex);
	}

	public void Shutdown() { }

	public void Update()
	{
		/// :CodeSample: Quat Quat.LookAt
		/// Quat.LookAt and LookDir are probably one of the easiest ways to
		/// work with quaternions in StereoKit! They're handy functions to
		/// have a good understanding of. Here's an example of how you might
		/// use them.
		
		// Draw a box that always rotates to face the user
		Vec3 boxPos = new Vec3(1,0,1);
		Quat boxRot = Quat.LookAt(boxPos, Input.Head.position);
		Mesh.Cube.Draw(Material.Default, Matrix.TR(boxPos, boxRot));

		// Make a Window that faces a user that enters the app looking
		// Forward.
		Pose winPose = new Pose(0,0,-0.5f, Quat.LookDir(0,0,1));
		UI.WindowBegin("Posed Window", ref winPose);
		UI.WindowEnd();

		/// :End:


		/// :CodeSample: Quat.Delta
		
		// Draw a sphere that you can spin around with your right hand!
		Vec3 palmPos = Input.Hand(1).palm.position - spherePose.position;
		if (palmPos.Length < 0.3f)
		{
			sphereDelta = Quat.Delta(oldPalmPos.Normalized, palmPos.Normalized);
		}
		spherePose.orientation = sphereDelta * spherePose.orientation;
		oldPalmPos = palmPos;
		Mesh.Sphere.Draw(matDev, spherePose.ToMatrix(0.5f));

		/// :End:
	}
}