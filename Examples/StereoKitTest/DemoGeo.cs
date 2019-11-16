using StereoKit;

class DemoGeo : IDemo
{
    Material defaultMaterial;

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

    public void Initialize()
    {
        defaultMaterial = Material.Find("default/material");

        /// :CodeSample: Mesh.GenerateCube
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  cubeMesh  = Mesh.GenerateCube(Vec3.One * 0.4f);
        Model cubeModel = new Model(cubeMesh, Material.Copy(DefaultIds.material));
        /// :End:
        demoCubeMesh  = cubeMesh;
        demoCubeModel = cubeModel;

        /// :CodeSample: Mesh.GenerateRoundedCube
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  roundedCubeMesh  = Mesh.GenerateRoundedCube(Vec3.One * 0.4f, 0.05f);
        Model roundedCubeModel = new Model(roundedCubeMesh, Material.Copy(DefaultIds.material));
        /// :End:
        demoRoundedCubeMesh  = roundedCubeMesh;
        demoRoundedCubeModel = roundedCubeModel;

        /// :CodeSample: Mesh.GenerateSphere
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  sphereMesh  = Mesh.GenerateSphere(0.4f);
        Model sphereModel = new Model(sphereMesh, Material.Copy(DefaultIds.material));
        /// :End:
        demoSphereMesh  = sphereMesh;
        demoSphereModel = sphereModel;

        /// :CodeSample: Mesh.GenerateCylinder
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  cylinderMesh  = Mesh.GenerateCylinder(0.4f, 0.4f, Vec3.Up);
        Model cylinderModel = new Model(cylinderMesh, Material.Copy(DefaultIds.material));
        /// :End:
        demoCylinderMesh  = cylinderMesh;
        demoCylinderModel = cylinderModel;

        /// :CodeSample: Mesh.GeneratePlane
        /// ![Procedural Geometry Demo]({{site.url}}/img/screenshots/ProceduralGeometry.jpg)
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  planeMesh  = Mesh.GeneratePlane(Vec2.One*0.4f);
        Model planeModel = new Model(planeMesh, Material.Copy(DefaultIds.material));
        /// :End:
        demoPlaneMesh  = planeMesh;
        demoPlaneModel = planeModel;
    }

    public void Shutdown()
    {
    }

    public void Update()
    {
        // Cube!
        Mesh  cubeMesh  = demoCubeMesh;
        Model cubeModel = demoCubeModel;

        if (Demos.TestMode)
            Renderer.Screenshot(new Vec3(0.25f,1.5f,2f)*0.75f, Vec3.Zero, 600, 400, "../../../docs/img/screenshots/ProceduralGeometry.jpg");

        /// :CodeSample: Mesh.GenerateCube
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix cubeTransform = Matrix.TRS(new Vec3(-1.0f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(cubeMesh, defaultMaterial, cubeTransform);

        cubeTransform = Matrix.TRS(new Vec3(-1.0f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(cubeModel, cubeTransform);
        /// :End:
        

        // Rounded Cube!
        Mesh  roundedCubeMesh  = demoRoundedCubeMesh;
        Model roundedCubeModel = demoRoundedCubeModel;

        /// :CodeSample: Mesh.GenerateRoundedCube
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix roundedCubeTransform = Matrix.TRS(new Vec3(-0.5f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(roundedCubeMesh, defaultMaterial, roundedCubeTransform);

        roundedCubeTransform = Matrix.TRS(new Vec3(-0.5f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(roundedCubeModel, roundedCubeTransform);
        /// :End:


        // Rounded Sphere!
        Mesh  sphereMesh  = demoSphereMesh;
        Model sphereModel = demoSphereModel;

        /// :CodeSample: Mesh.GenerateSphere
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix sphereTransform = Matrix.TRS(new Vec3(0.0f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(sphereMesh, defaultMaterial, sphereTransform);

        sphereTransform = Matrix.TRS(new Vec3(0.0f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(sphereModel, sphereTransform);
        /// :End:


        // Cylinder!
        Mesh  cylinderMesh  = demoCylinderMesh;
        Model cylinderModel = demoCylinderModel;

        /// :CodeSample: Mesh.GenerateCylinder
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix cylinderTransform = Matrix.TRS(new Vec3(0.5f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(cylinderMesh, defaultMaterial, cylinderTransform);

        cylinderTransform = Matrix.TRS(new Vec3(0.5f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(cylinderModel, cylinderTransform);
        /// :End:
        /// 


        // Plane!
        Mesh  planeMesh  = demoPlaneMesh;
        Model planeModel = demoPlaneModel;

        /// :CodeSample: Mesh.GeneratePlane
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix planeTransform = Matrix.TRS(new Vec3(1.0f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(planeMesh, defaultMaterial, planeTransform);

        planeTransform = Matrix.TRS(new Vec3(1.0f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(planeModel, planeTransform);
        /// :End:

    }
}
