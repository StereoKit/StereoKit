﻿using StereoKit;

class DemoRayMesh : ITest
{
	/// :CodeSample: Ray.Intersect Mesh.Intersect
	/// ### Ray Mesh Intersection
	/// Here's an example of casting a Ray at a mesh someplace in world space,
	/// transforming it into model space, calculating the intersection point,
	/// and displaying it back in world space.
	/// 
	/// ![Ray Mesh Intersection]({{site.url}}/img/screenshots/RayMeshIntersect.jpg)
	///
	Mesh sphereMesh = Default.MeshSphere;
	Mesh boxMesh    = Mesh.GenerateRoundedCube(Vec3.One*0.2f, 0.05f);
	Pose boxPose    = new Pose(0,     0,     -0.5f,  Quat.Identity);
	Pose castPose   = new Pose(0.25f, 0.21f, -0.36f, Quat.Identity);
	TextStyle style;
	public void Update()
	{
		// Draw our setup, and make the visuals grab/moveable!
		UI.Handle("Box",  ref boxPose,  boxMesh.Bounds);
		UI.Handle("Cast", ref castPose, sphereMesh.Bounds*0.03f);
		boxMesh   .Draw(Default.MaterialUI, boxPose .ToMatrix());
		sphereMesh.Draw(Default.MaterialUI, castPose.ToMatrix(0.03f));
		Lines.Add(castPose.position, boxPose.position, Color.White, 0.01f);

		// Create a ray that's in the Mesh's model space
		Matrix transform = boxPose.ToMatrix();
		Ray    ray       = transform
			.Inverse
			.Transform(Ray.FromTo(castPose.position, boxPose.position));

		// Draw a sphere at the intersection point, if the ray intersects 
		// with the mesh.
		if (ray.Intersect(boxMesh, out Ray at, out uint index))
		{
			sphereMesh.Draw(Default.Material, Matrix.TS(transform.Transform(at.position), 0.02f));
			Vec3 textTransform = transform.Transform(at.position * 1.5f) + new Vec3(0, 0.05f, 0f);
			if (boxMesh.GetTriangle(index,out Vertex a, out Vertex b, out Vertex c))
			{
				Text.Add($"a-Pos:{a.pos}\nb-Pos:{b.pos}\nc-Pos:{c.pos}", Matrix.TR(textTransform, Quat.LookAt(textTransform, Input.Head.position)), style);
			}
		}
	}
	/// :End:

	public void Initialize() {
		Tests.Screenshot("RayMeshIntersect.jpg", 600, 600, 90, new Vec3(0.2f, 0.16f, -0.192f), new Vec3(-0.036f, -0.021f, -1.163f));
		style = Text.MakeStyle(Font.Default, 0.01f, new Color(1,0.4f,0.4f));
	}

	public void Shutdown  () { }
}