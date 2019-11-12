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

    public void Initialize()
    {
        defaultMaterial = Material.Find("default/material");

        /// :CodeSample: Mesh.GenerateCube
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  cubeMesh  = Mesh.GenerateCube(Vec3.One * 0.8f);
        Model cubeModel = new Model(cubeMesh, Material.Copy(DefaultIds.material));
        /// :End:
        demoCubeMesh  = cubeMesh;
        demoCubeModel = cubeModel;

        /// :CodeSample: Mesh.GenerateRoundedCube
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  roundedCubeMesh  = Mesh.GenerateRoundedCube(Vec3.One * 0.8f, 0.2f);
        Model roundedCubeModel = new Model(roundedCubeMesh, Material.Copy(DefaultIds.material));
        /// :End:
        demoRoundedCubeMesh  = roundedCubeMesh;
        demoRoundedCubeModel = roundedCubeModel;

        /// :CodeSample: Mesh.GenerateSphere
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  sphereMesh  = Mesh.GenerateSphere(0.8f);
        Model sphereModel = new Model(sphereMesh, Material.Copy(DefaultIds.material));
        /// :End:
        demoSphereMesh  = sphereMesh;
        demoSphereModel = sphereModel;

        /// :CodeSample: Mesh.GenerateCylinder
        /// Here's a quick example of generating a mesh! You can store it in just a
        /// Mesh, or you can attach it to a Model for easier rendering later on.
        Mesh  cylinderMesh  = Mesh.GenerateCylinder(0.8f, 0.8f, Vec3.Up);
        Model cylinderModel = new Model(cylinderMesh, Material.Copy(DefaultIds.material));
        /// :End:
        demoCylinderMesh  = cylinderMesh;
        demoCylinderModel = cylinderModel;
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
            Renderer.Screenshot(new Vec3(0.25f,1.5f,2f), Vec3.Zero, "../../../docs/img/screenshots/ProceduralGeometry.jpg");

        /// :CodeSample: Mesh.GenerateCube
        /// ![Procedural Geometry Demo](/docs/img/screenshots/ProceduralGeometry.png)
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix cubeTransform = Matrix.TRS(new Vec3(-1.5f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(cubeMesh, defaultMaterial, cubeTransform);

        cubeTransform = Matrix.TRS(new Vec3(-1.5f, 0, -1), Quat.Identity, Vec3.One);
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
        Matrix sphereTransform = Matrix.TRS(new Vec3(0.5f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(sphereMesh, defaultMaterial, sphereTransform);

        sphereTransform = Matrix.TRS(new Vec3(0.5f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(sphereModel, sphereTransform);
        /// :End:


        // Cylinder!
        Mesh  cylinderMesh  = demoCylinderMesh;
        Model cylinderModel = demoCylinderModel;

        /// :CodeSample: Mesh.GenerateCylinder
        /// Drawing both a Mesh and a Model generated this way is reasonably simple, 
        /// here's a short example! For the Mesh, you'll need to create your own material, 
        /// we just loaded up the default Material here.
        Matrix cylinderTransform = Matrix.TRS(new Vec3(1.5f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(cylinderMesh, defaultMaterial, cylinderTransform);

        cylinderTransform = Matrix.TRS(new Vec3(1.5f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(cylinderModel, cylinderTransform);
        /// :End:

    }
}
