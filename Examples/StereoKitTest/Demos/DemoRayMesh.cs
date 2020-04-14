using StereoKit;

class DemoRayMesh : ITest
{
	Mesh sphereMesh = Default.MeshSphere;
	Mesh boxMesh  = Mesh.GenerateRoundedCube(Vec3.One*0.2f, 0.05f);
	Pose boxPose  = new Pose(0,     0,     -0.5f,  Quat.Identity);
	Pose castPose = new Pose(0.25f, 0.21f, -0.36f, Quat.Identity);

	public void Update()
	{
		// Draw our setup, and make the visuals grab/moveable!
		UI.Affordance("Box",  ref boxPose,  boxMesh.Bounds);
		UI.Affordance("Cast", ref castPose, sphereMesh.Bounds*0.03f);
		boxMesh   .Draw(Default.MaterialUI, boxPose .ToMatrix());
		sphereMesh.Draw(Default.MaterialUI, castPose.ToMatrix(0.03f));
		Lines.Add(castPose.position, boxPose.position, Color.White, 0.01f);

		// Create a ray that's in the Mesh's model space
		Matrix transform = boxPose.ToMatrix();
		Ray ray = transform.Inverse() 
			* Ray.FromTo(castPose.position, boxPose.position);

		// Draw a sphere at the intersection point, if the ray intersects 
		// with the mesh.
		if (ray.Intersect(boxMesh, out Vec3 at))
		{
			sphereMesh.Draw(Default.Material, Matrix.TS(transform * at, 0.02f));
		}
	}

	public void Initialize() {
		Tests.Screenshot(600, 600, "RayMeshIntersect.jpg", new Vec3(0.2f, 0.16f, -0.192f), new Vec3(-0.036f, -0.021f, -1.163f));
	}

	public void Shutdown  () { }
}