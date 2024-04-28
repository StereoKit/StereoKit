// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoRayMesh : ITest
{
	string title       = "Ray to Mesh";
	string description = "";

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
	Pose boxPose    = (Demo.contentPose * Matrix.T(0, -0.1f, 0)).Pose;
	Pose castPose   = (Demo.contentPose * Matrix.T(0.25f, 0.11f, 0.2f)).Pose;

	public void StepRayMesh()
	{
		// Draw our setup, and make the visuals grab/moveable!
		UI.Handle("Box",  ref boxPose,  boxMesh.Bounds);
		UI.Handle("Cast", ref castPose, sphereMesh.Bounds*0.03f);
		boxMesh   .Draw(Default.MaterialUI, boxPose .ToMatrix());
		sphereMesh.Draw(Default.MaterialUI, castPose.ToMatrix(0.03f));
		Lines.Add(castPose.position, boxPose.position, Color.White, 0.005f);

		// Create a ray that's in the Mesh's model space
		Matrix transform = boxPose.ToMatrix();
		Ray    ray       = transform
			.Inverse
			.Transform(Ray.FromTo(castPose.position, boxPose.position));

		// Draw a sphere at the intersection point, if the ray intersects 
		// with the mesh.
		if (ray.Intersect(boxMesh, Cull.Back, out Ray at, out uint index))
		{
			sphereMesh.Draw(Default.Material, Matrix.TS(transform.Transform(at.position), 0.01f));
			if (boxMesh.GetTriangle(index, out Vertex a, out Vertex b, out Vertex c))
			{
				Vec3 aPt = transform.Transform(a.pos);
				Vec3 bPt = transform.Transform(b.pos);
				Vec3 cPt = transform.Transform(c.pos);
				Lines.Add(aPt, bPt, new Color32(0,255,0,255), 0.005f);
				Lines.Add(bPt, cPt, new Color32(0,255,0,255), 0.005f);
				Lines.Add(cPt, aPt, new Color32(0,255,0,255), 0.005f);
			}
		}

	}
	/// :End:

	public void Initialize() {
		Tests.Screenshot("RayMeshIntersect.jpg", 0, 600, 600, 78,
			Demo.contentPose * new Vec3(-0.198f, 0.107f, -0.361f),
			Demo.contentPose * new Vec3(-0.046f, -0.200f, 0.578f));
	}

	public void Step()
	{
		StepRayMesh();
		Demo.ShowSummary(title, description, new Bounds(V.XYZ(-0.08f,-0.04f,-0.06f), V.XYZ(.44f, .36f, .36f)));
	}

	public void Shutdown  () { }
}