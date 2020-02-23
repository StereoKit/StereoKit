using StereoKit;

class DemoGeo : ITest
{
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

    Mesh  demoProcMesh = null;

    public void Initialize()
    {
        /// :CodeSample: Mesh.GenerateCube
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  cubeMesh  = Mesh.GenerateCube(Vec3.One * 0.4f);
        Model cubeModel = Model.FromMesh(cubeMesh, Default.Material);
        /// :End:
        demoCubeMesh  = cubeMesh;
        demoCubeModel = cubeModel;

        /// :CodeSample: Mesh.GenerateRoundedCube
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  roundedCubeMesh  = Mesh.GenerateRoundedCube(Vec3.One * 0.4f, 0.05f);
        Model roundedCubeModel = Model.FromMesh(roundedCubeMesh, Default.Material);
        /// :End:
        demoRoundedCubeMesh  = roundedCubeMesh;
        demoRoundedCubeModel = roundedCubeModel;

        /// :CodeSample: Mesh.GenerateSphere
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  sphereMesh  = Mesh.GenerateSphere(0.4f);
        Model sphereModel = Model.FromMesh(sphereMesh, Default.Material);
        /// :End:
        demoSphereMesh  = sphereMesh;
        demoSphereModel = sphereModel;

        /// :CodeSample: Mesh.GenerateCylinder
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  cylinderMesh  = Mesh.GenerateCylinder(0.4f, 0.4f, Vec3.Up);
        Model cylinderModel = Model.FromMesh(cylinderMesh, Default.Material);
        /// :End:
        demoCylinderMesh  = cylinderMesh;
        demoCylinderModel = cylinderModel;

        /// :CodeSample: Mesh.GeneratePlane
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  planeMesh  = Mesh.GeneratePlane(Vec2.One*0.4f);
        Model planeModel = Model.FromMesh(planeMesh, Default.Material);
        /// :End:
        demoPlaneMesh  = planeMesh;
        demoPlaneModel = planeModel;

        /// :CodeSample: Mesh.SetVerts Mesh.SetInds
        /// ## Procedurally generating a wavy grid
        /// ![Wavy Grid]({{site.url}}/img/screenshots/ProceduralGrid.jpg)
        /// Here, we'll generate a grid mesh using Mesh.SetVerts and Mesh.SetInds! This
        /// is a common example of creating a grid using code, we're using a sin wave
        /// to make it more visually interesting, but you could also substitute this for
        /// something like sampling a heightmap, or a more interesting mathematical 
        /// formula!
        /// 
        /// Note: x+y*gridSize is the formula for 2D (x,y) access of a 1D array that represents
        /// a grid.
        const int gridSize = 8;
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
                    x/(float)gridSize-0.5f, 
                    SKMath.Sin((x+y) * 0.7f)*0.1f, 
                    y/(float)gridSize-0.5f),
                new Vec3(
                    -SKMath.Cos((x+y) * 0.7f), 
                    1, 
                    -SKMath.Cos((x+y) * 0.7f)).Normalized());

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
        demoProcMesh.SetVerts(verts);
        demoProcMesh.SetInds (inds);
        /// :End:
    }

    public void Shutdown()
    {
    }

    public void Update()
    {
        // Cube!
        Mesh  cubeMesh  = demoCubeMesh;
        Model cubeModel = demoCubeModel;

        Tests.Screenshot(600, 400, "ProceduralGeometry.jpg", new Vec3(0.25f, 1.5f, 2f) * 0.75f, Vec3.Zero);
        Tests.Screenshot(600, 400, "ProceduralGrid.jpg", new Vec3(0.358f, -0.013f, 0.222f), new Vec3(1.012f, -0.682f, -0.131f));

        /// :CodeSample: Mesh.GenerateCube
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix cubeTransform = Matrix.T(-1.0f, 0, 1);
        Renderer.Add(cubeMesh, Default.Material, cubeTransform);

        cubeTransform = Matrix.T(-1.0f, 0, -1);
        Renderer.Add(cubeModel, cubeTransform);
        /// :End:
        

        // Rounded Cube!
        Mesh  roundedCubeMesh  = demoRoundedCubeMesh;
        Model roundedCubeModel = demoRoundedCubeModel;

        /// :CodeSample: Mesh.GenerateRoundedCube
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix roundedCubeTransform = Matrix.T(-0.5f, 0, 1);
        Renderer.Add(roundedCubeMesh, Default.Material, roundedCubeTransform);

        roundedCubeTransform = Matrix.T(-0.5f, 0, -1);
        Renderer.Add(roundedCubeModel, roundedCubeTransform);
        /// :End:


        // Rounded Sphere!
        Mesh  sphereMesh  = demoSphereMesh;
        Model sphereModel = demoSphereModel;

        /// :CodeSample: Mesh.GenerateSphere
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix sphereTransform = Matrix.T(0.0f, 0, 1);
        Renderer.Add(sphereMesh, Default.Material, sphereTransform);

        sphereTransform = Matrix.T(0.0f, 0, -1);
        Renderer.Add(sphereModel, sphereTransform);
        /// :End:


        // Cylinder!
        Mesh  cylinderMesh  = demoCylinderMesh;
        Model cylinderModel = demoCylinderModel;

        /// :CodeSample: Mesh.GenerateCylinder
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix cylinderTransform = Matrix.T(0.5f, 0, 1);
        Renderer.Add(cylinderMesh, Default.Material, cylinderTransform);

        cylinderTransform = Matrix.T(0.5f, 0, -1);
        Renderer.Add(cylinderModel, cylinderTransform);
        /// :End:


        // Plane!
        Mesh  planeMesh  = demoPlaneMesh;
        Model planeModel = demoPlaneModel;

        /// :CodeSample: Mesh.GeneratePlane
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix planeTransform = Matrix.T(1.0f, 0, 1);
        Renderer.Add(planeMesh, Default.Material, planeTransform);

        planeTransform = Matrix.T(1.0f, 0, -1);
        Renderer.Add(planeModel, planeTransform);
        /// :End:
        
        demoProcMesh.Draw(Default.Material, Matrix.T(1,-.3f,0));
    }
}
