using StereoKit;

class DemoGeo : ITest
{
	string title       = "Mesh Generation";
	string description = "Generating a Mesh or Model via code can be an important task, and StereoKit provides a number of tools to make this pretty easy! In addition to the Default meshes, you can also generate a number of shapes, seen here. (See the Mesh.Gen functions)\n\nIf the provided shapes aren't enough, it's also pretty easy to procedurally assemble a mesh of your own from vertices and indices! That's the wavy surface all the way to the right.";

	Mesh  demoCubeMesh  = null;
	Model demoCubeModel = null;
	Mesh  demoRoundedCubeMesh  = null;
	Model demoRoundedCubeModel = null;
	Mesh  demoSphereMesh  = null;
	Model demoSphereModel = null;
	Mesh  demoCylinderMesh  = null;
	Model demoCylinderModel = null;
	Mesh  demoPlaneMesh  = null;
	Model demoPlaneModel = null;
	Mesh  demoCircleMesh = null;
	Model demoCircleModel = null;

	Mesh  demoProcMesh = null;

	public void Initialize()
	{
		/// :CodeSample: Mesh.GenerateCube
		/// ### Generating a Mesh and Model
		/// 
		/// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
		/// 
		/// Here's a quick example of generating a mesh! You can store it in just a
		/// Mesh, or you can attach it to a Model for easier rendering later on.
		Mesh  cubeMesh  = Mesh.GenerateCube(Vec3.One * 0.4f);
		Model cubeModel = Model.FromMesh(cubeMesh, Default.Material);
		/// :End:
		demoCubeMesh  = cubeMesh;
		demoCubeModel = cubeModel;

		/// :CodeSample: Mesh Mesh.GenerateRoundedCube Mesh.Draw
		/// ### Generating a Mesh and Model
		/// 
		/// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
		/// 
		/// Here's a quick example of generating a mesh! You can store it in just a
		/// Mesh, or you can attach it to a Model for easier rendering later on.
		// Do this in your initialization
		Mesh  roundedCubeMesh  = Mesh.GenerateRoundedCube(Vec3.One * 0.4f, 0.05f);
		Model roundedCubeModel = Model.FromMesh(roundedCubeMesh, Default.Material);
		/// :End:
		demoRoundedCubeMesh  = roundedCubeMesh;
		demoRoundedCubeModel = roundedCubeModel;

		/// :CodeSample: Mesh.GenerateSphere
		/// ### Generating a Mesh and Model
		/// 
		/// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
		/// 
		/// Here's a quick example of generating a mesh! You can store it in just a
		/// Mesh, or you can attach it to a Model for easier rendering later on.
		// Do this in your initialization
		Mesh  sphereMesh  = Mesh.GenerateSphere(0.4f);
		Model sphereModel = Model.FromMesh(sphereMesh, Default.Material);
		/// :End:
		demoSphereMesh  = sphereMesh;
		demoSphereModel = sphereModel;

		/// :CodeSample: Mesh.GenerateCylinder
		/// ### Generating a Mesh and Model
		/// 
		/// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
		/// 
		/// Here's a quick example of generating a mesh! You can store it in just a
		/// Mesh, or you can attach it to a Model for easier rendering later on.
		// Do this in your initialization
		Mesh  cylinderMesh  = Mesh.GenerateCylinder(0.4f, 0.4f, Vec3.Up);
		Model cylinderModel = Model.FromMesh(cylinderMesh, Default.Material);
		/// :End:
		demoCylinderMesh  = cylinderMesh;
		demoCylinderModel = cylinderModel;

		/// :CodeSample: Mesh.GeneratePlane
		/// ### Generating a Mesh and Model
		/// 
		/// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
		/// 
		/// Here's a quick example of generating a mesh! You can store it in just a
		/// Mesh, or you can attach it to a Model for easier rendering later on.
		// Do this in your initialization
		Mesh  planeMesh  = Mesh.GeneratePlane(Vec2.One*0.4f);
		Model planeModel = Model.FromMesh(planeMesh, Default.Material);
		/// :End:
		demoPlaneMesh  = planeMesh;
		demoPlaneModel = planeModel;

		/// :CodeSample: Mesh.GenerateCircle
		/// ### Generating a Mesh and Model
		/// 
		/// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
		/// 
		/// Here's a quick example of generating a mesh! You can store it in just a
		/// Mesh, or you can attach it to a Model for easier rendering later on.
		// Do this in your initialization
		Mesh  circleMesh  = Mesh.GenerateCircle(0.4f);
		Model circleModel = Model.FromMesh(circleMesh, Default.Material);
		/// :End:
		demoCircleMesh  = circleMesh;
		demoCircleModel = circleModel;

		/// :CodeSample: Mesh.SetVerts Mesh.SetInds Mesh.SetData
		/// ### Procedurally generating a wavy grid
		/// 
		/// ![Wavy Grid]({{site.url}}/img/screenshots/ProceduralGrid.jpg)
		/// 
		/// Here, we'll generate a grid mesh using Mesh.SetVerts and Mesh.SetInds! This
		/// is a common example of creating a grid using code, we're using a sin wave
		/// to make it more visually interesting, but you could also substitute this for
		/// something like sampling a heightmap, or a more interesting mathematical 
		/// formula!
		/// 
		/// Note: x+y*gridSize is the formula for 2D (x,y) access of a 1D array that represents
		/// a grid.
		const int   gridSize = 8;
		const float gridMaxF = gridSize-1;
		Vertex[] verts = new Vertex[gridSize*gridSize];
		uint  [] inds  = new uint  [gridSize*gridSize*6];

		for (int y = 0; y < gridSize; y++) {
		for (int x = 0; x < gridSize; x++) {
			// Create a vertex on a grid, centered about the origin. The dimensions extends
			// from -0.5 to +0.5 on the X and Z axes. The Y value is then sampled from 
			// a sin wave using the x and y values.
			//
			// The normal of the vertex is then calculated from the derivative of the Y 
			// value!
			verts[x+y*gridSize] = new Vertex(
				new Vec3(
					x/gridMaxF-0.5f,
					SKMath.Sin((x+y) * 0.7f)*0.1f,
					y/gridMaxF-0.5f),
				new Vec3(
					-SKMath.Cos((x+y) * 0.7f),
					1,
					-SKMath.Cos((x+y) * 0.7f)).Normalized,
				new Vec2(
					x / gridMaxF,
					y / gridMaxF));

			// Create triangle face indices from the current vertex, and the vertices
			// on the next row and column! Since there is no 'next' row and column on
			// the last row and column, we guard this with a check for size-1.
			if (x<gridSize-1 && y<gridSize-1)
			{
				int ind = (x+y*gridSize)*6;
				inds[ind  ] = (uint)((x+1)+(y+1)*gridSize);
				inds[ind+1] = (uint)((x+1)+(y  )*gridSize);
				inds[ind+2] = (uint)((x  )+(y+1)*gridSize);

				inds[ind+3] = (uint)((x  )+(y+1)*gridSize);
				inds[ind+4] = (uint)((x+1)+(y  )*gridSize);
				inds[ind+5] = (uint)((x  )+(y  )*gridSize);
			}
		} }
		demoProcMesh = new Mesh();
		demoProcMesh.SetData(verts, inds);
		/// :End:
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		Hierarchy.Push(Matrix.S(0.2f) * Demo.contentPose);

		Tests.Screenshot("ProceduralGeometry.jpg", 600, 600, Hierarchy.ToWorld(V.XYZ(0, -0.15f, 1.6f)), Hierarchy.ToWorld(V.XYZ(0, -0.15f, 0)));
		Tests.Screenshot("ProceduralGrid.jpg",     600, 600, Hierarchy.ToWorld(V.X0Z(-2, -0.7f)), Hierarchy.ToWorld(V.X0Z(-2, 0)));

		// Circle!
		Mesh  circleMesh = demoCircleMesh;
		Model circleModel = demoCircleModel;

		/// :CodeSample: Mesh.GenerateCircle
		/// Drawing both a Mesh and a Model generated this way is reasonably simple, 
		/// here's a short example! For the Mesh, you'll need to create your own material, 
		/// we just loaded up the default Material here.
		Matrix circleTransform = Matrix.T(-.5f, -1.5f, 0);
		circleMesh.Draw(Default.Material, circleTransform);

		circleTransform = Matrix.T(.5f, -1.5f, 0);
		circleModel.Draw(circleTransform);
		/// :End:


		// Plane!
		Mesh  planeMesh  = demoPlaneMesh;
		Model planeModel = demoPlaneModel;

		/// :CodeSample: Mesh.GeneratePlane
		/// Drawing both a Mesh and a Model generated this way is reasonably simple, 
		/// here's a short example! For the Mesh, you'll need to create your own material, 
		/// we just loaded up the default Material here.
		Matrix planeTransform = Matrix.T(-.5f, -1, 0);
		planeMesh.Draw(Default.Material, planeTransform);

		planeTransform = Matrix.T(.5f, -1, 0);
		planeModel.Draw(planeTransform);
		/// :End:
		

		// Cube!
		Mesh  cubeMesh  = demoCubeMesh;
		Model cubeModel = demoCubeModel;

		/// :CodeSample: Mesh.GenerateCube
		/// Drawing both a Mesh and a Model generated this way is reasonably simple, 
		/// here's a short example! For the Mesh, you'll need to create your own material, 
		/// we just loaded up the default Material here.
		// Call this code every Step

		Matrix cubeTransform = Matrix.T(-.5f, -.5f, 0);
		cubeMesh.Draw(Default.Material, cubeTransform);

		cubeTransform = Matrix.T(.5f, -.5f, 0);
		cubeModel.Draw(cubeTransform);
		/// :End:


		// Rounded Cube!
		Mesh  roundedCubeMesh  = demoRoundedCubeMesh;
		Model roundedCubeModel = demoRoundedCubeModel;

		/// :CodeSample: Mesh Mesh.GenerateRoundedCube Mesh.Draw
		/// Drawing both a Mesh and a Model generated this way is reasonably simple, 
		/// here's a short example! For the Mesh, you'll need to create your own material, 
		/// we just loaded up the default Material here.
		// Call this code every Step

		Matrix roundedCubeTransform = Matrix.T(-.5f, 0, 0);
		roundedCubeMesh.Draw(Default.Material, roundedCubeTransform);

		roundedCubeTransform = Matrix.T(.5f, 0, 0);
		roundedCubeModel.Draw(roundedCubeTransform);
		/// :End:


		// Rounded Sphere!
		Mesh  sphereMesh  = demoSphereMesh;
		Model sphereModel = demoSphereModel;

		/// :CodeSample: Mesh.GenerateSphere
		/// Drawing both a Mesh and a Model generated this way is reasonably simple, 
		/// here's a short example! For the Mesh, you'll need to create your own material, 
		/// we just loaded up the default Material here.
		// Call this code every Step

		Matrix sphereTransform = Matrix.T(-.5f, .5f, 0);
		sphereMesh.Draw(Default.Material, sphereTransform);

		sphereTransform = Matrix.T(.5f, .5f, 0);
		sphereModel.Draw(sphereTransform);
		/// :End:


		// Cylinder!
		Mesh  cylinderMesh  = demoCylinderMesh;
		Model cylinderModel = demoCylinderModel;

		/// :CodeSample: Mesh.GenerateCylinder
		/// Drawing both a Mesh and a Model generated this way is reasonably simple, 
		/// here's a short example! For the Mesh, you'll need to create your own material, 
		/// we just loaded up the default Material here.
		// Call this code every Step

		Matrix cylinderTransform = Matrix.T(-.5f, 1, 0);
		cylinderMesh.Draw(Default.Material, cylinderTransform);

		cylinderTransform = Matrix.T(.5f, 1, 0);
		cylinderModel.Draw(cylinderTransform);
		/// :End:

		demoProcMesh.Draw(Default.Material, Matrix.TR(-2,0,0, Quat.FromAngles(-90,0,0)));

		Hierarchy.Pop();

		Demo.ShowSummary(title, description);
	}
}
