using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoMath : IDemo
{
    Mesh planeMesh;
    Mesh sphereMesh;
    Mesh cubeMesh;
    Material material;

    public void Update()
    {
        Color colIntersect = Color.HSV(0, 0.8f, 1);
        Color colTest      = Color.HSV(0, 0.6f, 1);
        Color colObj       = Color.HSV(0.05f, 0.7f, 1);

        // Plane and Ray
        Vec3  groundPlaneAt = new Vec3(-1.5f, 0, -0.5f);
        Plane ground    = new Plane(groundPlaneAt, new Vec3(1,2,0));
        Ray   groundRay = new Ray(groundPlaneAt + new Vec3(0,0.25f,0), Vec3.AngleXZ(Time.Totalf*90, -2).Normalized());

        Lines.Add(groundRay.position, groundRay.position + groundRay.direction * 0.1f, new Color32(255, 0, 0, 255), 2 * Units.mm2m);
        planeMesh.Draw(material, Matrix.TRS(groundPlaneAt, Quat.LookDir(ground.normal), 0.25f), colObj);
        if (groundRay.Intersect(ground, out Vec3 groundAt))
            sphereMesh.Draw(material, Matrix.TS(groundAt, 0.02f), colIntersect);

        if (Demos.TestMode)
            Renderer.Screenshot(groundPlaneAt+new Vec3(0.0f,0.3f,0.15f), groundPlaneAt+Vec3.Up*0.1f, 400, 400, "../../../docs/img/screenshots/RayIntersectPlane.jpg");

        // Line and Plane
        Vec3  groundLinePlaneAt = new Vec3(-1, 0, -0.5f);
        Plane groundLinePlane   = new Plane(groundLinePlaneAt, new Vec3(1,2,0));
        Ray   groundLineRay     = new Ray  (groundLinePlaneAt + new Vec3(0,0.25f,0), Vec3.AngleXZ(Time.Totalf*90, -2).Normalized());
        Vec3  groundLineP1  = groundLineRay.position + groundLineRay.direction * (SKMath.Cos(Time.Totalf*3)+1)*0.2f;
        Vec3  groundLineP2  = groundLineRay.position + groundLineRay.direction * ((SKMath.Cos(Time.Totalf*3)+1)*0.2f+ 0.1f);

        Lines.Add(groundLineP1, groundLineP2, colTest, 2 * Units.mm2m);
        sphereMesh.Draw(material, Matrix.TS(groundLineP1, 0.01f), colTest);
        sphereMesh.Draw(material, Matrix.TS(groundLineP2, 0.01f), colTest);
        bool groundLineIntersects = groundLinePlane.Intersect(groundLineP1, groundLineP2, out Vec3 groundLineAt);
        planeMesh.Draw(material, Matrix.TRS(groundLinePlaneAt, Quat.LookDir(groundLinePlane.normal), 0.25f), groundLineIntersects? colIntersect : colObj);
        if (groundLineIntersects)
            sphereMesh.Draw(material, Matrix.TS(groundLineAt, 0.02f), colIntersect);

        if (Demos.TestMode)
            Renderer.Screenshot(groundLinePlaneAt + new Vec3(0.0f, 0.3f, 0.15f), groundLinePlaneAt + Vec3.Up * 0.1f, 400, 400, "../../../docs/img/screenshots/LineIntersectPlane.jpg");

        // Sphere and Ray
        Sphere sphere    = new Sphere(new Vec3(0,0,-0.5f), 0.25f);
        Vec3   sphereDir = Vec3.AngleXZ(Time.Totalf*90, SKMath.Cos(Time.Totalf * 3) * 1.5f + 0.1f).Normalized();
        Ray    sphereRay = new Ray(sphere.center - sphereDir * 0.35f, sphereDir);

        Lines.Add(sphereRay.position, sphereRay.position + sphereRay.direction*0.1f, colTest, 2 * Units.mm2m);
        if (sphereRay.Intersect(sphere, out Vec3 sphereAt))
            sphereMesh.Draw(material, Matrix.TS(sphereAt, 0.02f), colIntersect);
        sphereMesh.Draw(material, Matrix.TS(sphere.center, 0.25f), colObj);

        if (Demos.TestMode)
            Renderer.Screenshot(sphere.center + new Vec3(0.0f, 0.3f, 0.15f), sphere.center, 400, 400, "../../../docs/img/screenshots/RayIntersectSphere.jpg");

        // Bounds and Ray
        Bounds bounds    = new Bounds(new Vec3(-.5f, 0, -0.5f), Vec3.One * 0.25f);
        Vec3   boundsDir = Vec3.AngleXZ(Time.Totalf*90, SKMath.Cos(Time.Totalf*3)*1.5f).Normalized();
        Ray    boundsRay = new Ray(bounds.center - boundsDir * 0.35f, boundsDir);

        Lines.Add(boundsRay.position, boundsRay.position + boundsRay.direction * 0.1f, colTest, 2 * Units.mm2m);
        if (boundsRay.Intersect(bounds, out Vec3 boundsAt))
            sphereMesh.Draw(material, Matrix.TS(boundsAt, 0.02f), colIntersect);
        cubeMesh.Draw(material, Matrix.TS(bounds.center, 0.25f), colObj);

        if (Demos.TestMode)
            Renderer.Screenshot(bounds.center + new Vec3(0.0f, 0.3f, 0.15f), bounds.center, 400, 400, "../../../docs/img/screenshots/RayIntersectBounds.jpg");

        // Bounds and Line
        Bounds boundsLine   = new Bounds(new Vec3(.5f, 0, -0.5f), Vec3.One * 0.25f);
        Vec3   boundsLineP1 = boundsLine.center + Vec3.AngleXZ(Time.Totalf*45, SKMath.Cos(Time.Totalf*3)) * 0.35f;
        Vec3   boundsLineP2 = boundsLine.center + Vec3.AngleXZ(Time.Totalf*90, SKMath.Cos(Time.Totalf*6)) * SKMath.Cos(Time.Totalf)*0.35f;

        Lines.Add(boundsLineP1, boundsLineP2, colTest, 2*Units.mm2m);
        sphereMesh.Draw(material, Matrix.TS(boundsLineP1, 0.01f), colTest);
        sphereMesh.Draw(material, Matrix.TS(boundsLineP2, 0.01f), colTest);
        cubeMesh  .Draw(material, Matrix.TS(boundsLine.center, 0.25f),
            boundsLine.Contains(boundsLineP1, boundsLineP2) ? colIntersect : colObj);

        if (Demos.TestMode)
            Renderer.Screenshot(boundsLine.center + new Vec3(0.0f, 0.3f, 0.15f), boundsLine.center, 400, 400, "../../../docs/img/screenshots/LineIntersectBounds.jpg");

        // Cross product
        Vec3 crossStart = new Vec3(0, 0, 0.5f);
        //Vec3 right      = Vec3.Cross(Vec3.Forward, Vec3.Up); // These are the same!
        Vec3 right      = Vec3.PerpendicularRight(Vec3.Forward, Vec3.Up);
        Lines.Add(crossStart, crossStart + Vec3.Up*0.1f,      new Color32(255,255,255,255), 2*Units.mm2m);
        Lines.Add(crossStart, crossStart + Vec3.Forward*0.1f, new Color32(255,255,255,255), 2*Units.mm2m);
        Lines.Add(crossStart, crossStart + right * 0.1f,      new Color32(0, 255, 0, 255),  2*Units.mm2m);
        Text.Add("Up",  Matrix.TRS(crossStart + Vec3.Up      * 0.1f, Quat.LookDir(-Vec3.Forward), 1), TextAlign.XCenter | TextAlign.YBottom);
        Text.Add("Fwd", Matrix.TRS(crossStart + Vec3.Forward * 0.1f, Quat.LookDir(-Vec3.Forward), 1), TextAlign.XCenter | TextAlign.YBottom);
        Text.Add("Vec3.Cross(Fwd,Up)", Matrix.TRS(crossStart + right * 0.1f, Quat.LookDir(-Vec3.Forward), 1), TextAlign.XCenter | TextAlign.YBottom);

        if (Demos.TestMode)
            Renderer.Screenshot(crossStart + new Vec3(0.075f, 0.1f, -0.15f), crossStart + new Vec3(0.075f,0,0), 400, 400, "../../../docs/img/screenshots/CrossProduct.jpg");
    }

    public void Initialize() {
        planeMesh  = Mesh.GeneratePlane(Vec2.One,Vec3.Forward, Vec3.Up);
        sphereMesh = Mesh.GenerateSphere(1);
        cubeMesh   = Mesh.GenerateCube(Vec3.One);
        material   = Material.Find(DefaultIds.material);
    }
    public void Shutdown  () { }
}