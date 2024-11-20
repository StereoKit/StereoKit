// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoMath : ITest
{
	string title       = "Math";
	string description = "StereoKit has a SIMD optimized math library that provides a wide array of high-level math functions, shapes, and intersection formulas!\n\nIn C#, math types are backed by System.Numerics for easy interop with code from the rest of the C# ecosystem.";

	Mesh suzanne;
	Mesh planeMesh;

	static Vec3 Grid(int i) => new Vec3(-0.3f + (i/2) * 0.2f, -0.1f + i%2 * 0.2f,0);
	Pose posePlaneRay   = new Pose(Demo.contentPose.Transform(Grid(0)));
	Pose poseLinePlane  = new Pose(Demo.contentPose.Transform(Grid(1)));
	Pose poseSphereRay  = new Pose(Demo.contentPose.Transform(Grid(2)));
	Pose poseBoundsRay  = new Pose(Demo.contentPose.Transform(Grid(3)));
	Pose poseBoundsLine = new Pose(Demo.contentPose.Transform(Grid(4)));
	Pose poseMeshRay    = new Pose(Demo.contentPose.Transform(Grid(5)));
	Pose poseCross      = new Pose(Demo.contentPose.Transform(Grid(6)));
	Vec3 screenshotFrom = new Vec3(0.1f, 0.1f, 0.5f);
	Vec3 screenshotAt   = new Vec3(0,0,0);

	public void Step()
	{
		Color colIntersect = Color.HSV(0, 0.8f, 1);
		Color colTest      = Color.HSV(0, 0.6f, 1);
		Color colObj       = Color.HSV(0.05f, 0.7f, 1);
		Color active       = new Color(1,1,1,0.7f);
		Color notActive    = new Color(1, 1, 1, 1);

		// Plane and Ray
		bool planeRayActive = UI.HandleBegin("PlaneRay", ref posePlaneRay, new Bounds(Vec3.One*0.1f));
		Mesh.Cube.Draw(Material.UIBox, Matrix.S(0.1f), planeRayActive ? active:notActive);

		Plane ground    = new Plane(new Vec3(1,2,0), 0);
		Ray   groundRay = new Ray(Vec3.Zero + new Vec3(0,0.05f,0), Vec3.AngleXZ(Time.Totalf*90, -2).Normalized);

		Lines.Add(groundRay.position, groundRay.At(0.1f), new Color32(255, 0, 0, 255), 2 * Units.mm2m);
		planeMesh.Draw(Material.Default, Matrix.TRS(Vec3.Zero, Quat.LookDir(ground.normal), 0.0625f), colObj);
		if (groundRay.Intersect(ground, out Vec3 groundAt))
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(groundAt, 0.005f), colIntersect);

		UI.HandleEnd();

		Tests.Screenshot("RayIntersectPlane.jpg", 0, 400, 400, 20, posePlaneRay.position + screenshotFrom, posePlaneRay.position + screenshotAt);

		// Line and Plane
		bool linePlaneActive = UI.HandleBegin("LinePlane", ref poseLinePlane, new Bounds(Vec3.One * 0.1f));
		Mesh.Cube.Draw(Material.UIBox, Matrix.S(0.1f), linePlaneActive ? active : notActive);

		Plane groundLinePlane = new Plane(new Vec3(1,2,0), 0);
		Ray   groundLineRay   = new Ray  (Vec3.Zero + new Vec3(0,0.0625f,0), Vec3.AngleXZ(Time.Totalf*90, -2).Normalized);
		Vec3  groundLineP1    = groundLineRay.At((SKMath.Cos(Time.Totalf*3)+1)*0.05f);
		Vec3  groundLineP2    = groundLineRay.At((SKMath.Cos(Time.Totalf*3)+1)*0.05f+ 0.025f);

		Lines.Add(groundLineP1, groundLineP2, colTest, 2 * Units.mm2m);
		Mesh.Sphere.Draw(Material.Default, Matrix.TS(groundLineP1, 0.0025f), colTest);
		Mesh.Sphere.Draw(Material.Default, Matrix.TS(groundLineP2, 0.0025f), colTest);
		bool groundLineIntersects = groundLinePlane.Intersect(groundLineP1, groundLineP2, out Vec3 groundLineAt);
		planeMesh.Draw(Material.Default, Matrix.TRS(Vec3.Zero, Quat.LookDir(groundLinePlane.normal), 0.0625f), groundLineIntersects? colIntersect : colObj);
		if (groundLineIntersects)
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(groundLineAt, 0.005f), colIntersect);

		UI.HandleEnd();

		Tests.Screenshot("LineIntersectPlane.jpg", 0, 400, 400, 20, poseLinePlane.position + screenshotFrom, poseLinePlane.position + screenshotAt);

		// Sphere and Ray
		bool sphereRayActive = UI.HandleBegin("SphereRay", ref poseSphereRay, new Bounds(Vec3.One * 0.1f));
		Mesh.Cube.Draw(Material.UIBox, Matrix.S(0.1f), sphereRayActive ? active : notActive);

		Sphere sphere    = new Sphere(Vec3.Zero, 0.0625f);
		Vec3   sphereDir = Vec3.AngleXZ(Time.Totalf*90, SKMath.Cos(Time.Totalf * 3) * 0.375f + 0.05f).Normalized;
		Ray    sphereRay = new Ray(sphere.center - sphereDir * 0.0875f, sphereDir);

		Lines.Add(sphereRay.position, sphereRay.At(0.05f), colTest, 2 * Units.mm2m);
		if (sphereRay.Intersect(sphere, out Vec3 sphereAt))
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(sphereAt, 0.005f), colIntersect);
		Mesh.Sphere.Draw(Material.Default, Matrix.TS(sphere.center, 0.0625f), colObj);

		UI.HandleEnd();

		Tests.Screenshot("RayIntersectSphere.jpg", 0, 400, 400, 20, poseSphereRay.position + screenshotFrom, poseSphereRay.position + screenshotAt);

		// Bounds and Ray
		bool boundsRayActive = UI.HandleBegin("BoundsRay", ref poseBoundsRay, new Bounds(Vec3.One * 0.1f));
		Mesh.Cube.Draw(Material.UIBox, Matrix.S(0.1f), boundsRayActive ? active : notActive);

		Bounds bounds    = new Bounds(Vec3.Zero, Vec3.One * 0.0625f);
		Vec3   boundsDir = Vec3.AngleXZ(Time.Totalf*90, SKMath.Cos(Time.Totalf*3)*0.375f).Normalized;
		Ray    boundsRay = new Ray(bounds.center - boundsDir * 0.0875f, boundsDir);

		Lines.Add(boundsRay.position, boundsRay.At(0.05f), colTest, 2 * Units.mm2m);
		if (boundsRay.Intersect(bounds, out Vec3 boundsAt))
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(boundsAt, 0.005f), colIntersect);
		Mesh.Cube.Draw(Material.Default, Matrix.TS(bounds.center, 0.0625f), colObj);

		UI.HandleEnd();

		Tests.Screenshot("RayIntersectBounds.jpg", 0, 400, 400, 20, poseBoundsRay.position + screenshotFrom, poseBoundsRay.position + screenshotAt);

		// Bounds and Line
		bool boundsLineActive = UI.HandleBegin("BoundsLine", ref poseBoundsLine, new Bounds(Vec3.One * 0.1f));
		Mesh.Cube.Draw(Material.UIBox, Matrix.TS(Vec3.Zero, 0.1f), boundsLineActive ? active : notActive);

		Bounds boundsLine   = new Bounds(Vec3.Zero, Vec3.One * 0.0625f);
		Vec3   boundsLineP1 = boundsLine.center + Vec3.AngleXZ(Time.Totalf*45, SKMath.Cos(Time.Totalf*3)) * 0.0875f;
		Vec3   boundsLineP2 = boundsLine.center + Vec3.AngleXZ(Time.Totalf*90, SKMath.Cos(Time.Totalf*6)) * SKMath.Cos(Time.Totalf)*0.0875f;

		Lines.Add(boundsLineP1, boundsLineP2, colTest, 2*Units.mm2m);
		Mesh.Sphere.Draw(Material.Default, Matrix.TS(boundsLineP1, 0.005f), colTest);
		Mesh.Sphere.Draw(Material.Default, Matrix.TS(boundsLineP2, 0.005f), colTest);
		Mesh.Cube  .Draw(Material.Default, Matrix.TS(boundsLine.center, 0.0625f),
			boundsLine.Contains(boundsLineP1, boundsLineP2) ? colIntersect : colObj);

		UI.HandleEnd();

		Tests.Screenshot("LineIntersectBounds.jpg", 0, 400, 400, 20, poseBoundsLine.position + screenshotFrom, poseBoundsLine.position + screenshotAt);

		// Mesh and Ray
		bool meshRayActive = UI.HandleBegin("MeshRay", ref poseMeshRay, new Bounds(Vec3.One * 0.1f));
		Matrix modelTransform = Matrix.TS(0,-0.01f,0,0.25f);
		Mesh.Cube.Draw(Material.UIBox, Matrix.S(0.1f), meshRayActive ? active : notActive);
		suzanne  .Draw(Material.Default, modelTransform, colObj);

		Vec3 meshDir = Vec3.AngleXZ(Time.Totalf * 90, SKMath.Cos(Time.Totalf * 3) * 0.375f).Normalized;
		Ray  meshRay = Ray.FromTo(meshDir*0.05f, V.XYZ(0,0.01f,0));
		if ((modelTransform.Inverse * meshRay).Intersect(suzanne, out Ray meshAt)) {
			meshAt = modelTransform * meshAt;
			Lines.Add(meshRay.position, meshAt.position, colTest, 2 * Units.mm2m);
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(meshAt.position, 0.005f), colIntersect);
		} else {
			Lines.Add(meshRay.position, meshRay.At(0.1f), colTest, 2 * Units.mm2m);
		}

		UI.HandleEnd();

		//if (Tests.IsTesting)
		//    Renderer.Screenshot(poseBoundsLine.position + new Vec3(0.0f, 0.3f, 0.15f), poseBoundsLine.position, 400, 400, "../../../docs/img/screenshots/LineIntersectBounds.jpg");

		// Cross product
		bool crossActive = UI.HandleBegin("Cross", ref poseCross, new Bounds(Vec3.One * 0.1f));
		Mesh.Cube.Draw(Material.UIBox, Matrix.S(0.1f), crossActive ? active : notActive);

		Vec3 crossStart = V.XYZ(-0.025f, -0.025f, 0.025f);
		//Vec3 right      = Vec3.Cross(Vec3.Forward, Vec3.Up); // These are the same!
		Vec3 right      = Vec3.PerpendicularRight(Vec3.Forward, Vec3.Up);
		Lines.Add(crossStart, crossStart + Vec3.Up*0.05f,      new Color32(255,255,255,255), 2*Units.mm2m);
		Lines.Add(crossStart, crossStart + Vec3.Forward*0.05f, new Color32(255,255,255,255), 2*Units.mm2m);
		Lines.Add(crossStart, crossStart + right * 0.05f,      new Color32(0, 255, 0, 255),  2*Units.mm2m);
		Text.Add("Up",                 Matrix.TRS(crossStart + Vec3.Up      * 0.05f, Quat.LookDir(-Vec3.Forward), 0.25f), TextAlign.BottomCenter);
		Text.Add("Fwd",                Matrix.TRS(crossStart + Vec3.Forward * 0.05f, Quat.LookDir(-Vec3.Forward), 0.25f), TextAlign.BottomCenter);
		Text.Add("Vec3.Cross(Fwd,Up)", Matrix.TRS(crossStart + right * 0.05f, Quat.LookDir(-Vec3.Forward), 0.25f), TextAlign.BottomCenter);

		UI.HandleEnd();

		Tests.Screenshot("CrossProduct.jpg", 0, 400, 400, 20, poseCross.position + screenshotFrom, poseCross.position + screenshotAt);

		Demo.ShowSummary(title, description, new Bounds(new Vec3(0,0,-.05f), new Vec3(.84f, .34f, 0)));
	}

	public void Initialize() {
		if (Tests.IsTesting)
			Time.SetTime(3.4, 1 / 90.0);

		planeMesh = Mesh.GeneratePlane(Vec2.One,Vec3.Forward, Vec3.Up);
		suzanne   = Model.FromFile("suzanne_bin.stl").RootNode.Mesh;
	}
	public void Shutdown  () { }
}