using StereoKit;

class DemoMath : ITest
{
    Mesh suzanne;
    Mesh planeMesh;
    Mesh sphereMesh;
    Mesh cubeMesh;
    Material material;

    Mesh     boundsMesh;
    Material boundsMat;
    Pose posePlaneRay   = new Pose(-1.5f, 0, -0.5f, Quat.Identity);
    Pose poseLinePlane  = new Pose(-1f, 0, -0.5f, Quat.Identity);
    Pose poseSphereRay  = new Pose(0, 0, -0.5f, Quat.Identity);
    Pose poseBoundsRay  = new Pose(-.5f, 0, -0.5f, Quat.Identity);
    Pose poseBoundsLine = new Pose(.5f, 0, -0.5f, Quat.Identity);
    Pose poseMeshRay    = new Pose(-.5f, 0, 0.5f, Quat.Identity);
    Pose poseCross      = new Pose(0, 0, 0.5f, Quat.Identity);

    public void Update()
    {
        Color colIntersect = Color.HSV(0, 0.8f, 1);
        Color colTest      = Color.HSV(0, 0.6f, 1);
        Color colObj       = Color.HSV(0.05f, 0.7f, 1);
        Color active       = new Color(1,1,1,0.7f);
        Color notActive    = new Color(1, 1, 1, 1);

        // Plane and Ray
        bool planeRayActive = UI.HandleBegin("PlaneRay", ref posePlaneRay, new Bounds(Vec3.One*0.4f));
        boundsMesh.Draw(boundsMat, Matrix.TS(Vec3.Zero, 0.4f), planeRayActive ? active:notActive);

        Plane ground    = new Plane(Vec3.Zero, new Vec3(1,2,0));
        Ray   groundRay = new Ray(Vec3.Zero + new Vec3(0,0.2f,0), Vec3.AngleXZ(Time.Totalf*90, -2).Normalized());

        Lines.Add(groundRay.position, groundRay.position + groundRay.direction * 0.1f, new Color32(255, 0, 0, 255), 2 * Units.mm2m);
        planeMesh.Draw(material, Matrix.TRS(Vec3.Zero, Quat.LookDir(ground.normal), 0.25f), colObj);
        if (groundRay.Intersect(ground, out Vec3 groundAt))
            sphereMesh.Draw(material, Matrix.TS(groundAt, 0.02f), colIntersect);

        UI.HandleEnd();

        if (Tests.IsTesting)
            Renderer.Screenshot(posePlaneRay.position+new Vec3(0.0f,0.3f,0.15f), posePlaneRay.position + Vec3.Up*0.1f, 400, 400, "../../../docs/img/screenshots/RayIntersectPlane.jpg");

        // Line and Plane
        bool linePlaneActive = UI.HandleBegin("LinePlane", ref poseLinePlane, new Bounds(Vec3.One * 0.4f));
        boundsMesh.Draw(boundsMat, Matrix.TS(Vec3.Zero, 0.4f), linePlaneActive ? active : notActive);

        Plane groundLinePlane   = new Plane(Vec3.Zero, new Vec3(1,2,0));
        Ray   groundLineRay     = new Ray  (Vec3.Zero + new Vec3(0,0.25f,0), Vec3.AngleXZ(Time.Totalf*90, -2).Normalized());
        Vec3  groundLineP1  = groundLineRay.position + groundLineRay.direction * (SKMath.Cos(Time.Totalf*3)+1)*0.2f;
        Vec3  groundLineP2  = groundLineRay.position + groundLineRay.direction * ((SKMath.Cos(Time.Totalf*3)+1)*0.2f+ 0.1f);

        Lines.Add(groundLineP1, groundLineP2, colTest, 2 * Units.mm2m);
        sphereMesh.Draw(material, Matrix.TS(groundLineP1, 0.01f), colTest);
        sphereMesh.Draw(material, Matrix.TS(groundLineP2, 0.01f), colTest);
        bool groundLineIntersects = groundLinePlane.Intersect(groundLineP1, groundLineP2, out Vec3 groundLineAt);
        planeMesh.Draw(material, Matrix.TRS(Vec3.Zero, Quat.LookDir(groundLinePlane.normal), 0.25f), groundLineIntersects? colIntersect : colObj);
        if (groundLineIntersects)
            sphereMesh.Draw(material, Matrix.TS(groundLineAt, 0.02f), colIntersect);

        UI.HandleEnd();

        if (Tests.IsTesting)
            Renderer.Screenshot(poseLinePlane.position + new Vec3(0.0f, 0.3f, 0.15f), poseLinePlane.position + Vec3.Up * 0.1f, 400, 400, "../../../docs/img/screenshots/LineIntersectPlane.jpg");

        // Sphere and Ray
        bool sphereRayActive = UI.HandleBegin("SphereRay", ref poseSphereRay, new Bounds(Vec3.One * 0.4f));
        boundsMesh.Draw(boundsMat, Matrix.TS(Vec3.Zero, 0.4f), sphereRayActive ? active : notActive);

        Sphere sphere    = new Sphere(Vec3.Zero, 0.25f);
        Vec3   sphereDir = Vec3.AngleXZ(Time.Totalf*90, SKMath.Cos(Time.Totalf * 3) * 1.5f + 0.1f).Normalized();
        Ray    sphereRay = new Ray(sphere.center - sphereDir * 0.35f, sphereDir);

        Lines.Add(sphereRay.position, sphereRay.position + sphereRay.direction*0.1f, colTest, 2 * Units.mm2m);
        if (sphereRay.Intersect(sphere, out Vec3 sphereAt))
            sphereMesh.Draw(material, Matrix.TS(sphereAt, 0.02f), colIntersect);
        sphereMesh.Draw(material, Matrix.TS(sphere.center, 0.25f), colObj);

        UI.HandleEnd();

        if (Tests.IsTesting)
            Renderer.Screenshot(poseSphereRay.position + new Vec3(0.0f, 0.3f, 0.15f), poseSphereRay.position, 400, 400, "../../../docs/img/screenshots/RayIntersectSphere.jpg");

        // Bounds and Ray
        bool boundsRayActive = UI.HandleBegin("BoundsRay", ref poseBoundsRay, new Bounds(Vec3.One * 0.4f));
        boundsMesh.Draw(boundsMat, Matrix.TS(Vec3.Zero, 0.4f), boundsRayActive ? active : notActive);

        Bounds bounds    = new Bounds(Vec3.Zero, Vec3.One * 0.25f);
        Vec3   boundsDir = Vec3.AngleXZ(Time.Totalf*90, SKMath.Cos(Time.Totalf*3)*1.5f).Normalized();
        Ray    boundsRay = new Ray(bounds.center - boundsDir * 0.35f, boundsDir);

        Lines.Add(boundsRay.position, boundsRay.position + boundsRay.direction * 0.1f, colTest, 2 * Units.mm2m);
        if (boundsRay.Intersect(bounds, out Vec3 boundsAt))
            sphereMesh.Draw(material, Matrix.TS(boundsAt, 0.02f), colIntersect);
        cubeMesh.Draw(material, Matrix.TS(bounds.center, 0.25f), colObj);

        UI.HandleEnd();

        if (Tests.IsTesting)
            Renderer.Screenshot(poseBoundsRay.position + new Vec3(0.0f, 0.3f, 0.15f), poseBoundsRay.position, 400, 400, "../../../docs/img/screenshots/RayIntersectBounds.jpg");

        // Bounds and Line
        bool boundsLineActive = UI.HandleBegin("BoundsLine", ref poseBoundsLine, new Bounds(Vec3.One * 0.4f));
        boundsMesh.Draw(boundsMat, Matrix.TS(Vec3.Zero, 0.4f), boundsLineActive ? active : notActive);

        Bounds boundsLine   = new Bounds(Vec3.Zero, Vec3.One * 0.25f);
        Vec3   boundsLineP1 = boundsLine.center + Vec3.AngleXZ(Time.Totalf*45, SKMath.Cos(Time.Totalf*3)) * 0.35f;
        Vec3   boundsLineP2 = boundsLine.center + Vec3.AngleXZ(Time.Totalf*90, SKMath.Cos(Time.Totalf*6)) * SKMath.Cos(Time.Totalf)*0.35f;

        Lines.Add(boundsLineP1, boundsLineP2, colTest, 2*Units.mm2m);
        sphereMesh.Draw(material, Matrix.TS(boundsLineP1, 0.01f), colTest);
        sphereMesh.Draw(material, Matrix.TS(boundsLineP2, 0.01f), colTest);
        cubeMesh  .Draw(material, Matrix.TS(boundsLine.center, 0.25f),
            boundsLine.Contains(boundsLineP1, boundsLineP2) ? colIntersect : colObj);

        UI.HandleEnd();

        if (Tests.IsTesting)
            Renderer.Screenshot(poseBoundsLine.position + new Vec3(0.0f, 0.3f, 0.15f), poseBoundsLine.position, 400, 400, "../../../docs/img/screenshots/LineIntersectBounds.jpg");

        // Mesh and Ray
        bool meshRayActive = UI.HandleBegin("MeshRay", ref poseMeshRay, new Bounds(Vec3.One * 0.4f));
        boundsMesh.Draw(boundsMat, Matrix.TS(Vec3.Zero, 0.4f), meshRayActive ? active : notActive);
        suzanne.Draw(material, Matrix.T(Vec3.Zero), colObj);

        Vec3 meshDir = Vec3.AngleXZ(Time.Totalf * 90, SKMath.Cos(Time.Totalf * 3) * 1.5f).Normalized();
        Ray  meshRay = new Ray(bounds.center+Vec3.Up*0.1f - meshDir * 0.35f, meshDir);
        if (meshRay.Intersect(suzanne, out Vec3 meshAt)) {
            Lines.Add(meshRay.position, meshAt, colTest, 2 * Units.mm2m);
            sphereMesh.Draw(material, Matrix.TS(meshAt, 0.02f), colIntersect);
        } else {
            Lines.Add(meshRay.position, meshRay.position + meshRay.direction * 0.4f, colTest, 2 * Units.mm2m);
        }

        UI.HandleEnd();

        //if (Tests.IsTesting)
        //    Renderer.Screenshot(poseBoundsLine.position + new Vec3(0.0f, 0.3f, 0.15f), poseBoundsLine.position, 400, 400, "../../../docs/img/screenshots/LineIntersectBounds.jpg");

        // Cross product
        bool crossActive = UI.HandleBegin("Cross", ref poseCross, new Bounds(Vec3.One * 0.4f));
        boundsMesh.Draw(boundsMat, Matrix.TS(Vec3.Zero, 0.4f), crossActive ? active : notActive);

        Vec3 crossStart = Vec3.Zero;
        //Vec3 right      = Vec3.Cross(Vec3.Forward, Vec3.Up); // These are the same!
        Vec3 right      = Vec3.PerpendicularRight(Vec3.Forward, Vec3.Up);
        Lines.Add(crossStart, crossStart + Vec3.Up*0.1f,      new Color32(255,255,255,255), 2*Units.mm2m);
        Lines.Add(crossStart, crossStart + Vec3.Forward*0.1f, new Color32(255,255,255,255), 2*Units.mm2m);
        Lines.Add(crossStart, crossStart + right * 0.1f,      new Color32(0, 255, 0, 255),  2*Units.mm2m);
        Text.Add("Up",  Matrix.TRS(crossStart + Vec3.Up      * 0.1f, Quat.LookDir(-Vec3.Forward), 1), TextAlign.XCenter | TextAlign.YBottom);
        Text.Add("Fwd", Matrix.TRS(crossStart + Vec3.Forward * 0.1f, Quat.LookDir(-Vec3.Forward), 1), TextAlign.XCenter | TextAlign.YBottom);
        Text.Add("Vec3.Cross(Fwd,Up)", Matrix.TRS(crossStart + right * 0.1f, Quat.LookDir(-Vec3.Forward), 1), TextAlign.XCenter | TextAlign.YBottom);

        UI.HandleEnd();

        if (Tests.IsTesting)
            Renderer.Screenshot(poseCross.position + new Vec3(0.075f, 0.1f, 0.15f), poseCross.position + new Vec3(0.075f,0,0), 400, 400, "../../../docs/img/screenshots/CrossProduct.jpg");
    }

    public void Initialize() {
        if (Tests.IsTesting)
            Time.SetTime(3.4, 1 / 90.0);

        planeMesh  = Mesh.GeneratePlane(Vec2.One,Vec3.Forward, Vec3.Up);
        sphereMesh = Mesh.GenerateSphere(1);
        cubeMesh   = Mesh.GenerateCube(Vec3.One);
        material   = Default.Material;
        suzanne    = Model.FromFile("suzanne_bin.stl").GetMesh(0);

        boundsMesh = cubeMesh;
        boundsMat  = Default.MaterialUI.Copy();
        boundsMat.Transparency = Transparency.Blend;
        boundsMat[MatParamName.ColorTint] = new Color(1,1,1,0.25f);
    }
    public void Shutdown  () { }
}