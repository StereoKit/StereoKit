using StereoKit;

class DemoProceduralGeometry : IDemo
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
        Mesh  cubeMesh  = Mesh.GenerateCube(Vec3.One * 0.8f);
        Model cubeModel = new Model(cubeMesh, Material.Copy("default/material"));
        /// :End:
        demoCubeMesh  = cubeMesh;
        demoCubeModel = cubeModel;

        /// :CodeSample: Mesh.GenerateRoundedCube
        Mesh  roundedCubeMesh  = Mesh.GenerateRoundedCube(Vec3.One * 0.8f, 0.2f);
        Model roundedCubeModel = new Model(roundedCubeMesh, Material.Copy("default/material"));
        /// :End:
        demoRoundedCubeMesh  = roundedCubeMesh;
        demoRoundedCubeModel = roundedCubeModel;

        /// :CodeSample: Mesh.GenerateSphere
        Mesh  sphereMesh  = Mesh.GenerateSphere(0.8f);
        Model sphereModel = new Model(sphereMesh, Material.Copy("default/material"));
        /// :End:
        demoSphereMesh  = sphereMesh;
        demoSphereModel = sphereModel;

        /// :CodeSample: Mesh.GenerateCylinder
        Mesh  cylinderMesh  = Mesh.GenerateCylinder(0.8f, 0.8f, Vec3.Up);
        Model cylinderModel = new Model(cylinderMesh, Material.Copy("default/material"));
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

        /// :CodeSample: Mesh.GenerateCube
        Matrix cubeTransform = Matrix.TRS(new Vec3(-1.5f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(cubeMesh, defaultMaterial, cubeTransform);

        cubeTransform = Matrix.TRS(new Vec3(-1.5f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(cubeModel, cubeTransform);
        /// :end:
        

        // Rounded Cube!
        Mesh  roundedCubeMesh  = demoRoundedCubeMesh;
        Model roundedCubeModel = demoRoundedCubeModel;

        /// :CodeSample: Mesh.GenerateRoundedCube
        Matrix roundedCubeTransform = Matrix.TRS(new Vec3(-0.5f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(roundedCubeMesh, defaultMaterial, roundedCubeTransform);

        roundedCubeTransform = Matrix.TRS(new Vec3(-0.5f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(roundedCubeModel, roundedCubeTransform);
        /// :end:


        // Rounded Sphere!
        Mesh  sphereMesh  = demoSphereMesh;
        Model sphereModel = demoSphereModel;

        /// :CodeSample: Mesh.GenerateSphere
        Matrix sphereTransform = Matrix.TRS(new Vec3(0.5f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(sphereMesh, defaultMaterial, sphereTransform);

        sphereTransform = Matrix.TRS(new Vec3(0.5f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(sphereModel, sphereTransform);
        /// :end:


        // Cylinder!
        Mesh  cylinderMesh  = demoCylinderMesh;
        Model cylinderModel = demoCylinderModel;

        /// :CodeSample: Mesh.GenerateSphere
        Matrix cylinderTransform = Matrix.TRS(new Vec3(1.5f, 0, 1), Quat.Identity, Vec3.One);
        Renderer.Add(cylinderMesh, defaultMaterial, cylinderTransform);

        cylinderTransform = Matrix.TRS(new Vec3(1.5f, 0, -1), Quat.Identity, Vec3.One);
        Renderer.Add(cylinderModel, cylinderTransform);
        /// :end:

    }
}
